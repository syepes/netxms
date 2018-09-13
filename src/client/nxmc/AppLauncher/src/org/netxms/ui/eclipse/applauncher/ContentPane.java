 
package org.netxms.ui.eclipse.applauncher;

import javax.inject.Inject;
import javax.annotation.PostConstruct;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;

public class ContentPane {
	@Inject
	public ContentPane() {
		
	}
	
	@PostConstruct
	public void postConstruct(Composite parent) {
      Composite content = new Composite(parent, SWT.NONE);
      content.setLayout(new GridLayout());
      new Label(content, SWT.NONE).setText("Content");
	}
	
	
	
	
}