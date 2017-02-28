/**
 * 
 */
package org.netxms.ui.eclipse.market.objects;

import java.util.UUID;

/**
 * Repository loading placeholder
 */
public class LoadingPlaceholder implements MarketObject
{
   private MarketObject parent;

   /**
    * @param parent
    */
   public LoadingPlaceholder(MarketObject parent)
   {
      this.parent = parent;
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.market.objects.MarketObject#getName()
    */
   @Override
   public String getName()
   {
      return "Loading...";
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.market.objects.MarketObject#getGuid()
    */
   @Override
   public UUID getGuid()
   {
      return null;
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
      return new MarketObject[0];
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
    * @see org.netxms.ui.eclipse.market.objects.MarketObject#add(org.netxms.ui.eclipse.market.objects.MarketObject)
    */
   @Override
   public void add(MarketObject object)
   {
   }

   /* (non-Javadoc)
    * @see org.netxms.ui.eclipse.market.objects.MarketObject#setParent(org.netxms.ui.eclipse.market.objects.MarketObject)
    */
   @Override
   public void setParent(MarketObject parent)
   {
   }
}
