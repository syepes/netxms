/*
 ** LoraWAN subagent
 ** Copyright (C) 2009 - 2017 Raden Solutions
 **
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation; either version 2 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software
 ** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 **
 **/

#include "lorawan.h"

/**
 * MQTT connection settings
 */
static MqttClient *s_mqtt;

/**
 * LoraWAN server link
 */
static LoraWanServerLink *s_link = NULL;

/**
 * Device map mutex
 */
static MUTEX s_deviceMapMutex = INVALID_MUTEX_HANDLE;

/**
 * Parameters
 */
static NETXMS_SUBAGENT_PARAM m_parameters[] =
{
   { _T("LoraWAN.RSSI(*)"), H_Communication, _T("R"), DCI_DT_INT, _T("RSSI") },
   { _T("LoraWAN.SNR(*)"), H_Communication, _T("S"), DCI_DT_INT, _T("SNR") },
   { _T("LoraWAN.Frequency(*)"), H_Communication, _T("F"), DCI_DT_UINT, _T("Frequency") },
   { _T("LoraWAN.MessageCount(*)"), H_Communication, _T("M"), DCI_DT_UINT, _T("Message count") },
   { _T("LoraWAN.DataRate(*)"), H_Communication, _T("D"), DCI_DT_STRING, _T("Data rate") },
   { _T("LoraWAN.LastContact(*)"), H_Communication, _T("C"), DCI_DT_STRING, _T("Last contact") }
};

/**
 * Add new device to local map and DB
 */
UINT32 AddDevice(deviceData *data)
{
   UINT32 rcc = ERR_IO_FAILURE;

   DB_HANDLE hdb = AgentGetLocalDatabaseHandle();
   DB_STATEMENT hStmt = DBPrepare(hdb, _T("INSERT INTO device_decoder_map(guid,devAddr,devEui,decoder) VALUES (?,?,?,?)"));

   if (hStmt != NULL)
   {
      DBBind(hStmt, 1, DB_SQLTYPE_VARCHAR, data->guid);
      DBBind(hStmt, 2, DB_SQLTYPE_VARCHAR, data->devAddr);
      DBBind(hStmt, 3, DB_SQLTYPE_VARCHAR, data->devEui);
      DBBind(hStmt, 4, DB_SQLTYPE_INTEGER, data->decoder);
      if (DBExecute(hStmt))
      {
         MutexLock(s_deviceMapMutex);
         s_deviceMap.set(data->guid, data);
         MutexUnlock(s_deviceMapMutex);
         rcc = ERR_SUCCESS;
      }
      else
         rcc = ERR_EXEC_FAILED;

      DBFreeStatement(hStmt);
   }
   DBConnectionPoolReleaseConnection(hdb);

   return rcc;
}

/**
 * Remove device from local map and DB
 */
UINT32 RemoveDevice(uuid guid)
{
   UINT32 rcc = ERR_IO_FAILURE;

   DB_HANDLE hdb = AgentGetLocalDatabaseHandle();
   DB_STATEMENT hStmt = DBPrepare(hdb, _T("DELETE FROM device_decoder_map WHERE guid=?"));

   if (hStmt != NULL)
   {
      DBBind(hStmt, 1, DB_SQLTYPE_VARCHAR, guid);
      if (DBExecute(hStmt))
      {
         MutexLock(s_deviceMapMutex);
         s_deviceMap.remove(guid);
         MutexUnlock(s_deviceMapMutex);

         rcc = ERR_SUCCESS;
      }
      else
         rcc = ERR_EXEC_FAILED;

      DBFreeStatement(hStmt);
   }
   DBConnectionPoolReleaseConnection(hdb);

   return rcc;
}

/**
 * Find device in local map
 */
struct deviceData *FindDevice(uuid guid)
{
   struct deviceData *data;

   MutexLock(s_deviceMapMutex);
   data = s_deviceMap.get(guid);
   MutexUnlock(s_deviceMapMutex);

   return data;
}

/**
 * Load LoraWAN devices from DB
 */
static void LoadDevices()
{
   DB_HANDLE hdb = AgentGetLocalDatabaseHandle();
   DB_RESULT hResult = DBSelect(hdb, _T("SELECT guid,devAddr,decoder,devEui FROM device_decoder_map"));

   if (hResult != NULL)
   {
      UINT32 nRows = DBGetNumRows(hResult);
      MutexLock(s_deviceMapMutex);
      for(int i = 0; i < nRows; i++)
      {
         struct deviceData *data = new struct deviceData();
         data->guid = DBGetFieldGUID(hResult, i, 0);
         DBGetFieldByteArray2(hResult, i, 1, data->devAddr, 4, 0);
         data->decoder = DBGetFieldULong(hResult, i, 2);
         DBGetFieldByteArray2(hResult, i, 3, data->devEui, 8, 0);
         s_deviceMap.set(data->guid, data);
      }
      MutexUnlock(s_deviceMapMutex);
      DBFreeResult(hResult);
   }
   else
      nxlog_debug(4, _T("LoraWAN Subagent: Unable to load device map table"));

   DBConnectionPoolReleaseConnection(hdb);
}

/**
 * Process commands regarding LoraWAN devices
 */
static BOOL ProcessCommands(UINT32 command, NXCPMessage *request, NXCPMessage *response, AbstractCommSession *session)
{
   switch(command)
   {
      case CMD_MODIFY_OBJECT:
         char buffer[MAX_CONFIG_VALUE];
         request->getFieldAsMBString(VID_XML_CONFIG, buffer, MAX_CONFIG_VALUE);
         s_link->registerDevice(buffer, request->getFieldAsGUID(VID_GUID));
         return TRUE;
         break;
      case CMD_DELETE_OBJECT:
         response->setField(VID_RCC, s_link->deleteDevice(request->getFieldAsGUID(VID_GUID)));
         return TRUE;
         break;
      default:
         return FALSE;
   }
}

/**
 * Startup handler
 */
static BOOL SubagentInit(Config *config)
{
   s_deviceMapMutex = MutexCreate();

   LoadDevices();

   s_mqtt = new MqttClient(config->getEntry(_T("/LORAWAN")));
   s_mqtt->setMessageHandler(MqttMessageHandler);
   s_mqtt->startNetworkLoop();
   s_link = new LoraWanServerLink(config->getEntry(_T("/LORAWAN")));

   s_link->connect();

   return TRUE;
}

/**
 * Shutdown handler
 */
static void SubagentShutdown()
{
   s_mqtt->stopNetworkLoop();

   MutexDestroy(s_deviceMapMutex);
   delete(s_mqtt);
   delete(s_link);
}

/**
 * Subagent information
 */
static NETXMS_SUBAGENT_INFO m_info =
{
	NETXMS_SUBAGENT_INFO_MAGIC,
	_T("LORAWAN"), NETXMS_VERSION_STRING,
	SubagentInit,
	SubagentShutdown,
	ProcessCommands, // command handler
	sizeof(m_parameters) / sizeof(NETXMS_SUBAGENT_PARAM),
	m_parameters,
	0, NULL,		// lists
	0, NULL,		// tables
   0, NULL,    // actions
	0, NULL		// push parameters
};

/**
 * Entry point for NetXMS agent
 */
DECLARE_SUBAGENT_ENTRY_POINT(LORAWAN)
{
	*ppInfo = &m_info;
	return TRUE;
}
