package org.netxms.ui.eclipse.market.dialogs;

import java.util.Arrays;
import java.util.List;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.jface.dialogs.Dialog;
import org.eclipse.jface.util.LocalSelectionTransfer;
import org.eclipse.jface.viewers.ArrayContentProvider;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.swt.SWT;
import org.eclipse.swt.dnd.DND;
import org.eclipse.swt.dnd.DragSourceEvent;
import org.eclipse.swt.dnd.DragSourceListener;
import org.eclipse.swt.dnd.DropTargetEvent;
import org.eclipse.swt.dnd.DropTargetListener;
import org.eclipse.swt.dnd.Transfer;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Shell;
import org.netxms.ui.eclipse.console.Activator;
import org.netxms.ui.eclipse.jobs.ConsoleJob;
import org.netxms.ui.eclipse.market.dialogs.helpers.PackageManagerContentProvider;
import org.netxms.ui.eclipse.market.objects.MarketObject;
import org.netxms.ui.eclipse.market.objects.RepositoryPackage;
import org.netxms.ui.eclipse.market.objects.RepositoryRuntimeInfo;
import org.netxms.ui.eclipse.market.views.helpers.RepositoryLabelProvider;
import org.netxms.ui.eclipse.widgets.SortableTableViewer;
import org.netxms.ui.eclipse.widgets.SortableTreeViewer;

public class PackageManagerDialog extends Dialog
{
   private Composite dialogArea;
   private SortableTableViewer packageViewer;
   private SortableTreeViewer elementViewer;
   private RepositoryPackage pkg;
   
   /**
    * @param parentShell
    */
   public PackageManagerDialog(Shell parentShell, RepositoryPackage pkg)
   {
      super(parentShell);
      setShellStyle(getShellStyle() | SWT.RESIZE);
      this.pkg = pkg;
   }

   /* (non-Javadoc)
    * @see org.eclipse.jface.dialogs.Dialog#createDialogArea(org.eclipse.swt.widgets.Composite)
    */
   @Override
   protected Control createDialogArea(Composite parent)
   {
      dialogArea = (Composite)super.createDialogArea(parent);
      
      GridLayout layout = new GridLayout(2, false);
      dialogArea.setLayout(layout);
      
      final String[] columnNames = { "Name", "Version", "GUID" };
      final int[] columnWidths = { 200, 100, 200 };
      
      packageViewer = new SortableTableViewer(dialogArea, columnNames, columnWidths, 0, SWT.UP, SWT.NONE);
      packageViewer.setContentProvider(new ArrayContentProvider());
      packageViewer.setLabelProvider(new RepositoryLabelProvider());
      
      GridData gd = new GridData();
      gd.horizontalAlignment = GridData.FILL;
      gd.verticalAlignment = GridData.FILL;
      gd.grabExcessHorizontalSpace = true;
      packageViewer.getTable().setLayoutData(gd);
      
      elementViewer = new SortableTreeViewer(dialogArea, columnNames, columnWidths, 0, SWT.UP, SWT.NONE);
      elementViewer.setContentProvider(new PackageManagerContentProvider());
      elementViewer.setLabelProvider(new RepositoryLabelProvider());
      gd = new GridData();
      gd.horizontalAlignment = GridData.FILL;
      gd.verticalAlignment = GridData.FILL;
      gd.grabExcessHorizontalSpace = true;
      elementViewer.getTree().setLayoutData(gd);
      
      enableDragAndDropSupport();
      setInput();
      
      return dialogArea;
   }
   
   /**
    * Enable drag and drop support
    */
   private void enableDragAndDropSupport()
   {
      final Transfer[] transfers = new Transfer[] { LocalSelectionTransfer.getTransfer() };
      
      elementViewer.addDragSupport(DND.DROP_COPY, transfers, new DragSourceListener() {
         
         @Override
         public void dragStart(DragSourceEvent event)
         {
            LocalSelectionTransfer.getTransfer().setSelection(elementViewer.getSelection());
            event.doit = true;
         }
         
         @Override
         public void dragSetData(DragSourceEvent event)
         {
            event.data = LocalSelectionTransfer.getTransfer().getSelection();
         }
         
         @Override
         public void dragFinished(DragSourceEvent event)
         {
         }
      });
      packageViewer.addDropSupport(DND.DROP_COPY, transfers, new DropTargetListener() {
         
         @Override
         public void dropAccept(DropTargetEvent event)
         {
         }
         
         @Override
         public void drop(DropTargetEvent event)
         {
            if (!(event.data instanceof IStructuredSelection))
               return;
            
            Object object = ((IStructuredSelection)event.data).getFirstElement();
            if (!(object instanceof MarketObject))
               return;
            
            @SuppressWarnings("unchecked")
            List<MarketObject> elements = ((IStructuredSelection)event.data).toList();
            
            for(MarketObject o : elements)
               pkg.add(o);
            
            packageViewer.add(pkg.getChildren());
         }
         
         @Override
         public void dragOver(DropTargetEvent event)
         {
         }
         
         @Override
         public void dragOperationChanged(DropTargetEvent event)
         {
         }
         
         @Override
         public void dragLeave(DropTargetEvent event)
         {
         }
         
         @Override
         public void dragEnter(DropTargetEvent event)
         {
         }
      });
   }
   
   /**
    * Set input for package manager
    */
   private void setInput()
   {
      new ConsoleJob("Get package contents", null, Activator.PLUGIN_ID, null) {         
         @Override
         protected void runInternal(IProgressMonitor monitor) throws Exception
         {
            pkg.loadContents();
            final List<MarketObject> contents = Arrays.asList(pkg.getChildren());
            final List<MarketObject> elements = Arrays.asList(((RepositoryRuntimeInfo)pkg.getParent().getParent()).getElements());
            runInUIThread(new Runnable() {
               
               @Override
               public void run()
               {
                  packageViewer.setInput(contents);
                  elementViewer.setInput(elements);                 
               }
            });
         }
         
         @Override
         protected String getErrorMessage()
         {
            return "Cannot get package contents";
         }
      }.start();
   }

   /* (non-Javadoc)
    * @see org.eclipse.jface.dialogs.Dialog#okPressed()
    */
   @Override
   protected void okPressed()
   {
      super.okPressed();
   }

   /* (non-Javadoc)
    * @see org.eclipse.jface.dialogs.Dialog#cancelPressed()
    */
   @Override
   protected void cancelPressed()
   {
      super.cancelPressed();
   }
}
