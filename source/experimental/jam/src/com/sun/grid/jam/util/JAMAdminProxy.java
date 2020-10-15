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
 *   and/or Swiss Center for Scientific Computing
 * 
 *   Copyright: 2002 by Sun Microsystems, Inc.
 *   Copyright: 2002 by Swiss Center for Scientific Computing
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.jam.util;

import net.jini.admin.*;
import com.sun.jini.admin.*;

import net.jini.core.entry.Entry;
import net.jini.core.discovery.LookupLocator;

import java.io.IOException;
import java.io.Serializable;
import java.rmi.*;

/**
 * Object returned by <code>getAdmin</code> calls on JAM service proxies.
 * This object implements <code>JAMAdmin</code>, which itself implements
 * <code>JoinAdmin</code> and <code>DestroyAdmin</code>.
 * 
 * @author Eric Sharakan
 * @version 1.3, 09/22/00
 */
public class JAMAdminProxy
  implements JAMAdmin
{
  private final JAMAdmin server;

  public JAMAdminProxy(JAMAdmin server)
  {
    this.server = server;
  }

  // JoinAdmin, DestroyAdmin implementation methods - Copied
  // from FiddlerAdminProxy.java

  /** 
     * Get the current attribute sets for the service. 
     * 
     * @return array of net.jini.core.entry.Entry containing the current
     *         attribute sets for the service
     * 
     * @throws java.rmi.RemoteException typically, this exception occurs when
     *         there is a communication failure between the client and the
     *         service.
     *
     */
  public Entry[] getLookupAttributes() throws RemoteException {
    return server.getLookupAttributes();
  }

  /** 
     * Add attribute sets to the current set of attributes associated
     * with the service. The resulting set will be used
     * for all future registrations with lookup services. The new attribute
     * sets are also added to the service's attributes
     * on each lookup service with which the service
     * is currently registered.
     *
     * @param  attrSets array of net.jini.core.entry.Entry containing the
     *         attribute sets to add
     * 
     * @throws java.rmi.RemoteException typically, this exception occurs when
     *         there is a communication failure between the client and the
     *         service. When this exception does occur, the
     *         attributes may or may not have been added successfully.
     *
     */
  public void addLookupAttributes(Entry[] attrSets) throws RemoteException {
    server.addLookupAttributes(attrSets);
  }

  /** 
     * Modify the current set of attributes associated with the lookup
     * discovery service. The resulting set will be used for all future
     * registrations with lookup services. The same modifications are 
     * also made to the service's attributes on each
     * lookup service with which the service is currently
     * registered.
     *
     * @param  attrSetTemplates  array of net.jini.core.entry.Entry containing
     *         the templates for matching attribute sets
     * @param  attrSets array of net.jini.core.entry.Entry containing the
     *         modifications to make to matching sets
     * 
     * @throws java.rmi.RemoteException typically, this exception occurs when
     *         there is a communication failure between the client and the
     *         service. When this exception does occur, the
     *         attributes may or may not have been modified successfully.
     *
     */
  public void modifyLookupAttributes(Entry[] attrSetTemplates,
				     Entry[] attrSets)
       throws RemoteException
  {
    server.modifyLookupAttributes(attrSetTemplates, attrSets);
  }

  /** 
     * Get the names of the groups whose members are lookup services the
     * services wishes to register with (join).
     * 
     * @return String array containing the names of the groups whose members
     *         are lookup services the service wishes to
     *         join.
     * <p>
     *         If the array returned is empty, the service
     *         is configured to join no groups. If null is returned, the
     *         service is configured to join all groups.
     * 
     * @throws java.rmi.RemoteException typically, this exception occurs when
     *         there is a communication failure between the client and the
     *         service.
     *
     */
  public String[] getLookupGroups() throws RemoteException {
    return server.getLookupGroups();
  }

  /** 
     * Add new names to the set consisting of the names of groups whose
     * members are lookup services the service wishes
     * to register with (join). Any lookup services belonging to the
     * new groups that the service has not yet registered
     * with, will be discovered and joined.
     *
     * @param  String array containing the names of the groups to add
     * 
     * @throws java.rmi.RemoteException typically, this exception occurs when
     *         there is a communication failure between the client and the
     *         service. When this exception does occur, the
     *         group names may or may not have been added successfully.
     *
     */
  public void addLookupGroups(String[] groups) throws RemoteException {
    server.addLookupGroups(groups);
  }

  /** 
     * Remove a set of group names from service's managed
     * set of groups (the set consisting of the names of groups whose
     * members are lookup services the service wishes
     * to join). Any leases granted to the service by
     * lookup services that are not members of the groups whose names 
     * remain in the managed set will be cancelled at those lookup services.
     *
     * @param  String array containing the names of the groups to remove
     * 
     * @throws java.rmi.RemoteException typically, this exception occurs when
     *         there is a communication failure between the client and the
     *         service. When this exception does occur, the
     *         group names may or may not have been removed successfully.
     *
     */
  public void removeLookupGroups(String[] groups) throws RemoteException {
    server.removeLookupGroups(groups);
  }

  /** 
     * Replace the service's managed set of groups with a
     * new set of group names. Any leases granted to the lookup discovery
     * service by lookup services that are not members of the groups whose
     * names are in the new managed set will be cancelled at those lookup
     * services. Lookup services that are members of groups reflected in
     * the new managed set will be discovered and joined.
     *
     * @param  String array containing the names of the new groups
     * 
     * @throws java.rmi.RemoteException typically, this exception occurs when
     *         there is a communication failure between the client and the
     *         service. When this exception does occur, the
     *         group names may or may not have been replaced successfully.
     *
     */
  public void setLookupGroups(String[] groups) throws RemoteException {
    server.setLookupGroups(groups);
  }

  /** 
   * Get the service's managed set of locators. The
   * managed set of locators is the set of LookupLocator objects
   * corresponding to the specific lookup services with which the lookup
   * discovery service wishes to register (join).
   * 
   * @return array of objects of type net.jini.core.discovery.LookupLocator,
   *         each of which corresponds to a specific lookup service the
   *         service wishes to join.
   * 
   * @throws java.rmi.RemoteException typically, this exception occurs when
   *         there is a communication failure between the client and the
   *         service.
   *
   */
  public LookupLocator[] getLookupLocators() throws RemoteException {
    return server.getLookupLocators();
  }

  /** 
   * Add a set of LookupLocator objects to the service's
   * managed set of locators. The managed set of locators is the set of
   * LookupLocator objects corresponding to the specific lookup services
   * with which the service wishes to register (join).
   * <p>
   * Any lookup services corresponding to the new locators that the lookup
   * discovery service has not yet joined, will be discovered and joined.
   *
   * @param  array of net.jini.core.discovery.LookupLocator objects to add
   *         to the managed set of locators
   * 
   * @throws java.rmi.RemoteException typically, this exception occurs when
   *         there is a communication failure between the client and the
   *         service. When this exception does occur, the
   *         new locators may or may not have been added successfully.
   *
   */
  public void addLookupLocators(LookupLocator[] locators)
       throws RemoteException
  {
    server.addLookupLocators(locators);
  }

  /** 
     * Remove a set of LookupLocator objects from the lookup discovery
     * service's managed set of locators. The managed set of locators is the
     * set of LookupLocator objects corresponding to the specific lookup
     * services with which the service wishes to register
     * (join).
     * <p>
     * Note that any leases granted to the service by
     * lookup services that do not correspond to any of the locators
     * remaining in the managed set will be cancelled at those lookup
     * services.
     *
     * @param  array of net.jini.core.discovery.LookupLocator objects to
     *         remove from the managed set of locators
     * 
     * @throws java.rmi.RemoteException typically, this exception occurs when
     *         there is a communication failure between the client and the
     *         service. When this exception does occur, the
     *         new locators may or may not have been removed successfully.
     *
     */
  public void removeLookupLocators(LookupLocator[] locators)
       throws RemoteException
  {
    server.removeLookupLocators(locators);
  }

  /** 
   * Replace the service's managed set of locators with
   * a new set of locators. The managed set of locators is the set of
   * LookupLocator objects corresponding to the specific lookup services
   * with which the service wishes to register (join).
   * <p>
   * Note that any leases granted to the service by
   * lookup services whose corresponding locator is removed from the
   * managed setwill be cancelled at those lookup services. The lookup
   * services corresponding to the new locators in the managed set
   * will be discovered and joined.
   *
   * @param  array of net.jini.core.discovery.LookupLocator objects with
   *         which to replace the current managed set of locators
   *         remove from the managed set of locators
   * 
   * @throws java.rmi.RemoteException typically, this exception occurs when
   *         there is a communication failure between the client and the
   *         service. When this exception does occur, the
   *         locators in the managed set may or may not have been replaced
   *         successfully.
   *
   */
  public void setLookupLocators(LookupLocator[] locators)
       throws RemoteException
  {
    server.setLookupLocators(locators);
  }

  /**
   * Destroy the service, if possible, including its
   * persistent storage. This method will typically spawn a separate
   * thread to do the actual work asynchronously, so a successful
   * return from this method usually does not mean that the service
   * has been destroyed.
   * 
   * @throws java.rmi.RemoteException typically, this exception occurs when
   *         there is a communication failure between the client and the
   *         service. When this exception does occur, the
   *         service may or may not have been successfully
   *         destroyed.
   *
   */
  public void destroy() throws RemoteException {
    server.destroy();
  }
}
