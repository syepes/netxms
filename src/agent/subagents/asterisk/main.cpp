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

/******* Externals *******/
LONG H_ChannelList(const TCHAR *param, const TCHAR *arg, StringList *value, AbstractCommSession *session);
LONG H_ChannelStats(const TCHAR *param, const TCHAR *arg, TCHAR *value, AbstractCommSession *session);
LONG H_ChannelTable(const TCHAR *param, const TCHAR *arg, Table *value, AbstractCommSession *session);
LONG H_EventCounters(const TCHAR *param, const TCHAR *arg, TCHAR *value, AbstractCommSession *session);
LONG H_SIPPeerDetails(const TCHAR *param, const TCHAR *arg, TCHAR *value, AbstractCommSession *session);
LONG H_SIPPeerList(const TCHAR *param, const TCHAR *arg, StringList *value, AbstractCommSession *session);
LONG H_SIPPeerStats(const TCHAR *param, const TCHAR *arg, TCHAR *value, AbstractCommSession *session);
LONG H_SIPPeerTable(const TCHAR *param, const TCHAR *arg, Table *value, AbstractCommSession *session);

/**
 * Configured systems
 */
static ObjectArray<AsteriskSystem> s_systems(16, 16, true);
static StringObjectMap<AsteriskSystem> s_indexByName(false);

/**
 * Get asterisk system by name
 */
AsteriskSystem *GetAsteriskSystemByName(const TCHAR *name)
{
   return s_indexByName.get(name);
}

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
 * Handler for Asterisk.CurrentCalls parameter
 */
static LONG H_CurrentCalls(const TCHAR *param, const TCHAR *arg, TCHAR *value, AbstractCommSession *session)
{
   GET_ASTERISK_SYSTEM;
   return sys->readSingleTag("CoreStatus", "CurrentCalls", value);
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
 * Handler for Asterisk.CommandOutput list
 */
static LONG H_CommandOutput(const TCHAR *param, const TCHAR *arg, StringList *value, AbstractCommSession *session)
{
   GET_ASTERISK_SYSTEM;

   char command[256];
   if (!AgentGetParameterArgA(param, 2, command, 256))
      return SYSINFO_RC_UNSUPPORTED;

   StringList *output = sys->executeCommand(command);
   if (output == NULL)
      return SYSINFO_RC_ERROR;

   value->addAll(output);
   delete output;
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
   { _T("Asterisk.Channels.Active(*)"), H_ChannelStats, _T("A"), DCI_DT_UINT, _T("Asterisk system {instance}: active channels") },
   { _T("Asterisk.Channels.Busy(*)"), H_ChannelStats, _T("B"), DCI_DT_UINT, _T("Asterisk system {instance}: busy channels") },
   { _T("Asterisk.Channels.Dialing(*)"), H_ChannelStats, _T("D"), DCI_DT_UINT, _T("Asterisk system {instance}: dialing channels") },
   { _T("Asterisk.Channels.OffHook(*)"), H_ChannelStats, _T("A"), DCI_DT_UINT, _T("Asterisk system {instance}: off-hook channels") },
   { _T("Asterisk.Channels.Reserved(*)"), H_ChannelStats, _T("R"), DCI_DT_UINT, _T("Asterisk system {instance}: reserved channels") },
   { _T("Asterisk.Channels.Ringing(*)"), H_ChannelStats, _T("r"), DCI_DT_UINT, _T("Asterisk system {instance}: ringing channels") },
   { _T("Asterisk.Channels.Up(*)"), H_ChannelStats, _T("A"), DCI_DT_UINT, _T("Asterisk system {instance}: up channels") },
   { _T("Asterisk.CurrentCalls(*)"), H_CurrentCalls, NULL, DCI_DT_UINT, _T("Asterisk system {instance}: current calls") },
   { _T("Asterisk.Events.CallBarred(*)"), H_EventCounters, _T("B"), DCI_DT_COUNTER64, _T("Asterisk system {instance}: call barred events") },
   { _T("Asterisk.Events.CallRejected(*)"), H_EventCounters, _T("R"), DCI_DT_COUNTER64, _T("Asterisk system {instance}: call rejected events") },
   { _T("Asterisk.Events.ChannelUnavailable(*)"), H_EventCounters, _T("U"), DCI_DT_COUNTER64, _T("Asterisk system {instance}: channel unavailable events") },
   { _T("Asterisk.Events.Congestion(*)"), H_EventCounters, _T("C"), DCI_DT_COUNTER64, _T("Asterisk system {instance}: congestion events") },
   { _T("Asterisk.Events.NoRoute(*)"), H_EventCounters, _T("N"), DCI_DT_COUNTER64, _T("Asterisk system {instance}: no route events") },
   { _T("Asterisk.Events.SubscriberAbsent(*)"), H_EventCounters, _T("A"), DCI_DT_COUNTER64, _T("Asterisk system {instance}: subscriber absent events") },
   { _T("Asterisk.SIP.Peer.Details(*)"), H_SIPPeerDetails, NULL, DCI_DT_STRING, _T("Asterisk system {instance}: SIP peer detailed information") },
   { _T("Asterisk.SIP.Peer.IPAddress(*)"), H_SIPPeerDetails, _T("I"), DCI_DT_STRING, _T("Asterisk system {instance}: SIP peer IP address") },
   { _T("Asterisk.SIP.Peer.Status(*)"), H_SIPPeerDetails, _T("S"), DCI_DT_STRING, _T("Asterisk system {instance}: SIP peer status") },
   { _T("Asterisk.SIP.Peer.Type(*)"), H_SIPPeerDetails, _T("T"), DCI_DT_STRING, _T("Asterisk system {instance}: SIP peer type") },
   { _T("Asterisk.SIP.Peer.UserAgent(*)"), H_SIPPeerDetails, _T("U"), DCI_DT_STRING, _T("Asterisk system {instance}: SIP peer user agent") },
   { _T("Asterisk.SIP.Peer.VoiceMailbox(*)"), H_SIPPeerDetails, _T("V"), DCI_DT_STRING, _T("Asterisk system {instance}: SIP peer voice mailbox") },
   { _T("Asterisk.SIP.Peers.Connected(*)"), H_SIPPeerStats, _T("C"), DCI_DT_UINT, _T("Asterisk system {instance}: connected SIP peers") },
   { _T("Asterisk.SIP.Peers.Total(*)"), H_SIPPeerStats, _T("T"), DCI_DT_UINT, _T("Asterisk system {instance}: total SIP peers") },
   { _T("Asterisk.SIP.Peers.Unknown(*)"), H_SIPPeerStats, _T("U"), DCI_DT_UINT, _T("Asterisk system {instance}: unknown state SIP peers") },
   { _T("Asterisk.SIP.Peers.Unmonitored(*)"), H_SIPPeerStats, _T("M"), DCI_DT_UINT, _T("Asterisk system {instance}: unmonitored SIP peers") },
   { _T("Asterisk.SIP.Peers.Unreachable(*)"), H_SIPPeerStats, _T("R"), DCI_DT_UINT, _T("Asterisk system {instance}: unreachable SIP peers") },
   { _T("Asterisk.Version(*)"), H_AsteriskVersion, NULL, DCI_DT_STRING, _T("Asterisk system {instance}: version") }
};

/**
 * Lists
 */
static NETXMS_SUBAGENT_LIST s_lists[] =
{
	{ _T("Asterisk.Channels(*)"), H_ChannelList, NULL },
   { _T("Asterisk.CommandOutput(*)"), H_CommandOutput, NULL },
   { _T("Asterisk.SIP.Peers(*)"), H_SIPPeerList, NULL },
   { _T("Asterisk.Systems"), H_SystemList, NULL }
};

/**
 * Tables
 */
static NETXMS_SUBAGENT_TABLE s_tables[] =
{
   { _T("Asterisk.Channels(*)"), H_ChannelTable, NULL, _T("CHANNEL"), _T("Asterisk system {instance}: channels") },
   { _T("Asterisk.SIP.Peers(*)"), H_SIPPeerTable, NULL, _T("NAME"), _T("Asterisk system {instance}: SIP peers") }
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
   sizeof(s_tables) / sizeof(NETXMS_SUBAGENT_TABLE),
   s_tables,
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
