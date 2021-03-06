/*
 * canon.c - Canon protocol "native" operations.
 *
 * Written 1999 by Wolfgang G. Reissnegger and Werner Almesberger
 * Additions 2000 by Philippe Marzouk and Edouard Lafargue
 * USB support, 2000, by Mikael Nystr�m
 *
 * This file includes both USB and serial support for the cameras
 * manufactured by Canon. These comprise all (or at least almost all)
 * of the digital models of the IXUS and PowerShot series, and EOS
 * D30, D60, and 10D. The EOS-1D and EOS-1Ds are not supported; they
 * use a FireWire (IEEE 1394) interface.
 *
 * We are working at moving serial and USB specific stuff to serial.c
 * and usb.c, keeping the common protocols/busses support in this
 * file.
 *
 * $Id: canon.c,v 1.3 2009-12-29 18:10:45 michelpujol Exp $
 */

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#ifdef TM_IN_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif

#ifdef OS2
#include <db.h>
#endif

#include <gphoto2.h>
#include <gphoto2-port-log.h>

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (GETTEXT_PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define _(String) (String)
#  define N_(String) (String)
#endif

#include "usb.h"
#include "util.h"
#include "library.h"
#include "canon.h"
#include "serial.h"

#ifdef HAVE_LIBEXIF
#  include <libexif/exif-data.h>
#  include <libexif/exif-utils.h>
#endif

/************************************************************************
 * Camera definitions
 ************************************************************************/

/**
 * models:
 *
 * Contains list of all camera models currently supported with their
 * respective USB IDs and a serial ID String for cameras with RS232
 * serial support.
 *
 * Some cameras are sold under different names in different regions,
 * but are technically the same. We treat them the same from a
 * technical point of view. To avoid unnecessary questions from users,
 * we add the other names to the camera list after the primary name,
 * such that their camera name occurs in the list of supported
 * cameras.
 *
 * Notes:
 * - At least some serial cameras require a certain name for correct
 *   detection.
 * - Newer Canon USB cameras also support a PTP mode. See ptp2 camlib.
 * - No IEEE1394 cameras supported yet.
 * - The size limit constants aren't used properly anywhere. We should
 *   probably get rid of them altogether.
 **/

/* SL_* - size limit constants */
#define KILOBYTE	(1024U)
#define MEGABYTE	(1024U * KILOBYTE)
#define SL_THUMB	( 100U * KILOBYTE)
#define SL_THUMB_CR2    (1024U * KILOBYTE)
#define SL_PICTURE	(  10U * MEGABYTE)
#define SL_MOVIE_SMALL	( 100U * MEGABYTE)
#define SL_MOVIE_LARGE	(2048U * MEGABYTE)
#define NO_USB  0


/* Models with unknown USB ID's:
  European name          North American                 Japanese             Intro date

  PowerShot A520                                                             January 2005
  Digital IXUS 40        PowerShot SD300                IXY Digital 50       September 2004
  PowerShot Pro1                                                             February 2004
                         PowerShot SD30                 IXY i zoom           August 2005
                         PowerShot A410                                      August 2005
                         PowerShot A610                                      August 2005
                         PowerShot A620                                      August 2005
  Digital IXUS 55        PowerShot SD450                                     August 2005
  Digital IXUS 750       PowerShot SD550                                     August 2005
                         PowerShot S80                                       August 2005
  Digital IXUS Wireless  PowerShot SD430                                     August 2005
  */
const struct canonCamModelData models[] = {
	/* *INDENT-OFF* */
	{"Canon:PowerShot A5",		CANON_CLASS_3,	NO_USB, NO_USB, CAP_NON, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, "DE300 Canon Inc."},
	{"Canon:PowerShot A5 Zoom",	CANON_CLASS_3,	NO_USB, NO_USB, CAP_NON, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, "Canon PowerShot A5 Zoom"},
	{"Canon:PowerShot A50",		CANON_CLASS_1,	NO_USB, NO_USB, CAP_NON, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, "Canon PowerShot A50"},
	{"Canon:PowerShot Pro70",	CANON_CLASS_2,	NO_USB, NO_USB, CAP_NON, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, "Canon PowerShot Pro70"},
	{"Canon:PowerShot S10",		CANON_CLASS_0,	0x04A9, 0x3041, CAP_NON, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, "Canon PowerShot S10"},
        /* 3042 is a scanner, so it will never be added here. */
        {"Canon:PowerShot S20",         CANON_CLASS_0,  0x04A9, 0x3043, CAP_NON, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, "Canon PowerShot S20"},
        {"Canon:EOS D30",               CANON_CLASS_4,  0x04A9, 0x3044, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot S100",        CANON_CLASS_0,  0x04A9, 0x3045, CAP_NON, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:IXY DIGITAL",           CANON_CLASS_0,  0x04A9, 0x3046, CAP_NON, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:Digital IXUS",          CANON_CLASS_0,  0x04A9, 0x3047, CAP_NON, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot G1",          CANON_CLASS_0,  0x04A9, 0x3048, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, "Canon PowerShot G1"},
        {"Canon:PowerShot Pro90 IS",    CANON_CLASS_0,  0x04A9, 0x3049, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, "Canon PowerShot Pro90 IS"},

        {"Canon:IXY DIGITAL 300",       CANON_CLASS_1,  0x04A9, 0x304B, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot S300",        CANON_CLASS_1,  0x04A9, 0x304C, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:Digital IXUS 300",      CANON_CLASS_1,  0x04A9, 0x304D, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot A20",         CANON_CLASS_1,  0x04A9, 0x304E, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot A10",         CANON_CLASS_1,  0x04A9, 0x304F, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        /* Mac OS includes this as a valid ID; don't know which camera model --swestin */
        {"Canon:PowerShot unknown 1",   CANON_CLASS_1,  0x04A9, 0x3050, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        /* Canon IXY DIGITAL 200 here? */
        {"Canon:PowerShot S110",        CANON_CLASS_0,  0x04A9, 0x3051, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:Digital IXUS v",        CANON_CLASS_0,  0x04A9, 0x3052, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},

        {"Canon:PowerShot G2",          CANON_CLASS_1,  0x04A9, 0x3055, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot S40",         CANON_CLASS_1,  0x04A9, 0x3056, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot S30",         CANON_CLASS_1,  0x04A9, 0x3057, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot A40",         CANON_CLASS_1,  0x04A9, 0x3058, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot A30",         CANON_CLASS_1,  0x04A9, 0x3059, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        /* 305a is the ZR50 Digital Camcorder in USB Mass Storage mode. */
        /* 305b is the ZR45MC Digital Camcorder in USB Mass Storage mode. */
        /* 305c is in MacOS Info.plist, but I don't know what it is --swestin. */
        {"Canon:PowerShot unknown 2",   CANON_CLASS_1,  0x04A9, 0x305c, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},

        {"Canon:EOS D60",               CANON_CLASS_4,  0x04A9, 0x3060, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot A100",        CANON_CLASS_1,  0x04A9, 0x3061, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot A200",        CANON_CLASS_1,  0x04A9, 0x3062, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},

        {"Canon:PowerShot S200",        CANON_CLASS_1,  0x04A9, 0x3065, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:Digital IXUS v2",       CANON_CLASS_1,  0x04A9, 0x3065, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot S330",        CANON_CLASS_1,  0x04A9, 0x3066, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:Digital IXUS 330",      CANON_CLASS_1,  0x04A9, 0x3066, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        /* 3067  MV550i Digital Video Camera */

        /* Reported at http://www.linux-usb.org/usb.ids, we have 306E. */
        /*{"Canon:PowerShot G3",        CANON_CLASS_1,  0x04A9, 0x3069, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},*/
        /* 306a is in MacOS Info.plist, but I don't know what it is --swestin. */
        {"Canon:Digital unknown 3",     CANON_CLASS_1,  0x04A9, 0x306a, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:Optura 200 MC",         CANON_CLASS_1,  0x04A9, 0x306B, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:MVX2i",                 CANON_CLASS_1,  0x04A9, 0x306B, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:IXY DV M",              CANON_CLASS_1,  0x04A9, 0x306B, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot S45 (normal mode)",   CANON_CLASS_5,  0x04A9, 0x306C, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        /* 0x306D is S45 in PTP mode */
        {"Canon:PowerShot G3 (normal mode)",    CANON_CLASS_5,  0x04A9, 0x306E, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        /* 0x306F is G3 in PTP mode */
        {"Canon:PowerShot S230 (normal mode)",  CANON_CLASS_4,  0x04A9, 0x3070, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:Digital IXUS v3 (normal mode)", CANON_CLASS_4,  0x04A9, 0x3070, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        /* 0x3071 is S230/IXUS v3 in PTP mode */
        /* Following cameras share the ID for PTP and Canon modes */
        {"Canon:PowerShot SD100 (normal mode)", CANON_CLASS_5,  0x04A9, 0x3072, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:Digital IXUS II (normal mode)", CANON_CLASS_5,  0x04A9, 0x3072, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot A70",         CANON_CLASS_1,  0x04A9, 0x3073, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot A60",         CANON_CLASS_1,  0x04A9, 0x3074, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:Digital IXUS 400",      CANON_CLASS_1,  0x04A9, 0x3075, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot S400",        CANON_CLASS_1,  0x04A9, 0x3075, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        /* End of shared ID's */
        {"Canon:PowerShot A300",        CANON_CLASS_1,  0x04A9, 0x3076, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        /* S50 also shares ID for PTP and Canon modes */
        {"Canon:PowerShot S50 (normal mode)",   CANON_CLASS_4,  0x04A9, 0x3077, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:ZR70MC (normal mode)",      CANON_CLASS_1,  0x04A9, 0x3078, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        /* 3079 is listed for MV650i, probably in PTP mode. */
        {"Canon:MV650i (normal mode)",  CANON_CLASS_1,  0x04A9, 0x307a, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        /* 307b is listed for MV630i, probably in PTP mode. */
        {"Canon:MV630i (normal mode)",  CANON_CLASS_1,  0x04A9, 0x307c, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},

        /* 307f is Optura 20/MVX150i in PTP mode */
        {"Canon:Optura 20 (normal mode)", CANON_CLASS_1,  0x04A9, 0x3080, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:MVX150i (normal mode)", CANON_CLASS_1,  0x04A9, 0x3080, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        /* Sighted at
         * <http://www.qbik.ch/usb/devices/showdescr.php?id=2232>. */
        {"Canon:Optura 10",             CANON_CLASS_1,  0x04A9, 0x3081, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:MVX100i",               CANON_CLASS_1,  0x04A9, 0x3081, CAP_NON, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},


        {"Canon:Optura 10",             CANON_CLASS_1,  0x04A9, 0x3082, CAP_NON, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:MVX100i",               CANON_CLASS_1,  0x04A9, 0x3082, CAP_NON, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:EOS 10D",               CANON_CLASS_4,  0x04A9, 0x3083, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:EOS 300D (normal mode)", CANON_CLASS_4, 0x04A9, 0x3084, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:EOS Digital Rebel (normal mode)",CANON_CLASS_4, 0x04A9, 0x3084, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:EOS Kiss Digital (normal mode)",CANON_CLASS_4,  0x04A9, 0x3084, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        /* PS G5 uses the same ProductID for PTP and Canon, with protocol autodetection */
        {"Canon:PowerShot G5 (normal mode)", CANON_CLASS_5,     0x04A9, 0x3085, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},

        /* Elura 50 camcorder is 0x3087 in PTP mode; 3088 in Canon mode? */
        {"Canon:Elura 50 (normal mode)",  CANON_CLASS_1,  0x04A9, 0x3088, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},

        /* Optura Xi/MVX 3i/FV M1 uses 308d in PTP mode; 308e in Canon mode? */
        {"Canon:Optura Xi (normal mode)",  CANON_CLASS_1,  0x04A9, 0x308e, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:MVX 3i (normal mode)",  CANON_CLASS_1,  0x04A9, 0x308e, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:FV M1 (normal mode)",  CANON_CLASS_1,  0x04A9, 0x308e, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},

        /* Optura 300/MVX 10i/IXY DV M2 video camera uses 3093 in USB Mass Storage mode. */

        /* Optura 300/MVX 10i/IXY DV M2 video camera uses 3095 in PTP mode; 3096 in Canon mode? */
        {"Canon:Optura 300 (normal mode)",  CANON_CLASS_1,  0x04A9, 0x3096, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:MVX 10i (normal mode)",  CANON_CLASS_1,  0x04A9, 0x3096, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:IXY DV M2 (normal mode)",  CANON_CLASS_1,  0x04A9, 0x3096, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},

        /* 0x3099 is the EOS 300D/Digital Rebel in PTP mode */
        /* A80 seems to share the ID for PTP and Canon modes */
        {"Canon:PowerShot A80 (normal mode)",   CANON_CLASS_1,  0x04A9, 0x309A, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        /* 0x309b is the SD10 Digital ELPH/Digital IXUS i/IXY Digital L
           in PTP mode; will it work in Canon mode? */
        {"Canon:PowerShot SD10 Digital ELPH (normal mode)",   CANON_CLASS_1,  0x04A9, 0x309B, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:Digital IXUS i (normal mode)",  CANON_CLASS_1,  0x04A9, 0x309B, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot IXY Digital L (normal mode)", CANON_CLASS_1,  0x04A9, 0x309B, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},

#if 0
/* reportedly not working ... comment out for now - Marcus */
        /* Product ID shared between PTP and Canon mode */
        {"Canon:PowerShot S1 IS (normal mode)", CANON_CLASS_5,  0x04A9, 0x309C, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
#endif

        /* 30a0 is ZR90/MV750i camcorder */

        /* 30a8 is Elura 60E/MVX200i camcorder in PTP mode; don't know
         * if it will respond in native mode at the same product
         * ID. */
        /* 30a9 is Optura 40/MVX25i camcorder, reported working at
         * <http://www.qbik.ch/usb/devices/showdescr.php?id=2700>. Seems
         * to share ID with PTP. */
        {"Canon:Optura 40 (normal mode)", CANON_CLASS_1,  0x04A9, 0x30A9, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:MVX25i (normal mode)", CANON_CLASS_1,  0x04A9, 0x30A9, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},

        /* Another block of cameras that share the ID for PTP and Canon modes */
        {"Canon:PowerShot S70 (normal mode)",   CANON_CLASS_5,  0x04A9, 0x30b1, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot S60 (normal mode)",   CANON_CLASS_5,  0x04A9, 0x30b2, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot G6 (normal mode)",    CANON_CLASS_5,  0x04A9, 0x30b3, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:Digital IXUS 500 (normal mode)",CANON_CLASS_5,  0x04A9, 0x30b4, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot S500 Digital ELPH (normal mode)",CANON_CLASS_5,       0x04A9, 0x30b4, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:IXY Digital 500 (normal mode)", CANON_CLASS_5,  0x04A9, 0x30b4, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot A75",                 CANON_CLASS_1,  0x04A9, 0x30b5, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot SD110 Digital ELPH",  CANON_CLASS_1,  0x04A9, 0x30b6, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:Digital IXUS IIs",              CANON_CLASS_1,  0x04A9, 0x30b6, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot A400",                CANON_CLASS_5,  0x04A9, 0x30b7, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot A310",                CANON_CLASS_5,  0x04A9, 0x30b8, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot A85 (normal mode)",   CANON_CLASS_5,  0x04A9, 0x30b9, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot S410 Digital ELPH (normal mode)", CANON_CLASS_5,      0x04A9, 0x30ba, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:Digital IXUS 430 (normal mode)",CANON_CLASS_5,  0x04A9, 0x30ba, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:IXY Digital 430 (normal mode)", CANON_CLASS_5,  0x04A9, 0x30ba, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot A95 (normal mode)",   CANON_CLASS_5,  0x04A9, 0x30bb, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        /* End of shared ID's */

        /* 0x30bf is PowerShot SD300/Digital IXUS 40 in PTP mode */
        /* Another block of cameras that share the ID for PTP and Canon modes */
#ifdef CANON_EXPERIMENTAL_20D
        {"Canon:PowerShot SD200 (normal mode)", CANON_CLASS_6,  0x04A9, 0x30c0, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:Digital IXUS 30 (normal mode)", CANON_CLASS_6,  0x04A9, 0x30c0, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:IXY Digital 40 (normal mode)",  CANON_CLASS_6,  0x04A9, 0x30c0, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
#endif /* CANON_EXPERIMENTAL_20D */
        {"Canon:PowerShot SD400 (normal mode)", CANON_CLASS_4,  0x04A9, 0x30c1, CAP_NON, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:Digital IXUS 50 (normal mode)", CANON_CLASS_4,  0x04A9, 0x30c1, CAP_NON, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:IXY Digital 55 (normal mode)",  CANON_CLASS_4,  0x04A9, 0x30c1, CAP_NON, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:PowerShot A510 (normal mode)",  CANON_CLASS_1,  0x04A9, 0x30c2, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        /* End of shared ID's */

        {"Canon:PowerShot SD20 (normal mode)",  CANON_CLASS_5,  0x04A9, 0x30c4, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:Digital IXUS i5 (normal mode)", CANON_CLASS_5,  0x04A9, 0x30c4, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:IXY Digital L2 (normal mode)",  CANON_CLASS_5,  0x04A9, 0x30c4, CAP_SUP, SL_MOVIE_SMALL, SL_THUMB, SL_PICTURE, NULL},

        /* Is 0x30e9 EOS 1D Mark II in Canon mode? */
        /* 0x30ea is EOS 1D Mark II in PTP mode */

#ifdef CANON_EXPERIMENTAL_20D
        {"Canon:EOS 20D (normal mode)",         CANON_CLASS_6,  0x04A9, 0x30eb, CAP_EXP, SL_MOVIE_LARGE, SL_THUMB_CR2, SL_PICTURE, NULL},
        /* 0x30ec is EOS 20D in PTP mode */

        {"Canon:EOS 350D (normal mode)",                CANON_CLASS_6,  0x04A9, 0x30ee, CAP_EXP, SL_MOVIE_LARGE, SL_THUMB_CR2, SL_PICTURE, NULL},
        {"Canon:Digital Rebel XT (normal mode)",                CANON_CLASS_6,  0x04A9, 0x30ee, CAP_EXP, SL_MOVIE_LARGE, SL_THUMB_CR2, SL_PICTURE, NULL},
        {"Canon:EOS Kiss Digital N (normal mode)",              CANON_CLASS_6,  0x04A9, 0x30ee, CAP_EXP, SL_MOVIE_LARGE, SL_THUMB_CR2, SL_PICTURE, NULL},
        /* 30ef is EOS 350D/Digital Rebel XT/EOS Kiss Digital N in PTP mode. */
        {"Canon:EOS 5D (normal mode)",          CANON_CLASS_6,  0x04A9, 0x3101, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
#endif /* CANON_EXPERIMENTAL_20D */

#if 0
/* reportedly not working ... comment out for now - Marcus */
        {"Canon:PowerShot S2 IS (normal mode)",  CANON_CLASS_1,  0x04A9, 0x30f0, CAP_EXP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
#endif

#if 0
/* reportedly not working either. failing with lock keys - Marcus */
        {"Canon:PowerShot SD500 (normal mode)",  CANON_CLASS_5,  0x04A9, 0x30f2, CAP_NON, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:Digital IXUS 700 (normal mode)",  CANON_CLASS_5,  0x04A9, 0x30f2, CAP_NON, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
        {"Canon:IXY Digital 600 (normal mode)",  CANON_CLASS_5,  0x04A9, 0x30f2, CAP_NON, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
#endif

	/* I was not able to make A620 work with CLASS 5, 1 and 6. -Marcus */

#if 0
	/* also reported as not working. */
        {"Canon:PowerShot A610 (normal mode)",   CANON_CLASS_5,  0x04A9, 0x30fd, CAP_SUP, SL_MOVIE_LARGE, SL_THUMB, SL_PICTURE, NULL},
#endif
        {NULL}
        /* *INDENT-ON* */
};

struct canonValueStruct
{
      unsigned int value;
      char *label;
      float fvalue;
};

const struct canonValueStruct canonShutterValues[] =
{
      { 0x0010, "30"     , (float)30.0  },
      { 0x0013, "25"     , (float)25.0  },
      { 0x0015, "20"     , (float)20.0  },
      { 0x0018, "15"     , (float)15.0  },
      { 0x001b, "13"     , (float)13.0  },
      { 0x001d, "10"     , (float)10.0  },
      { 0x0020, "8"      , (float)8.0   },
      { 0x0023, "6"      , (float)6.0   },
      { 0x0025, "5"      , (float)5.0   },
      { 0x0028, "4"      , (float)4.0   },
      { 0x002b, "3.2"    , (float)3.2   },
      { 0x002d, "2.5"    , (float)2.5   },
      { 0x0030, "2"      , (float)2.0   },
      { 0x0033, "1.6"    , (float)1.6   },
      { 0x0035, "1.3"    , (float)1.3   },
      { 0x0038, "1"      , (float)1.0   },
      { 0x003b, "0.8"    , (float)0.8   },
      { 0x003d, "0.6"    , (float)0.6   },
      { 0x0040, "0.5"    , (float)0.5   },
      { 0x0043, "0.4"    , (float)0.4   },
      { 0x0045, "0.3"    , (float)0.3   },
      { 0x0048, "1/4"    , (float)0.25000   },
      { 0x004b, "1/5"    , (float)0.20000   },
      { 0x004d, "1/6"    , (float)0.16667   },
      { 0x0050, "1/8"    , (float)0.12500   },
      { 0x0053, "1/10"   , (float)0.10000   },
      { 0x0055, "1/13"   , (float)0.07692   },
      { 0x0058, "1/15"   , (float)0.06667   },
      { 0x005b, "1/20"   , (float)0.05000   },
      { 0x005d, "1/25"   , (float)0.04000   },
      { 0x0060, "1/30"   , (float)0.03333   },
      { 0x0063, "1/40"   , (float)0.02500   },
      { 0x0065, "1/50"   , (float)0.02000   },
      { 0x0068, "1/60"   , (float)0.01667   },
      { 0x006b, "1/80"   , (float)0.01250   },
      { 0x006d, "1/100"  , (float)0.01000   },
      { 0x0070, "1/125"  , (float)0.00800   },
      { 0x0073, "1/160"  , (float)0.00625   },
      { 0x0075, "1/200"  , (float)0.00500   },
      { 0x0078, "1/250"  , (float)0.00400   },
      { 0x007b, "1/320"  , (float)0.003125  },
      { 0x007d, "1/400"  , (float)0.00250   },
      { 0x0080, "1/500"  , (float)0.00200   },
      { 0x0083, "1/640"  , (float)0.0015625 },
      { 0x0085, "1/800"  , (float)0.00125   },
      { 0x0088, "1/1000" , (float)0.00100   },
      { 0x008b, "1/1250" , (float)0.00080   },
      { 0x008d, "1/1600" , (float)0.000625  },
      { 0x0090, "1/2000" , (float)0.00050   },
      { 0x0093, "1/2500" , (float)0.00040   },
      { 0x0095, "1/3200" , (float)0.0003125 },
      { 0x0098, "1/4000" , (float)0.00025   },
      { 0x009b, "1/5000" , (float)0.00020   },
      { 0x009d, "1/6400" , (float)0.00015625},
      { 0x00a0, "1/8000" , (float)0.000125  },
      { 0xffff, "auto"   , (float)0.000 },
      { 0x0004, "bulb"   , (float)-1.0  },
      { 0, NULL , (float)0.0 },
};

const struct canonValueStruct canonDriveMode[] =
{
      { 0xff00, "single"     , (float)0.0  },
      { 0xff01, "continuous" , (float)1.0  },
      { 0xff02, "timer"      , (float)2.0  },
      { 0, NULL , (float)0 }
};

/*
const struct canonValueStruct canonQuality[]= {
      { 0x00020420, "CRW" ,           (float)0.0  },
      { 0x00010320, "Large:Fine" ,    (float)0.0  },
      { 0x00010220, "Large:Normal" ,  (float)0.0  },
      { 0x01010320, "Middle:Fine" ,   (float)0.0  },
      { 0x01010220, "Middle:Normal" , (float)0.0  },
      { 0x02010320, "Small:Fine" ,    (float)0.0  },
      { 0x02010220, "Small:Normal" ,  (float)0.0  },
      { 0, NULL, (float)0.0 }
};
*/

const struct canonValueStruct canonValueQuality[] =
{
      { 0x00020420, "RAW"        , (float)0.0  },
      { 0x00010320, "Large:Fine" , (float)0.0  },
      { 0x00010220, "Large:Normal" , (float)0.0  },
      { 0x01010320, "Middle:Fine" , (float)0.0  },
      { 0x01010220, "Middle:Normal" , (float)0.0  },
      { 0x02010320, "Small:Fine" , (float)0.0  },
      { 0x02010220, "Small:Normal" , (float)0.0  },
      { 0, NULL , (float)0 }
};


#undef NO_USB
#undef SL_MOVIE_LARGE
#undef SL_MOVIE_SMALL
#undef SL_PICTURE
#undef S1M
#undef SL_THUMB

#ifndef HAVE_TM_GMTOFF
/* required for time conversions in canon_int_set_time() */
//modif michel 
#ifdef WIN32
//#define timezone _timezone
#else
extern long int timezone;
#endif
#endif


/************************************************************************
 * Methods
 ************************************************************************/

/* Simulate capabilities
 *
 * If you need to change these for a certain camera, it is time to
 * introduce and use general capabilities as replacement.
 */

#define extra_file_for_thumb_of_jpeg FALSE
#define extra_file_for_thumb_of_crw TRUE
#define extra_file_for_thumb_of_cr2 FALSE

static const char *
replace_filename_extension(const char *filename, const char *newext)
{
        char *p;
        static char buf[1024];

        /* We just replace file ending by .THM and assume this is the
         * name of the thumbnail file.
         */
        if (strncpy (buf, filename, sizeof (buf)) < 0) {
                GP_DEBUG ("replace_filename_extension: Buffer too small in %s line %i.",
                          __FILE__, __LINE__);
                return NULL;
        }
        if ((p = strrchr (buf, '.')) == NULL) {
                GP_DEBUG ("replace_filename_extension: No '.' found in filename '%s' "
                          "in %s line %i.", filename, __FILE__, __LINE__);
                return NULL;
        }
        if (((p - buf) < sizeof (buf) - 4) && strncpy (p, ".THM", 4)) {
                GP_DEBUG ("replace_filename_extension: New name for '%s' is '%s'",
                          filename, buf);
                return buf;
        } else {
                GP_DEBUG ("replace_filename_extension: "
                          "New name for filename '%s' doesnt fit in %s line %i.",
                          filename, __FILE__, __LINE__);
                return NULL;
        }
}

static char *
filename_to_audio(const char *filename, const char *newext)
{
        char *p;
        static char buf[1024];

        /* We just replace file ending by .WAV, the first three
         * letters by SND and assume this is the name of the audio file.
         */
        if (strncpy (buf, filename, sizeof (buf)) < 0) {
                GP_DEBUG ("filename_to_audio: Buffer too small in %s line %i.",
                          __FILE__, __LINE__);
                return NULL;
        }
        if ((p = strrchr (buf, '_')) == NULL) {
                GP_DEBUG ("filename_to_audio: No '.' found in filename '%s' "
                          "in %s line %i.", filename, __FILE__, __LINE__);
                return NULL;
        }
        if ((p - buf) > 3) {
                p -= 3;
                p[0] = 'S';
                p[1] = 'N';
                p[2] = 'D';
        }
        if ((p = strrchr (buf, '.')) == NULL) {
                GP_DEBUG ("filename_to_audio: No '.' found in filename '%s' "
                          "in %s line %i.", filename, __FILE__, __LINE__);
                return NULL;
        }
        if (((p - buf) < sizeof (buf) - 4) && strncpy (p, ".WAV", 4)) {
                GP_DEBUG ("filename_to_audio: New name for '%s' is '%s'",
                          filename, buf);
                return buf;
        } else {
                GP_DEBUG ("filename_to_audio: "
                          "New name for filename '%s' doesnt fit in %s line %i.",
                          filename, __FILE__, __LINE__);
                return NULL;
        }
}

/**
 * canon_int_filename2audioname:
 * @camera: Camera to work on
 * @filename: the file for which to get the name of the audio annotation
 *
 * The identifier returned is 
 *  a) NULL if no audio file exists for this file or an internal error occured
 *  b) pointer to string with file name of the corresponding thumbnail
 *  c) pointer to filename in case filename is a thumbnail itself
 *
 * Returns: identifier for corresponding thumbnail
 */

const char *
canon_int_filename2audioname (Camera *camera, const char *filename)
{
        char *result;

        /* We use the audio file itself as the audio file. In short:
         * audiofile = audiofile(audiofile)
         */
        if (is_audio (filename)) {
                GP_DEBUG ("canon_int_filename2audioname: \"%s\" IS an audio file",
                          filename);
                return filename;
        }

        /* There are only audio files for images and movies */
        if (!(is_movie (filename) || is_image (filename))) {
                GP_DEBUG ("canon_int_filename2audioname: "
                          "\"%s\" is neither movie nor image -> no audio file", filename);
                return NULL;
        }

        result = filename_to_audio (filename, ".WAV");

        GP_DEBUG ("canon_int_filename2audioname: audio for file \"%s\" is external: \"%s\"",
                  filename, result);

        return result;
}

/**
 * canon_int_filename2thumbname:
 * @camera: Camera to work on
 * @filename: the file to get the name of the thumbnail of
 *
 * The identifier returned is 
 *  a) NULL if no thumbnail exists for this file or an internal error occured
 *  b) pointer to empty string if thumbnail is contained in the file itself
 *  c) pointer to string with file name of the corresponding thumbnail
 *  d) pointer to filename in case filename is a thumbnail itself
 *
 * Returns: identifier for corresponding thumbnail
 */

const char *
canon_int_filename2thumbname (Camera *camera, const char *filename)
{
        static char *nullstring = "";

        /* First handle cases where we shouldn't try to get extra .THM
         * file but use the special get_thumbnail_of_xxx function.
         */
        if (!extra_file_for_thumb_of_jpeg && is_jpeg (filename)) {
                GP_DEBUG ("canon_int_filename2thumbname: thumbnail for JPEG \"%s\" is internal",
                          filename);
                return nullstring;
        }
        if (!extra_file_for_thumb_of_crw && is_crw (filename)) {
                GP_DEBUG ("canon_int_filename2thumbname: thumbnail for CRW \"%s\" is internal",
                          filename);
                return nullstring;
        }
        if (!extra_file_for_thumb_of_cr2 && is_cr2 (filename)) {
                GP_DEBUG ("canon_int_filename2thumbname: thumbnail for CR2 \"%s\" is internal",
                          filename);
                return nullstring;
        }

        /* We use the thumbnail file itself as the thumbnail of the
         * thumbnail file. In short thumbfile = thumbnail(thumbfile)
         */
        if (is_thumbnail (filename)) {
                GP_DEBUG ("canon_int_filename2thumbname: \"%s\" IS a thumbnail file",
                          filename);
                return filename;
        }

        /* There are only thumbnails for images and movies */
        if (!is_movie (filename) && !is_image (filename)) {
                GP_DEBUG ("canon_int_filename2thumbname: "
                          "\"%s\" is neither movie nor image -> no thumbnail", filename);
                return NULL;
        }

        GP_DEBUG ("canon_int_filename2thumbname: thumbnail for file \"%s\" is external",
                  filename);

        /* We just replace file ending by .THM and assume this is the
         * name of the thumbnail file.
         */
        return replace_filename_extension (filename, ".THM");

        /* never reached */
        return NULL;
}

/**
 * canon_int_directory_operations:
 * @camera: Camera to work on
 * @path: Path to directory on which to operate
 * @action: #canonDirFunctionCode (either #DIR_CREATE or #DIR_REMOVE)
 * @context: context for error reporting
 *
 * Creates or removes a directory in camera storage.
 *
 *
 * Returns: gphoto2 status code. %GP_OK on success, otherwise status
 * from canon_serial_error(), %GP_ERROR if USB operation fails,
 * %GP_ERROR_CORRUPTED_DATA if the camera response is not the expected
 * length, %GP_ERROR_BAD_PARAMETERS if @action is invalid.
 *
 */
int
canon_int_directory_operations (Camera *camera, const char *path, canonDirFunctionCode action,
                                GPContext *context)
{
        unsigned char *msg;
        int len, canon_usb_funct;
        char type;

        switch (action) {
                case DIR_CREATE:
                        type = 0x5;
                        canon_usb_funct = CANON_USB_FUNCTION_MKDIR;
                        break;
                case DIR_REMOVE:
                        type = 0x6;
                        canon_usb_funct = CANON_USB_FUNCTION_RMDIR;
                        break;
                default:
                        GP_DEBUG ("canon_int_directory_operations: "
                                  "Bad operation specified : %i", action);
                        return GP_ERROR_BAD_PARAMETERS;
                        break;
        }

        GP_DEBUG ("canon_int_directory_operations() called to %s the directory '%s'",
                  canon_usb_funct == CANON_USB_FUNCTION_MKDIR ? "create" : "remove", path);
        switch (camera->port->type) {
                case GP_PORT_USB:
                        msg = canon_usb_dialogue (camera, canon_usb_funct, &len, path,
                                                  strlen (path) + 1);
                        if ( msg == NULL )
                                return GP_ERROR_OS_FAILURE;
                        break;
                case GP_PORT_SERIAL:
                        msg = canon_serial_dialogue (camera, context, type, 0x11, &len, path,
                                                     strlen (path) + 1, NULL);
                        if ( msg == NULL ) {
                                canon_serial_error_type (camera);
                                return GP_ERROR_OS_FAILURE;
                        }

                        break;
                GP_PORT_DEFAULT
        }

        if (len != 0x4) {
                GP_DEBUG ("canon_int_directory_operations: Unexpected amount "
                          "of data returned (expected %i got %i)", 0x4, len);
                return GP_ERROR_CORRUPTED_DATA;
        }

        if (msg[0] != 0x00) {
                if ( action == DIR_CREATE )
                        gp_context_error (context, _("Could not create directory %s."),
                                          path );
                else
                        gp_context_error (context, _("Could not remove directory %s."),
                                          path);
                return GP_ERROR_CAMERA_ERROR;
        }

        return GP_OK;
}

/**
 * canon_int_identify_camera:
 * @camera: the camera to work with
 * @context: context for error reporting
 *
 * Gets the camera identification string, usually the owner name.
 *
 * This information is then stored in the "camera" structure, which 
 * is a global variable for the driver.
 *
 * This function also gets the firmware revision in the camera struct.
 *
 * Returns: gphoto2 error code
 *
 */
int
canon_int_identify_camera (Camera *camera, GPContext *context)
{
        unsigned char *msg;
        int len;

        GP_DEBUG ("canon_int_identify_camera() called");

        switch (camera->port->type) {
                case GP_PORT_USB:
                        msg = canon_usb_dialogue (camera, CANON_USB_FUNCTION_IDENTIFY_CAMERA,
                                                  &len, NULL, 0);
                        if ( msg == NULL )
                                return GP_ERROR_OS_FAILURE;
                        break;
                case GP_PORT_SERIAL:
                        msg = canon_serial_dialogue (camera, context, 0x01, 0x12, &len, NULL);
                        if ( msg == NULL ) {
                                GP_DEBUG ("canon_int_identify_camera: msg error");
                                canon_serial_error_type (camera);
                                return GP_ERROR_OS_FAILURE;
                        }
                        break;
                GP_PORT_DEFAULT
        }

        if (len != 0x4c) {
                GP_DEBUG ("canon_int_identify_camera: Unexpected length returned "
                          "(expected %i got %i); continuing.", 0x4c, len);
        }

        /* Store these values in our "camera" structure: */
        memcpy (camera->pl->firmwrev, (char *) msg + 8, 4);
        strncpy (camera->pl->ident, (char *) msg + 12, 32);
        if ( camera->pl->md->model != CANON_CLASS_6 )
                strncpy (camera->pl->owner, (char *) msg + 44, 32);
        else {
                /* Need to get owner explicitly. */
                msg = canon_usb_dialogue ( camera, CANON_USB_FUNCTION_GET_OWNER, &len, NULL, 0 );
                if ( msg == NULL )
                        return GP_ERROR_OS_FAILURE;
                strncpy (camera->pl->owner, (char *) msg + 4, 32);
        }


        GP_DEBUG ("canon_int_identify_camera: ident '%s' owner '%s', firmware %d.%d.%d.%d",
                  camera->pl->ident, camera->pl->owner,
                  camera->pl->firmwrev[3], camera->pl->firmwrev[2],
                  camera->pl->firmwrev[1], camera->pl->firmwrev[0] );

// modif michel
        //init_quality_list();

        return GP_OK;
}

/**
 * canon_int_get_battery:
 * @camera: the camera to work on
 * @pwr_status: pointer to integer determining power status
 * @pwr_source: pointer to integer determining power source
 * @context: context for error reporting
 *
 * Gets battery status.
 *
 * Returns: gphoto2 error code
 *
 */
int
canon_int_get_battery (Camera *camera, int *pwr_status, int *pwr_source, GPContext *context)
{
        unsigned char *msg;
        int len;

        GP_DEBUG ("canon_int_get_battery()");

        switch (camera->port->type) {
                case GP_PORT_USB:
                        if ( camera->pl->md->model == CANON_CLASS_6 )
                                /* Newer protocol uses a different code, but with same response. */
                                msg = canon_usb_dialogue (camera, CANON_USB_FUNCTION_POWER_STATUS_2,
                                                          &len, NULL, 0);
                        else
                                msg = canon_usb_dialogue (camera, CANON_USB_FUNCTION_POWER_STATUS,
                                                          &len, NULL, 0);
                        if ( msg == NULL )
                                return GP_ERROR_OS_FAILURE;
                        break;
                case GP_PORT_SERIAL:
                        msg = canon_serial_dialogue (camera, context, 0x0a, 0x12, &len, NULL);
                        if ( msg == NULL ) {
                                canon_serial_error_type (camera);
                                return GP_ERROR_OS_FAILURE;
                        }

                        break;
                GP_PORT_DEFAULT
        }

        if (len != 0x8) {
                GP_DEBUG ("canon_int_get_battery: Unexpected length returned "
                          "(expected %i got %i)", 0x8, len);
                return GP_ERROR_CORRUPTED_DATA;
        }

        if (pwr_status)
                *pwr_status = msg[4];
        if (pwr_source)
                *pwr_source = msg[7];

        GP_DEBUG ("canon_int_get_battery: Status: %02x (%s) / Source: %02x (%s)\n",
                  msg[4], (msg[4]==CAMERA_POWER_OK?"OK":"BAD"),
                  msg[7], (msg[7]&CAMERA_MASK_BATTERY?"BATTERY":"AC") );

        return GP_OK;
}

/**
 * canon_int_get_picture_abilities:
 * @camera: the camera to work on
 * @context: the context to print on error
 *
 * Reads the camera picture abilities, but ignores them as we don't
 *  know how to interpret the data.
 * NOTE: Command is not implemented for some cameras
 *  and will cause an error if issued. The cameras for which we know
 *  it won't work are
 *  EOS D30
 *  EOS D60
 *  PowerShot S45.
 * NOTE: The "Protocol" file said that exactly 0x380 (900) bytes would
 *  be returned, but it's actually 904 bytes for the PowerShot G2.
 *  I don't know whether this was an error in the "Protocol" document
 *  or if different models return different lengths. -swestin 2002.01.09
 *
 * Returns: gphoto2 error code
 *
 */
int
canon_int_get_picture_abilities (Camera *camera, GPContext *context)
{
        unsigned char *msg;
        int len;

        GP_DEBUG ("canon_int_get_picture_abilities()");

        switch (camera->port->type) {
                case GP_PORT_USB:
                        msg = canon_usb_dialogue (camera, CANON_USB_FUNCTION_GET_PIC_ABILITIES,
                                                  &len, NULL, 0);
                        if ( msg == NULL )
                                return GP_ERROR_OS_FAILURE;
                        break;
                case GP_PORT_SERIAL:
                        msg = canon_serial_dialogue (camera, context, 0x1f, 0x12, &len, NULL);
                        if ( msg == NULL ) {
                                canon_serial_error_type (camera);
                                return GP_ERROR_OS_FAILURE;
                        }

                        break;
                GP_PORT_DEFAULT
        }

        if (len != 0x334) {
                GP_DEBUG ("canon_int_get_picture_abilities: Unexpected length returned "
                          "(expected %i got %i)", 0x334, len);
                return GP_ERROR_CORRUPTED_DATA;
        }

        return GP_OK;
}

/**
 * canon_int_pack_control_subcmd
 * @payload: to receive payload for %CANON_USB_FUNCTION_CONTROL_CAMERA
 *   command
 * @subcmd: Subcommand for remote capture command
 *   (e.g. %CANON_USB_CONTROL_SHUTTER_RELEASE)
 * @word0: 32-bit first word of payload (first command parameter)
 * @word1: 32-bit second word of payload (second command parameter)
 * @desc: to receive string describing subcommand.
 *
 * Builds the payload for a remote capture command. The length varies
 * by command, so this function a buffer of the correct size with the
 * proper stuff.
 *
 * Returns: size of payload buffer, payload data in @payload, command
 *  description in @desc
 *
 */
int
canon_int_pack_control_subcmd (unsigned char *payload, int subcmd,
                               int word0, int word1,
                               char *desc)
{
        int i, paysize;

        i = 0;
        while (canon_usb_control_cmd[i].num != 0) {
                if (canon_usb_control_cmd[i].num == subcmd)
                        break;
                i++;
        }
        if (canon_usb_control_cmd[i].num == 0) {
                GP_DEBUG ("canon_int_pack_control_subcmd: unknown subcommand %d", subcmd);
                sprintf (desc, "unknown subcommand");
                return 0;
        }

        sprintf (desc, "%s", canon_usb_control_cmd[i].description);
        paysize = canon_usb_control_cmd[i].cmd_length - 0x10;
        memset (payload, 0, paysize);
        if (paysize >= 0x04) htole32a(payload,     canon_usb_control_cmd[i].subcmd);
        if (paysize >= 0x08) htole32a(payload+0x4, word0);
        if (paysize >= 0x0c) htole32a(payload+0x8, word1);

        return paysize;
}

/**
 * canon_int_do_control_command:
 * @camera: Camera to work on
 * @subcmd: Subcommand for remote capture command
 *   (e.g. %CANON_USB_CONTROL_INIT)
 * @a: 32-bit first word of payload (first command parameter)
 * @b: 32-bit second word of payload (second command parameter)
 *
 * Executes a normal remote capture command (i.e. not
 * %CANON_USB_CONTROL_SHUTTER_RELEASE, which has its own special
 * function).
 *
 * Returns: gphoto2 status code
 *
 */
int
canon_int_do_control_command (Camera *camera, int subcmd, int a, int b)
{
        char payload[0x4c];
        char desc[128];
        int payloadlen;
        int datalen = 0;
        unsigned char *msg = NULL;

        payloadlen = canon_int_pack_control_subcmd(payload, subcmd,
                                                   a, b, desc);
        GP_DEBUG("%s++ with %x, %x", desc, a, b);

        if ( camera->pl->md->model == CANON_CLASS_6 ) {
                /* Newer protocol uses a different code, but with same
                 * response. It also needs an extra zero byte at the
                 * end. */
                payload[payloadlen++] = 0;
                msg = canon_usb_dialogue ( camera, 
                                           CANON_USB_FUNCTION_CONTROL_CAMERA_2,
                                           &datalen, payload, payloadlen );
        }
        else
                msg = canon_usb_dialogue ( camera, 
                                           CANON_USB_FUNCTION_CONTROL_CAMERA,
                                           &datalen, payload, payloadlen );
        if ( msg == NULL  && datalen != 0x1c) {
                /* ERROR */
                GP_DEBUG("%s datalen=%x",
                         desc, datalen);
                return GP_ERROR_CORRUPTED_DATA;
        }
        msg = NULL;
        datalen = 0;
        GP_DEBUG("%s--", desc);

        return GP_OK;
}

/**
 * canon_int_start_remote_control:
 * @camera: Camera to work on
 * @context: context for error reporting
 *
 * Initiate remote control of the shutter release and related photographic
 * parameters.
 *
 * Returns: gphoto2 status code
 *
 */
int 
canon_int_start_remote_control (Camera *camera, GPContext *context) 
{
        int status;

        GP_DEBUG ("canon_int_start_remote_control");

        if (camera->pl->remote_control) {
                GP_DEBUG ("canon_int_start_remote_control: Camera already under remote control");
                return GP_ERROR; 
        }

        /* Init, extends camera lens, puts us in remote capture mode */
        status = canon_int_do_control_command (camera,
                                               CANON_USB_CONTROL_INIT, 0, 0);


        if (status == GP_OK) 
                camera->pl->remote_control = 1;

        return status;
}


/**
 * canon_int_end_remote_control:
 * @camera: Camera to work on
 * @context: context for error reporting
 *
+ * Terminate remote control of the shutter release and related photographic
+ * parameters.
+ *
+ * Returns: gphoto2 status code
+ *
+ */
int 
canon_int_end_remote_control (Camera *camera, GPContext *context) 
{
        int status;


        if (!camera->pl->remote_control) {
                GP_DEBUG ("canon_int_end_remote_control: Camera not currently under remote control");
                return GP_ERROR;
        }

        GP_DEBUG ("canon_int_end_remote_control");

        /* End release mode */
        status = canon_int_do_control_command (camera,
                                               CANON_USB_CONTROL_EXIT, 0, 0);


        if (status == GP_OK) 
                camera->pl->remote_control = 0;

        return status;
}
 
/**
 * canon_int_capture_preview
 * @camera: camera to work with
 * @data: gets thumbnail image data.
 * @length: gets length of @data.
 * @context: context for error reporting
 *
 * Directs the camera to capture an image without storing it on the
 * camera. (remote shutter release via USB). The thumbnail will be
 * returned in @data. See the 'Protocol' file for details. Capture
 * through serial port is not (yet) supported.
 *
 * Returns: gphoto2 error code
 *
 */
int
canon_int_capture_preview (Camera *camera, unsigned char **data, int *length,
                           GPContext *context)
{
   int mstimeout = -1;
   int status;
   
   // modif michel
   //canonTransferMode transfermode;
   /* Should we download the thumbnail, or the full image ? */
   //if (camera->pl->capture_size == CAPTURE_FULL_IMAGE) 
   //        transfermode = REMOTE_CAPTURE_FULL_TO_PC;
   //else 
   //        transfermode = REMOTE_CAPTURE_THUMB_TO_PC;
   
   
   switch (camera->port->type) {
   case GP_PORT_USB:
      
      // modif michel 
      if( camera->pl->long_exposure == 0) {
         gp_port_get_timeout (camera->port, &mstimeout);
         GP_DEBUG("canon_int_capture_preview: usb port timeout starts at %dms", mstimeout);
         
         /*
         * Send a sequence of CONTROL_CAMERA commands.
         */
         
         gp_port_set_timeout (camera->port, 15000);
         /* Init, extends camera lens, puts us in remote capture mode */
         status = canon_int_start_remote_control (camera, context);
         
         if ( status < 0 )
            return status;
         
            /*
            * Set the captured image transfer mode.  We have four options
            * that we can specify any combo of, captured thumb to PC,
            * full to PC, thumb to disk, and full to disk.
            *
            * The to-PC option will return a length and integer
            * key from canon_usb_capture_dialogue() to use in the
            * "Download Captured Image" command.
            *
         */
         camera->pl->transfert_mode = REMOTE_CAPTURE_THUMB_TO_PC;
         GP_DEBUG ( "canon_int_capture_preview: transfer mode is %x\n", camera->pl->transfert_mode );
         status = canon_int_do_control_command (camera,
               CANON_USB_CONTROL_SET_TRANSFER_MODE,
               0x04, camera->pl->transfert_mode );   
         if ( status < 0 )
            return status;
         
         gp_port_set_timeout (camera->port, mstimeout);
         GP_DEBUG("canon_int_capture_preview: set camera port timeout back to %d seconds...", mstimeout / 1000 );
         
         /* Get release parameters a couple of times, just to
         see if that helps. */
         status = canon_int_do_control_command (camera,
            CANON_USB_CONTROL_GET_PARAMS,
            0x04, camera->pl->transfert_mode);
         if ( status < 0 )
            return status;
         
         status = canon_int_do_control_command (camera,
            CANON_USB_CONTROL_GET_PARAMS,
            0x04, camera->pl->transfert_mode);
         if ( status < 0 )
            return status;
         
         /* Lock keys here for EOS cameras */
         if ( camera->pl->md->model == CANON_CLASS_4 || camera->pl->md->model == CANON_CLASS_6 ) {
            status = canon_usb_lock_keys(camera,context);
            if ( status < 0 ) {
               gp_context_error (context, _("lock keys failed."));
               return status;
            }
         }
         // modif michel
      }
      /* Shutter Release
      Can't use normal "canon_int_do_control_command", as
      we must read the interrupt pipe before the response
      comes back for this commmand. */
      status = canon_usb_capture_dialogue ( camera,context );
      if ( status != GP_OK ) {
         /* Try to leave camera in a usable state. */
         canon_int_end_remote_control (camera, context);
         return GP_ERROR_OS_FAILURE;
      }
      
      /* Download the thumbnail image. */
      if (camera->pl->transfert_mode == REMOTE_CAPTURE_FULL_TO_PC) {
         
         /* Download the image. */
         if ( camera->pl->image_length > 0 ) {
            status = canon_usb_get_captured_image ( camera, camera->pl->image_key, data, length, context );
            if ( status < 0 ) {
               GP_DEBUG ( "canon_int_capture_preview:"
                  " image download failed, status= %i", status );
               return status;
            }
         }
      } else if (camera->pl->transfert_mode == REMOTE_CAPTURE_THUMB_TO_PC) {
         
         /* Download the thumbnail image. */
         if ( camera->pl->thumb_length > 0 ) {
            status = canon_usb_get_captured_thumbnail ( camera, camera->pl->image_key, data, length, context );
            if ( status < 0 ) {
               GP_DEBUG ( "canon_int_capture_preview:"
                  " thumbnail download failed, status= %i", status );
               return status;
            }
         }
         
      }
      
      /* End release mode */
      status = canon_int_end_remote_control (camera, context);
      if ( status < 0 )
         return status;
      
      break;
        case GP_PORT_SERIAL:
           return GP_ERROR_NOT_SUPPORTED;
           break;
           GP_PORT_DEFAULT
        }
        
        return GP_OK;
}

/**
 * canon_int_find_new_image:
 * @camera: Camera * to this camera
 * @initial_state: Camera directory dump before image capture
 * @final_state: Directory dump after image capture
 * @path: Will be filled in with the path and filename of the captured
 *        image, in canonical gphoto2 format.
 *
 * Compares two complete dumps of the camera directory: before and
 * after an image capture command. The first new image file is found,
 * and its pathname is decoded into the given CameraFilePath.
 *
 */
static void canon_int_find_new_image ( Camera *camera, unsigned char *initial_state, unsigned char *final_state,
                           CameraFilePath *path )
{
        unsigned char *old_entry = initial_state, *new_entry = final_state;

        /* Set default path name */
        strncpy ( path->name, _("*UNKNOWN*"), sizeof(path->name) );
        strncpy ( path->folder, _("*UNKNOWN*"), sizeof(path->folder) );

        path->folder[0] = 0; /* Start with null pathname string. */
        GP_DEBUG ( "canon_int_find_new_image: starting directory compare" );
        while ( le16atoh ( old_entry+CANON_DIRENT_ATTRS ) != 0
                || le32atoh ( old_entry + CANON_DIRENT_SIZE ) != 0
                || le32atoh ( old_entry + CANON_DIRENT_TIME ) != 0 ) {
                char *old_name = old_entry + CANON_DIRENT_NAME,
                        *new_name = new_entry + CANON_DIRENT_NAME;
                GP_DEBUG ( " old entry \"%s\", attr = 0x%02x, size=%i",
                           old_name,
                           old_entry[CANON_DIRENT_ATTRS],
                           le32atoh ( old_entry + CANON_DIRENT_SIZE ) );
                GP_DEBUG ( " new entry \"%s\", attr = 0x%02x, size=%i",
                           new_name,
                           new_entry[CANON_DIRENT_ATTRS],
                           le32atoh ( new_entry + CANON_DIRENT_SIZE ) );
                if ( *old_entry != *new_entry
                     || le32atoh ( old_entry + CANON_DIRENT_SIZE ) != le32atoh ( new_entry + CANON_DIRENT_SIZE )
                     || le32atoh ( old_entry + CANON_DIRENT_TIME ) != le32atoh ( new_entry + CANON_DIRENT_TIME )
                     || strcmp ( old_name, new_name ) ) {
                        /* Mismatch. Presumably a
                           new file, but is it an
                           image file? */
                        GP_DEBUG ( "Found mismatch" );
                        if ( is_image ( new_name ) ) {
                                /* Yup, we'll assume that this is the new image. */
                                GP_DEBUG ( "  Found our new image file" );
                                strncpy ( path->name, new_name,
                                          strlen ( new_name ) );
                                strcpy ( path->folder, canon2gphotopath ( camera, path->folder ) );
                                break;
                        }
                        else {
                                /* The mismatch is not an image
                                   file. There are three
                                   possibilities:

                                   1. This is a new directory with no
                                      files. The next entry will be
                                      another directory.

                                   2. This is an auxiliary file
                                      (sound, thumbnail, catalog).

                                   In either of these cases, the thing
                                   to do is to skip this entry in the
                                   new directory.

                                   3. This is a new directory with new
                                      files, and we will enter it.
                                      The next entry in the new
                                      directory will be a new file,
                                      which may well be our new image
                                      file.

                                    */
                                if ( le16atoh ( new_entry+CANON_DIRENT_ATTRS ) & CANON_ATTR_RECURS_ENT_DIR ) {
                                        if ( !strcmp ( "..", new_name ) ) {
                                                /* Pop out of this directory */
                                                char *local_dir = strrchr(path->folder,'\\') + 1;
                                                /* The EOS 350D has
                                                 * ".." entries right
                                                 * up to the root, so
                                                 * we need to avoid
                                                 * dereferencing a
                                                 * null pointer. */
                                                if ( local_dir != NULL && local_dir > path->folder ) {
                                                        GP_DEBUG ( "Leaving directory \"%s\"", local_dir );
                                                        local_dir[-1] = 0;
                                                }
                                                else
                                                        GP_DEBUG ( "Leaving top directory" );
                                        }
                                        else {
                                                /* New directory, and we need to enter it. */
                                                GP_DEBUG ( "Entering directory \"%s\"", new_name );
                                                if ( new_entry[CANON_DIRENT_NAME] == '.' )
                                                        /* Ignore a leading dot */
                                                        strncat ( path->folder,
                                                                  new_name + 1,
                                                                  sizeof(path->folder) - strlen(path->folder) - 1 );
                                                else
                                                        strncat ( path->folder,
                                                                  new_name,
                                                                  sizeof(path->folder) - strlen(path->folder) - 1 );
                                        }
                                }
                                new_entry += CANON_MINIMUM_DIRENT_SIZE + strlen ( new_entry+CANON_DIRENT_NAME );
                        }
                }
                else {
                        if ( le16atoh ( old_entry+CANON_DIRENT_ATTRS ) & CANON_ATTR_RECURS_ENT_DIR ) {
                                /* Entered a new directory; append its
                                   name to the current folder path.
                                   The end of a directory is signaled
                                   by an entry with zero length and
                                   time, and name "..". */
                                if ( !strcmp ( "..", old_name ) ) {
                                        /* Pop out of this directory */
                                        char *local_dir = strrchr(path->folder,'\\') + 1;
                                        /* The EOS 350D has
                                         * ".." entries right
                                         * up to the root, so
                                         * we need to avoid
                                         * dereferencing a
                                         * null pointer. */
                                        if ( local_dir != NULL && local_dir > path->folder ) {
                                                GP_DEBUG ( "Leaving directory \"%s\"", local_dir );
                                                local_dir[-1] = 0;
                                        }
                                        else
                                                GP_DEBUG ( "Leaving top directory" );
                                 }
                                else {
                                        GP_DEBUG ( "Entering directory \"%s\"", old_name );
                                        if ( old_name[0] == '.' )
                                                /* Ignore a leading dot */
                                                strncat ( path->folder,
                                                          old_name + 1,
                                                          sizeof(path->folder) - strlen(path->folder) - 1 );
                                        else
                                                strncat ( path->folder,
                                                          old_name,
                                                          sizeof(path->folder) - strlen(path->folder) - 1 );
                                }
                        }
                        /* Move to next entry */
                        new_entry += CANON_MINIMUM_DIRENT_SIZE + strlen ( new_entry+CANON_DIRENT_NAME );
                        old_entry += CANON_MINIMUM_DIRENT_SIZE + strlen ( old_entry+CANON_DIRENT_NAME );
                }
        }
}

/**
 * canon_int_capture_image:
 * @camera: camera to work with
 * @path: gets filled in with the path and filename of the captured
 *   image, in canonical gphoto2 format.
 * @context: context for error reporting
 *
 * Directs the camera to capture an image (remote shutter release via
 * USB).  See the 'Protocol' file for details. Capture through serial
 * port is not (yet) supported.
 *
 * Returns: gphoto2 error code
 *
 */
int
canon_int_capture_image (Camera *camera, CameraFilePath *path,
                         GPContext *context)
{
   int mstimeout = -1;
   int status;
   
   unsigned char *data = NULL;
   unsigned char *initial_state, *final_state; /* For comparing
                                               * before/after
   * directories */
   int initial_state_len, final_state_len;
   
   switch (camera->port->type) {
   case GP_PORT_USB:
   /* List all directories on the camera to get a
      baseline to find the new file. */
      status = canon_usb_list_all_dirs ( camera, &initial_state, &initial_state_len, context );
      
      if ( status < 0 ) {
         gp_context_error (context,
            _("canon_int_capture_image: initial canon_usb_list_all_dirs() failed with status %li"),
            (long)status );
         return status;
      }
      
      gp_port_get_timeout (camera->port, &mstimeout);
                GP_DEBUG("canon_int_capture_image: usb port timeout starts at %dms", mstimeout);

      // if INTERNAL remote cpature
      if( camera->pl->long_exposure == 0 ) {
         /*
          * Send a sequence of CONTROL_CAMERA commands.
          */
         
         gp_port_set_timeout (camera->port, 15000);
         /* Init, extends camera lens, puts us in remote capture mode */
         status = canon_int_start_remote_control (camera, context);
         if ( status < 0 )
            return status;
         
            /*
            * Set the captured image transfer mode.  We have four options
            * that we can specify any combo of, captured thumb to PC,
            * full to PC, thumb to disk, and full to disk.
            *
            * The to-PC option will return a length and integer
            * key from canon_usb_capture_dialogue() to use in the
            * "Download Captured Image" command.
            *
         */
         GP_DEBUG ( "canon_int_capture_image: transfer mode is %x\n", camera->pl->transfert_mode );
         
         //modif michel
         status = canon_int_do_control_command (camera,
                         CANON_USB_CONTROL_SET_TRANSFER_MODE,
                         0x04, camera->pl->transfert_mode );   
         if ( status < 0 ) {
            canon_int_end_remote_control (camera, context);
            return status;
         }
         
         gp_port_set_timeout (camera->port, mstimeout);
         GP_DEBUG("canon_int_capture_image: set camera port timeout back to %d seconds...", mstimeout / 1000 );
         
         /* Get release parameters a couple of times, just to
         see if that helps. */
         //status = canon_int_do_control_command (camera,
         //   CANON_USB_CONTROL_GET_PARAMS,
         //   0x00, 0);
         //if ( status < 0 ) {
         //   canon_int_do_control_command (camera,
         //      CANON_USB_CONTROL_EXIT,
         //      0, 0);
         //   return status;
         //}

#ifdef DEBUG_TINY_IMAGES
         {
            unsigned char *result_block;
            int result_len, payload_len;
            unsigned char payload[0x59];
            unsigned char *params;
            unsigned char desc[1024];
            payload_len = canon_int_pack_control_subcmd ( payload,
               CANON_USB_CONTROL_GET_PARAMS,
               0x00, 0,
               desc );
            result_block = canon_usb_dialogue ( camera, CANON_USB_FUNCTION_CONTROL_CAMERA,
               &result_len, payload, payload_len );
            if ( result_block == NULL )
               return GP_ERROR;
            
            memset ( payload, 0, sizeof(payload) );
            params = payload+0x08;
            memcpy ( params, result_block+0x0c, 0x30 );
            
            payload[0] = 0x07;
            payload[4] = 0x30;
            params[1] = 2;       /* "Normal" compression */
            params[2] = 1;       /* JPEG */
            params[3] = 2;       /* small */
            params[4] = params[5] = 0; /* Self timer off */
            params[6] = 0;             /* Flash off */
            params[7] = 0;             /* Beep off */
            result_block = canon_usb_dialogue ( camera, CANON_USB_FUNCTION_CONTROL_CAMERA,
               &result_len, payload, 0x38 );
            if ( result_block == NULL )
               return GP_ERROR;
         }
#endif /* DEBUG_TINY_IMAGES */

         //status = canon_int_do_control_command (camera,
         //   CANON_USB_CONTROL_GET_PARAMS,
         //   0x04, camera->pl->transfert_mode);
         //if ( status < 0 ) {
         //   canon_int_do_control_command (camera,
         //      CANON_USB_CONTROL_EXIT,
         //      0, 0);
         //   return status;
         //}
         
         // Lock keys here for EOS 
         if ( camera->pl->md->model == CANON_CLASS_4 || camera->pl->md->model == CANON_CLASS_6 ) {
            status = canon_usb_lock_keys(camera,context);
            if ( status < 0 ) {
               gp_context_error (context, _("lock keys failed."));
               canon_int_end_remote_control (camera, context);
               return status;
            }
         }

      }
         
      /* Shutter Release
      Can't use normal "canon_int_do_control_command", as
      we must read the interrupt pipe before the response
      comes back for this commmand. */
      status = canon_usb_capture_dialogue ( camera,context );
      if ( status != GP_OK ) {
         /* Try to leave camera in a usable state. */
         canon_int_end_remote_control (camera, context);
         return GP_ERROR_OS_FAILURE;
      }
      
      /* End release mode */
      canon_int_end_remote_control (camera, context);
      if ( status < 0 )
         return status;
      
      // modif michel  unlock keys
      if( camera->pl->long_exposure == 0 ) {
         if ( canon_usb_unlock_keys ( camera, context ) < 0 ) {
             GP_DEBUG ( "canon_int_capture_image_path: couldn't unlock keys after capture." );
            return status;
         }
      } 

      /* Now list all directories on the camera; this has
         presumably added an image file. Find the difference
      and decode to return real path and file names. */
      status = canon_usb_list_all_dirs ( camera, &final_state, &final_state_len, context );
      if ( status < 0 ) {
         gp_context_error ( context,
            _("canon_int_capture_image:"
            " final canon_usb_list_all_dirs() failed with status %i"),
            status );
         return status;
      }
      
      /* Find new file name in camera directory */
      canon_int_find_new_image ( camera, initial_state, final_state, path );
      free ( initial_state );
      free ( final_state );
      break;
        case GP_PORT_SERIAL:
           return GP_ERROR_NOT_SUPPORTED;
           break;
           GP_PORT_DEFAULT
        }
        
        return GP_OK;
}

/**
 * canon_int_capture_without_cf:
 * @camera: camera to work with
 * @path: gets filled in with the path and filename of the captured
 *   image, in canonical gphoto2 format.
 * @context: context for error reporting
 *
 * Directs the camera to capture an image (remote shutter release via
 * USB).  See the 'Protocol' file for details. Capture through serial
 * port is not (yet) supported.
 *
 * Returns: gphoto2 error code
 *
 */
 
int
canon_int_capture_without_cf (Camera *camera, unsigned char **data, int *length,
                         GPContext *context)
{
   
   int mstimeout = -1;
   int status;
   
   switch (camera->port->type) {
   case GP_PORT_USB:
      gp_port_get_timeout (camera->port, &mstimeout);
      GP_DEBUG("canon_int_capture_without_cf: usb port timeout starts at %dms", mstimeout);
      
      // if INTERNAL remote cpature
      if( camera->pl->long_exposure == 0 ) {         
         // Send a sequence of CONTROL_CAMERA commands.
         gp_port_set_timeout (camera->port, 15000);
         // Init, extends camera lens, puts us in remote capture mode 
         status = canon_int_start_remote_control (camera, context);
         if ( status < 0 ) {
            return status;
         }
         
         /*
            * Set the captured image transfer mode.  We have four options
            * that we can specify any combo of, captured thumb to PC,
            * full to PC, thumb to disk, and full to disk.
            *
            * The to-PC option will return a length and integer
            * key from canon_usb_capture_dialogue() to use in the
            * "Download Captured Image" command.
            *
         */
         GP_DEBUG ( "canon_int_capture_without_cf: transfer mode is %x\n", camera->pl->transfert_mode );
         status = canon_int_do_control_command (camera,
               CANON_USB_CONTROL_SET_TRANSFER_MODE,
               0x04, camera->pl->transfert_mode );            
         if ( status < 0 ) {
            canon_int_end_remote_control (camera, context);
            return status;
         }
         
         gp_port_set_timeout (camera->port, mstimeout);
         GP_DEBUG("canon_int_capture_without_cf: set camera port timeout back to %d seconds...", mstimeout / 1000 );
         
         /* Get release parameters a couple of times, just to
         see if that helps. */
         status = canon_int_do_control_command (camera,
                                                CANON_USB_CONTROL_GET_PARAMS,
                                                0x00, 0);
         if ( status < 0 ) {
                 canon_int_end_remote_control (camera, context);
         return status;
         }
         
        /* Lock keys here for EOS */
         if ( camera->pl->md->model == CANON_CLASS_4 || camera->pl->md->model == CANON_CLASS_6 ) {
            status = canon_usb_lock_keys(camera,context);
            if ( status < 0 ) {
               gp_context_error (context, _("lock keys failed."));
               canon_int_end_remote_control (camera, context);
               return status;
            }
         }
      }      
 
 
      /* Shutter Release
      Can't use normal "canon_int_do_control_command", as
      we must read the interrupt pipe before the response
      comes back for this commmand. */
      status = canon_usb_capture_dialogue ( camera, context );
      if ( status != GP_OK ) {
         /* Try to leave camera in a usable state. */
         canon_int_end_remote_control (camera, context);
         return GP_ERROR_OS_FAILURE;
      }
      
      if (camera->pl->transfert_mode == REMOTE_CAPTURE_FULL_TO_PC) {
         status = canon_usb_get_captured_image ( camera, camera->pl->image_key, data, length, context );
      } else {
         *data = NULL;
         length = 0;
      }
      
     // modif michel  unlock keys
      if( camera->pl->long_exposure == 0 ) {
         if ( canon_usb_unlock_keys ( camera, context ) < 0 ) {
             GP_DEBUG ( "canon_int_capture_image_path: couldn't unlock keys after capture." );
            return status;
         }
      } 

      /* End release mode */
      canon_int_end_remote_control (camera, context);
      if ( status < 0 ) {
         GP_DEBUG ( "canon_int_capture_without_cf:"
            " get_captured_image failed, status= %d", status );
         return status;
      }
      break;
   case GP_PORT_SERIAL:
      return GP_ERROR_NOT_SUPPORTED;
      break;
      GP_PORT_DEFAULT
   }
   
   return GP_OK;
}


/**
 * canon_int_set_long_exposure:
 * @camera: camera to get the transfert mode 
 * @mode: the date to set (in UTC)
 * @context: context for error reporting
 *
 * Set camera's current time.
 *
 * Canon cameras know nothing about time zones so we have to convert it to local
 * time (but still expressed in UNIX time format (seconds since 1970-01-01).
 *
 * Returns: gphoto2 error code
 *
 */

int
canon_int_prepare_long_exposure (Camera *camera, int remoteState, GPContext *context)
{
   int status;
   
   GP_DEBUG ("canon_int_prepare_long_exposure: %d", (unsigned int) remoteState);   
   
   switch (camera->port->type) {
   case GP_PORT_USB:

      if( remoteState == 1 ) {
         status = canon_int_start_remote_control (camera, context);
         if ( status == GP_OK) {
            //modif michel
            status = canon_int_do_control_command (camera,
                         CANON_USB_CONTROL_SET_TRANSFER_MODE,
                         0x04, camera->pl->transfert_mode );   
            if (status == GP_OK ) {
               camera->pl->long_exposure= 1;
               //Clear pipe
               canon_usb_clear_pipe(camera, context);            
            }
         }
         
      } else {
         // disable  external remote capture
         camera->pl->long_exposure =  0;
      }


      return status;        
      break;
   case GP_PORT_SERIAL:
      GP_DEBUG ("canon_int_set_long_exposure not implemented for serial port");                   
      return GP_ERROR_BAD_PARAMETERS;
      break;
      GP_PORT_DEFAULT
   }
   
   return GP_OK;
}

/**
 * canon_int_set_file_attributes:
 * @camera: camera to work with
 * @file: file to work on
 * @dir: directory to work in
 * @attrs: #canonDirentAttributeBits with the bits to set
 * @context: context for error reporting
 *
 * Sets a file's attributes. See the 'Protocol' file for details.
 *
 * Returns: gphoto2 error code
 *
 */
int
canon_int_set_file_attributes (Camera *camera, const char *file, const char *dir,
                               canonDirentAttributeBits attrs, GPContext *context)
{
        unsigned char *payload;
        unsigned char *msg;
        unsigned char attr[4];
        int len, payload_length;

        GP_DEBUG ("canon_int_set_file_attributes() called for '%s' '%s', attributes 0x%x",
                  dir, file, attrs);

        attr[0] = attr[1] = attr[2] = 0;
        attr[3] = attrs;

        switch (camera->port->type) {
                case GP_PORT_USB:
                        payload_length = 4 + strlen (dir) + 1 + strlen (file) + 2;
                        payload = (unsigned char*) calloc ( payload_length, sizeof(unsigned char) );
                        /* create payload (yes, path and filename are two different strings
                         * and not meant to be concatenated)
                         */
                        return canon_usb_set_file_attributes ( camera, attrs, dir, file, context );
                        break;
                case GP_PORT_SERIAL:
                        msg = canon_serial_dialogue (camera, context, 0xe, 0x11, &len, attr, 4,
                                                     dir, strlen (dir) + 1, file,
                                                     strlen (file) + 1, NULL);
                        if ( msg == NULL ) {
                                canon_serial_error_type (camera);
                                return GP_ERROR_OS_FAILURE;
                        }
                        break;
                GP_PORT_DEFAULT
        }

        if (len != 0x4) {
                GP_DEBUG ("canon_int_set_file_attributes: Unexpected length returned "
                          "(expected %i got %i)", 0x4, len);
                return GP_ERROR_CORRUPTED_DATA;
        }

        GP_LOG (GP_LOG_DATA,
                "canon_int_set_file_attributes: returned four bytes as expected, "
                "we should check if they indicate error or not. Returned data :");
        gp_log_data ("canon", msg, 4);

        return GP_OK;
}

/**
 * canon_int_set_owner_name:
 * @camera: the camera to set the owner name of
 * @name: owner name to set the camera to
 * @context: context for error reporting
 *
 * Sets the camera owner name. The string should not be more than 30
 * characters long. We call #get_owner_name afterwards in order to
 * check that everything went fine.
 *
 * Returns: gphoto2 error code
 *
 */
int
canon_int_set_owner_name (Camera *camera, const char *name, GPContext *context)
{
        unsigned char *msg;
        char payload[4];
        int len;

        GP_DEBUG ("canon_int_set_owner_name() called, name = '%s'", name);
        if (strlen (name) > 30) {
                gp_context_error (context,
                                  _("Name '%s' (%li characters) "
                                    "too long, maximum 30 characters are "
                                    "allowed."), name, (long)strlen (name));
                return GP_ERROR_BAD_PARAMETERS;
        }

        switch (camera->port->type) {
                case GP_PORT_USB:
                        if ( camera->pl->md->model == CANON_CLASS_6 ) {
                                msg = canon_usb_dialogue (camera, CANON_USB_FUNCTION_CAMERA_CHOWN_2,
                                                          &len, name, strlen (name) + 1);
                                htole32a ( payload, 0x0f );
                        }
                        else
                                msg = canon_usb_dialogue (camera, CANON_USB_FUNCTION_CAMERA_CHOWN,
                                                          &len, name, strlen (name) + 1);
                        if ( msg == NULL )
                                return GP_ERROR_OS_FAILURE;
                        break;
                case GP_PORT_SERIAL:
                        msg = canon_serial_dialogue (camera, context, 0x05, 0x12, &len, name,
                                                     strlen (name) + 1, NULL);
                        if ( msg == NULL ) {
                                canon_serial_error_type (camera);
                                return GP_ERROR_OS_FAILURE;
                        }
                        break;
                GP_PORT_DEFAULT
        }

        if (len != 0x04) {
                GP_DEBUG ("canon_int_set_owner_name: Unexpected length returned "
                          "(expected %i got %i)", 0x4, len);
                return GP_ERROR_CORRUPTED_DATA;
        }

        return canon_int_identify_camera (camera, context);
}


/**
 * canon_int_get_time:
 * @camera: camera to get the current time of
 * @camera_time: pointer to where you want the camera time (NOT IN UTC!!!)
 * @context: context for error reporting
 *
 * Get camera's current time.
 *
 * The camera gives time in little endian format, therefore we need
 * to swap the 4 bytes on big-endian machines.
 *
 * Note: the time returned from the camera is not UTC but local time.
 * This means you should only use functions that don't adjust the
 * timezone, like gmtime(), instead of functions that do, like localtime()
 * since otherwise you will end up with the wrong time.
 *
 * We pass it along to calling functions in local time instead of UTC
 * since it is more correct to say 'Camera time: 2002-01-01 00:00:00'
 * if that is what the display says and not to display the cameras time
 * converted to local timezone (which will of course be wrong if you
 * are not in the timezone the cameras clock was set in).
 *
 * Returns: gphoto2 error code
 *
 */
int
canon_int_get_time (Camera *camera, time_t *camera_time, GPContext *context)
{
        unsigned char *msg;
        int len;

        GP_DEBUG ("canon_int_get_time()");

        switch (camera->port->type) {
                case GP_PORT_USB:
                        msg = canon_usb_dialogue (camera, CANON_USB_FUNCTION_GET_TIME, &len,
                                                  NULL, 0);
                        if ( msg == NULL )
                                return GP_ERROR_OS_FAILURE;
                        break;
                case GP_PORT_SERIAL:
                        msg = canon_serial_dialogue (camera, context, 0x03, 0x12, &len, NULL);
                        if ( msg == NULL ) {
                                canon_serial_error_type (camera);
                                return GP_ERROR_OS_FAILURE;
                        }
                        break;
                GP_PORT_DEFAULT
        }

        if (len != 0x10) {
                GP_DEBUG ("canon_int_get_time: Unexpected length returned "
                          "(expected %i got %i)", 0x10, len);
                return GP_ERROR_CORRUPTED_DATA;
        }

        if (camera_time != NULL)
                *camera_time = (time_t) le32atoh (msg + 4);

        /* XXX should strip \n at the end of asctime() return data */
        GP_DEBUG ("Camera time: %s", asctime (gmtime (camera_time)));

        return GP_OK;
}


/**
 * canon_int_set_time:
 * @camera: camera to get the current time of
 * @date: the date to set (in UTC)
 * @context: context for error reporting
 *
 * Set camera's current time.
 *
 * Canon cameras know nothing about time zones so we have to convert it to local
 * time (but still expressed in UNIX time format (seconds since 1970-01-01).
 *
 * Returns: gphoto2 error code
 *
 */

int
canon_int_set_time (Camera *camera, time_t date, GPContext *context)
{
        unsigned char *msg;
        int len;
        char payload[12];
        time_t new_date;
        struct tm *tm;

        GP_DEBUG ("canon_int_set_time: %i=0x%x %s", (unsigned int) date, (unsigned int) date,
                  asctime (localtime (&date)));

        /* call localtime() just to get 'extern long timezone' / tm->tm_gmtoff set.
         *
         * this handles DST too (at least if HAVE_TM_GMTOFF), if you are in UTC+1
         * tm_gmtoff is 3600 and if you are in UTC+1+DST tm_gmtoff is 7200 (if your
         * DST is one hour of course).
         */
        tm = localtime (&date);

        /* convert to local UNIX time since canon cameras know nothing about timezones */

#ifdef HAVE_TM_GMTOFF
        new_date = date + tm->tm_gmtoff;
        GP_DEBUG ("canon_int_set_time: converted %ld to localtime %ld (tm_gmtoff is %ld)",
                  (long)date, (long)new_date, (long)tm->tm_gmtoff);
#else
        new_date = date - timezone;
        GP_DEBUG ("canon_int_set_time: converted %ld to localtime %ld (timezone is %ld)",
                  (long)date, (long)new_date, (long)timezone);
#endif

        memset (payload, 0, sizeof (payload));

        htole32a (payload, (unsigned int) new_date);

        switch (camera->port->type) {
                case GP_PORT_USB:
                        msg = canon_usb_dialogue (camera, CANON_USB_FUNCTION_SET_TIME, &len,
                                                  payload, sizeof (payload));
                        if ( msg == NULL )
                                return GP_ERROR_OS_FAILURE;
                        break;
                case GP_PORT_SERIAL:
                        msg = canon_serial_dialogue (camera, context, 0x04, 0x12, &len,
                                                     payload, sizeof (payload), NULL);
                        if ( msg == NULL ) {
                                canon_serial_error_type (camera);
                                return GP_ERROR_OS_FAILURE;
                        }

                        break;
                GP_PORT_DEFAULT
        }

        if (len != 0x4) {
                GP_DEBUG ("canon_int_set_time: Unexpected length returned "
                          "(expected %i got %i)", 0x4, len);
                return GP_ERROR_CORRUPTED_DATA;
        }

        return GP_OK;
}

/**
 * canon_int_ready:
 * @camera: camera to get ready
 * @context: context for error reporting
 *
 * Switches the camera on, detects the model and sets its speed.
 *
 * Returns: gphoto2 error code
 *
 */
int
canon_int_ready (Camera *camera, GPContext *context)
{
        int res;

        GP_DEBUG ("canon_int_ready()");

        switch (camera->port->type) {
                case GP_PORT_USB:
                        res = canon_usb_ready (camera, context);
                        break;
                case GP_PORT_SERIAL:
                        res = canon_serial_ready (camera, context);
                        break;
                GP_PORT_DEFAULT
        }

        return (res);
}

/**
 * canon_int_get_disk_name:
 * @camera: camera to ask for disk drive
 * @context: context for error reporting
 *
 * Ask the camera for the name of the flash storage
 * device. Usually "D:" or something like that.
 *
 * Returns: name of disk
 *
 */
char *
canon_int_get_disk_name (Camera *camera, GPContext *context)
{
        unsigned char *msg;
        int len, res;

        GP_DEBUG ("canon_int_get_disk_name()");

        switch (camera->port->type) {
                case GP_PORT_USB:
                        if ( camera->pl->md->model == CANON_CLASS_6 )
                                /* Newer protocol uses a different code, but with same response. */
                                res = canon_usb_long_dialogue (camera,
                                                               CANON_USB_FUNCTION_FLASH_DEVICE_IDENT_2,
                                                               &msg, &len, 1024, NULL, 0, 0, context);
                        else
                                res = canon_usb_long_dialogue (camera,
                                                               CANON_USB_FUNCTION_FLASH_DEVICE_IDENT,
                                                               &msg, &len, 1024, NULL, 0, 0, context);
                        if (res != GP_OK) {
                                GP_DEBUG ("canon_int_get_disk_name: canon_usb_long_dialogue "
                                          "failed! returned %i", res);
                                return NULL;
                        }
                        break;
                case GP_PORT_SERIAL:
                        msg = canon_serial_dialogue (camera, context, 0x0a, 0x11, &len, NULL);
                        if ( msg == NULL ) {
                                canon_serial_error_type (camera);
                                return NULL;
                        }

                        if (len < 5)
                                return NULL;    /* should be GP_ERROR_CORRUPTED_DATA */

                        /* this is correct even though it looks a bit funny. canon_serial_dialogue()
                         * has a static buffer, strdup() part of that buffer and return to our caller.
                         */
                        msg = strdup ((char *) msg + 4);        /* @@@ should check length */
                        if ( msg == NULL ) {
                                GP_DEBUG ("canon_int_get_disk_name: could not allocate %li "
                                          "bytes of memory to hold response",
                                          (long)(strlen ((char *) msg + 4)));
                                return NULL;
                        }
                        break;
                GP_PORT_DEFAULT_RETURN (NULL)
        }

        if ( msg == NULL )
                return NULL;

        GP_DEBUG ("canon_int_get_disk_name: disk '%s'", msg);

        return msg;
}

/**
 * canon_int_get_disk_name_info:
 * @camera: camera to ask about disk
 * @name: name of the disk
 * @capacity: returned maximum disk capacity
 * @available: returned currently available disk capacity
 * @context: context for error reporting
 *
 * Gets available room and max capacity of a disk given by @name.
 *
 * Returns: boolean value denoting success (FIXME: ATTENTION!)
 *
 */
int
canon_int_get_disk_name_info (Camera *camera, const char *name, int *capacity, int *available,
                              GPContext *context)
{
        unsigned char *msg = NULL;
        char name_local[128];
        int len;                /* is set in both USB and SERIAL cases */
        int cap=0, ava=0;       /* only set in USB case */

        GP_DEBUG ("canon_int_get_disk_name_info() name '%s'", name);

        CON_CHECK_PARAM_NULL (name);
        CON_CHECK_PARAM_NULL (capacity);
        CON_CHECK_PARAM_NULL (available);

        switch (camera->port->type) {
                case GP_PORT_USB:
                        if ( camera->pl->md->model == CANON_CLASS_6 ) {
                                /* Newer protocol uses a different code, but with same response. */
                                strncpy ( name_local, name, sizeof(name_local) );
                                len = strlen(name_local);
                                if ( name_local[len-1] == '\\' )
                                        name_local[len-1] = 0;
                                msg = canon_usb_dialogue (camera, CANON_USB_FUNCTION_DISK_INFO_2, &len,
                                                          name_local, len );
				if ( msg == NULL )
					return GP_ERROR_OS_FAILURE;
                                /* These newer cameras report sizes in
                                 * K instead of bytes, so max capacity
                                 * is 4TB rather than 4GB. */
                                cap = le32atoh (msg + 4) * 1024;
                                ava = le32atoh (msg + 8) * 1024;
                        }
                        else {
                                msg = canon_usb_dialogue (camera, CANON_USB_FUNCTION_DISK_INFO, &len,
                                                          name, strlen (name) + 1);
				if ( msg == NULL )
					return GP_ERROR_OS_FAILURE;
                                cap = le32atoh (msg + 4);
                                ava = le32atoh (msg + 8);
                        }
                        break;
                case GP_PORT_SERIAL:
                        msg = canon_serial_dialogue (camera, context, 0x09, 0x11, &len, name,
                                                     strlen (name) + 1, NULL);
                        if ( msg == NULL ) {
                                canon_serial_error_type (camera);
                                return GP_ERROR_OS_FAILURE;
                        }
                        break;
                GP_PORT_DEFAULT
        }

        if (len < 0x0c) {
                GP_DEBUG ("canon_int_get_disk_name_info: "
                        "Unexpected length returned "
                        "(expected %i got %i)", 0x0c, len);
                return GP_ERROR_CORRUPTED_DATA;
        }
        
        /* Capacity and available are not NULL as verified above.
        * But cap and ava are only set for the USB case, so we have to check for that.
        * If you know the logic better, feel free to improve it. */
        switch (camera->port->type) {
                case GP_PORT_USB:                         
                        *capacity = cap;
                        *available = ava;
                        GP_DEBUG ("canon_int_get_disk_name_info: "
                                "capacity %i kb, available %i kb",
                                cap > 0 ? (cap / 1024) : 0,
                                ava > 0 ? (ava / 1024) : 0);
                        break;
                GP_PORT_DEFAULT
        }

        return GP_OK;
}


/**
 * gphoto2canonpath:
 * @camera: camera to use
 * @path: gphoto2 path
 * @context: context for error reporting
 *
 * convert gphoto2 path  (e.g.   "/dcim/116CANON/img_1240.jpg")
 * into canon style path (e.g. "D:\DCIM\116CANON\IMG_1240.JPG")
 *
 * Canon cameras use FAT with upper case internally, so we convert
 * into that.
 *
 * Returns: immutable string with converted path name
 *
 */
const char *
gphoto2canonpath (Camera *camera, const char *path, GPContext *context)
{
        static char tmp[2000];
        char *p;

        if (path[0] != '/') {
                GP_DEBUG ("Non-absolute gphoto2 path cannot be converted");
                return NULL;
        }

        if (camera->pl->cached_drive == NULL) {
                GP_DEBUG ("NULL camera->pl->cached_drive in gphoto2canonpath");
                camera->pl->cached_drive = canon_int_get_disk_name (camera, context);
                if (camera->pl->cached_drive == NULL) {
                        GP_DEBUG ("2nd NULL camera->pl->cached_drive in gphoto2canonpath");
                        return NULL;
                }
        }

        snprintf (tmp, sizeof (tmp), "%s%s", camera->pl->cached_drive, path);

        /* Convert to upper case, since FAT file system on camera
         doesn't do case, and replace all slashes by backslashes */
        for (p = tmp; *p != '\0'; p++) {
                if ( *p != (char)toupper ( *p ) )
                        /* We don't allow lower-case in path names. */
                        gp_context_error (context, _("Lower case letters in %s not allowed."),
                                          path );
                if (*p == '/')
                        *p = '\\';
                *p = (char) toupper(*p);
        }

        /* remove trailing backslash, making sure buffer ends with \0 */
        if ((p > tmp) && (*(p - 1) == '\\'))
                *(p - 1) = '\0';

        GP_LOG (GP_LOG_DATA, "gphoto2canonpath: converted '%s' to '%s'", path, tmp);

        return (tmp);
}

/**
 * canon2gphotopath:
 * @camera: camera to use
 * @path: canon style path
 *
 * convert canon style path (e.g. "D:\DCIM\116CANON\IMG_1240.JPG")
 * into gphoto2 path        (e.g.   "/DCIM/116CANON/IMG_1240.JPG")
 *
 * Assumes @path to start with drive name followed by a colon.
 * It just drops the drive name.
 *
 * Returns: immutable string with gphoto2 path
 *
 */
const char *
canon2gphotopath (Camera *camera, const char *path)
{
        static char tmp[2000];
        char *p;

        if (!((path[1] == ':') && (path[2] == '\\'))) {
                GP_DEBUG ("canon2gphotopath called on invalid canon path '%s'", path);
                return NULL;
        }

        /* 3 is D: plus NULL byte */
        if (strlen (path) - 3 > sizeof (tmp)) {
                GP_DEBUG ("canon2gphotopath called on too long canon path (%li bytes): %s",
                          (long)(strlen (path)), path);
                return NULL;
        }

        /* path is something like D:\FOO, we want what is after the colon */
        strcpy (tmp, path + 2);

        /* replace backslashes by slashes */
        for (p = tmp; *p != '\0'; p++) {
                if (*p == '\\')
                        *p = '/';
        }

        GP_LOG (GP_LOG_DATA, "canon2gphotopath: converted '%s' to '%s'", path, tmp);

        return (tmp);
}

/**
 * debug_fileinfo:
 * @info: information block
 *
 * Writes file info to debug log. Usable only from within "canon.c".
 *
 */
static void
debug_fileinfo (CameraFileInfo * info)
{
        GP_DEBUG ("<CameraFileInfo>");
        GP_DEBUG ("  <CameraFileInfoFile>");
        if ((info->file.fields & GP_FILE_INFO_NAME) != 0)
                GP_DEBUG ("    Name:   %s", info->file.name);
        if ((info->file.fields & GP_FILE_INFO_TYPE) != 0)
                GP_DEBUG ("    Type:   %s", info->file.type);
        if ((info->file.fields & GP_FILE_INFO_SIZE) != 0)
                GP_DEBUG ("    Size:   %i", (int)info->file.size);
        if ((info->file.fields & GP_FILE_INFO_WIDTH) != 0)
                GP_DEBUG ("    Width:  %i", info->file.width);
        if ((info->file.fields & GP_FILE_INFO_HEIGHT) != 0)
                GP_DEBUG ("    Height: %i", info->file.height);
        if ((info->file.fields & GP_FILE_INFO_PERMISSIONS) != 0)
                GP_DEBUG ("    Perms:  0x%x", info->file.permissions);
        if ((info->file.fields & GP_FILE_INFO_STATUS) != 0)
                GP_DEBUG ("    Status: %i", info->file.status);
        if ((info->file.fields & GP_FILE_INFO_MTIME) != 0) {
                char *p, *time = asctime (gmtime (&info->file.mtime));

                /* remove trailing \n */
                for (p = time; *p != 0; ++p)
                        /* do nothing */ ;
                *(p - 1) = '\0';
                GP_DEBUG ("    Time:   %s (%ld)", time, (long)info->file.mtime);
        }
        GP_DEBUG ("  </CameraFileInfoFile>");
        GP_DEBUG ("</CameraFileInfo>");
}

/**
 * canon_int_list_directory:
 * @camera: Camera to access
 * @folder: Folder on the camera to list
 * @list: Returns list of folders in this directory
 * @flags: #canonDirlistFunctionBits specifying to list files, folders, or both
 * @context: context for error reporting
 *
 * Gets the directory tree of a given flash device, unsorted and with
 * a few missing features (such as correct sorting of files and
 * correctly associating files with each other).
 *
 * Implicitly assumes that uint8_t[] is a char[] for strings.
 *
 * A few notes about listing files and camera->pl->list_all_files:
 * 
 *   If camera->pl->list_all_files is false (intended default), then
 *   we list only "primary files" or ("main" files). Those are files
 *   like image files or movie files which the user explicitly created
 *   on the camera. "Secondary files" such as .thm files which contain
 *   the thumbnail of a movie are not listed here - you can get them using
 *   the --get-thumbnail command in gphoto2 or the analog command of your
 *   frontend. Same story with ,wav files containing audio annotations
 *   to primary files.
 *
 *   If camera->pl->list_all_files is true (can be changed via the
 *   configuration interface), then we list _all_ files. This means
 *   that we just use the camera like a CF/SD card reader. Use this mode
 *   to read/write your .pdf files, .tgz backup tarballs, or whatever.
 *   Of course, all the "internal" files the camera puts on the storage
 *   medium will be listed in this mode as well.
 *
 * Returns: a gphoto2 status code.
 *   @list will contain a list of folders (directories) contained in this folder.
 *   Files will be added to the internal gphoto2 file system only if
 *     they are image or movie files.
 *
 */
int
canon_int_list_directory (Camera *camera, const char *folder, CameraList *list,
                          const canonDirlistFunctionBits flags, GPContext *context)
{
        CameraFileInfo info;
        int res;
        unsigned int dirents_length;
        unsigned char *dirent_data = NULL;
        unsigned char *end_of_data, *temp_ch, *pos;
        const char *canonfolder = gphoto2canonpath (camera, folder, context);
        int list_files = ((flags & CANON_LIST_FILES) != 0);
        int list_folders = ((flags & CANON_LIST_FOLDERS) != 0);

        GP_DEBUG ("BEGIN canon_int_list_directory() folder '%s' aka '%s' (%s, %s)", folder,
                  canonfolder, list_files ? "files" : "no files",
                  list_folders ? "folders" : "no folders");

        if ( canonfolder == NULL ) {
                GP_DEBUG ( "Error: canon_int_list_directory called with null name for camera folder" );
                return GP_ERROR;
        }

        /* Fetch all directory entries from the camera */
        switch (camera->port->type) {
                case GP_PORT_USB:
                        res = canon_usb_get_dirents (camera, &dirent_data, &dirents_length,
                                                     canonfolder, context);
                        break;
                case GP_PORT_SERIAL:
                        res = canon_serial_get_dirents (camera, &dirent_data, &dirents_length,
                                                        canonfolder, context);
                        break;
                GP_PORT_DEFAULT
        }
        if (res != GP_OK)
                return res;

        end_of_data = dirent_data + dirents_length;

        if (dirents_length < CANON_MINIMUM_DIRENT_SIZE) {
                gp_context_error (context,
                                  _("canon_int_list_directory: ERROR: "
                                    "initial message too short (%i < minimum %i)"),
                                  dirents_length, CANON_MINIMUM_DIRENT_SIZE);
                free (dirent_data);
                dirent_data = NULL;
                return GP_ERROR_CORRUPTED_DATA;
        }

        /* The first data we have got here is the dirent for the
         * directory we are reading. Skip over 10 bytes
         * (2 for attributes, 4 date and 4 size) and then go find
         * the end of the directory name so that we get to the next
         * dirent which is actually the first one we are interested
         * in
         */
        GP_DEBUG ("canon_int_list_directory: Camera directory listing for directory '%s'",
                  dirent_data + CANON_DIRENT_NAME);

        for (pos = dirent_data + CANON_DIRENT_NAME; pos < end_of_data && *pos != 0; pos++)
                /* do nothing */ ;
        if (pos == end_of_data || *pos != 0) {
                gp_context_error (context,
                                  _("canon_int_list_directory: Reached end of packet while "
                                   "examining the first dirent"));
                free (dirent_data);
                dirent_data = NULL;
                return GP_ERROR_CORRUPTED_DATA;
        }
        pos++;                  /* skip NULL byte terminating directory name */

        /* we are now positioned at the first interesting dirent */

        /* This is the main loop, for every directory entry returned */
        while (pos < end_of_data) {
                int is_dir, is_file;
                uint16_t dirent_attrs;  /* attributes of dirent */
                uint32_t dirent_file_size;      /* size of dirent in octets */
                uint32_t dirent_time;   /* time stamp of dirent (Unix Epoch) */
                uint8_t *dirent_name;   /* name of dirent */
                size_t dirent_name_len; /* length of dirent_name */
                size_t dirent_ent_size; /* size of dirent in octets */
                uint32_t tmp_time;
                time_t date;
                struct tm *tm;

                dirent_attrs = le16atoh (pos + CANON_DIRENT_ATTRS);
                dirent_file_size = le32atoh (pos + CANON_DIRENT_SIZE);
                dirent_name = pos + CANON_DIRENT_NAME;

                /* see canon_int_set_time() for timezone handling */
                tmp_time = le32atoh (pos + CANON_DIRENT_TIME);
                if (tmp_time != 0) {
                        /* FIXME: I just want the tm_gmtoff/timezone info */
                        date = time(NULL);
                        tm   = localtime (&date);
#ifdef HAVE_TM_GMTOFF
                        dirent_time = tmp_time - tm->tm_gmtoff;
                        GP_DEBUG ("canon_int_list_directory: converted %ld to UTC %ld (tm_gmtoff is %ld)",
                                (long)tmp_time, (long)dirent_time, (long)tm->tm_gmtoff);
#else
                        dirent_time = tmp_time + timezone;
                        GP_DEBUG ("canon_int_list_directory: converted %ld to UTC %ld (timezone is %ld)",
                                (long)tmp_time, (long)dirent_time, (long)timezone);
#endif
                } else {
                        dirent_time = tmp_time;
                }

                is_dir = ((dirent_attrs & CANON_ATTR_NON_RECURS_ENT_DIR) != 0)
                        || ((dirent_attrs & CANON_ATTR_RECURS_ENT_DIR) != 0);
                is_file = !is_dir;

                GP_LOG (GP_LOG_DATA,
                        "canon_int_list_directory: "
                        "reading dirent at position %li of %li (0x%lx of 0x%lx)",
                        (long)(pos - dirent_data), (long)(end_of_data - dirent_data),
                        (long)(pos - dirent_data), (long)(end_of_data - dirent_data) );

                if (pos + CANON_MINIMUM_DIRENT_SIZE > end_of_data) {
                        if (camera->port->type == GP_PORT_SERIAL) {
                                /* check to see if it is only NULL bytes left,
                                 * that is not an error for serial cameras
                                 * (at least the A50 adds five zero bytes at the end)
                                 */
                                for (temp_ch = pos; (temp_ch < end_of_data) && (!*temp_ch); temp_ch++) ;        /* do nothing */

                                if (temp_ch == end_of_data) {
                                        GP_DEBUG ("canon_int_list_directory: "
                                                  "the last %li bytes were all 0 - ignoring.",
                                                  (long)(temp_ch - pos));
                                        break;
                                } else {
                                        GP_DEBUG ("canon_int_list_directory: "
                                                  "byte[%li=0x%lx] == %i=0x%x", (long)(temp_ch - pos),
                                                  (long)(temp_ch - pos), *temp_ch, *temp_ch);
                                        GP_DEBUG ("canon_int_list_directory: "
                                                  "pos is %p, end_of_data is %p, temp_ch is %p - diff is 0x%lx",
                                                  pos, end_of_data, temp_ch, (long)(temp_ch - pos));
                                }
                        }
                        GP_DEBUG ("canon_int_list_directory: "
                                  "dirent at position %li=0x%lx of %li=0x%lx is too small, "
                                  "minimum dirent is %i bytes",
                                  (long)(pos - dirent_data), (long)(pos - dirent_data),
                                  (long)(end_of_data - dirent_data), (long)(end_of_data - dirent_data),
                                  CANON_MINIMUM_DIRENT_SIZE);
                        gp_context_error (context,
                                          _("canon_int_list_directory: "
                                           "truncated directory entry encountered"));
                        free (dirent_data);
                        dirent_data = NULL;
                        return GP_ERROR_CORRUPTED_DATA;
                }

                /* Check end of this dirent, 10 is to skip over
                 * 2    attributes + 0x00
                 * 4    file size
                 * 4    file date (UNIX localtime)
                 * to where the direntry name begins.
                 */
                for (temp_ch = dirent_name; temp_ch < end_of_data && *temp_ch != 0;
                     temp_ch++) ;

                if (temp_ch == end_of_data || *temp_ch != 0) {
                        GP_DEBUG ("canon_int_list_directory: "
                                  "dirent at position %li of %li has invalid name in it."
                                  "bailing out with what we've got.",
                                  (long)(pos - dirent_data),
                                  (long)(end_of_data - dirent_data));
                        break;
                }
                dirent_name_len = strlen (dirent_name);
                dirent_ent_size = CANON_MINIMUM_DIRENT_SIZE + dirent_name_len;

                /* check that length of name in this dirent is not of unreasonable size.
                 * 256 was picked out of the blue
                 */
                if (dirent_name_len > 256) {
                        GP_DEBUG ("canon_int_list_directory: "
                                  "the name in dirent at position %li of %li is too long. (%li bytes)."
                                  "bailing out with what we've got.",
                                  (long)(pos - dirent_data), (long)(end_of_data - dirent_data),
                                  (long)dirent_name_len);
                        break;
                }

                /* 10 bytes of attributes, size and date, a name and a NULL terminating byte */
                /* don't use GP_DEBUG since we log this with GP_LOG_DATA */
                GP_LOG (GP_LOG_DATA,
                        "canon_int_list_directory: dirent determined to be %li=0x%lx bytes :",
                        (long)dirent_ent_size, (long)dirent_ent_size);
                gp_log_data ("canon", pos, dirent_ent_size);
                if (dirent_name_len) {
                        /* OK, this directory entry has a name in it. */

                        if ((list_folders && is_dir) || (list_files && is_file)) {

                                /* we're going to fill out the info structure
                                   in this block */
                                memset (&info, 0, sizeof (info));

                                /* we start with nothing and continously add stuff */
                                info.file.fields = GP_FILE_INFO_NONE;

                                strncpy (info.file.name, dirent_name, sizeof (info.file.name));
                                info.file.fields |= GP_FILE_INFO_NAME;

                                info.file.mtime = dirent_time;
                                if (info.file.mtime != 0)
                                        info.file.fields |= GP_FILE_INFO_MTIME;

                                if (is_file) {
                                        /* determine file type based on file name
                                         * this stuff only makes sense for files, not for folders
                                         */

                                        strncpy (info.file.type,
                                                 filename2mimetype (info.file.name),
                                                 sizeof (info.file.type));
                                        info.file.fields |= GP_FILE_INFO_TYPE;

                                        if ((dirent_attrs & CANON_ATTR_DOWNLOADED) == 0)
                                                info.file.status = GP_FILE_STATUS_DOWNLOADED;
                                        else
                                                info.file.status =
                                                        GP_FILE_STATUS_NOT_DOWNLOADED;
                                        info.file.fields |= GP_FILE_INFO_STATUS;

                                        /* the size is located at offset 2 and is 4
                                         * bytes long, re-order little/big endian */
                                        info.file.size = dirent_file_size;
                                        info.file.fields |= GP_FILE_INFO_SIZE;

                                        /* file access modes */
                                        if ((dirent_attrs & CANON_ATTR_WRITE_PROTECTED) == 0)
                                                info.file.permissions =
                                                        GP_FILE_PERM_READ |
                                                        GP_FILE_PERM_DELETE;
                                        else
                                                info.file.permissions = GP_FILE_PERM_READ;
                                        info.file.fields |= GP_FILE_INFO_PERMISSIONS;
                                }

                                /* print dirent as text */
                                GP_DEBUG ("Raw info: name=%s is_dir=%i, is_file=%i, attrs=0x%x",
                                          dirent_name, is_dir, is_file, dirent_attrs);
                                debug_fileinfo (&info);

                                if (is_file) {
                                        /*
                                         * Append directly to the filesystem instead of to the list,
                                         * because we have additional information.
                                         */
                                        if (!camera->pl->list_all_files
                                            && !is_image (info.file.name)
                                            && !is_movie (info.file.name)
                                            && !is_audio (info.file.name)) {
                                                /* FIXME: Find associated main file and add it there */
                                                /* do nothing */
                                                GP_DEBUG ("Ignored %s/%s", folder,
                                                          info.file.name);
                                        } else {
                                                const char *thumbname;

                                                res = gp_filesystem_append (camera->fs, folder,
                                                                      info.file.name, context);
                                                if (res != GP_OK) {
                                                        GP_DEBUG ("Could not gp_filesystem_append "
                                                                  "%s in folder %s: %s",
                                                                  info.file.name, folder, gp_result_as_string (res));
                                                } else {
                                                        GP_DEBUG ("Added file %s/%s", folder,
                                                                  info.file.name);

                                                        thumbname =
                                                                canon_int_filename2thumbname (camera,
                                                                                              info.file.name);
                                                        if (thumbname == NULL) {
                                                                /* no thumbnail */
                                                        } else {
                                                                if ( is_cr2 ( info.file.name ) ) {
                                                                        /* We get the first part of the raw file as the thumbnail;
                                                                           this is (almost) a valid EXIF file. */
                                                                        info.preview.fields =
                                                                                GP_FILE_INFO_TYPE;
                                                                        strncpy (info.preview.type,
                                                                                 GP_MIME_EXIF,
                                                                                 sizeof (info.preview.type));
                                                                }
                                                                else {
                                                                        /* Older Canon cams have JPEG thumbs */
                                                                        info.preview.fields =
                                                                                GP_FILE_INFO_TYPE;
                                                                        strncpy (info.preview.type,
                                                                                 GP_MIME_JPEG,
                                                                                 sizeof (info.preview.type));
                                                                }
                                                        }

                                                        res = gp_filesystem_set_info_noop (camera->fs,
                                                                                     folder, info,
                                                                                     context);
                                                        if (res != GP_OK) {
                                                                GP_DEBUG ("Could not gp_filesystem_set_info_noop() "
                                                                          "%s in folder %s: %s",
                                                                          info.file.name, folder, gp_result_as_string (res));
                                                        }
                                                }
                                                GP_DEBUG ( "file \"%s\" has preview of MIME type \"%s\"\n",
                                                           info.file.name, info.preview.type );
                                        }
                                }
                                /* Some cameras have ".." explicitly
                                 * at the end of each directory. We
                                 * will silently omit this from the
                                 * directory returned. */
                                if ( is_dir && strcmp ( "..", info.file.name ) ) {
                                        res = gp_list_append (list, info.file.name, NULL);
                                        if (res != GP_OK)
                                                GP_DEBUG ("Could not gp_list_append "
                                                          "folder %s: %s",
                                                          folder, gp_result_as_string (res));
                                }
                        } else {
                                /* this case could mean that this was the last dirent */
                                GP_DEBUG ("canon_int_list_directory: "
                                          "dirent at position %li of %li has NULL name, skipping.",
                                          (long)(pos - dirent_data), (long)(end_of_data - dirent_data));
                        }
                }

                /* make 'pos' point to next dirent in packet.
                 * first we skip 10 bytes of attribute, size and date,
                 * then we skip the name plus 1 for the NULL
                 * termination bytes.
                 */
                pos += dirent_ent_size;
        }
        free (dirent_data);
        dirent_data = NULL;

        GP_DEBUG ("<FILESYSTEM-DUMP>");
        gp_filesystem_dump (camera->fs);
        GP_DEBUG ("</FILESYSTEM-DUMP>");

        GP_DEBUG ("END canon_int_list_dir() folder '%s' aka '%s'", folder, canonfolder);

        return GP_OK;
}

/**
 * canon_int_get_file:
 * @camera:
 * @name: name of file to fetch
 * @data: to receive file data
 * @length: length of @data
 * @context: context for error reporting
 *
 * Gets the directory tree of a given flash device.
 *
 * Returns: gphoto2 error code, file contents in @data,
 *  length of file in @length.
 *
 */
int
canon_int_get_file (Camera *camera, const char *name, unsigned char **data, int *length,
                    GPContext *context)
{
        switch (camera->port->type) {
                case GP_PORT_USB:
                        return canon_usb_get_file (camera, name, data, length, context);
                        break;
                case GP_PORT_SERIAL:
                        *data = canon_serial_get_file (camera, name, length, context);
                        if (*data)
                                return GP_OK;
                        return GP_ERROR_OS_FAILURE;
                        break;
                GP_PORT_DEFAULT
        }
}

/**
 * canon_int_get_thumbnail:
 * @camera: camera to work with
 * @name: image to get thumbnail of
 * @length: length of data returned
 * @retdata: The thumbnail data
 * @context: context for error reporting
 *
 * NOTE: Since cameras that do not store the thumbnail in a separate
 * file does not return just the thumbnail but the first 10813 bytes
 * of the image (most oftenly the EXIF header with thumbnail data in
 * it) this must be treated before called a true thumbnail.
 *
 * Returns: result from canon_usb_get_thumbnail()
 *   or canon_serial_get_thumbnail()
 *
 */
int
canon_int_get_thumbnail (Camera *camera, const char *name, unsigned char **retdata,
                         int *length, GPContext *context)
{
        int res;

        GP_DEBUG ("canon_int_get_thumbnail() called for file '%s'", name);

        CON_CHECK_PARAM_NULL (retdata);
        CON_CHECK_PARAM_NULL (length);

        switch (camera->port->type) {
                case GP_PORT_USB:
                        res = canon_usb_get_thumbnail (camera, name, retdata, length, context);
                        break;
                case GP_PORT_SERIAL:
                        res = canon_serial_get_thumbnail (camera, name, retdata, length,
                                                          context);
                        break;
                GP_PORT_DEFAULT
        }
        if (res != GP_OK) {
                GP_DEBUG ("canon_int_get_thumbnail() failed, returned %i", res);
                return res;
        }

        return res;
}

/**
 * canon_int_delete_file:
 * @camera: camera to work with
 * @name: image to delete
 * @dir: directory from which to delete the file
 * @context: context for error reporting
 *
 * Deletes a file from the camera storage.
 *
 * Returns: result from canon_usb_dialogue() or canon_serial_dialogue()
 *
 */
int
canon_int_delete_file (Camera *camera, const char *name, const char *dir, GPContext *context)
{
        unsigned char payload[300];
        unsigned char *msg;
        int len, payload_length;

        switch (camera->port->type) {
                case GP_PORT_USB:
                        //modif michel
                        //memcpy (payload, dir, strlen (dir) + 1);
                        strcpy( payload, dir);
                        if ( payload[strlen (dir)] != '\\' ) {
                           strcat(payload, "\\");
                        }
                        strcat(payload, name);
                        payload_length = strlen (payload) + 1;
                        payload[payload_length++] = 0; /* Double NUL to end command */

                        if ( camera->pl->md->model == CANON_CLASS_6 ) {
                                char *ptr = payload + strlen(dir);
                                char last_byte = dir[strlen(dir)-1];
                                /* Newer protocol uses a different
                                 * code and has different parameters:
                                 * - full path name together, rather than directory and file
                                 *   as separate strings
                                 * - 8 bytes of other stuff, starting at 0x20 in
                                 *   payload
                                 * - directory name (again)
                                 * - 2 null bytes */

                                // modif michel
                                //if ( last_byte != '\\' && last_byte != '/' )
                                //        *ptr++ = '\\'; /* Need path separator between */
                                //memcpy ( ptr, name, 0x30 - strlen(dir) - 1 );
                                //memcpy ( payload + 0x30, dir, 0x30 );
                                //payload_length = 0x30 + strlen(dir);
                                //if ( last_byte != '\\' && last_byte != '/' )
                                //        payload[payload_length++] = '\\'; /* Need path separator at end of directory name */
                                
                                //memcpy (payload + strlen (dir) , "\\", 1);
                                //memcpy (payload + strlen (dir) + 1, name, strlen (name) + 1);
                                //payload_length = strlen (dir) + strlen (name) + 2;
                                //payload[payload_length++] = 0; /* Double NUL to end command */

                                msg = canon_usb_dialogue (camera, CANON_USB_FUNCTION_DELETE_FILE_2, &len,
                                                          payload, payload_length);
                        }
                        else {
                                
                                //memcpy (payload + strlen (dir) + 1, name, strlen (name) + 1);
                                msg = canon_usb_dialogue (camera, CANON_USB_FUNCTION_DELETE_FILE, &len,
                                                          payload, payload_length);
                        }
                        if ( msg == NULL )
                                return GP_ERROR_OS_FAILURE;
                        else if ( le32atoh ( msg ) != 0 ) {
                                GP_DEBUG ( "canon_int_delete_file: non-zero return code 0x%x from camera.\n   Possibly tried to delete a nonexistent file.", le32atoh ( msg ) );
                                return GP_ERROR_FILE_NOT_FOUND;
                        }

                        break;
                case GP_PORT_SERIAL:
                        msg = canon_serial_dialogue (camera, context, 0xd, 0x11, &len, dir,
                                                     strlen (dir) + 1, name, strlen (name) + 1,
                                                     NULL);
                        if ( msg == NULL ) {
                                canon_serial_error_type (camera);
                                return GP_ERROR_OS_FAILURE;
                        }
                        break;
                GP_PORT_DEFAULT
        }

        if (len != 4) {
                /* XXX should mark folder as dirty since we can't be sure if the file
                 * got deleted or not
                 */
                return GP_ERROR_CORRUPTED_DATA;
        }

        if (msg[0] == 0x29) {
                gp_context_error (context, _("File protected."));
                return GP_ERROR_CAMERA_ERROR;
        }

        /* XXX we should mark folder as dirty, re-read it and check if the file
         * is gone or not.
         */
        return GP_OK;
}

/**
 * canon_int_put_file:
 * @camera: camera to work with
 * @file: gphoto2 file object to upload
 * @destname: name for file on camera
 * @destpath: directory in which to put the file
 * @context: context for error reporting
 *
 * Uploads a file to the camera.
 *
 * Returns: result from canon_usb_put_file() or canon_serial_put_file()
 *
 */
int
canon_int_put_file (Camera *camera, CameraFile *file, char *destname, char *destpath,
                    GPContext *context)
{
        switch (camera->port->type) {
                case GP_PORT_USB:
                        return canon_usb_put_file (camera, file, destname, destpath,
                                                      context);
                        break;
                case GP_PORT_SERIAL:
                        return canon_serial_put_file (camera, file, destname, destpath,
                                                      context);
                        break;
                GP_PORT_DEFAULT
        }
        /* Never reached */
        return GP_ERROR;
}

/**
 * canon_int_extract_jpeg_thumb:
 * @data: raw JFIF from which to extract thumbnail
 * @datalen: length of @data
 * @retdata: to receive extracted thumbnail
 * @retdatalen: length of @retdata
 * @context: context for error reporting
 *
 * Extracts thumbnail from JFIF image (A70) or movie .thm file.
 * just extracted the code from the old #canon_int_get_thumbnail
 *
 * Returns: gphoto2 error code
 *
 */

int
canon_int_extract_jpeg_thumb (unsigned char *data, const unsigned int datalen,
                             unsigned char **retdata, unsigned int *retdatalen,
                             GPContext *context)
{
        unsigned int i, thumbstart = 0, thumbsize = 0;

        CHECK_PARAM_NULL (data);
        CHECK_PARAM_NULL (retdata);

        *retdata = NULL;
        *retdatalen = 0;

        if (data[0] == JPEG_ESC || data[1] == JPEG_BEG) {
                GP_DEBUG ("canon_int_extract_jpeg_thumb: this is a JFIF file.");

                /* pictures are JFIF files, we skip the first 2 bytes (0xFF 0xD8)
                 * first go look for start of JPEG, when that is found we set thumbstart
                 * to the current position and never look for JPEG begin bytes again.
                 * when thumbstart is set look for JPEG end.
                 */
                for (i = 3; i < datalen; i++)
                        if (data[i] == JPEG_ESC) {
                                if (! thumbstart) {
                                        if (i < (datalen - 3) &&
                                            data[i + 1] == JPEG_BEG &&
                                            ((data[i + 3] == JPEG_SOS) || (data[i + 3] == JPEG_A50_SOS)))
                                                thumbstart = i;
                                } else if (i < (datalen - 1) && (data[i + 1] == JPEG_END)) {
                                        thumbsize = i + 2 - thumbstart;
                                        break;
                                }

                        }
                if (! thumbsize) {
                        gp_context_error (context, _("Could not extract JPEG "
                                                     "thumbnail from data: No beginning/end"));
                        GP_DEBUG ("canon_int_extract_jpeg_thumb: could not find JPEG "
                                  "beginning (offset %i) or end (size %i) in %i bytes of data",
                                  datalen, thumbstart, thumbsize);
                        return GP_ERROR_CORRUPTED_DATA;
                }

                /* now that we know the size of the thumbnail embedded in the JFIF data, malloc() */
                *retdata = malloc (thumbsize);
                if (! *retdata) {
                        GP_DEBUG ("canon_int_extract_jpeg_thumb: could not allocate %i bytes of memory", thumbsize);
                        return GP_ERROR_NO_MEMORY;
                }

                /* and copy */
                memcpy (*retdata, data + thumbstart, thumbsize);
                *retdatalen = thumbsize;
        }
        else if ( !strcmp ( data, "II*" ) && data[8] == 'C' && data[9] == 'R' ) {

                /* This is a valid EXIF file; we need to sort through
                 * to get the JPEG thumbnail. */
#ifdef HAVE_LIBEXIF
                /* We have to do this ourselves, because libexif
                 * assumes that the EXIF info is encapsulated in an
                 * APP1 marker in a JFIF file. So we parse enough to
                 * get the thumbnail, which is found via IFD 1. */
                /* FIXME: We need to extract the EXIF IFD and copy it
                 * into the image we return, as higher levels of
                 * software assume that the EXIF is included in the
                 * JPEG thumbnail and just fetch the thumbnail to get
                 * the EXIF data. */
                int ifd0_offset, ifd1_offset, n_tags;
                int jpeg_offset = -1, jpeg_size = -1, i;

                GP_DEBUG ( "canon_int_extract_jpeg_thumb: this is from a CR2 file.");
                dump_hex ( stderr, data, 32 );
                ifd0_offset = exif_get_long ( data+4, EXIF_BYTE_ORDER_INTEL );
                GP_DEBUG ( "canon_int_extract_jpeg_thumb: IFD 0 at 0x%x\n", ifd0_offset );
                n_tags = exif_get_short ( data+ifd0_offset, EXIF_BYTE_ORDER_INTEL );
                GP_DEBUG ( "canon_int_extract_jpeg_thumb: %d tags in IFD 0\n", n_tags );
                ifd1_offset = exif_get_long ( data + ifd0_offset + 2 + 12*n_tags, EXIF_BYTE_ORDER_INTEL );
                GP_DEBUG ( "canon_int_extract_jpeg_thumb: IFD 1 at 0x%x\n", ifd1_offset );
                n_tags = exif_get_short ( data+ifd1_offset, EXIF_BYTE_ORDER_INTEL );
                GP_DEBUG ( "canon_int_extract_jpeg_thumb: %d tags in IFD 1\n", n_tags );

                /* Now go through IFD 1 and find the two tags we need. */
                for ( i=0; i<n_tags; i++ ) {
                        unsigned char *entry = data+ifd1_offset + 2 + 12*i;
                        short tag = exif_get_short ( entry, EXIF_BYTE_ORDER_INTEL );
                        GP_DEBUG ( "canon_int_extract_jpeg_thumb: tag %d is %s\n",
                                   i, exif_tag_get_name ( tag ) );
                        switch ( tag ) {
                        case EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH:
                                jpeg_size = exif_get_long ( entry + 8, EXIF_BYTE_ORDER_INTEL );
                                GP_DEBUG ( "canon_int_extract_jpeg_thumb: JPEG length is %d\n",
                                           jpeg_size );
                                break;
                        case EXIF_TAG_JPEG_INTERCHANGE_FORMAT:
                                jpeg_offset = exif_get_long ( entry + 8, EXIF_BYTE_ORDER_INTEL );
                                GP_DEBUG ( "canon_int_extract_jpeg_thumb: JPEG offset is 0x%x\n",
                                           jpeg_offset );
                                break;
                        default:
                                break;
                        }
                }
                if ( jpeg_size < 0 || jpeg_offset < 0 ) {
                        GP_DEBUG ( "canon_int_extract_jpeg_thumb: missing a required tag: length=%d, offset=%d\n",
                                   jpeg_size, jpeg_offset );
                        return GP_ERROR_CORRUPTED_DATA;
                }

                /* Now we should have enough to extract the thumbnail. */
                GP_DEBUG ( "canon_int_extract_jpeg_thumb: %d bytes of JPEG image\n",
                            jpeg_size );
                *retdatalen = jpeg_size;
                *retdata = malloc ( *retdatalen );
                memcpy ( *retdata, data + jpeg_offset, *retdatalen );
                dump_hex ( stderr, *retdata, 32 );
#else /* HAVE_LIBEXIF */
                GP_DEBUG ( "canon_int_extract_jpeg_thumb: Can't grok thumbnail from a CR2 file without libexif");
                return GP_ERROR_NOT_SUPPORTED;
#endif /* HAVE_LIBEXIF */
        }
        else {
                gp_context_error (context, _("Could not extract JPEG "
                                             "thumbnail from data: Data is not JFIF"));
                GP_DEBUG ("canon_int_extract_jpeg_thumb: data is not JFIF, cannot extract thumbnail");
                return GP_ERROR_CORRUPTED_DATA;
        }

        return GP_OK;
}

int
canon_int_get_release_parameters (Camera *camera, GPContext *context) {
   unsigned char *result_block;
   int subcmd;
   char payload[0x4c];
   char desc[128];
   int payloadlen;
   int datalen = 0;
   int status;

   switch (camera->port->type) {
   case GP_PORT_USB:
      //gp_port_get_timeout (camera->port, &mstimeout);
      //GP_DEBUG("canon_int_capture_preview: usb port timeout starts at %dms", mstimeout);
      gp_port_set_timeout (camera->port, 15000);
      status = canon_int_start_remote_control (camera, context);
      if ( status < 0 )
         return status;
  
      subcmd = CANON_USB_CONTROL_GET_PARAMS;
      payloadlen = canon_int_pack_control_subcmd(payload, subcmd, 0, 0, desc);

  
      if ( camera->pl->md->model == CANON_CLASS_6 ) {
          // Newer protocol uses a different code, but with same
          // response. It also needs an extra zero byte at the end. 
          payload[payloadlen++] = 0;
          result_block = canon_usb_dialogue ( camera, CANON_USB_FUNCTION_CONTROL_CAMERA_2,
                                     &datalen, payload, payloadlen );
      } else { 
          result_block = canon_usb_dialogue ( camera, CANON_USB_FUNCTION_CONTROL_CAMERA,
                                     &datalen, payload, payloadlen );
      }
      if ( result_block != NULL ) {  
         int i = 0;
         int shutterIndex;
         int driveMode;
         int quality;
         
         // get shutter value
         shutterIndex = le16atoh (result_block + 0x2A);
         i = 0;
         while ( canonShutterValues[i].value != 0 ) {
            if( canonShutterValues[i].value == shutterIndex ) {
               camera->pl->timeValue = canonShutterValues[i].fvalue;
               break;
            }
            i++;
         }
         if( canonShutterValues[i].value == 0 ) {
            camera->pl->timeValue = 1;
         }

         // get drive mode
         driveMode = le16atoh (result_block + 0x16);
         i = 0;
         while ( canonDriveMode[i].value != 0 ) {
            if( canonDriveMode[i].value == driveMode ) {
               camera->pl->driveMode = (int) canonDriveMode[i].fvalue;
               break;
            }
            i++;
         }
         if( canonDriveMode[i].value == 0 ) {
            camera->pl->driveMode = 0;
         }

         // get quality
         quality = le32atoh (result_block + 0x0C);
         i = 0;
         while ( canonValueQuality[i].label != NULL) {
            if( canonValueQuality[i].value == quality ) {
               strcpy(camera->pl->quality, canonValueQuality[i].label);
               break;
            }
            i++;
         }
         if( canonValueQuality[i].value == 0 ) {
             strcpy(camera->pl->quality, canonValueQuality[0].label);
         }

         GP_DEBUG ("canon_int_get_release_parameters  driveMode   = %4X ivalue= %d", driveMode,  camera->pl->driveMode );
         GP_DEBUG ("canon_int_get_release_parameters  shutterIndex = %4X fvalue= %f", shutterIndex, camera->pl->timeValue );
         GP_DEBUG ("canon_int_get_release_parameters  quality = %4X svalue= %s", quality, camera->pl->quality);

      }
                    
      canon_int_end_remote_control (camera, context);
      if ( status < 0 )
         return status;      
      break;

   case GP_PORT_SERIAL:
      return GP_ERROR_NOT_SUPPORTED;
      break;
      GP_PORT_DEFAULT
   }

   return GP_OK;
}


int
canon_int_set_release_parameters (Camera *camera, GPContext *context) {
   unsigned char *result_block;
   int subcmd;
   char payload[0x4c];
   char desc[128];
   int payloadlen;
   int datalen = 0;
   int status;
   int i = 0;
   int shutterIndex = 0;
   int driveMode = 0;
   int quality = 0;


   GP_DEBUG ("canon_int_set_release_parameters()");

   // find shutterIndex 
   i = 0;
   while ( canonShutterValues[i].value != 0 ) {
      if( canonShutterValues[i].fvalue <= camera->pl->timeValue  ) {
         shutterIndex = canonShutterValues[i].value;
         break;
      }
      i++;
   }
   if( canonShutterValues[i].value == 0 ) {
        return GP_ERROR_BAD_PARAMETERS;
   }

   // find driveModeIndex 
   i = 0;
   while ( canonDriveMode[i].value != 0 ) {
      if( canonDriveMode[i].fvalue == (float) camera->pl->driveMode  ) {
         driveMode = canonDriveMode[i].value;
         break;
      }
      i++;
   }
   if( canonDriveMode[i].value == 0 ) {
        return GP_ERROR_BAD_PARAMETERS;
   }

   // find quality
   i = 0;
   while ( canonValueQuality[i].label != NULL ) {
      if( strcmp(canonValueQuality[i].label, camera->pl->quality) == 0  ) {
         quality = canonValueQuality[i].value;
         break;
      }
      i++;
   }
   if( canonValueQuality[i].value == 0 ) {
        return GP_ERROR_BAD_PARAMETERS;
   }




   switch (camera->port->type) {
   case GP_PORT_USB:
      
      //gp_port_get_timeout (camera->port, &mstimeout);
      //GP_DEBUG("canon_int_capture_preview: usb port timeout starts at %dms", mstimeout);
      gp_port_set_timeout (camera->port, 15000);
      status = canon_int_start_remote_control (camera, context);
      if ( status < 0 )
         return status;
  
      // get current release param
      subcmd = CANON_USB_CONTROL_GET_PARAMS;
      payloadlen = canon_int_pack_control_subcmd(payload, subcmd, 0, 0, desc);
      if ( camera->pl->md->model == CANON_CLASS_6 ) {
          // Newer protocol uses a different code, but with same
          // response. It also needs an extra zero byte at the end. 
          payload[payloadlen++] = 0;
          result_block = canon_usb_dialogue ( camera, CANON_USB_FUNCTION_CONTROL_CAMERA_2,
                                     &datalen, payload, payloadlen );
      } else { 
          result_block = canon_usb_dialogue ( camera, CANON_USB_FUNCTION_CONTROL_CAMERA,
                                     &datalen, payload, payloadlen );
      }

      if ( result_block != NULL ) {  

         // update payload
         subcmd = CANON_USB_CONTROL_SET_PARAMS;
         payloadlen = canon_int_pack_control_subcmd(payload, subcmd, 0, 0, desc);
         // copy 
         memcpy(payload + 0x04, result_block + 0x08, 0x34);
         // set shutter index
         htole16a(payload+0x26, shutterIndex);
         // set drive mode
         htole16a(payload+0x12, driveMode);
         // set quality
         htole32a(payload+0x08, quality);
         
         if ( camera->pl->md->model == CANON_CLASS_6 ) {
            payload[payloadlen++] = 0;
            result_block = canon_usb_dialogue ( camera, CANON_USB_FUNCTION_CONTROL_CAMERA_2,
                                     &datalen, payload, payloadlen );
         } else { 
            result_block = canon_usb_dialogue ( camera, CANON_USB_FUNCTION_CONTROL_CAMERA,
                                     &datalen, payload, payloadlen );
         }         
      }
                    
      status = canon_int_end_remote_control (camera, context);
      if ( status < 0 )
         return status;      
      break;

   case GP_PORT_SERIAL:
      return GP_ERROR_NOT_SUPPORTED;
      break;
      GP_PORT_DEFAULT
   }

   return GP_OK;
}

/*
 * Local Variables:
 * c-file-style:"linux"
 * indent-tabs-mode:nil
 * End:
 */
