package org.netxms.ui.eclipse.market.objects;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.UUID;
import org.json.JSONArray;
import org.json.JSONObject;
import org.netxms.ui.eclipse.market.objects.helpers.RepositoryObjectInstance;

/**
 * Config repository package
 */
public class RepositoryPackage implements MarketObject, RepositoryObject
{
   private UUID guid;
   private String name;
   private MarketObject parent;
   private List<RepositoryObjectInstance> instances;
   private List<RepositoryObject> objects = new ArrayList<RepositoryObject>();
   private RepositoryRuntimeInfo repository;
   private boolean marked;
   
   /**
    * Create package from JSON object
    * 
    * @param guid
    * @param json
    */
   public RepositoryPackage(UUID guid, JSONObject json, RepositoryRuntimeInfo repository)
   {
      this.guid = guid;
      name = json.getString("name");
      parent = null;
      marked = false;
      this.repository = repository;
      
      JSONArray a = json.getJSONArray("instances");
      if (a != null)
      {
         instances = new ArrayList<RepositoryObjectInstance>(a.length());
         for(int i = 0; i < a.length(); i++)
         {
            instances.add(new RepositoryObjectInstance(a.getJSONObject(i)));
         }
         Collections.sort(instances, new Comparator<RepositoryObjectInstance>() {
            @Override
            public int compare(RepositoryObjectInstance o1, RepositoryObjectInstance o2)
            {
               return o2.getVersion() - o1.getVersion();
            }
         });
      }
      else
      {
         instances = new ArrayList<RepositoryObjectInstance>(0);
      }
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.market.objects.MarketObject#getName()
    */
   @Override
   public String getName()
   {
      return name;
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.market.objects.MarketObject#getGuid()
    */
   @Override
   public UUID getGuid()
   {
      return guid;
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.market.objects.MarketObject#getParent()
    */
   @Override
   public MarketObject getParent()
   {
      return parent;
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.market.objects.MarketObject#getChildren()
    */
   @Override
   public MarketObject[] getChildren()
   {
      return objects.toArray(new MarketObject[objects.size()]);
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.market.objects.MarketObject#hasChildren()
    */
   @Override
   public boolean hasChildren()
   {
      return false;
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.market.objects.RepositoryObject#isMarked()
    */
   @Override
   public boolean isMarked()
   {
      return marked;
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.market.objects.RepositoryObject#setMarked(boolean)
    */
   @Override
   public void setMarked(boolean marked)
   {
      this.marked = marked;
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.market.objects.RepositoryObject#getInstances()
    */
   @Override
   public List<RepositoryObjectInstance> getInstances()
   {
      return instances;
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.market.objects.RepositoryObject#getActualInstance()
    */
   @Override
   public RepositoryObjectInstance getActualInstance()
   {
      return instances.isEmpty() ? null : instances.get(0);
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.market.objects.RepositoryObject#getActualVersion()
    */
   @Override
   public int getActualVersion()
   {
      return instances.isEmpty() ? 0 : instances.get(0).getVersion();
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.market.objects.MarketObject#add(org.netxms.ui.eclipse.market.objects.MarketObject)
    */
   public void add(MarketObject object)
   {
      if (object instanceof RepositoryObject)
         objects.add((RepositoryObject)object);
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.market.objects.MarketObject#setParent(org.netxms.ui.eclipse.market.objects.MarketObject)
    */
   @Override
   public void setParent(MarketObject parent)
   {
      this.parent = parent;
   }
   
   /**
    * Load package contents
    * @return success
    * @throws Exception 
    */
   public void loadContents() throws Exception
   {
      objects = repository.loadRepositoryPackageContent();
   }
}
