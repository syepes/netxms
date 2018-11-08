/*
** NetXMS Asterisk subagent
** Copyright (C) 2004-2018 Victor Kirhenshtein
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
** File: main.cpp
**
**/

#include "asterisk.h"

/**
 * Configured systems
 */
static ObjectArray<AsteriskSystem> s_systems(16, 16, true);
static StringObjectMap<AsteriskSystem> s_indexByName(false);

/**
 * Handler for Asterisk.AMI.Status parameter
 */
static LONG H_AMIStatus(const TCHAR *param, const TCHAR *arg, TCHAR *value, AbstractCommSession *session)
{
   GET_ASTERISK_SYSTEM;
   ret_int(value, sys->isAmiSessionReady() ? 1 : 0);
   return SYSINFO_RC_SUCCESS;
}

/**
 * Handler for Asterisk.AMI.Version parameter
 */
static LONG H_AMIVersion(const TCHAR *param, const TCHAR *arg, TCHAR *value, AbstractCommSession *session)
{
   GET_ASTERISK_SYSTEM;
   return sys->readSingleTag("CoreSettings", "AMIversion", value);
}

/**
 * Handler for Asterisk.Version parameter
 */
static LONG H_AsteriskVersion(const TCHAR *param, const TCHAR *arg, TCHAR *value, AbstractCommSession *session)
{
   GET_ASTERISK_SYSTEM;
   return sys->readSingleTag("CoreSettings", "AsteriskVersion", value);
}

/**
 * Handler for Asterisk.Systems list
 */
static LONG H_SystemList(const TCHAR *param, const TCHAR *arg, StringList *value, AbstractCommSession *session)
{
   for(int i = 0; i < s_systems.size(); i++)
      value->add(s_systems.get(i)->getName());
   return SYSINFO_RC_SUCCESS;
}

/**
 * Subagent initialization
 */
static bool SubagentInit(Config *config)
{
   ObjectArray<ConfigEntry> *systems = config->getSubEntries(_T("/Asterisk/Systems"), _T("*"));
   if (systems != NULL)
   {
      for(int i = 0; i < systems->size(); i++)
      {
         AsteriskSystem *s = AsteriskSystem::createFromConfig(systems->get(i));
         if (s != NULL)
         {
            s_systems.add(s);
            s_indexByName.set(s->getName(), s);
            nxlog_debug_tag(DEBUG_TAG, 1, _T("Added Asterisk system %s"), s->getName());
         }
         else
         {
            AgentWriteLog(NXLOG_WARNING, _T("Asterisk: cannot add system %s definition from config"), systems->get(i)->getName());
         }
      }
      delete systems;
   }

   for(int i = 0; i < s_systems.size(); i++)
   {
      s_systems.get(i)->start();
   }
	return true;
}

/**
 * Shutdown subagent
 */
static void SubagentShutdown()
{
   for(int i = 0; i < s_systems.size(); i++)
   {
      s_systems.get(i)->stop();
   }
   nxlog_debug_tag(DEBUG_TAG, 4, _T("All AMI connectors stopped"));
}

/**
 * Parameters
 */
static NETXMS_SUBAGENT_PARAM m_parameters[] =
{
	{ _T("Asterisk.AMI.Status(*)"), H_AMIStatus, NULL, DCI_DT_INT, _T("Asterisk system {instance}: AMI connection status") },
   { _T("Asterisk.AMI.Version(*)"), H_AMIVersion, NULL, DCI_DT_STRING, _T("Asterisk system {instance}: AMI version") },
   { _T("Asterisk.Version(*)"), H_AsteriskVersion, NULL, DCI_DT_STRING, _T("Asterisk system {instance}: version") }
};

/**
 * Lists
 */
static NETXMS_SUBAGENT_LIST s_lists[] =
{
	{ _T("Asterisk.Systems"), H_SystemList, NULL }
};

/**
 * Subagent information
 */
static NETXMS_SUBAGENT_INFO m_info =
{
	NETXMS_SUBAGENT_INFO_MAGIC,
	_T("ASTERISK"), NETXMS_BUILD_TAG,
	SubagentInit, SubagentShutdown, NULL, NULL,
	sizeof(m_parameters) / sizeof(NETXMS_SUBAGENT_PARAM),
	m_parameters,
	sizeof(s_lists) / sizeof(NETXMS_SUBAGENT_LIST),
	s_lists,
	0, NULL,	// tables
   0, NULL,	// actions
	0, NULL	// push parameters
};

/**
 * Entry point for NetXMS agent
 */
DECLARE_SUBAGENT_ENTRY_POINT(PING)
{
	*ppInfo = &m_info;
	return true;
}

#ifdef _WIN32

/**
 * DLL entry point
 */
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
		DisableThreadLibraryCalls(hInstance);
	return TRUE;
}

#endif
