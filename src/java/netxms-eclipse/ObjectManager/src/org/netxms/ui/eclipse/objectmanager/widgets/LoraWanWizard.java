/**
 * NetXMS - open source network management system
 * Copyright (C) 2017 Raden Solutions
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
package org.netxms.ui.eclipse.objectmanager.widgets;

import org.eclipse.jface.wizard.IWizard;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.netxms.client.MacAddress;
import org.netxms.client.MacAddressFormatException;
import org.netxms.client.objects.Sensor;
import org.netxms.client.sensor.configs.LoraWanConfig;
import org.netxms.ui.eclipse.tools.WidgetHelper;
import org.netxms.ui.eclipse.widgets.LabeledText;
/**
 * LoRaWAN configuration
 */
public class LoraWanWizard extends Composite
{
   private Combo comboRegistrationType;
   private LabeledText field1;
   private LabeledText field2;
   private LabeledText field3;
   private LoraWanConfig conf;
   
   /**
    * Common sensor changeable field constructor
    *  
    * @param parent
    * @param style
    */
   public LoraWanWizard(Composite parent, int style, LoraWanConfig conf, final IWizard wizard)
   {
      super(parent, style);     
      this.conf = conf;
      
      GridLayout layout = new GridLayout();
      layout.marginHeight = 0;
      layout.marginWidth = 0;
      setLayout(layout);
      
      comboRegistrationType = WidgetHelper.createLabeledCombo(parent, SWT.BORDER | SWT.READ_ONLY, "Registration type", 
            WidgetHelper.DEFAULT_LAYOUT_DATA);
      String[] items = {"OTAA", "APB"};
      comboRegistrationType.setItems(items);
      comboRegistrationType.select(0);
      comboRegistrationType.addModifyListener(new ModifyListener() {
         
         @Override
         public void modifyText(ModifyEvent e)
         {
            if(comboRegistrationType.getSelectionIndex() == 0)
            {
               field1.setLabel("DevEUI");
               field2.setLabel("AppEUI");
               field3.setLabel("AppKey");
            }
            else
            {
               field1.setLabel("DevAddr");
               field2.setLabel("NwkSKey");
               field3.setLabel("AppSKey");
            }
            if(wizard != null)
               wizard.getContainer().updateButtons();
         }
      });
      comboRegistrationType.setEnabled(wizard != null);
      
      field1 = new LabeledText(parent, SWT.NONE);
      field1.setLabel("DevEUI");
      field1.setText("");
      field1.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false));
      field1.getTextControl().addModifyListener(new ModifyListener() {
         @Override
         public void modifyText(ModifyEvent e)
         {
            if(wizard != null)
               wizard.getContainer().updateButtons();
         }
      });
      field1.setEditable(wizard != null);
      
      field2 = new LabeledText(parent, SWT.NONE);
      field2.setLabel("AppEUI");
      field2.setText("");
      field2.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false));
      field2.getTextControl().addModifyListener(new ModifyListener() {
         @Override
         public void modifyText(ModifyEvent e)
         {
            if(wizard != null)
               wizard.getContainer().updateButtons();
         }
      });
      field2.setEditable(wizard != null);
      
      field3 = new LabeledText(parent, SWT.NONE);
      field3.setLabel("AppKey");
      field3.setText("");
      field3.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false));
      field3.getTextControl().addModifyListener(new ModifyListener() {
         @Override
         public void modifyText(ModifyEvent e)
         {
            if(wizard != null)
               wizard.getContainer().updateButtons();
         }
      });
      field3.setEditable(wizard != null);
   }
   
   public boolean validate()
   {
      if(comboRegistrationType.getSelectionIndex() == 0)
      {
         return field1.getText().length() == 16 && field2.getText().length() == 16 &&  field3.getText().length() == 32;
      }
      return field1.getText().length() == 8 && field2.getText().length() == 32 &&  field3.getText().length() == 32;         
   }
   
   public String getConfig()
   {
      LoraWanConfig conf = new LoraWanConfig();
      conf.registrationType = comboRegistrationType.getSelectionIndex();
      if(comboRegistrationType.getSelectionIndex() == 0)
      {
         conf.DevEUI = field1.getText();
         conf.AppEUI = field2.getText();
         conf.AppKey = field3.getText();
      }
      else
      {
         conf.DevAddr = field1.getText();
         conf.NwkSKey = field2.getText();
         conf.AppSKey = field3.getText();
      }
      try
      {
         return conf.createXml();
      }
      catch(Exception e)
      {
         e.printStackTrace();
         return null;
      }
   }
}
