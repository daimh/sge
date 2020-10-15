/*
 * IzPack - Copyright 2001-2008 Julien Ponge, All Rights Reserved.
 *
 * http://izpack.org/
 * http://izpack.codehaus.org/
 *
 * Copyright 2007 Dennis Reil
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
package com.izforge.izpack.rules;

import com.izforge.izpack.Pack;
import com.izforge.izpack.installer.InstallData;
import com.izforge.izpack.util.Debug;
import net.n3.nanoxml.XMLElement;

import java.util.*;

/**
 * The rules engine class is the central point for checking conditions
 *
 * @author Dennis Reil, <Dennis.Reil@reddot.de> created: 09.11.2006, 13:48:39
 */
public class RulesEngine
{

    protected Map<String, String> panelconditions;

    protected Map<String, String> packconditions;

    protected Map<String, String> optionalpackconditions;

    protected XMLElement conditionsspec;

    protected static Map conditionsmap = new Hashtable();

    protected static InstallData installdata;

    private RulesEngine()
    {
        conditionsmap = new Hashtable();
        this.panelconditions = new Hashtable<String, String>();
        this.packconditions = new Hashtable<String, String>();
        this.optionalpackconditions = new Hashtable<String, String>();
    }

    /**
     * initializes builtin conditions
     */
    private void init()
    {
        createBuiltinOsCondition("IS_WINDOWS", "izpack.windowsinstall");
        createBuiltinOsCondition("IS_LINUX", "izpack.linuxinstall");
        createBuiltinOsCondition("IS_SUNOS", "izpack.solarisinstall");
        createBuiltinOsCondition("IS_MAC", "izpack.macinstall");
        createBuiltinOsCondition("IS_SUNOS", "izpack.solarisinstall");
        createBuiltinOsCondition("IS_SUNOS_X86", "izpack.solarisinstall.x86");
        createBuiltinOsCondition("IS_SUNOS_SPARC", "izpack.solarisinstall.sparc");

        if ((installdata != null) && (installdata.allPacks != null))
        {
            for (Pack pack : installdata.allPacks)
            {
                if (pack.id != null)
                {
                    // automatically add packselection condition
                    PackselectionCondition packselcond = new PackselectionCondition();
                    packselcond.setInstalldata(installdata);
                    packselcond.id = "izpack.selected." + pack.id;
                    packselcond.packid = pack.id;
                    conditionsmap.put(packselcond.id, packselcond);
                }
            }
        }
    }

    private void createBuiltinOsCondition(String osVersionField, String conditionId)
    {
        JavaCondition condition = new JavaCondition();
        condition.setInstalldata(installdata);
        condition.id = conditionId;
        condition.classname = "com.izforge.izpack.util.OsVersion";
        condition.fieldname = osVersionField;
        condition.returnvalue = "true";
        condition.returnvaluetype = "boolean";
        condition.complete = true;
        conditionsmap.put(condition.id, condition);
    }

    /**
     *
     */
    public RulesEngine(XMLElement conditionsspecxml, InstallData installdata)
    {
        this();
        this.conditionsspec = conditionsspecxml;
        RulesEngine.installdata = installdata;
        this.readConditions();
        init();
    }

    public RulesEngine(Map rules, InstallData installdata)
    {
        this();
        RulesEngine.installdata = installdata;
        conditionsmap = rules;
        Iterator keyiter = conditionsmap.keySet().iterator();
        while (keyiter.hasNext())
        {
            String key = (String) keyiter.next();
            Condition condition = (Condition) conditionsmap.get(key);
            condition.setInstalldata(installdata);
        }
        init();
    }

    /**
     * Returns the current known condition ids.
     *
     * @return
     */
    public String[] getKnownConditionIds()
    {
        String[] conditionids = (String[]) this.conditionsmap.keySet().toArray(new String[this.conditionsmap.size()]);
        Arrays.sort(conditionids);
        return conditionids;
    }

    /**
     * Checks if an attribute for an xmlelement is set.
     *
     * @param val       value of attribute to check
     * @param attribute the attribute which is checked
     * @param element   the element
     * @return true value was set false no value was set
     */
    protected boolean checkAttribute(String val, String attribute, String element)
    {
        if ((val != null) && (val.length() > 0))
        {
            return true;
        }
        else
        {
            Debug.trace("Element " + element + " has to specify an attribute " + attribute);
            return false;
        }
    }

    public static Condition analyzeCondition(XMLElement condition)
    {
        String condid = condition.getAttribute("id");
        String condtype = condition.getAttribute("type");
        Condition result = null;
        if (condtype != null)
        {
            String conditionclassname = "";
            if (condtype.indexOf('.') > -1)
            {
                conditionclassname = condtype;
            }
            else
            {
                String conditiontype = condtype.toLowerCase();
                conditionclassname = "com.izforge.izpack.rules."
                        + conditiontype.substring(0, 1).toUpperCase()
                        + conditiontype.substring(1, conditiontype.length());
                conditionclassname += "Condition";
            }
            //ClassLoader loader = ClassLoader.getSystemClassLoader();
            ClassLoader loader = RulesEngine.class.getClassLoader();
            try
            {
                Class<Condition> conditionclass = (Class<Condition>) loader.loadClass(conditionclassname);
                result = conditionclass.newInstance();
                result.readFromXML(condition);
                result.setId(condid);
                result.setInstalldata(RulesEngine.installdata);
            }
            catch (ClassNotFoundException e)
            {
                Debug.trace(conditionclassname + " not found.");
            }
            catch (InstantiationException e)
            {
                Debug.trace(conditionclassname + " couldn't be instantiated.");
            }
            catch (IllegalAccessException e)
            {
                Debug.trace("Illegal access to " + conditionclassname);
            }
        }
        return result;
    }

    /**
     * Read the spec for the conditions
     */
    protected void readConditions()
    {
        if (this.conditionsspec == null)
        {
            Debug.trace("No specification for conditions found.");
            return;
        }
        try
        {
            if (this.conditionsspec.hasChildren())
            {
                // read in the condition specs
                Vector<XMLElement> childs = this.conditionsspec.getChildrenNamed("condition");

                for (XMLElement condition : childs)
                {
                    Condition cond = analyzeCondition(condition);
                    if (cond != null)
                    {
                        // this.conditionslist.add(cond);
                        String condid = cond.getId();
                        cond.setInstalldata(RulesEngine.installdata);
                        if ((condid != null) && !("UNKNOWN".equals(condid)))
                        {
                            conditionsmap.put(condid, cond);
                        }
                    }
                }

                Vector<XMLElement> panelconditionels = this.conditionsspec.getChildrenNamed("panelcondition");
                for (XMLElement panelel : panelconditionels)
                {
                    String panelid = panelel.getAttribute("panelid");
                    String conditionid = panelel.getAttribute("conditionid");
                    this.panelconditions.put(panelid, conditionid);
                }

                Vector<XMLElement> packconditionels = this.conditionsspec.getChildrenNamed("packcondition");
                for (XMLElement panelel : packconditionels)
                {
                    String panelid = panelel.getAttribute("packid");
                    String conditionid = panelel.getAttribute("conditionid");
                    this.packconditions.put(panelid, conditionid);
                    // optional install allowed, if condition is not met?
                    String optional = panelel.getAttribute("optional");
                    if (optional != null)
                    {
                        boolean optionalinstall = Boolean.valueOf(optional);
                        if (optionalinstall)
                        {
                            // optional installation is allowed
                            this.optionalpackconditions.put(panelid, conditionid);
                        }
                    }
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public static Condition getCondition(String id)
    {
        Condition result = (Condition) conditionsmap.get(id);
        if (result == null)
        {
            result = getConditionByExpr(new StringBuffer(id));
        }
        return result;
    }

    protected static Condition getConditionByExpr(StringBuffer conditionexpr)
    {
        //System.out.println("ExprStart: " + conditionexpr);
        int openeingIndex = -1;
        while ((openeingIndex = conditionexpr.indexOf("(")) > -1) {
            int level = 0;
            int closingIndex = -1;
            for (int i = openeingIndex + 1; i < conditionexpr.length(); i++) {
                if (conditionexpr.charAt(i) == '(') {
                    level++;
                }

                if (conditionexpr.charAt(i) == ')') {
                    if (level == 0) {
                        closingIndex = i;
                        break;
                    } else {
                        level--;
                    }
                }
            }
            if (closingIndex == -1) {
                Debug.trace("error: missing closing bracket for pair at: " + openeingIndex);
                conditionexpr.delete(openeingIndex, openeingIndex + 1);
                break;
            }

            //System.out.println("ExprSub: " + conditionexpr.substring(openeingIndex + 1, closingIndex));
            Condition cond = getCondition(conditionexpr.substring(openeingIndex + 1, closingIndex));
            
            if (openeingIndex - 1 > -1 && conditionexpr.substring(openeingIndex - 1, openeingIndex).equals("!")) {
                conditionexpr.replace(openeingIndex - 1, closingIndex + 1, (!cond.isTrue() ? "T" : "F"));
            } else {
                conditionexpr.replace(openeingIndex, closingIndex + 1, (cond.isTrue() ? "T" : "F"));
            }

            //System.out.println("ExprEnd: " + conditionexpr);
        }

        Condition result = null;
        int index = 0;
        while (index < conditionexpr.length())
        {
            char currentchar = conditionexpr.charAt(index);
            switch (currentchar)
            {
                case '+':
                    // and-condition
                    Condition op1 = (Condition) conditionsmap.get(conditionexpr.substring(0, index));
                    conditionexpr.delete(0, index + 1);
                    result = new AndCondition(op1, getConditionByExpr(conditionexpr));
                    result.setInstalldata(RulesEngine.installdata);
                    break;
                case '|':
                    // or-condition
                    op1 = (Condition) conditionsmap.get(conditionexpr.substring(0, index));
                    conditionexpr.delete(0, index + 1);
                    result = new OrCondition(op1, getConditionByExpr(conditionexpr));
                    result.setInstalldata(RulesEngine.installdata);
                    break;
                case '\\':
                    // xor-condition
                    op1 = (Condition) conditionsmap.get(conditionexpr.substring(0, index));
                    conditionexpr.delete(0, index + 1);
                    result = new XOrCondition(op1, getConditionByExpr(conditionexpr));
                    result.setInstalldata(RulesEngine.installdata);
                    break;
                case '!':
                    // not-condition
                    if (index > 0)
                    {
                        Debug.trace("error: ! operator only allowed at position 0");
                    }
                    else
                    {
                        // delete not symbol
                        conditionexpr.deleteCharAt(index);
                        result = new NotCondition(getConditionByExpr(conditionexpr));
                        result.setInstalldata(RulesEngine.installdata);
                    }
                    break;
                default:
                    // do nothing
            }
            index++;
        }
        if (conditionexpr.length() > 0)
        {
            result = (Condition) conditionsmap.get(conditionexpr.toString());
            if (result != null)
            {
                result.setInstalldata(RulesEngine.installdata);
                conditionexpr.delete(0, conditionexpr.length());
            }
        }
        return result;
    }

    public boolean isConditionTrue(String id, Properties variables)
    {
        Condition cond = getCondition(id);
        if (cond == null)
        {
            Debug.trace("Condition (" + id + ") not found.");
            return true;
        }
        else
        {
            Debug.trace("Checking condition");
            try
            {
                return cond.isTrue();
            }
            catch (NullPointerException npe)
            {
                Debug.error("Nullpointerexception checking condition: " + id);
                return false;
            }
        }
    }

    public boolean isConditionTrue(Condition cond, Properties variables)
    {
        if (cond == null)
        {
            Debug.trace("Condition not found.");
            return true;
        }
        else
        {
            Debug.trace("Checking condition");
            return cond.isTrue();
        }
    }

    public boolean isConditionTrue(String id)
    {
        Condition cond = RulesEngine.getCondition(id);
        if (cond != null)
        {
            return this.isConditionTrue(cond);
        }
        else
        {
            return false;
        }
    }

    public boolean isConditionTrue(Condition cond)
    {
        return cond.isTrue();
    }

    /**
     * Can a panel be shown?
     *
     * @param panelid   - id of the panel, which should be shown
     * @param variables - the variables
     * @return true - there is no condition or condition is met false - there is a condition and the
     *         condition was not met
     */
    public boolean canShowPanel(String panelid, Properties variables)
    {
        Debug.trace("can show panel with id " + panelid + " ?");
        if (!this.panelconditions.containsKey(panelid))
        {
            Debug.trace("no condition, show panel");
            return true;
        }
        Debug.trace("there is a condition");
        Condition condition = getCondition(this.panelconditions.get(panelid));
        if (condition != null)
        {
            return condition.isTrue();
        }
        return false;
    }

    /**
     * Is the installation of a pack possible?
     *
     * @param packid
     * @param variables
     * @return true - there is no condition or condition is met false - there is a condition and the
     *         condition was not met
     */
    public boolean canInstallPack(String packid, Properties variables)
    {
        if (packid == null)
        {
            return true;
        }
        Debug.trace("can install pack with id " + packid + "?");
        if (!this.packconditions.containsKey(packid))
        {
            Debug.trace("no condition, can install pack");
            return true;
        }
        Debug.trace("there is a condition");
        Condition condition = getCondition(this.packconditions.get(packid));
        if (condition != null)
        {
            return condition.isTrue();
        }
        return false;
    }

    /**
     * Is an optional installation of a pack possible if the condition is not met?
     *
     * @param packid
     * @param variables
     * @return
     */
    public boolean canInstallPackOptional(String packid, Properties variables)
    {
        Debug.trace("can install pack optional with id " + packid + "?");
        if (!this.optionalpackconditions.containsKey(packid))
        {
            Debug.trace("not in optionalpackconditions.");
            return false;
        }
        else
        {
            Debug.trace("optional install possible");
            return true;
        }
    }
}
