
package org.netxms.ui.eclipse.applauncher;

import java.util.ArrayList;
import java.util.List;
import javax.annotation.PostConstruct;
import javax.inject.Inject;
import org.eclipse.e4.ui.model.application.ui.MElementContainer;
import org.eclipse.e4.ui.model.application.ui.MUIElement;
import org.eclipse.e4.ui.model.application.ui.advanced.MPerspective;
import org.eclipse.e4.ui.model.application.ui.advanced.MPerspectiveStack;
import org.eclipse.e4.ui.model.application.ui.basic.MWindow;
import org.eclipse.e4.ui.workbench.modeling.EModelService;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;

public class NavigationPane
{
   @Inject
   EModelService modelService;

   @Inject
   public NavigationPane()
   {

   }

   @PostConstruct
   public void postConstruct(Composite parent)
   {
      Composite content = new Composite(parent, SWT.NONE);
      content.setLayout(new GridLayout());
   }
}
