/* drmaa2_list_dict.c -- DRMAA2 list and dictionary routines */

/* Extracted from the Gridway implementation with mods for error
   reporting and namespace cleanliness.  (It looks better than the one
   in the drmaav2-mock distribution.)  Maybe it's possible to adapt SGE
   CULL, but grabbing this is quicker and simpler.  */

/* -------------------------------------------------------------------------- */
/* Copyright 2002-2012, GridWay Project Leads (GridWay.org)                   */
/*                                                                            */
/* Licensed under the Apache License, Version 2.0 (the "License"); you may    */
/* not use this file except in compliance with the License. You may obtain    */
/* a copy of the License at                                                   */
/*                                                                            */
/* http://www.apache.org/licenses/LICENSE-2.0                                 */
/*                                                                            */
/* Unless required by applicable law or agreed to in writing, software        */
/* distributed under the License is distributed on an "AS IS" BASIS,          */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   */
/* See the License for the specific language governing permissions and        */
/* limitations under the License.                                             */
/* -------------------------------------------------------------------------- */

#include <stdlib.h>
#include <string.h>

#include "drmaa2.h"

#define STRING_BUFSIZE   100

//==================================================================
//               List related functions
//==================================================================

drmaa2_list  drmaa2_list_create (const drmaa2_listtype t, const drmaa2_list_entryfree callback)
{
    drmaa2_list list=NULL;

    if((list = (drmaa2_list) malloc(sizeof(drmaa2_list_s))) == NULL)
    {
        _drmaa2_err_set (DRMAA2_OUT_OF_RESOURCE, "Memeory allocation failure!");
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->current = NULL;
    list->valuesize = sizeof(char)*STRING_BUFSIZE;
    list->listsize = 0;
    list->current_pos = 0;

    switch (t)
    {
      case DRMAA2_STRINGLIST:
        return (drmaa2_string_list)list;
        break;
      case DRMAA2_JOBLIST:
        return (drmaa2_j_list)list;
        break;
      case DRMAA2_QUEUEINFOLIST:
        return (drmaa2_queueinfo_list)list;
        break;
      case DRMAA2_MACHINEINFOLIST:
        return (drmaa2_machineinfo_list)list;
        break;
      case DRMAA2_SLOTINFOLIST:
        return (drmaa2_slotinfo_list)list;
        break;
      case DRMAA2_RESERVATIONLIST:
        return (drmaa2_r_list)list;
        break;
      default:
        return list;
        break;
    }

    return list;
}



//----------------------------------------------------------
//----------------------------------------------------------
static drmaa2_error MoveToHead(drmaa2_list list)
{
    if(list->head == NULL)
    {
        return _drmaa2_err_set (DRMAA2_INTERNAL, "List head is NULL!");
    }

    list->current = list->head;
    list->current_pos = 0;

    return DRMAA2_SUCCESS;
}



//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_list_free(drmaa2_list* list)
{
    void *oldData;
    _drmaa2_Node *oldNode;

    if(*list != NULL && (*list)->head != NULL)
    {
      do
      {
        oldData = (*list)->head->value;
        oldNode = (*list)->head;
        (*list)->head = (*list)->head->next;
        free(oldData);
        free(oldNode);
      } while((*list)->head != NULL);

      free(*list);
      *list = DRMAA2_UNSET_LIST;
    }
    else
    {
      if(*list == NULL)
      {
         _drmaa2_err_set (DRMAA2_INTERNAL, "List is NULL!");
      }
      if((*list)->head == NULL)
      {
         _drmaa2_err_set (DRMAA2_INTERNAL, "List is NULL!");
      }
    }
}



//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_list_add(drmaa2_list list, const void* value)
{
    _drmaa2_Node *newNode = NULL, *old;
    void *newData = NULL;

    if((newNode = (_drmaa2_Node *) malloc(sizeof(_drmaa2_Node))) == NULL)
    {
        return _drmaa2_err_set (DRMAA2_OUT_OF_RESOURCE,
                                "Memory allocation failure!");
    }

    if((newData = (void *) malloc(list->valuesize)) == NULL)
    {
        free(newNode);
        return _drmaa2_err_set (DRMAA2_OUT_OF_RESOURCE,
                                "Memory allocation failure!");
    }

    memcpy(newData, value, list->valuesize);

    if(list->head == NULL)
    {
        (newNode)->value = newData;
        (newNode)->next = NULL;
        (newNode)->prev = NULL;
        list->head = newNode;
        list->tail = newNode;
        list->current = newNode;
        list->listsize = 1;
        list->current_pos = 0;

        return DRMAA2_SUCCESS;
    }

     old = list->tail;

     list->current_pos = list->listsize+1;
     newNode->value = newData;
     old->next = newNode;
     newNode->next = NULL;
     newNode->prev = old;
     list->tail = newNode;
     list->current = newNode;
     list->listsize++;

     return DRMAA2_SUCCESS;
}


//----------------------------------------------------------
//----------------------------------------------------------
long drmaa2_list_size(const drmaa2_list list)
{
   return list->listsize;
}


//----------------------------------------------------------
//----------------------------------------------------------
static drmaa2_error MoveToNext(drmaa2_list list)
{
    if(list->current == NULL)
    {
        return _drmaa2_err_set (DRMAA2_INTERNAL,
                                "Current element of list is NULL!");
    }

    if(list->current->next == NULL)
    {
        return _drmaa2_err_set (DRMAA2_INTERNAL,
                                "Next element of list is NULL!");
    }

    list->current = list->current->next;
    list->current_pos++;

    return(DRMAA2_SUCCESS);
}

#if 0
//----------------------------------------------------------
//----------------------------------------------------------
void display_list(drmaa2_list list)
{
    if(list->current == NULL)
        return;

    if(MoveToHead(list) != DRMAA2_SUCCESS)
    {
        printf("List is empty!\n");
        return;
    }

    do
    {
        printf("%s\n",(char*)(list->current->value));
    }
    while(MoveToNext(list) == DRMAA2_SUCCESS);

}
#endif



//----------------------------------------------------------
//----------------------------------------------------------
const void* drmaa2_list_get(const drmaa2_list list, long pos)
{
    _drmaa2_Node *node=NULL;

    if(list->current == NULL)
    {
       _drmaa2_err_set (DRMAA2_INTERNAL, "Current element of list is NULL!");
       return NULL;
    }
    if(pos<0 || pos>=list->listsize)
    {
       _drmaa2_err_set (DRMAA2_INTERNAL, "Index of list is out of range!");
       return NULL;
    }

    if(MoveToHead(list) != DRMAA2_SUCCESS)
    {
       _drmaa2_err_set (DRMAA2_INTERNAL, "Moving to head of list failed!");
       return NULL;
    }

    do
    {
        if(list->current_pos==pos)
        {
           node=list->current;
           return node->value;
        }
    }while(MoveToNext(list) == DRMAA2_SUCCESS);

    return NULL;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_list_del(drmaa2_list list, long pos)
{
    void *oldData;
    _drmaa2_Node *oldNode;

    if(pos<0 || pos>=list->listsize)
    {
       return _drmaa2_err_set (DRMAA2_INTERNAL,
                               "Index of list is out of range!");
    }

    if(MoveToHead(list) != DRMAA2_SUCCESS)
    {
       return _drmaa2_err_set (DRMAA2_INTERNAL,
                               "Moving to head of list failed!");
    }

    do
    {
        if(list->current_pos==pos) break;
    }while(MoveToNext(list) == DRMAA2_SUCCESS);

    if(list->current == NULL)
    {
       return _drmaa2_err_set (DRMAA2_INTERNAL,
                               "Current element of list is NULL!");
    }

    oldData = list->current->value;
    oldNode = list->current;

    if(list->current == list->head)
    {
        if(list->current->next != NULL)
            list->current->next->prev = NULL;

        list->head = list->current->next;
        list->current = list->head;
    }
    else if(list->current == list->tail)
    {
        list->current->prev->next = NULL;
        list->tail = list->current->prev;
        list->current = list->tail;
        list->current_pos--;
    }
    else
    {
        list->current->prev->next = list->current->next;
        list->current->next->prev = list->current->prev;
        list->current = list->current->next;
    }

    free(oldData);
    free(oldNode);
    list->listsize--;

    return DRMAA2_SUCCESS;
}

//==================================================================
//               Dict related functions
//==================================================================

drmaa2_dict drmaa2_dict_create(const drmaa2_dict_entryfree callback)
{
    drmaa2_dict dict=NULL;
    if((dict = (drmaa2_dict) malloc(sizeof(drmaa2_dict_s))) == NULL)
    {
        _drmaa2_err_set (DRMAA2_OUT_OF_RESOURCE, "Memory allocation failure!");
        return NULL;
    }

    dict->head = NULL;
    dict->tail = NULL;
    dict->current = NULL;
    dict->valuesize = sizeof(drmaa2_dict_s);
    dict->dictsize = 0;
    dict->current_pos = 0;

    return dict;
}


//----------------------------------------------------------
//----------------------------------------------------------
int drmaa2_dict_size(const drmaa2_dict dict)
{
   return dict->dictsize;
}


//----------------------------------------------------------
//----------------------------------------------------------
static drmaa2_error MoveToHead_Dict(drmaa2_dict dict)
{
    if(dict->head == NULL)
    {
        return _drmaa2_err_set (DRMAA2_INTERNAL, "Dict head is NULL!");
    }

    dict->current = dict->head;
    dict->current_pos = 0;

    return DRMAA2_SUCCESS;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error MoveToNext_Dict(drmaa2_dict dict)
{
    if(dict->current == NULL)
    {
        return _drmaa2_err_set (DRMAA2_INTERNAL,
                                "Current element of dict is NULL!");
    }

    if(dict->current->next == NULL)
    {
        return _drmaa2_err_set (DRMAA2_INTERNAL, "Next element of dict is NULL!");
    }

    dict->current = dict->current->next;
    dict->current_pos++;

    return(DRMAA2_SUCCESS);
}


//----------------------------------------------------------
//----------------------------------------------------------
static _drmaa2_dictentry_t* FindNode_dict(drmaa2_dict dict, void* data)
{
    int cmp;
    _drmaa2_dictentry_t* entry=NULL;

    _gw_dict_elem* tmp;

    if(dict->current == NULL)
    {
        _drmaa2_err_set (DRMAA2_INTERNAL, "Current element of dict is NULL!");
        return NULL;
    }

    if(MoveToHead_Dict(dict) != DRMAA2_SUCCESS)
    {
        _drmaa2_err_set (DRMAA2_INTERNAL, "Moving to head of dict failed!");
        return NULL;
    }

    do
    {
        tmp=(dict->current)->elem;
        cmp=memcmp(data,(void*)(tmp->key),strlen(tmp->key)>=strlen(data)?strlen(tmp->key):strlen(data));
        if(cmp==0)
        {
          entry=dict->current;
          break;
        }
    }while(MoveToNext_Dict(dict) == DRMAA2_SUCCESS);

    return entry;
}


//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_dict_free(drmaa2_dict* dict)
{
    _gw_dict_elem* oldData;
    _drmaa2_dictentry_t*  oldNode;

    if(*dict != NULL && (*dict)->head != NULL)
    {
      do
      {
        oldData = (*dict)->head->elem;
        oldNode = (*dict)->head;
        (*dict)->head = (*dict)->head->next;
        free(oldData);
        free(oldNode);
      } while((*dict)->head != NULL);

      free(*dict);
      *dict = DRMAA2_UNSET_DICT;
    }
    else
    {
      if(*dict == NULL)
      {
         _drmaa2_err_set (DRMAA2_INTERNAL, "Dict is NULL!");
      }
      if((*dict)->head == NULL)
      {
         _drmaa2_err_set (DRMAA2_INTERNAL, "Dict head is NULL!");
      }
    }
}

#if 0
//----------------------------------------------------------
//----------------------------------------------------------
void display_dict(drmaa2_dict dict)
{
    if(dict->current == NULL)
        return;

    if(MoveToHead_Dict(dict) != DRMAA2_SUCCESS)
    {
        printf("Dict is empty!\n");
        return;
    }

    do
    {
        printf("key: %s  value: %s\n",dict->current->elem->key,dict->current->elem->value);
    }
    while(MoveToNext_Dict(dict) == DRMAA2_SUCCESS);

}
#endif

//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string_list drmaa2_dict_list(const drmaa2_dict dict)
{

    drmaa2_list keys=NULL;
    char* key;

    if((keys = (drmaa2_list) malloc(sizeof(drmaa2_list_s))) == NULL)
    {
        _drmaa2_err_set (DRMAA2_OUT_OF_RESOURCE, "Memory allocation failure!");
        return NULL;
    }

    keys->head = NULL;
    keys->tail = NULL;
    keys->current = NULL;
    keys->valuesize = sizeof(char)*STRING_BUFSIZE;
    keys->listsize = 0;
    keys->current_pos = 0;

    if(dict->current == NULL)
    {
        _drmaa2_err_set (DRMAA2_INTERNAL, "Current element of dict is NULL!");
	free(keys);
        return NULL;
    }

    if(MoveToHead_Dict(dict) != DRMAA2_SUCCESS)
    {
        _drmaa2_err_set (DRMAA2_INTERNAL, "Moving to head of dict failed!");
	free(keys);
        return NULL;
    }

    do
    {
        if((key=(char*) malloc(strlen(dict->current->elem->key)))==NULL)
        {
            _drmaa2_err_set (DRMAA2_OUT_OF_RESOURCE,
                             "Memory allocation failure!");
	    free(key);
            return NULL;
        }
        key=strdup(dict->current->elem->key);
        if(drmaa2_list_add(keys, key) == DRMAA2_OUT_OF_RESOURCE)
        {
           free(key);
           return NULL;
        }
        free(key);
    }while(MoveToNext_Dict(dict) == DRMAA2_SUCCESS);

    return (drmaa2_string_list) keys;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_bool drmaa2_dict_has(const drmaa2_dict dict, const char* key)
{
    int cmp;
    _gw_dict_elem* tmp;

    if(dict->current == NULL)
    {
        return _drmaa2_err_set (DRMAA2_INTERNAL,
                                "Current element of dict is NULL!");
    }

    if(MoveToHead_Dict(dict) != DRMAA2_SUCCESS)
        return DRMAA2_FALSE;

    do
    {
        tmp=dict->current->elem;
        cmp=memcmp(key,(void*)tmp->key,strlen(tmp->key)>=strlen(key)?strlen(tmp->key):strlen(key));
        if(cmp==0) return DRMAA2_TRUE;
    }while(MoveToNext_Dict(dict) == DRMAA2_SUCCESS);

    return DRMAA2_FALSE;
}


//----------------------------------------------------------
//----------------------------------------------------------
const char* drmaa2_dict_get(const drmaa2_dict dict, const char* key)
{
  _drmaa2_dictentry_t* entry=NULL;


  if((entry=FindNode_dict(dict,(void*)key))!=NULL)
    return entry->elem->value;
  else
  {
     _drmaa2_err_set (DRMAA2_INTERNAL, "Finding element of dict failed!");
     return NULL;
  }
}



//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_dict_del(drmaa2_dict dict, const char* key)
{
    _drmaa2_dictentry_t* entry=NULL;

    _drmaa2_dictentry_t* saved=NULL;
    unsigned long saved_index;

    _gw_dict_elem *oldData;
    _drmaa2_dictentry_t *oldNode;

    if((entry=FindNode_dict(dict,(void*)key))==NULL)
    {
       return _drmaa2_err_set (DRMAA2_INTERNAL,
                               "Finding element of dict failed!");
    }
    if(dict->current == NULL)
    {
       return _drmaa2_err_set (DRMAA2_INTERNAL,
                               "Current element of dict is NULL!");
    }
    saved = dict->current;
    saved_index = dict->current_pos;

    dict->current=entry;

    oldData = dict->current->elem;
    oldNode = dict->current;

    if(dict->current == dict->head)
    {
        if(dict->current->next != NULL)
            dict->current->next->prev = NULL;

        dict->head = dict->current->next;
        dict->current = dict->head;
    }
    else if(dict->current == dict->tail)
    {
        dict->current->prev->next = NULL;
        dict->tail = dict->current->prev;
        dict->current = dict->tail;
        dict->current_pos--;
     }
     else
     {
        dict->current->prev->next = dict->current->next;
        dict->current->next->prev = dict->current->prev;
        dict->current = dict->current->next;
     }

    free(oldData);
    free(oldNode);
    dict->dictsize--;

    dict->current = saved;
    saved = NULL;
    dict->current_pos = saved_index;

    return DRMAA2_SUCCESS;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_dict_set(drmaa2_dict dict, const char* key, const char* value)
{
    _gw_dict_elem dictentry;
    _drmaa2_dictentry_t* newNode = NULL, *old;
    _gw_dict_elem *newData = NULL;

    if(drmaa2_dict_has(dict,key)==DRMAA2_TRUE)
    {
       return _drmaa2_err_set (DRMAA2_INTERNAL,
                               "Element of dict already existed!");
    }

    dictentry.key = strdup(key);
    dictentry.value = strdup(value);

    if((newNode = (_drmaa2_dictentry_t *) malloc(sizeof(_drmaa2_dictentry_t))) == NULL)
    {
        free (dictentry.key);
        free (dictentry.value);
        return _drmaa2_err_set (DRMAA2_OUT_OF_RESOURCE,
                                "Memory allocation failure!");
    }

    if((newData = (void *) malloc(dict->valuesize)) == NULL)
    {
       free(newNode);
       free(dictentry.key);
       free(dictentry.value);
       return _drmaa2_err_set (DRMAA2_OUT_OF_RESOURCE,
                               "Memory allocation failure!");
    }

    memcpy((void*)newData, (void*)&dictentry, sizeof(_gw_dict_elem));

    if(dict->head == NULL)
    {
        (newNode)->elem = newData;
        (newNode)->next = NULL;
        (newNode)->prev = NULL;
        dict->head = newNode;
        dict->tail = newNode;
        dict->current = newNode;
        dict->dictsize = 1;
        dict->current_pos = 0;

        return DRMAA2_SUCCESS;
    }

     old = dict->tail;

     dict->current_pos = dict->dictsize+1;
     newNode->elem = newData;
     old->next = newNode;
     newNode->next = NULL;
     newNode->prev = old;
     dict->tail = newNode;
     dict->current = newNode;
     dict->dictsize++;

    return DRMAA2_SUCCESS;
}
