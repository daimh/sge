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
 *   Copyright: 2009 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.jsv;

import java.util.LinkedList;
import java.util.List;
import junit.framework.TestCase;

/*
 * TODO: add negative tests
 */
public class BindingSpecifierTest extends TestCase {
    public BindingSpecifierTest() {
    }

    /**
     * Test of setStrategy, getStrategy, isXxxStrategy() method, of class BindingSpecifier.
     */
    public void testStrategy() {
        System.out.println("getStrategy(), setStrategy(), isLinearStrategy(), isStridingStrategy(), isExplicit()");

        BindingSpecifier linearInstance = new BindingSpecifier();
        BindingSpecifier stridingInstance = new BindingSpecifier();
        BindingSpecifier explicitInstance = new BindingSpecifier();

        BindingSpecifier.Strategy expLinearResult = BindingSpecifier.Strategy.LINEAR;
        BindingSpecifier.Strategy expStridingResult = BindingSpecifier.Strategy.STRIDING;
        BindingSpecifier.Strategy expExplicitResult = BindingSpecifier.Strategy.EXPLICIT;

        linearInstance.setStrategy(expLinearResult);
        stridingInstance.setStrategy(expStridingResult);
        explicitInstance.setStrategy(expExplicitResult);

        BindingSpecifier.Strategy linearResult = linearInstance.getStrategy();
        BindingSpecifier.Strategy stridingResult = stridingInstance.getStrategy();
        BindingSpecifier.Strategy explicitResult = explicitInstance.getStrategy();

        assertEquals("The getStriding() method did not return the value set with setStrategy(BindingSpecifier.LINEAR)", expLinearResult, linearResult);
        assertEquals("The getStriding() method did not return the value set with setStrategy(BindingSpecifier.STRIDING)", expStridingResult, stridingResult);
        assertEquals("The getStriding() method did not return the value set with setStrategy(BindingSpecifier.EXPLICIT)", expExplicitResult, explicitResult);

        Boolean isLinear = linearInstance.isLinear() && !linearInstance.isStriding() && !linearInstance.isExplicit();
        Boolean isStriding = !stridingInstance.isLinear() && stridingInstance.isStriding() && !stridingInstance.isExplicit();
        Boolean isExplicit = !explicitInstance.isLinear() && !explicitInstance.isStriding() && explicitInstance.isExplicit();

        assertEquals("The isXxxType() methods do not return the correct value", Boolean.TRUE, isLinear);
        assertEquals("The isXxxType() methods do not return the correct value", Boolean.TRUE, isStriding);
        assertEquals("The isXxxType() methods do not return the correct value", Boolean.TRUE, isExplicit);
    }

    /**
     * Test of setType, getType, isXxxType() method, of class BindingSpecifier.
     */
    public void testType() {
        System.out.println("getType(), setType(), isPeType(), isEnvType(), isSetType()");

        BindingSpecifier peInstance = new BindingSpecifier();
        BindingSpecifier envInstance = new BindingSpecifier();
        BindingSpecifier setInstance = new BindingSpecifier();

        BindingSpecifier.Type expPeResult = BindingSpecifier.Type.PE;
        BindingSpecifier.Type expEnvResult = BindingSpecifier.Type.ENV;
        BindingSpecifier.Type expSetResult = BindingSpecifier.Type.SET;

        peInstance.setType(expPeResult);
        envInstance.setType(expEnvResult);
        setInstance.setType(expSetResult);

        BindingSpecifier.Type peResult = peInstance.getType();
        BindingSpecifier.Type envResult = envInstance.getType();
        BindingSpecifier.Type setResult = setInstance.getType();

        assertEquals("The getStrategy() method did not return the value set with setStartegy(BindingSpecifier.PE)", expPeResult, peResult);
        assertEquals("The getStrategy() method did not return the value set with setStartegy(BindingSpecifier.ENV)", expEnvResult, envResult);
        assertEquals("The getStrategy() method did not return the value set with setStartegy(BindingSpecifier.SET)", expSetResult, setResult);

        Boolean isPe = peInstance.isPeType() && !peInstance.isEnvType() && !peInstance.isSetType();
        Boolean isEnv = !envInstance.isPeType() && envInstance.isEnvType() && !envInstance.isSetType();
        Boolean isSet = !setInstance.isPeType() && !setInstance.isEnvType() && setInstance.isSetType();
        
        assertEquals("The isXxxType() methods do not return the correct value", Boolean.TRUE, isPe);
        assertEquals("The isXxxType() methods do not return the correct value", Boolean.TRUE, isEnv);
        assertEquals("The isXxxType() methods do not return the correct value", Boolean.TRUE, isSet);
    }

    public void testAmount() {
        System.out.println("getAmount(), setAmount()");

        BindingSpecifier instance = new BindingSpecifier();
        Integer expAmount = new Integer(1234);
        instance.setAmount(expAmount);
        Integer amount = instance.getAmount();
        assertEquals("The getAmount() methods do not return the correct value", expAmount, amount);
    }

    public void testSocket() {
        System.out.println("getSocket(), setSocket()");

        BindingSpecifier instance = new BindingSpecifier();
        Integer expSocket = new Integer(1234);
        instance.setSocket(expSocket);
        Integer Socket = instance.getSocket();
        assertEquals("The getSocket() methods do not return the correct value", expSocket, Socket);
    }

    public void testCore() {
        System.out.println("getCore(), setCore()");

        BindingSpecifier instance = new BindingSpecifier();
        Integer expCore = new Integer(1234);
        instance.setCore(expCore);
        Integer Core = instance.getCore();
        assertEquals("The getCore() methods do not return the correct value", expCore, Core);
    }

    public void testStep() {
        System.out.println("getStep(), setStep()");

        BindingSpecifier instance = new BindingSpecifier();
        Integer expStep = new Integer(1234);
        instance.setStep(expStep);
        Integer Step = instance.getStep();
        assertEquals("The getStep() methods do not return the correct value", expStep, Step);
    }

    public void testCoreSpecifiers() {
        System.out.println("getStep(), setStep()");

        BindingSpecifier instance = new BindingSpecifier();
        List<BindingSpecifier.CoreSpecifier> expList = new LinkedList<BindingSpecifier.CoreSpecifier>();
        expList.add(instance.new CoreSpecifier(123, 456));

        instance.setExplicitStrategy(expList);
        List<BindingSpecifier.CoreSpecifier> list = instance.getCoreSpecifiers();
        assertEquals("The getStep() methods do not return the correct value", expList, list);
    }
}
