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
 * Copy string depending on encoding
 */
static void CopyString(const char *value, TCHAR *buffer, size_t length)
{
#ifdef UNICODE
      MultiByteToWideChar(CP_UTF8, 0, value, -1, buffer, length);
#else
      nx_strncpy(buffer, value, length);
#endif
}

/**
 * Create new Lora Device Data object from NXCPMessage
 */
LoraDeviceData::LoraDeviceData(NXCPMessage *request)
{
   m_guid = request->getFieldAsGUID(VID_GUID);
   if (request->getFieldAsUInt32(VID_REG_TYPE) == 0) // OTAA
      m_devEui = request->getFieldAsMacAddress(VID_MAC_ADDR);
   else
   {
      char devAddr[12];
      request->getFieldAsMBString(VID_DEVICE_ADDRESS, devAddr, 12);
      m_devAddr = MacAddress::parse(devAddr);
   }

   memset(m_payload, 0, 36);
   m_decoder = request->getFieldAsInt32(VID_DECODER);
   m_dataRate[0] = 0;
   m_rssi = 0;
   m_snr = -100;
   m_freq = 0;
   m_fcnt = 0;
   m_port = 0;
   m_lastContact[0] = 0;
}

/**
 * Create Lora Device Data object from DB record
 */
LoraDeviceData::LoraDeviceData(DB_RESULT result, int row)
{
   m_guid = DBGetFieldGUID(result, row, 0);
   m_devAddr = DBGetFieldMacAddr(result, row, 1);
   m_devEui = DBGetFieldMacAddr(result, row, 2);
   m_decoder = DBGetFieldULong(result, row, 3);

   memset(m_payload, 0, 36);
   m_dataRate[0] = 0;
   m_rssi = 0;
   m_snr = -100;
   m_freq = 0;
   m_fcnt = 0;
   m_port = 0;
   m_lastContact[0] = 0;
}

LoraDeviceData::~LoraDeviceData()
{

}

/**
 * Update device DevAddr
 */
void LoraDeviceData::setDevAddr(MacAddress devAddr)
{
   m_devAddr = devAddr;
   save();
}

/**
 * Save new device in the database and local map
 */
UINT32 LoraDeviceData::save()
{
   UINT32 rcc = ERR_IO_FAILURE;

   DB_HANDLE hdb = AgentGetLocalDatabaseHandle();
   DB_STATEMENT hStmt;
   if (FindDevice(m_guid) == NULL)
      hStmt = DBPrepare(hdb, _T("INSERT INTO device_decoder_map(devAddr,devEui,decoder,guid) VALUES (?,?,?,?)"));
   else
      hStmt = DBPrepare(hdb, _T("UPDATE device_decoder_map SET devAddr=?,devEui=?,decoder=? WHERE guid=?"));

   if (hStmt != NULL)
   {
      DBBind(hStmt, 1, DB_SQLTYPE_VARCHAR, m_devAddr.length() > 0 ? (const TCHAR*)m_devAddr.toString(MAC_ADDR_FLAT_STRING) : _T(""), DB_BIND_STATIC);
      DBBind(hStmt, 2, DB_SQLTYPE_VARCHAR, m_devEui.length() > 0 ? (const TCHAR*)m_devEui.toString(MAC_ADDR_FLAT_STRING) : _T(""), DB_BIND_STATIC);
      DBBind(hStmt, 3, DB_SQLTYPE_INTEGER, m_decoder);
      DBBind(hStmt, 4, DB_SQLTYPE_VARCHAR, m_guid);
      if (DBExecute(hStmt))
      {
         MutexLock(g_deviceMapMutex);
         g_deviceMap.set(m_guid, this);
         MutexUnlock(g_deviceMapMutex);
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
UINT32 LoraDeviceData::remove()
{
   UINT32 rcc = ERR_IO_FAILURE;

   DB_HANDLE hdb = AgentGetLocalDatabaseHandle();
   DB_STATEMENT hStmt = DBPrepare(hdb, _T("DELETE FROM device_decoder_map WHERE guid=?"));

   if (hStmt != NULL)
   {
      DBBind(hStmt, 1, DB_SQLTYPE_VARCHAR, m_guid);
      if (DBExecute(hStmt))
      {
         MutexLock(g_deviceMapMutex);
         g_deviceMap.remove(m_guid);
         MutexUnlock(g_deviceMapMutex);

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
 * Set device data rate
 */
void LoraDeviceData::setDataRate(const char *dataRate)
{
   CopyString(dataRate, m_dataRate, 24);
}

/**
 * Update device last contact
 */
void LoraDeviceData::setLastContact()
{
   char date[64];
   struct tm *pCurrentTM;
   INT64 now = GetCurrentTimeMs();
   time_t t = now / 1000;
#ifdef HAVE_LOCALTIME_R
   struct tm currentTM;
   localtime_r(&t, &currentTM);
   pCurrentTM = &currentTM;
#else
   pCurrentTM = localtime(&t);
#endif
   strftime(date, sizeof(date), "%x %X", pCurrentTM);
   CopyString(date, m_lastContact, 64);
}

/**
 * MQTT Mesage handler
 */
void MqttMessageHandler(const char *payload, char *topic)
{
   json_error_t error;
   json_t *root = json_loads(payload, 0, &error);

   json_t *tmp = json_object_get(root, "appargs");
   if (json_is_string(tmp))
   {
      TCHAR guid[38];
      CopyString(json_string_value(tmp), guid, 38);
      LoraDeviceData *data = FindDevice(uuid::parse(guid));

      if (data != NULL)
      {
         MacAddress rxDevAddr;
         MacAddress rxDevEui;

         tmp = json_object_get(root, "deveui");
         if (json_is_string(tmp))
            rxDevEui = MacAddress::parse(json_string_value(tmp));
         tmp = json_object_get(root, "devaddr");
         if (json_is_string(tmp))
            rxDevAddr = MacAddress::parse(json_string_value(tmp));

         if (data->getDevAddr().equals(rxDevAddr) || data->getDevEui().equals(rxDevEui))
         {
            if (data->getDevAddr().length() == 0 && rxDevAddr.length() != 0)
               data->setDevAddr(rxDevAddr);

            tmp = json_object_get(root, "data");
            if (json_is_string(tmp))
               data->setPayload(json_string_value(tmp));

            tmp = json_object_get(root, "fcnt");
            if (json_is_integer(tmp))
               data->setFcnt(json_integer_value(tmp));

            tmp = json_object_get(root, "port");
            if (json_is_integer(tmp))
               data->setPort(json_integer_value(tmp));

            tmp = json_object_get(root, "rxq");
            if (json_is_object(tmp))
            {
               json_t *tmp2 = json_object_get(tmp, "datr");
               if (json_is_string(tmp2))
                  data->setDataRate(json_string_value(tmp2));

               tmp2 = json_object_get(tmp, "freq");
               if (json_is_real(tmp2))
                  data->setFreq(json_real_value(tmp2));

               tmp2 = json_object_get(tmp, "lsnr");
               if (json_is_real(tmp2))
                  data->setSnr(json_real_value(tmp2));

               tmp2 = json_object_get(tmp, "rssi");
               if (json_is_real(tmp2))
                  data->setRssi(json_integer_value(tmp2));

               free(tmp2);
            }
            else
               nxlog_debug(6, _T("LoraWAN Module:[MqttMessageHandler] No RX quality data received..."));

            NXMBDispatcher *dispatcher = NXMBDispatcher::getInstance();
            if (!dispatcher->call(_T("NOTIFY_DECODERS"), data, NULL))
               nxlog_debug(6, _T("LoraWAN Module:[MqttMessageHandler] Call to NXMBDispacher failed..."));

            data->setLastContact();
         }
         else
            nxlog_debug(6, _T("LoraWAN Module:[MqttMessageHandler] Neither the devAddr nor the devEUI returned a match..."));
      }
   }
   else
      nxlog_debug(6, _T("LoraWAN Module:[MqttMessageHandler] No GUID found due to missing \"appargs\" field in received JSON"));

   free(root);
   free(tmp);
}

/**
 * Handler for communication parameters
 */
LONG H_Communication(const TCHAR *param, const TCHAR *arg, TCHAR *value, AbstractCommSession *session)
{
   TCHAR guid[38];
   if (!AgentGetParameterArg(param, 1, guid, 38))
      return SYSINFO_RC_ERROR;

   LoraDeviceData *data = FindDevice(uuid::parse(guid));
   if (data == NULL)
      return SYSINFO_RC_ERROR;

   switch(*arg)
   {
      case 'A':
         ret_string(value, data->getDevAddr().toString(MAC_ADDR_FLAT_STRING));
         break;
      case 'C':
         ret_string(value, data->getLastContact());
         break;
      case 'D':
         ret_string(value, data->getDataRate());
         break;
      case 'F':
         ret_uint(value, data->getFreq());
         break;
      case 'M':
         ret_uint(value, data->getFcnt());
         break;
      case 'R':
         ret_int(value, data->getRssi());
         break;
      case 'S':
         ret_int(value, data->getSnr());
         break;
      default:
         return SYSINFO_RC_UNSUPPORTED;
   }

   return SYSINFO_RC_SUCCESS;
}
