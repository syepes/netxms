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
package org.netxms.websvc.handlers;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import org.netxms.client.NXCException;
import org.netxms.client.NXCSession;
import org.netxms.client.constants.RCC;
import org.netxms.client.datacollection.DciData;
import org.netxms.client.datacollection.DciValue;
import org.netxms.client.objects.AbstractObject;
import org.netxms.client.objects.DataCollectionTarget;
import org.netxms.websvc.json.ResponseContainer;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Handler for /objects/{object-id}/lastvalues
 */
public class LastValues extends AbstractObjectHandler
{
   private Logger log = LoggerFactory.getLogger(LastValues.class);

   /* (non-Javadoc)
    * @see org.netxms.websvc.handlers.AbstractHandler#getCollection(java.util.Map)
    */
   @Override
   protected Object getCollection(Map<String, String> query) throws Exception
   {
      NXCSession session = getSession();
      AbstractObject obj = getObject();
      List<DciValue[]> values = new ArrayList<DciValue[]>();
      if (!(obj instanceof DataCollectionTarget) || obj.getObjectClass() == AbstractObject.OBJECT_CONTAINER)
      {
         AbstractObject[] children = obj.getChildsAsArray();
         for(AbstractObject child : children)
         {
            if (child instanceof DataCollectionTarget)
               values.add(session.getLastValues(child.getObjectId()));
         }
      }
      else if (obj instanceof DataCollectionTarget)
      {
         values.add(session.getLastValues(obj.getObjectId()));
      }
      else
         throw new NXCException(RCC.INVALID_OBJECT_ID);
      
      return new ResponseContainer("lastValues", values.toArray());
   }
}
