/**
 * NetXMS - open source network management system
 * Copyright (C) 2019 Raden Solutions
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
package org.netxms.ui.eclipse.datacollection.views;

import java.util.HashMap;
import java.util.UUID;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.jface.action.Action;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.ui.ISaveablePart2;
import org.eclipse.ui.IViewSite;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.part.ViewPart;
import org.netxms.client.NXCSession;
import org.netxms.client.objects.AgentPolicy;
import org.netxms.client.objects.Template;
import org.netxms.ui.eclipse.shared.ConsoleSharedData;

/**
 * Editor for the policies
 */
public class PolicyEditor  extends ViewPart implements ISaveablePart2
{
   public static final String ID = "org.netxms.ui.eclipse.datacollection.view.policy_editor"; //$NON-NLS-1$
   
   private NXCSession session;
   private long templateId;
   private HashMap<UUID, AgentPolicy> policyList = null;
   private boolean modified = false;

   protected Action actionRefresh;
   protected Action actionSave;
   protected Button buttonAdd;
   protected Button buttonRemove;
   //area for selection
   //are for policy editor(name, guid(not editable), the )
   
   
   
   /* (non-Javadoc)
    * @see org.eclipse.ui.part.ViewPart#init(org.eclipse.ui.IViewSite)
    */
   @Override
   public void init(IViewSite site) throws PartInitException
   {
      super.init(site);
      session = ConsoleSharedData.getSession();
   }

   /**
    * Set template object to edit
    * 
    * @param policy policy object
    */
   public void setPolicy(Template templateObj)
   {
      templateId = templateObj.getObjectId();
      setPartName("Edit Policies for \""+ templateObj.getObjectName() +"\" template");
      doRefresh();
      modified = false;
      firePropertyChange(PROP_DIRTY);
      actionSave.setEnabled(false);
   }

   private void doRefresh()
   {
      // TODO Auto-generated method stub
      
   }

   @Override
   public void createPartControl(Composite parent)
   {
      
      // TODO Auto-generated method stub
      
   }

   @Override
   public void setFocus()
   {
      // TODO Auto-generated method stub
      
   }

   @Override
   public void doSave(IProgressMonitor monitor)
   {
      // TODO Auto-generated method stub
      
   }

   @Override
   public void doSaveAs()
   {
      // TODO Auto-generated method stub
      
   }

   @Override
   public boolean isDirty()
   {
      // TODO Auto-generated method stub
      return false;
   }

   @Override
   public boolean isSaveAsAllowed()
   {
      // TODO Auto-generated method stub
      return false;
   }

   @Override
   public boolean isSaveOnCloseNeeded()
   {
      // TODO Auto-generated method stub
      return false;
   }

   @Override
   public int promptToSaveOnClose()
   {
      // TODO Auto-generated method stub
      return 0;
   }
   
}
