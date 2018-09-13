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
package org.netxms.ui.eclipse.tools;

import java.net.URL;
import org.eclipse.core.runtime.FileLocator;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.widgets.Display;

/**
 * Platform tools
 */
public class PlatformTools
{
   /**
    * Get image descriptor from plugin
    * 
    * @param pluginId plugin ID
    * @param path image path
    * @return image descriptor
    */
   public static ImageDescriptor getImageDescriptorFromPlugin(String pluginId, String path)
   {
      try
      {
         if (!path.startsWith("/"))
         {
            path = "/" + path;
         }
         URL url = new URL("platform:/plugin/" + pluginId + path);
         url = FileLocator.resolve(url);
         return ImageDescriptor.createFromURL(url);
      }
      catch(Exception e)
      {
         return null;
      }
   }

   /**
    * Get image descriptor from plugin
    *
    * @param display display for image creation
    * @param pluginId plugin ID
    * @param path image path
    * @return image descriptor
    */
   public static Image getImageFromPlugin(Display display, String pluginId, String path)
   {
      ImageDescriptor d = getImageDescriptorFromPlugin(pluginId, path);
      return (d != null) ? d.createImage(display) : null;
   }
}
