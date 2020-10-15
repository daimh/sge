# Copyright Dave Love,  2011

# Copyright (C) 2011 Dave Love, University of Liverpool <d.love@liv.ac.uk>
#
# This file is free software: you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation; either version 3 of
# the License, or (at your option) any later version.
#
# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this file.  If not, a copy can be downloaded
# from http://www.gnu.org/licenses/lgpl.html.

UTI2LIB_DIR  = $(SDIR)/libs/uti2

UTI2LIB  = libuti2$(LIBEXT)

UTI2LIB_SRC = sge_execvlp.c sge_cgroup.c util.c
UTI2LIB_OBJS = $(UTI2LIB_SRC:.c=.o)

uti2lib: $(UTI2LIB)

uti2lib_clean: 
	$(RM) $(UTI2LIB_OBJS) $(UTI2LIB) libuti2$(SHAREDEXT)

uti2lib_depend:
	$(DEPEND) -f$(UTI2LIB_DIR)/uti2_dependencies $(UTI2LIB_DIR)/*.c

uti2lib_messages:
	@echo Creating $@
	@for i in `ls $(UTI2LIB_DIR)/*.c`; do \
	   $(CC) $(CFLAGS) -DEXTRACT_MESSAGES -E $$i | $(XGETTEXT) - ;\
	done

libuti2.a: $(UTI2LIB_OBJS)
	$(AR) $(ARFLAGS) $(UTI2LIB) $(UTI2LIB_OBJS)
	$(RANLIB) $(UTI2LIB)

libuti2$(SHAREDEXT):  $(UTI2LIB_OBJS)
	$(SHAREDLD) $(SHARED_LFLAGS) -o libuti2$(SHAREDEXT) $(UTI2LIB_OBJS) $(LOADAVGLIBS) $(LIBS) -lc

include $(UTI2LIB_DIR)/uti2_dependencies

sge_execvlp.o: $(UTI2LIB_DIR)/sge_execvlp.c
	$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTI2LIB_DIR)/sge_execvlp.c

util.o: $(UTI2LIB_DIR)/util.c
	$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTI2LIB_DIR)/util.c

sge_cgroup.o: $(UTI2LIB_DIR)/sge_cgroup.c
	$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $(UTI2LIB_DIR)/sge_cgroup.c

getgrouplist.o: $(UTI2LIB_DIR)/getgrouplist.c
	$(CC) $(CFLAGS) $(SHARED_CFLAGS) -c $<
