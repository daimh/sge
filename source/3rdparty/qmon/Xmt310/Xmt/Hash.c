/* 
 * Motif Tools Library, Version 3.1
 * $Id: Hash.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Hash.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

/*
 * This file is derived from the X11R5 source code.
 * See the file COPYRIGHT for the MIT and Digital copyrights.
 */

#include <Xmt/Xmt.h>
#include <Xmt/Hash.h>

typedef struct _Bucket {	/* Stores one entry. */
    XtPointer key;
    XtPointer data;
    struct _Bucket *next;
} Bucket;

typedef struct _XmtHashTableRec {
    Bucket **table;		/* Pointer to array of hash entries. */
    long size;                   /* log2 of the size */
    long mask;			/* Current size of hash table minus 1. */
    long numentries;		/* Number of entries currently in table. */
} XmtHashTableRec;


#define Hash(ht,key) \
    (ht)->table[(((long)(key)) >> 2) & (ht)->mask]

#if NeedFunctionPrototypes
static void ResizeTable(register XmtHashTable ht, Boolean grow)    
#else
static void ResizeTable(ht, grow)
register XmtHashTable ht;
Boolean grow;    
#endif
{
    Bucket **otable;
    long otablesize;
    register Bucket *bucket, *next, **head;
    register long i;

    otable = ht->table;
    otablesize =  1 << ht->size;

    if (grow) ht->size++;
    else if (ht->size > 2) ht->size--;
    else return;
    
    ht->table = (Bucket **) XtCalloc(1<<ht->size, sizeof(Bucket *));
    ht->mask = (1<<ht->size) - 1;

    for(i=0; i < otablesize; i++) {
	for(bucket = otable[i]; bucket; bucket = next) {
	    next = bucket->next;
	    head = &Hash(ht, bucket->key);
	    bucket->next = *head;
	    *head = bucket;
	}
    }
    XtFree((char *) otable);
}

#if NeedFunctionPrototypes
XmtHashTable XmtHashTableCreate(int size)
#else
XmtHashTable XmtHashTableCreate(size)
int size;
#endif
{
    XmtHashTable ht = (XmtHashTable) XtMalloc(sizeof(XmtHashTableRec));

    ht->size = size;
    ht->mask = (1<<size)-1;
    ht->table = (Bucket **)XtCalloc(ht->mask + 1, sizeof(Bucket *));
    ht->numentries = 0;
    return ht;
}

#if NeedFunctionPrototypes
void XmtHashTableDestroy(XmtHashTable ht)
#else
void XmtHashTableDestroy(ht)
XmtHashTable ht;
#endif
{
    register long i;
    register Bucket *bucket, *next;

    for(i=0; i < ht->mask+1; i++) {
	for (bucket = ht->table[i]; bucket; bucket = next) {
	    next = bucket->next;
	    XtFree((char *)bucket);
	}
    }
    XtFree((char *) ht->table);
    XtFree((char *) ht);
}

#if NeedFunctionPrototypes
void XmtHashTableForEach(XmtHashTable table, XmtHashTableForEachProc proc)
#else
void XmtHashTableForEach(table, proc)
XmtHashTable table;
XmtHashTableForEachProc proc;
#endif
{
    register long i;
    register Bucket *bucket;

    for(i=0; i < table->mask+1; i++) {
	for (bucket = table->table[i]; bucket; bucket = bucket->next)
	    (*proc)(table, bucket->key, &bucket->data);
    }
}    

#if NeedFunctionPrototypes
void XmtHashTableStore(XmtHashTable table, XtPointer key, XtPointer data)
#else
void XmtHashTableStore(table, key, data)
XmtHashTable table;
XtPointer key;
XtPointer data;
#endif
{
    Bucket **head;
    register Bucket *bucket;

    head = &Hash(table, key);
    for (bucket = *head; bucket; bucket = bucket->next) {
	if (bucket->key == key) {
	    bucket->data = data;
	    return;
	}
    }
    bucket = XtNew(Bucket);
    bucket->key = key;
    bucket->data = data;
    bucket->next = *head;
    *head = bucket;
    table->numentries++;
    if (table->numentries > table->mask)
	ResizeTable(table, True);
    return;
}

#if NeedFunctionPrototypes
Boolean XmtHashTableLookup(XmtHashTable table, XtPointer key, XtPointer *data)
#else
Boolean XmtHashTableLookup(table, key, data)
XmtHashTable table;
XtPointer key;
XtPointer *data;
#endif
{
    register Bucket *bucket;

    for (bucket = Hash(table, key); bucket; bucket = bucket->next)
    {
	if (bucket->key == key) {
	    *data = bucket->data;
	    return True;
	}
    }
    return False;
}

#if NeedFunctionPrototypes
void XmtHashTableDelete(XmtHashTable table, XtPointer key)
#else
void XmtHashTableDelete(table, key)
XmtHashTable table;
XtPointer key;
#endif
{
    register Bucket *bucket, **prev;

    for (prev = &Hash(table, key); (bucket = *prev); prev = &bucket->next) {
	if (bucket->key == key) {
	    *prev = bucket->next;
	    XtFree((char *) bucket);
	    table->numentries--;
	    if (table->numentries < (table->mask>>1))
		ResizeTable(table, False);
	    return;
	}
    }
}
