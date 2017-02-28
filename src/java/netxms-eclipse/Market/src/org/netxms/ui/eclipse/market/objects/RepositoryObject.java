package org.netxms.ui.eclipse.market.objects;

import java.util.List;
import org.netxms.ui.eclipse.market.objects.helpers.RepositoryObjectInstance;

/**
 * Common interface for all repository objects
 */
public interface RepositoryObject extends MarketObject
{   
   /**
    * @return the marked
    */
   public boolean isMarked();
   
   /**
    * @param marked the marked to set
    */
   public void setMarked(boolean marked);
   
   /**
    * Get all instances of this element
    * 
    * @return
    */
   public List<RepositoryObjectInstance> getInstances();
   
   /**
    * Get most actual instance of this element
    * 
    * @return
    */
   public RepositoryObjectInstance getActualInstance();
   
   /**
    * Get version of most actual instance
    * 
    * @return
    */
   public int getActualVersion();
}
