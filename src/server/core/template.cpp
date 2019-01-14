/*
** NetXMS - Network Management System
** Copyright (C) 2003-2018 Victor Kirhenshtein
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
** File: template.cpp
**
**/

#include "nxcore.h"

/**
 * Redefined status calculation for template group
 */
void TemplateGroup::calculateCompoundStatus(BOOL bForcedRecalc)
{
   m_status = STATUS_NORMAL;
}

/**
 * Called by client session handler to check if threshold summary should be shown for this object.
 */
bool TemplateGroup::showThresholdSummary()
{
   return false;
}

static GenericAgentPolicy *CreatePolicy(uuid guid, UINT32 m_id, const TCHAR *type)
{
   GenericAgentPolicy *obj;
   if(!_tcscmp(type, _T("AgentConfig")) || !_tcscmp(type, _T("LogParserConfig")))
      obj = new GenericAgentPolicy(guid, m_id);
   return obj;
}

static GenericAgentPolicy *CreatePolicy(const TCHAR *name,const TCHAR *type, UINT32 m_id)
{
   GenericAgentPolicy *obj;
   if(!_tcscmp(type, _T("AgentConfig")) || !_tcscmp(type, _T("LogParserConfig")))
      obj = new GenericAgentPolicy(name, type, m_id);
   return obj;
}

Template::Template() : super(), AutoBindTarget(this), VersionableObject(this)
{
   m_policyList = new HashMap<uuid, GenericAgentPolicy>(true);
}

/**
 * Create template object from import file
 */
Template::Template(ConfigEntry *config) : super(config), AutoBindTarget(this, config), VersionableObject(this, config)
{
   UINT32 flags = config->getSubEntryValueAsUInt(_T("flags"), 0, 0);
   if(flags != 0)
   {
      if(flags & AAF_AUTO_APPLY)
         m_autoBindFlag = true;
      if(flags & AAF_AUTO_REMOVE)
         m_autoUnbindFlag = true;
      flags &= !(AAF_AUTO_APPLY | AAF_AUTO_REMOVE);
   }

   m_policyList = new HashMap<uuid, GenericAgentPolicy>();
   ObjectArray<ConfigEntry> *dcis = config->getSubEntries(_T("agentPolicy#*"));
   for(int i = 0; i < dcis->size(); i++)
   {
      ConfigEntry *e = dcis->get(i);
      uuid guid = e->getSubEntryValueAsUUID(_T("guid"));
      GenericAgentPolicy *curr = CreatePolicy(guid, m_id, e->getSubEntryValue(_T("policyType")));
      curr->updateFromImport(e);
      m_policyList->set(guid, curr);
   }
   delete dcis;

   nx_strncpy(m_name, config->getSubEntryValue(_T("name"), 0, _T("Unnamed Template")), MAX_OBJECT_NAME);
}

Template::Template(const TCHAR *pszName) : super(pszName), AutoBindTarget(this), VersionableObject(this)
{
   m_policyList = new HashMap<uuid, GenericAgentPolicy>();
}

Template::~Template()
{
   delete m_policyList;
}

/**
 * Save template object to database
 */
bool Template::saveToDatabase(DB_HANDLE hdb)
{
   bool success = super::saveToDatabase(hdb);
   if(success)
   {
      if ((m_modified & MODIFY_OTHER) && !IsDatabaseRecordExist(hdb, _T("templates"), _T("id"), m_id))
      {
         DB_STATEMENT hStmt = DBPrepare(hdb, _T("INSERT INTO templates (id) VALUES (?)"));
         if (hStmt != NULL)
         {
            DBBind(hStmt, 1, DB_SQLTYPE_INTEGER, m_id);
            success = DBExecute(hStmt);
            DBFreeStatement(hStmt);
         }
      }
   }

   if (success && (m_modified & MODIFY_RELATIONS))
   {
      // Update members list
      success = executeQueryOnObject(hdb, _T("DELETE FROM dct_node_map WHERE template_id=?"));
      lockChildList(false);
      if (success && !m_childList->isEmpty())
      {
         DB_STATEMENT hStmt = DBPrepare(hdb, _T("INSERT INTO dct_node_map (template_id,node_id) VALUES (?,?)"), m_childList->size() > 1);
         if (hStmt != NULL)
         {
            DBBind(hStmt, 1, DB_SQLTYPE_INTEGER, m_id);
            for(int i = 0; success && (i < m_childList->size()); i++)
            {
               DBBind(hStmt, 2, DB_SQLTYPE_INTEGER, m_childList->get(i)->getId());
               success = DBExecute(hStmt);
            }
            DBFreeStatement(hStmt);
         }
         else
         {
            success = false;
         }
      }
      unlockChildList();
   }

   if(success && (m_modified & MODIFY_OTHER))
      success = AutoBindTarget::saveToDatabase(hdb);
   if(success && (m_modified & MODIFY_OTHER))
      success = VersionableObject::saveToDatabase(hdb);

   Iterator<GenericAgentPolicy> *it = m_policyList->iterator();
   while(it->hasNext() && success)
   {
      GenericAgentPolicy *object = it->next();
      success = object->saveToDatabase(hdb);
   }
   delete it;

   // Clear modifications flag
   lockProperties();
   m_modified = 0;
   unlockProperties();

   return success;
}

/**
 * Delete template object from database
 */
bool Template::deleteFromDatabase(DB_HANDLE hdb)
{
   bool success = super::deleteFromDatabase(hdb);
   if(success)
      success = executeQueryOnObject(hdb, _T("DELETE FROM templates WHERE id=?"));
   if (success)
      success = executeQueryOnObject(hdb, _T("DELETE FROM dct_node_map WHERE template_id=?"));
   if(success)
      success = AutoBindTarget::deleteFromDatabase(hdb);
   if(success)
      success = VersionableObject::deleteFromDatabase(hdb);
   if(success)
   {
      Iterator<GenericAgentPolicy> *it = m_policyList->iterator();
      while(it->hasNext() && success)
      {
         GenericAgentPolicy *object = it->next();
         success = object->deleteFromDatabase(hdb);
      }
      delete it;
   }
   return success;
}

/**
 * Load template object from database
 */
bool Template::loadFromDatabase(DB_HANDLE hdb, UINT32 id)
{
   bool success = super::loadFromDatabase(hdb, id);

   if(success)
      success = AutoBindTarget::loadFromDatabase(hdb, id);
   if(success)
      success = VersionableObject::loadFromDatabase(hdb, id);
   if(success)
   {
      DB_STATEMENT hStmt = DBPrepare(hdb, _T("SELECT guid FROM ap_common,policy_type WHERE owner_id=?"));
      if (hStmt != NULL)
      {
         DBBind(hStmt, 1, DB_SQLTYPE_INTEGER, m_id);
         DB_RESULT hResult = DBSelectPrepared(hStmt);
         if (hResult != NULL)
         {
            int count = DBGetNumRows(hResult);
            for(int i = 0; i < count && success; i++)
            {
               TCHAR type[32];
               uuid guid = DBGetFieldGUID(hResult, i, 0);
               DBGetField(hResult, i, 1, type, 32);
               GenericAgentPolicy *curr = CreatePolicy(guid, m_id, type);
               success = curr->loadFromDatabase(hdb);
               m_policyList->set(guid, curr);
            }
            DBFreeResult(hResult);
         }
         else
         {
            success = false;
         }
         DBFreeStatement(hStmt);
      }
      else
      {
         success = false;
      }
   }

   // Load related nodes list
   if (!m_isDeleted)
   {
      TCHAR szQuery[256];
      _sntprintf(szQuery, sizeof(szQuery) / sizeof(TCHAR), _T("SELECT node_id FROM dct_node_map WHERE template_id=%d"), m_id);
      DB_RESULT hResult = DBSelect(hdb, szQuery);
      if (hResult != NULL)
      {
         UINT32 dwNumNodes = DBGetNumRows(hResult);
         for(int i = 0; i < dwNumNodes; i++)
         {
            UINT32 dwNodeId = DBGetFieldULong(hResult, i, 0);
            NetObj *pObject = FindObjectById(dwNodeId);
            if (pObject != NULL)
            {
               if ((pObject->getObjectClass() == OBJECT_NODE) || (pObject->getObjectClass() == OBJECT_CLUSTER) || (pObject->getObjectClass() == OBJECT_MOBILEDEVICE) || (pObject->getObjectClass() == OBJECT_SENSOR))
               {
                  addChild(pObject);
                  pObject->addParent(this);
               }
               else
               {
                  nxlog_write(MSG_DCT_MAP_NOT_NODE, EVENTLOG_ERROR_TYPE, "dd", m_id, dwNodeId);
               }
            }
            else
            {
               nxlog_write(MSG_INVALID_DCT_MAP, EVENTLOG_ERROR_TYPE, "dd", m_id, dwNodeId);
            }
         }
         DBFreeResult(hResult);
      }
   }

   return success;
}

/**
 * Create management pack record
 */
void Template::createExportRecord(String &str)
{
   TCHAR guid[48];
   str.appendFormattedString(_T("\t\t<template id=\"%d\">\n\t\t\t<guid>%s</guid>\n\t\t\t<name>%s</name>\n\t\t\t<flags>%d</flags>\n"),
                             m_id, m_guid.toString(guid), (const TCHAR *)EscapeStringForXML2(m_name), m_flags);

   // Path in groups
   StringList path;
   ObjectArray<NetObj> *list = getParentList(OBJECT_TEMPLATEGROUP);
   TemplateGroup *parent = NULL;
   while(list->size() > 0)
   {
      parent = (TemplateGroup *)list->get(0);
      path.add(parent->getName());
      delete list;
      list = parent->getParentList(OBJECT_TEMPLATEGROUP);
   }
   delete list;

   str.append(_T("\t\t\t<path>\n"));
   for(int j = path.size() - 1, id = 1; j >= 0; j--, id++)
   {
      str.append(_T("\t\t\t\t<element id=\""));
      str.append(id);
      str.append(_T("\">"));
      str.append(path.get(j));
      str.append(_T("</element>\n"));
   }
   str.append(_T("\t\t\t</path>\n\t\t\t<dataCollection>\n"));

   lockDciAccess(false);
   for(int i = 0; i < m_dcObjects->size(); i++)
      m_dcObjects->get(i)->createExportRecord(str);
   unlockDciAccess();

   str.append(_T("\t\t\t</dataCollection>\n"));

   Iterator<GenericAgentPolicy> *it = m_policyList->iterator();
   while(it->hasNext())
   {
      GenericAgentPolicy *object = it->next();
      object->createExportRecord(str);
   }
   delete it;

   AutoBindTarget::createExportRecord(str);
   str.append(_T("\t\t</template>\n"));
}


/**
 * Apply template to data collection target
 */
BOOL Template::applyToTarget(DataCollectionTarget *target)
{
   //Print in log that policies will be installed
   nxlog_debug_tag(_T("obj.dc"), 2, _T("Apply %d policy items from template \"%s\" to target \"%s\""),
                   m_policyList->size(), m_name, target->getName());

   Iterator<GenericAgentPolicy> *it = m_policyList->iterator();
   while(it->hasNext())
   {
      GenericAgentPolicy *object = it->next();
      ServerJob *job = new PolicyInstallJob(target, m_id, object->getGuid(), object->getName(), 0);
      if (!AddJob(job))
      {
         delete job;
         nxlog_debug_tag(_T("obj.dc"), 2, _T("Unable to create job for %s(%s) policy installation on %s[%d] node"), object->getName(),
               (const TCHAR *)object->getGuid().toString(), target->getName(), target->getId());
      }
   }
   delete it;

   return super::applyToTarget(target);
}

/**
 * Unlock data collection items list
 */
void Template::applyDCIChanges()
{
   if (m_dciListModified)
   {
      if (getObjectClass() == OBJECT_TEMPLATE)
         updateVersion();
      setModified(MODIFY_OTHER);
   }
   super::applyDCIChanges();
}

/**
 * Create NXCP message with object's data
 */
void Template::fillMessageInternal(NXCPMessage *pMsg, UINT32 userId)
{
   super::fillMessageInternal(pMsg, userId);
   AutoBindTarget::fillMessageInternal(pMsg, userId);
   VersionableObject::fillMessageInternal(pMsg, userId);
   UINT32 baseId = VID_AGENT_POLICY_BASE;
   Iterator<GenericAgentPolicy> *it = m_policyList->iterator();
   int count = 0;
   while(it->hasNext())
   {
      GenericAgentPolicy *object = it->next();
      object->fillMessageInternal(pMsg, baseId);
      baseId+=100;
      count++;
   }
   delete it;
   pMsg->setField(VID_NOMBER_OF_POLICIES, count);
}

UINT32 Template::modifyFromMessageInternal(NXCPMessage *pRequest)
{
   // Change flags
   if (pRequest->isFieldExist(VID_FLAGS))
   {
      UINT32 mask = pRequest->isFieldExist(VID_FLAGS_MASK) ? pRequest->getFieldAsUInt32(VID_FLAGS_MASK) : 0xFFFFFFFF;
      m_flags &= ~mask;
      m_flags |= pRequest->getFieldAsUInt32(VID_FLAGS) & mask;
   }
   AutoBindTarget::modifyFromMessageInternal(pRequest);

   return super::modifyFromMessageInternal(pRequest);
}

void Template::updateFromImport(ConfigEntry *config)
{
   super::updateFromImport(config);
   AutoBindTarget::updateFromImport(config);
   VersionableObject::updateFromImport(config);

   m_policyList->clear();
   ObjectArray<ConfigEntry> *dcis = config->getSubEntries(_T("agentPolicy#*"));
   for(int i = 0; i < dcis->size(); i++)
   {
      ConfigEntry *e = dcis->get(i);
      uuid guid = e->getSubEntryValueAsUUID(_T("guid"));
      GenericAgentPolicy *curr = CreatePolicy(guid, m_id, e->getSubEntryValue(_T("policyType")));
      curr->updateFromImport(e);
      m_policyList->set(guid, curr);
   }
   delete dcis;
}

/**
 * Serialize object to JSON
 */
json_t *Template::toJson()
{
   json_t *root = super::toJson();
   AutoBindTarget::toJson(root);
   VersionableObject::toJson(root);
   Iterator<GenericAgentPolicy> *it = m_policyList->iterator();
   while(it->hasNext())
   {
      GenericAgentPolicy *object = it->next();
      json_object_set_new(root, "agentPolicy", object->toJson());
   }
   delete it;

   return root;
}

/**
 * Called when data collection configuration changed
 */
void Template::onDataCollectionChange()
{
   queueUpdate();
}

/**
 * Prepare template for deletion
 */
void Template::prepareForDeletion()
{
   lockChildList(false);
   for(int i = 0; i < m_childList->size(); i++)
   {
      NetObj *object = m_childList->get(i);
      if (object->isDataCollectionTarget())
         queueRemoveFromTarget(object->getId(), true);
   }
   unlockChildList();
   super::prepareForDeletion();
}

/**
 * Check policy exist
 */
bool Template::hasPolicy(uuid guid)
{
   lockProperties();
   bool hasPolicy = m_policyList->contains(guid);
   unlockProperties();
   return hasPolicy;
}

/**
 * Update policy if guid is provided and create policy if uuid is NULL
 */
uuid Template::updatePolicyFromMessage(NXCPMessage *request)
{
   lockProperties();
   uuid guid = request->getFieldAsGUID(VID_GUID);
   if(guid.isNull())
   {
      GenericAgentPolicy *curr = CreatePolicy(request->getFieldAsString(VID_NAME), request->getFieldAsString(VID_POLICY_TYPE), m_id);
      m_policyList->set(guid, curr);
   }
   else
   {
      if(m_policyList->contains(guid))
      {
         m_policyList->get(guid)->updateFromMessage(request);
      }
      else
         return uuid::NULL_UUID;
   }
   m_policyList->remove(guid);
   NotifyClientPolicyUpdate();
   //Here or in session notify all sessions about object update
   unlockProperties();
   return guid;
}

/**
 * Remove policy by uuid
 */
bool Template::removePolicy(uuid guid)
{
   lockProperties();
   bool success = m_policyList->contains(guid);
   m_policyList->remove(guid);
   NotifyClientPolicyUpdate();
   //Here or in session notify all sessions about object update
   unlockProperties();
   return success;
}

/**
 * Get agent policy copy
 */
GenericAgentPolicy *Template::getAgentPolicyCopy(uuid guid)
{
   GenericAgentPolicy *policy = NULL;
   lockProperties();
   policy = m_policyList->get(guid);
   if(policy != NULL)
      policy = new GenericAgentPolicy(policy);
   unlockProperties();
   return policy;
}


ObjectArray<GenericAgentPolicy> *Template::getpolicyListCopy()
{
   lockProperties();
   ObjectArray<GenericAgentPolicy> *list = new ObjectArray<GenericAgentPolicy>(0, m_policyList->size(), true);
   Iterator<GenericAgentPolicy> *it = m_policyList->iterator();
   while(it->hasNext())
   {
      list->add(new GenericAgentPolicy(it->next()));
   }
   delete it;
   unlockProperties();
   return list;
}

//Modify policy
//delete policy
//Deploy policy
