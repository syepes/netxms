package org.netxms.ui.eclipse.applauncher;

import org.eclipse.osgi.util.NLS;

public class Messages extends NLS
{
	private static final String BUNDLE_NAME = "org.netxms.ui.eclipse.applauncher.messages"; //$NON-NLS-1$
	
	public static String LoginDialog_Auth;
   public static String LoginDialog_Cert;
   public static String LoginDialog_Error;
	public static String LoginDialog_login;
   public static String LoginDialog_NoCertSelected;
   public static String LoginDialog_Passwd;
	public static String LoginDialog_server;
   public static String LoginDialog_SlowLinkConnection;
	public static String LoginDialog_title;
   public static String LoginDialog_Warning;
   public static String LoginDialog_WrongKeyStorePasswd;
	public static String ApplicationWorkbenchAdvisor_CommunicationError;
	public static String ApplicationWorkbenchAdvisor_ConnectionLostMessage;
	public static String ApplicationWorkbenchAdvisor_OKToCloseMessage;
	public static String ApplicationWorkbenchAdvisor_ServerShutdownMessage;
   public static String ApplicationWorkbenchAdvisor_SessionTerminated;
	public static String ApplicationWorkbenchWindowAdvisor_CannotChangePswd;
   public static String ApplicationWorkbenchWindowAdvisor_CannotOpenDashboard;
   public static String ApplicationWorkbenchWindowAdvisor_CannotOpenDashboardType2;
	public static String ApplicationWorkbenchWindowAdvisor_CertDialogTitle;
   public static String ApplicationWorkbenchWindowAdvisor_CertPassword;
   public static String ApplicationWorkbenchWindowAdvisor_CertPasswordMsg;
   public static String ApplicationWorkbenchWindowAdvisor_CertStorePassword;
   public static String ApplicationWorkbenchWindowAdvisor_CertStorePasswordMsg;
   public static String ApplicationWorkbenchWindowAdvisor_ChangingPassword;
	public static String ApplicationWorkbenchWindowAdvisor_ConnectionError;
	public static String ApplicationWorkbenchWindowAdvisor_Exception;
   public static String ApplicationWorkbenchWindowAdvisor_NoEncryptionSupport;
   public static String ApplicationWorkbenchWindowAdvisor_NoEncryptionSupportDetails;
	public static String ApplicationWorkbenchWindowAdvisor_PasswordChanged;
   public static String ApplicationWorkbenchWindowAdvisor_PkcsFiles;
   public static String ApplicationWorkbenchWindowAdvisor_Error;
	public static String ApplicationWorkbenchWindowAdvisor_Information;
   public static String SecurityWarningDialog_DontAskAgain;
   public static String SecurityWarningDialog_Title;

   static
	{
		// initialize resource bundle
		NLS.initializeMessages(BUNDLE_NAME, Messages.class);
	}

	private Messages()
	{
 }


	private static Messages instance = new Messages();

	public static Messages get()
	{
		return instance;
	}

}
