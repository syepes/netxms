/*
** NetXMS - Network Management System
** Copyright (C) 2003-2016 Victor Kirhenshtein
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
** File: sensor.cpp
**/

#include "nxcore.h"

/**
 * Default empty Sensor class constructior
 */
Sensor::Sensor() : DataCollectionTarget()
{
   m_proxyNodeId = 0;
	m_flags = 0;
	m_deviceClass = SENSOR_CLASS_UNKNOWN;
	m_vendor = NULL;
	m_commProtocol = SENSOR_PROTO_UNKNOWN;
	m_xmlRegConfig = NULL;
	m_xmlConfig = NULL;
	m_serialNumber = NULL;
	m_deviceAddress = NULL;
	m_metaType = NULL;
	m_description = NULL;
	m_lastConnectionTime = 0;
	m_frameCount = 0; //zero when no info
   m_signalStrenght = 1; //+1 when no information(cannot be +)
   m_signalNoise = MAX_INT32; //*10 from origin number
   m_frequency = 0; //*10 from origin number
}

/**
 * Constructor with all fields for Sensor class
 */
Sensor::Sensor(TCHAR *name, UINT32 flags, MacAddress macAddress, UINT32 deviceClass, TCHAR *vendor,
               UINT32 commProtocol, TCHAR *xmlRegConfig, TCHAR *xmlConfig, TCHAR *serialNumber, TCHAR *deviceAddress,
               TCHAR *metaType, TCHAR *description, UINT32 proxyNode) : DataCollectionTarget(name)
{
   m_flags = flags;
   m_macAddress = macAddress;
	m_deviceClass = deviceClass;
	m_vendor = vendor;
	m_commProtocol = commProtocol;
	m_xmlRegConfig = xmlRegConfig;
	m_xmlConfig = xmlConfig;
	m_serialNumber = serialNumber;
	m_deviceAddress = deviceAddress;
	m_metaType = metaType;
	m_description = description;
	m_proxyNodeId = proxyNode;
   m_lastConnectionTime = 0;
	m_frameCount = 0; //zero when no info
   m_signalStrenght = 1; //+1 when no information(cannot be +)
   m_signalNoise = MAX_INT32; //*10 from origin number
   m_frequency = 0; //*10 from origin number
}

Sensor *Sensor::createSensor(TCHAR *name, NXCPMessage *request)
{
   Sensor *sensor = new Sensor(name,
                          request->getFieldAsUInt32(VID_SENSOR_FLAGS),
                          request->getFieldAsMacAddress(VID_MAC_ADDR),
                          request->getFieldAsUInt32(VID_DEVICE_CLASS),
                          request->getFieldAsString(VID_VENDOR),
                          request->getFieldAsUInt32(VID_COMM_PROTOCOL),
                          request->getFieldAsString(VID_XML_REG_CONFIG),
                          request->getFieldAsString(VID_XML_CONFIG),
                          request->getFieldAsString(VID_SERIAL_NUMBER),
                          request->getFieldAsString(VID_DEVICE_ADDRESS),
                          request->getFieldAsString(VID_META_TYPE),
                          request->getFieldAsString(VID_DESCRIPTION),
                          request->getFieldAsUInt32(VID_SENSOR_PROXY));

   switch(request->getFieldAsUInt32(VID_COMM_PROTOCOL))
   {
      case COMM_LORAWAN:
         sensor = registerLoraDevice(sensor);
         break;
   }
   return sensor;
}

/**
 * Register LoRa WAN device
 */
Sensor *Sensor::registerLoraDevice(Sensor *sensor)
{
   Config regConfig;
   char regXml[MAX_CONFIG_VALUE];
   WideCharToMultiByte(CP_UTF8, 0, sensor->getXmlRegConfig(), -1, regXml, MAX_CONFIG_VALUE, NULL, NULL);
   regConfig.loadXmlConfigFromMemory(regXml, MAX_CONFIG_VALUE, NULL, "config", false);

   Config config;
   char xml[MAX_CONFIG_VALUE];
   WideCharToMultiByte(CP_UTF8, 0, sensor->getXmlConfig(), -1, xml, MAX_CONFIG_VALUE, NULL, NULL);
   config.loadXmlConfigFromMemory(xml, MAX_CONFIG_VALUE, NULL, "config", false);

   NetObj *proxy = FindObjectById(sensor->getProxyNodeId(), OBJECT_NODE);
   if(proxy == NULL)
   {
      delete sensor;
      return NULL;
   }

   AgentConnection *conn = ((Node *)proxy)->createAgentConnection();//provide server id
   if(conn == NULL)
      return sensor; //Unprovisoned sensor - will try to provison it on next connect

   NXCPMessage msg(conn->getProtocolVersion());
   msg.setCode(CMD_REGISTER_LORAWAN_SENSOR);
   msg.setId(conn->generateRequestId());
   msg.setField(VID_DEVICE_ADDRESS, sensor->getDeviceAddress());
   msg.setField(VID_MAC_ADDR, sensor->getMacAddress());
   msg.setField(VID_GUID, sensor->getGuid());
   msg.setField(VID_DECODER, config.getValueAsInt(_T("/decoder"), 0));
   msg.setField(VID_REG_TYPE, regConfig.getValueAsInt(_T("/registrationType"), 0));
   if(regConfig.getValueAsInt(_T("/registrationType"), 0) == 0)
   {
      msg.setField(VID_LORA_APP_EUI, regConfig.getValue(_T("/appEUI")));
      msg.setField(VID_LORA_APP_KEY, regConfig.getValue(_T("/appKey")));
   }
   else
   {
      msg.setField(VID_LORA_APP_S_KEY, regConfig.getValue(_T("/appSKey")));
      msg.setField(VID_LORA_NWK_S_KWY, regConfig.getValue(_T("/nwkSKey")));
   }
   NXCPMessage *response = conn->customRequest(&msg);
   if (response != NULL)
   {
      if(response->getFieldAsUInt32(VID_RCC) == RCC_SUCCESS)
         sensor->setProvisoned();

      delete response;
   }


   conn->decRefCount();
   return sensor;
}

//set correct status calculation function
//set correct configuration poll - provision if possible, for lora get device name, get all possible DCI's, try to do provisionning
//set status poll - check if connection is on if not generate alarm, check, that proxy is up and running

/**
 * Sensor class destructor
 */
Sensor::~Sensor()
{
   free(m_vendor);
   free(m_xmlRegConfig);
   free(m_xmlConfig);
   free(m_serialNumber);
   free(m_deviceAddress);
   free(m_metaType);
   free(m_description);
}

/**
 * Load from database SensorDevice class
 */
bool Sensor::loadFromDatabase(DB_HANDLE hdb, UINT32 id)
{
   m_id = id;

   if (!loadCommonProperties(hdb))
   {
      DbgPrintf(2, _T("Cannot load common properties for sensor object %d"), id);
      return false;
   }

	TCHAR query[256];
	_sntprintf(query, 256, _T("SELECT flags,mac_address,device_class,vendor,communication_protocol,xml_config,serial_number,device_address,")
                          _T("meta_type,description,last_connection_time,frame_count,signal_strenght,signal_noise,frequency,proxy_node,xml_reg_config FROM sensors WHERE id=%d"), (int)m_id);
	DB_RESULT hResult = DBSelect(hdb, query);
	if (hResult == NULL)
		return false;

   m_flags = DBGetFieldULong(hResult, 0, 0);
   m_macAddress = DBGetFieldMacAddr(hResult, 0, 1);
	m_deviceClass = DBGetFieldULong(hResult, 0, 2);
	m_vendor = DBGetField(hResult, 0, 3, NULL, 0);
	m_commProtocol = DBGetFieldULong(hResult, 0, 4);
	m_xmlConfig = DBGetField(hResult, 0, 5, NULL, 0);
	m_serialNumber = DBGetField(hResult, 0, 6, NULL, 0);
	m_deviceAddress = DBGetField(hResult, 0, 7, NULL, 0);
	m_metaType = DBGetField(hResult, 0, 8, NULL, 0);
	m_description = DBGetField(hResult, 0, 9, NULL, 0);
   m_lastConnectionTime = DBGetFieldULong(hResult, 0, 10);
   m_frameCount = DBGetFieldULong(hResult, 0, 11);
   m_signalStrenght = DBGetFieldLong(hResult, 0, 12);
   m_signalNoise = DBGetFieldLong(hResult, 0, 13);
   m_frequency = DBGetFieldLong(hResult, 0, 14);
   m_proxyNodeId = DBGetFieldLong(hResult, 0, 15);
   m_xmlRegConfig = DBGetField(hResult, 0, 16, NULL, 0);

   // Load DCI and access list
   loadACLFromDB(hdb);
   loadItemsFromDB(hdb);
   for(int i = 0; i < m_dcObjects->size(); i++)
      if (!m_dcObjects->get(i)->loadThresholdsFromDB(hdb))
         return false;

   return true;
}

/**
 * Save to database Sensor class
 */
BOOL Sensor::saveToDatabase(DB_HANDLE hdb)
{
   lockProperties();

   BOOL success = saveCommonProperties(hdb);

   if(success)
   {
      DB_STATEMENT hStmt;
      bool isNew = !(IsDatabaseRecordExist(hdb, _T("sensors"), _T("id"), m_id));
      if (isNew)
         hStmt = DBPrepare(hdb, _T("INSERT INTO sensors (flags,mac_address,device_class,vendor,communication_protocol,xml_config,serial_number,device_address,meta_type,description,last_connection_time,frame_count,signal_strenght,signal_noise,frequency,proxy_node,id,xml_reg_config) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"));
      else
         hStmt = DBPrepare(hdb, _T("UPDATE sensors SET flags=?,mac_address=?,device_class=?,vendor=?,communication_protocol=?,xml_config=?,serial_number=?,device_address=?,meta_type=?,description=?,last_connection_time=?,frame_count=?,signal_strenght=?,signal_noise=?,frequency=?,proxy_node=?,=? WHERE id=?"));
      if (hStmt != NULL)
      {
         DBBind(hStmt, 1, DB_SQLTYPE_INTEGER, m_flags);
         DBBind(hStmt, 2, DB_SQLTYPE_VARCHAR, m_macAddress);
         DBBind(hStmt, 3, DB_SQLTYPE_INTEGER, (INT32)m_deviceClass);
         DBBind(hStmt, 4, DB_SQLTYPE_VARCHAR, m_vendor, DB_BIND_STATIC);
         DBBind(hStmt, 5, DB_SQLTYPE_INTEGER, (INT32)m_commProtocol);
         DBBind(hStmt, 6, DB_SQLTYPE_VARCHAR, m_xmlConfig, DB_BIND_STATIC);
         DBBind(hStmt, 7, DB_SQLTYPE_VARCHAR, m_serialNumber, DB_BIND_STATIC);
         DBBind(hStmt, 8, DB_SQLTYPE_VARCHAR, m_deviceAddress, DB_BIND_STATIC);
         DBBind(hStmt, 9, DB_SQLTYPE_VARCHAR, m_metaType, DB_BIND_STATIC);
         DBBind(hStmt, 10, DB_SQLTYPE_VARCHAR, m_description, DB_BIND_STATIC);
         DBBind(hStmt, 11, DB_SQLTYPE_INTEGER, (UINT32)m_lastConnectionTime);
         DBBind(hStmt, 12, DB_SQLTYPE_INTEGER, m_frameCount);
         DBBind(hStmt, 13, DB_SQLTYPE_INTEGER, m_signalStrenght);
         DBBind(hStmt, 14, DB_SQLTYPE_INTEGER, m_signalNoise);
         DBBind(hStmt, 15, DB_SQLTYPE_INTEGER, m_frequency);
         DBBind(hStmt, 16, DB_SQLTYPE_INTEGER, m_proxyNodeId);
         DBBind(hStmt, 17, DB_SQLTYPE_INTEGER, m_id);
         if(isNew)
            DBBind(hStmt, 18, DB_SQLTYPE_VARCHAR, m_xmlRegConfig, DB_BIND_STATIC);

         success = DBExecute(hStmt);

         DBFreeStatement(hStmt);
      }
      else
      {
         success = FALSE;
      }
	}

   // Save data collection items
   if (success)
   {
		lockDciAccess(false);
      for(int i = 0; i < m_dcObjects->size(); i++)
         m_dcObjects->get(i)->saveToDatabase(hdb);
		unlockDciAccess();
   }

   // Save access list
   if(success)
      saveACLToDB(hdb);

   // Clear modifications flag and unlock object
	if (success)
		m_isModified = FALSE;
   unlockProperties();

   return success;
}

/**
 * Delete from database
 */
bool Sensor::deleteFromDatabase(DB_HANDLE hdb)
{
   bool success = DataCollectionTarget::deleteFromDatabase(hdb);
   if (success)
      success = executeQueryOnObject(hdb, _T("DELETE FROM sensors WHERE id=?"));
   return success;
}


NXSL_Value *Sensor::createNXSLObject()
{
   return new NXSL_Value(new NXSL_Object(&g_nxslMobileDeviceClass, this));
}

/**
 * Sensor class serialization to json
 */
json_t *Sensor::toJson()
{
   json_t *root = DataCollectionTarget::toJson();
   json_object_set_new(root, "flags", json_integer(m_flags));
   json_object_set_new(root, "macAddr", json_string_t(m_macAddress.toString(MAC_ADDR_FLAT_STRING)));
   json_object_set_new(root, "deviceClass", json_integer(m_deviceClass));
   json_object_set_new(root, "vendor", json_string_t(m_vendor));
   json_object_set_new(root, "commProtocol", json_integer(m_commProtocol));
   json_object_set_new(root, "xmlConfig", json_string_t(m_xmlConfig));
   json_object_set_new(root, "serialNumber", json_string_t(m_serialNumber));
   json_object_set_new(root, "deviceAddress", json_string_t(m_deviceAddress));
   json_object_set_new(root, "metaType", json_string_t(m_metaType));
   json_object_set_new(root, "description", json_string_t(m_description));
   json_object_set_new(root, "proxyNode", json_integer(m_proxyNodeId));
   return root;
}

void Sensor::fillMessageInternal(NXCPMessage *msg)
{
   DataCollectionTarget::fillMessageInternal(msg);
   msg->setField(VID_SENSOR_FLAGS, m_flags);
	msg->setField(VID_MAC_ADDR, m_macAddress);
   msg->setField(VID_DEVICE_CLASS, m_deviceClass);
	msg->setField(VID_VENDOR, CHECK_NULL_EX(m_vendor));
   msg->setField(VID_COMM_PROTOCOL, m_commProtocol);
	msg->setField(VID_XML_CONFIG, CHECK_NULL_EX(m_xmlConfig));
	msg->setField(VID_XML_REG_CONFIG, CHECK_NULL_EX(m_xmlRegConfig));
	msg->setField(VID_SERIAL_NUMBER, CHECK_NULL_EX(m_serialNumber));
	msg->setField(VID_DEVICE_ADDRESS, CHECK_NULL_EX(m_deviceAddress));
	msg->setField(VID_META_TYPE, CHECK_NULL_EX(m_metaType));
	msg->setField(VID_DESCRIPTION, CHECK_NULL_EX(m_description));
	msg->setFieldFromTime(VID_LAST_CONN_TIME, m_lastConnectionTime);
	msg->setField(VID_FRAME_COUNT, m_frameCount);
	msg->setField(VID_SIGNAL_STRENGHT, m_signalStrenght);
	msg->setField(VID_SIGNAL_NOISE, m_signalNoise);
	msg->setField(VID_FREQUENCY, m_frequency);
	msg->setField(VID_SENSOR_PROXY, m_proxyNodeId);
}

UINT32 Sensor::modifyFromMessageInternal(NXCPMessage *request)
{
   if (request->isFieldExist(VID_MAC_ADDR))
      m_macAddress = request->getFieldAsMacAddress(VID_MAC_ADDR);
   if (request->isFieldExist(VID_VENDOR))
   {
      free(m_vendor);
      m_vendor = request->getFieldAsString(VID_VENDOR);
   }
   if (request->isFieldExist(VID_DEVICE_CLASS))
      m_deviceClass = request->getFieldAsUInt32(VID_DEVICE_CLASS);
   if (request->isFieldExist(VID_SERIAL_NUMBER))
   {
      free(m_serialNumber);
      m_serialNumber = request->getFieldAsString(VID_SERIAL_NUMBER);
   }
   if (request->isFieldExist(VID_DEVICE_ADDRESS))
   {
      free(m_deviceAddress);
      m_deviceAddress = request->getFieldAsString(VID_DEVICE_ADDRESS);
   }
   if (request->isFieldExist(VID_META_TYPE))
   {
      free(m_metaType);
      m_metaType = request->getFieldAsString(VID_META_TYPE);
   }
   if (request->isFieldExist(VID_DESCRIPTION))
   {
      free(m_description);
      m_description = request->getFieldAsString(VID_DESCRIPTION);
   }
   if (request->isFieldExist(VID_SENSOR_PROXY))
      m_proxyNodeId = request->getFieldAsUInt32(VID_SENSOR_PROXY);
   if (request->isFieldExist(VID_XML_CONFIG))
   {
      free(m_xmlConfig);
      m_xmlConfig = request->getFieldAsString(VID_XML_CONFIG);
   }

   return DataCollectionTarget::modifyFromMessageInternal(request);
}

