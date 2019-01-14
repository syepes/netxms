/**
 * NetXMS - open source network management system
 * Copyright (C) 2003-2010 Victor Kirhenshtein
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
package org.netxms.client.objects;

import java.util.UUID;
import org.netxms.base.NXCPCodes;
import org.netxms.base.NXCPMessage;
import org.netxms.client.NXCSession;

/**
 * Generic agent policy object
 *
 */
public class AgentPolicy
{   
   private UUID guid;
   private String name;
	private int version;
	private int policyType;
	private int flags;
	private String content;
	
	/**
	 * @param msg
	 * @param session
	 */
	public AgentPolicy(NXCPMessage msg, NXCSession session)
	{		
		guid = msg.getFieldAsUUID(NXCPCodes.VID_GUID);
		name = msg.getFieldAsString(NXCPCodes.VID_NAME);
		policyType = msg.getFieldAsInt32(NXCPCodes.VID_POLICY_TYPE);
		version = msg.getFieldAsInt32(NXCPCodes.VID_VERSION);
		flags = msg.getFieldAsInt32(NXCPCodes.VID_FLAGS);
		content = msg.getFieldAsString(NXCPCodes.VID_CONFIG_FILE_DATA);
	}

	/**
	 * @return the version
	 */
	public int getVersion()
	{
		return version;
	}

	/**
	 * @return the policyType
	 */
	public int getPolicyType()
	{
		return policyType;
	}

   /**
    * @return the flags
    */
   public int getFlags()
   {
      return flags;
   }

   /**
    * @return the guid
    */
   public UUID getGuid()
   {
      return guid;
   }

   /**
    * @return the name
    */
   public String getName()
   {
      return name;
   }

   /**
    * @return the content
    */
   public String getContent()
   {
      return content;
   }
}
