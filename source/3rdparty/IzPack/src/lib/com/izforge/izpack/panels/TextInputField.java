/*
 * IzPack - Copyright 2001-2008 Julien Ponge, All Rights Reserved.
 * 
 * http://izpack.org/
 * http://izpack.codehaus.org/
 * 
 * Copyright 2008 Piotr Skowronek
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 *     
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.izforge.izpack.panels;

import com.izforge.izpack.util.Debug;

import javax.swing.*;
import java.util.Map;
import java.util.List;

/*---------------------------------------------------------------------------*/
/**
 * This class is a wrapper for JTextField to allow field validation.
 * Based on RuleInputField.
 *
 * @author Piotr Skowronek
 */
/*---------------------------------------------------------------------------*/
public class TextInputField extends JComponent implements ProcessingClient
{

    /**
     *
     */
    private static final long serialVersionUID = 8611515659787697087L;

    /**
     * Validator parameters.
     */
    private Map<String, String> validatorParams;

    /**
     * Holds an instance of the <code>Validator</code> if one was specified and available
     */
    private Validator validationService;

    /**
     * This composite can only contain one component ie JTextField
     */
    private JTextField field;

    private List<ValidatorContainer> validatorContainers = null;
    private int currentValidator = 0;

    /*--------------------------------------------------------------------------*/
    /**
     * Constructs a text input field.
     *
     * @param set             A default value for field.
     * @param size            The size of the field.
     * @param validator       A string that specifies a class to perform validation services. The string
     *                        must completely identify the class, so that it can be instantiated. The class must implement
     *                        the <code>RuleValidator</code> interface. If an attempt to instantiate this class fails, no
     *                        validation will be performed.
     * @param validatorParams validator parameters.
     */
    /*--------------------------------------------------------------------------*/
    public TextInputField(String set, int size, List<ValidatorContainer> validatorContainers, Map<String, String> validatorParams)
    {
        this(set, size, validatorContainers);
        this.validatorParams = validatorParams;
    }


    /*--------------------------------------------------------------------------*/
    /**
     * Constructs a text input field.
     *
     * @param set       A default value for field.
     * @param size      The size of the field.
     * @param validator A string that specifies a class to perform validation services. The string
     *                  must completely identify the class, so that it can be instantiated. The class must implement
     *                  the <code>RuleValidator</code> interface. If an attempt to instantiate this class fails, no
     *                  validation will be performed.
     */
    /*--------------------------------------------------------------------------*/
    public TextInputField(String set, int size, List<ValidatorContainer> validatorContainers)
    {
        // ----------------------------------------------------
        // attempt to create an instance of the Validator
        // ----------------------------------------------------
        try
        {
//            if (validator != null)
//            {
//                Debug.trace("Making Validator for: " + validator);
//                validationService = (Validator) Class.forName(validator).newInstance();
//            }

            this.validatorContainers = validatorContainers;
        }
        catch (Throwable exception)
        {
            validationService = null;
            Debug.trace(exception);
        }

        com.izforge.izpack.gui.FlowLayout layout = new com.izforge.izpack.gui.FlowLayout();
        layout.setAlignment(com.izforge.izpack.gui.FlowLayout.LEADING);
        layout.setVgap(0);
        setLayout(layout);

        // ----------------------------------------------------
        // construct the UI element and add it to the composite
        // ----------------------------------------------------
        field = new JTextField(set, size);
        field.setCaretPosition(0);
        add(field);
    }


    /*--------------------------------------------------------------------------*/
    /**
     * Returns the validator parameters, if any. The caller should check for the existence of
     * validator parameters via the <code>hasParams()</code> method prior to invoking this method.
     *
     * @return a java.util.Map containing the validator parameters.
     */
//    public Map<String, String> getValidatorParams()
//    {
//        return validatorParams;
//    }

    /*---------------------------------------------------------------------------*/
    /**
     * Returns the field contents, assembled acording to the encryption and separator rules.
     *
     * @return the field contents
     */
    /*--------------------------------------------------------------------------*/
    public String getText()
    {
        return (field.getText());
    }

    // javadoc inherited
    public void setText(String value)
    {
        field.setText(value);
    }

    // javadoc inherited
    public String getFieldContents(int index)
    {
        return field.getText();
    }

    // javadoc inherited
    public int getNumFields()
    {
        // We've got only one field
        return 1;
    }
    
    @Override
    public void setToolTipText(String text) {
    	field.setToolTipText(text);
    }
    
    @Override
    public String getToolTipText() {
    	return field.getToolTipText();
    }

    @Override
    public void setEnabled(boolean enabled) {
        field.setEnabled(enabled);
    }

    @Override
    public boolean isEnabled() {
        return field.isEnabled();
    }

    @Override
    public boolean isVisible() {
        return field.isVisible();
    }

    /*--------------------------------------------------------------------------*/
    /**
     * This method validates the field content. Validating is performed through a user supplied
     * service class that provides the validation rules.
     *
     * @return <code>true</code> if the validation passes or no implementation of a validation
     *         rule exists. Otherwise <code>false</code> is returned.
     */
    /*--------------------------------------------------------------------------*/
//    public boolean validateContents()
//    {
//        if (validationService != null)
//        {
//            Debug.trace("Validating contents");
//            return (validationService.validate(this));
//        }
//        else
//        {
//            Debug.trace("Not validating contents");
//            return (true);
//        }
//    }

        /*--------------------------------------------------------------------------*/
    /**
     * This method validates the group content. Validating is performed through a user supplied
     * service class that provides the validation rules.
     *
     * @return <code>true</code> if the validation passes or no implementation of a validation
     *         rule exists. Otherwise <code>false</code> is returned.
     */
    /*--------------------------------------------------------------------------*/
    public boolean validateContents(int i)
    {
    	boolean returnValue = true;
        try
        {
            currentValidator = i;
            ValidatorContainer container = getValidatorContainer(i);
            Validator validator = container.getValidator();
            if (validator != null)
            {
                returnValue = validator.validate(this);
            }
        }
        catch (Exception e)
        {
            Debug.trace("validateContents(" + i + ") failed: " + e);
            // just return true
        }
        return returnValue;
    }

    public String getValidatorMessage(int i)
    {
        String returnValue = null;
        try
        {
            ValidatorContainer container = getValidatorContainer(i);
            if (container != null)
            {
                returnValue = container.getMessage();
            }
        }
        catch (Exception e)
        {
            Debug.trace("getValidatorMessage(" + i + ") failed: " + e);
            // just return true
        }
        return returnValue;
    }

    public int validatorSize()
    {
        int size = 0;
        if (validatorContainers != null)
        {
            size = validatorContainers.size();
        }
        return size;
    }

    public ValidatorContainer getValidatorContainer()
    {
        return getValidatorContainer(currentValidator);
    }

    public ValidatorContainer getValidatorContainer(int i)
    {
        ValidatorContainer container = null;
        try
        {
            container = validatorContainers.get(i);
        }
        catch (Exception e)
        {
            container = null;
        }
        return container;
    }

    public boolean hasParams()
    {
        return hasParams(currentValidator);
    }

    public boolean hasParams(int i)
    {
        boolean returnValue = false;
        try
        {
            ValidatorContainer container = getValidatorContainer(i);
            if (container != null)
            {
                returnValue = container.hasParams();
            }
        }
        catch (Exception e)
        {
            Debug.trace("hasParams(" + i + ") failed: " + e);
            // just return true
        }
        return returnValue;
    }

    public Map<String, String> getValidatorParams()
    {
        return getValidatorParams(currentValidator);
    }

    public Map<String, String> getValidatorParams(int i)
    {
        Map<String, String> returnValue = null;
        try
        {
            ValidatorContainer container = getValidatorContainer(i);
            if (container != null)
            {
                returnValue = container.getValidatorParams();
            }
        }
        catch (Exception e)
        {
            Debug.trace("getValidatorParams(" + i + ") failed: " + e);
            // just return true
        }
        return returnValue;
    }

    // ----------------------------------------------------------------------------
}
/*---------------------------------------------------------------------------*/
