/*
** NetXMS - Network Management System
** Generic driver for Cisco devices
** Copyright (C) 2003-2018 Victor Kirhenshtein
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
** File: cisco.cpp
**
**/

#include "cisco.h"

/**
 * Get driver version
 */
const TCHAR *CiscoDeviceDriver::getVersion()
{
   return NETXMS_BUILD_TAG;
}

/**
 * Handler for VLAN enumeration on Cisco device
 */
static UINT32 HandlerVlanList(SNMP_Variable *var, SNMP_Transport *transport, void *arg)
{
   VlanList *vlanList = (VlanList *)arg;

	VlanInfo *vlan = new VlanInfo(var->getName().getElement(var->getName().length() - 1), VLAN_PRM_IFINDEX);

	TCHAR buffer[256];
	vlan->setName(var->getValueAsString(buffer, 256));

	vlanList->add(vlan);
   return SNMP_ERR_SUCCESS;
}

/**
 * Parse VLAN membership bit map
 *
 * @param vlanList VLAN list
 * @param ifIndex interface index for current interface
 * @param map VLAN membership map
 * @param offset VLAN ID offset from 0
 */
static void ParseVlanMap(VlanList *vlanList, UINT32 ifIndex, BYTE *map, int offset)
{
	// VLAN map description from Cisco MIB:
	// ======================================
	// A string of octets containing one bit per VLAN in the
	// management domain on this trunk port.  The first octet
	// corresponds to VLANs with VlanIndex values of 0 through 7;
	// the second octet to VLANs 8 through 15; etc.  The most
	// significant bit of each octet corresponds to the lowest
	// value VlanIndex in that octet.  If the bit corresponding to
	// a VLAN is set to '1', then the local system is enabled for
	// sending and receiving frames on that VLAN; if the bit is set
	// to '0', then the system is disabled from sending and
	// receiving frames on that VLAN.

	int vlanId = offset;
	for(int i = 0; i < 128; i++)
	{
		BYTE mask = 0x80;
		while(mask > 0)
		{
			if (map[i] & mask)
			{
				vlanList->addMemberPort(vlanId, ifIndex);
			}
			mask >>= 1;
			vlanId++;
		}
	}
}

/**
 * Handler for trunk port enumeration on Cisco device
 */
static UINT32 HandlerTrunkPorts(SNMP_Variable *var, SNMP_Transport *transport, void *arg)
{
   VlanList *vlanList = (VlanList *)arg;
   size_t nameLen = var->getName().length();
   UINT32 ifIndex = var->getName().getElement(nameLen - 1);

   // Check if port is acting as trunk
   UINT32 oidName[256], value;
   memcpy(oidName, var->getName().value(), nameLen * sizeof(UINT32));
   oidName[nameLen - 2] = 14;	// .1.3.6.1.4.1.9.9.46.1.6.1.1.14
	if (SnmpGetEx(transport, NULL, oidName, nameLen, &value, sizeof(UINT32), 0, NULL) != SNMP_ERR_SUCCESS)
	   return SNMP_ERR_SUCCESS;	// Cannot get trunk state, ignore port
	if (value != 1)
	   return SNMP_ERR_SUCCESS;	// Not a trunk port, ignore

	// Native VLAN
	int vlanId = var->getValueAsInt();
	if (vlanId != 0)
		vlanList->addMemberPort(vlanId, ifIndex);

	// VLAN map for VLAN IDs 0..1023
   oidName[nameLen - 2] = 4;	// .1.3.6.1.4.1.9.9.46.1.6.1.1.4
	BYTE map[128];
	memset(map, 0, 128);
	if (SnmpGetEx(transport, NULL, oidName, nameLen, map, 128, SG_RAW_RESULT, NULL) == SNMP_ERR_SUCCESS)
		ParseVlanMap(vlanList, ifIndex, map, 0);

	// VLAN map for VLAN IDs 1024..2047
   oidName[nameLen - 2] = 17;	// .1.3.6.1.4.1.9.9.46.1.6.1.1.17
	memset(map, 0, 128);
	if (SnmpGetEx(transport, NULL, oidName, nameLen, map, 128, SG_RAW_RESULT, NULL) == SNMP_ERR_SUCCESS)
		ParseVlanMap(vlanList, ifIndex, map, 1024);

	// VLAN map for VLAN IDs 2048..3071
   oidName[nameLen - 2] = 18;	// .1.3.6.1.4.1.9.9.46.1.6.1.1.18
	memset(map, 0, 128);
	if (SnmpGetEx(transport, NULL, oidName, nameLen, map, 128, SG_RAW_RESULT, NULL) == SNMP_ERR_SUCCESS)
		ParseVlanMap(vlanList, ifIndex, map, 2048);

	// VLAN map for VLAN IDs 3072..4095
   oidName[nameLen - 2] = 19;	// .1.3.6.1.4.1.9.9.46.1.6.1.1.19
	memset(map, 0, 128);
	if (SnmpGetEx(transport, NULL, oidName, nameLen, map, 128, SG_RAW_RESULT, NULL) == SNMP_ERR_SUCCESS)
		ParseVlanMap(vlanList, ifIndex, map, 3072);

   return SNMP_ERR_SUCCESS;
}


/**
 * Handler for access port enumeration on Cisco device
 */
static UINT32 HandlerAccessPorts(SNMP_Variable *var, SNMP_Transport *transport, void *arg)
{
   VlanList *vlanList = (VlanList *)arg;
   size_t nameLen = var->getName().length();
   UINT32 ifIndex = var->getName().getElement(nameLen - 1);

   UINT32 oidName[256];
   memcpy(oidName, var->getName().value(), nameLen * sizeof(UINT32));

	// Entry type: 3=multi-vlan
	if (var->getValueAsInt() == 3)
	{
		BYTE map[128];

		oidName[nameLen - 2] = 4;	// .1.3.6.1.4.1.9.9.68.1.2.2.1.4
		memset(map, 0, 128);
		if (SnmpGetEx(transport, NULL, oidName, nameLen, map, 128, SG_RAW_RESULT, NULL) == SNMP_ERR_SUCCESS)
			ParseVlanMap(vlanList, ifIndex, map, 0);

		// VLAN map for VLAN IDs 1024..2047
		oidName[nameLen - 2] = 5;	// .1.3.6.1.4.1.9.9.68.1.2.2.1.5
		memset(map, 0, 128);
		if (SnmpGetEx(transport, NULL, oidName, nameLen, map, 128, SG_RAW_RESULT, NULL) == SNMP_ERR_SUCCESS)
			ParseVlanMap(vlanList, ifIndex, map, 1024);

		// VLAN map for VLAN IDs 2048..3071
		oidName[nameLen - 2] = 6;	// .1.3.6.1.4.1.9.9.68.1.2.2.1.6
		memset(map, 0, 128);
		if (SnmpGetEx(transport, NULL, oidName, nameLen, map, 128, SG_RAW_RESULT, NULL) == SNMP_ERR_SUCCESS)
			ParseVlanMap(vlanList, ifIndex, map, 2048);

		// VLAN map for VLAN IDs 3072..4095
		oidName[nameLen - 2] = 7;	// .1.3.6.1.4.1.9.9.68.1.2.2.1.7
		memset(map, 0, 128);
		if (SnmpGetEx(transport, NULL, oidName, nameLen, map, 128, SG_RAW_RESULT, NULL) == SNMP_ERR_SUCCESS)
			ParseVlanMap(vlanList, ifIndex, map, 3072);
	}
	else
	{
		// Port is in just one VLAN, it's ID must be in vmVlan
	   oidName[nameLen - 2] = 2;	// .1.3.6.1.4.1.9.9.68.1.2.2.1.2
		UINT32 vlanId = 0;
		if (SnmpGetEx(transport, NULL, oidName, nameLen, &vlanId, sizeof(UINT32), 0, NULL) == SNMP_ERR_SUCCESS)
		{
			if (vlanId != 0)
				vlanList->addMemberPort((int)vlanId, ifIndex);
		}
	}

	return SNMP_ERR_SUCCESS;
}

/**
 * Get VLANs
 */
VlanList *CiscoDeviceDriver::getVlans(SNMP_Transport *snmp, StringMap *attributes, DriverData *driverData)
{
	VlanList *list = new VlanList();

	// Vlan list
	if (SnmpWalk(snmp, _T(".1.3.6.1.4.1.9.9.46.1.3.1.1.4"), HandlerVlanList, list) != SNMP_ERR_SUCCESS)
		goto failure;

	// Trunk ports
	if (SnmpWalk(snmp, _T(".1.3.6.1.4.1.9.9.46.1.6.1.1.5"), HandlerTrunkPorts, list) != SNMP_ERR_SUCCESS)
		goto failure;

	// Access ports
	if (SnmpWalk(snmp, _T(".1.3.6.1.4.1.9.9.68.1.2.2.1.1"), HandlerAccessPorts, list) != SNMP_ERR_SUCCESS)
		goto failure;

	return list;

failure:
	delete list;
	return NULL;
}

/**
 * Returns true if per-VLAN FDB supported by device (accessible using community@vlan_id).
 * Default implementation always return false;
 *
 * @return true if per-VLAN FDB supported by device
 */
bool CiscoDeviceDriver::isPerVlanFdbSupported()
{
	return true;
}

/**
 * Driver module entry point
 */
NDD_BEGIN_DRIVER_LIST
NDD_DRIVER(CiscoWLC)
NDD_DRIVER(CatalystDriver)
NDD_DRIVER(Cat2900Driver)
NDD_DRIVER(CiscoEswDriver)
NDD_DRIVER(CiscoSbDriver)
NDD_DRIVER(GenericCiscoDriver)
NDD_END_DRIVER_LIST
DECLARE_NDD_MODULE_ENTRY_POINT

/**
 * DLL entry point
 */
#ifdef _WIN32

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
		DisableThreadLibraryCalls(hInstance);
	return TRUE;
}

#endif
