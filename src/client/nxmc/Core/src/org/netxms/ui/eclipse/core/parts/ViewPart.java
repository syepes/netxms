
package org.netxms.ui.eclipse.core.parts;

import javax.annotation.PostConstruct;
import javax.annotation.PreDestroy;
import javax.inject.Inject;
import org.eclipse.e4.ui.di.Focus;
import org.eclipse.e4.ui.model.application.ui.basic.MPart;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Shell;
import org.netxms.ui.eclipse.core.ViewManager;
import org.netxms.ui.eclipse.core.views.View;

public class ViewPart
{
   private MPart part;
   private View view = null;
   
   @Inject
   public ViewPart()
   {
   }

   @PostConstruct
   public void createContent(Composite parent, MPart part, Shell shell)
   {
      this.part = part;
      String viewId = part.getProperties().get("org.netxms.ui.eclipse.viewId");
      if (viewId != null)
      {
         view = ViewManager.getInstance().createView(viewId);
         if (view != null)
         {
            view.init(part, shell);
            parent.setLayout(new FillLayout());
            view.createContent(parent);
         }
      }
   }

   @Focus
   public void onFocus()
   {
      if (view != null)
         view.setFocus();
   }
   
   @PreDestroy
   public void dispose()
   {
      if (view != null)
         view.dispose();
   }
}
