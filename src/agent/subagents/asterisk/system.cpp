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
** File: system.cpp
**
**/

#include "asterisk.h"

/**
 * Create from configuration entry
 */
AsteriskSystem *AsteriskSystem::createFromConfig(ConfigEntry *config)
{
   AsteriskSystem *as = new AsteriskSystem(config->getName());

   as->m_ipAddress = InetAddress::resolveHostName(config->getSubEntryValue(_T("Hostname"), 0, _T("127.0.0.1")));
   if (!as->m_ipAddress.isValidUnicast() && !as->m_ipAddress.isLoopback())
   {
      nxlog_debug_tag(DEBUG_TAG, 4, _T("Invalid or unknown hostname for system %s"), config->getName());
      delete as;
      return NULL;
   }

   int port = config->getSubEntryValueAsInt(_T("Port"), 0, 5038);
   if ((port < 1) || (port > 65535))
   {
      nxlog_debug_tag(DEBUG_TAG, 4, _T("Invalid port number for system %s"), config->getName());
      delete as;
      return NULL;
   }
   as->m_port = (UINT16)port;

   as->m_login = UTF8StringFromTString(config->getSubEntryValue(_T("Login"), 0, _T("root")));
   as->m_password = UTF8StringFromTString(config->getSubEntryValue(_T("Password"), 0, _T("")));

   return as;
}

/**
 * Constructor
 */
AsteriskSystem::AsteriskSystem(const TCHAR *name)
{
   m_name = _tcsdup(name);
   m_port = 5038;
   m_login = NULL;
   m_password = NULL;
   m_connectorThread = INVALID_THREAD_HANDLE;
   m_socket = INVALID_SOCKET;
   m_requestId = 1;
   m_activeRequestId = 0;
   m_requestLock = MutexCreate();
   m_requestCompletion = ConditionCreate(false);
   m_response = NULL;
   m_amiSessionReady = false;
}

/**
 * Destructor
 */
AsteriskSystem::~AsteriskSystem()
{
   MemFree(m_name);
   MemFree(m_login);
   MemFree(m_password);
   MutexDestroy(m_requestLock);
   ConditionDestroy(m_requestCompletion);
   delete m_response;
}
