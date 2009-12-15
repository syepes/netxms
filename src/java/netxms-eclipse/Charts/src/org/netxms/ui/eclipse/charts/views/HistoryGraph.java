/**
 * NetXMS - open source network management system
 * Copyright (C) 2003-2009 Victor Kirhenshtein
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
package org.netxms.ui.eclipse.charts.views;

import java.util.ArrayList;
import java.util.Date;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;
import org.eclipse.core.runtime.jobs.Job;
import org.eclipse.jface.action.IMenuManager;
import org.eclipse.jface.action.IToolBarManager;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.IActionBars;
import org.eclipse.ui.IViewSite;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.part.ViewPart;
import org.eclipse.ui.progress.IWorkbenchSiteProgressService;
import org.eclipse.ui.progress.UIJob;
import org.netxms.client.NXCException;
import org.netxms.client.NXCSession;
import org.netxms.client.datacollection.DciData;
import org.netxms.client.datacollection.DciDataRow;
import org.netxms.ui.eclipse.charts.Activator;
import org.netxms.ui.eclipse.charts.views.helpers.DCIInfo;
import org.netxms.ui.eclipse.charts.widgets.HistoricDataChart;
import org.netxms.ui.eclipse.shared.NXMCSharedData;
import org.netxms.ui.eclipse.tools.RefreshAction;

/**
 * @author Victor
 *
 */
public class HistoryGraph extends ViewPart
{
	public static final String ID = "org.netxms.ui.eclipse.charts.view.history_graph";
	public static final String JOB_FAMILY = "HistoryGraphJob";
	
	private NXCSession session;
	private ArrayList<DCIInfo> items = new ArrayList<DCIInfo>(1);
	private HistoricDataChart chart;
	private boolean updateInProgress = false;
	
	private Date timeFrom;
	private Date timeTo;
	private long timeRange = 3600000;
	
	private RefreshAction actionRefresh;

	/* (non-Javadoc)
	 * @see org.eclipse.ui.part.ViewPart#init(org.eclipse.ui.IViewSite)
	 */
	@Override
	public void init(IViewSite site) throws PartInitException
	{
		super.init(site);
		
		session = NXMCSharedData.getInstance().getSession();
		String id = site.getSecondaryId();
		
		timeFrom = new Date(System.currentTimeMillis() - timeRange);
		timeTo = new Date(System.currentTimeMillis());
		
		// Extract DCI ids from view id
		// (first field will be unique view id, so we skip it)
		String[] fields = id.split("&");
		for(int i = 1; i < fields.length; i++)
		{
			String[] subfields = fields[i].split("\\@");
			if (subfields.length == 3)
			{
				items.add(new DCIInfo(
						Long.parseLong(subfields[1], 10), 
						Long.parseLong(subfields[0], 10),
						subfields[2]));
			}
		}
	}

	/* (non-Javadoc)
	 * @see org.eclipse.ui.part.WorkbenchPart#createPartControl(org.eclipse.swt.widgets.Composite)
	 */
	@Override
	public void createPartControl(Composite parent)
	{
		chart = new HistoricDataChart(parent, SWT.NONE);
		
		makeActions();
		contributeToActionBars();
		
		getDataFromServer();
	}

	/**
	 * Get DCI data from server
	 */
	private void getDataFromServer()
	{
		// Request data from server
		Job job = new Job("Get DCI values history graph")
		{
			@Override
			protected IStatus run(IProgressMonitor monitor)
			{
				IStatus status;
				
				final DciData[] data = new DciData[items.size()];
				try
				{
					for(int i = 0; i < items.size(); i++)
					{
						DCIInfo info = items.get(i);
						data[i] = session.getCollectedData(info.getNodeId(), info.getDciId(), timeFrom, timeTo, 0);
					}
					status = Status.OK_STATUS;
	
					new UIJob("Update chart") {
						@Override
						public IStatus runInUIThread(IProgressMonitor monitor)
						{
							chart.setTimeRange(timeFrom, timeTo);
							setChartData(data);
							updateInProgress = false;
							return Status.OK_STATUS;
						}
					}.schedule();
				}
				catch(Exception e)
				{
					status = new Status(Status.ERROR, Activator.PLUGIN_ID, 
	                    (e instanceof NXCException) ? ((NXCException)e).getErrorCode() : 0,
	                    "Cannot get DCI values for history graph: " + e.getMessage(), e);
					updateInProgress = false;
				}
				return status;
			}

			/* (non-Javadoc)
			 * @see org.eclipse.core.runtime.jobs.Job#belongsTo(java.lang.Object)
			 */
			@Override
			public boolean belongsTo(Object family)
			{
				return family == HistoryGraph.JOB_FAMILY;
			}
		};
		IWorkbenchSiteProgressService siteService =
	      (IWorkbenchSiteProgressService)getSite().getAdapter(IWorkbenchSiteProgressService.class);
		siteService.schedule(job, 0, true);
	}

	/* (non-Javadoc)
	 * @see org.eclipse.ui.part.WorkbenchPart#setFocus()
	 */
	@Override
	public void setFocus()
	{
		chart.setFocus();
	}
	
	/**
	 * Create actions
	 */
	private void makeActions()
	{
		actionRefresh = new RefreshAction() {
			/* (non-Javadoc)
			 * @see org.eclipse.jface.action.Action#run()
			 */
			@Override
			public void run()
			{
				updateChart();
			}
		};
	}
	
	/**
	 * Fill action bars
	 */
	private void contributeToActionBars()
	{
		IActionBars bars = getViewSite().getActionBars();
		fillLocalPullDown(bars.getMenuManager());
		fillLocalToolBar(bars.getToolBarManager());
	}

	private void fillLocalPullDown(IMenuManager manager)
	{
		manager.add(actionRefresh);
	}

	private void fillLocalToolBar(IToolBarManager manager)
	{
		manager.add(actionRefresh);
	}

	/**
	 * Set chart data
	 * 
	 * @param data Retrieved DCI data
	 */
	private void setChartData(final DciData[] data)
	{
		for(int i = 0; i < data.length; i++)
			addItemToChart(i, items.get(i), data[i]);
		
		chart.getAxisSet().getYAxis(0).adjustRange();
		chart.redraw();
	}
	
	/**
	 * Add single DCI to chart
	 * 
	 * @param data DCI data
	 */
	private void addItemToChart(int index, final DCIInfo info, final DciData data)
	{
		final DciDataRow[] values = data.getValues();
		
		// Create series
		Date[] xSeries = new Date[values.length];
		double[] ySeries = new double[values.length];
		for(int i = 0; i < values.length; i++)
		{
			xSeries[i] = values[i].getTimestamp();
			ySeries[i] = values[i].getValueAsDouble();
		}
		
		chart.addLineSeries(index, info.getDescription(), xSeries, ySeries);
	}
	
	/**
	 * Update chart
	 */
	private void updateChart()
	{
		if (updateInProgress)
			return;
		
		updateInProgress = true;
		timeFrom = new Date(System.currentTimeMillis() - timeRange);
		timeTo = new Date(System.currentTimeMillis());
		getDataFromServer();
	}
}
