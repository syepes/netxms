/*
** NetXMS - Network Management System
** Log Parsing Library
** Copyright (C) 2003-2017 Victor Kirhenshtein
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
** File: wevt.cpp
**
**/

#include "libnxlp.h"
#include <winevt.h>
#include <winmeta.h>

/**
 * Static data
 */
static BOOL (WINAPI *_EvtClose)(EVT_HANDLE);
static EVT_HANDLE (WINAPI *_EvtCreateRenderContext)(DWORD, LPCWSTR *, DWORD);
static BOOL (WINAPI *_EvtGetPublisherMetadataProperty)(EVT_HANDLE, EVT_PUBLISHER_METADATA_PROPERTY_ID, DWORD, DWORD, PEVT_VARIANT, PDWORD);
static BOOL (WINAPI *_EvtFormatMessage)(EVT_HANDLE, EVT_HANDLE, DWORD, DWORD, PEVT_VARIANT, DWORD, DWORD, LPWSTR, PDWORD);
static BOOL (WINAPI *_EvtNext)(EVT_HANDLE, DWORD, EVT_HANDLE *, DWORD, DWORD, PDWORD);
static EVT_HANDLE (WINAPI *_EvtOpenPublisherMetadata)(EVT_HANDLE, LPCWSTR, LPCWSTR, LCID, DWORD);
static EVT_HANDLE (WINAPI *_EvtQuery)(EVT_HANDLE, LPCWSTR, LPCWSTR, DWORD);
static BOOL (WINAPI *_EvtRender)(EVT_HANDLE, EVT_HANDLE, DWORD, DWORD, PVOID, PDWORD, PDWORD);
static EVT_HANDLE (WINAPI *_EvtSubscribe)(EVT_HANDLE, HANDLE, LPCWSTR, LPCWSTR, EVT_HANDLE, PVOID, EVT_SUBSCRIBE_CALLBACK, DWORD);

/**
 * Log metadata property
 */
static void LogMetadataProperty(EVT_HANDLE pubMetadata, EVT_PUBLISHER_METADATA_PROPERTY_ID id, const TCHAR *name)
{
	WCHAR buffer[4096];
	PEVT_VARIANT p = PEVT_VARIANT(buffer);
	DWORD size;
	if (_EvtGetPublisherMetadataProperty(pubMetadata, id, 0, sizeof(buffer), p, &size))
   {
      switch(p->Type)
      {
         case EvtVarTypeNull:
      	   nxlog_debug_tag(DEBUG_TAG, 5, _T("Publisher %s: NULL"), name);
            break;
         case EvtVarTypeString:
      	   nxlog_debug_tag(DEBUG_TAG, 5, _T("Publisher %s: %ls"), name, p->StringVal);
            break;
         case EvtVarTypeAnsiString:
      	   nxlog_debug_tag(DEBUG_TAG, 5, _T("Publisher %s: %hs"), name, p->AnsiStringVal);
            break;
         default:
      	   nxlog_debug_tag(DEBUG_TAG, 5, _T("Publisher %s: (variant type %d)"), name, (int)p->Type);
            break;
      }
   }
}

/**
 * Extract variables from event
 */
static StringList *ExtractVariables(EVT_HANDLE event)
{
   TCHAR buffer[8192];

   static PCWSTR eventProperties[] = { L"Event/EventData/Data[1]" };
   EVT_HANDLE renderContext = _EvtCreateRenderContext(0, NULL, EvtRenderContextUser);
   if (renderContext == NULL)
   {
      nxlog_debug_tag(DEBUG_TAG, 5, _T("ExtractVariables: Call to EvtCreateRenderContext failed: %s"),
         GetSystemErrorText(GetLastError(), (TCHAR *)buffer, 4096));
      return NULL;
   }

   StringList *variables = NULL;

   // Get event values
   DWORD reqSize, propCount = 0;
   if (!_EvtRender(renderContext, event, EvtRenderEventValues, 8192, buffer, &reqSize, &propCount))
   {
      nxlog_debug_tag(DEBUG_TAG, 5, _T("ExtractVariables: Call to EvtRender failed: %s"),
         GetSystemErrorText(GetLastError(), (TCHAR *)buffer, 4096));
      goto cleanup;
   }

   if (propCount > 0)
   {
      variables = new StringList();
      PEVT_VARIANT values = PEVT_VARIANT(buffer);
      for (DWORD i = 0; i < propCount; i++)
      {
         switch (values[i].Type)
         {
            case EvtVarTypeString:
               variables->add(values[i].StringVal);
               break;
            case EvtVarTypeAnsiString:
               variables->addMBString(values[i].AnsiStringVal);
               break;
            case EvtVarTypeSByte:
               variables->add((INT32)values[i].SByteVal);
               break;
            case EvtVarTypeByte:
               variables->add((INT32)values[i].ByteVal);
               break;
            case EvtVarTypeInt16:
               variables->add((INT32)values[i].Int16Val);
               break;
            case EvtVarTypeUInt16:
               variables->add((UINT32)values[i].UInt16Val);
               break;
            case EvtVarTypeInt32:
               variables->add(values[i].Int32Val);
               break;
            case EvtVarTypeUInt32:
               variables->add(values[i].UInt32Val);
               break;
            case EvtVarTypeInt64:
               variables->add(values[i].Int64Val);
               break;
            case EvtVarTypeUInt64:
               variables->add(values[i].UInt64Val);
               break;
            case EvtVarTypeSingle:
               variables->add(values[i].SingleVal);
               break;
            case EvtVarTypeDouble:
               variables->add(values[i].DoubleVal);
               break;
            case EvtVarTypeBoolean:
               variables->add(values[i].BooleanVal ? _T("True") : _T("False"));
               break;
            default:
               variables->add(_T(""));
               break;
         }
      }
   }

cleanup:
   _EvtClose(renderContext);
   return variables;
}

/**
 * Subscription callback
 */
static DWORD WINAPI SubscribeCallback(EVT_SUBSCRIBE_NOTIFY_ACTION action, PVOID userContext, EVT_HANDLE event)
{
	WCHAR buffer[4096], *msg = buffer;
	DWORD reqSize, propCount = 0;
	EVT_HANDLE pubMetadata = NULL;
	BOOL success;
	TCHAR publisherName[MAX_PATH];

	// Create render context for event values - we need provider name,
	// event id, and severity level
	static PCWSTR eventProperties[] =
   {
      L"Event/System/Provider/@Name",
      L"Event/System/EventID",
      L"Event/System/Level",
      L"Event/System/Keywords",
      L"Event/System/EventRecordID",
      L"Event/System/TimeCreated/@SystemTime"
   };
	EVT_HANDLE renderContext = _EvtCreateRenderContext(6, eventProperties, EvtRenderContextValues);
	if (renderContext == NULL)
	{
		nxlog_debug_tag(DEBUG_TAG, 5, _T("Call to EvtCreateRenderContext failed: %s"),
							    GetSystemErrorText(GetLastError(), (TCHAR *)buffer, 4096));
		return 0;
	}

	// Get event values
	if (!_EvtRender(renderContext, event, EvtRenderEventValues, 4096, buffer, &reqSize, &propCount))
	{
		nxlog_debug_tag(DEBUG_TAG, 5, _T("Call to EvtRender failed: %s"),
		                   GetSystemErrorText(GetLastError(), (TCHAR *)buffer, 4096));
		goto cleanup;
	}

	// Publisher name
	PEVT_VARIANT values = PEVT_VARIANT(buffer);
	if ((values[0].Type == EvtVarTypeString) && (values[0].StringVal != NULL))
	{
#ifdef UNICODE
		wcslcpy(publisherName, values[0].StringVal, MAX_PATH);
#else
		WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DEFAULTCHAR, values[0].StringVal, -1, publisherName, MAX_PATH, NULL, NULL);
#endif
      static_cast<LogParser*>(userContext)->trace(7, _T("Publisher name is %s"), publisherName);
   }
	else
	{
      static_cast<LogParser*>(userContext)->trace(7, _T("Unable to get publisher name from event"));
	}

	// Event id
	DWORD eventId = 0;
	if (values[1].Type == EvtVarTypeUInt16)
		eventId = values[1].UInt16Val;

	// Severity level
	DWORD level = 0;
	if (values[2].Type == EvtVarTypeByte)
	{
		switch(values[2].ByteVal)
		{
			case WINEVENT_LEVEL_CRITICAL:
            level = 0x0100;
            break;
			case WINEVENT_LEVEL_ERROR:
				level = EVENTLOG_ERROR_TYPE;
				break;
			case WINEVENT_LEVEL_WARNING :
				level = EVENTLOG_WARNING_TYPE;
				break;
			case WINEVENT_LEVEL_INFO:
			case WINEVENT_LEVEL_VERBOSE:
				level = EVENTLOG_INFORMATION_TYPE;
				break;
			default:
				level = EVENTLOG_INFORMATION_TYPE;
				break;
		}
	}

	// Keywords
	if (values[3].UInt64Val & WINEVENT_KEYWORD_AUDIT_SUCCESS)
		level = EVENTLOG_AUDIT_SUCCESS;
	else if (values[3].UInt64Val & WINEVENT_KEYWORD_AUDIT_FAILURE)
		level = EVENTLOG_AUDIT_FAILURE;

   // Record ID
   UINT64 recordId = 0;
   if ((values[4].Type == EvtVarTypeUInt64) || (values[4].Type == EvtVarTypeInt64))
      recordId = values[4].UInt64Val;
   else if ((values[4].Type == EvtVarTypeUInt32) || (values[4].Type == EvtVarTypeInt32))
      recordId = values[4].UInt32Val;

   // Time created
   time_t timestamp;
   if (values[5].Type == EvtVarTypeFileTime)
   {
      timestamp = static_cast<time_t>((values[5].FileTimeVal - EPOCHFILETIME) / 10000000);
   }
   else if (values[5].Type == EvtVarTypeSysTime)
   {
      FILETIME ft;
      SystemTimeToFileTime(values[5].SysTimeVal, &ft);

      LARGE_INTEGER li;
      li.LowPart = ft.dwLowDateTime;
      li.HighPart = ft.dwHighDateTime;
      timestamp = static_cast<time_t>((li.QuadPart - EPOCHFILETIME) / 10000000);
   }
   else
   {
      timestamp = time(NULL);
      static_cast<LogParser*>(userContext)->trace(7, _T("Cannot get timestamp from event (values[5].Type == %d)"), values[5].Type);
   }

	// Open publisher metadata
	pubMetadata = _EvtOpenPublisherMetadata(NULL, values[0].StringVal, NULL, MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), SORT_DEFAULT), 0);
	if (pubMetadata == NULL)
	{
		nxlog_debug_tag(DEBUG_TAG, 5, _T("Call to EvtOpenPublisherMetadata failed: %s"), GetSystemErrorText(GetLastError(), (TCHAR *)buffer, 4096));
		goto cleanup;
	}

	// Format message text
	success = _EvtFormatMessage(pubMetadata, event, 0, 0, NULL, EvtFormatMessageEvent, 4096, buffer, &reqSize);
	if (!success)
	{
      DWORD error = GetLastError();
		if ((error != ERROR_INSUFFICIENT_BUFFER) && (error != ERROR_EVT_UNRESOLVED_VALUE_INSERT))
		{
			nxlog_debug_tag(DEBUG_TAG, 5, _T("Call to EvtFormatMessage failed: error %u (%s)"), error, GetSystemErrorText(error, (TCHAR *)buffer, 4096));
			LogMetadataProperty(pubMetadata, EvtPublisherMetadataMessageFilePath, _T("message file"));
			LogMetadataProperty(pubMetadata, EvtPublisherMetadataParameterFilePath, _T("parameter file"));
			LogMetadataProperty(pubMetadata, EvtPublisherMetadataResourceFilePath, _T("resource file"));
			goto cleanup;
		}
      if (error == ERROR_INSUFFICIENT_BUFFER)
      {
		   msg = MemAllocStringW(reqSize);
		   success = _EvtFormatMessage(pubMetadata, event, 0, 0, NULL, EvtFormatMessageEvent, reqSize, msg, &reqSize);
		   if (!success)
		   {
            error = GetLastError();
            if (error != ERROR_EVT_UNRESOLVED_VALUE_INSERT)
            {
			      nxlog_debug_tag(DEBUG_TAG, 5, _T("Retry call to EvtFormatMessage failed: error %u (%s)"), error, GetSystemErrorText(error, (TCHAR *)buffer, 4096));
			      LogMetadataProperty(pubMetadata, EvtPublisherMetadataMessageFilePath, _T("message file"));
			      LogMetadataProperty(pubMetadata, EvtPublisherMetadataParameterFilePath, _T("parameter file"));
			      LogMetadataProperty(pubMetadata, EvtPublisherMetadataResourceFilePath, _T("resource file"));
			      goto cleanup;
            }
		   }
      }
	}

   StringList *variables = ExtractVariables(event);
#ifdef UNICODE
   static_cast<LogParser*>(userContext)->matchEvent(publisherName, eventId, level, msg, variables, recordId, 0, timestamp);
#else
	char *mbmsg = MBStringFromWideString(msg);
   static_cast<LogParser*>(userContext)->matchEvent(publisherName, eventId, level, mbmsg, variables, recordId, 0, timestamp);
	MemFree(mbmsg);
#endif
   delete variables;

   static_cast<LogParser*>(userContext)->saveLastProcessedRecordTimestamp(timestamp);

cleanup:
	if (pubMetadata != NULL)
		_EvtClose(pubMetadata);
	_EvtClose(renderContext);
	if (msg != buffer)
		MemFree(msg);
	return 0;
}

/**
 * Event log parser thread
 */
bool LogParser::monitorEventLogV6()
{
   bool success;

   // Read old records if needed
   if (m_marker != NULL)
   {
      time_t startTime = readLastProcessedRecordTimestamp();
      time_t now = time(NULL);
      if (startTime < now)
      {
		   nxlog_debug_tag(DEBUG_TAG, 1, _T("Reading old events between %I64d and %I64d from %s"), (INT64)startTime, (INT64)now, &m_fileName[1]);

         WCHAR query[256];
         _snwprintf(query, 256, L"*[System/TimeCreated[timediff(@SystemTime) < %I64d]]", (INT64)(now - startTime) * 1000LL);
         nxlog_debug_tag(DEBUG_TAG, 4, _T("Event log query: \"%s\""), query);
         EVT_HANDLE handle = _EvtQuery(NULL, &m_fileName[1], query, EvtQueryChannelPath | EvtQueryForwardDirection);
         if (handle != NULL)
         {
            EVT_HANDLE events[64];
            DWORD count;
            while(_EvtNext(handle, 64, events, 5000, 0, &count))
            {
               for(DWORD i = 0; i < count; i++)
               {
                  SubscribeCallback(EvtSubscribeActionDeliver, this, events[i]);
                  _EvtClose(events[i]);
               }
            }
   		   _EvtClose(handle);
         }
         else
         {
            TCHAR buffer[1024];
   		   nxlog_debug_tag(DEBUG_TAG, 1, _T("EvtQuery failed (%s)"), GetSystemErrorText(GetLastError(), buffer, 1024));
         }
      }
   }

#ifdef UNICODE
	EVT_HANDLE handle = _EvtSubscribe(NULL, NULL, &m_fileName[1], NULL, NULL, this, SubscribeCallback, EvtSubscribeToFutureEvents);
#else
	WCHAR channel[MAX_PATH];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, &m_fileName[1], -1, channel, MAX_PATH);
	channel[MAX_PATH - 1] = 0;
	EVT_HANDLE handle = _EvtSubscribe(NULL, NULL, channel, NULL, NULL, this, SubscribeCallback, EvtSubscribeToFutureEvents);
#endif
	if (handle != NULL)
	{
		nxlog_debug_tag(DEBUG_TAG, 1, _T("Start watching event log \"%s\" (using EvtSubscribe)"), &m_fileName[1]);
		setStatus(LPS_RUNNING);
		WaitForSingleObject(m_stopCondition, INFINITE);
		nxlog_debug_tag(DEBUG_TAG, 1, _T("Stop watching event log \"%s\" (using EvtSubscribe)"), &m_fileName[1]);
		_EvtClose(handle);
      success = true;
	}
	else
	{
		TCHAR buffer[1024];
		nxlog_debug_tag(DEBUG_TAG, 0, _T("Unable to open event log \"%s\" with EvtSubscribe(): %s"),
		               &m_fileName[1], GetSystemErrorText(GetLastError(), buffer, 1024));
		setStatus(LPS_EVT_SUBSCRIBE_ERROR);
      success = false;
	}
   return success;
}

/**
 * Initialize event log reader for Windows Vista and later
 */
bool InitEventLogParsersV6()
{
	HMODULE module = LoadLibrary(_T("wevtapi.dll"));
	if (module == NULL)
	{
		TCHAR buffer[1024];
		nxlog_debug_tag(DEBUG_TAG, 1, _T("Cannot load wevtapi.dll: %s"), GetSystemErrorText(GetLastError(), buffer, 1024));
		return false;
	}

	_EvtClose = (BOOL (WINAPI *)(EVT_HANDLE))GetProcAddress(module, "EvtClose");
	_EvtCreateRenderContext = (EVT_HANDLE (WINAPI *)(DWORD, LPCWSTR *, DWORD))GetProcAddress(module, "EvtCreateRenderContext");
	_EvtGetPublisherMetadataProperty = (BOOL (WINAPI *)(EVT_HANDLE, EVT_PUBLISHER_METADATA_PROPERTY_ID, DWORD, DWORD, PEVT_VARIANT, PDWORD))GetProcAddress(module, "EvtGetPublisherMetadataProperty");
	_EvtFormatMessage = (BOOL (WINAPI *)(EVT_HANDLE, EVT_HANDLE, DWORD, DWORD, PEVT_VARIANT, DWORD, DWORD, LPWSTR, PDWORD))GetProcAddress(module, "EvtFormatMessage");
   _EvtNext = (BOOL (WINAPI *)(EVT_HANDLE, DWORD, EVT_HANDLE *, DWORD, DWORD, PDWORD))GetProcAddress(module, "EvtNext");
   _EvtOpenPublisherMetadata = (EVT_HANDLE (WINAPI *)(EVT_HANDLE, LPCWSTR, LPCWSTR, LCID, DWORD))GetProcAddress(module, "EvtOpenPublisherMetadata");
	_EvtQuery = (EVT_HANDLE (WINAPI *)(EVT_HANDLE, LPCWSTR, LPCWSTR, DWORD))GetProcAddress(module, "EvtQuery");
	_EvtRender = (BOOL (WINAPI *)(EVT_HANDLE, EVT_HANDLE, DWORD, DWORD, PVOID, PDWORD, PDWORD))GetProcAddress(module, "EvtRender");
	_EvtSubscribe = (EVT_HANDLE (WINAPI *)(EVT_HANDLE, HANDLE, LPCWSTR, LPCWSTR, EVT_HANDLE, PVOID, EVT_SUBSCRIBE_CALLBACK, DWORD))GetProcAddress(module, "EvtSubscribe");

	return (_EvtClose != NULL) && (_EvtCreateRenderContext != NULL) &&
          (_EvtGetPublisherMetadataProperty != NULL) && (_EvtFormatMessage != NULL) && 
	       (_EvtNext != NULL) && (_EvtOpenPublisherMetadata != NULL) &&
          (_EvtQuery != NULL) && (_EvtRender != NULL) && (_EvtSubscribe != NULL);
}
