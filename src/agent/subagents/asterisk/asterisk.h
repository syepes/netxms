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
** File: asterisk.h
**
**/

#ifndef _asterisk_h_
#define _asterisk_h_

#include <nms_common.h>
#include <nms_util.h>
#include <nms_agent.h>

#define DEBUG_TAG _T("asterisk")

#define MAX_AMI_TAG_LEN       32
#define MAX_AMI_SUBTYPE_LEN   64

/**
 * AMI message type
 */
enum AmiMessageType
{
   AMI_UNKNOWN = 0,
   AMI_EVENT = 1,
   AMI_ACTION = 2,
   AMI_RESPONSE = 3
};

/**
 * AMI message tag
 */
struct AmiMessageTag
{
   AmiMessageTag *next;
   char name[MAX_AMI_TAG_LEN];
   char *value;

   AmiMessageTag(const char *_name, const char *_value, AmiMessageTag *_next)
   {
      strlcpy(name, _name, MAX_AMI_TAG_LEN);
      value = strdup(_value);
      next = _next;
   }

   ~AmiMessageTag()
   {
      MemFree(value);
   }
};

/**
 * AMI message
 */
class AmiMessage : public RefCountObject
{
private:
   AmiMessageType m_type;
   char m_subType[MAX_AMI_SUBTYPE_LEN];
   INT64 m_id;
   AmiMessageTag *m_tags;
   StringList *m_data;

   AmiMessage();

   AmiMessageTag *findTag(const char *name);

protected:
   virtual ~AmiMessage();

public:
   AmiMessage(const char *subType);

   AmiMessageType getType() const { return m_type; }
   const char *getSubType() const { return m_subType; }
   bool isSuccess() const { return !stricmp(m_subType, "Success") || !stricmp(m_subType, "Follows"); }

   INT64 getId() const { return m_id; }
   void setId(INT64 id) { m_id = id; }

   const char *getTag(const char *name);
   void setTag(const char *name, const char *value);

   const StringList *getData() const { return m_data; }

   ByteStream *serialize();

   static AmiMessage *createFromNetwork(RingBuffer& buffer);
};

/**
 * AMI event listener
 */
class AmiEventListener
{
public:
   virtual ~AmiEventListener() { }

   virtual void processEvent(AmiMessage *event) = 0;
};

/**
 * Asterisk system information
 */
class AsteriskSystem
{
private:
   TCHAR *m_name;
   InetAddress m_ipAddress;
   UINT16 m_port;
   char *m_login;
   char *m_password;
   SOCKET m_socket;
   THREAD m_connectorThread;
   RingBuffer m_networkBuffer;
   INT64 m_requestId;
   INT64 m_activeRequestId;
   MUTEX m_requestLock;
   CONDITION m_requestCompletion;
   AmiMessage *m_response;
   bool m_amiSessionReady;
   ObjectArray<AmiEventListener> m_eventListeners;
   MUTEX m_eventListenersLock;
   UINT32 m_amiTimeout;

   static THREAD_RESULT THREAD_CALL connectorThreadStarter(void *arg);

   AmiMessage *readMessage() { return AmiMessage::createFromNetwork(m_networkBuffer); }
   bool processMessage(AmiMessage *msg);
   void connectorThread();

   bool sendLoginRequest();

   AsteriskSystem(const TCHAR *name);

public:
   static AsteriskSystem *createFromConfig(ConfigEntry *config);

   ~AsteriskSystem();

   const TCHAR *getName() const { return m_name; }
   bool isAmiSessionReady() const { return m_amiSessionReady; }

   void start();
   void stop();

   void addEventListener(AmiEventListener *listener);
   void removeEventListener(AmiEventListener *listener);

   AmiMessage *sendRequest(AmiMessage *request, ObjectRefArray<AmiMessage> *list = NULL, UINT32 timeout = 0);

   LONG readSingleTag(const char *rqname, const char *tag, TCHAR *value);
   ObjectRefArray<AmiMessage> *readTable(const char *rqname);
};

/**
 * Get configured asterisk system by name
 */
AsteriskSystem *GetAsteriskSystemByName(const TCHAR *name);

/**
 * Standard prologue for parameter handler - retrieve system from first argument
 */
#define GET_ASTERISK_SYSTEM \
TCHAR sysName[256]; \
if (!AgentGetParameterArg(param, 1, sysName, 256)) \
   return SYSINFO_RC_UNSUPPORTED; \
AsteriskSystem *sys = GetAsteriskSystemByName(sysName); \
if (sys == NULL) \
   return SYSINFO_RC_UNSUPPORTED; \

#endif
