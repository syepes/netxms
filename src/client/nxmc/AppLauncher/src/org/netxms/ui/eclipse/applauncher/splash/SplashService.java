/**
 * 
 */
package org.netxms.ui.eclipse.applauncher.splash;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import org.eclipse.core.runtime.FileLocator;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.osgi.util.NLS;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.DisposeEvent;
import org.eclipse.swt.events.DisposeListener;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.RGB;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Monitor;
import org.eclipse.swt.widgets.Shell;
import org.netxms.ui.eclipse.tools.ColorCache;

public class SplashService implements ISplashService
{
   private String pluginId = null;
   private String splashPath = "splash.bmp";
   private RGB backgroundColor = new RGB(255, 255, 255);
   private RGB messageColor = new RGB(0, 0, 0);
   private RGB versionColor = new RGB(0, 0, 0);
   private Shell splashShell = null;
   private Label messageLabel = null;
   private Label copyrightLabel = null;
   private Label versionLabel = null;
   private String message = "";
   private String version = "";
   private String copyright = "";
   private ColorCache colorCache = null;
   private IProgressMonitor progressMonitor = null;

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.applauncher.splash.ISplashService#setSplashPluginId(java.lang.String)
    */
   @Override
   public void setSplashPluginId(String pluginId)
   {
      this.pluginId = pluginId;
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.applauncher.splash.ISplashService#setSplashImagePath(java.lang.String)
    */
   @Override
   public void setSplashImagePath(String splashPath)
   {
      this.splashPath = splashPath;
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.applauncher.splash.ISplashService#setSplashBackgroundColor(org.eclipse.swt.graphics.RGB)
    */
   @Override
   public void setSplashBackgroundColor(RGB color)
   {
      backgroundColor = color;
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.applauncher.splash.ISplashService#setSplashMessageColor(org.eclipse.swt.graphics.RGB)
    */
   @Override
   public void setSplashMessageColor(RGB color)
   {
      messageColor = color;
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.applauncher.splash.ISplashService#setSplashVersionColor(org.eclipse.swt.graphics.RGB)
    */
   @Override
   public void setSplashVersionColor(RGB color)
   {
      versionColor = color;
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.applauncher.splash.ISplashService#open()
    */
   @Override
   public void open()
   {
      if (pluginId == null)
         throw new IllegalStateException("Splash plugin ID has not been set.");
      if (splashPath == null)
         throw new IllegalStateException("Splash image path has not been set.");
      
      progressMonitor = new IProgressMonitor() {
         @Override
         public void worked(int work)
         {
         }
         
         @Override
         public void subTask(String name)
         {
            setMessage(name);
         }
         
         @Override
         public void setTaskName(String name)
         {
            setMessage(name);
         }
         
         @Override
         public void setCanceled(boolean value)
         {
         }
         
         @Override
         public boolean isCanceled()
         {
            return false;
         }
         
         @Override
         public void internalWorked(double work)
         {
         }
         
         @Override
         public void done()
         {
         }
         
         @Override
         public void beginTask(String name, int totalWork)
         {
            setMessage(name);
         }
      };

      splashShell = createSplashShell();
      splashShell.open();
      splashShell.layout(true, true);
      splashShell.redraw();
   }

   /**
    * Create splash shell
    * 
    * @return splash shell
    */
   private Shell createSplashShell()
   {
      final Shell shell = new Shell(SWT.TOOL | SWT.NO_TRIM);
      Image image = createBackgroundImage(shell);
      shell.setBackgroundImage(image);
      shell.setBackgroundMode(SWT.INHERIT_NONE);
      
      colorCache = new ColorCache(shell);
      shell.setBackground(colorCache.create(backgroundColor));

      final GridLayout layout = new GridLayout();
      layout.numColumns = 1;
      layout.marginHeight = 20;
      layout.marginWidth = 20;
      layout.verticalSpacing = 4;
      layout.horizontalSpacing = 4;
      shell.setLayout(layout);

      versionLabel = new Label(shell, SWT.LEFT);
      versionLabel.setBackground(shell.getBackground());
      versionLabel.setForeground(colorCache.create(versionColor));
      versionLabel.setText(version);
      GridData gd = new GridData();
      gd.horizontalAlignment = SWT.RIGHT;
      gd.verticalAlignment = SWT.BOTTOM;
      gd.grabExcessHorizontalSpace = true;
      gd.grabExcessVerticalSpace = true;
      versionLabel.setLayoutData(gd);

      copyrightLabel = new Label(shell, SWT.LEFT);
      copyrightLabel.setBackground(shell.getBackground());
      copyrightLabel.setForeground(colorCache.create(versionColor));
      copyrightLabel.setText(copyright);
      gd = new GridData();
      gd.horizontalAlignment = SWT.RIGHT;
      gd.verticalAlignment = SWT.BOTTOM;
      copyrightLabel.setLayoutData(gd);
      
      messageLabel = new Label(shell, SWT.LEFT);
      messageLabel.setBackground(shell.getBackground());
      messageLabel.setForeground(colorCache.create(messageColor));
      messageLabel.setText(message);
      gd = new GridData();
      gd.horizontalAlignment = SWT.FILL;
      gd.verticalAlignment = SWT.BOTTOM;
      gd.grabExcessHorizontalSpace = true;
      gd.verticalIndent = 20;
      messageLabel.setLayoutData(gd);

      Rectangle imageBounds = image.getBounds();
      shell.setSize(imageBounds.width, imageBounds.height);
      shell.setLocation(getMonitorCenter(shell));
      return shell;
   }

   /**
    * Create background image for shell
    * 
    * @param parent
    * @return
    */
   private Image createBackgroundImage(Shell parent)
   {
      final Image splashImage = getImageDescriptor(pluginId, splashPath).createImage();
      parent.addDisposeListener(new DisposeListener() {
         @Override
         public void widgetDisposed(DisposeEvent e)
         {
            splashImage.dispose();
         }
      });
      return splashImage;
   }

   private ImageDescriptor getImageDescriptor(String pluginId, String path)
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
      catch(MalformedURLException e)
      {
         String msg = NLS.bind("The image path {0} in not a valid location the bundle {1}.", path, pluginId);
         throw new RuntimeException(msg, e);
      }
      catch(IOException e)
      {
         String msg = NLS.bind("The image {0} was not found in the bundle {1}.", path, pluginId);
         throw new RuntimeException(msg, e);
      }
   }

   /**
    * Get center of shell's monitor
    * 
    * @param shell shell
    * @return center point of shell's monitor
    */
   private Point getMonitorCenter(Shell shell)
   {
      Monitor primary = shell.getDisplay().getPrimaryMonitor();
      Rectangle bounds = primary.getBounds();
      Rectangle rect = shell.getBounds();
      int x = bounds.x + (bounds.width - rect.width) / 2;
      int y = bounds.y + (bounds.height - rect.height) / 2;
      return new Point(x, y);
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.applauncher.splash.ISplashService#close()
    */
   @Override
   public void close()
   {
      splashShell.close();
      splashShell = null;
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.applauncher.splash.ISplashService#setMessage(java.lang.String)
    */
   @Override
   public void setMessage(final String message)
   {
      this.message = message;
      updateLabel(messageLabel, message);
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.applauncher.splash.ISplashService#setProductVersion(java.lang.String)
    */
   @Override
   public void setProductVersion(String version)
   {
      this.version = version;
      updateLabel(versionLabel, version);
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.applauncher.splash.ISplashService#setProductCopyright(java.lang.String)
    */
   @Override
   public void setProductCopyright(String copyright)
   {
      this.copyright = copyright;
      updateLabel(copyrightLabel, copyright);
   }
   
   /**
    * Update splash screen label
    * 
    * @param label label control
    * @param text new text
    */
   private void updateLabel(final Label label, final String text)
   {
      if ((label == null) || label.isDisposed())
         return;
      
      splashShell.getDisplay().syncExec(new Runnable() {
         @Override
         public void run()
         {
            label.setText(text);
            splashShell.layout();
            splashShell.update();
         }
      });
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.applauncher.splash.ISplashService#getShell()
    */
   @Override
   public Shell getShell()
   {
      return splashShell;
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.applauncher.splash.ISplashService#getProgressMonitor()
    */
   @Override
   public IProgressMonitor getProgressMonitor()
   {
      return progressMonitor;
   }
}
