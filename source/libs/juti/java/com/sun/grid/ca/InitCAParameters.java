/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 *
 *  Sun Microsystems Inc., March, 2001
 *
 *
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *   Copyright: 2001 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.ca;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.MissingResourceException;
import javax.security.auth.callback.Callback;
import javax.security.auth.callback.CallbackHandler;
import javax.security.auth.callback.NameCallback;
import javax.security.auth.callback.TextOutputCallback;
import javax.security.auth.callback.UnsupportedCallbackException;

/**
 * This class hold the parameters for Certificate Authority
 */
public class InitCAParameters implements java.io.Serializable {
    
    /**
     *  Defines a parameter for the init ca command.
     */
    public static class ParamDef {
        
        private final static char [] FORBIDDEN_LETTERS = {
            '"', '\''
        };
        
        private String name;
        private Integer minLen;
        private Integer maxLen;
        
        private ParamDef(String name) {
            this.name = name;
            minLen = new Integer(RB.getString("initCAParam." + name + ".minLen"));
            maxLen = new Integer(RB.getString("initCAParam." + name + ".maxLen"));
        }
        
        /**
         * Get name of this parameter
         * @return name of this parameter
         */
        public String getName() {
            return name;
        }
        
        /**
         *  Get the localized name of this parameter
         *
         *  @return the localized name of this parameter
         */
        public String getLocalizedName() {
            return RB.getString("initCAParam." + name + ".name");
        }
        
        /**
         * Get the description of this parameter
         * @return the description
         */
        public String getDescription() {
            try {
                return RB.getString("initCAParam." + name + ".description");
            } catch(MissingResourceException ex) {
                return null;
            }
        }
        
        /**
         * Get the default value of this param
         * @return the default value of this param
         */
        public String getDefaultValue() {
            try {
                return RB.getString("initCAParam." + name + ".default");
            } catch(MissingResourceException ex) {
                return null;
            }
        }
        
        /**
         * Get the minimum number of character for this parameter
         * @return minimum number of character for this parameter
         */
        public Integer getMinLen() {
            return minLen;
        }
        
        /**
         * Get the maximum number of characters for this parameter
         * @return maximum number of characters for this parameter
         */
        public Integer getMaxLen() {
            return maxLen;
        }
        
        private NameCallback createCallback() {
            
            String description = getDescription();
            String prompt = null;
            if(description == null) {
                prompt = RB.getString("initCAParam.prompt", new Object[] { getLocalizedName() });
            } else {
                prompt = RB.getString("initCAParam.promptWithDescription",
                        new Object [] { getLocalizedName(), description });
                
            }
            String defaultValue = getDefaultValue();
            
            if(defaultValue == null) {
                return new NameCallback(prompt);
            } else {
                return new NameCallback(prompt, defaultValue);
            }
        }
        
        /**
         * Check the validity of a param value
         * @param value 
         * @throws com.sun.grid.ca.GridCAException 
         */
        public void check(String value) throws GridCAException {
            if(value == null) {
                throw RB.newGridCAException("initCAParam.empty", name);
            }
            if(value.length() < minLen.intValue()) {
                throw RB.newGridCAException("initCAParam.tooShort", new Object [] { getLocalizedName(), value, minLen });
            }
            if(value.length() > maxLen.intValue()) {
                throw RB.newGridCAException("initCAParam.tooLong", new Object [] { getLocalizedName(), value, maxLen });
            }
            
            for(int i = 0; i < FORBIDDEN_LETTERS.length; i++) {
                if(value.indexOf(FORBIDDEN_LETTERS[i]) >= 0) {
                    throw RB.newGridCAException("gridCAParam.forbiddenLetter",
                            new Object [] { getLocalizedName(), value, new Character(FORBIDDEN_LETTERS[i]) });
                }
            }
        }
        
        public boolean equals(Object obj) {
            return obj instanceof ParamDef &&
                    name.equals(((ParamDef)obj).name);
        }
        
        public int hashCode() {
            return name.hashCode();
        }
        
    }
    
    /** Definition of the country parameter */
    public final static ParamDef COUNTRY = new ParamDef("country");
    
    /** Definition of the state parameter */
    public final static ParamDef STATE = new ParamDef("state");
    
    /** Definition of the location parameter */
    public final static ParamDef LOCATION = new ParamDef("location");
    
    /** Definition of the organisation parameter */
    public final static ParamDef ORG = new ParamDef("organisation");
    
    /** Definition of the organisation unit parameter */
    public final static ParamDef ORG_UNIT = new ParamDef("organisationUnit");
    
    /** Definition of the admin email address parameter */
    public final static ParamDef ADMIN_EMAIL = new ParamDef("adminEmailAddress");
    
    private final static ParamDef [] PARAMS = {
        COUNTRY, STATE, LOCATION, ORG, ORG_UNIT, ADMIN_EMAIL
    };
    
    private Map values = new HashMap();
    
    public InitCAParameters() {
        
    }
    
    /**
     * Query the values for a init ca command.
     *
     * @param callbackHandler   the callback handler which is used to query the values
     * @throws java.io.IOException if the callback handler throws a IOException
     * @throws javax.security.auth.callback.UnsupportedCallbackException if the the callback handler does not 
     *            support <code>NameCallback</code> or <code>TextOutputCallback</code>
     * @return the <code>InitCAParameters</code> object with the queried values
     */
    public static InitCAParameters queryNewInstance(CallbackHandler callbackHandler) throws IOException, UnsupportedCallbackException {
        
        InitCAParameters params = new InitCAParameters();
        
        Callback [] cb = new Callback[1];
        Callback [] ecb = new Callback[1];
        for(int i = 0; i < PARAMS.length; i++) {
            cb[0] = PARAMS[i].createCallback();
           do {
                callbackHandler.handle(cb);
                try {
                    params.setValue(PARAMS[i], ((NameCallback)cb[0]).getName());
                    break;
                } catch (GridCAException ex) {
                    ecb[0] = new TextOutputCallback(TextOutputCallback.ERROR, ex.getLocalizedMessage());
                    callbackHandler.handle(ecb);
                }
           } while(true);
           
            
        }
        return params;
    }
    
    
    private String getValue(ParamDef param) {
        return (String)values.get(param);
    }
    
    private void setValue(ParamDef param, String value) throws GridCAException {
        param.check(value);
        values.put(param, value);
    }
    
    /**
     * Get the country of the ca.
     * @return the country of the ca
     */
    public String getCountry() {
        return getValue(COUNTRY);
    }
    
    /**
     * Set the country of the ca (e.g. Germany)
     * @param country country of the ca (exact two characters)
     * @throws com.sun.grid.ca.GridCAException if <code>country</code> is not valid
     */
    public void setCountry(String country) throws GridCAException {
        setValue(COUNTRY, country);
    }
    
    /**
     * Get the state of the ca.
     * @return the state of the ca
     */
    public String getState() {
        return getValue(STATE);
    }
    
    /**
     * Set the state of the ca (e.g. Bayern)
     * @param state state of the ca (at least one char)
     * @throws com.sun.grid.ca.GridCAException if <code>state</code> is not valid
     */
    public void setState(String state) throws GridCAException {
        setValue(STATE, state);
    }
    
    /**
     * Get the location of the ca.
     * @return the location of the ca
     */
    public String getLocation() {
        return getValue(LOCATION);
    }
    
    /**
     * Set the location of the ca (e.g. city)
     * @param location location of the ca (at least one char)
     * @throws com.sun.grid.ca.GridCAException if <code>location</code> is not valid
     */
    public void setLocation(String location) throws GridCAException {
        setValue(LOCATION, location);
    }
    
    /**
     * Get the organization of the ca.
     * @return the organization of the ca
     */
    public String getOrganization() {
        return getValue(ORG);
    }
    
    /**
     * Set the organization of the ca.
     * @param organization organization of the ca (at least one char)
     * @throws com.sun.grid.ca.GridCAException if <code>organization</code> is not valid
     */
    public void setOrganization(String organization) throws GridCAException {
        setValue(ORG, organization);
    }
    
    /**
     * Get the organization unit of the ca.
     * @return the organization unit of the ca
     */
    public String getOrganizationUnit() {
        return getValue(ORG_UNIT);
    }
    
    /**
     * Set the organization unit of the ca.
     * @param organizationUnit organization unit of the ca (at least one char)
     * @throws com.sun.grid.ca.GridCAException if <code>organizationUnit</code> is not valid
     */
    public void setOrganizationUnit(String organizationUnit) throws GridCAException {
        setValue(ORG_UNIT, organizationUnit);
    }
    
    /**
     * Get the email address of the ca adminstrator
     * @return the email address of the ca adminstrator
     */
    public String getAdminEmailAddress() {
        return getValue(ADMIN_EMAIL);
    }
    
    /**
     * Set the email address of the ca adminstrator
     * @param adminEmailAddress email address of the ca adminstrator (at least one char)
     * @throws com.sun.grid.ca.GridCAException if <code>adminEmailAddress</code> is not valid
     */
    public void setAdminEmailAddress(String adminEmailAddress) throws GridCAException {
        setValue(ADMIN_EMAIL, adminEmailAddress);
    }
    
    /**
     * Validate the <code>InitCAParameters</code>
     * @throws com.sun.grid.ca.GridCAException  if the parameters are not valid
     */
    public void validate() throws GridCAException {
        for(int i = 0; i < PARAMS.length; i++) {
            PARAMS[i].check(getValue(PARAMS[i]));
        }
    }
    
    
}
