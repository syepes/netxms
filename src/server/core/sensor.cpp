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
	memset(&m_macAddress, 0, sizeof(BYTE)*MAC_ADDR_LENGTH);
	m_deviceClass = SENSOR_CLASS_UNKNOWN;
	m_vendor = _tcsdup(_T(""));
	m_commProtocol = SENSOR_PROTO_UNKNOWN;
	m_xmlConfig = _tcsdup(_T(""));
	m_serialNumber = _tcsdup(_T(""));
	m_deviceAddress = _tcsdup(_T(""));
	m_metaType = _tcsdup(_T(""));
	m_description = _tcsdup(_T(""));
	m_lastConnectionTime = 0;
	m_frameCount = 0; //zero when no info
   m_signalStreight = 1; //+1 when no information(cannot be +)
   m_signalNoice = 0x7FFFFFFF; //*10 from origin number
   m_frequency = 0; //*10 from origin number
}

/**
 * Constructor with all fields for Sensor class
 */
Sensor::Sensor(TCHAR * name, UINT32 flags, BYTE *macAddress, UINT32 deviceClass, TCHAR *vendor,
               UINT32 commProtocol, TCHAR *xmlConfig, TCHAR *serialNumber, TCHAR *deviceAddress,
               TCHAR *metaType, TCHAR *description, UINT32 proxyNode) : DataCollectionTarget(name)
{
   m_flags = flags;
   memcpy(m_macAddress, macAddress, MAC_ADDR_LENGTH);
	m_deviceClass = deviceClass;
	m_vendor = vendor;
	m_commProtocol = commProtocol;
	m_xmlConfig = xmlConfig;
	m_serialNumber = serialNumber;
	m_deviceAddress = deviceAddress;
	m_metaType = metaType;
	m_description = description;
	m_proxyNodeId = proxyNode;
   m_lastConnectionTime = 0;
	m_frameCount = 0; //zero when no info
   m_signalStreight = 1; //+1 when no information(cannot be +)
   m_signalNoice = 0x7FFFFFFF; //*10 from origin number
   m_frequency = 0; //*10 from origin number
}

/**
 * Sensor class destructor
 */
Sensor::~Sensor()
{
   free(m_vendor);
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
      DbgPrintf(2, _T("Cannot load common properties for mobile device object %d"), id);
      return false;
   }

	TCHAR query[256];
	_sntprintf(query, 256, _T("SELECT flags,mac_address,device_class,vendor,communication_protocol,xml_config,serial_number,device_address,")
                          _T("meta_type,description,last_connection_time,frame_count,signal_streight,signal_noice,frequency,proxy_node FROM sensor WHERE id=%d"), (int)m_id);
	DB_RESULT hResult = DBSelect(hdb, query);
	if (hResult == NULL)
		return false;

   m_flags = DBGetFieldULong(hResult, 0, 0);
   DBGetFieldByteArray2(hResult, 0, 1, m_macAddress, MAC_ADDR_LENGTH, 0);
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
   m_signalStreight = DBGetFieldLong(hResult, 0, 12);
   m_signalNoice = DBGetFieldLong(hResult, 0, 13);
   m_frequency = DBGetFieldLong(hResult, 0, 14);
   m_proxyNodeId = DBGetFieldLong(hResult, 0, 15);

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

   saveCommonProperties(hdb);

   BOOL bResult;
	DB_STATEMENT hStmt;
   if (IsDatabaseRecordExist(hdb, _T("sensor"), _T("id"), m_id))
		hStmt = DBPrepare(hdb, _T("UPDATE sensor SET flags=?,mac_address=?,device_class=?,vendor=?,communication_protocol=?,xml_config=?,serial_number=?,device_address=?,meta_type=?,description=?,last_connection_time=?,frame_count=?,signal_streight=?,signal_noice=?,frequency=?,proxy_node=? WHERE id=?"));
	else
		hStmt = DBPrepare(hdb, _T("INSERT INTO sensor (flags,mac_address,device_class,vendor,communication_protocol,xml_config,serial_number,device_address,meta_type,description,last_connection_time,frame_count,signal_streight,signal_noice,frequency,proxy_node,id) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"));
	if (hStmt != NULL)
	{
		DBBind(hStmt, 1, DB_SQLTYPE_INTEGER, m_flags);
		TCHAR macStr[16];
		DBBind(hStmt, 2, DB_SQLTYPE_VARCHAR, BinToStr(m_macAddress, MAC_ADDR_LENGTH, macStr), DB_BIND_STATIC);
		DBBind(hStmt, 3, DB_SQLTYPE_INTEGER, (INT32)m_deviceClass);
		DBBind(hStmt, 4, DB_SQLTYPE_VARCHAR, CHECK_NULL_EX(m_vendor), DB_BIND_STATIC);
		DBBind(hStmt, 5, DB_SQLTYPE_INTEGER, (INT32)m_commProtocol);
		DBBind(hStmt, 6, DB_SQLTYPE_VARCHAR, CHECK_NULL_EX(m_xmlConfig), DB_BIND_STATIC);
		DBBind(hStmt, 7, DB_SQLTYPE_VARCHAR, CHECK_NULL_EX(m_serialNumber), DB_BIND_STATIC);
		DBBind(hStmt, 8, DB_SQLTYPE_VARCHAR, CHECK_NULL_EX(m_deviceAddress), DB_BIND_STATIC);
		DBBind(hStmt, 9, DB_SQLTYPE_VARCHAR, CHECK_NULL_EX(m_metaType), DB_BIND_STATIC);
		DBBind(hStmt, 10, DB_SQLTYPE_VARCHAR, CHECK_NULL_EX(m_description), DB_BIND_STATIC);
		DBBind(hStmt, 11, DB_SQLTYPE_INTEGER, (UINT32)m_lastConnectionTime);
		DBBind(hStmt, 12, DB_SQLTYPE_INTEGER, m_frameCount);
		DBBind(hStmt, 13, DB_SQLTYPE_INTEGER, m_signalStreight);
		DBBind(hStmt, 14, DB_SQLTYPE_INTEGER, m_signalNoice);
		DBBind(hStmt, 15, DB_SQLTYPE_INTEGER, m_frequency);
		DBBind(hStmt, 16, DB_SQLTYPE_INTEGER, m_proxyNodeId);
		DBBind(hStmt, 17, DB_SQLTYPE_INTEGER, m_id);
		bResult = DBExecute(hStmt);

		DBFreeStatement(hStmt);
	}
	else
	{
		bResult = FALSE;
	}

   // Save data collection items
   if (bResult)
   {
		lockDciAccess(false);
      for(int i = 0; i < m_dcObjects->size(); i++)
         m_dcObjects->get(i)->saveToDatabase(hdb);
		unlockDciAccess();
   }

   // Save access list
   saveACLToDB(hdb);

   // Clear modifications flag and unlock object
	if (bResult)
		m_isModified = false;
   unlockProperties();

   return bResult;
}

/**
 * Delete from database
 */
bool Sensor::deleteFromDatabase(DB_HANDLE hdb)
{
   bool success = DataCollectionTarget::deleteFromDatabase(hdb);
   if (success)
      success = executeQueryOnObject(hdb, _T("DELETE FROM sensor WHERE id=?"));
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
   char macAddrText[64];
   json_object_set_new(root, "macAddr", json_string_a(BinToStrA(m_macAddress, sizeof(m_macAddress), macAddrText)));
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
	msg->setField(VID_MAC_ADDR, m_macAddress, MAC_ADDR_LENGTH);
   msg->setField(VID_DEVICE_CLASS, m_deviceClass);
	msg->setField(VID_VENDOR, CHECK_NULL_EX(m_vendor));
   msg->setField(VID_COMM_PROTOCOL, m_commProtocol);
	msg->setField(VID_XML_CONFIG, CHECK_NULL_EX(m_xmlConfig));
	msg->setField(VID_SERIAL_NUMBER, CHECK_NULL_EX(m_serialNumber));
	msg->setField(VID_DEVICE_ADDRESS, CHECK_NULL_EX(m_deviceAddress));
	msg->setField(VID_META_TYPE, CHECK_NULL_EX(m_metaType));
	msg->setField(VID_DESCRIPTION, CHECK_NULL_EX(m_description));
	msg->setFieldFromTime(VID_LAST_CONN_TIME, m_lastConnectionTime);
	msg->setField(VID_FRAME_COUNT, m_frameCount);
	msg->setField(VID_SIGNAL_STREIGHT, m_signalStreight);
	msg->setField(VID_SIGNAL_NOICE, m_signalNoice);
	msg->setField(VID_FREQUENCY, m_frequency);
	msg->setField(VID_SENSOR_PROXY, m_proxyNodeId);
}

UINT32 Sensor::modifyFromMessageInternal(NXCPMessage *request)
{
   if (request->isFieldExist(VID_MAC_ADDR))
      request->getFieldAsBinary(VID_MAC_ADDR, m_macAddress, MAC_ADDR_LENGTH);
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

   return DataCollectionTarget::modifyFromMessageInternal(request);
}

