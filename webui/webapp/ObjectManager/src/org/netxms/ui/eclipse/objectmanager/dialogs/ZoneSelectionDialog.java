/**
 * NetXMS - open source network management system
 * Copyright (C) 2003-2013 Victor Kirhenshtein
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
package org.netxms.ui.eclipse.objectmanager.dialogs;

import org.eclipse.jface.dialogs.Dialog;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Shell;
import org.netxms.client.NXCSession;
import org.netxms.client.objects.AbstractObject;
import org.netxms.client.objects.Zone;
import org.netxms.ui.eclipse.objectbrowser.dialogs.ObjectSelectionDialog;
import org.netxms.ui.eclipse.objectbrowser.widgets.ObjectSelector;
import org.netxms.ui.eclipse.objectmanager.Messages;
import org.netxms.ui.eclipse.shared.ConsoleSharedData;
import org.netxms.ui.eclipse.tools.MessageDialogHelper;
import org.netxms.ui.eclipse.tools.WidgetHelper;

/**
 * Dialog for selecting zone
 */
public class ZoneSelectionDialog extends Dialog
{
	private ObjectSelector objectSelector;
	private long zoneId;
	
	/**
	 * Constructor
	 * 
	 * @param parent Parent shell
	 */
	public ZoneSelectionDialog(Shell parent)
	{
		super(parent);
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.window.Window#configureShell(org.eclipse.swt.widgets.Shell)
	 */
	@Override
	protected void configureShell(Shell newShell)
	{
		super.configureShell(newShell);
		newShell.setText(Messages.get().ZoneSelectionDialog_Title);
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.jface.dialogs.Dialog#createDialogArea(org.eclipse.swt.widgets.Composite)
	 */
	@Override
	protected Control createDialogArea(Composite parent)
	{
		Composite dialogArea = (Composite)super.createDialogArea(parent);

		GridLayout layout = new GridLayout();
      layout.marginWidth = WidgetHelper.DIALOG_WIDTH_MARGIN;
      layout.marginHeight = WidgetHelper.DIALOG_HEIGHT_MARGIN;
      dialogArea.setLayout(layout);

      objectSelector = new ObjectSelector(dialogArea, SWT.NONE, false);
      objectSelector.setLabel(Messages.get().ZoneSelectionDialog_ZoneObject);
      objectSelector.setObjectClass(Zone.class);
      objectSelector.setClassFilter(ObjectSelectionDialog.createZoneSelectionFilter());
      GridData gd = new GridData();
      gd.horizontalAlignment = SWT.FILL;
      gd.grabExcessHorizontalSpace = true;
      gd.widthHint = 300;
      objectSelector.setLayoutData(gd);
      
		return dialogArea;
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.dialogs.Dialog#okPressed()
	 */
	@Override
	protected void okPressed()
	{
		long objectId = objectSelector.getObjectId();
		if (objectId == 0)
		{
			MessageDialogHelper.openWarning(getShell(), Messages.get().ZoneSelectionDialog_Warning, Messages.get().ZoneSelectionDialog_EmptySelectionWarning);
			return;
		}
		AbstractObject object = ((NXCSession)ConsoleSharedData.getSession()).findObjectById(objectId);
		if ((object == null) || !(object instanceof Zone))
		{
			MessageDialogHelper.openWarning(getShell(), Messages.get().ZoneSelectionDialog_Warning, Messages.get().ZoneSelectionDialog_EmptySelectionWarning);
			return;
		}
		zoneId = ((Zone)object).getZoneId();
		super.okPressed();
	}

	/**
	 * @return the zoneObjectId
	 */
	public long getZoneId()
	{
		return zoneId;
	}
}