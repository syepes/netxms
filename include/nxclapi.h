/* 
** NetXMS - Network Management System
** Client Library API
** Copyright (C) 2004 Victor Kirhenshtein
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
** $module: nxclapi.h
**
**/

#ifndef _nxclapi_h_
#define _nxclapi_h_

#include <nms_common.h>
#include <nms_util.h>
#include <nxevent.h>
#include <nximage.h>
#include <nxcscpapi.h>

#ifdef _WIN32
#ifdef LIBNXCL_EXPORTS
#define LIBNXCL_EXPORTABLE __declspec(dllexport)
#else
#define LIBNXCL_EXPORTABLE __declspec(dllimport)
#endif
#else    /* _WIN32 */
#define LIBNXCL_EXPORTABLE
#endif


//
// Some constants
//

#define MAX_OBJECT_NAME          64
#define MAX_COMMUNITY_LENGTH     32
#define MAX_OID_LENGTH           1024
#define MAX_EVENT_MSG_LENGTH     256
#define MAX_EVENT_NAME           64
#define MAX_USER_NAME            64
#define MAX_USER_FULLNAME        128
#define MAX_USER_DESCR           256
#define MAX_ITEM_NAME            256
#define MAX_STRING_VALUE         256
#define GROUP_FLAG               ((DWORD)0x80000000)
#define GROUP_EVERYONE           ((DWORD)0x80000000)
#define INVALID_UID              ((DWORD)0xFFFFFFFF)
#define OBJECT_STATUS_COUNT      9
#define MAX_RCPT_ADDR_LEN        256
#define MAX_EMAIL_SUBJECT_LEN    256
#define MAC_ADDR_LENGTH          6


//
// Image formats
//

#define IMAGE_FORMAT_ICO      0
#define IMAGE_FORMAT_PNG      1


//
// Authentication methods
//

#define AUTH_NONE             0
#define AUTH_PLAINTEXT        1
#define AUTH_MD5_HASH         2
#define AUTH_SHA1_HASH        3


//
// Forced poll types
//

#define POLL_STATUS           1
#define POLL_CONFIGURATION    2


//
// Object types
//

#define OBJECT_GENERIC        0
#define OBJECT_SUBNET         1
#define OBJECT_NODE           2
#define OBJECT_INTERFACE      3
#define OBJECT_NETWORK        4
#define OBJECT_CONTAINER      5
#define OBJECT_ZONE           6
#define OBJECT_SERVICEROOT    7
#define OBJECT_TEMPLATE       8
#define OBJECT_TEMPLATEGROUP  9
#define OBJECT_TEMPLATEROOT   10


//
// Object's status
//

#define STATUS_NORMAL         0
#define STATUS_WARNING        1
#define STATUS_MINOR          2
#define STATUS_MAJOR          3
#define STATUS_CRITICAL       4
#define STATUS_UNKNOWN        5
#define STATUS_UNMANAGED      6
#define STATUS_DISABLED       7
#define STATUS_TESTING        8


//
// Event and alarm severity
//

#define SEVERITY_NORMAL       0
#define SEVERITY_WARNING      1
#define SEVERITY_MINOR        2
#define SEVERITY_MAJOR        3
#define SEVERITY_CRITICAL     4
#define SEVERITY_FROM_EVENT   5
#define SEVERITY_NONE         6


//
// Node flags
//

#define NF_IS_SNMP         0x0001
#define NF_IS_NATIVE_AGENT 0x0002
#define NF_IS_BRIDGE       0x0004
#define NF_IS_ROUTER       0x0008
#define NF_IS_LOCAL_MGMT   0x0010
#define NF_IS_PRINTER      0x0020
#define NS_IS_OSPF         0x0040


//
// Events
//

#define NXC_EVENT_STATE_CHANGED        1
#define NXC_EVENT_ERROR                2
#define NXC_EVENT_LOGIN_RESULT         3
#define NXC_EVENT_NEW_ELOG_RECORD      4
#define NXC_EVENT_USER_DB_CHANGED      5
#define NXC_EVENT_OBJECT_CHANGED       6
#define NXC_EVENT_NOTIFICATION         7


//
// Errors
//

#define NXC_ERR_INTERNAL            1


//
// States (used both by client library and server)
//

#define STATE_DISCONNECTED    0
#define STATE_CONNECTING      1
#define STATE_CONNECTED       2
#define STATE_AUTHENTICATED   3


//
// Notification codes
//

#define NX_NOTIFY_SHUTDOWN          1
#define NX_NOTIFY_EVENTDB_CHANGED   2
#define NX_NOTIFY_ALARM_DELETED     3
#define NX_NOTIFY_NEW_ALARM         4
#define NX_NOTIFY_ALARM_ACKNOWLEGED 5
#define NX_NOTIFY_ACTION_CREATED    6
#define NX_NOTIFY_ACTION_MODIFIED   7
#define NX_NOTIFY_ACTION_DELETED    8


//
// Request completion codes
//

#define RCC_SUCCESS                 ((DWORD)0)
#define RCC_COMPONENT_LOCKED        ((DWORD)1)
#define RCC_ACCESS_DENIED           ((DWORD)2)
#define RCC_INVALID_REQUEST         ((DWORD)3)
#define RCC_TIMEOUT                 ((DWORD)4)
#define RCC_OUT_OF_STATE_REQUEST    ((DWORD)5)
#define RCC_DB_FAILURE              ((DWORD)6)
#define RCC_INVALID_OBJECT_ID       ((DWORD)7)
#define RCC_ALREADY_EXIST           ((DWORD)8)
#define RCC_COMM_FAILURE            ((DWORD)9)
#define RCC_SYSTEM_FAILURE          ((DWORD)10)
#define RCC_INVALID_USER_ID         ((DWORD)11)
#define RCC_INVALID_ARGUMENT        ((DWORD)12)
#define RCC_DUPLICATE_DCI           ((DWORD)13)
#define RCC_INVALID_DCI_ID          ((DWORD)14)
#define RCC_OUT_OF_MEMORY           ((DWORD)15)
#define RCC_IO_ERROR                ((DWORD)16)
#define RCC_INCOMPATIBLE_OPERATION  ((DWORD)17)
#define RCC_OBJECT_CREATION_FAILED  ((DWORD)18)
#define RCC_OBJECT_LOOP             ((DWORD)19)
#define RCC_INVALID_OBJECT_NAME     ((DWORD)20)
#define RCC_INVALID_ALARM_ID        ((DWORD)21)
#define RCC_INVALID_ACTION_ID       ((DWORD)22)
#define RCC_OPERATION_IN_PROGRESS   ((DWORD)23)
#define RCC_DCI_COPY_ERRORS         ((DWORD)24)
#define RCC_INVALID_EVENT_CODE      ((DWORD)25)
#define RCC_NO_WOL_INTERFACES       ((DWORD)26)
#define RCC_NO_MAC_ADDRESS          ((DWORD)27)
#define RCC_NOT_IMPLEMENTED         ((DWORD)28)
#define RCC_INVALID_TRAP_ID         ((DWORD)29)
#define RCC_DCI_NOT_SUPPORTED       ((DWORD)30)
#define RCC_VERSION_MISMATCH        ((DWORD)31)


//
// Mask bits for NXCModifyEventTemplate()
//

#define EM_SEVERITY        ((DWORD)0x01)
#define EM_FLAGS           ((DWORD)0x02)
#define EM_NAME            ((DWORD)0x04)
#define EM_MESSAGE         ((DWORD)0x08)
#define EM_DESCRIPTION     ((DWORD)0x10)
#define EM_ALL             ((DWORD)0x1F)


//
// Mask bits (flags) for NXCModifyObject()
//

#define OBJ_UPDATE_NAME             ((DWORD)0x0001)
#define OBJ_UPDATE_AGENT_PORT       ((DWORD)0x0002)
#define OBJ_UPDATE_AGENT_AUTH       ((DWORD)0x0004)
#define OBJ_UPDATE_AGENT_SECRET     ((DWORD)0x0008)
#define OBJ_UPDATE_SNMP_VERSION     ((DWORD)0x0010)
#define OBJ_UPDATE_SNMP_COMMUNITY   ((DWORD)0x0020)
#define OBJ_UPDATE_ACL              ((DWORD)0x0040)
#define OBJ_UPDATE_IMAGE            ((DWORD)0x0080)
#define OBJ_UPDATE_DESCRIPTION      ((DWORD)0x0100)
#define OBJ_UPDATE_ALL              ((DWORD)0x01FF)


//
// Global user rights
//

#define SYSTEM_ACCESS_MANAGE_USERS        0x0001
#define SYSTEM_ACCESS_SERVER_CONFIG       0x0002
#define SYSTEM_ACCESS_CONFIGURE_TRAPS     0x0004
#define SYSTEM_ACCESS_DROP_CONNECTIONS    0x0008
#define SYSTEM_ACCESS_VIEW_EVENT_DB       0x0010
#define SYSTEM_ACCESS_EDIT_EVENT_DB       0x0020
#define SYSTEM_ACCESS_EPP                 0x0040
#define SYSTEM_ACCESS_MANAGE_ACTIONS      0x0080

#define SYSTEM_ACCESS_FULL                0x00FF


//
// Object access rights
//

#define OBJECT_ACCESS_READ          0x00000001
#define OBJECT_ACCESS_MODIFY        0x00000002
#define OBJECT_ACCESS_CREATE        0x00000004
#define OBJECT_ACCESS_DELETE        0x00000008
#define OBJECT_ACCESS_READ_ALARMS   0x00000010
#define OBJECT_ACCESS_CONTROL       0x00000020
#define OBJECT_ACCESS_ACK_ALARMS    0x00000040
#define OBJECT_ACCESS_SEND_EVENTS   0x00000080


//
// User/group flags
//

#define UF_MODIFIED                 0x0001
#define UF_DELETED                  0x0002
#define UF_DISABLED                 0x0004
#define UF_CHANGE_PASSWORD          0x0008
#define UF_CANNOT_CHANGE_PASSWORD   0x0010


//
// User database change notification types
//

#define USER_DB_CREATE              0
#define USER_DB_DELETE              1
#define USER_DB_MODIFY              2


//
// Data types
//

#define DCI_DT_INT         0
#define DCI_DT_UINT        1
#define DCI_DT_INT64       2
#define DCI_DT_UINT64      3
#define DCI_DT_STRING      4
#define DCI_DT_FLOAT       5


//
// Data sources
//

#define DS_INTERNAL        0
#define DS_NATIVE_AGENT    1
#define DS_SNMP_AGENT      2


//
// Item status
//

#define ITEM_STATUS_ACTIVE          0
#define ITEM_STATUS_DISABLED        1
#define ITEM_STATUS_NOT_SUPPORTED   2


//
// Delta calculation methods for DCIs
//

#define DCM_ORIGINAL_VALUE       0
#define DCM_SIMPLE               1
#define DCM_AVERAGE_PER_SECOND   2
#define DCM_AVERAGE_PER_MINUTE   3


//
// Threshold functions and operations
//

#define F_LAST       0
#define F_AVERAGE    1
#define F_DEVIATION  2

#define OP_LE        0
#define OP_LE_EQ     1
#define OP_EQ        2
#define OP_GT_EQ     3
#define OP_GT        4
#define OP_NE        5
#define OP_LIKE      6
#define OP_NOTLIKE   7


//
// Event policy rule flags
//

#define RF_STOP_PROCESSING    0x0001
#define RF_NEGATED_SOURCE     0x0002
#define RF_NEGATED_EVENTS     0x0004
#define RF_GENERATE_ALARM     0x0008
#define RF_DISABLED           0x0010
#define RF_SEVERITY_INFO      0x0100
#define RF_SEVERITY_WARNING   0x0200
#define RF_SEVERITY_MINOR     0x0400
#define RF_SEVERITY_MAJOR     0x0800
#define RF_SEVERITY_CRITICAL  0x1000


//
// Action types
//

#define ACTION_EXEC           0
#define ACTION_REMOTE         1
#define ACTION_SEND_EMAIL     2
#define ACTION_SEND_SMS       3


//
// Action structure
//

typedef struct
{
   DWORD dwId;
   int iType;
   BOOL bIsDisabled;
   TCHAR szName[MAX_OBJECT_NAME];
   TCHAR szRcptAddr[MAX_RCPT_ADDR_LEN];
   TCHAR szEmailSubject[MAX_EMAIL_SUBJECT_LEN];
   TCHAR *pszData;
} NXC_ACTION;


//
// Alarm structure
//

typedef struct
{
   DWORD dwAlarmId;        // Unique alarm ID
   DWORD dwTimeStamp;      // Timestamp in time() format
   DWORD dwSourceObject;   // Source object ID
   DWORD dwSourceEventCode;// Originating event code
   QWORD qwSourceEventId;  // Originating event ID
   TCHAR szMessage[MAX_DB_STRING];
   TCHAR szKey[MAX_DB_STRING];
   WORD wSeverity;         // Alarm's severity
   WORD wIsAck;            // Non-zero if acknowleged
   DWORD dwAckByUser;      // Id of user who acknowleges this alarm (0 for system)
} NXC_ALARM;


//
// Event log record structure
//

typedef struct
{
   QWORD qwEventId;
   DWORD dwTimeStamp;
   DWORD dwEventCode;
   DWORD dwSourceId;
   DWORD dwSeverity;
   TCHAR  szMessage[MAX_EVENT_MSG_LENGTH];
} NXC_EVENT;


//
// Image information
//

typedef struct
{
   DWORD dwId;
   TCHAR szName[MAX_OBJECT_NAME];
   BYTE hash[MD5_DIGEST_SIZE];
} NXC_IMAGE;


//
// Image list
//

typedef struct
{
   DWORD dwNumImages;
   NXC_IMAGE *pImageList;
} NXC_IMAGE_LIST;


//
// Trap parameter mapping entry
//

struct NXC_OID_MAP
{
   DWORD *pdwObjectId;     // Trap OID
   DWORD dwOidLen;         // Trap OID length
   TCHAR szDescription[MAX_DB_STRING];
};


//
// Trap configuration entry
//

struct NXC_TRAP_CFG_ENTRY
{
   DWORD dwId;             // Entry ID
   DWORD *pdwObjectId;     // Trap OID
   DWORD dwOidLen;         // Trap OID length
   DWORD dwEventCode;      // Event code
   DWORD dwNumMaps;        // Number of parameter mappings
   NXC_OID_MAP *pMaps;
   TCHAR szDescription[MAX_DB_STRING];
};


/********************************************************************
 * Following part of this file shouldn't be included by server code *
 ********************************************************************/

#ifndef LIBNXCL_NO_DECLARATIONS


//
// Callbacks data types
//

typedef void (* NXC_EVENT_HANDLER)(DWORD dwEvent, DWORD dwCode, void *pArg);
typedef void (* NXC_DEBUG_CALLBACK)(TCHAR *pMsg);


//
// Event configuration structure
//

typedef struct
{
   DWORD dwCode;
   DWORD dwSeverity;
   DWORD dwFlags;
   TCHAR szName[MAX_EVENT_NAME];
   TCHAR *pszMessage;
   TCHAR *pszDescription;
} NXC_EVENT_TEMPLATE;


//
// Entry in object's ACL
//

typedef struct
{
   DWORD dwUserId;
   DWORD dwAccessRights;
} NXC_ACL_ENTRY;


//
// Network object structure
//

typedef struct
{
   DWORD dwId;          // Unique object's identifier
   int iClass;          // Object's class
   int iStatus;         // Object's status
   DWORD dwIpAddr;      // IP address
   TCHAR szName[MAX_OBJECT_NAME];
   DWORD dwNumParents;
   DWORD *pdwParentList;
   DWORD dwNumChilds;
   DWORD *pdwChildList;
   BOOL bIsDeleted;     // TRUE for deleted objects
   BOOL bInheritRights;
   DWORD dwAclSize;
   NXC_ACL_ENTRY *pAccessList;
   DWORD dwImage;       // Image ID or IMG_DEFAULT for default image
   union
   {
      struct
      {
         DWORD dwIpNetMask;   // Ip netmask.
         DWORD dwIfIndex;     // Interface index.
         DWORD dwIfType;      // Interface type
         BYTE bMacAddr[MAC_ADDR_LENGTH];
      } iface;
      struct
      {
         DWORD dwFlags;
         DWORD dwDiscoveryFlags;
         DWORD dwNodeType;
         TCHAR szSharedSecret[MAX_SECRET_LENGTH];
         TCHAR szCommunityString[MAX_COMMUNITY_LENGTH];
         TCHAR szObjectId[MAX_OID_LENGTH];
         WORD wAgentPort;     // Listening TCP port for native agent
         WORD wAuthMethod;    // Native agent's authentication method
         TCHAR *pszDescription;
         WORD wSNMPVersion;
      } node;
      struct
      {
         DWORD dwIpNetMask;
      } subnet;
      struct
      {
         DWORD dwCategory;
         TCHAR *pszDescription;
      } container;
      struct
      {
         DWORD dwVersion;
         TCHAR *pszDescription;
      } dct;
   };
} NXC_OBJECT;


//
// Structure with data for NXCModifyObject
//

typedef struct
{
   DWORD dwObjectId;
   DWORD dwFlags;
   TCHAR *pszName;
   TCHAR *pszDescription;
   int iAgentPort;
   int iAuthType;
   TCHAR *pszSecret;
   TCHAR *pszCommunity;
   BOOL bInheritRights;
   DWORD dwImage;
   DWORD dwAclSize;
   NXC_ACL_ENTRY *pAccessList;
   WORD wSNMPVersion;
} NXC_OBJECT_UPDATE;


//
// NetXMS user information structure
//

typedef struct
{
   TCHAR szName[MAX_USER_NAME];
   DWORD dwId;
   WORD wFlags;
   WORD wSystemRights;
   DWORD dwNumMembers;     // Only for groups
   DWORD *pdwMemberList;   // Only for groups
   TCHAR szFullName[MAX_USER_FULLNAME];    // Only for users
   TCHAR szDescription[MAX_USER_DESCR];
} NXC_USER;


//
// Data collection item threshold structure
//

typedef struct
{
   DWORD dwId;
   DWORD dwEvent;
   WORD wFunction;
   WORD wOperation;
   DWORD dwArg1;
   DWORD dwArg2;
   union
   {
      DWORD dwInt32;
      INT64 qwInt64;
      double dFloat;
      TCHAR szString[MAX_STRING_VALUE];
   } value;
} NXC_DCI_THRESHOLD;


//
// Data collection item structure
//

typedef struct
{
   DWORD dwId;
   DWORD dwTemplateId;
   TCHAR szName[MAX_ITEM_NAME];
   TCHAR szDescription[MAX_DB_STRING];
   TCHAR szInstance[MAX_DB_STRING];
   int iPollingInterval;
   int iRetentionTime;
   BYTE iSource;
   BYTE iDataType;
   BYTE iStatus;
   BYTE iDeltaCalculation;
   DWORD dwNumThresholds;
   NXC_DCI_THRESHOLD *pThresholdList;
   TCHAR *pszFormula;
} NXC_DCI;


//
// Node's data collection list information
//

typedef struct
{
   DWORD dwNodeId;
   DWORD dwNumItems;
   NXC_DCI *pItems;
} NXC_DCI_LIST;


//
// Row structure
//

#pragma pack(1)

typedef struct
{
   DWORD dwTimeStamp;
   union
   {
      DWORD dwInt32;
      INT64 qwInt64;
      double dFloat;
      TCHAR szString[MAX_STRING_VALUE];
   } value;
} NXC_DCI_ROW;

#pragma pack()


//
// DCI's collected data
//

typedef struct
{
   DWORD dwNodeId;      // Owner's node id
   DWORD dwItemId;      // Item id
   DWORD dwNumRows;     // Number of rows in set
   WORD wDataType;      // Data type (integer, string, etc.)
   WORD wRowSize;       // Size of single row in bytes
   NXC_DCI_ROW *pRows;  // Array of rows
} NXC_DCI_DATA;


//
// MIB file list
//

typedef struct
{
   DWORD dwNumFiles;
   TCHAR **ppszName;
   BYTE **ppHash;
} NXC_MIB_LIST;


//
// Event processing policy rule
//

typedef struct
{
   DWORD dwFlags;
   DWORD dwId;
   DWORD dwNumActions;
   DWORD dwNumEvents;
   DWORD dwNumSources;
   DWORD *pdwActionList;
   DWORD *pdwEventList;
   DWORD *pdwSourceList;
   TCHAR *pszComment;
   TCHAR szAlarmKey[MAX_DB_STRING];
   TCHAR szAlarmAckKey[MAX_DB_STRING];
   TCHAR szAlarmMessage[MAX_DB_STRING];
   WORD wAlarmSeverity;
} NXC_EPP_RULE;


//
// Event processing policy
//

typedef struct
{
   DWORD dwNumRules;
   NXC_EPP_RULE *pRuleList;
} NXC_EPP;


//
// Object creation information structure
//

typedef struct
{
   int iClass;
   DWORD dwParentId;
   TCHAR *pszName;
   union
   {
      struct
      {
         DWORD dwIpAddr;
         DWORD dwNetMask;
      } node;
      struct
      {
         DWORD dwCategory;
         TCHAR *pszDescription;
      } container;
      struct
      {
         TCHAR *pszDescription;
      } templateGroup;
   } cs;
} NXC_OBJECT_CREATE_INFO;


//
// Container category
//

typedef struct
{
   DWORD dwId;
   DWORD dwImageId;
   TCHAR szName[MAX_OBJECT_NAME];
   TCHAR *pszDescription;
} NXC_CONTAINER_CATEGORY;


//
// Container categories list
//

typedef struct
{
   DWORD dwNumElements;
   NXC_CONTAINER_CATEGORY *pElements;
} NXC_CC_LIST;


//
// Functions
//

#ifdef __cplusplus
extern "C" {
#endif

DWORD LIBNXCL_EXPORTABLE NXCGetVersion(void);
const TCHAR LIBNXCL_EXPORTABLE *NXCGetErrorText(DWORD dwError);

BOOL LIBNXCL_EXPORTABLE NXCInitialize(void);
void LIBNXCL_EXPORTABLE NXCShutdown(void);
void LIBNXCL_EXPORTABLE NXCSetCommandTimeout(DWORD dwTimeout);

DWORD LIBNXCL_EXPORTABLE NXCConnect(TCHAR *szServer, TCHAR *szLogin, TCHAR *szPassword);
void LIBNXCL_EXPORTABLE NXCDisconnect(void);
void LIBNXCL_EXPORTABLE NXCSetEventHandler(NXC_EVENT_HANDLER pHandler);
void LIBNXCL_EXPORTABLE NXCSetDebugCallback(NXC_DEBUG_CALLBACK pFunc);

DWORD LIBNXCL_EXPORTABLE NXCSyncObjects(void);
NXC_OBJECT LIBNXCL_EXPORTABLE *NXCFindObjectById(DWORD dwId);
NXC_OBJECT LIBNXCL_EXPORTABLE *NXCFindObjectByName(TCHAR *pszName);
void LIBNXCL_EXPORTABLE NXCEnumerateObjects(BOOL (* pHandler)(NXC_OBJECT *));
NXC_OBJECT LIBNXCL_EXPORTABLE *NXCGetTopologyRootObject(void);
NXC_OBJECT LIBNXCL_EXPORTABLE *NXCGetServiceRootObject(void);
NXC_OBJECT LIBNXCL_EXPORTABLE *NXCGetTemplateRootObject(void);
void LIBNXCL_EXPORTABLE *NXCGetObjectIndex(DWORD *pdwNumObjects);
void LIBNXCL_EXPORTABLE NXCLockObjectIndex(void);
void LIBNXCL_EXPORTABLE NXCUnlockObjectIndex(void);
DWORD LIBNXCL_EXPORTABLE NXCModifyObject(NXC_OBJECT_UPDATE *pData);
DWORD LIBNXCL_EXPORTABLE NXCSetObjectMgmtStatus(DWORD dwObjectId, BOOL bIsManaged);
DWORD LIBNXCL_EXPORTABLE NXCCreateObject(NXC_OBJECT_CREATE_INFO *pCreateInfo, DWORD *pdwObjectId);
DWORD LIBNXCL_EXPORTABLE NXCBindObject(DWORD dwParentObject, DWORD dwChildObject);
DWORD LIBNXCL_EXPORTABLE NXCUnbindObject(DWORD dwParentObject, DWORD dwChildObject);
DWORD LIBNXCL_EXPORTABLE NXCDeleteObject(DWORD dwObject);
DWORD LIBNXCL_EXPORTABLE NXCPollNode(DWORD dwObjectId, int iPollType, 
                                     void (* pCallback)(TCHAR *, void *), void *pArg);
DWORD LIBNXCL_EXPORTABLE NXCWakeUpNode(DWORD dwObjectId);

DWORD LIBNXCL_EXPORTABLE NXCLoadCCList(NXC_CC_LIST **ppList);
void LIBNXCL_EXPORTABLE NXCDestroyCCList(NXC_CC_LIST *pList);

DWORD LIBNXCL_EXPORTABLE NXCSyncEvents(void);
DWORD LIBNXCL_EXPORTABLE NXCLoadEventDB(void);
DWORD LIBNXCL_EXPORTABLE NXCSetEventInfo(NXC_EVENT_TEMPLATE *pArg);
DWORD LIBNXCL_EXPORTABLE NXCGenerateEventCode(DWORD *pdwEventCode);
void LIBNXCL_EXPORTABLE NXCAddEventTemplate(NXC_EVENT_TEMPLATE *pEventTemplate);
void LIBNXCL_EXPORTABLE NXCDeleteEDBRecord(DWORD dwEventCode);
DWORD LIBNXCL_EXPORTABLE NXCDeleteEventTemplate(DWORD dwEventCode);
DWORD LIBNXCL_EXPORTABLE NXCLockEventDB(void);
DWORD LIBNXCL_EXPORTABLE NXCUnlockEventDB(void);
BOOL LIBNXCL_EXPORTABLE NXCGetEventDB(NXC_EVENT_TEMPLATE ***pppTemplateList, DWORD *pdwNumRecords);
const TCHAR LIBNXCL_EXPORTABLE *NXCGetEventName(DWORD dwId);
BOOL LIBNXCL_EXPORTABLE NXCGetEventNameEx(DWORD dwId, TCHAR *pszBuffer, DWORD dwBufSize);
int LIBNXCL_EXPORTABLE NXCGetEventSeverity(DWORD dwId);
BOOL LIBNXCL_EXPORTABLE NXCGetEventText(DWORD dwId, TCHAR *pszBuffer, DWORD dwBufSize);
DWORD LIBNXCL_EXPORTABLE NXCSendEvent(DWORD dwEventCode, DWORD dwObjectId, int iNumArgs, TCHAR **pArgList);

DWORD LIBNXCL_EXPORTABLE NXCLoadUserDB(void);
NXC_USER LIBNXCL_EXPORTABLE *NXCFindUserById(DWORD dwId);
BOOL LIBNXCL_EXPORTABLE NXCGetUserDB(NXC_USER **ppUserList, DWORD *pdwNumUsers);
DWORD LIBNXCL_EXPORTABLE NXCLockUserDB(void);
DWORD LIBNXCL_EXPORTABLE NXCUnlockUserDB(void);
DWORD LIBNXCL_EXPORTABLE NXCCreateUser(TCHAR *pszName, BOOL bIsGroup, DWORD *pdwNewId);
DWORD LIBNXCL_EXPORTABLE NXCDeleteUser(DWORD dwId);
DWORD LIBNXCL_EXPORTABLE NXCModifyUser(NXC_USER *pUserInfo);
DWORD LIBNXCL_EXPORTABLE NXCSetPassword(DWORD dwUserId, TCHAR *pszNewPassword);

DWORD LIBNXCL_EXPORTABLE NXCOpenNodeDCIList(DWORD dwNodeId, NXC_DCI_LIST **ppItemList);
DWORD LIBNXCL_EXPORTABLE NXCCloseNodeDCIList(NXC_DCI_LIST *pItemList);
DWORD LIBNXCL_EXPORTABLE NXCCreateNewDCI(NXC_DCI_LIST *pItemList, DWORD *pdwItemId);
DWORD LIBNXCL_EXPORTABLE NXCUpdateDCI(DWORD dwNodeId, NXC_DCI *pItem);
DWORD LIBNXCL_EXPORTABLE NXCDeleteDCI(NXC_DCI_LIST *pItemList, DWORD dwItemId);
DWORD LIBNXCL_EXPORTABLE NXCSetDCIStatus(DWORD dwNodeId, DWORD dwNumItems, 
                                         DWORD *pdwItemList, int iStatus);
DWORD LIBNXCL_EXPORTABLE NXCCopyDCI(DWORD dwSrcNodeId, DWORD dwDstNodeId, 
                                    DWORD dwNumItems, DWORD *pdwItemList);
DWORD LIBNXCL_EXPORTABLE NXCItemIndex(NXC_DCI_LIST *pItemList, DWORD dwItemId);
DWORD LIBNXCL_EXPORTABLE NXCGetDCIData(DWORD dwNodeId, DWORD dwItemId, DWORD dwMaxRows,
                                       DWORD dwTimeFrom, DWORD dwTimeTo, NXC_DCI_DATA **ppData);
void LIBNXCL_EXPORTABLE NXCDestroyDCIData(NXC_DCI_DATA *pData);
NXC_DCI_ROW LIBNXCL_EXPORTABLE *NXCGetRowPtr(NXC_DCI_DATA *pData, DWORD dwRow);
DWORD LIBNXCL_EXPORTABLE NXCAddThresholdToItem(NXC_DCI *pItem, NXC_DCI_THRESHOLD *pThreshold);
BOOL LIBNXCL_EXPORTABLE NXCDeleteThresholdFromItem(NXC_DCI *pItem, DWORD dwIndex);
BOOL LIBNXCL_EXPORTABLE NXCSwapThresholds(NXC_DCI *pItem, DWORD dwIndex1, DWORD dwIndex2);
DWORD LIBNXCL_EXPORTABLE NXCQueryParameter(DWORD dwNodeId, int iOrigin, TCHAR *pszParameter,
                                           TCHAR *pszBuffer, DWORD dwBufferSize);

DWORD LIBNXCL_EXPORTABLE NXCGetMIBList(NXC_MIB_LIST **ppMibList);
void LIBNXCL_EXPORTABLE NXCDestroyMIBList(NXC_MIB_LIST *pMibList);
DWORD LIBNXCL_EXPORTABLE NXCDownloadMIBFile(TCHAR *pszName, TCHAR *pszDestDir);

DWORD LIBNXCL_EXPORTABLE NXCOpenEventPolicy(NXC_EPP **ppEventPolicy);
DWORD LIBNXCL_EXPORTABLE NXCCloseEventPolicy(void);
DWORD LIBNXCL_EXPORTABLE NXCSaveEventPolicy(NXC_EPP *pEventPolicy);
void LIBNXCL_EXPORTABLE NXCDestroyEventPolicy(NXC_EPP *pEventPolicy);
void LIBNXCL_EXPORTABLE NXCDeletePolicyRule(NXC_EPP *pEventPolicy, DWORD dwRule);

DWORD LIBNXCL_EXPORTABLE NXCSyncImages(NXC_IMAGE_LIST **ppImageList, TCHAR *pszCacheDir, WORD wFormat);
DWORD LIBNXCL_EXPORTABLE NXCLoadImageFile(DWORD dwImageId, TCHAR *pszCacheDir, WORD wFormat);
void LIBNXCL_EXPORTABLE NXCDestroyImageList(NXC_IMAGE_LIST *pImageList);
DWORD LIBNXCL_EXPORTABLE NXCLoadDefaultImageList(DWORD *pdwListSize,
                                                 DWORD **ppdwClassId, DWORD **ppdwImageId);

DWORD LIBNXCL_EXPORTABLE NXCLoadAllAlarms(BOOL bIncludeAck, DWORD *pdwNumAlarms, NXC_ALARM **ppAlarmList);
DWORD LIBNXCL_EXPORTABLE NXCAcknowlegeAlarm(DWORD dwAlarmId);
DWORD LIBNXCL_EXPORTABLE NXCDeleteAlarm(DWORD dwAlarmId);

DWORD LIBNXCL_EXPORTABLE NXCLoadActions(DWORD *pdwNumActions, NXC_ACTION **ppActionList);
DWORD LIBNXCL_EXPORTABLE NXCLockActionDB(void);
DWORD LIBNXCL_EXPORTABLE NXCUnlockActionDB(void);
DWORD LIBNXCL_EXPORTABLE NXCCreateAction(TCHAR *pszName, DWORD *pdwNewId);
DWORD LIBNXCL_EXPORTABLE NXCModifyAction(NXC_ACTION *pAction);
DWORD LIBNXCL_EXPORTABLE NXCDeleteAction(DWORD dwActionId);

DWORD LIBNXCL_EXPORTABLE NXCLockTrapCfg(void);
DWORD LIBNXCL_EXPORTABLE NXCUnlockTrapCfg(void);
DWORD LIBNXCL_EXPORTABLE NXCLoadTrapCfg(DWORD *pdwNumTraps, NXC_TRAP_CFG_ENTRY **ppTrapList);
void LIBNXCL_EXPORTABLE NXCDestroyTrapList(DWORD dwNumTraps, NXC_TRAP_CFG_ENTRY *pTrapList);
DWORD LIBNXCL_EXPORTABLE NXCCreateTrap(DWORD *pdwTrapId);
DWORD LIBNXCL_EXPORTABLE NXCModifyTrap(NXC_TRAP_CFG_ENTRY *pTrap);
DWORD LIBNXCL_EXPORTABLE NXCDeleteTrap(DWORD dwTrapId);

#ifdef __cplusplus
}
#endif

#endif   /* LIBNXCL_NO_DECLARATIONS */

#endif   /* _nxclapi_h_ */
