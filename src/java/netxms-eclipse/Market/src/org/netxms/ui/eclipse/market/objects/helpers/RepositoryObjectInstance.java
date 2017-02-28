package org.netxms.ui.eclipse.market.objects.helpers;

import java.util.Date;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * Repository object`s instance
 */
public class RepositoryObjectInstance
{
   private Date timestamp;
   private int version;
   private String comments;
   
   /**
    * Create instance from JSON object
    * 
    * @param json
    */
   public RepositoryObjectInstance(JSONObject json)
   {
      timestamp = new Date(json.getLong("timestamp") * 1000L);
      version = json.getInt("version");
      try
      {
         comments = json.getString("comment");
      }
      catch(JSONException e)
      {
         comments = "";
      }
   }

   /**
    * @return the timestamp
    */
   public Date getTimestamp()
   {
      return timestamp;
   }

   /**
    * @return the version
    */
   public int getVersion()
   {
      return version;
   }

   /**
    * @return the comments
    */
   public String getComments()
   {
      return comments;
   }
}