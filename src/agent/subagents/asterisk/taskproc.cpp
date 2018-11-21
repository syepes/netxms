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
** File: taskproc.cpp
**/

#include "asterisk.h"

/**
 * Task processor information
 */
struct TaskProcessor
{
   TCHAR name[128];
   UINT64 processed;
   UINT32 queued;
   UINT32 maxDepth;
   UINT32 lowWatermark;
   UINT32 highWatermark;
};

/**
 * Get list of task processors
 */
static ObjectArray<TaskProcessor> *ReadTaskProcessorList(AsteriskSystem *sys)
{
   StringList *rawData = sys->executeCommand("core show taskprocessors");
   if (rawData == NULL)
      return NULL;

   ObjectArray<TaskProcessor> *list = new ObjectArray<TaskProcessor>(128, 128, true);
   for(int i = 0; i < rawData->size(); i++)
   {
      TCHAR name[128];
      unsigned long long processed;
      unsigned int queued, maxDepth, lw, hw;
      if (_stscanf(rawData->get(i), _T("%127s %Lu %u %u %u %u"), name, &processed, &queued, &maxDepth, &lw, &hw) == 6)
      {
         TaskProcessor *p = new TaskProcessor;
         _tcscpy(p->name, name);
         p->processed = processed;
         p->queued = queued;
         p->maxDepth = maxDepth;
         p->lowWatermark = lw;
         p->highWatermark = hw;
         list->add(p);
      }
   }

   delete rawData;
   return list;
}

/**
 * Handler for Asterisk.TaskProcessors list
 */
LONG H_TaskProcessorList(const TCHAR *param, const TCHAR *arg, StringList *value, AbstractCommSession *session)
{
   GET_ASTERISK_SYSTEM;
   ObjectArray<TaskProcessor> *list = ReadTaskProcessorList(sys);
   if (list == NULL)
      return SYSINFO_RC_ERROR;

   for(int i = 0; i < list->size(); i++)
      value->add(list->get(i)->name);
   delete list;

   return SYSINFO_RC_SUCCESS;
}
