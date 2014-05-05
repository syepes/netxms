/**
 * NetXMS - open source network management system
 * Copyright (C) 2003-2014 Raden Solutions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
package org.netxms.api.client.reporting;

import java.util.Date;
import java.util.UUID;
import org.netxms.base.NXCPMessage;

/**
 * Reporting job
 */
public class ReportingJob
{
	static public final int TYPE_ONCE = 0;
	static public final int TYPE_DAILY = 1;
	static public final int TYPE_WEEKLY = 2;
	static public final int TYPE_MONTHLY = 3;

	private UUID reportId;
	private UUID jobId;
	private int daysOfWeek;
	private int daysOfMonth;
	private int userId;
	private int type = TYPE_ONCE;
	private Date startTime;
	private String comments;

	/**
	 * Create reportingJob
	 * 
	 * @param reportGuid
	 */
	public ReportingJob(UUID reportId)
	{
		jobId = UUID.randomUUID();
		this.reportId = reportId;
		startTime = new Date();
		daysOfWeek = 0;
		daysOfMonth = 0;
		type = TYPE_ONCE;
	}

	/**
	 * Create reportingJob object from NXCP message
	 * 
	 * @param msg
	 * @param baseId
	 */
	public ReportingJob(NXCPMessage msg, long varId)
	{
		jobId = msg.getVariableAsUUID(varId);
		reportId = msg.getVariableAsUUID(varId + 1);
		userId = msg.getVariableAsInteger(varId + 2);
		startTime = msg.getVariableAsDate(varId + 3);
		daysOfWeek = msg.getVariableAsInteger(varId + 4);
		daysOfMonth = msg.getVariableAsInteger(varId + 5);
		type = msg.getVariableAsInteger(varId + 6);
		comments = msg.getVariableAsString(varId + 7);
	}

	/**
	 * @return
	 */
	public UUID getReportId()
	{
		return reportId;
	}

	/**
	 * @param reportId
	 */
	public void setReportId(UUID reportId)
	{
		this.reportId = reportId;
	}

	/**
	 * @return
	 */
	public UUID getJobId()
	{
		return jobId;
	}

	/**
	 * @param jobId
	 */
	public void setJobId(UUID jobId)
	{
		this.jobId = jobId;
	}

	/**
	 * @return
	 */
	public int getDaysOfWeek()
	{
		return daysOfWeek;
	}

	/**
	 * @param daysOfWeek
	 */
	public void setDaysOfWeek(int daysOfWeek)
	{
		this.daysOfWeek = daysOfWeek;
	}

	/**
	 * @return
	 */
	public int getDaysOfMonth()
	{
		return daysOfMonth;
	}

	/**
	 * @param daysOfMonth
	 */
	public void setDaysOfMonth(int daysOfMonth)
	{
		this.daysOfMonth = daysOfMonth;
	}

	/**
	 * @return
	 */
	public int getUserId()
	{
		return userId;
	}

	/**
	 * @return
	 */
	public int getType()
	{
		return type;
	}

	/**
	 * @param type
	 */
	public void setType(int type)
	{
		this.type = type;
	}

	/**
	 * @return
	 */
	public Date getStartTime()
	{
		return startTime;
	}

	/**
	 * @param startTime
	 */
	public void setStartTime(Date startTime)
	{
		this.startTime = startTime;
	}

	public String getComments()
	{
		return comments == null ? "" : comments;
	}

	public void setComments(String comments)
	{
		this.comments = comments;
	}

	@Override
	public String toString()
	{
		return "ReportingJob [reportId=" + reportId + ", jobId=" + jobId + ", daysOfWeek=" + daysOfWeek + ", daysOfMonth="
				+ daysOfMonth + ", userId=" + userId + ", type=" + type + ", startTime=" + startTime + ", comments=" + comments + "]";
	}
}
