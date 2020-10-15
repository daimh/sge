#___INFO__MARK_BEGIN__
#########################################################################
# 
#  The Contents of this file are made available subject to the terms of
#  the Sun Industry Standards Source License Version 1.2
# 
#  Sun Microsystems Inc., March, 2001
# 
# 
#  Sun Industry Standards Source License Version 1.2
#  =================================================
#  The contents of this file are subject to the Sun Industry Standards
#  Source License Version 1.2 (the "License"); You may not use this file
#  except in compliance with the License. You may obtain a copy of the
#  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
# 
#  Software provided under this License is provided on an "AS IS" basis,
#  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#  See the License for the specific provisions governing your rights and
#  obligations concerning the Software.
# 
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#   and/or Swiss Center for Scientific Computing
# 
#   Copyright: 2002 by Sun Microsystems, Inc.
#   Copyright: 2002 by Swiss Center for Scientific Computing
# 
#   All Rights Reserved.
# 
########################################################################
#___INFO__MARK_END__
include $(TOPDIR)/GNUmakefileConstants.gmk
include $(TOPDIR)/GNUmakefileRules.gmk

.PHONY: all $(SUBDIRS) makedirs lib clm-stub app-jars rms-jars fe-jars \
	doc clean jar clean-java TAGS test

all: $(SUBDIRS) jar

makedirs:
	mkdir -p $(CLASSDIRS) jar $(JINIWEB) $(HTMLDOCDIR)

$(SUBDIRS): makedirs
	$(MAKE) -C $@ all
	$(MAKE) -C $@ stub

APP_TOP_LEVEL_CLASSES = $(addprefix com.sun.grid.jam., \
	app.StartApp)

APP_PROXY_ENTRY_CLASSES = $(addprefix com.sun.grid.jam., \
	app.NativeApp app.UserDefinedApp app.SortApp \
	app.NativeAppUI app.UserAppUI app.SortAppUI \
	app.NativeAppAgent app.UserAppAgent app.SortAgent)

APP_STUB_CLASSES = $(addprefix com.sun.grid.jam., \
	util.JAMServiceRegistrar_Stub)

RMSERVER_TOP_LEVEL_CLASSES = $(addprefix com.sun.grid.jam., \
	rms.RMServer job.entry.JobUserKey)

RMSERVER_PROXY_ENTRY_CLASSES = $(addprefix com.sun.grid.jam., \
	ui.entry.ServiceUIFactoryEntry \
	queue.QueueProxy queue.entry.ComputeInfoEntry \
	queue.entry.QueueInfo queue.entry.QueueStatus \
	job.JobProxy job.entry.JobMonitorJFrameFactory \
	job.entry.JobUserKey)

RMSERVER_STUB_CLASSES = $(addprefix com.sun.grid.jam., \
	job.JobObserver_Stub job.ui.JobMonitorJPanel'$$'ChangeListener_Stub \
	queue.ExecutionEngineImpl_Stub \
	rms.RMServer'$$'ServiceCreatorListener_Stub \
	rms.RMServer'$$'JobDestroyListener_Stub \
	gridengine.RMAdapter'$$'SubmissionListener_Stub \
	gridengine.RMAdapter'$$'ResumeRequestListener_Stub \
	gridengine.RMAdapter'$$'SuspendRequestListener_Stub \
	gridengine.RMAdapter'$$'RestartRequestListener_Stub \
	gridengine.RMAdapter'$$'StartRequestListener_Stub \
	gridengine.RMAdapter'$$'StopRequestListener_Stub \
	gridengine.RMAdapter'$$'KilledJobListener_Stub \
	util.RemoteInputStream_Stub \
	util.JAMServiceRegistrar_Stub)

FE_TOP_LEVEL_CLASSES = $(addprefix com.sun.grid.jam., \
	tools.JAMServiceHandler tools.JAMJobSubmitter)

# FE is not a Jini service, so has no proxy or Entry classes

FE_STUB_CLASSES = $(addprefix com.sun.grid.jam., tools.JAMJobHandler_Stub)

jar: app-jars rms-jars fe-jars

# Extract ServiceDiscoveryManager's remote event listener from Jini
# distribution.  Any ServiceDiscoveryManager user needs this class available
# via the designated codebase.
clm-stub:
	(cd $(CODEBASE); $(JAR) xf $(JINILIB)/jini-ext.jar net/jini/lookup/ServiceDiscoveryManager\$$LookupCacheImpl\$$LookupListener_Stub.class)

app-jars: clm-stub
	(cd $(CODEBASE); $(JAR) cvf $(APP_JAR) `$(CLASSDEP) \
	$(APP_TOP_LEVEL_CLASSES) $(APP_STUB_CLASSES)`)

	(cd $(CODEBASE); $(JAR) cvf $(APP_DL_JAR) `$(CLASSDEP) \
	$(APP_PROXY_ENTRY_CLASSES) $(APP_STUB_CLASSES)` \
	net/jini/lookup/ServiceDiscoveryManager\$$LookupCacheImpl\$$LookupListener_Stub.class)
	cp $(APP_JAR) $(APP_DL_JAR) $(JINIWEB)

rms-jars: clm-stub
	(cd $(CODEBASE); $(JAR) cvf $(RMS_JAR) `$(CLASSDEP) \
	$(RMSERVER_TOP_LEVEL_CLASSES) $(RMSERVER_STUB_CLASSES)`)

	(cd $(CODEBASE); $(JAR) cvf $(RMS_DL_JAR) `$(CLASSDEP) \
	$(RMSERVER_PROXY_ENTRY_CLASSES) $(RMSERVER_STUB_CLASSES)` \
	net/jini/lookup/ServiceDiscoveryManager\$$LookupCacheImpl\$$LookupListener_Stub.class)
	cp $(RMS_JAR) $(RMS_DL_JAR) $(JINIWEB)

# ClassDep doesn't realize JobInterface is required since it's only in
# the source (JAMApplicationHandler) as a Class (String) declaration.
# Is there a better way to deal with cases like this? XXX
fe-jars: clm-stub
	(cd $(CODEBASE); $(JAR) cvf $(FE_JAR) `$(CLASSDEP) \
	$(FE_TOP_LEVEL_CLASSES)` \
	com/sun/grid/jam/job/JobInterface.class)

	(cd $(CODEBASE); $(JAR) cvf $(FE_DL_JAR) `$(CLASSDEP) \
	$(FE_STUB_CLASSES)` \
	net/jini/lookup/ServiceDiscoveryManager\$$LookupCacheImpl\$$LookupListener_Stub.class)
	cp $(FE_JAR) $(FE_DL_JAR) $(JINIWEB)

doc: makedirs
	$(JAVADOC) $(JAVADOCFLAGS) $(PACKAGES)

install:

clean: clean-java
	rm -f $(INCDIR)/com_sun_grid_jam_gridengine_NativeSGERMAdapter.h $(CODEBASE)/libsge_rmadapter.so $(TOPDIR)/native/gridengine/*.o

clean-java:
	(cd $(CODEBASE); find . -name *.class -exec rm {} \;)
	rm -f $(APP_JAR) $(APP_DL_JAR) $(RMS_JAR) $(RMS_DL_JAR) $(FE_JAR) $(FE_DL_JAR)
	rm -f $(JINIWEB)/*.jar
	rm -rf $(TOPDIR)/http/www
	rm -rf $(HTMLDOCDIR)
	rm -rf $(TOPDIR)/jar
	rm -rf $(TOPDIR)/lib

ALLJAVAFILES = $(addsuffix /*.java, $(addprefix src/, $(JAMSUBDIRS)))

TAGS:
	etags $(ALLJAVAFILES) native/gridengine/*.cpp

