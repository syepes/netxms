package org.netxms.ui.eclipse.applauncher;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import javax.annotation.PostConstruct;
import javax.inject.Inject;
import org.eclipse.e4.core.contexts.IEclipseContext;
import org.eclipse.e4.ui.model.application.MApplication;
import org.eclipse.e4.ui.model.application.ui.MElementContainer;
import org.eclipse.e4.ui.model.application.ui.MUIElement;
import org.eclipse.e4.ui.model.application.ui.advanced.MPerspective;
import org.eclipse.e4.ui.model.application.ui.advanced.MPerspectiveStack;
import org.eclipse.e4.ui.model.application.ui.basic.MWindow;
import org.eclipse.e4.ui.workbench.modeling.EModelService;
import org.eclipse.e4.ui.workbench.modeling.EPartService;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Label;
import org.eclipse.ui.forms.events.HyperlinkAdapter;
import org.eclipse.ui.forms.events.HyperlinkEvent;
import org.eclipse.ui.forms.widgets.Hyperlink;
import org.eclipse.ui.internal.dialogs.PropertyPageNode;

public class MainMenu
{
   private final Color COLOR_BACKGROUND = new Color(Display.getCurrent(), 48, 48, 48);
   private final Color COLOR_PERSPECTIVE = new Color(Display.getCurrent(), 200, 200, 200);
   
   @Inject
   private MWindow window;
   
   private Composite buttonArea;

   @Inject
   public MainMenu()
   {
   }

   @PostConstruct
   public void createContent(Composite parent)
   {
      parent.setLayout(new FillLayout());
      
      Composite content = new Composite(parent, SWT.NONE);
      GridLayout layout = new GridLayout();
      content.setLayout(layout);
      content.setBackground(COLOR_BACKGROUND);
      
      List<MPerspective> perspectives = new ArrayList<MPerspective>();
      List<MPerspectiveStack> appPerspectiveStacks = getMatchingChildren(window, MPerspectiveStack.class);
      for(MPerspectiveStack s : appPerspectiveStacks)
         for(MPerspective p : s.getChildren())
         {
            perspectives.add(p);
         }
      Collections.sort(perspectives, new Comparator<MPerspective>() {
         @Override
         public int compare(MPerspective p1, MPerspective p2)
         {
            int pri1 = getPerspectivePriority(p1);
            int pri2 = getPerspectivePriority(p2);
            
            // if both have no priority or equal priority, compare by name
            if (pri1 == pri2)
               return p1.getLabel().compareToIgnoreCase(p2.getLabel());
            
            // perspective with priority wins
            if (pri1 == -1)
               return 1;

            if (pri2 == -1)
               return -1;
            
            return pri1 - pri2;
         }
      });

      buttonArea = new Composite(content, SWT.NONE);
      buttonArea.setBackground(content.getBackground());
      layout = new GridLayout();
      layout.numColumns = perspectives.size();
      layout.horizontalSpacing = 30;
      buttonArea.setLayout(layout);
      GridData gd = new GridData();
      gd.grabExcessHorizontalSpace = true;
      gd.horizontalAlignment = SWT.LEFT;
      gd.grabExcessVerticalSpace = true;
      gd.verticalAlignment = SWT.CENTER;
      buttonArea.setLayoutData(gd);
      
      for(MPerspective p : perspectives)
         createPerspectiveButton(p);
   }
   
   private void createPerspectiveButton(MPerspective p)
   {
      Hyperlink b = new Hyperlink(buttonArea, SWT.NONE);
      b.setText(p.getLabel().toUpperCase());
      b.setBackground(buttonArea.getBackground());
      b.setForeground(COLOR_PERSPECTIVE);
      final String id = p.getElementId();
      b.addHyperlinkListener(new HyperlinkAdapter() {
         @Override
         public void linkActivated(HyperlinkEvent e)
         {
            switchPerspective(id);
         }
      });
   }
   
   private void switchPerspective(String id)
   {
      IEclipseContext context = window.getContext();
      MApplication application = context.get(MApplication.class);
      EPartService partService = context.get(EPartService.class);
      EModelService modelService = context.get(EModelService.class);
      
      List<MPerspective> perspectives = modelService.findElements(application, id, MPerspective.class, null);
      partService.switchPerspective(perspectives.get(0));
   }
   
   /**
    * Extract page priority from perspective id. Expects perspective id to be in format id#priority. If priority is not given,
    * -1 is returned.
    * 
    * @param p perspective
    * @return extracted priority
    */
   private static int getPerspectivePriority(MPerspective p)
   {
      int idx = p.getElementId().indexOf('#');
      if (idx == -1)
         return -1;
      try
      {
         return Integer.parseInt(p.getElementId().substring(idx + 1));
      }
      catch(Exception e)
      {
         return -1;
      }
   }

   @SuppressWarnings("unchecked")
   private static <T extends MUIElement> List<T> getMatchingChildren(MElementContainer<?> container, Class<T> type)
   {
      List<T> matchingChildren = new ArrayList<>();

      for(Object child : container.getChildren())
      {
         if (type.isInstance(child))
            matchingChildren.add((T)child);
      }

      return matchingChildren;
   }
}
