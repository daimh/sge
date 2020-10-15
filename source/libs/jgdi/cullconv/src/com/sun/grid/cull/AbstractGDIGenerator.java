/*
 * AbstractGDIGenerator.java
 *
 * Created on April 4, 2006, 9:23 AM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package com.sun.grid.cull;

import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Abstract helper class for generating code which performs GDI actions
 */
public abstract class AbstractGDIGenerator {
   
   protected Logger logger = Logger.getLogger("cullconv");
   
   /* classname of the cull object without packagename */
   protected String classname;
   
   protected String name;
   
   /* key is the name of the primary key field, value is the java type */
   protected java.util.Map<String,String> primaryKeys = new java.util.HashMap<String,String>();
   
   protected com.sun.grid.cull.CullObject cullObject;
   
   public AbstractGDIGenerator(String name, String classname,
           com.sun.grid.cull.CullObject cullObject) {
      this.name = name;
      this.classname = classname;
      this.cullObject = cullObject;
   }
   
   public void addPrimaryKey(String name, String type) {
      primaryKeys.put(name, type);
   }
   
   public void addPrimaryKeys(CullObject cullObj, JavaHelper jh) {
      
      for(int i = 0; i < cullObj.getPrimaryKeyCount(); i++ ) {
         CullAttr attr = cullObj.getPrimaryKeyAttr(i);
         addPrimaryKey(attr.getName(), jh.getClassName(attr.getType()));
      }
   }
   
   public int getPrimaryKeyCount() {
      return primaryKeys.size();
   }
   public CullObject getCullObject() {
      return cullObject;
   }
   
   
   public void genMethods() {
      
      if (cullObject.hasModifyOperation()) {
         if(logger.isLoggable(Level.FINE)) {
            logger.fine("generate update method for " + cullObject.getName());
         }
         genUpdateMethod();
      }
      if (cullObject.hasDeleteOperation()) {
         if(logger.isLoggable(Level.FINE)) {
            logger.fine("generate delete method for " + cullObject.getName());
         }
         genDeleteMethod();
         genDeleteByPrimaryKeyMethod();
      }
      if (cullObject.hasAddOperation()) {
         if(logger.isLoggable(Level.FINE)) {
            logger.fine("generate add method for " + cullObject.getName());
         }
         genAddMethod();
      }
      if (cullObject.hasGetListOperation()) {
         if(logger.isLoggable(Level.FINE)) {
            logger.fine("generate get list method for " + cullObject.getName());
         }
         genGetListMethod();
      }
      if (cullObject.hasGetOperation() ) {
         
         if(cullObject.getPrimaryKeyCount() > 0) {
            if(logger.isLoggable(Level.FINE)) {
               logger.fine("generate get by primary key method for " + cullObject.getName());
            }
            genGetByPrimaryKeyMethod();
         } else {
            if(logger.isLoggable(Level.FINE)) {
               logger.fine("generate gen get method for " + cullObject.getName());
            }
            genGetMethod();
         }
      }
   }
   
   protected abstract void genUpdateMethod();
   protected abstract void genGetMethod();
   protected abstract void genGetListMethod();
   protected abstract void genGetByPrimaryKeyMethod();
   protected abstract void genAddMethod();
   protected abstract void genDeleteMethod();
   protected abstract void genDeleteByPrimaryKeyMethod();
   public abstract void genImport();
   
   
}
