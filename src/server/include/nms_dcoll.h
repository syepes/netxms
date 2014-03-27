/* 
** NetXMS - Network Management System
** Copyright (C) 2003-2013 Victor Kirhenshtein
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
** File: nms_dcoll.h
**
**/

#ifndef _nms_dcoll_h_
#define _nms_dcoll_h_


//
// Data collection errors
//

#define DCE_SUCCESS        0
#define DCE_COMM_ERROR     1
#define DCE_NOT_SUPPORTED  2
#define DCE_IGNORE         3

/**
 * Threshold check results
 */
enum ThresholdCheckResult
{
   ACTIVATED = 0,
   DEACTIVATED = 1,
   ALREADY_ACTIVE = 2,
   ALREADY_INACTIVE = 3
};

/**
 * DCI value
 */
class NXCORE_EXPORTABLE ItemValue
{
private:
   double m_dFloat;
   INT32 m_iInt32;
   INT64 m_iInt64;
   UINT32 m_dwInt32;
   UINT64 m_qwInt64;
   TCHAR m_szString[MAX_DB_STRING];
   UINT32 m_dwTimeStamp;

public:
   ItemValue();
   ItemValue(const TCHAR *pszValue, UINT32 dwTimeStamp);
   ItemValue(const ItemValue *pValue);
   ~ItemValue();

   void setTimeStamp(UINT32 dwTime) { m_dwTimeStamp = dwTime; }
   UINT32 getTimeStamp() { return m_dwTimeStamp; }

   const TCHAR *getString() { return m_szString; }

   operator double() { return m_dFloat; }
   operator UINT32() { return m_dwInt32; }
   operator UINT64() { return m_qwInt64; }
   operator INT32() { return m_iInt32; }
   operator INT64() { return m_iInt64; }
   operator const TCHAR*() const { return m_szString; }

   const ItemValue& operator=(const ItemValue &src);
   const ItemValue& operator=(const TCHAR *pszStr);
   const ItemValue& operator=(double dFloat);
   const ItemValue& operator=(INT32 iInt32);
   const ItemValue& operator=(INT64 iInt64);
   const ItemValue& operator=(UINT32 dwInt32);
   const ItemValue& operator=(UINT64 qwInt64);
};


class DCItem;
class DataCollectionTarget;

/**
 * Threshold definition class
 */
class NXCORE_EXPORTABLE Threshold
{
private:
   UINT32 m_id;             // Unique threshold id
   UINT32 m_itemId;         // Parent item id
   UINT32 m_targetId;       // Parent data collection target ID
   UINT32 m_eventCode;      // Event code to be generated
   UINT32 m_rearmEventCode;
   ItemValue m_value;
   BYTE m_function;          // Function code
   BYTE m_operation;         // Comparision operation code
   BYTE m_dataType;          // Related item data type
	BYTE m_currentSeverity;   // Current everity (NORMAL if threshold is inactive)
   int m_sampleCount;        // Number of samples to calculate function on
   TCHAR *m_scriptSource;
   NXSL_Program *m_script;
   BOOL m_isReached;
	int m_numMatches;			// Number of consecutive matches
	int m_repeatInterval;		// -1 = default, 0 = off, >0 = seconds between repeats
	time_t m_lastEventTimestamp;

   const ItemValue& value() { return m_value; }
   void calculateAverageValue(ItemValue *pResult, ItemValue &lastValue, ItemValue **ppPrevValues);
   void calculateSumValue(ItemValue *pResult, ItemValue &lastValue, ItemValue **ppPrevValues);
   void calculateMDValue(ItemValue *pResult, ItemValue &lastValue, ItemValue **ppPrevValues);
   void calculateDiff(ItemValue *pResult, ItemValue &lastValue, ItemValue **ppPrevValues);
   void setScript(TCHAR *script);

public:
	Threshold();
   Threshold(DCItem *pRelatedItem);
   Threshold(Threshold *pSrc);
   Threshold(DB_RESULT hResult, int iRow, DCItem *pRelatedItem);
	Threshold(ConfigEntry *config, DCItem *parentItem);
   ~Threshold();

   void bindToItem(UINT32 itemId, UINT32 targetId) { m_itemId = itemId; m_targetId = targetId; }

   UINT32 getId() { return m_id; }
   UINT32 getEventCode() { return m_eventCode; }
   UINT32 getRearmEventCode() { return m_rearmEventCode; }
	int getFunction() { return m_function; }
	int getOperation() { return m_operation; }
	int getSampleCount() { return m_sampleCount; }
   const TCHAR *getStringValue() { return m_value.getString(); }
   BOOL isReached() { return m_isReached; }
	
	int getRepeatInterval() { return m_repeatInterval; }
	time_t getLastEventTimestamp() { return m_lastEventTimestamp; }
	int getCurrentSeverity() { return m_currentSeverity; }
	void markLastEvent(int severity);

   BOOL saveToDB(DB_HANDLE hdb, UINT32 dwIndex);
   ThresholdCheckResult check(ItemValue &value, ItemValue **ppPrevValues, ItemValue &fvalue, NetObj *target, DCItem *dci);
   ThresholdCheckResult checkError(UINT32 dwErrorCount);

   void createMessage(CSCPMessage *msg, UINT32 baseId);
   void updateFromMessage(CSCPMessage *msg, UINT32 baseId);

   void createId();
   UINT32 getRequiredCacheSize() { return ((m_function == F_LAST) || (m_function == F_ERROR)) ? 0 : m_sampleCount; }

   BOOL compare(Threshold *pThr);

   void createNXMPRecord(String &str, int index);

	void associate(DCItem *pItem);
	void setFunction(int nFunc) { m_function = nFunc; }
	void setOperation(int nOp) { m_operation = nOp; }
	void setEvent(UINT32 dwEvent) { m_eventCode = dwEvent; }
	void setRearmEvent(UINT32 dwEvent) { m_rearmEventCode = dwEvent; }
	void setSampleCount(int nVal) { m_sampleCount = nVal; }
	void setValue(const TCHAR *value) { m_value = value; }
};

/**
 * Table threshold definition
 */
class NXCORE_EXPORTABLE TableThreshold
{
private:

public:

};

class Template;

/**
 * Generic data collection object
 */
class NXCORE_EXPORTABLE DCObject
{
protected:
   UINT32 m_dwId;
   TCHAR m_szName[MAX_ITEM_NAME];
   TCHAR m_szDescription[MAX_DB_STRING];
	TCHAR m_systemTag[MAX_DB_STRING];
   time_t m_tLastPoll;           // Last poll time
   int m_iPollingInterval;       // Polling interval in seconds
   int m_iRetentionTime;         // Retention time in seconds
   BYTE m_source;                // origin: SNMP, agent, etc.
   BYTE m_status;                // Item status: active, disabled or not supported
   BYTE m_busy;                  // 1 when item is queued for polling, 0 if not
	BYTE m_scheduledForDeletion;  // 1 when item is scheduled for deletion, 0 if not
	WORD m_flags;
   UINT32 m_dwTemplateId;         // Related template's id
   UINT32 m_dwTemplateItemId;     // Related template item's id
   Template *m_pNode;             // Pointer to node or template object this item related to
   MUTEX m_hMutex;
   UINT32 m_dwNumSchedules;
   TCHAR **m_ppScheduleList;
   time_t m_tLastCheck;          // Last schedule checking time
   UINT32 m_dwErrorCount;         // Consequtive collection error count
	UINT32 m_dwResourceId;	   	// Associated cluster resource ID
	UINT32 m_dwProxyNode;          // Proxy node ID or 0 to disable
	WORD m_snmpPort;					// Custom SNMP port or 0 for node default
	TCHAR *m_pszPerfTabSettings;
   TCHAR *m_transformationScriptSource;   // Transformation script (source code)
   NXSL_Program *m_transformationScript;  // Compiled transformation script

   void lock() { MutexLock(m_hMutex); }
   void unlock() { MutexUnlock(m_hMutex); }

	BOOL loadCustomSchedules();
   bool matchSchedule(struct tm *pCurrTime, TCHAR *pszSchedule, BOOL *bWithSeconds, time_t currTimestamp);

	void expandMacros(const TCHAR *src, TCHAR *dst, size_t dstLen);

	virtual bool isCacheLoaded();

	// --- constructors ---
   DCObject();
   DCObject(const DCObject *src);
   DCObject(UINT32 dwId, const TCHAR *szName, int iSource, int iPollingInterval, int iRetentionTime, Template *pNode,
            const TCHAR *pszDescription = NULL, const TCHAR *systemTag = NULL);
	DCObject(ConfigEntry *config, Template *owner);

public:
	virtual ~DCObject();

	virtual int getType() const { return DCO_TYPE_GENERIC; }

   virtual void updateFromTemplate(DCObject *dcObject);

   virtual BOOL saveToDB(DB_HANDLE hdb);
   virtual void deleteFromDB();
   virtual bool loadThresholdsFromDB();

   virtual void processNewValue(time_t nTimeStamp, void *value);
   virtual void processNewError();

	virtual bool hasValue();

	UINT32 getId() { return m_dwId; }
   int getDataSource() { return m_source; }
   int getStatus() { return m_status; }
   const TCHAR *getName() { return m_szName; }
   const TCHAR *getDescription() { return m_szDescription; }
	const TCHAR *getSystemTag() { return m_systemTag; }
	const TCHAR *getPerfTabSettings() { return m_pszPerfTabSettings; }
   Template *getTarget() { return m_pNode; }
   UINT32 getTemplateId() { return m_dwTemplateId; }
   UINT32 getTemplateItemId() { return m_dwTemplateItemId; }
	UINT32 getResourceId() { return m_dwResourceId; }
	UINT32 getProxyNode() { return m_dwProxyNode; }
	time_t getLastPollTime() { return m_tLastPoll; }
	UINT32 getErrorCount() { return m_dwErrorCount; }
	WORD getSnmpPort() { return m_snmpPort; }
   bool isShowOnObjectTooltip() { return (m_flags & DCF_SHOW_ON_OBJECT_TOOLTIP) ? true : false; }
   bool isAggregateOnCluster() { return (m_flags & DCF_AGGREGATE_ON_CLUSTER) ? true : false; }
   int getAggregationFunction() { return DCF_GET_AGGREGATION_FUNCTION(m_flags); }

	bool matchClusterResource();
   bool isReadyForPolling(time_t currTime);
	bool isScheduledForDeletion() { return m_scheduledForDeletion ? true : false; }
   void setLastPollTime(time_t tLastPoll) { m_tLastPoll = tLastPoll; }
   void setStatus(int status, bool generateEvent);
   void setBusyFlag(BOOL busy) { m_busy = (BYTE)busy; }
   void setTemplateId(UINT32 dwTemplateId, UINT32 dwItemId) 
         { m_dwTemplateId = dwTemplateId; m_dwTemplateItemId = dwItemId; }

   virtual void createMessage(CSCPMessage *pMsg);
   virtual void updateFromMessage(CSCPMessage *pMsg);

   virtual void changeBinding(UINT32 dwNewId, Template *pNode, BOOL doMacroExpansion);

	virtual void deleteExpiredData();
	virtual bool deleteAllData();

   virtual void getEventList(UINT32 **ppdwList, UINT32 *pdwSize);
   virtual void createNXMPRecord(String &str);

	void setName(const TCHAR *pszName) { nx_strncpy(m_szName, pszName, MAX_ITEM_NAME); }
	void setDescription(const TCHAR *pszDescr) { nx_strncpy(m_szDescription, pszDescr, MAX_DB_STRING); }
	void setOrigin(int origin) { m_source = origin; }
	void setRetentionTime(int nTime) { m_iRetentionTime = nTime; }
	void setInterval(int nInt) { m_iPollingInterval = nInt; }
	void setAdvScheduleFlag(BOOL bFlag) { if (bFlag) m_flags |= DCF_ADVANCED_SCHEDULE; else m_flags &= ~DCF_ADVANCED_SCHEDULE; }
	void addSchedule(const TCHAR *pszSchedule);
   void setTransformationScript(const TCHAR *pszScript);

	bool prepareForDeletion();
};

/**
 * Data collection item class
 */
class NXCORE_EXPORTABLE DCItem : public DCObject
{
protected:
   TCHAR m_instance[MAX_DB_STRING];
   BYTE m_deltaCalculation;      // Delta calculation method
   BYTE m_dataType;
	int m_sampleCount;            // Number of samples required to calculate value
	ObjectArray<Threshold> *m_thresholds;
   UINT32 m_dwCacheSize;          // Number of items in cache
   ItemValue **m_ppValueCache;
   ItemValue m_prevRawValue;     // Previous raw value (used for delta calculation)
   time_t m_tPrevValueTimeStamp;
   bool m_bCacheLoaded;
	int m_nBaseUnits;
	int m_nMultiplier;
	TCHAR *m_customUnitName;
	WORD m_snmpRawValueType;		// Actual SNMP raw value type for input transformation
	WORD m_instanceDiscoveryMethod;
	TCHAR *m_instanceDiscoveryData;
	TCHAR *m_instanceFilterSource;
	NXSL_Program *m_instanceFilter;

   void transform(ItemValue &value, time_t nElapsedTime);
   void checkThresholds(ItemValue &value);
   void clearCache();

	virtual bool isCacheLoaded();

public:
   DCItem();
   DCItem(const DCItem *pItem);
   DCItem(DB_RESULT hResult, int iRow, Template *pNode);
   DCItem(UINT32 dwId, const TCHAR *szName, int iSource, int iDataType, 
          int iPollingInterval, int iRetentionTime, Template *pNode,
          const TCHAR *pszDescription = NULL, const TCHAR *systemTag = NULL);
	DCItem(ConfigEntry *config, Template *owner);
   virtual ~DCItem();

	virtual int getType() const { return DCO_TYPE_ITEM; }

   virtual void updateFromTemplate(DCObject *dcObject);

   virtual BOOL saveToDB(DB_HANDLE hdb);
   virtual void deleteFromDB();
   virtual bool loadThresholdsFromDB();

   void updateCacheSize(UINT32 dwCondId = 0);

   int getDataType() { return m_dataType; }
	bool isInterpretSnmpRawValue() { return (m_flags & DCF_RAW_VALUE_OCTET_STRING) ? true : false; }
	WORD getSnmpRawValueType() { return m_snmpRawValueType; }
	bool hasActiveThreshold();
   int getThresholdSeverity();
	WORD getInstanceDiscoveryMethod() { return m_instanceDiscoveryMethod; }
	const TCHAR *getInstanceDiscoveryData() { return m_instanceDiscoveryData; }
	NXSL_Program *getInstanceFilter() { return m_instanceFilter; }
	const TCHAR *getInstance() { return m_instance; }
	int getSampleCount() { return m_sampleCount; }

	void filterInstanceList(StringList *instances);
	void expandInstance();

   void processNewValue(time_t nTimeStamp, void *value);
   void processNewError();

	virtual bool hasValue();

   void fillLastValueMessage(CSCPMessage *pMsg, UINT32 dwId);
   NXSL_Value *getValueForNXSL(int nFunction, int nPolls);
   NXSL_Value *getRawValueForNXSL();
   const TCHAR *getLastValue();
   ItemValue *getInternalLastValue();

   virtual void createMessage(CSCPMessage *pMsg);
#if defined(__SUNPRO_CC) || defined(__HP_aCC)
   using DCObject::updateFromMessage;
#endif
   void updateFromMessage(CSCPMessage *pMsg, UINT32 *pdwNumMaps, UINT32 **ppdwMapIndex, UINT32 **ppdwMapId);
   void fillMessageWithThresholds(CSCPMessage *msg);

   virtual void changeBinding(UINT32 dwNewId, Template *pNode, BOOL doMacroExpansion);

   virtual void deleteExpiredData();
	virtual bool deleteAllData();

   virtual void getEventList(UINT32 **ppdwList, UINT32 *pdwSize);
   virtual void createNXMPRecord(String &str);

	int getThresholdCount() const { return (m_thresholds != NULL) ? m_thresholds->size() : 0; }
	BOOL enumThresholds(BOOL (* pfCallback)(Threshold *, UINT32, void *), void *pArg);

	void setInstance(const TCHAR *instance) { nx_strncpy(m_instance, instance, MAX_DB_STRING); }
	void setDataType(int dataType) { m_dataType = dataType; }
	void setDeltaCalcMethod(int method) { m_deltaCalculation = method; }
	void setAllThresholdsFlag(BOOL bFlag) { if (bFlag) m_flags |= DCF_ALL_THRESHOLDS; else m_flags &= ~DCF_ALL_THRESHOLDS; }
	void addThreshold(Threshold *pThreshold);
	void deleteAllThresholds();
	void setInstanceDiscoveryMethod(WORD method) { m_instanceDiscoveryMethod = method; }
	void setInstanceDiscoveryData(const TCHAR *data) { safe_free(m_instanceDiscoveryData); m_instanceDiscoveryData = (data != NULL) ? _tcsdup(data) : NULL; }
   void setInstanceFilter(const TCHAR *pszScript);

	static bool testTransformation(Node *node, const TCHAR *script, const TCHAR *value, TCHAR *buffer, size_t bufSize);
};

/**
 * Table column definition
 */
class NXCORE_EXPORTABLE DCTableColumn
{
private:
	TCHAR m_name[MAX_COLUMN_NAME];
   TCHAR *m_displayName;
	SNMP_ObjectId *m_snmpOid;
	UINT16 m_flags;

public:
	DCTableColumn(const DCTableColumn *src);
	DCTableColumn(CSCPMessage *msg, UINT32 baseId);
	DCTableColumn(DB_RESULT hResult, int row);
   DCTableColumn(ConfigEntry *e);
	~DCTableColumn();

	const TCHAR *getName() { return m_name; }
   const TCHAR *getDisplayName() { return (m_displayName != NULL) ? m_displayName : m_name; }
   UINT16 getFlags() { return m_flags; }
   int getDataType() { return TCF_GET_DATA_TYPE(m_flags); }
   int getAggregationFunction() { return TCF_GET_AGGREGATION_FUNCTION(m_flags); }
	SNMP_ObjectId *getSnmpOid() { return m_snmpOid; }
   bool isInstanceColumn() { return (m_flags & TCF_INSTANCE_COLUMN) != 0; }

   void createNXMPRecord(String &str, int id);
};

/**
 * Table column ID hash entry
 */
struct TC_ID_MAP_ENTRY
{
	INT32 id;
	TCHAR name[MAX_COLUMN_NAME];
};

/**
 * Condition for table DCI threshold
 */
class NXCORE_EXPORTABLE DCTableCondition
{
private:
   TCHAR *m_column;
   int m_operation;
   ItemValue m_value;

public:
   DCTableCondition(const TCHAR *column, int operation, const TCHAR *value);
   DCTableCondition(DCTableCondition *src);
   ~DCTableCondition();

   bool check(Table *value, int row);

   const TCHAR *getColumn() { return m_column; }
   int getOperation() { return m_operation; }
   const TCHAR *getValue() { return m_value.getString(); }
};

/**
 * Condition group for table DCI threshold
 */
class NXCORE_EXPORTABLE DCTableConditionGroup
{
private:
   ObjectArray<DCTableCondition> *m_conditions;

public:
   DCTableConditionGroup();
   DCTableConditionGroup(CSCPMessage *msg, UINT32 *baseId);
   DCTableConditionGroup(DCTableConditionGroup *src);
   DCTableConditionGroup(ConfigEntry *e);
   ~DCTableConditionGroup();

   bool check(Table *value, int row);

   UINT32 fillMessage(CSCPMessage *msg, UINT32 baseId);

   ObjectArray<DCTableCondition> *getConditions() { return m_conditions; }
};

/**
 * Threshold definition for tabe DCI
 */
class NXCORE_EXPORTABLE DCTableThreshold
{
private:
   UINT32 m_id;
   ObjectArray<DCTableConditionGroup> *m_groups;
   UINT32 m_activationEvent;
   UINT32 m_deactivationEvent;
   StringSet *m_activeKeys;

   void loadConditions();

public:
   DCTableThreshold();
   DCTableThreshold(DB_RESULT hResult, int row);
   DCTableThreshold(CSCPMessage *msg, UINT32 *baseId);
   DCTableThreshold(DCTableThreshold *src);
   DCTableThreshold(ConfigEntry *e);
   ~DCTableThreshold();

   void copyState(DCTableThreshold *src);

   ThresholdCheckResult check(Table *value, int row, const TCHAR *instance);

   bool saveToDatabase(DB_HANDLE hdb, UINT32 tableId, int seq);
   UINT32 fillMessage(CSCPMessage *msg, UINT32 baseId);
   void createNXMPRecord(String &str, int id);

   UINT32 getId() { return m_id; }
   UINT32 getActivationEvent() { return m_activationEvent; }
   UINT32 getDeactivationEvent() { return m_deactivationEvent; }
};

/**
 * Tabular data collection object
 */
class NXCORE_EXPORTABLE DCTable : public DCObject
{
protected:
	ObjectArray<DCTableColumn> *m_columns;
   ObjectArray<DCTableThreshold> *m_thresholds;
	Table *m_lastValue;

	static TC_ID_MAP_ENTRY *m_cache;
	static int m_cacheSize;
	static int m_cacheAllocated;
	static MUTEX m_cacheMutex;

   void transform(Table *value);
   void checkThresholds(Table *value);
   
   bool loadThresholds();
   bool saveThresholds(DB_HANDLE hdb);

public:
	DCTable();
   DCTable(const DCTable *src);
   DCTable(UINT32 id, const TCHAR *name, int source, int pollingInterval, int retentionTime,
	        Template *node, const TCHAR *description = NULL, const TCHAR *systemTag = NULL);
   DCTable(DB_RESULT hResult, int iRow, Template *pNode);
   DCTable(ConfigEntry *config, Template *owner);
	virtual ~DCTable();

	virtual int getType() const { return DCO_TYPE_TABLE; }

   virtual void updateFromTemplate(DCObject *dcObject);

   virtual BOOL saveToDB(DB_HANDLE hdb);
   virtual void deleteFromDB();

   virtual void processNewValue(time_t nTimeStamp, void *value);
   virtual void processNewError();

   virtual void createMessage(CSCPMessage *pMsg);
   virtual void updateFromMessage(CSCPMessage *pMsg);

	virtual void deleteExpiredData();
	virtual bool deleteAllData();

   virtual void createNXMPRecord(String &str);

	void fillLastValueMessage(CSCPMessage *msg);
   void fillLastValueSummaryMessage(CSCPMessage *pMsg, UINT32 dwId);

   int getColumnDataType(const TCHAR *name);
   ObjectArray<DCTableColumn> *getColumns() { return m_columns; }
   Table *getLastValue();

   void mergeValues(Table *dest, Table *src, int count);

   void updateResultColumns(Table *t);

	static INT32 columnIdFromName(const TCHAR *name);
};

/**
 * Functions
 */
BOOL InitDataCollector();
void DeleteAllItemsForNode(UINT32 dwNodeId);
void WriteFullParamListToMessage(CSCPMessage *pMsg, WORD flags);
int GetDCObjectType(UINT32 nodeId, UINT32 dciId);

void CalculateItemValueDiff(ItemValue &result, int nDataType, ItemValue &value1, ItemValue &value2);
void CalculateItemValueAverage(ItemValue &result, int nDataType, int nNumValues, ItemValue **ppValueList);
void CalculateItemValueMD(ItemValue &result, int nDataType, int nNumValues, ItemValue **ppValueList);
void CalculateItemValueTotal(ItemValue &result, int nDataType, int nNumValues, ItemValue **ppValueList);
void CalculateItemValueMin(ItemValue &result, int nDataType, int nNumValues, ItemValue **ppValueList);
void CalculateItemValueMax(ItemValue &result, int nDataType, int nNumValues, ItemValue **ppValueList);

/**
 * Global variables
 */
extern double g_dAvgPollerQueueSize;
extern double g_dAvgDBWriterQueueSize;
extern double g_dAvgIDataWriterQueueSize;
extern double g_dAvgDBAndIDataWriterQueueSize;
extern double g_dAvgStatusPollerQueueSize;
extern double g_dAvgConfigPollerQueueSize;
extern UINT32 g_dwAvgDCIQueuingTime;


#endif   /* _nms_dcoll_h_ */
