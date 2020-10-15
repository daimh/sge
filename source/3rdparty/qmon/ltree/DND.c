#ifndef DND 
   static int DNDdummy;
#else   
/*-------------------------------------------------------------------------*/
/*----------------- Drag & Drop -------------------------------------------*/
/*-------------------------------------------------------------------------*/

#include <stdio.h>

#include <Xm/MessageB.h>
#include <Xm/AtomMgr.h>
#include <Xm/DragDrop.h>

#include "ListTreeP.h"
#include "DND.h"

#if NeedFunctionPrototypes
static void DragDropFinish(Widget w, XtPointer cld,  XtPointer cad);
static Boolean ConvertProc(Widget w, Atom *selection, Atom *target, 
                     Atom *type_return, XtPointer *value_return, 
                     unsigned long *length_return, int *format_return);
static void TransferProc(Widget w, XtPointer cld, Atom *seltype, Atom *type,
                  XtPointer value, unsigned long *length, int format);
static void HandleDropOK(Widget w, XtPointer cld, XtPointer cad);
static void HandleDropCancel(Widget w, XtPointer cld, XtPointer cad);
static void HandleDropLabel(Widget w, XtPointer cld, XtPointer cad);
#else
static void DragDropFinish();
static Boolean ConvertProc();
static void TransferProc();
static void HandleDropOK();
static void HandleDropCancel();
static void HandleDropLabel();
#endif


/*-------------------------------------------------------------------------*/
/* InitializeDropSite() registers the list tree widget as a drop site for  */
/* list sub trees                                                          */
/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
void InitializeDropSite(Widget aw)
#else
void InitializeDropSite(aw)
Widget aw;
#endif
{
   int ac;
   Arg args[10];
   Atom LIST_SUB_TREE;
   Atom import_list[1];
   Display *dpy;

   dpy = XtDisplay(aw);
   LIST_SUB_TREE = XmInternAtom(dpy, "LIST_SUB_TREE", False);

   import_list[0] = LIST_SUB_TREE;

   ac = 0;
   XtSetArg(args[ac], XmNimportTargets, import_list); ac++;
   XtSetArg(args[ac], XmNnumImportTargets, XtNumber(import_list)); ac++;
   XtSetArg(args[ac], XmNdropSiteOperations, XmDROP_COPY); ac++;
   XtSetArg(args[ac], XmNdropProc, HandleDropLabel); ac++;
   XmDropSiteRegister(aw, args, ac);

}

/*-------------------------------------------------------------------------*/
/* HandleDropLabel() -- start the data transfer when data is dropped in    */
/* the filename status area.                                               */
/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void HandleDropLabel(Widget w, XtPointer cld, XtPointer cad)
#else
static void HandleDropLabel(w, cld, cad)
Widget          w;
XtPointer       cld;
XtPointer       cad;
#endif
{
#if 0
   static Widget help_dialog = 0;
   Display *dpy;
   Atom LIST_SUB_TREE;
   static struct {
      XmDropProcCallbackStruct client;
      DNDInfo dndinfo;
   } data;
   XmDropProcCallback DropData;
   XmDropTransferEntryRec transferEntries[1];
   XmDropTransferEntry transferList;
   Arg args[10];
   int ac, i;
   Widget dc;
   Cardinal numExportTargets;
   Atom *exportTargets;
   Boolean list_sub_tree = False;
   static DNDInfo dndinfo;
   XmString message;

   /* intern the Atoms for data targets */
   dpy = XtDisplay (w);
   LIST_SUB_TREE = XmInternAtom(dpy, "LIST_SUB_TREE", False);

   DropData = (XmDropProcCallback) cad;
   dc = DropData->dragContext;

   /* get the item */
   dndinfo.item = GetItem((ListTreeWidget)w, DropData->y);
   dndinfo.list_tree = (ListTreeWidget)w;

   /* retrieve the data targets and search for LIST_SUB_TREE */
   ac = 0;
   XtSetArg(args[ac], XmNexportTargets, &exportTargets); ac++;
   XtSetArg(args[ac], XmNnumExportTargets, &numExportTargets); ac++;
   XtGetValues(dc, args, ac);

   for (i = 0; i < numExportTargets; i++) {
      if (exportTargets[i] == LIST_SUB_TREE) {
         list_sub_tree = True;
         break;
      }
   }

   if (DropData->dropAction == XmDROP_HELP) {
      /* create a dialog if it doesn't already exist */
      if (!help_dialog) {
         ac = 0;
         message = XmStringCreateLtoR("Blabla", XmFONTLIST_DEFAULT_TAG);
         XtSetArg(args[ac], XmNdialogStyle, 
                     XmDIALOG_FULL_APPLICATION_MODAL); ac++;
         XtSetArg(args[ac], XmNtitle, "Drop Help"); ac++;
         XtSetArg(args[ac], XmNmessageString, message); ac++;
         help_dialog = XmCreateInformationDialog((Widget)dndinfo.list_tree,
                           "ListTreeDropHelp", args, ac);
         XmStringFree(message);
         XtAddCallback(help_dialog, XmNokCallback,
                        HandleDropOK, (XtPointer) &data);
         XtAddCallback(help_dialog, XmNcancelCallback,
                        HandleDropCancel, (XtPointer) &data.client);
      }
      /* set up the callback structure for when the user proceeds
       * with the drop and pass it as client data to the callbacks
       * for the buttons.
       */	
      data.client.dragContext = dc;
      data.client.x = DropData->x;
      data.client.y = DropData->y;
      data.client.dropSiteStatus = DropData->dropSiteStatus;
      data.client.operation = DropData->operation;
      data.client.operations = DropData->operations;
      data.dndinfo.list_tree = dndinfo.list_tree;
      data.dndinfo.item = dndinfo.item;

      XtManageChild(help_dialog);
      return;
    }
         

   /* make sure we have a drop that is a copy operation and one of
    * the targets is LIST_SUB_TREE.  if not, set the status to failure.
    */
   ac = 0;
   if ((!list_sub_tree) || (DropData->dropAction != XmDROP) || 
      (DropData->operation != XmDROP_COPY)) {
      XtSetArg(args[ac], XmNtransferStatus, XmTRANSFER_FAILURE); ac++;
      XtSetArg(args[ac], XmNnumDropTransfers, 0); ac++;
   }
   else {
      /* set up transfer requests for drop site */
      transferEntries[0].target = LIST_SUB_TREE;
      transferEntries[0].client_data = &dndinfo;
      transferList = transferEntries;
      XtSetArg(args[ac], XmNdropTransfers, transferEntries); ac++;
      XtSetArg(args[ac], XmNnumDropTransfers, XtNumber(transferEntries));ac++;
      XtSetArg(args[ac], XmNtransferProc, TransferProc); ac++;
   }
   XmDropTransferStart(dc, args, ac);
#endif
}

/*-------------------------------------------------------------------------*/
/* HandleDropOK() -- callback routine for OK button in drop site help      */
/* dialog that processes the drop as normal.                               */
/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void HandleDropOK(Widget w, XtPointer cld, XtPointer cad)
#else
static void HandleDropOK(w, cld, cad)
Widget          w;
XtPointer       cld;
XtPointer       cad;
#endif
{
   Display *dpy;
   Atom LIST_SUB_TREE;
   struct _data {
      XmDropProcCallbackStruct client;
      DNDInfo dndinfo;
   } *data;
   XmDropTransferEntryRec transferEntries[1];
   XmDropTransferEntry transferList;
   Arg args[10];
   int n;
   Widget dc;

   /* intern the Atoms for data targets */
   dpy = XtDisplay(w);
   LIST_SUB_TREE = XmInternAtom(dpy, "LIST_SUB_TREE", False);

   /* get the callback structure passed via client data */
   data = (struct _data*) cld;
   dc = data->client.dragContext;

   n = 0;
   /* if operation is not a copy, the transfer fails */
   if (data->client.operation != XmDROP_COPY) {
      XtSetArg(args[n], XmNtransferStatus, XmTRANSFER_FAILURE); n++;
      XtSetArg(args[n], XmNnumDropTransfers, 0); n++;
   }
   else {
      /* set up transfer requests to process data transfer */
      transferEntries[0].target = LIST_SUB_TREE;
      transferEntries[0].client_data = (XtPointer) &data->dndinfo;
      transferList = transferEntries;
      XtSetArg(args[n], XmNdropTransfers, transferEntries); n++;
      XtSetArg(args[n], XmNnumDropTransfers, 
                  XtNumber(transferEntries)); n++;
      XtSetArg(args[n], XmNtransferProc, TransferProc); n++;
   }
   XmDropTransferStart(dc, args, n);
}

/*-------------------------------------------------------------------------*/
/* HandleDropCancel() -- callback routine for Cancel button in drop site   */
/* help dialog that cancels the transfer.                                  */
/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void HandleDropCancel(Widget w, XtPointer cld, XtPointer cad)
#else
static void HandleDropCancel(w, cld, cad)
Widget w;
XtPointer cld;
XtPointer cad;
#endif
{
   XmDropProcCallbackStruct        *DropData;
   Arg                             args[10];
   int                             n;
   Widget                          dc;

   /* get the callback structures passed via client data */
   DropData = (XmDropProcCallbackStruct *) cld;
   dc = DropData->dragContext;

   /* user has canceled the transfer, so it fails */
   n = 0;
   XtSetArg (args[n], XmNtransferStatus, XmTRANSFER_FAILURE); n++;
   XtSetArg (args[n], XmNnumDropTransfers, 0); n++;
   XmDropTransferStart (dc, args, n);
}


/*-------------------------------------------------------------------------*/
/* TransferProc() -- handle data transfer of converted data from drag      */
/* source to drop site.                                                    */
/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void TransferProc(Widget w, XtPointer cld, Atom *seltype, Atom *type,
                  XtPointer value, unsigned long *length, int format)
#else
static void TransferProc(w, cld, seltype, type, value, length, format)
Widget w;
XtPointer cld;
Atom *seltype;
Atom *type;
XtPointer value;
unsigned long *length;
int format;
#endif
{
   Display *dpy;
   Atom LIST_SUB_TREE;
   DNDInfo *dndinfo = (DNDInfo*)cld;

   /* intern the Atoms for data targets */
   dpy = XtDisplay(w);
   LIST_SUB_TREE = XmInternAtom(dpy, "LIST_SUB_TREE", False);

   if (*type == LIST_SUB_TREE) {
      if (dndinfo->item)
         printf("TransferProc is working '%s' '%s'\n", dndinfo->item->text, 
                     ((ListTreeItem*)value)->text);
      ListTreeReparent((Widget)dndinfo->list_tree, (ListTreeItem*)value, 
                           dndinfo->item); 
   }

}

/*-------------------------------------------------------------------------*/
/* start_drag action routine - starts a drag & drop action                 */
/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
void start_drag(Widget aw, XEvent *event, String *params, 
                    Cardinal *num_params)
#else
void start_drag(aw, event, params, num_params)
Widget aw;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
#if 0
   ListTreeWidget w = (ListTreeWidget)aw;
   ListTreeItem *item;    
   Arg args[20];
   int n;
   Display *dpy;
   Atom LIST_SUB_TREE;
   Atom exportList[1];
   Widget dc;
   Pixel fg, bg;
#if 0
   Pixmap icon, iconmask;
   Widget drag_icon;
#endif
   /* intern the Atoms for data targets */
   dpy = XtDisplay(aw);
   LIST_SUB_TREE = XmInternAtom(dpy, "LIST_SUB_TREE", False);

   /* get the current ListTree Item */
   item = GetItem(w, event->xbutton.y);

   XtVaGetValues( aw,
                  XmNforeground, &fg,
                  XmNbackground, &bg,
                  NULL);

#if 0
   /* create pixmaps for drag icon -- either file or directory */
   icon = XmGetPixmapByDepth(XtScreen(aw), "file.xbm", 1, 0, 1);
   iconmask = XmGetPixmapByDepth(XtScreen(aw), "filemask.xbm", 1, 0, 1);
   if (icon == XmUNSPECIFIED_PIXMAP || iconmask == XmUNSPECIFIED_PIXMAP) {
       puts("Couldn't load pixmaps");
       return;
   }

   n = 0;
   XtSetArg(args[n], XmNpixmap, icon); n++;
   XtSetArg(args[n], XmNmask, iconmask); n++;
   drag_icon = XmCreateDragIcon(aw, "drag_icon", args, n);
#endif

   /* specify resources for DragContext for the transfer */
   n = 0;
   XtSetArg(args[n], XmNblendModel, XmBLEND_ALL); n++;
   XtSetArg(args[n], XmNcursorBackground, bg); n++;
   XtSetArg(args[n], XmNcursorForeground, fg); n++;
/*    XtSetArg(args[n], XmNsourceCursorIcon, drag_icon); n++;  */
   /* establish the list of valid target types */
   exportList[0] = LIST_SUB_TREE;
   XtSetArg(args[n], XmNexportTargets, exportList); n++;
   XtSetArg(args[n], XmNnumExportTargets, 1); n++;
   XtSetArg(args[n], XmNdragOperations, XmDROP_COPY); n++;
   XtSetArg(args[n], XmNconvertProc, ConvertProc); n++;
   XtSetArg(args[n], XmNclientData, (XtPointer)item); n++;

   /* start the drag and register a callback to clean up when done */
   dc = XmDragStart(aw, event, args, n);
   if (dc) {
      XtAddCallback(dc, XmNdragDropFinishCallback, DragDropFinish, NULL);
   }
   else
      fprintf(stderr, "XmDragStart() failed\n");
#endif
}

/*-------------------------------------------------------------------------*/
/* ConvertProc() -- convert the file data to the format requested          */
/* by the drop site.                                                       */
/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static Boolean ConvertProc(Widget w, Atom *selection, Atom *target, 
                     Atom *type_return, XtPointer *value_return, 
                     unsigned long *length_return, int *format_return)
#else
static Boolean ConvertProc(w, selection, target, type_return, value_return, 
       length_return, format_return)
Widget              w;
Atom                *selection;
Atom                *target;
Atom                *type_return;
XtPointer           *value_return;
unsigned long       *length_return;
int                 *format_return;
#endif
{
   Display     *dpy;
   Atom        LIST_SUB_TREE, MOTIF_DROP;
   XtPointer   ptr;
   ListTreeItem *item;

   /* intern the Atoms for data targets */
   dpy = XtDisplay(w);
   LIST_SUB_TREE = XmInternAtom(dpy, "LIST_SUB_TREE", False);
   MOTIF_DROP = XmInternAtom(dpy, "_MOTIF_DROP", False);

   /* check if we are dealing with a drop */
   if (*selection != MOTIF_DROP)
       return False;

   /* get the drag source widget */
   XtVaGetValues(w, XmNclientData, &ptr, NULL);
   item = (ListTreeItem*) ptr;
   
   if (item == NULL)
       return False;

   /* this routine processes only file contents and file name */
   if (*target == LIST_SUB_TREE) {
      /* format the value for transfer */
      *type_return = LIST_SUB_TREE;
      *value_return = (XtPointer) item;
      *length_return = 0;
      *format_return = 0;
      return True;
   }
   else
      return False;
}

/*-------------------------------------------------------------------------*/
/* DragDropFinish() -- clean up after a drag and drop transfer.            */
/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void DragDropFinish(Widget w, XtPointer cld,  XtPointer cad)
#else
static void DragDropFinish(w, cld, cad)
Widget w;
XtPointer cld;
XtPointer cad;
#endif
{
   Widget source_icon = NULL;

   XtVaGetValues(w, XmNsourceCursorIcon, &source_icon, NULL);

   if (source_icon)
      XtDestroyWidget(source_icon);
}
#endif
