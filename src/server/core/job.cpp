/* 
** NetXMS - Network Management System
** Copyright (C) 2003-2009 Victor Kirhenshtein
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
** File: job.cpp
**
**/

#include "nxcore.h"


//
// Static members
//

DWORD ServerJob::s_freeId = 1;


//
// Constructor
//

ServerJob::ServerJob(const TCHAR *type, const TCHAR *description, DWORD node)
{
	m_id = s_freeId++;
	m_type = _tcsdup(CHECK_NULL(type));
	m_description = _tcsdup(CHECK_NULL(description));
	m_status = JOB_PENDING;
	m_lastStatusChange = time(NULL);
	m_autoCancelDelay = 0;
	m_remoteNode = node;
	m_progress = 0;
	m_failureMessage = NULL;
	m_owningQueue = NULL;
	m_workerThread = INVALID_THREAD_HANDLE;
	m_lastNotification = 0;
	m_notificationLock = MutexCreate();
}


//
// Destructor
//

ServerJob::~ServerJob()
{
	ThreadJoin(m_workerThread);

	safe_free(m_type);
	safe_free(m_description);
	MutexDestroy(m_notificationLock);
}


//
// Send notification to clients
//

void ServerJob::sendNotification(ClientSession *session, void *arg)
{
	ServerJob *job = (ServerJob *)arg;
	if (job->m_resolvedObject->CheckAccessRights(session->GetUserId(), OBJECT_ACCESS_READ))
		session->SendMessage(&job->m_notificationMessage);
}


//
// Notify clients
//

void ServerJob::notifyClients(bool isStatusChange)
{
	time_t t = time(NULL);
	if (!isStatusChange && (t - m_lastNotification < 10))
		return;	// Don't send progress notifycations often then 10 seconds

	MutexLock(m_notificationLock, INFINITE);
	m_notificationMessage.SetCode(CMD_JOB_CHANGE_NOTIFICATION);
	fillMessage(&m_notificationMessage);
	m_resolvedObject = FindObjectById(m_remoteNode);
	if (m_resolvedObject != NULL)
		EnumerateClientSessions(ServerJob::sendNotification, this);
	MutexUnlock(m_notificationLock);
}


//
// Change status
//

void ServerJob::changeStatus(ServerJobStatus newStatus)
{
	m_status = newStatus;
	m_lastStatusChange = time(NULL);
	notifyClients(true);
}


//
// Set owning queue
//

void ServerJob::setOwningQueue(ServerJobQueue *queue)
{
	m_owningQueue = queue;
	notifyClients(true);
}


//
// Update progress
//

void ServerJob::markProgress(int pctCompleted)
{
	if ((pctCompleted > m_progress) && (pctCompleted <= 100))
	{
		m_progress = pctCompleted;
		notifyClients(false);
	}
}


//
// Worker thread starter
//

THREAD_RESULT THREAD_CALL ServerJob::WorkerThreadStarter(void *arg)
{
	ServerJob *job = (ServerJob *)arg;
	DbgPrintf(4, _T("Job %d started"), job->m_id);

	if (job->run())
	{
		job->changeStatus(JOB_COMPLETED);
	}
	else
	{
		job->changeStatus((job->m_status == JOB_CANCEL_PENDING) ? JOB_CANCELLED : JOB_FAILED);
	}
	job->m_workerThread = INVALID_THREAD_HANDLE;

	DbgPrintf(4, _T("Job %d finished, status=%s"), job->m_id, (job->m_status == JOB_COMPLETED) ? _T("COMPLETED") : ((job->m_status == JOB_CANCELLED) ? _T("CANCELLED") : _T("FAILED")));

	if (job->m_owningQueue != NULL)
		job->m_owningQueue->jobCompleted(job);
	return THREAD_OK;
}


//
// Start job
//

void ServerJob::start()
{
	m_status = JOB_ACTIVE;
	m_workerThread = ThreadCreateEx(WorkerThreadStarter, 0, this);
}


//
// Cancel job
//

bool ServerJob::cancel()
{
	switch(m_status)
	{
		case JOB_COMPLETED:
		case JOB_CANCEL_PENDING:
			return false;
		case JOB_ACTIVE:
			if (!onCancel())
				return false;
			changeStatus(JOB_CANCEL_PENDING);
			return true;
		default:
			changeStatus(JOB_CANCELLED);
			return true;
	}
}


//
// Default run (empty)
//

bool ServerJob::run()
{
	return true;
}


//
// Default cancel handler
//

bool ServerJob::onCancel()
{
	return false;
}


//
// Set failure message
//

void ServerJob::setFailureMessage(const TCHAR *msg)
{
	safe_free(m_failureMessage);
	m_failureMessage = (msg != NULL) ? _tcsdup(msg) : NULL;
}


//
// Set description
//

void ServerJob::setDescription(const TCHAR *description)
{ 
	safe_free(m_description);
	m_description = _tcsdup(description); 
}


//
// Fill NXCP message with job's data
//

void ServerJob::fillMessage(CSCPMessage *msg)
{
	msg->SetVariable(VID_JOB_ID, m_id);
	msg->SetVariable(VID_JOB_TYPE, m_type);
	msg->SetVariable(VID_OBJECT_ID, m_remoteNode);
	msg->SetVariable(VID_DESCRIPTION, CHECK_NULL_EX(m_description));
	msg->SetVariable(VID_JOB_STATUS, (WORD)m_status);
	msg->SetVariable(VID_JOB_PROGRESS, (WORD)m_progress);
	msg->SetVariable(VID_FAILURE_MESSAGE, CHECK_NULL(m_failureMessage));
}
