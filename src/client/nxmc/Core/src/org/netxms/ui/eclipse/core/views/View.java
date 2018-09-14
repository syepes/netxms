/**
 * NetXMS - open source network management system
 * Copyright (C) 2003-2018 Victor Kirhenshtein
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
package org.netxms.ui.eclipse.core.views;

import org.eclipse.e4.ui.model.application.ui.basic.MPart;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Shell;

/**
 * Base class for console views
 */
public abstract class View
{
   private String id;
   private String label;
   private int priority;
   private Shell shell;
   private MPart part;

   /**
    * Create new view 
    */
   protected View()
   {
   }
   
   /**
    * Configure view. Intended to be called by framework.
    * 
    * @param id
    * @param label
    * @param priority
    */
   public final void configure(String id, String label, int priority)
   {
      this.id = id;
      this.label = label;
      this.priority = priority;
   }
   
   /**
    * Initialize view
    * 
    * @param part
    * @param shell
    */
   public void init(MPart part, Shell shell)
   {
      this.part = part;
      this.shell = shell;
   }
   
   /**
    * Create content of the view
    * 
    * @param parent parent composite
    */
   public abstract void createContent(Composite parent);
   
   /**
    * Called when view is focused
    */
   public abstract void setFocus();
   
   /**
    * Called by framework when view is about to be destroyed 
    */
   public void dispose()
   {
   }

   /**
    * @return the id
    */
   public String getId()
   {
      return id;
   }

   /**
    * @return the label
    */
   public String getLabel()
   {
      return label;
   }

   /**
    * @return the priority
    */
   public int getPriority()
   {
      return priority;
   }

   /**
    * @return the part
    */
   public MPart getPart()
   {
      return part;
   }
   
   /**
    * Get shell
    * 
    * @return
    */
   public Shell getShell()
   {
      return shell;
   }
}
