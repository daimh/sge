#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define __SGE_GDI_LIBRARY_HOME_OBJECT_FILE__
#include "cull/cull.h"

#include "uti/sge_profiling.h"
#include "uti/sge_stdio.h"

#include "msg_common.h"

enum {
   TEST_host = 1,
   TEST_string,
   TEST_double,
   TEST_ulong,
   TEST_ulong64,
   TEST_bool,
   TEST_list,
   TEST_object,
   TEST_ref
};

LISTDEF(TEST_Type)
   SGE_HOST   (TEST_host,              CULL_DEFAULT | CULL_SPOOL)
   SGE_STRING (TEST_string,            CULL_DEFAULT)
   SGE_DOUBLE (TEST_double,            CULL_DEFAULT)
   SGE_ULONG  (TEST_ulong,             CULL_DEFAULT)
   SGE_ULONG64 (TEST_ulong64,          CULL_DEFAULT)
   SGE_BOOL   (TEST_bool,              CULL_DEFAULT)
   SGE_LIST   (TEST_list, TEST_Type,   CULL_DEFAULT | CULL_SPOOL)
   SGE_OBJECT (TEST_object, TEST_Type, CULL_DEFAULT | CULL_SPOOL)
   SGE_REF    (TEST_ref, TEST_Type,    CULL_DEFAULT)
LISTEND

NAMEDEF(TEST_Name)
   NAME("TEST_host")
   NAME("TEST_string")
   NAME("TEST_double")
   NAME("TEST_ulong")
   NAME("TEST_ulong64")
   NAME("TEST_bool")
   NAME("TEST_list")
   NAME("TEST_object")
   NAME("TEST_ref")
NAMEEND   

#define TEST_Size sizeof(TEST_Name) / sizeof(char *)

lNameSpace nmv[] = {
   {1, TEST_Size, TEST_Name},
   {0, 0, NULL}
};

int main(int argc, char *argv[])
{
   const char *const filename = "test_cull_pack.txt";
   lListElem *ep, *obj, *copy;
   sge_pack_buffer pb, copy_pb;
   int pack_ret;
   FILE *fd;
   char *buffer;
   u_long32 counted_size;

   lInit(nmv);
   prof_mt_init();

   /* create an element */
   ep = lCreateElem(TEST_Type);
   obj = lCreateElem(TEST_Type);

   /* test field access functions */
   lSetHost(ep, TEST_host, "test_host");
   lSetString(ep, TEST_string, "test_string");
   lSetDouble(ep, TEST_double, 3.1);
   lSetUlong(ep, TEST_ulong, 3);
   lSetUlong64(ep, TEST_ulong64, 3);
   lSetBool(ep, TEST_bool, true);

   lSetHost(obj, TEST_host, "test sub host");
   lSetString(obj, TEST_string, "test sub string");
   lSetObject(ep, TEST_object, obj);

   lSetRef(ep, TEST_ref, ep);

   /* fill the sublist */
   {
      lList *lp;
      int i;
      lp = lCreateList("", TEST_Type);
      lSetList(ep, TEST_list, lp);
      for (i=0; i<10000; i++) {
         char name[1024];
         lListElem *ep1= lCreateElem(TEST_Type);
         sprintf(name, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%d", i); 
         lSetHost(ep1, TEST_host, name);
         lSetString(ep1, TEST_string, name);
         lAppendElem(lp, ep1);
      }
   }
#if 0
   printf("element after setting fields\n");
   lWriteElemTo(ep, stdout);
#endif

   /* test just count */
   if((pack_ret = init_packbuffer(&pb, 100, 1)) != PACK_SUCCESS) {
      printf("intializing packbuffer failed: %s\n", cull_pack_strerror(pack_ret));
      return EXIT_FAILURE;
   }

   if((pack_ret = cull_pack_elem(&pb, ep)) != PACK_SUCCESS) {
      printf("packing element failed: %s\n", cull_pack_strerror(pack_ret));
      return EXIT_FAILURE;
   }
   counted_size = pb.bytes_used + (2*INTSIZE);
   clear_packbuffer(&pb);

   /* test packing */
   if((pack_ret = init_packbuffer(&pb, 100, 0)) != PACK_SUCCESS) {
      printf("intializing packbuffer failed: %s\n", cull_pack_strerror(pack_ret));
      return EXIT_FAILURE;
   }

   if((pack_ret = cull_pack_elem(&pb, ep)) != PACK_SUCCESS) {
      printf("packing element failed: %s\n", cull_pack_strerror(pack_ret));
      return EXIT_FAILURE;
   }

   if (counted_size != pb.bytes_used) {
      printf("just_count does not work, reported "sge_u32", expected "sge_u32"\n", counted_size, (u_long32)pb.bytes_used); 
      return EXIT_FAILURE;
   }
   printf("element uses "sge_u32" kb, mem_size is "sge_u32" kb\n", (u_long32)pb.bytes_used/1024, (u_long32)pb.mem_size/1024);

   buffer = sge_malloc(pb.bytes_used);
   memcpy(buffer, pb.head_ptr, pb.bytes_used);
   if((pack_ret = init_packbuffer_from_buffer(&copy_pb, buffer, pb.bytes_used)) != PACK_SUCCESS) {
      printf("initializing packbuffer from packed data failed: %s\n", cull_pack_strerror(pack_ret));
      return EXIT_FAILURE;
   }

   if((pack_ret = cull_unpack_elem(&copy_pb, &copy, TEST_Type)) != PACK_SUCCESS) {
      printf("unpacking element failed: %s\n", cull_pack_strerror(pack_ret));
      return EXIT_FAILURE;
   }
   clear_packbuffer(&pb);
   clear_packbuffer(&copy_pb);

#if 0
   printf("element after packing and unpacking\n");
   lWriteElemTo(copy, stdout);
#endif
   lFreeElem(&copy);

   /* test partial packing */
   if((pack_ret = init_packbuffer(&pb, 100, 0)) != PACK_SUCCESS) {
      printf("intializing packbuffer failed: %s\n", cull_pack_strerror(pack_ret));
      return EXIT_FAILURE;
   }

   if((pack_ret = cull_pack_elem_partial(&pb, ep, NULL, CULL_SPOOL)) != PACK_SUCCESS) {
      printf("partially packing element failed: %s\n", cull_pack_strerror(pack_ret));
      return EXIT_FAILURE;
   }

   buffer = sge_malloc(pb.bytes_used);
   memcpy(buffer, pb.head_ptr, pb.bytes_used);
   if((pack_ret = init_packbuffer_from_buffer(&copy_pb, buffer, pb.bytes_used)) != PACK_SUCCESS) {
      printf("initializing packbuffer from partially packed data failed: %s\n", cull_pack_strerror(pack_ret));
      return EXIT_FAILURE;
   }

   if((pack_ret = cull_unpack_elem_partial(&copy_pb, &copy, TEST_Type, CULL_SPOOL)) != PACK_SUCCESS) {
      printf("partially unpacking element failed: %s\n", cull_pack_strerror(pack_ret));
      return EXIT_FAILURE;
   }
   clear_packbuffer(&pb);
   clear_packbuffer(&copy_pb);

#if 0
   printf("element after partial packing and unpacking\n");
   lWriteElemTo(copy, stdout);
#endif
   lFreeElem(&copy);

   /* test lDump functions */
   if(lDumpElem(filename, ep, 1)) {
      printf("error dumping element\n");
      return EXIT_FAILURE;
   }

   if((fd = fopen(filename, "r")) == NULL) {
      printf("error opening dump file test_cull_pack.txt\n");
      return EXIT_FAILURE;
   }

   if((copy = lUndumpElemFp(fd, TEST_Type)) == NULL) {
      FCLOSE(fd);
      unlink(filename);
      printf("error undumping element\n");
      return EXIT_FAILURE;
   }
   FCLOSE(fd);
#if 0
   printf("element after dumping and undumping\n");
   lWriteElemTo(copy, stdout);
#endif
   lFreeElem(&copy);
   unlink(filename);

   /* cleanup and exit */
   lFreeElem(&ep);
   return EXIT_SUCCESS;
FCLOSE_ERROR:
   printf(MSG_FILE_NOCLOSE_SS, filename, strerror(errno));
   return EXIT_FAILURE;
}


