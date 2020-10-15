/* 
 * Motif Tools Library, Version 3.1
 * $Id: Xpm.c,v 1.2 2005-05-20 14:39:02 andre Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Xpm.c,v $
 * Revision 1.2  2005-05-20 14:39:02  andre
 * AA-2005-05-20-0: Bugfix:    qmon crash (segmentation fault) on Solaris64
 *                  Bugtraq:   6250603
 *                  Issue:     1541
 *                  Changed:   qmon
 *                  Review:    CR
 *
 * Revision 1.1.1.1  2001/07/18 11:06:03  root
 * Initial checkin.
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

/*
 * This file is derived in part from the XPM 3.0 distribution by
 * Arnaud Le Hors.  See the file COPYRIGHT for the Groupe Bull copyright.
 */

#include <string.h>
#include <Xmt/Xmt.h>
#include <Xmt/Xpm.h>

/*
 * Fix to prevent 'Unaligned access' errors on DEC OSF/1 AXP,
 * when running the DEC port of X11R5. Note that X11R6 defines this
 * correctly for the alpha.
 *				 -- fredrik_lundh@ivab.se 10/25/94
 */
#if defined(__alpha) || defined(TARGET_64BIT)
#  ifndef LONG64
#    define	LONG64
#  endif
#endif

/* Return ErrorStatus codes:
 * null     if full success
 * positive if partial success
 * negative if failure
 */

#define XpmColorError    1
#define XpmSuccess       0
#define XpmOpenFailed   -1
#define XpmFileInvalid  -2
#define XpmNoMemory     -3
#define XpmColorFailed  -4

/*
 * minimal portability layer between ansi and KR C 
 */

/* forward declaration of functions with prototypes */

#if NeedFunctionPrototypes              /* ANSI || C++ */
#define FUNC(f, t, p) extern t f p
#define LFUNC(f, t, p) static t f p
#else					/* K&R */
#define FUNC(f, t, p) extern t f()
#define LFUNC(f, t, p) static t f()
#endif					/* end of K&R */


LFUNC(CreateImage, int, (Widget, 
			 Visual *, Colormap, unsigned int, XmtColorTable,
			 XmtImage * xmtimage,
			 XImage ** image_return,
			 XImage ** shapeimage_return,
			 Pixel **, int *));

LFUNC(VisualType, XmtXpmVisualClass, (Visual * visual));

LFUNC(CreateXImage, int, (Display * display, Visual * visual,
			  unsigned int depth, unsigned int width,
			  unsigned int height, XImage ** image_return));

LFUNC(SetImagePixels, void, (XImage * image, unsigned int width,
			    unsigned int height, unsigned char *pixelindex,
			    Pixel * pixels));

LFUNC(SetImagePixels32, void, (XImage * image, unsigned int width,
			      unsigned int height, unsigned char *pixelindex,
			      Pixel * pixels));

LFUNC(SetImagePixels16, void, (XImage * image, unsigned int width,
			      unsigned int height, unsigned char *pixelindex,
			      Pixel * pixels));

LFUNC(SetImagePixels8, void, (XImage * image, unsigned int width,
			     unsigned int height, unsigned char *pixelindex,
			     Pixel * pixels));

LFUNC(SetImagePixels1, void, (XImage * image, unsigned int width,
			     unsigned int height, unsigned char *pixelindex,
			     Pixel * pixels));


/*
 * Macros
 * 
 * The XYNORMALIZE macro determines whether XY format data requires 
 * normalization and calls a routine to do so if needed. The logic in
 * this module is designed for LSBFirst byte and bit order, so 
 * normalization is done as required to present the data in this order.
 *
 * The ZNORMALIZE macro performs byte and nibble order normalization if 
 * required for Z format data.
 * 
 * The XYINDEX macro computes the index to the starting byte (char) boundary
 * for a bitmap_unit containing a pixel with coordinates x and y for image
 * data in XY format.
 * 
 * The ZINDEX* macros compute the index to the starting byte (char) boundary 
 * for a pixel with coordinates x and y for image data in ZPixmap format.
 * 
 */

#define XYNORMALIZE(bp, img) \
    if ((img->byte_order == MSBFirst) || (img->bitmap_bit_order == MSBFirst)) \
	xpm_xynormalizeimagebits((unsigned char *)(bp), img)

#define ZNORMALIZE(bp, img) \
    if (img->byte_order == MSBFirst) \
	xpm_znormalizeimagebits((unsigned char *)(bp), img)

#define XYINDEX(x, y, img) \
    ((y) * img->bytes_per_line) + \
    (((x) + img->xoffset) / img->bitmap_unit) * (img->bitmap_unit >> 3)

#define ZINDEX(x, y, img) ((y) * img->bytes_per_line) + \
    (((x) * img->bits_per_pixel) >> 3)

#define ZINDEX32(x, y, img) ((y) * img->bytes_per_line) + ((x) << 2)

#define ZINDEX16(x, y, img) ((y) * img->bytes_per_line) + ((x) << 1)

#define ZINDEX8(x, y, img) ((y) * img->bytes_per_line) + (x)

#define ZINDEX1(x, y, img) ((y) * img->bytes_per_line) + ((x) >> 3)

#if __STDC__
#define Const const
#else
#define Const				/**/
#endif


#if NeedFunctionPrototypes
static XmtXpmVisualClass VisualType(Visual *visual)
#else
static XmtXpmVisualClass VisualType(visual)
Visual *visual;
#endif
{
    switch (visual->class) {
    case StaticGray:
    case GrayScale:
	switch (visual->map_entries) {
	case 2:
	    return (Mono);
	case 4:
	    return (Gray4);
	default:
	    return (Gray);
	}
    default:
	return (Color);
    }
}

#if NeedFunctionPrototypes
static Boolean is_transparent(register char *s)
#else
static Boolean is_transparent(s)
register char *s;
#endif
{
    register char *p = "none";
    register char *q = "NONE";
    
    for(; *p; p++, q++, s++)
	if ((*s != *p) && (*s != *q)) return False;

    return True;
}


/*
 * set the color pixel related to the given colorname,
 * return 0 if success, 1 otherwise.
 */

static int
#if NeedFunctionPrototypes
SetColor(Widget widget, Colormap colormap, Visual *visual, XmtColorTable table,
	 char *colorname,
	 Pixel *image_pixel, Pixel *mask_pixel,
	 Pixel *alloc_pixels, int *num_alloc_pixels, Boolean *has_transparent)
#else
SetColor(widget, colormap, visual, table, colorname, image_pixel, mask_pixel,
	 alloc_pixels, num_alloc_pixels, has_transparent)
Widget widget;
Colormap colormap;
Visual *visual;
XmtColorTable table;
char *colorname;
Pixel *image_pixel;
Pixel *mask_pixel;
Pixel *alloc_pixels;
int *num_alloc_pixels;
Boolean *has_transparent;
#endif
{
    if (!is_transparent(colorname)) {
	if (XmtAllocColor(widget, colormap, visual, table, colorname,
			  image_pixel) != 0)
	    return (1);
	*mask_pixel = 1;
	alloc_pixels[*num_alloc_pixels] = *image_pixel;
	*num_alloc_pixels += 1;
    } else {
	*image_pixel = 0;
	*mask_pixel = 0;
	*has_transparent = True;
    }
    return (0);
}


/*
 * Create an XImage
 */
static int
#if NeedFunctionPrototypes
CreateXImage(Display *display, Visual *visual, unsigned int depth,
	     unsigned int width, unsigned int height, XImage **image_return)
#else
CreateXImage(display, visual, depth, width, height, image_return)
Display *display;
Visual *visual;
unsigned int depth;
unsigned int width;
unsigned int height;
XImage **image_return;
#endif
{
    int bitmap_pad;

    /* first get bitmap_pad */
    if (depth > 16)
	bitmap_pad = 32;
    else if (depth > 8)
	bitmap_pad = 16;
    else
	bitmap_pad = 8;

    /* then create the XImage with data = NULL and bytes_per_line = 0 */

    *image_return = XCreateImage(display, visual, depth, ZPixmap, 0, 0,
				 width, height, bitmap_pad, 0);
    if (!*image_return)
	return (XpmNoMemory);

    /* now that bytes_per_line must have been set properly alloc data */

    (*image_return)->data =
	(char *) XtMalloc((*image_return)->bytes_per_line * height);

    return (XpmSuccess);
}


static int
#if NeedFunctionPrototypes
CreateImage(Widget widget, Visual *visual, Colormap colormap,
	    unsigned int depth, XmtColorTable symbolic_colors,
	    XmtImage *xmtimage, XImage **image_return, XImage **mask_return,
	    Pixel **alloc_pixels_return, int *num_alloc_pixels_return)
#else
CreateImage(widget, visual, colormap, depth, symbolic_colors, xmtimage,
	    image_return, mask_return,
	    alloc_pixels_return, num_alloc_pixels_return)
Widget widget;
Visual *visual;
Colormap colormap;
unsigned int depth;
XmtColorTable symbolic_colors;
XmtImage *xmtimage;
XImage **image_return;
XImage **mask_return;
Pixel **alloc_pixels_return;
int *num_alloc_pixels_return;
#endif
{
    Display *display = XtDisplayOfObject(widget);
    
    /* variables to return */
    XImage *image = NULL;
    XImage *mask = NULL;
    Pixel *alloc_pixels = (Pixel *)XtMalloc((xmtimage->ncolors+1)
					    * sizeof(Pixel));
    int num_alloc_pixels = 0;
    unsigned int ErrorStatus;

    /* calculation variables */
    Pixel *image_pixels = NULL;
    Pixel *mask_pixels = NULL;
    char *colorname;
    Boolean has_transparent = False;
    unsigned int color;
    XmtXpmVisualClass default_vclass;
    int vclass;
    Boolean pixel_defined;
    int status;

    /*
     * get default values for unspecified parameters
     */
    if (visual == NULL || visual == (Visual *)CopyFromParent) 
	visual = DefaultVisual(display, DefaultScreen(display));

    if (colormap == None)
	colormap = DefaultColormap(display, DefaultScreen(display));

    if (depth == 0)
	depth = DefaultDepth(display, DefaultScreen(display));


    ErrorStatus = XpmSuccess;

    /*
     * alloc pixel tables 
     */
    default_vclass = VisualType(visual);
    image_pixels = (Pixel *) XtMalloc(sizeof(Pixel) * xmtimage->ncolors);
    mask_pixels = (Pixel *) XtMalloc(sizeof(Pixel) * xmtimage->ncolors);

    /*
     * get pixel colors, store them in index tables 
     */
    for (color = 0; color < xmtimage->ncolors; color++) {
	colorname = NULL;
	pixel_defined = False;

	/*
	 * look for a defined symbol
	 */
	if (symbolic_colors && xmtimage->color_table[color].symbolic_name) {
	    colorname = XmtLookupColorName(symbolic_colors,
			          xmtimage->color_table[color].symbolic_name);
	    if (colorname) {
		if (is_transparent(colorname)) {
		    image_pixels[color] = mask_pixels[color] = 0;
		    has_transparent = True;
		    pixel_defined = True;
		}
		else {
		    int status;
		    status = XmtAllocColor(widget, colormap, visual,
				    symbolic_colors, colorname,
				    &image_pixels[color]);
		    if (status == 0) {
			pixel_defined = True;
			mask_pixels[color] = 1;
			alloc_pixels[num_alloc_pixels++] = image_pixels[color];
		    }
		}
	    }
	}

	/*
	 * if no symbolic name, or nothing matched it, then try to allocate
	 * colors hardcoded into the XmtImage.  Start at the color defined
	 * for the visual class that matches our visual, and if that fails,
	 * try the simpler visuals, and then the more complex visuals.
	 */
	if (!pixel_defined) {	/* pixel not given as symbol value */
	    vclass = default_vclass;
	    while (!pixel_defined && vclass >= 0) {
		if (xmtimage->color_table[color].default_colors[vclass]) {
		    if (!SetColor(widget, colormap, visual, symbolic_colors,
			   xmtimage->color_table[color].default_colors[vclass],
				  &image_pixels[color],
				  &mask_pixels[color],
				  alloc_pixels, &num_alloc_pixels,
				  &has_transparent)) {
			pixel_defined = True;
			break;
		    } else
			ErrorStatus = XpmColorError;
		}
		vclass--;
	    }

	    vclass = default_vclass + 1;
	    while (!pixel_defined && vclass < NVISUALS) {
		if (xmtimage->color_table[color].default_colors[vclass]) {
		    if (!SetColor(widget, colormap, visual, symbolic_colors,
			   xmtimage->color_table[color].default_colors[vclass],
				  &image_pixels[color],
				  &mask_pixels[color],
				  alloc_pixels, &num_alloc_pixels,
				  &has_transparent)) {
			pixel_defined = True;
			break;
		    } else
			ErrorStatus = XpmColorError;
		}
		vclass++;
	    }

	    if (!pixel_defined) {
		status = XpmColorFailed;
		goto error;
	    }
	}
    }

    /*
     * If there are transparent bits, set them to the $background color,
     * if any is defined, in case the bitmask isn't used
     */
    if (has_transparent) {
	Pixel bg;
	int i;

	if (XmtAllocColor(widget, colormap, visual, symbolic_colors,
			  "$background", &bg) == 0) {
	    alloc_pixels[num_alloc_pixels++] = bg;
	    for(i=0; i < (int)xmtimage->ncolors; i++)
		if (mask_pixels[i] == 0) image_pixels[i] = bg;
	}
    }

    /*
     * create the image 
     */
    if (image_return) {
	status = CreateXImage(display, visual, depth,
			      xmtimage->width, xmtimage->height, &image);
	if (status != XpmSuccess)
	    goto error;

	/*
	 * set the image data 
	 *
	 * In case depth is 1 or bits_per_pixel is 4, 6, 8, 24 or 32 use
	 * optimized functions, otherwise use slower but sure general one. 
	 *
	 */

	if (image->depth == 1)
	    SetImagePixels1(image, xmtimage->width, xmtimage->height,
			    xmtimage->data, image_pixels);
	else if (image->bits_per_pixel == 8)
	    SetImagePixels8(image, xmtimage->width, xmtimage->height,
			    xmtimage->data, image_pixels);
	else if (image->bits_per_pixel == 16)
	    SetImagePixels16(image, xmtimage->width, xmtimage->height,
			     xmtimage->data, image_pixels);
	else if (image->bits_per_pixel == 32)
	    SetImagePixels32(image, xmtimage->width, xmtimage->height,
			     xmtimage->data, image_pixels);
	else
	    SetImagePixels(image, xmtimage->width, xmtimage->height,
			   xmtimage->data, image_pixels);
    }

    /*
     * create the shape mask image 
     */
    if (has_transparent && mask_return) {
	status = CreateXImage(display, visual, 1, xmtimage->width,
			      xmtimage->height, &mask);
	if (status != XpmSuccess)
	    goto error;

	SetImagePixels1(mask, xmtimage->width, xmtimage->height,
			xmtimage->data, mask_pixels);
    }
    XtFree((char *)mask_pixels);
    XtFree((char *)image_pixels);

    /*
     * return created images 
     */
    if (image_return)
	*image_return = image;

    if (mask_return)
	*mask_return = mask;

    /* return array of pixels that must be freed */
    if (alloc_pixels_return) {
	alloc_pixels = (Pixel *)XtRealloc((char *)alloc_pixels,
					  num_alloc_pixels*sizeof(Pixel));
	*alloc_pixels_return = alloc_pixels;
    }
    else
	XtFree((char *)alloc_pixels_return);
    if (num_alloc_pixels_return)
	*num_alloc_pixels_return = num_alloc_pixels;
	       

    return (ErrorStatus);

 error:
    if (alloc_pixels) XtFree((char *)alloc_pixels);
    if (image_pixels) XtFree((char *)image_pixels);
    if (mask_pixels) XtFree((char *)mask_pixels);
    if (image) XDestroyImage(image);
    if (mask) XDestroyImage(mask);
    return(status);
}




/*
 * The functions below are written from X11R5 MIT's code (XImUtil.c)
 *
 * The idea is to have faster functions than the standard XPutPixel function
 * to build the image data. Indeed we can speed up things by supressing tests
 * performed for each pixel. We do exactly the same tests but at the image
 * level. Assuming that we use only ZPixmap images. 
 */

LFUNC(_putbits, void, (register char *src, int dstoffset,
		      register int numbits, register char *dst));

LFUNC(_XReverse_Bytes, void, (register unsigned char *bpt, register int nb));

static unsigned char Const _reverse_byte[0x100] = {
			    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
			    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
			    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
			    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
			    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
			    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
			    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
			    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
			    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
			    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
			    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
			    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
			    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
			    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
			    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
			    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
			    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
			    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
			    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
			    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
			    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
			    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
			    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
			    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
			    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
			    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
			    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
			    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
			    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
			    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
			    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
			     0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

static void
#if NeedFunctionPrototypes
_XReverse_Bytes(register unsigned char *bpt, register int nb)
#else
_XReverse_Bytes(bpt, nb)
register unsigned char *bpt;
register int nb;
#endif
{
    do {
	*bpt = _reverse_byte[*bpt];
	bpt++;
    } while (--nb > 0);
}


static void
#if NeedFunctionPrototypes
xpm_xynormalizeimagebits(register unsigned char *bp, register XImage *img)
#else
xpm_xynormalizeimagebits(bp, img)
register unsigned char *bp;
register XImage *img;
#endif
{
    register unsigned char c;

    if (img->byte_order != img->bitmap_bit_order) {
	switch (img->bitmap_unit) {

	case 16:
	    c = *bp;
	    *bp = *(bp + 1);
	    *(bp + 1) = c;
	    break;

	case 32:
	    c = *(bp + 3);
	    *(bp + 3) = *bp;
	    *bp = c;
	    c = *(bp + 2);
	    *(bp + 2) = *(bp + 1);
	    *(bp + 1) = c;
	    break;
	}
    }
    if (img->bitmap_bit_order == MSBFirst)
	_XReverse_Bytes(bp, img->bitmap_unit >> 3);
}

static void
#if NeedFunctionPrototypes
xpm_znormalizeimagebits(register unsigned char *bp, register XImage *img)
#else
xpm_znormalizeimagebits(bp, img)
register unsigned char *bp;
register XImage *img;
#endif
{
    register unsigned char c;

    switch (img->bits_per_pixel) {

    case 2:  /* this case added from Xpm 3.3 code -- djf */
	_XReverse_Bytes(bp, 1);
	break;

    case 4:
	*bp = ((*bp >> 4) & 0xF) | ((*bp << 4) & ~0xF);
	break;

    case 16:
	c = *bp;
	*bp = *(bp + 1);
	*(bp + 1) = c;
	break;

    case 24:
	c = *(bp + 2);
	*(bp + 2) = *bp;
	*bp = c;
	break;

    case 32:
	c = *(bp + 3);
	*(bp + 3) = *bp;
	*bp = c;
	c = *(bp + 2);
	*(bp + 2) = *(bp + 1);
	*(bp + 1) = c;
	break;
    }
}

static unsigned char Const _lomask[0x09] = {
		     0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};
static unsigned char Const _himask[0x09] = {
		     0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00};

static void
#if NeedFunctionPrototypes
_putbits(register char *src, int dstoffset, register int numbits,
	 register char *dst)
#else
_putbits(src, dstoffset, numbits, dst)
register char *src;
int dstoffset;
register int numbits;
register char *dst;
#endif
                       			/* address of source bit string */
                  			/* bit offset into destination;
					 * range is 0-31 */
                         		/* number of bits to copy to
					 * destination */
                       			/* address of destination bit string */
{
    register unsigned char chlo, chhi;
    int hibits;

    dst = dst + (dstoffset >> 3);
    dstoffset = dstoffset & 7;
    hibits = 8 - dstoffset;
    chlo = *dst & _lomask[dstoffset];
    for (;;) {
	chhi = (*src << dstoffset) & _himask[dstoffset];
	if (numbits <= hibits) {
	    chhi = chhi & _lomask[dstoffset + numbits];
	    *dst = (*dst & _himask[dstoffset + numbits]) | chlo | chhi;
	    break;
	}
	*dst = chhi | chlo;
	dst++;
	numbits = numbits - hibits;
	chlo = (unsigned char) (*src & _himask[hibits]) >> hibits;
	src++;
	if (numbits <= dstoffset) {
	    chlo = chlo & _lomask[numbits];
	    *dst = (*dst & _himask[numbits]) | chlo;
	    break;
	}
	numbits = numbits - dstoffset;
    }
}


#ifndef OLDXPMCODE

/*
 * This new, improved code is from the XPM 3.3 library.
 * All I've done to it is added prototypes, and changed the
 * pixelindex argument from an int * to a char *
 *     -- djf
 */
 

/*
 * Default method to write pixels into a Z image data structure.
 * The algorithm used is:
 *
 *	copy the destination bitmap_unit or Zpixel to temp
 *	normalize temp if needed
 *	copy the pixel bits into the temp
 *	renormalize temp if needed
 *	copy the temp back into the destination image data
 */

static void
#if NeedFunctionPrototypes
SetImagePixels(XImage *image, unsigned int width, unsigned int height,
	       unsigned char *pixelindex, Pixel *pixels)
#else
SetImagePixels(image, width, height, pixelindex, pixels)
XImage *image;
unsigned int width;
unsigned int height;
unsigned char *pixelindex;
Pixel *pixels;
#endif
{
    register char *src;
    register char *dst;
    register unsigned char *iptr;
    register int x, y, i;
    register char *data;
    Pixel pixel, px;
    int nbytes, depth, ibu, ibpp;

    data = image->data;
    iptr = pixelindex;
    depth = image->depth;
    if (depth == 1) {
	ibu = image->bitmap_unit;
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		pixel = pixels[*iptr];
		for (i = 0, px = pixel; i < sizeof(unsigned long);
		     i++, px >>= 8)
		    ((unsigned char *) &pixel)[i] = px;
		src = &data[XYINDEX(x, y, image)];
		dst = (char *) &px;
		px = 0;
		nbytes = ibu >> 3;
		for (i = nbytes; --i >= 0;)
		    *dst++ = *src++;
		XYNORMALIZE(&px, image);
		_putbits((char *) &pixel, (x % ibu), 1, (char *) &px);
		XYNORMALIZE(&px, image);
		src = (char *) &px;
		dst = &data[XYINDEX(x, y, image)];
		for (i = nbytes; --i >= 0;)
		    *dst++ = *src++;
	    }
    } else {
	ibpp = image->bits_per_pixel;
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		pixel = pixels[*iptr];
		if (depth == 4)
		    pixel &= 0xf;
		for (i = 0, px = pixel; i < sizeof(unsigned long); i++,
		     px >>= 8)
		    ((unsigned char *) &pixel)[i] = px;
		src = &data[ZINDEX(x, y, image)];
		dst = (char *) &px;
		px = 0;
		nbytes = (ibpp + 7) >> 3;
		for (i = nbytes; --i >= 0;)
		    *dst++ = *src++;
		ZNORMALIZE(&px, image);
		_putbits((char *) &pixel, (x * ibpp) & 7, ibpp, (char *) &px);
		ZNORMALIZE(&px, image);
		src = (char *) &px;
		dst = &data[ZINDEX(x, y, image)];
		for (i = nbytes; --i >= 0;)
		    *dst++ = *src++;
	    }
    }
}

/*
 * write pixels into a 32-bits Z image data structure
 */

#if !defined(WORD64) && !defined(LONG64)
/* this item is static but deterministic so let it slide; doesn't
** hurt re-entrancy of this library. Note if it is actually const then would
** be OK under rules of ANSI-C but probably not C++ which may not
** want to allocate space for it.
*/
static unsigned long /*constant */ RTXpm_byteorderpixel = MSBFirst << 24;

#endif

static void
#if NeedFunctionPrototypes
SetImagePixels32(XImage *image, unsigned int width, unsigned int height,
		 unsigned char *pixelindex, Pixel *pixels)
#else
SetImagePixels32(image, width, height, pixelindex, pixels)
XImage *image;
unsigned int width;
unsigned int height;
unsigned char *pixelindex;
Pixel *pixels;
#endif
{
    unsigned char *data;
    unsigned char *iptr;
    int y;
    Pixel pixel;

    int bpl = image->bytes_per_line;
    unsigned char *data_ptr, *max_data;

    data = (unsigned char *) image->data;
    iptr = pixelindex;
#if !defined(WORD64) && !defined(LONG64)
    if (*((char *) &RTXpm_byteorderpixel) == image->byte_order) {
	for (y = 0; y < height; y++) {
	    data_ptr = data;
	    max_data = data_ptr + (width<<2);

	    while (data_ptr < max_data) {
		*((unsigned long *)data_ptr) = pixels[*(iptr++)];
		data_ptr += (1<<2);
	    }
	    data += bpl;
	}
    } else
#endif
	if (image->byte_order == MSBFirst)
	    for (y = 0; y < height; y++) {
		data_ptr = data;
		max_data = data_ptr + (width<<2);

		while (data_ptr < max_data) {
		    pixel = pixels[*(iptr++)];

		    *data_ptr++ = pixel >> 24;
		    *data_ptr++ = pixel >> 16;
		    *data_ptr++ = pixel >> 8;
		    *data_ptr++ = pixel;

		}
		data += bpl;
	    }
	  else
	      for (y = 0; y < height; y++) {
		  data_ptr = data;
		  max_data = data_ptr + (width<<2);

		  while (data_ptr < max_data) {
		      pixel = pixels[*(iptr++)];

		      *data_ptr++ = pixel;
		      *data_ptr++ = pixel >> 8;
		      *data_ptr++ = pixel >> 16;
		      *data_ptr++ = pixel >> 24;
		  }
		  data += bpl;
	      }
}

/*
 * write pixels into a 16-bits Z image data structure
 */

static void
#if NeedFunctionPrototypes
SetImagePixels16(XImage *image, unsigned int width, unsigned int height,
		 unsigned char *pixelindex, Pixel *pixels)
#else
SetImagePixels16(image, width, height, pixelindex, pixels)
XImage *image;
unsigned int width;
unsigned int height;
unsigned char *pixelindex;
Pixel *pixels;
#endif
{
    unsigned char *data;
    unsigned char *iptr;
    int y;
    Pixel pixel;

    int bpl=image->bytes_per_line;
    unsigned char *data_ptr,*max_data;
    
    data = (unsigned char *) image->data;
    iptr = pixelindex;
    if (image->byte_order == MSBFirst)
	  for (y = 0; y < height; y++) {
	      data_ptr = data;
	      max_data = data_ptr + (width<<1);

	      while (data_ptr < max_data) {
		  pixel = pixels[*(iptr++)];

		  data_ptr[0] = pixel >> 8;
		  data_ptr[1] = pixel;

		  data_ptr+=(1<<1);
	      }
	      data += bpl;
	  }
      else
	  for (y = 0; y < height; y++) {
	      data_ptr  = data;
	      max_data = data_ptr + (width<<1);

	      while (data_ptr < max_data) {
		  pixel = pixels[*(iptr++)];

		  data_ptr[0] = pixel;
		  data_ptr[1] = pixel >> 8;

		  data_ptr+=(1<<1);
	      }
	      data += bpl;
	  }
}

/*
 * write pixels into a 8-bits Z image data structure
 */

static void
#if NeedFunctionPrototypes
SetImagePixels8(XImage *image, unsigned int width, unsigned int height,
		unsigned char *pixelindex, Pixel *pixels)
#else
SetImagePixels8(image, width, height, pixelindex, pixels)
XImage *image;
unsigned int width;
unsigned int height;
unsigned char *pixelindex;
Pixel *pixels;
#endif
{
    char *data;
    unsigned char *iptr;
    int y;
    int bpl = image->bytes_per_line;
    char *data_ptr,*max_data;

    data = image->data;
    iptr = pixelindex;

    for (y = 0; y < height; y++) {
	data_ptr = data;
	max_data = data_ptr + width;

	while (data_ptr < max_data)
	    *(data_ptr++) = pixels[*(iptr++)];

	data += bpl;
    }
}

/*
 * write pixels into a 1-bit depth image data structure and **offset null**
 */

static void
#if NeedFunctionPrototypes
SetImagePixels1(XImage *image, unsigned int width, unsigned int height,
		unsigned char *pixelindex, Pixel *pixels)
#else
SetImagePixels1(image, width, height, pixelindex, pixels)
XImage *image;
unsigned int width;
unsigned int height;
unsigned char *pixelindex;
Pixel *pixels;
#endif
{
    if (image->byte_order != image->bitmap_bit_order)
	SetImagePixels(image, width, height, pixelindex, pixels);
    else {
	unsigned char *iptr;
	int y;
	char *data;
	char value;
	char *data_ptr, *max_data;
	int bpl = image->bytes_per_line;
	int diff, count;

	data = image->data;
	iptr = pixelindex;

	diff = width & 7;
	width >>= 3;

	if (image->bitmap_bit_order == MSBFirst)
	    for (y = 0; y < height; y++) {
		data_ptr = data;
		max_data = data_ptr + width;
		while (data_ptr < max_data) {
		    value=0;

		    value=(value<<1) | (pixels[*(iptr++)] & 1);
		    value=(value<<1) | (pixels[*(iptr++)] & 1);
		    value=(value<<1) | (pixels[*(iptr++)] & 1);
		    value=(value<<1) | (pixels[*(iptr++)] & 1);
		    value=(value<<1) | (pixels[*(iptr++)] & 1);
		    value=(value<<1) | (pixels[*(iptr++)] & 1);
		    value=(value<<1) | (pixels[*(iptr++)] & 1);
		    value=(value<<1) | (pixels[*(iptr++)] & 1);

		    *(data_ptr++) = value;
		}
		if (diff) {
		    value = 0;
		    for (count = 0; count < diff; count++) {
			if (pixels[*(iptr++)] & 1) 
			    value |= (0x80>>count);
		    }
		    *(data_ptr) = value;			  
		}
		data += bpl;
	    }
	else
	    for (y = 0; y < height; y++) {
		data_ptr = data;
		max_data = data_ptr + width;
		while (data_ptr < max_data) {
		    value=0;
		    iptr+=8;

		    value=(value<<1) | (pixels[*(--iptr)] & 1);
		    value=(value<<1) | (pixels[*(--iptr)] & 1);
		    value=(value<<1) | (pixels[*(--iptr)] & 1);
		    value=(value<<1) | (pixels[*(--iptr)] & 1);
		    value=(value<<1) | (pixels[*(--iptr)] & 1);
		    value=(value<<1) | (pixels[*(--iptr)] & 1);
		    value=(value<<1) | (pixels[*(--iptr)] & 1);
		    value=(value<<1) | (pixels[*(--iptr)] & 1);

		    iptr+=8;
		    *(data_ptr++) = value;			  
		}
		if (diff) {
		    value=0;
		    for (count = 0; count < diff; count++) {
			if (pixels[*(iptr++)] & 1) 
			    value |= (1<<count);
		    }
		    *(data_ptr) = value;			  
		}
		data += bpl;
	    }
    }
}

#else  /* OLDXPMCODE */
/*
 * Default method to write pixels into a Z image data structure.
 * The algorithm used is:
 *
 *	copy the destination bitmap_unit or Zpixel to temp
 *	normalize temp if needed
 *	copy the pixel bits into the temp
 *	renormalize temp if needed
 *	copy the temp back into the destination image data
 */

static void
#if NeedFunctionPrototypes
SetImagePixels(XImage *image, unsigned int width, unsigned int height,
	       unsigned char *xpmdata, Pixel *pixels)
#else
SetImagePixels(image, width, height, xpmdata, pixels)
XImage *image;
unsigned int width;
unsigned int height;
unsigned char *xpmdata;
Pixel *pixels;
#endif
{
    Pixel pixel;
    unsigned long px;
    register char *src;
    register char *dst;
    int nbytes;
    register unsigned char *iptr;
    register int x, y, i;

    iptr = xpmdata;
    if (image->depth == 1) {
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		pixel = pixels[*iptr];
		for (i = 0, px = pixel;
		     i < sizeof(unsigned long); i++, px >>= 8)
		    ((unsigned char *) &pixel)[i] = px;
		src = &image->data[XYINDEX(x, y, image)];
		dst = (char *) &px;
		px = 0;
		nbytes = image->bitmap_unit >> 3;
		for (i = nbytes; --i >= 0;)
		    *dst++ = *src++;
		XYNORMALIZE(&px, image);
		i = ((x + image->xoffset) % image->bitmap_unit);
		_putbits((char *) &pixel, i, 1, (char *) &px);
		XYNORMALIZE(&px, image);
		src = (char *) &px;
		dst = &image->data[XYINDEX(x, y, image)];
		for (i = nbytes; --i >= 0;)
		    *dst++ = *src++;
	    }
    } else {
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		pixel = pixels[*iptr];
		if (image->depth == 4)
		    pixel &= 0xf;
		for (i = 0, px = pixel;
		     i < sizeof(unsigned long); i++, px >>= 8)
		    ((unsigned char *) &pixel)[i] = px;
		src = &image->data[ZINDEX(x, y, image)];
		dst = (char *) &px;
		px = 0;
		nbytes = (image->bits_per_pixel + 7) >> 3;
		for (i = nbytes; --i >= 0;)
		    *dst++ = *src++;
		ZNORMALIZE(&px, image);
		_putbits((char *) &pixel,
			 (x * image->bits_per_pixel) & 7,
			 image->bits_per_pixel, (char *) &px);
		ZNORMALIZE(&px, image);
		src = (char *) &px;
		dst = &image->data[ZINDEX(x, y, image)];
		for (i = nbytes; --i >= 0;)
		    *dst++ = *src++;
	    }
    }
}

/*
 * write pixels into a 32-bits Z image data structure
 */

#if !defined(WORD64) && !defined(LONG64)
static unsigned long byteorderpixel = MSBFirst << 24;

#endif

static void
#if NeedFunctionPrototypes
SetImagePixels32(XImage *image, unsigned int width, unsigned int height,
		 unsigned char *xpmdata, Pixel *pixels)
#else
SetImagePixels32(image, width, height, xpmdata, pixels)
XImage *image;
unsigned int width;
unsigned int height;
unsigned char *xpmdata;
Pixel *pixels;
#endif
{
    register unsigned char *addr;
    register unsigned char *iptr;
    register int x, y;

    iptr = xpmdata;
#if !defined(WORD64) && !defined(LONG64)
    if (*((char *) &byteorderpixel) == image->byte_order) {
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++)
		*((Pixel *)(&image->data[ZINDEX32(x, y, image)]))
		    = pixels[*iptr];
    } else
#endif
    if (image->byte_order == MSBFirst)
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		addr = &((unsigned char *) image->data)[ZINDEX32(x, y, image)];
		addr[0] = pixels[*iptr] >> 24;
		addr[1] = pixels[*iptr] >> 16;
		addr[2] = pixels[*iptr] >> 8;
		addr[3] = pixels[*iptr];
	    }
    else
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		addr = &((unsigned char *) image->data)[ZINDEX32(x, y, image)];
		addr[3] = pixels[*iptr] >> 24;
		addr[2] = pixels[*iptr] >> 16;
		addr[1] = pixels[*iptr] >> 8;
		addr[0] = pixels[*iptr];
	    }
}

/*
 * write pixels into a 16-bits Z image data structure
 */

static void
#if NeedFunctionPrototypes
SetImagePixels16(XImage *image, unsigned int width, unsigned int height,
		 unsigned char *xpmdata, Pixel *pixels)
#else
SetImagePixels16(image, width, height, xpmdata, pixels)
XImage *image;
unsigned int width;
unsigned int height;
unsigned char *xpmdata;
Pixel *pixels;
#endif
{
    register unsigned char *addr;
    register unsigned char *iptr;
    register int x, y;

    iptr = xpmdata;
    if (image->byte_order == MSBFirst)
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		addr = &((unsigned char *) image->data)[ZINDEX16(x, y, image)];
		addr[0] = pixels[*iptr] >> 8;
		addr[1] = pixels[*iptr];
	    }
    else
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		addr = &((unsigned char *) image->data)[ZINDEX16(x, y, image)];
		addr[1] = pixels[*iptr] >> 8;
		addr[0] = pixels[*iptr];
	    }
}

/*
 * write pixels into a 8-bits Z image data structure
 */

static void
#if NeedFunctionPrototypes
SetImagePixels8(XImage *image, unsigned int width, unsigned int height,
		unsigned char *xpmdata, Pixel *pixels)
#else
SetImagePixels8(image, width, height, xpmdata, pixels)
XImage *image;
unsigned int width;
unsigned int height;
unsigned char *xpmdata;
Pixel *pixels;
#endif
{
    register unsigned char *iptr;
    register int x, y;

    iptr = xpmdata;
    for (y = 0; y < height; y++)
	for (x = 0; x < width; x++, iptr++)
	    image->data[ZINDEX8(x, y, image)] = pixels[*iptr];
}

/*
 * write pixels into a 1-bit depth image data structure and **offset null**
 */

static void
#if NeedFunctionPrototypes
SetImagePixels1(XImage *image, unsigned int width, unsigned int height,
		unsigned char *xpmdata, Pixel *pixels)
#else
SetImagePixels1(image, width, height, xpmdata, pixels)
XImage *image;
unsigned int width;
unsigned int height;
unsigned char *xpmdata;
Pixel *pixels;
#endif
{
    unsigned char bit;
    int xoff, yoff;
    register unsigned char *iptr;
    register int x, y;

    if (image->byte_order != image->bitmap_bit_order)
	SetImagePixels(image, width, height, xpmdata, pixels);
    else {
	iptr = xpmdata;
	if (image->bitmap_bit_order == MSBFirst)
	    for (y = 0; y < height; y++)
		for (x = 0; x < width; x++, iptr++) {
		    yoff = ZINDEX1(x, y, image);
		    xoff = x & 7;
		    bit = 0x80 >> xoff;
		    if (pixels[*iptr] & 1)
			image->data[yoff] |= bit;
		    else
			image->data[yoff] &= ~bit;
		}
	else
	    for (y = 0; y < height; y++)
		for (x = 0; x < width; x++, iptr++) {
		    yoff = ZINDEX1(x, y, image);
		    xoff = x & 7;
		    bit = 1 << xoff;
		    if (pixels[*iptr] & 1)
			image->data[yoff] |= bit;
		    else
			image->data[yoff] &= ~bit;
		}
    }
}

#endif /* OLDXPMCODE */

/*
 * Public Xmt routines that use the above private Xpm routines
 */

#if NeedFunctionPrototypes
Boolean XmtCreateXImageFromXmtImage(Widget widget, Visual *visual,
				    Colormap cmap, unsigned int depth,
				    XmtColorTable colors,
				    XmtImage *xmtimage,
				    XImage **image_return,
				    XImage **mask_return,
				    Pixel **allocated_pixels_return,
				    int *num_allocated_pixels_return)
#else
Boolean XmtCreateXImageFromXmtImage(widget, visual, cmap, depth, colors,
				    xmtimage, image_return, mask_return,
				    allocated_pixels_return,
				    num_allocated_pixels_return)
Widget widget;
Visual *visual;
Colormap cmap;
unsigned int depth;
XmtColorTable colors;
XmtImage *xmtimage;
XImage **image_return;
XImage **mask_return;
Pixel **allocated_pixels_return;
int *num_allocated_pixels_return;
#endif
{
    int status;
    
    if (image_return) *image_return = NULL;
    if (mask_return) *mask_return = NULL;

    status = CreateImage(widget, visual, cmap, depth, colors, xmtimage,
			 image_return, mask_return, allocated_pixels_return,
			 num_allocated_pixels_return);
    if (status == XpmSuccess)
	return True;
    else
	return False;
}

#if NeedFunctionPrototypes
Boolean XmtCreatePixmapFromXmtImage(Widget widget,Drawable win, Visual *visual,
				    Colormap cmap, unsigned int depth,
				    XmtColorTable colors,
				    XmtImage *xmtimage,
				    Pixmap *image_return,
				    Pixmap *mask_return,
				    Pixel **allocated_pixels_return,
				    int *num_allocated_pixels_return)
#else
Boolean XmtCreatePixmapFromXmtImage(widget, win, visual, cmap, depth, colors,
				    xmtimage, image_return, mask_return,
				    allocated_pixels_return,
				    num_allocated_pixels_return)
Widget widget;
Drawable win;
Visual *visual;
Colormap cmap;
unsigned int depth;
XmtColorTable colors;
XmtImage *xmtimage;
Pixmap *image_return;
Pixmap *mask_return;
Pixel **allocated_pixels_return;
int *num_allocated_pixels_return;
#endif
{
    Display *dpy = XtDisplayOfObject(widget);
    int status;
    XImage *image, **imageptr = NULL;
    XImage *mask, **maskptr = NULL;
    XGCValues gcv;
    GC gc = NULL;
    
    /*
     * initialize return values 
     */
    if (image_return) {
	*image_return = None;
	image = NULL;
	imageptr = &image;
    }
    if (mask_return) {
	*mask_return = None;
	mask = NULL;
	maskptr = &mask;
    }

    status = CreateImage(widget, visual, cmap, depth, colors, xmtimage,
			 imageptr, maskptr,
			 allocated_pixels_return,
			 num_allocated_pixels_return);
    if (status < 0) return False;

    gcv.function = GXcopy;

    /*
     * create the pixmaps 
     */
    if (imageptr && image) {
	*image_return = XCreatePixmap(dpy, win, image->width,
				      image->height, image->depth);
	gc = XCreateGC(dpy, *image_return, GCFunction, &gcv);
	XPutImage(dpy, *image_return, gc, image, 0, 0, 0, 0,
		  image->width, image->height);
	XDestroyImage(image);
	XFreeGC(dpy, gc);
    }
    if (maskptr && mask) {
	*mask_return = XCreatePixmap(dpy, win, mask->width,
				     mask->height, mask->depth);
	gc = XCreateGC(dpy, *mask_return, GCFunction, &gcv);
	XPutImage(dpy, *mask_return, gc, mask, 0, 0, 0, 0,
		  mask->width, mask->height);
	XDestroyImage(mask);
	XFreeGC(dpy, gc);
    }

    return True;
}

