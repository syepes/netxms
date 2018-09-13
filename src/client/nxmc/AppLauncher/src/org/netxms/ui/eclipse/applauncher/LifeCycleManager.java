/**
 * NetXMS - open source network management system
 * Copyright (C) 2003-2018 Raden Solutions
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
package org.netxms.ui.eclipse.applauncher;

import java.lang.reflect.InvocationTargetException;
import java.security.Signature;
import java.security.cert.Certificate;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.Platform;
import org.eclipse.e4.core.contexts.IEclipseContext;
import org.eclipse.e4.core.services.events.IEventBroker;
import org.eclipse.e4.ui.workbench.UIEvents;
import org.eclipse.e4.ui.workbench.lifecycle.PostContextCreate;
import org.eclipse.e4.ui.workbench.lifecycle.ProcessAdditions;
import org.eclipse.jface.dialogs.IDialogSettings;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.operation.IRunnableWithProgress;
import org.eclipse.jface.operation.ModalContext;
import org.eclipse.jface.window.Window;
import org.eclipse.swt.graphics.RGB;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.Shell;
import org.netxms.base.BuildNumber;
import org.netxms.base.NXCommon;
import org.netxms.certificate.loader.KeyStoreRequestListener;
import org.netxms.certificate.manager.CertificateManager;
import org.netxms.certificate.manager.CertificateManagerProvider;
import org.netxms.certificate.request.KeyStoreEntryPasswordRequestListener;
import org.netxms.client.NXCException;
import org.netxms.client.NXCSession;
import org.netxms.client.constants.AuthenticationType;
import org.netxms.client.constants.RCC;
import org.netxms.ui.eclipse.applauncher.dialogs.LoginDialog;
import org.netxms.ui.eclipse.applauncher.dialogs.PasswordRequestDialog;
import org.netxms.ui.eclipse.applauncher.dialogs.SecurityWarningDialog;
import org.netxms.ui.eclipse.applauncher.splash.ISplashService;
import org.netxms.ui.eclipse.console.dialogs.PasswordExpiredDialog;
import org.netxms.ui.eclipse.jobs.LoginJob;
import org.netxms.ui.eclipse.shared.ConsoleSharedData;
import org.osgi.service.event.Event;
import org.osgi.service.event.EventHandler;

/**
 * Application life cycle manager
 */
@SuppressWarnings("restriction")
public class LifeCycleManager implements KeyStoreRequestListener, KeyStoreEntryPasswordRequestListener
{
   /**
    * Process context create
    * 
    * @param splashService
    * @param eventBroker
    */
   @PostContextCreate
   void postContextCreate(final ISplashService splashService, final IEventBroker eventBroker)
   {
      splashService.setSplashPluginId("org.netxms.ui.eclipse.applauncher");
      splashService.setSplashImagePath("icons/splash.bmp");
      splashService.setSplashBackgroundColor(new RGB(48, 48, 48));
      splashService.setSplashVersionColor(new RGB(224, 224, 224));
      splashService.setSplashMessageColor(new RGB(241, 133, 21));
      splashService.setProductVersion(String.format("Version %s (%s)", NXCommon.VERSION, BuildNumber.TEXT));
      splashService.setProductCopyright("\u00A9 2003-2018 Raden Solutions");
      splashService.open();
      splashService.setMessage("Loading...");
      
      eventBroker.subscribe(UIEvents.UILifeCycle.APP_STARTUP_COMPLETE, new EventHandler() {
         @Override
         public void handleEvent(Event event) {
            splashService.close();
            eventBroker.unsubscribe(this);
         }
      });
   }
   
   /**
    * Process additions
    * 
    * @param splashService
    * @param context
    */
   @ProcessAdditions
   public void processAdditions(final ISplashService splashService, IEclipseContext context)
   {
      IDialogSettings settings = Activator.getDefault().getDialogSettings();
      boolean success = false;
      boolean autoConnect = false;
      boolean ignoreProtocolVersion = false;
      String password = ""; //$NON-NLS-1$

      CertificateManager certMgr = CertificateManagerProvider.provideCertificateManager();
      certMgr.setKeyStoreRequestListener(this);
      certMgr.setPasswordRequestListener(this);

      for(String s : Platform.getCommandLineArgs())
      {
         if (s.startsWith("-server=")) //$NON-NLS-1$
         {
            settings.put("Connect.Server", s.substring(8)); //$NON-NLS-1$
         }
         else if (s.startsWith("-login=")) //$NON-NLS-1$
         {
            settings.put("Connect.Login", s.substring(7)); //$NON-NLS-1$
         }
         else if (s.startsWith("-password=")) //$NON-NLS-1$
         {
            password = s.substring(10);
            settings.put("Connect.AuthMethod", AuthenticationType.PASSWORD.getValue()); //$NON-NLS-1$
         }
         else if (s.equals("-auto")) //$NON-NLS-1$
         {
            autoConnect = true;
         }
         else if (s.equals("-ignore-protocol-version")) //$NON-NLS-1$
         {
            ignoreProtocolVersion = true;
         }
      }

      boolean encrypt = true;
      LoginDialog loginDialog = new LoginDialog(null, certMgr);

      while(!success)
      {
         if (!autoConnect)
         {
            splashService.setMessage("Reading login information...");
            if (loginDialog.open() != Window.OK)
               System.exit(0);
            password = loginDialog.getPassword();
         }
         else
         {
            autoConnect = false; // only do auto connect first time
         }

         ConsoleSharedData.setProperty("SlowLink", settings.getBoolean("Connect.SlowLink")); //$NON-NLS-1$ //$NON-NLS-2$
         LoginJob job = new LoginJob(Display.getCurrent(), 
               settings.get("Connect.Server"), //$NON-NLS-1$ 
               settings.get("Connect.Login"), //$NON-NLS-1$
               encrypt, ignoreProtocolVersion);

         AuthenticationType authMethod;
         try
         {
            authMethod = AuthenticationType.getByValue(settings.getInt("Connect.AuthMethod")); //$NON-NLS-1$
         }
         catch(NumberFormatException e)
         {
            authMethod = AuthenticationType.PASSWORD;
         }
         switch(authMethod)
         {
            case PASSWORD:
               job.setPassword(password);
               break;
            case CERTIFICATE:
               job.setCertificate(loginDialog.getCertificate(), getSignature(certMgr, loginDialog.getCertificate()));
               break;
            default:
               break;
         }

         try
         {
            ModalContext.run(job, true, splashService.getProgressMonitor(), Display.getCurrent());
            success = true;
         }
         catch(InvocationTargetException e)
         {
            if ((e.getCause() instanceof NXCException)
                  && ((((NXCException)e.getCause()).getErrorCode() == RCC.NO_ENCRYPTION_SUPPORT) ||
                      (((NXCException)e.getCause()).getErrorCode() == RCC.NO_CIPHERS)) && encrypt)
            {
               boolean alwaysAllow = settings.getBoolean("Connect.AllowUnencrypted." + settings.get("Connect.Server")); //$NON-NLS-1$ //$NON-NLS-2$
               int action = getSecurityAction(settings, alwaysAllow);
               if (action != SecurityWarningDialog.NO)
               {
                  autoConnect = true;
                  encrypt = false;
                  if (action == SecurityWarningDialog.ALWAYS)
                  {
                     settings.put("Connect.AllowUnencrypted." + settings.get("Connect.Server"), true); //$NON-NLS-1$ //$NON-NLS-2$
                  }
               }
            }
            else
            {
               e.getCause().printStackTrace();
               MessageDialog.openError(null, Messages.get().ApplicationWorkbenchWindowAdvisor_ConnectionError, e.getCause().getLocalizedMessage());
            }
         }
         catch(Exception e)
         {
            e.printStackTrace();
            MessageDialog.openError(null, Messages.get().ApplicationWorkbenchWindowAdvisor_Exception, e.toString()); //$NON-NLS-1$
         }
      }

      CertificateManagerProvider.dispose();

      // Suggest user to change password if it is expired
      final NXCSession session = ConsoleSharedData.getSession();
      if ((session.getAuthenticationMethod() == AuthenticationType.PASSWORD) && session.isPasswordExpired())
      {
         requestPasswordChange(loginDialog.getPassword(), session, splashService);
      }
   }

   /* (non-Javadoc)
    * @see org.netxms.certificate.request.KeyStoreLocationRequestListener#keyStoreLocationRequested()
    */
   @Override
   public String keyStoreLocationRequested()
   {
      Shell shell = Display.getCurrent().getActiveShell();

      FileDialog dialog = new FileDialog(shell);
      dialog.setText(Messages.get().ApplicationWorkbenchWindowAdvisor_CertDialogTitle);
      dialog.setFilterExtensions(new String[] { "*.p12; *.pfx" }); //$NON-NLS-1$
      dialog.setFilterNames(new String[] { Messages.get().ApplicationWorkbenchWindowAdvisor_PkcsFiles });

      return dialog.open();
   }

   /* (non-Javadoc)
    * @see org.netxms.certificate.request.KeyStorePasswordRequestListener#keyStorePasswordRequested()
    */
   @Override
   public String keyStorePasswordRequested()
   {
      return showPasswordRequestDialog(Messages.get().ApplicationWorkbenchWindowAdvisor_CertStorePassword,
            Messages.get().ApplicationWorkbenchWindowAdvisor_CertStorePasswordMsg);
   }

   /* (non-Javadoc)
    * @see org.netxms.certificate.request.KeyStoreEntryPasswordRequestListener#keyStoreEntryPasswordRequested()
    */
   @Override
   public String keyStoreEntryPasswordRequested()
   {
      return showPasswordRequestDialog(Messages.get().ApplicationWorkbenchWindowAdvisor_CertPassword,
            Messages.get().ApplicationWorkbenchWindowAdvisor_CertPasswordMsg);
   }

   /**
    * @param certMgr
    * @param cert
    * @return
    */
   private static Signature getSignature(CertificateManager certMgr, Certificate cert)
   {
      Signature sign;

      try
      {
         sign = certMgr.extractSignature(cert);
      }
      catch(Exception e)
      {
         Activator.logError("Exception in getSignature", e); //$NON-NLS-1$
         return null;
      }

      return sign;
   }
   
   /**
    * @param title
    * @param message
    * @return
    */
   private String showPasswordRequestDialog(String title, String message)
   {
      Shell shell = Display.getCurrent().getActiveShell();

      PasswordRequestDialog dialog = new PasswordRequestDialog(shell);
      dialog.setTitle(title);
      dialog.setMessage(message);
      
      if(dialog.open() == Window.OK)
      {
         return dialog.getPassword();
      }
      
      return null;
   }

   /**
    * @param currentPassword
    * @param session
    */
   private void requestPasswordChange(final String currentPassword, final NXCSession session, final ISplashService splashService)
   {
      final PasswordExpiredDialog dlg = new PasswordExpiredDialog(null, session.getGraceLogins());

      while(true)
      {
         if (dlg.open() != Window.OK)
            return;

         IRunnableWithProgress job = new IRunnableWithProgress() {
            @Override
            public void run(IProgressMonitor monitor) throws InvocationTargetException, InterruptedException
            {
               try
               {
                  monitor.setTaskName(Messages.get().ApplicationWorkbenchWindowAdvisor_ChangingPassword);
                  session.setUserPassword(session.getUserId(), dlg.getPassword(), currentPassword);
                  monitor.setTaskName(""); //$NON-NLS-1$
               }
               catch(Exception e)
               {
                  throw new InvocationTargetException(e);
               }
               finally
               {
                  monitor.done();
               }
            }
         };

         try
         {
            ModalContext.run(job, true, splashService.getProgressMonitor(), Display.getCurrent());
            MessageDialog.openInformation(null, Messages.get().ApplicationWorkbenchWindowAdvisor_Information,
                  Messages.get().ApplicationWorkbenchWindowAdvisor_PasswordChanged);
            return;
         }
         catch(InvocationTargetException e)
         {
            MessageDialog.openError(null, Messages.get().ApplicationWorkbenchWindowAdvisor_Error,
                  Messages.get().ApplicationWorkbenchWindowAdvisor_CannotChangePswd + " " + e.getCause().getLocalizedMessage()); //$NON-NLS-1$
         }
         catch(InterruptedException e)
         {
            MessageDialog.openError(null, Messages.get().ApplicationWorkbenchWindowAdvisor_Exception, e.toString());
         }
      }
   }

   /**
    * @param settings
    * @param alwaysAllow
    * @return
    */
   private int getSecurityAction(IDialogSettings settings, boolean alwaysAllow)
   {
      if (alwaysAllow)
         return SecurityWarningDialog.YES;

      return SecurityWarningDialog.showSecurityWarning(null,
                  String.format(Messages.get().ApplicationWorkbenchWindowAdvisor_NoEncryptionSupport, settings.get("Connect.Server")), //$NON-NLS-1$
                  Messages.get().ApplicationWorkbenchWindowAdvisor_NoEncryptionSupportDetails);
   }
}
