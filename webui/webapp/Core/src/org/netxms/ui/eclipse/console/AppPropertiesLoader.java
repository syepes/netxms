/**
 * 
 */
package org.netxms.ui.eclipse.console;

import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;
import javax.servlet.ServletContext;

import org.eclipse.rap.rwt.RWT;

/**
 * Loader class for application properties
 */
public class AppPropertiesLoader
{
	/**
	 * Read application properties from nxmc.properties
	 */
	public Properties load()
	{
		Properties properties = new Properties();
		InputStream in = null;
		try
		{
			in = getClass().getResourceAsStream("/nxmc.properties"); //$NON-NLS-1$
			if (in == null) {
				ServletContext context = RWT.getRequest().getSession().getServletContext();
				if (context != null) {
					in = context.getResourceAsStream("/nxmc.properties"); //$NON-NLS-1$
				}
			}
			if (in != null) {
				properties.load(in);
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		finally
		{
			if (in != null)
			{
				try
				{
					in.close();
				}
				catch(IOException e)
				{
				}
			}
		}
		return properties;
	}
}
