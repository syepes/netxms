package org.netxms.ui.eclipse.applauncher;

import org.eclipse.core.runtime.Status;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.ui.plugin.AbstractUIPlugin;
import org.netxms.ui.eclipse.tools.PlatformTools;
import org.osgi.framework.BundleContext;

public class Activator extends AbstractUIPlugin
{
   private static final String PLUGIN_ID = "org.netxms.ui.eclipse.applauncher"; 

   private static Activator plugin;

   /* (non-Javadoc)
    * @see org.eclipse.ui.plugin.AbstractUIPlugin#start(org.osgi.framework.BundleContext)
    */
   @Override
   public void start(BundleContext context) throws Exception
   {
      super.start(context);
      plugin = this;
   }
   
   /* (non-Javadoc)
    * @see org.eclipse.ui.plugin.AbstractUIPlugin#stop(org.osgi.framework.BundleContext)
    */
   @Override
   public void stop(BundleContext context) throws Exception
   {
      plugin = null;
      super.stop(context);
   }

   /**
    * Returns the shared instance
    *
    * @return the shared instance
    */
   public static Activator getDefault()
   {
      return plugin;
   }
   
   /**
    * Returns an image descriptor for the image file at the given
    * plug-in relative path
    *
    * @param path the path
    * @return the image descriptor
    */
   public static ImageDescriptor getImageDescriptor(String path)
   {
      return PlatformTools.getImageDescriptorFromPlugin(PLUGIN_ID, path);
   }

   /**
    * Log via platform logging facilities
    * 
    * @param msg
    */
   public static void logInfo(String msg)
   {
      log(Status.INFO, msg, null);
   }

   /**
    * Log via platform logging facilities
    * 
    * @param msg
    */
   public static void logError(String msg, Throwable t)
   {
      log(Status.ERROR, msg, t);
   }

   /**
    * Log via platform logging facilities
    * 
    * @param msg
    * @param t
    */
   public static void log(int status, String msg, Throwable t)
   {
      getDefault().getLog().log(new Status(status, PLUGIN_ID, Status.OK, msg, t));
   }
}
