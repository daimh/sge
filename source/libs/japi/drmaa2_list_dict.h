#ifndef DRMAA2_LIST_DICT_H
#define DRMAA2_LIST_DICT_H

#include <sys/types.h>

typedef struct _drmaa2_node
{
   void *value;
   struct _drmaa2_node *prev;
   struct _drmaa2_node *next;
} _drmaa2_Node;

/* static */ struct drmaa2_list_s
{
   _drmaa2_Node   *head;
   _drmaa2_Node   *tail;
   _drmaa2_Node   *current;
   size_t         valuesize;
   unsigned long  listsize;
   unsigned long  current_pos;
} drmaa2_list_s;

typedef struct _gw_dict_elem
{
  char * key;
  char * value;
} _gw_dict_elem;

typedef struct _drmaa2_dictentry_t
{
  struct _gw_dict_elem* elem;
  struct _drmaa2_dictentry_t* prev;
  struct _drmaa2_dictentry_t* next;
} _drmaa2_dictentry_t;

/* static */ struct drmaa2_dict_s
{
  _drmaa2_dictentry_t    *head;
  _drmaa2_dictentry_t    *tail;
  _drmaa2_dictentry_t    *current;
  size_t         valuesize;
  unsigned long  dictsize;
  unsigned long  current_pos;
} drmaa2_dict_s;

#endif
