/*
** NetXMS - Network Management System
** Driver for Cisco 4400 (former Airespace) wireless switches
** Copyright (C) 2013-2016 Raden Solutions
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** File: ciscowlc.cpp

1.3.6.1.4.1.9.9.513.1.1.1.1.5 - string with withe AP name
1.3.6.1.4.1.14179.2.2.1.1.6 - integer string with the status
1.3.6.1.4.1.14179.2.2.1.1.4 - string with the access points location
1.3.6.1.4.1.14179.2.2.1.1.19 - IP address of the access point
1.3.6.1.4.1.14179.2.2.1.1.22 - integer representing the model of the access point
1.3.6.1.4.1.14179.2.2.16.1.2 - Interference on the wireless network

**/

#include "cisco.h"

#define DEBUG_TAG _T("ndd.ciscowlc")

/**
 * Get driver name
 */
const TCHAR *CiscoWLC::getName()
{
   return _T("CISCO-WLC");
}

/**
 * Check if given device can be potentially supported by driver
 *
 * @param oid Device OID
 */
int CiscoWLC::isPotentialDevice(const TCHAR *oid)
{
   nxlog_debug_tag(DEBUG_TAG, 4, _T("isPotentialDevice: OID: %lu") INT64_FMT, oid);
   return (_tcsncmp(oid, _T(".1.3.6.1.4.1.9.1."), 17) == 0) ? 255 : 0;
}

/**
 * Check if given device is supported by driver
 *
 * @param snmp SNMP transport
 * @param oid Device OID
 */
bool CiscoWLC::isDeviceSupported(SNMP_Transport *snmp, const TCHAR *oid)
{
   // read agentInventoryMachineModel
   TCHAR buffer[1024];
   if (SnmpGetEx(snmp, _T(".1.3.6.1.4.1.14179.1.1.1.3.0"), NULL, 0, buffer, sizeof(buffer), 0, NULL) != SNMP_ERR_SUCCESS)
      return false;

   nxlog_debug_tag(DEBUG_TAG, 3, _T("isDeviceSupported: Model: %s") INT64_FMT, buffer);

   // model must start with AIR-CT55, like AIR-CT5508-K90
   return _tcsncmp(buffer, _T("AIR-CT55"), 8) == 0;
}

/**
 * Get cluster mode for device (standalone / active / standby)
 *
 * @param snmp SNMP transport
 * @param attributes Node custom attributes
 * @param driverData optional pointer to user data
 */
int CiscoWLC::getClusterMode(SNMP_Transport *snmp, StringMap *attributes, DriverData *driverData)
{
   /* TODO: check if other cluster modes possible */
   return CLUSTER_MODE_STANDALONE;
}

/*
 * Check switch for wireless capabilities
 *
 * @param snmp SNMP transport
 * @param attributes Node custom attributes
 * @param driverData optional pointer to user data
 */
bool CiscoWLC::isWirelessController(SNMP_Transport *snmp, StringMap *attributes, DriverData *driverData)
{
   return true;
}

/**
 * Handler for access point enumeration
 */
static UINT32 HandlerAccessPointList(SNMP_Variable *var, SNMP_Transport *snmp, void *arg)
{
   ObjectArray<AccessPointInfo> *apList = (ObjectArray<AccessPointInfo> *)arg;

   const SNMP_ObjectId& name = var->getName();
   size_t nameLen = name.length();
   UINT32 oid[MAX_OID_LEN];
   memcpy(oid, name.value(), nameLen * sizeof(UINT32));

   //nxlog_debug_tag(DEBUG_TAG, 2, _T("HandlerAccessPointList: oid: %lu / %s") INT64_FMT, oid, (const TCHAR *)name.toString());

   SNMP_PDU *request = new SNMP_PDU(SNMP_GET_REQUEST, SnmpNewRequestId(), snmp->getSnmpVersion());

   oid[11] = 1;   // bsnAPDot3MacAddress
   request->bindVariable(new SNMP_Variable(oid, nameLen));

   oid[11] = 19;  // bsnApIpAddress
   request->bindVariable(new SNMP_Variable(oid, nameLen));

   oid[11] = 6;   // bsnAPOperationStatus
   request->bindVariable(new SNMP_Variable(oid, nameLen));

   oid[11] = 3;   // bsnAPName
   request->bindVariable(new SNMP_Variable(oid, nameLen));

   oid[11] = 16;  // bsnAPModel
   request->bindVariable(new SNMP_Variable(oid, nameLen));

   oid[11] = 17;  // bsnAPSerialNumber
   request->bindVariable(new SNMP_Variable(oid, nameLen));

   oid[9] = 2;    // bsnAPIfTable
   oid[10] = 1;   // bsnAPIfEntry
   oid[11] = 4;   // bsnAPIfPhyChannelNumber

   nameLen++;
   oid[nameLen - 1] = 0;   // first radio
   request->bindVariable(new SNMP_Variable(oid, nameLen));

   oid[nameLen - 1] = 1;   // second radio
   request->bindVariable(new SNMP_Variable(oid, nameLen));

   SNMP_PDU *response;
   UINT32 rcc = snmp->doRequest(request, &response, SnmpGetDefaultTimeout(), 3);
	delete request;
   if (rcc == SNMP_ERR_SUCCESS)
   {
      if (response->getNumVariables() == 8)
      {
         BYTE macAddr[16];
         memset(macAddr, 0, sizeof(macAddr));
         var->getRawValue(macAddr, sizeof(macAddr));

         nxlog_debug_tag(DEBUG_TAG, 2, _T("HandlerAccessPointList: name: %s / mac: %s / mac: %u.%u.%u.%u.%u.%u"), (const TCHAR *)name.toString(), (const TCHAR *)var->getValueAsMACAddr().toString(), macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5] );

         TCHAR ipAddr[32], name[MAX_OBJECT_NAME], model[MAX_OBJECT_NAME], serial[MAX_OBJECT_NAME];
         AccessPointInfo *ap =
            new AccessPointInfo(
               0,
               macAddr,
               ntohl(_t_inet_addr(response->getVariable(1)->getValueAsString(ipAddr, 32))),
               (response->getVariable(2)->getValueAsInt() == 1) ? AP_ADOPTED : AP_UNADOPTED,
               response->getVariable(3)->getValueAsString(name, MAX_OBJECT_NAME),
               _T("Cisco"),   // vendor
               response->getVariable(4)->getValueAsString(model, MAX_OBJECT_NAME),
               response->getVariable(5)->getValueAsString(serial, MAX_OBJECT_NAME));

         RadioInterfaceInfo radio;
         memset(&radio, 0, sizeof(RadioInterfaceInfo));
         _tcscpy(radio.name, _T("slot0"));
         radio.index = 0;
         response->getVariable(0)->getRawValue(radio.macAddr, MAC_ADDR_LENGTH);
         radio.channel = response->getVariable(6)->getValueAsInt();
         ap->addRadioInterface(&radio);


         if ((response->getVariable(7)->getType() != ASN_NO_SUCH_INSTANCE) && (response->getVariable(7)->getType() != ASN_NO_SUCH_OBJECT))
         {
            _tcscpy(radio.name, _T("slot1"));
            radio.index = 1;
            radio.channel = response->getVariable(7)->getValueAsInt();
            ap->addRadioInterface(&radio);
         }

         apList->add(ap);
      }
      delete response;
   }

   return SNMP_ERR_SUCCESS;
}

/**
 * Get access points
 *
 * @param snmp SNMP transport
 * @param attributes Node custom attributes
 * @param driverData optional pointer to user data
 */
ObjectArray<AccessPointInfo> *CiscoWLC::getAccessPoints(SNMP_Transport *snmp, StringMap *attributes, DriverData *driverData)
{
   ObjectArray<AccessPointInfo> *apList = new ObjectArray<AccessPointInfo>(0, 16, true);

    // bsnAPEthernetMacAddress
   if (SnmpWalk(snmp, _T(".1.3.6.1.4.1.14179.2.2.1.1.33"), HandlerAccessPointList, apList) != SNMP_ERR_SUCCESS)
   {
      delete apList;
      return NULL;
   }

   return apList;
}


/**
 * Data for AP search
 */
struct AP_SEARCH_DATA
{
   BYTE macAddr[MAC_ADDR_LENGTH];
   bool found;
};

/**
 * Handler for unadopted access point search by MAC
 */
static UINT32 HandlerAccessPointFindUnadopted(SNMP_Variable *var, SNMP_Transport *transport, void *arg)
{
   AP_SEARCH_DATA *data = (AP_SEARCH_DATA *)arg;
   if (!memcmp(var->getValue(), data->macAddr, MAC_ADDR_LENGTH))
   {
      data->found = true;
   }
   // nxlog_debug_tag(DEBUG_TAG, 2, _T("getAccessPointState: mac:%d") INT64_FMT, var->getValue());

   return SNMP_ERR_SUCCESS;
}


/**
 * Get access point state
 *
 * @param snmp SNMP transport
 * @param attributes Node's custom attributes
 * @param driverData driver-specific data previously created in analyzeDevice
 * @param apIndex access point index
 * @param macAdddr access point MAC address
 * @param ipAddr access point IP address
 * @return state of access point or AP_UNKNOWN if it cannot be determined
 */
AccessPointState CiscoWLC::getAccessPointState(SNMP_Transport *snmp, StringMap *attributes, DriverData *driverData,
                                                     UINT32 apIndex, const BYTE *macAddr, const InetAddress& ipAddr)
{
   return AP_UNKNOWN;
   nxlog_debug_tag(DEBUG_TAG, 2, _T("getAccessPointState: apIndex: %d / mac: %u.%u.%u.%u.%u.%u"), apIndex, macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]);
   UINT32 value = 0;
   TCHAR oid[256];
   // _sntprintf(oid, 256, _T(".1.3.6.1.4.1.14179.2.2.1.1.6.0.%u"), ipAddr.getAddressV4());
   _sntprintf(oid, 256, _T(".1.3.6.1.4.1.14179.2.2.1.1.6.%u.%u.%u.%u.%u.%u"), macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]);
   if (SnmpGet(snmp->getSnmpVersion(), snmp, oid, NULL, 0, &value, sizeof(UINT32), 0) != SNMP_ERR_SUCCESS)
   {
      nxlog_debug_tag(DEBUG_TAG, 2, _T("getAccessPointState: NOK"));
   } else {
      nxlog_debug_tag(DEBUG_TAG, 2, _T("getAccessPointState: OK"));
   }

   switch (value)
   {
      case 1:
         return AP_ADOPTED;
      case 2:
         return AP_UNADOPTED;
      case 3:
         return AP_UNKNOWN;
      default:
         return AP_DOWN;
   }
}

/**
 * Handler for mobile units enumeration
 */
static UINT32 HandlerWirelessStationList(SNMP_Variable *var, SNMP_Transport *snmp, void *arg)
{
   ObjectArray<WirelessStationInfo> *wsList = (ObjectArray<WirelessStationInfo> *)arg;

   const SNMP_ObjectId& name = var->getName();
   size_t nameLen = name.length();
   UINT32 oid[MAX_OID_LEN];
   memcpy(oid, name.value(), nameLen * sizeof(UINT32));

   SNMP_PDU *request = new SNMP_PDU(SNMP_GET_REQUEST, SnmpNewRequestId(), snmp->getSnmpVersion());

   oid[11] = 2; // bsnMobileStationIpAddress
   request->bindVariable(new SNMP_Variable(oid, nameLen));

   oid[11] = 4; // bsnMobileStationAPMacAddr
   request->bindVariable(new SNMP_Variable(oid, nameLen));

   oid[11] = 5; // bsnMobileStationAPIfSlotId
   request->bindVariable(new SNMP_Variable(oid, nameLen));

   oid[11] = 7; // bsnMobileStationSsid
   request->bindVariable(new SNMP_Variable(oid, nameLen));

   oid[11] = 29; // bsnMobileStationVlanId
   request->bindVariable(new SNMP_Variable(oid, nameLen));

   SNMP_PDU *response;
   UINT32 rcc = snmp->doRequest(request, &response, SnmpGetDefaultTimeout(), 3);
	delete request;
   if (rcc == SNMP_ERR_SUCCESS)
   {
      if (response->getNumVariables() == 5)
      {
         WirelessStationInfo *info = new WirelessStationInfo;
         memset(info, 0, sizeof(WirelessStationInfo));

         var->getRawValue(info->macAddr, MAC_ADDR_LENGTH);
         TCHAR ipAddr[32];
         info->ipAddr = ntohl(_t_inet_addr(response->getVariable(0)->getValueAsString(ipAddr, 32)));
         response->getVariable(3)->getValueAsString(info->ssid, MAX_OBJECT_NAME);
         info->vlan = response->getVariable(4)->getValueAsInt();
         response->getVariable(1)->getRawValue(info->bssid, MAC_ADDR_LENGTH);
         info->rfIndex = response->getVariable(2)->getValueAsInt();
         info->apMatchPolicy = AP_MATCH_BY_BSSID;

         wsList->add(info);
      }
      delete response;
   }
   return SNMP_ERR_SUCCESS;
}

/**
 * Get registered wireless stations (clients)
 *
 * @param snmp SNMP transport
 * @param attributes Node custom attributes
 * @param driverData optional pointer to user data
 */
ObjectArray<WirelessStationInfo> *CiscoWLC::getWirelessStations(SNMP_Transport *snmp, StringMap *attributes, DriverData *driverData)
{
   ObjectArray<WirelessStationInfo> *wsList = new ObjectArray<WirelessStationInfo>(0, 16, true);

   // bsnMobileStationMacAddress
   if (SnmpWalk(snmp, _T(".1.3.6.1.4.1.14179.2.1.4.1.1"), HandlerWirelessStationList, wsList) != SNMP_ERR_SUCCESS)
   {
      delete wsList;
      wsList = NULL;
   }

   return wsList;
}
