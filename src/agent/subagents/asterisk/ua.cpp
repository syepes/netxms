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
** File: ua.cpp
**/

#include "asterisk.h"
#include <eXosip2/eXosip.h>

/**
 * Do SIP client registration
 */
static INT64 DoRegistration(const char *login, const char *password, const char *domain, const char *proxy)
{
   char uri[256];
   snprintf(uri, 256, "sip:%s@%s", login, domain);

   struct eXosip_t *ctx = eXosip_malloc();
   if (ctx == NULL)
   {
      nxlog_debug_tag(DEBUG_TAG, 5, _T("TestRegistration(%hs via %hs): call to eXosip_malloc() failed"), uri, proxy);
      return -1;
   }

   if (eXosip_init(ctx) != 0)
   {
      nxlog_debug_tag(DEBUG_TAG, 5, _T("TestRegistration(%hs via %hs): call to eXosip_init() failed"), uri, proxy);
      return -1;
   }

   eXosip_set_user_agent(ctx, "NetXMS/" NETXMS_BUILD_TAG_A);

   // open a UDP socket
   if (eXosip_listen_addr(ctx, IPPROTO_UDP, NULL, 0, AF_INET, 0) != 0)
   {
      eXosip_quit(ctx);
      nxlog_debug_tag(DEBUG_TAG, 5, _T("TestRegistration(%hs via %hs): call to eXosip_listen_addr() failed"), uri, proxy);
      return -1;
   }

   eXosip_lock(ctx);

   eXosip_add_authentication_info(ctx, login, login, password, NULL, NULL);

   osip_message_t *regmsg = NULL;
   int registrationId = eXosip_register_build_initial_register(ctx, uri, proxy, uri, 600, &regmsg);
   if (registrationId < 0)
   {
      eXosip_unlock(ctx);
      eXosip_quit(ctx);
      nxlog_debug_tag(DEBUG_TAG, 5, _T("TestRegistration(%hs via %hs): call to eXosip_register_build_initial_register() failed"), uri, proxy);
      return -1;
   }

   INT64 startTime = GetCurrentTimeMs();
   int rc = eXosip_register_send_register(ctx, registrationId, regmsg);
   eXosip_unlock(ctx);

   if (rc != 0)
   {
      eXosip_quit(ctx);
      nxlog_debug_tag(DEBUG_TAG, 5, _T("TestRegistration(%hs via %hs): call to eXosip_register_send_register() failed"), uri, proxy);
      return -1;
   }

   bool success = false;
   INT64 elapsedTime;
   while(true)
   {
      eXosip_event_t *evt = eXosip_event_wait(ctx, 0, 100);

      eXosip_execute(ctx);
      eXosip_automatic_action(ctx);

      if (evt == NULL)
         continue;

      if ((evt->type == EXOSIP_REGISTRATION_FAILURE) && (evt->response != NULL) &&
          ((evt->response->status_code == 401) || (evt->response->status_code == 407)))
      {
         nxlog_debug_tag(DEBUG_TAG, 8, _T("TestRegistration(%hs via %hs): registration failed with status code %03d (will retry)"),
                  uri, proxy, evt->response->status_code);
         eXosip_default_action(ctx, evt);
         eXosip_event_free(evt);
         continue;
      }

      if (evt->type == EXOSIP_REGISTRATION_SUCCESS)
      {
         elapsedTime = GetCurrentTimeMs() - startTime;
         nxlog_debug_tag(DEBUG_TAG, 8, _T("TestRegistration(%hs via %hs): registration successful"), uri, proxy);
         success = true;
         eXosip_event_free(evt);
         break;
      }

      if (evt->type != EXOSIP_MESSAGE_PROCEEDING)
      {
         int status = (evt->response != NULL) ? evt->response->status_code : 408;
         nxlog_debug_tag(DEBUG_TAG, 8, _T("TestRegistration(%hs via %hs): registration failed with status code %03d (will not retry)"), uri, proxy, status);
         eXosip_event_free(evt);
         break;
      }

      eXosip_event_free(evt);
   }

   eXosip_quit(ctx);
   return success ? elapsedTime : -1;
}

/**
 * Handler for Asterisk.SIP.Register parameter
 */
LONG H_SIPRegister(const TCHAR *param, const TCHAR *arg, TCHAR *value, AbstractCommSession *session)
{
   GET_ASTERISK_SYSTEM(3);

   char login[128], password[128], domain[128];
   GET_ARGUMENT_A(1, login, 128);
   GET_ARGUMENT_A(2, password, 128);
   GET_ARGUMENT_A(3, domain, 128);

   char proxy[256] = "sip:";
   sys->getIpAddress().toStringA(&proxy[4]);
   ret_int64(value, DoRegistration(login, password, domain, proxy));
   return SYSINFO_RC_SUCCESS;
}
