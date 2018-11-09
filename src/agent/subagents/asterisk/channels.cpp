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
** File: channels.cpp
**
**/

#include "asterisk.h"

/**
 * Handler for channels list
 */
LONG H_ChannelList(const TCHAR *param, const TCHAR *arg, StringList *value, AbstractCommSession *session)
{
   GET_ASTERISK_SYSTEM;

   ObjectRefArray<AmiMessage> *messages = sys->readTable("Status");
   if (messages == NULL)
      return SYSINFO_RC_ERROR;

   for(int i = 0; i < messages->size(); i++)
   {
      const char *ch = messages->get(i)->getTag("Channel");
      if (ch != NULL)
         value->addMBString(ch);
   }

   delete messages;
   return SYSINFO_RC_SUCCESS;
}

/**
 * Handler for channels table
 */
LONG H_ChannelTable(const TCHAR *param, const TCHAR *arg, Table *value, AbstractCommSession *session)
{
   GET_ASTERISK_SYSTEM;

   ObjectRefArray<AmiMessage> *messages = sys->readTable("Status");
   if (messages == NULL)
      return SYSINFO_RC_ERROR;

   value->addColumn(_T("CHANNEL"), DCI_DT_STRING, _T("Channel"), true);

   delete messages;
   return SYSINFO_RC_SUCCESS;
}
