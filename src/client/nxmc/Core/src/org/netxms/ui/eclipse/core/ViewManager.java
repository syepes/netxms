/**
 * 
 */
package org.netxms.ui.eclipse.core;

import java.util.HashMap;
import java.util.Map;
import org.eclipse.core.runtime.IConfigurationElement;
import org.eclipse.core.runtime.IExtensionRegistry;
import org.eclipse.core.runtime.Platform;
import org.netxms.ui.eclipse.core.views.View;

/**
 * Manager for registered views
 */
public class ViewManager
{
   private static ViewManager instance = null;
   
   /**
    * Get view manager instance
    * 
    * @return view manager instance
    */
   public static ViewManager getInstance()
   {
      if (instance == null)
         instance = new ViewManager();
      return instance;
   }
   
   private Map<String, IConfigurationElement> registry = new HashMap<String, IConfigurationElement>();
   
   private ViewManager()
   {
      final IExtensionRegistry reg = Platform.getExtensionRegistry();
      IConfigurationElement[] elements = reg.getConfigurationElementsFor("org.netxms.ui.eclipse.views"); //$NON-NLS-1$
      for(int i = 0; i < elements.length; i++)
      {
         String id = elements[i].getAttribute("id"); //$NON-NLS-1$
         registry.put(id, elements[i]);
      }
   }
   
   /**
    * Create view with given ID
    * 
    * @param id view ID
    * @return view object or null
    */
   public View createView(String id)
   {
      IConfigurationElement element = registry.get(id);
      if (element == null)
         return null;
      try
      {
         View view = (View)element.createExecutableExtension("class");

         int p = 65535;
         String value = element.getAttribute("priority"); //$NON-NLS-1$
         if (value != null)
         {
            try
            {
               p = Integer.parseInt(value);
               if ((p < 0) || (p > 65535))
                  p = 65535;
            }
            catch(NumberFormatException e)
            {
            }
         }
         view.configure(id, element.getAttribute("label"), p);
         return view;
      }
      catch(Exception e)
      {
         return null;
      }
   }
}
