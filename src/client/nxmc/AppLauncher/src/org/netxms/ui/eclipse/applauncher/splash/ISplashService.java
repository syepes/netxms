/**
 * 
 */
package org.netxms.ui.eclipse.applauncher.splash;

import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.swt.graphics.RGB;
import org.eclipse.swt.widgets.Shell;

public interface ISplashService
{
   /**
    * Tell the Service where to find the splash image
    * 
    * @param pluginId ID of the plugin where the image resides
    */
   public void setSplashPluginId(String pluginId);

   /**
    * Tell the service the path and name of the splash image
    * 
    * @param path Path and filename of the spalsh image
    */
   public void setSplashImagePath(String path);
   
   /**
    * Set background color for splash screen messages
    * 
    * @param color background color for splash screen messages
    */
   public void setSplashBackgroundColor(RGB color);

   /**
    * Set text color for splash screen messages
    * 
    * @param color text color for splash screen messages
    */
   public void setSplashMessageColor(RGB color);

   /**
    * Set text color for splash screen version information
    * 
    * @param color text color for splash screen version information
    */
   public void setSplashVersionColor(RGB color);

   /**
    * Open the splash screen
    */
   public void open();

   /**
    * Close the splash screen
    */
   public void close();

   /**
    * Set the displayed message on the splash screen
    * 
    * @param message Text message to be displayed.
    */
   public void setMessage(String message);
   
   /**
    * Set product version to display
    * 
    * @param version product version to display
    */
   public void setProductVersion(String version);
   
   /**
    * Set product copyright message to display
    * 
    * @param copyright product copyright message to display
    */
   public void setProductCopyright(String copyright);
   
   /**
    * Get splash screen shell
    * 
    * @return splash screen shell
    */
   public Shell getShell();
   
   /**
    * Get progress monitor for splash screen
    * 
    * @return progress monitor for splash screen
    */
   public IProgressMonitor getProgressMonitor();
}
