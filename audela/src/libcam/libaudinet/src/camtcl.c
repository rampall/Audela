/* camtcl.c
 *
 * This file is part of the AudeLA project : <http://software.audela.free.fr>
 * Copyright (C) 1998-2004 The AudeLA Core Team
 *
 * Initial author : Michel PUJOL <michel-pujol@wanadoo.fr>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Fonctions C-Tcl specifiques a cette camera. A programmer.
 *
 * $Id: camtcl.c,v 1.6 2010-02-06 11:25:17 michelpujol Exp $
 */

#include "sysexp.h"

#if defined(OS_WIN)
#include <windows.h>
#endif
#if defined(OS_LIN) || defined(OS_MACOS)
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "camera.h"
#include <libcam/libcam.h>

#include "scanstruct.h"
#include "camtcl.h"
#include <libcam/util.h>
#include "log.h"
#include "contstruct.h"
#include "setip.h"


extern struct camini CAM_INI[];

#define STRNCPY(_d,_s)  strncpy(_d,_s,sizeof _d) ; _d[sizeof _d-1] = 0

/* ----- defines specifiques aux fonctions de cette camera ----*/
static void ScanCallback(ClientData clientData);
static void ScanLibereStructure();
static void ScanTerminateSequence(ClientData clientData, int camno,
				  char *reason);
static void ScanTransfer(ClientData clientData);
// static int UpdateDisplay(ContinuousStruct * continuousData);


/* --- Global variable for SCAN acquisition mode ---*/
static ScanStruct *TheScanStruct = NULL;

int cmdAudinetWipe(ClientData clientData, Tcl_Interp * interp, int argc,
		   char *argv[])
/*
 * -----------------------------------------------------------------------------
 *  cmdAudinetWipe()
 *
 * Wipe the CCD matrix
 *
 * -----------------------------------------------------------------------------
 */
{
    struct camprop *cam;


    cam = (struct camprop *) clientData;

    logInfo("cmdAudinetWipe begin");

    CAM_DRV.start_exp(cam, "amplioff");
    libcam_GetCurrentFITSDate(interp, cam->date_obs);
    logInfo("cmdAudinetWipe end");

    return TCL_OK;
}

/*
 * -----------------------------------------------------------------------------
 *  cmdAudinetRead()
 *
 * Read the CCD matrix immediatly, no clear
 *
 * -----------------------------------------------------------------------------
 */
int cmdAudinetRead(ClientData clientData, Tcl_Interp * interp, int argc,
                   char *argv[])
{
   struct camprop *cam;
   int naxis1, naxis2, bin1, bin2;
   char s[100];
   unsigned short *p;
   double ra, dec, exptime = 0.;
   float *pp;
   int t, status;
   
   cam = (struct camprop *) clientData;
   logInfo("cmdAudinetRead begin");
   
   /* Le code suivant est tire de AcqRead anciennement dans libcam.c */
   // AcqRead((ClientData) cam);
   
   interp = cam->interp;
   naxis1 = cam->w;
   naxis2 = cam->h;
   bin1 = cam->binx;
   bin2 = cam->biny;
   
   p = (unsigned short *) calloc(naxis1 * naxis2, sizeof(unsigned short));
   
   libcam_GetCurrentFITSDate(interp, cam->date_end);
   CAM_DRV.read_ccd(cam, p);
   
   /*
   * Ce test permet de savoir si le buffer existe
   */
   sprintf(s, "buf%d bitpix", cam->bufno);
   if (Tcl_Eval(interp, s) == TCL_ERROR) {
      /* Creation du buffer */
      sprintf(s, "buf::create %d", cam->bufno);
      Tcl_Eval(interp, s);
   }
   
   sprintf(s, "buf%d format %d %d", cam->bufno, naxis1, naxis2);
   Tcl_Eval(interp, s);
   sprintf(s, "buf%d pointer", cam->bufno);
   Tcl_Eval(interp, s);
   
   pp = (float *) atoi(interp->result);
   
   t = naxis1 * naxis2;
   while (--t >= 0)
      *(pp + t) = (float) *((unsigned short *) (p + t));
   
   sprintf(s, "buf%d bitpix ushort", cam->bufno);
   Tcl_Eval(interp, s);
   
   /* Add FITS keywords */
   sprintf(s, "buf%d setkwd {NAXIS 2 int \"\" \"\"}", cam->bufno);
   sprintf(s, "buf%d setkwd {NAXIS1 %d int \"\" \"\"}", cam->bufno,
      naxis1);
   Tcl_Eval(interp, s);
   sprintf(s, "buf%d setkwd {NAXIS2 %d int \"\" \"\"}", cam->bufno,
      naxis2);
   Tcl_Eval(interp, s);
   sprintf(s, "buf%d setkwd {BIN1 %d int \"\" \"\"}", cam->bufno, bin1);
   Tcl_Eval(interp, s);
   sprintf(s, "buf%d setkwd {BIN2 %d int \"\" \"\"}", cam->bufno, bin2);
   Tcl_Eval(interp, s);
   sprintf(s, "buf%d setkwd {CAMERA \"%s %s %s\" string \"\" \"\"}",
      cam->bufno, CAM_INI[cam->index_cam].name,
      CAM_INI[cam->index_cam].ccd, CAM_LIBNAME);
   Tcl_Eval(interp, s);
   sprintf(s, "buf%d setkwd {DATE-OBS %s string \"\" \"\"}",
      cam->bufno, cam->date_obs);
   Tcl_Eval(interp, s);
   if (cam->timerExpiration != NULL) {
      sprintf(s, "buf%d setkwd {EXPOSURE %f float \"\" \"s\"}",
         cam->bufno, cam->exptime);
   } else {
      sprintf(s, "expr (([mc_date2jd %s]-[mc_date2jd %s])*86400.)",
         cam->date_end, cam->date_obs);
      Tcl_Eval(interp, s);
      exptime = atof(interp->result);
      sprintf(s, "buf%d setkwd {EXPOSURE %f float \"\" \"s\"}",
         cam->bufno, exptime);
   }
   Tcl_Eval(interp, s);
   
   libcam_get_tel_coord(interp, &ra, &dec, cam, &status);
   if (status == 0) {
      /* Add FITS keywords */
      sprintf(s,
         "buf%d setkwd {RA %f float \"Right ascension telescope encoder\" \"\"}",
         cam->bufno, ra);
      Tcl_Eval(interp, s);
      sprintf(s,
         "buf%d setkwd {DEC %f float \"Declination telescope encoder\" \"\"}",
         cam->bufno, dec);
      Tcl_Eval(interp, s);
   }
   //if (cam->timerExpiration != NULL) {
   //   sprintf(s, "status_cam%d", cam->camno);
   //}
   //Tcl_SetVar(interp, s, "stand", TCL_GLOBAL_ONLY);
   setCameraStatus(cam,interp,"stand");
   cam->clockbegin = 0;
   
   if (cam->timerExpiration != NULL) {
      //	free(cam->timerExpiration->dateobs);
      free(cam->timerExpiration);
      cam->timerExpiration = NULL;
   }
   free(p);
   
   /* Fin du code AcqRead */
   
   logInfo("cmdAudinetRead end");
   
   return TCL_OK;
}


/*
 * -----------------------------------------------------------------------------
 *  cmdAudinetHost()
 *
 * Change or returns the IP host 
 * 
 * -----------------------------------------------------------------------------
 */
int cmdAudinetHost(ClientData clientData, Tcl_Interp * interp, int argc,
		   char *argv[])
{
    char ligne[256];
    int result = TCL_OK, pb = 0;
    struct camprop *cam;
    cam = (struct camprop *) clientData;
    if ((argc != 2) && (argc != 3)) {
	pb = 1;
    } else if (argc == 2) {
	pb = 0;
    } else {
	//cam->port=(unsigned short)atoi(argv[2]);
	STRNCPY(cam->host, argv[2]);
	pb = 0;
    }
    if (pb == 1) {
	sprintf(ligne, "Usage: %s %s ?host?", argv[0], argv[1]);
	Tcl_SetResult(interp, ligne, TCL_VOLATILE);
	result = TCL_ERROR;
    } else {
	strcpy(ligne, "");
	sprintf(ligne, "%s", cam->host);
	Tcl_SetResult(interp, ligne, TCL_VOLATILE);
    }
    return result;
}



/*
 * -----------------------------------------------------------------------------
 *  cmdAudinetAmpli()
 *
 * Setect the synchronisation of the CCD amplifier
 *
 * -----------------------------------------------------------------------------
 */
int cmdAudinetAmpli(ClientData clientData, Tcl_Interp * interp, int argc,
		    char *argv[])
{
    char ligne[256];
    int result = TCL_OK, pb = 0;
    struct camprop *cam;
    cam = (struct camprop *) clientData;
    if ((argc != 2) && (argc != 3) && (argc != 4)) {
	pb = 1;
    } else if (argc == 2) {
	pb = 0;
    } else {
	if (argc == 4) {
	    cam->nbampliclean = (int) fabs(atoi(argv[3]));
	}
	if (strcmp(argv[2], "synchro") == 0) {
	    cam->ampliindex = 0;
	    pb = 0;
	} else if (strcmp(argv[2], "on") == 0) {
	    cam->ampliindex = 1;
	    CAM_DRV.ampli_on(cam);
	    pb = 0;
	} else if (strcmp(argv[2], "off") == 0) {
	    cam->ampliindex = 2;
	    CAM_DRV.ampli_off(cam);
	    pb = 0;
	} else {
	    pb = 1;
	}
    }
    if (pb == 1) {
	sprintf(ligne, "Usage: %s %s synchro|on|off ?nbcleanings?",
		argv[0], argv[1]);
	Tcl_SetResult(interp, ligne, TCL_VOLATILE);
	result = TCL_ERROR;
    } else {
	strcpy(ligne, "");
	if (cam->ampliindex == 0) {
	    sprintf(ligne, "synchro %d", cam->nbampliclean);
	} else if (cam->ampliindex == 1) {
	    sprintf(ligne, "on %d", cam->nbampliclean);
	} else if (cam->ampliindex == 2) {
	    sprintf(ligne, "off %d", cam->nbampliclean);
	}
	Tcl_SetResult(interp, ligne, TCL_VOLATILE);
    }
    return result;
}



/*
 * -----------------------------------------------------------------------------
 *  cmdAudinetShutterType()
 *
 * Setect the type of shutter. 2 shutter types are supported :
 *  audine : build by Raymond David (Essentiel Electronic)
 *  thierry : build by Pierre Thierry.
 *
 * -----------------------------------------------------------------------------
 */
int cmdAudinetShutterType(ClientData clientData, Tcl_Interp * interp,
			  int argc, char *argv[])
{
    char ligne[256];
    int result = TCL_OK, pb = 0;
    struct camprop *cam;
    cam = (struct camprop *) clientData;
    if ((argc != 2) && (argc > 4)) {
	pb = 1;
    } else if (argc == 2) {
	pb = 0;
    } else {
	if (strcmp(argv[2], "audine") == 0) {
	    cam->shuttertypeindex = 0;
	    cam->shutteraudinereverse = 0;
	    if (argc >= 4) {
		if (strcmp(argv[3], "reverse") == 0) {
		    cam->shutteraudinereverse = 1;
		}
	    }
	    pb = 0;
	} else if (strcmp(argv[2], "thierry") == 0) {
	    cam->shuttertypeindex = 1;
	    pb = 0;
	} else {
	    pb = 1;
	}
    }
    if (pb == 1) {
	sprintf(ligne, "Usage: %s %s audine|thierry ?options?", argv[0],
		argv[1]);
	Tcl_SetResult(interp, ligne, TCL_VOLATILE);
	result = TCL_ERROR;
    } else {
	strcpy(ligne, "");
	if (cam->shuttertypeindex == 0) {
	    if (cam->shutteraudinereverse == 0) {
		sprintf(ligne, "audine");
	    } else {
		sprintf(ligne, "audine reverse");
	    }
	} else if (cam->shuttertypeindex == 1) {
	    sprintf(ligne, "thierry");
	}
	Tcl_SetResult(interp, ligne, TCL_VOLATILE);
    }
    return result;
}


/*
 * -----------------------------------------------------------------------------
 *  DRIFT SCAN
 * 
 *  cmdAudinetScan()  lance un scan
 *  ScanCallback() fonction appellee par le timer
 * 
 * -----------------------------------------------------------------------------
 */


/*
 * -----------------------------------------------------------------------------
 *  cmdAudinetScan()
 *
 *  Lance un scan.
 *
 *  buf scan width height bin dt
 *    - width : largeur de l'image
 *    - height : largeur de l'image
 *    - bin : facteur de binning
 *    - dt : intervalle de temps entre chaque lecture de ligne (en ms, float)
 *    - -firstpix index : la fenetre commence au pixel num�ro (1 a DimX).
 *    - -fast speed : bloque les interruptions en permanence pour assurer
 *      des vitesses rapides (<500 ms). La valeur de speed provient du 
 *      resultat de la fonction scanloop.
 *    - -tmpfile : l'image est stockee sur un fichier binaire avant d'etre
 *      recuperee en memoire a la fin du scan. Utile pour les grandes images.
 *      Le fichier temporaire s'appelle #scan.bin.
 *    - -perfo : cree un fichier de performances. Inutile pour -fast.
 *
 *  Retourne TCL_OK/TCL_ERROR pour indiquer soit le succes, soit l'echec
 * -----------------------------------------------------------------------------
 */
int cmdAudinetScan(ClientData clientData, Tcl_Interp * interp, int argc,
		   char *argv[])
{
    int w;			/* parametre d'appel : largeur */
    int h;			/* parametre d'appel : hauteur */
    int b;			/* parametre d'appel : binning */
    double dt;			/* parametre d'appel : intervalle de temps */
    int retour = TCL_OK;
    struct camprop *cam;
    char *ligne;		/* Texte pour le retour */
    char *ligne2;		/* Texte pour le retour */
    int idt;			/* Intervalle de temps en ms vers le 1er evenement */
    int i;
    int offset = 1;
    int blocking = 0;
    int keep_perfos = 0;
    int fileima = 0;
    int ok = 1;
    int status;
    double loopmilli1 = 0;

    long next_occur;
    int nb_lignes;
    unsigned long msloop, msloop10, msloops[10];

    char msgtcl[] =
	"Usage: %s %s width height bin dt ?-firstpix index? ?-fast speed? ?-perfo? ?-tmpfile?";
    char text[1024];
    ligne = (char *) calloc(200, sizeof(char));
    ligne2 = (char *) calloc(200, sizeof(char));
    if (argc < 6) {
	sprintf(ligne, msgtcl, argv[0], argv[1]);
	Tcl_SetResult(interp, ligne, TCL_VOLATILE);
	retour = TCL_ERROR;
    } else {
	cam = (struct camprop *) clientData;
	if (TheScanStruct == NULL) {
	    if (Tcl_GetInt(interp, argv[2], &w) != TCL_OK) {
		sprintf(ligne2,
			"%s\nwidth : must be an integer between 1 and %d",
			msgtcl, cam->nb_photox);
		sprintf(ligne, ligne2, argv[0], argv[1]);
		Tcl_SetResult(interp, ligne, TCL_VOLATILE);
		retour = TCL_ERROR;
	    } else if (Tcl_GetInt(interp, argv[3], &h) != TCL_OK) {
		sprintf(ligne2, "%s\nheight : must be an integer >= 1",
			msgtcl);
		sprintf(ligne, ligne2, argv[0], argv[1]);
		Tcl_SetResult(interp, ligne, TCL_VOLATILE);
		retour = TCL_ERROR;
	    } else if (Tcl_GetInt(interp, argv[4], &b) != TCL_OK) {
		sprintf(ligne2, "%s\nbin : must be an integer >= 1",
			msgtcl);
		sprintf(ligne, ligne2, argv[0], argv[1]);
		Tcl_SetResult(interp, ligne, TCL_VOLATILE);
		retour = TCL_ERROR;
	    } else if (Tcl_GetDouble(interp, argv[5], &dt) != TCL_OK) {
		sprintf(ligne2,
			"%s\ndt : must be an floating point number >= 0, expressed in milliseconds",
			msgtcl);
		sprintf(ligne, ligne2, argv[0], argv[1]);
		Tcl_SetResult(interp, ligne, TCL_VOLATILE);
		retour = TCL_ERROR;
	    } else if (dt < 50 || dt > 9000) {
		/* control  50 < dt < 9000 milliseconds */
		sprintf(ligne2,
			"%s\ndt : must be  between 50 and 9000 , expressed in milliseconds",
			msgtcl);
		sprintf(ligne, ligne2, argv[0], argv[1]);
		Tcl_SetResult(interp, ligne, TCL_VOLATILE);
		retour = TCL_ERROR;
	    } else {

		for (i = 6; i < argc; i++) {
//logInfo("cmdAudinetScan argv[%d]=%s",i, argv[i]);
		    if ((strcmp(argv[i], "-offset") == 0)
			|| (strcmp(argv[i], "-firstpix") == 0)) {
			if (Tcl_GetInt(interp, argv[++i], &offset) !=
			    TCL_OK) {
			    sprintf(ligne,
				    "Usage: %s %s width height bin dt ?-firstpix index? ?-blocking? ?-perfo?\nfirstpix index \"%s\" must be an integer",
				    argv[0], argv[1], argv[i]);
			    Tcl_SetResult(interp, ligne, TCL_VOLATILE);
			    ok = 0;
			}
		    } else if (strcmp(argv[i], "-fast") == 0) {
			loopmilli1 = 0.;
			if (i < argc - 1) {
			    loopmilli1 = atof(argv[i + 1]);
			}
			blocking = 1;
		    } else if (strcmp(argv[i], "-perfo") == 0) {
			keep_perfos = 1;
		    } else if (strcmp(argv[i], "-tmpfile") == 0) {
			fileima = 1;
		    }
		}
		if (ok) {

		    if (dt < 50) {
			logError("cmdCamScan dt=%d ", dt);

		    }

		    /* Pour avertir les gens du status de la camera. */
		    //sprintf(ligne, "status_cam%d", cam->camno);
		    //Tcl_SetVar(interp, ligne, "exp", TCL_GLOBAL_ONLY);
          setCameraStatus(cam,interp,"exp");
		    idt = (int) dt;
		    TheScanStruct =
			(ScanStruct *) calloc(1, sizeof(ScanStruct));
		    TheScanStruct->clientData = clientData;
		    TheScanStruct->interp = interp;
		    TheScanStruct->dateobs =
			(char *) calloc(32, sizeof(char));
		    TheScanStruct->dateend =
			(char *) calloc(32, sizeof(char));
		    if (offset > cam->nb_photox) {
			offset = cam->nb_photox;
		    }
		    if (offset < 1) {
			offset = 1;
		    }
		    if (w < 1) {
			w = 1;
		    }
		    if (offset + w > cam->nb_photox) {
			w = cam->nb_photox - (offset - 1);
		    }
		    TheScanStruct->width = w;
		    TheScanStruct->offset = offset;
		    TheScanStruct->height = h;
		    if (b < 1) {
			b = 1;
		    }
		    TheScanStruct->bin = b;
		    if (dt < 0) {
			dt = -dt;
		    }
		    TheScanStruct->dt = (float) dt;
		    TheScanStruct->y = 0;
		    TheScanStruct->blocking = blocking;
		    TheScanStruct->keep_perfos = keep_perfos;
		    TheScanStruct->fileima = fileima;
		    TheScanStruct->fima = NULL;
		    TheScanStruct->dts = (int *) calloc(h, sizeof(int));
		    TheScanStruct->stop = 0;
		    TheScanStruct->pix = NULL;
		    TheScanStruct->pix2 = NULL;
		    TheScanStruct->loopmilli1 = (unsigned long) loopmilli1;
		    /* --- reserve de memoire ou fichier pour les datas intermediaires */
//logInfo("cmdCamScan fileima=%d", TheScanStruct->fileima);
		    if (TheScanStruct->fileima == 1) {
			TheScanStruct->fima = fopen("#scan.bin", "wb");
			if (TheScanStruct->fima == NULL) {
			    TheScanStruct->fileima = 0;
			} else {
			    /* j'alloue un buffer pour une ligne */
			    TheScanStruct->pix = (unsigned short *)
				calloc((unsigned int) (w / b),
				       sizeof(unsigned short));
//logInfo("cmdCamScan file size pix=%d w=%d h=%d b=%d", w/b, w, h, b);
			    TheScanStruct->pix2 = TheScanStruct->pix;
			}
		    }
		    if (TheScanStruct->fileima == 0) {
			/* j'alloue le buffer pour toute l'image */
			TheScanStruct->pix = (unsigned short *)
			    calloc((unsigned int) (w * h / b),
				   sizeof(unsigned short));
//logInfo("cmdCamScan mem size pix=%d w=%d h=%d b=%d", w*h/b, w, h, b);

			TheScanStruct->pix2 = TheScanStruct->pix;
			if (TheScanStruct->pix == NULL) {
			    TheScanStruct->fileima = 1;
			    /* j'alloue un buffer pour une ligne */
			    TheScanStruct->pix = (unsigned short *)
				calloc((unsigned int) (w / b),
				       sizeof(unsigned short));
//logInfo("cmdCamScan file2 size pix=%d w=%d h=%d b=%d", w/b, w, h, b);
			    TheScanStruct->pix2 = TheScanStruct->pix;
			}
			if (TheScanStruct->fileima == 1) {
			    TheScanStruct->fima = fopen("#scan.bin", "wb");
			    if (TheScanStruct->fima == NULL) {
				/* traiter ce cas ou l'on ne peut pas enregistrer l'image */
				/* ni ds la memoire, ni ds le disque */
			    }
         }
          }
                    /* coordonnes du telescope au debut de l'acquisition */
          libcam_get_tel_coord(interp, &TheScanStruct->ra,
             &TheScanStruct->dec, cam,
             &status);
          if (status == 1) {
             TheScanStruct->ra = -1.;
          }
          
          /* Nettoyage du ccd  */
          audinet_fast_vidage_inv(cam);



		    /* Memorise l'heure du top depart */
		    Tcl_Eval(interp, "clock seconds");
		    cam->clockbegin = (int) atoi(interp->result);
		    TheScanStruct->t0 = libcam_getms();

		    /* Declenche le premier evenement */

		    if (blocking == 0) {
			TheScanStruct->TimerToken =
			    Tcl_CreateTimerHandler(idt, ScanCallback,
						   (ClientData) cam);
			libcam_GetCurrentFITSDate(interp,
						  TheScanStruct->dateobs);
			Tcl_ResetResult(interp);
			audinet_startScan(cam, TheScanStruct);

		    } else {
			nb_lignes = TheScanStruct->height;
			libcam_GetCurrentFITSDate(interp,
						  TheScanStruct->dateobs);

			if (cam->authorized == 1) {
			    audinet_startScan(cam, TheScanStruct);
			    if (cam->interrupt == 1) {
				libcam_bloquer();
			    }
			    for (i = 0; i < nb_lignes; i++) {

				ScanCallback((ClientData) cam);
				next_occur = (long) TheScanStruct->dt;
				TheScanStruct->last_delta =
				    (int) next_occur;
				TheScanStruct->dts[i] = (int) next_occur;
				for (msloop10 = 0, msloop = 0;
				     msloop <
				     next_occur *
				     TheScanStruct->loopmilli1; msloop++) {
				    msloops[msloop10] =
					(unsigned long) (0);
				    if (++msloop10 > 9) {
					msloop10 = 0;
				    }
				}
			    }
			    if (cam->interrupt == 1) {
				libcam_debloquer();
			    }
			    if (cam->interrupt == 1) {
				update_clock();
			    }
			}
			libcam_GetCurrentFITSDate(interp,
						  TheScanStruct->dateend);
			ScanTerminateSequence(clientData, cam->camno,
					      "Normally terminated.");
		    }


		} else {
		    retour = TCL_ERROR;
		}
	    }
	} else {
	    sprintf(ligne, "Camera already in use");
	    Tcl_SetResult(interp, ligne, TCL_VOLATILE);
	    retour = TCL_ERROR;
	}
    }
    free(ligne);
    free(ligne2);

    logInfo("cmdCamScan fin");
    return retour;
}

/*
 * ScanCallback --
 *
 * Fonction de lecture du CCD et de constitution de l'image, appellee regulierement
 * au cours de l'acquisition.
 *
 * Effets de bord :
 *   - relance eventuellement un nouveau timer avec ScanCallback en callback
 *   - En cas d'arret (volontaire ou non), met a jour les variables
 *     status_cam%d et scan_result%d (%d : numero de la camera).
 *
 */
void ScanCallback(ClientData clientData)
{
    struct camprop *cam;
    long next_occur;		/* Prochain intervalle de temps a attendre.   */
    //unsigned short scanbuf[10000];

//logInfo("ScanCallback debut ");
    cam = (struct camprop *) clientData;

    libcam_bloquer();
//logInfo("ScanCallback avant scanReadLine");
    audinet_scanReadLine(cam, TheScanStruct, TheScanStruct->pix2);
    libcam_debloquer();
    /* Stocke la ligne dans le fichier temporaire ou dans la memoire temporaire */
    if (TheScanStruct->fileima == 1) {
	fwrite(TheScanStruct->pix2,
	       sizeof(unsigned short) * TheScanStruct->width /
	       TheScanStruct->bin, 1, TheScanStruct->fima);
    } else {
	//memcpy(TheScanStruct->pix2,scanbuf,sizeof(unsigned short)*TheScanStruct->width/TheScanStruct->bin);
	TheScanStruct->pix2 =
	    TheScanStruct->pix2 +
	    TheScanStruct->width / TheScanStruct->bin;
    }
    // j'incremente le nombre de lignes recues
    TheScanStruct->y = TheScanStruct->y + 1;

    if (TheScanStruct->blocking == 1) {
	return;
    }

    if (TheScanStruct->stop == 1) {	/* Arret a la demande de l'utilisateur */

	ScanTerminateSequence(clientData, cam->camno,
			      "User aborted exposure.");
    } else if (TheScanStruct->y < TheScanStruct->height) {	/* On continue : */
	//  je fixe la prochaine lecture dans 10 milli secondes
	//  le d�compte r�el est fait par l'interface audinet
	//  Il est toujours sup�rieur � 10 milli-secondes
	next_occur = 10;

	TheScanStruct->last_delta = (int) next_occur;
	TheScanStruct->dts[TheScanStruct->y - 1] = (int) next_occur;
	if (next_occur <= 0) {	/* ben non : decalage temporel trop grand, on arrete l'image. */
	    libcam_GetCurrentFITSDate(TheScanStruct->interp,
				      TheScanStruct->dateend);
	    ScanTerminateSequence(clientData, cam->camno,
				  "Error : Aborted because CCD line transfers couldn't be re-scheduled (too busy system, or dt too small).");
	} else {		/* OK on continue */
	    TheScanStruct->TimerToken =
		Tcl_CreateTimerHandler((int) next_occur, ScanCallback,
				       (ClientData) cam);
	}
    } else {			/* Image terminee : */
	libcam_GetCurrentFITSDate(TheScanStruct->interp,
				  TheScanStruct->dateend);
	ScanTerminateSequence(clientData, cam->camno,
			      "Normally terminated.");
    }

}

/*
 * ScanTerminateSequence --
 *
 *
 */

void ScanTerminateSequence(ClientData clientData, int camno, char *reason)
{
   //char s[80];
   FILE *f;
   int i;
   
   
   if (TheScanStruct->fileima == 1) {
      fclose(TheScanStruct->fima);
      TheScanStruct->fima = NULL;
   }
   
   audinet_stopScan((struct camprop *) clientData, TheScanStruct);
   
   ScanTransfer(clientData);

   //printf(s, "scan_result%d", camno);
   //Tcl_SetVar(TheScanStruct->interp, s, reason, TCL_GLOBAL_ONLY);
   setScanResult(clientData, TheScanStruct->interp, reason);
   
   //sprintf(s, "status_cam%d", camno);
   //Tcl_SetVar(TheScanStruct->interp, s, "stand", TCL_GLOBAL_ONLY);
   setCameraStatus(clientData,TheScanStruct->interp,"stand");

   if (TheScanStruct->keep_perfos) {
      f = fopen("scanperf.txt", "wt");
      for (i = 0; i < TheScanStruct->height; i++) {
         fprintf(f, "%d %d\n", i, TheScanStruct->dts[i]);
      }
      fclose(f);
   }
   
   ScanLibereStructure();
}

/*
 * ScanTransfer --
 *
 * Transfere l'image accumulee dans un buffer accessible depuis les
 * scripts TCL.
 *
 * Effets de bord :
 *   - Buffer realloue et rempli de l'image acquise.
 *
 */
void ScanTransfer(ClientData clientData)
{
   int naxis1, naxis2, bin1, bin2;
   char s[200];
   double ra, dec;
   float *pp;			/* fpix */
   int t, tt;
   struct camprop *cam;
   Tcl_Interp *interp;
   double exptime;
   double dt;
   double dteff, jdend, jdobs, bloceff;
   int status;
   unsigned short tmp;
   char dateobs_tu[50], dateend_tu[50];
   
   interp = TheScanStruct->interp;
   
   cam = (struct camprop *) clientData;
   cam->clockbegin = 0;
   naxis1 = TheScanStruct->width / TheScanStruct->bin;
   naxis2 = TheScanStruct->y;
   bin1 = TheScanStruct->bin;
   bin2 = TheScanStruct->bin;
   dt = TheScanStruct->dt / 1000.;
   exptime = -1;
   
   /* peu importe le nom de fonction qui suit buf1 */
   sprintf(s, "buf%d bitpix", cam->bufno);
   if (Tcl_Eval(interp, s) == TCL_ERROR) {
      /* Creation du buffer car il n'existe pas */
      sprintf(s, "buf::create %d", cam->bufno);
      Tcl_Eval(interp, s);
   }
   
   sprintf(s, "buf%d format %d %d", cam->bufno, naxis1, naxis2);
   Tcl_Eval(interp, s);
   sprintf(s, "buf%d pointer", cam->bufno);
   Tcl_Eval(interp, s);
   pp = (float *) atoi(interp->result);
   
   strcpy(dateobs_tu, TheScanStruct->dateobs);
   strcpy(dateend_tu, TheScanStruct->dateend);
   
   sprintf(s, "mc_date2jd %s", TheScanStruct->dateobs);
   Tcl_Eval(interp, s);
   jdobs = atof(interp->result);
   sprintf(s, "mc_date2jd %s", TheScanStruct->dateend);
   Tcl_Eval(interp, s);
   jdend = atof(interp->result);
   if (jdend <= jdobs) {
      dteff = dt;
   } else {
      dteff = (jdend - jdobs) * 86400. / (naxis2);
   }
   bloceff = TheScanStruct->loopmilli1;
   if (dteff != 0.) {
      bloceff = dt / dteff * TheScanStruct->loopmilli1;
   }
   
   /* Transfert du fichier ou memoire temporaire dans le buffer image */
   t = naxis1 * naxis2;
   if (TheScanStruct->fileima == 1) {
      TheScanStruct->fima = fopen("#scan.bin", "rb");
      if (TheScanStruct->fima != NULL) {
         tt = 0;
         while (tt < t) {
            fread(&tmp, sizeof(unsigned short), 1,
               TheScanStruct->fima);
            *(pp + tt++) = (float) tmp;
         }
         fclose(TheScanStruct->fima);
         TheScanStruct->fima = NULL;
      }
   } else {
      while (--t >= 0)
         *(pp + t) = (float) *(TheScanStruct->pix + t);
   }
   
   sprintf(s, "buf%d bitpix ushort", cam->bufno);
   Tcl_Eval(interp, s);
   
   sprintf(s, "buf%d setkwd {NAXIS 2 int \"\" \"\"}", cam->bufno);
   sprintf(s, "buf%d setkwd {NAXIS1 %d int \"\" \"\"}", cam->bufno,
      naxis1);
   Tcl_Eval(interp, s);
   sprintf(s, "buf%d setkwd {NAXIS2 %d int \"\" \"\"}", cam->bufno,
      naxis2);
   Tcl_Eval(interp, s);
   sprintf(s, "buf%d setkwd {BIN1 %d int \"\" \"\"}", cam->bufno, bin1);
   Tcl_Eval(interp, s);
   sprintf(s, "buf%d setkwd {BIN2 %d int \"\" \"\"}", cam->bufno, bin2);
   Tcl_Eval(interp, s);
   sprintf(s,
      "buf%d setkwd {DATE-OBS %s string \"Begin of scan exposure.\" \"\"}",
      cam->bufno, dateobs_tu);
   Tcl_Eval(interp, s);
   sprintf(s, "buf%d setkwd {EXPOSURE %f float \"\" \"\"}", cam->bufno,
      exptime);
   Tcl_Eval(interp, s);
   sprintf(s,
      "buf%d setkwd {DATE-END %s string \"End of scan exposure.\" \"\"}",
      cam->bufno, dateend_tu);
   Tcl_Eval(interp, s);
   sprintf(s,
      "buf%d setkwd {SP %lu int \"Asked speed parameter for fast\" \"\"}",
      cam->bufno, TheScanStruct->loopmilli1);
   Tcl_Eval(interp, s);
   sprintf(s,
      "buf%d setkwd {SPEFF %d int \"Effective speed parameter for fast\" \"\"}",
      cam->bufno, (int) bloceff);
   Tcl_Eval(interp, s);
   sprintf(s,
      "buf%d setkwd {DT %f float \"Asked Time Delay Integration\" \"s/line\"}",
      cam->bufno, dt);
   Tcl_Eval(interp, s);
   sprintf(s,
      "buf%d setkwd {DTEFF %f float \"Effective Time Delay Integration\" \"s/line\"}",
      cam->bufno, dteff);
   Tcl_Eval(interp, s);
   
   libcam_get_tel_coord(interp, &ra, &dec, cam, &status);
   if (status == 0) {
      /* Add FITS keywords */
      sprintf(s,
         "buf%d setkwd {RA %f float \"Right ascension telescope at the end\" \"\"}",
         cam->bufno, ra);
      Tcl_Eval(interp, s);
      sprintf(s,
         "buf%d setkwd {DEC %f float \"Declination telescope at the end\" \"\"}",
         cam->bufno, dec);
      Tcl_Eval(interp, s);
   }
   if (TheScanStruct->ra != -1.) {
      /* Add FITS keywords */
      sprintf(s,
         "buf%d setkwd {RA_BEG %f float \"Right ascension telescope at the begining\" \"\"}",
         cam->bufno, TheScanStruct->ra);
      Tcl_Eval(interp, s);
      sprintf(s,
         "buf%d setkwd {DEC_BEG %f float \"Declination telescope at the begining\" \"\"}",
         cam->bufno, TheScanStruct->dec);
      Tcl_Eval(interp, s);
   }
   
   //sprintf(s, "status_cam%d", cam->camno);
   //Tcl_SetVar(interp, s, "stand", TCL_GLOBAL_ONLY);
   setCameraStatus(cam,interp,"stand");
   
}

/*
 * ScanLibereStructure --
 *
 * Libere la structure necessaire au fonctionnement du scan.
 *
 * Effets de bord :
 *   - Structure liberee et remise a NULL.
 *
 */
void ScanLibereStructure()
{
    if (TheScanStruct->fileima == 1) {
	if (TheScanStruct->fima != NULL) {
	    fclose(TheScanStruct->fima);
	    TheScanStruct->fima = NULL;
	}
    }
    free(TheScanStruct->pix);
    free(TheScanStruct->dateobs);
    free(TheScanStruct->dateend);
    free(TheScanStruct->dts);
    free(TheScanStruct);
    TheScanStruct = NULL;
}


/*
 * -----------------------------------------------------------------------------
 *  cmdAudinetBreakScan --
 *
 *  Commande d'arret d'acquisition de scan.
 *
 *  Retourne TCL_OK/TCL_ERROR pour indiquer soit le succes, soit l'echec
 * -----------------------------------------------------------------------------
 */
int cmdAudinetBreakScan(ClientData clientData, Tcl_Interp * interp,
			int argc, char *argv[])
{
    int retour;

    if (TheScanStruct) {
	TheScanStruct->stop = 1;
	retour = TCL_OK;
    } else {
	Tcl_SetResult(interp, "No current exposure", TCL_STATIC);
	retour = TCL_ERROR;
    }

    return retour;
}


/*
 * -----------------------------------------------------------------------------
 *  cmdAudinetScanLoop()
 *
 *  Calcule le nombre de boucles pour attendre une milliseconde en mode cli
 *  C'est une fonction a utiliser avant de faire un scan avec l'option -fast.
 *
 *  buf scanloop
 *
 * -----------------------------------------------------------------------------
 */
int cmdAudinetScanLoop(ClientData clientData, Tcl_Interp * interp,
		       int argc, char *argv[])
{
    char ligne[100];
    sprintf(ligne, "%ld", loopsmillisec());
    Tcl_SetResult(interp, ligne, TCL_VOLATILE);
    return TCL_OK;
}

/*
 * -----------------------------------------------------------------------------
 *  cmdAudinetSetIP()
 *
 *  Envoie une nouvelle adresse IP � l'interface Audinet ayant l'adresse MAC specifiee
 *
 * -----------------------------------------------------------------------------
 */
int cmdAudinetSetIP(ClientData clientData, Tcl_Interp * interp, int argc,
		    char *argv[])
{
    char errorMessage[256];
    int result = TCL_OK;
    char macAddress[20];
    char ipAddress[20];
    char networkmask[20];
    char gateway[20];


    if ((argc < 4)) {
	sprintf(errorMessage,
		"Usage: %s %s ?macaddress? ?ipaddress? [?networkmask? ?gateway?]",
		argv[0], argv[1]);
	Tcl_SetResult(interp, errorMessage, TCL_VOLATILE);
	result = TCL_ERROR;
    } else {

	STRNCPY(macAddress, argv[2]);
	STRNCPY(ipAddress, argv[3]);
	if (argc >= 5) {
	    STRNCPY(networkmask, argv[4]);
	    STRNCPY(gateway, argv[5]);
	    result =
		setip(ipAddress, macAddress, networkmask, gateway,
		      errorMessage);
	} else {
	    result =
		setip(ipAddress, macAddress, NULL, NULL, errorMessage);
	}

	if (result == 0) {
	    sprintf(errorMessage, "setip %s OK", ipAddress);
	    // j'attends une seconde pour laisser Audinet prendre en compte le changement d'adresse
#if defined(OS_WIN)
	    Sleep(1000);
#endif
#if defined(OS_LIN) || defined(OS_MACOS)
	    sleep(1);
#endif
	    result = TCL_OK;
	} else {
	    sprintf(errorMessage, "ERROR setip ipadress=%s macadress=%s ",
		    ipAddress, macAddress);
	    result = TCL_ERROR;
	}
	Tcl_SetResult(interp, errorMessage, TCL_VOLATILE);
    }
    return result;
}
