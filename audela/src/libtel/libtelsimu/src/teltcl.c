/* teltcl.c
 *
 * This file is part of the AudeLA project : <http://software.audela.free.fr>
 * Copyright (C) 1998-2004 The AudeLA Core Team
 *
 * Initial author : Alain KLOTZ <alain.klotz@free.fr>
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

#include "sysexp.h"

#if defined(OS_WIN)
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "telescop.h"
#include <libtel/libtel.h>
#include "teltcl.h"
#include <libtel/util.h>

/*
 *   structure pour les fonctions �tendues
 */

int cmdTelAllCoord(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) {
	char list[2048],elem[1024];
   my_pthread_mutex_lock(&mutex);
	strcpy(list,"");
	sprintf(elem,"{cat_cod_deg %f %f} ",telthread->coord_cat_cod_deg_ra,telthread->coord_cat_cod_deg_dec);
	strcat(list,elem);
	sprintf(elem,"{app_cod_deg %f %f %f %f %f %f} ",telthread->coord_app_cod_deg_ra,telthread->coord_app_cod_deg_dec,telthread->coord_app_cod_deg_ha,telthread->coord_app_cod_deg_elev,telthread->coord_app_cod_deg_az,telthread->coord_app_cod_deg_rot);
	strcat(list,elem);
	sprintf(elem,"{cat_sim_deg %f %f} ",telthread->coord_cat_sim_deg_ra,telthread->coord_cat_sim_deg_dec);
	strcat(list,elem);
	sprintf(elem,"{app_sim_deg %f %f %f %f %f %f} ",telthread->coord_app_sim_deg_ra,telthread->coord_app_sim_deg_dec,telthread->coord_app_sim_deg_ha,telthread->coord_app_sim_deg_elev,telthread->coord_app_sim_deg_az,telthread->coord_app_sim_deg_rot);
	strcat(list,elem);
	Tcl_SetResult(interp,list,TCL_VOLATILE);
	my_pthread_mutex_unlock(&mutex);
	return TCL_OK;
}

int cmdTelAllCoordJd(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) {
/* --- Tcl code to generate automatically the C code of this function
set textes ""
set f [open $audace(rep_install)/src/libtel/libmcmt/src/telescop.h r]
set lignes [split [read $f] \n]
close $f
set n [llength $lignes]
for {set k 0} {$k<$n} {incr k} {
	set ligne [lindex $lignes $k]
	set f [string first "START OF COORDSYS" $ligne]
	if {$f>=0} { break }
}
set k1 [expr $k+1]
for {set k $k1} {$k<$n} {incr k} {
	set f [string first "END OF COORDSYS" $ligne]
	if {$f>=0} { break }
	set ligne [lindex $lignes $k]
	set type [lindex $ligne 0]
	if {[string length $type]<3} { continue }
	set varname [string range [lindex $ligne 1] 0 end-1]
	set comment [lrange $ligne 3 end]
	::console::affiche_resultat "$type $varname \{$comment\}\n"
	if {$type=="int"} { set symbtype "%d" } else { set symbtype "%f" }
	set texte "sprintf(elem,\"{$varname $symbtype \{${comment}\}} \",telthread->${varname});"
	append textes "   $texte\n"
	append textes "   strcat(list,elem);\n"
}
*/
	/* --- To read these coordinates:
	set res [tel1 allcoordjd]
	foreach re $res {
		::console::affiche_resultat "$re\n"
	}
	*/
	char list[50048],elem[1024];
   my_pthread_mutex_lock(&mutex);
	strcpy(list,"");
	/* --- beginning of automatically generated C code ---*/
    sprintf(elem,"{coord_cat_cod_deg_ra %f {current catalog coordinates computed from coders}} ",telthread->coord_cat_cod_deg_ra);
   strcat(list,elem);
   sprintf(elem,"{coord_cat_cod_deg_dec %f {current catalog coordinates computed from coders}} ",telthread->coord_cat_cod_deg_dec);
   strcat(list,elem);
   sprintf(elem,"{utcjd_cat_cod_deg_ra %f {date of catalog coordinates computed from coders}} ",telthread->utcjd_cat_cod_deg_ra);
   strcat(list,elem);
   sprintf(elem,"{utcjd_cat_cod_deg_dec %f {date of catalog coordinates computed from coders}} ",telthread->utcjd_cat_cod_deg_dec);
   strcat(list,elem);
   sprintf(elem,"{coord_cat_cod_deg_ra0 %f {initial catalog coordinates computed from coders}} ",telthread->coord_cat_cod_deg_ra0);
   strcat(list,elem);
   sprintf(elem,"{coord_cat_cod_deg_dec0 %f {initial catalog coordinates computed from coders}} ",telthread->coord_cat_cod_deg_dec0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_cat_cod_deg_ra0 %f {date of initial catalog coordinates computed from coders}} ",telthread->utcjd_cat_cod_deg_ra0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_cat_cod_deg_dec0 %f {date of initial catalog coordinates computed from coders}} ",telthread->utcjd_cat_cod_deg_dec0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_deg_ha %f {current apparent coordinates computed from coders}} ",telthread->coord_app_cod_deg_ha);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_deg_dec %f {current apparent coordinates computed from coders}} ",telthread->coord_app_cod_deg_dec);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_deg_ra %f {current apparent coordinates computed from coders}} ",telthread->coord_app_cod_deg_ra);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_deg_az %f {current apparent coordinates computed from coders}} ",telthread->coord_app_cod_deg_az);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_deg_elev %f {current apparent coordinates computed from coders}} ",telthread->coord_app_cod_deg_elev);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_deg_rot %f {current apparent coordinates computed from coders}} ",telthread->coord_app_cod_deg_rot);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_deg_ha %f {date of current apparent coordinates computed from coders}} ",telthread->utcjd_app_cod_deg_ha);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_deg_dec %f {date of current apparent coordinates computed from coders}} ",telthread->utcjd_app_cod_deg_dec);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_deg_ra %f {date of current apparent coordinates computed from coders}} ",telthread->utcjd_app_cod_deg_ra);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_deg_az %f {date of current apparent coordinates computed from coders}} ",telthread->utcjd_app_cod_deg_az);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_deg_elev %f {date of current apparent coordinates computed from coders}} ",telthread->utcjd_app_cod_deg_elev);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_deg_rot %f {date of current apparent coordinates computed from coders}} ",telthread->utcjd_app_cod_deg_rot);
   strcat(list,elem);
   sprintf(elem,"{speed_app_cod_deg_ra %f {current apparent speed computed for coders}} ",telthread->speed_app_cod_deg_ra);
   strcat(list,elem);
   sprintf(elem,"{speed_app_cod_deg_dec %f {current apparent speed computed for coders}} ",telthread->speed_app_cod_deg_dec);
   strcat(list,elem);
   sprintf(elem,"{speed_app_cod_deg_ha %f {current apparent speed computed for coders}} ",telthread->speed_app_cod_deg_ha);
   strcat(list,elem);
   sprintf(elem,"{speed_app_cod_deg_az %f {current apparent speed computed for coders}} ",telthread->speed_app_cod_deg_az);
   strcat(list,elem);
   sprintf(elem,"{speed_app_cod_deg_elev %f {current apparent speed computed for coders}} ",telthread->speed_app_cod_deg_elev);
   strcat(list,elem);
   sprintf(elem,"{speed_app_cod_deg_rot %f {current apparent speed computed for coders}} ",telthread->speed_app_cod_deg_rot);
   strcat(list,elem);
   sprintf(elem,"{speed_app_sim_deg_ra %f {current apparent speed computed from simulation}} ",telthread->speed_app_sim_deg_ra);
   strcat(list,elem);
   sprintf(elem,"{speed_app_sim_deg_dec %f {current apparent speed computed from simulation}} ",telthread->speed_app_sim_deg_dec);
   strcat(list,elem);
   sprintf(elem,"{speed_app_sim_deg_ha %f {current apparent speed computed from simulation}} ",telthread->speed_app_sim_deg_ha);
   strcat(list,elem);
   sprintf(elem,"{speed_app_sim_deg_az %f {current apparent speed computed from simulation}} ",telthread->speed_app_sim_deg_az);
   strcat(list,elem);
   sprintf(elem,"{speed_app_sim_deg_elev %f {current apparent speed computed from simulation}} ",telthread->speed_app_sim_deg_elev);
   strcat(list,elem);
   sprintf(elem,"{speed_app_sim_deg_rot %f {current apparent speed computed from simulation}} ",telthread->speed_app_sim_deg_rot);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_deg_ha0 %f {initial apparent coordinates computed from coders}} ",telthread->coord_app_cod_deg_ha0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_deg_dec0 %f {initial apparent coordinates computed from coders}} ",telthread->coord_app_cod_deg_dec0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_deg_ra0 %f {initial apparent coordinates computed from coders}} ",telthread->coord_app_cod_deg_ra0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_deg_az0 %f {initial apparent coordinates computed from coders}} ",telthread->coord_app_cod_deg_az0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_deg_elev0 %f {initial apparent coordinates computed from coders}} ",telthread->coord_app_cod_deg_elev0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_deg_rot0 %f {initial apparent coordinates computed from coders}} ",telthread->coord_app_cod_deg_rot0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_deg_ha0 %f {date of initial apparent coordinates computed from coders}} ",telthread->utcjd_app_cod_deg_ha0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_deg_dec0 %f {date of initial apparent coordinates computed from coders}} ",telthread->utcjd_app_cod_deg_dec0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_deg_ra0 %f {date of initial apparent coordinates computed from coders}} ",telthread->utcjd_app_cod_deg_ra0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_deg_az0 %f {date of initial apparent coordinates computed from coders}} ",telthread->utcjd_app_cod_deg_az0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_deg_elev0 %f {date of initial apparent coordinates computed from coders}} ",telthread->utcjd_app_cod_deg_elev0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_deg_rot0 %f {date of initial apparent coordinates computed from coders}} ",telthread->utcjd_app_cod_deg_rot0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_adu_ha %f {current ADU from/to coder}} ",telthread->coord_app_cod_adu_ha);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_adu_dec %f {current ADU from/to coder}} ",telthread->coord_app_cod_adu_dec);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_adu_az %f {current ADU from/to coder}} ",telthread->coord_app_cod_adu_az);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_adu_elev %f {current ADU from/to coder}} ",telthread->coord_app_cod_adu_elev);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_adu_rot %f {current ADU from/to coder}} ",telthread->coord_app_cod_adu_rot);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_adu_hapec %f {current ADU from/to coder}} ",telthread->coord_app_cod_adu_hapec);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_adu_ha %f {date of current ADU from/to coder}} ",telthread->utcjd_app_cod_adu_ha);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_adu_dec %f {date of current ADU from/to coder}} ",telthread->utcjd_app_cod_adu_dec);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_adu_az %f {current ADU from/to coder}} ",telthread->utcjd_app_cod_adu_az);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_adu_elev %f {current ADU from/to coder}} ",telthread->utcjd_app_cod_adu_elev);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_adu_rot %f {current ADU from/to coder}} ",telthread->utcjd_app_cod_adu_rot);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_adu_hapec %f {current ADU from/to coder}} ",telthread->utcjd_app_cod_adu_hapec);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_adu_ha0 %f {initial ADU from/to coder}} ",telthread->coord_app_cod_adu_ha0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_adu_dec0 %f {initial ADU from/to coder}} ",telthread->coord_app_cod_adu_dec0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_adu_az0 %f {initial ADU from/to coder}} ",telthread->coord_app_cod_adu_az0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_adu_elev0 %f {initial ADU from/to coder}} ",telthread->coord_app_cod_adu_elev0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_adu_rot0 %f {initial ADU from/to coder}} ",telthread->coord_app_cod_adu_rot0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_adu_ha0 %f {date of initial ADU from/to coder}} ",telthread->utcjd_app_cod_adu_ha0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_adu_dec0 %f {date of initial ADU from/to coder}} ",telthread->utcjd_app_cod_adu_dec0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_adu_az0 %f {date of initial ADU from/to coder}} ",telthread->utcjd_app_cod_adu_az0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_adu_elev0 %f {date of initial ADU from/to coder}} ",telthread->utcjd_app_cod_adu_elev0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_cod_adu_rot0 %f {date of initial ADU from/to coder}} ",telthread->utcjd_app_cod_adu_rot0);
   strcat(list,elem);
   sprintf(elem,"{speed_app_cod_adu_ra %f {current apparent ADU speed computed for coders}} ",telthread->speed_app_cod_adu_ra);
   strcat(list,elem);
   sprintf(elem,"{speed_app_cod_adu_dec %f {current apparent ADU speed computed for coders}} ",telthread->speed_app_cod_adu_dec);
   strcat(list,elem);
   sprintf(elem,"{speed_app_cod_adu_ha %f {current apparent ADU speed computed for coders}} ",telthread->speed_app_cod_adu_ha);
   strcat(list,elem);
   sprintf(elem,"{speed_app_cod_adu_az %f {current apparent ADU speed computed for coders}} ",telthread->speed_app_cod_adu_az);
   strcat(list,elem);
   sprintf(elem,"{speed_app_cod_adu_elev %f {current apparent ADU speed computed for coders}} ",telthread->speed_app_cod_adu_elev);
   strcat(list,elem);
   sprintf(elem,"{speed_app_cod_adu_rot %f {current apparent ADU speed computed for coders}} ",telthread->speed_app_cod_adu_rot);
   strcat(list,elem);
   sprintf(elem,"{speed_app_sim_adu_ra %f {current apparent ADU speed computed from simulation}} ",telthread->speed_app_sim_adu_ra);
   strcat(list,elem);
   sprintf(elem,"{speed_app_sim_adu_dec %f {current apparent ADU speed computed from simulation}} ",telthread->speed_app_sim_adu_dec);
   strcat(list,elem);
   sprintf(elem,"{speed_app_sim_adu_ha %f {current apparent ADU speed computed from simulation}} ",telthread->speed_app_sim_adu_ha);
   strcat(list,elem);
   sprintf(elem,"{speed_app_sim_adu_az %f {current apparent ADU speed computed from simulation}} ",telthread->speed_app_sim_adu_az);
   strcat(list,elem);
   sprintf(elem,"{speed_app_sim_adu_elev %f {current apparent ADU speed computed from simulation}} ",telthread->speed_app_sim_adu_elev);
   strcat(list,elem);
   sprintf(elem,"{speed_app_sim_adu_rot %f {current apparent ADU speed computed from simulation}} ",telthread->speed_app_sim_adu_rot);
   strcat(list,elem);
   sprintf(elem,"{coord_cat_sim_deg_ra %f {current catalog coordinates computed from coders}} ",telthread->coord_cat_sim_deg_ra);
   strcat(list,elem);
   sprintf(elem,"{coord_cat_sim_deg_dec %f {current catalog coordinates computed from coders}} ",telthread->coord_cat_sim_deg_dec);
   strcat(list,elem);
   sprintf(elem,"{utcjd_cat_sim_deg_ra %f {date of catalog coordinates computed from coders}} ",telthread->utcjd_cat_sim_deg_ra);
   strcat(list,elem);
   sprintf(elem,"{utcjd_cat_sim_deg_dec %f {date of catalog coordinates computed from coders}} ",telthread->utcjd_cat_sim_deg_dec);
   strcat(list,elem);
   sprintf(elem,"{coord_cat_sim_deg_ra0 %f {initial catalog coordinates computed from coders}} ",telthread->coord_cat_sim_deg_ra0);
   strcat(list,elem);
   sprintf(elem,"{coord_cat_sim_deg_dec0 %f {initial catalog coordinates computed from coders}} ",telthread->coord_cat_sim_deg_dec0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_cat_sim_deg_ra0 %f {date of initial catalog coordinates computed from coders}} ",telthread->utcjd_cat_sim_deg_ra0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_cat_sim_deg_dec0 %f {date of initial catalog coordinates computed from coders}} ",telthread->utcjd_cat_sim_deg_dec0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_deg_ha %f {current apparent coordinates computed from coders}} ",telthread->coord_app_sim_deg_ha);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_deg_dec %f {current apparent coordinates computed from coders}} ",telthread->coord_app_sim_deg_dec);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_deg_ra %f {current apparent coordinates computed from coders}} ",telthread->coord_app_sim_deg_ra);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_deg_az %f {current apparent coordinates computed from coders}} ",telthread->coord_app_sim_deg_az);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_deg_elev %f {current apparent coordinates computed from coders}} ",telthread->coord_app_sim_deg_elev);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_deg_rot %f {current apparent coordinates computed from coders}} ",telthread->coord_app_sim_deg_rot);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_deg_ha %f {date of current apparent coordinates computed from coders}} ",telthread->utcjd_app_sim_deg_ha);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_deg_dec %f {date of current apparent coordinates computed from coders}} ",telthread->utcjd_app_sim_deg_dec);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_deg_ra %f {date of current apparent coordinates computed from coders}} ",telthread->utcjd_app_sim_deg_ra);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_deg_az %f {date of current apparent coordinates computed from coders}} ",telthread->utcjd_app_sim_deg_az);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_deg_elev %f {date of current apparent coordinates computed from coders}} ",telthread->utcjd_app_sim_deg_elev);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_deg_rot %f {date of current apparent coordinates computed from coders}} ",telthread->utcjd_app_sim_deg_rot);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_deg_ha0 %f {initial apparent coordinates computed from coders}} ",telthread->coord_app_sim_deg_ha0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_deg_dec0 %f {initial apparent coordinates computed from coders}} ",telthread->coord_app_sim_deg_dec0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_deg_ra0 %f {initial apparent coordinates computed from coders}} ",telthread->coord_app_sim_deg_ra0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_deg_az0 %f {initial apparent coordinates computed from coders}} ",telthread->coord_app_sim_deg_az0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_deg_elev0 %f {initial apparent coordinates computed from coders}} ",telthread->coord_app_sim_deg_elev0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_deg_rot0 %f {initial apparent coordinates computed from coders}} ",telthread->coord_app_sim_deg_rot0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_deg_ha0 %f {date of initial apparent coordinates computed from coders}} ",telthread->utcjd_app_sim_deg_ha0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_deg_dec0 %f {date of initial apparent coordinates computed from coders}} ",telthread->utcjd_app_sim_deg_dec0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_deg_ra0 %f {date of initial apparent coordinates computed from coders}} ",telthread->utcjd_app_sim_deg_ra0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_deg_az0 %f {date of initial apparent coordinates computed from coders}} ",telthread->utcjd_app_sim_deg_az0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_deg_elev0 %f {date of initial apparent coordinates computed from coders}} ",telthread->utcjd_app_sim_deg_elev0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_deg_rot0 %f {date of initial apparent coordinates computed from coders}} ",telthread->utcjd_app_sim_deg_rot0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_ha %f {current ADU from/to coder}} ",telthread->coord_app_sim_adu_ha);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_dec %f {current ADU from/to coder}} ",telthread->coord_app_sim_adu_dec);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_az %f {current ADU from/to coder}} ",telthread->coord_app_sim_adu_az);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_elev %f {current ADU from/to coder}} ",telthread->coord_app_sim_adu_elev);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_rot %f {current ADU from/to coder}} ",telthread->coord_app_sim_adu_rot);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_hapec %f {current ADU from/to coder}} ",telthread->coord_app_sim_adu_hapec);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_adu_ha %f {date of current ADU from/to coder}} ",telthread->utcjd_app_sim_adu_ha);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_adu_dec %f {date of current ADU from/to coder}} ",telthread->utcjd_app_sim_adu_dec);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_adu_az %f {current ADU from/to coder}} ",telthread->utcjd_app_sim_adu_az);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_adu_elev %f {current ADU from/to coder}} ",telthread->utcjd_app_sim_adu_elev);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_adu_rot %f {current ADU from/to coder}} ",telthread->utcjd_app_sim_adu_rot);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_adu_hapec %f {current ADU from/to coder}} ",telthread->utcjd_app_sim_adu_hapec);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_ha0 %f {initial ADU from/to coder}} ",telthread->coord_app_sim_adu_ha0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_dec0 %f {initial ADU from/to coder}} ",telthread->coord_app_sim_adu_dec0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_az0 %f {initial ADU from/to coder}} ",telthread->coord_app_sim_adu_az0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_elev0 %f {initial ADU from/to coder}} ",telthread->coord_app_sim_adu_elev0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_rot0 %f {initial ADU from/to coder}} ",telthread->coord_app_sim_adu_rot0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_hapec0 %f {initial ADU from/to coder}} ",telthread->coord_app_sim_adu_hapec0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_adu_ha0 %f {date of initial ADU from/to coder}} ",telthread->utcjd_app_sim_adu_ha0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_adu_dec0 %f {date of initial ADU from/to coder}} ",telthread->utcjd_app_sim_adu_dec0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_adu_az0 %f {date of initial ADU from/to coder}} ",telthread->utcjd_app_sim_adu_az0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_adu_elev0 %f {date of initial ADU from/to coder}} ",telthread->utcjd_app_sim_adu_elev0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_adu_rot0 %f {date of initial ADU from/to coder}} ",telthread->utcjd_app_sim_adu_rot0);
   strcat(list,elem);
   sprintf(elem,"{utcjd_app_sim_adu_hapec0 %f {date of initial ADU from/to coder}} ",telthread->utcjd_app_sim_adu_hapec0);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_adu_dha %f {next goto ADU from/to coder}} ",telthread->coord_app_cod_adu_dha);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_adu_ddec %f {next goto ADU from/to coder}} ",telthread->coord_app_cod_adu_ddec);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_adu_daz %f {next goto ADU from/to coder}} ",telthread->coord_app_cod_adu_daz);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_adu_delev %f {next goto ADU from/to coder}} ",telthread->coord_app_cod_adu_delev);
   strcat(list,elem);
   sprintf(elem,"{coord_app_cod_adu_drot %f {next goto ADU from/to coder}} ",telthread->coord_app_cod_adu_drot);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_dha %f {next goto ADU from/to coder}} ",telthread->coord_app_sim_adu_dha);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_ddec %f {next goto ADU from/to coder}} ",telthread->coord_app_sim_adu_ddec);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_daz %f {next goto ADU from/to coder}} ",telthread->coord_app_sim_adu_daz);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_delev %f {next goto ADU from/to coder}} ",telthread->coord_app_sim_adu_delev);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_drot %f {next goto ADU from/to coder}} ",telthread->coord_app_sim_adu_drot);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_cumdha %f {cumulative gotos ADU from/to coder}} ",telthread->coord_app_sim_adu_cumdha);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_cumddec %f {cumulative gotos ADU from/to coder}} ",telthread->coord_app_sim_adu_cumddec);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_cumdaz %f {cumulative gotos ADU from/to coder}} ",telthread->coord_app_sim_adu_cumdaz);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_cumdelev %f {cumulative gotos ADU from/to coder}} ",telthread->coord_app_sim_adu_cumdelev);
   strcat(list,elem);
   sprintf(elem,"{coord_app_sim_adu_cumdrot %f {cumulative gotos ADU from/to coder}} ",telthread->coord_app_sim_adu_cumdrot);
   strcat(list,elem);
	/* --- end of automatically generated C code ---*/
	Tcl_SetResult(interp,list,TCL_VOLATILE);
	my_pthread_mutex_unlock(&mutex);
	return TCL_OK;
}

int cmdTelAction(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) {
   char ligne[256];
   if (argc<2) {
   	sprintf(ligne,"usage: %s %s action",argv[0],argv[1]);
      Tcl_SetResult(interp,ligne,TCL_VOLATILE);
		return TCL_ERROR;
	}
   my_pthread_mutex_lock(&mutex);
	if (argc==2) {
		sprintf(ligne,"%s %d",telthread->action_cur,telthread->compteur);
		Tcl_SetResult(interp,ligne,TCL_VOLATILE);
	} else {
		strcpy(telthread->action_next,argv[2]);
		Tcl_SetResult(interp,telthread->action_next,TCL_VOLATILE);
	}
	my_pthread_mutex_unlock(&mutex);
	return TCL_OK;
}

int cmdTelLoopEval(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) {
   char ligne[256];
   if (argc<3) {
   	sprintf(ligne,"usage: %s %s command_line",argv[0],argv[1]);
      Tcl_SetResult(interp,ligne,TCL_VOLATILE);
		return TCL_ERROR;
	}
   my_pthread_mutex_lock(&mutex);
	strcpy(telthread->eval_command_line,argv[2]);
	sprintf(ligne,"Result stored in %s loopresult",argv[0]);
	Tcl_SetResult(interp,ligne,TCL_VOLATILE);
	my_pthread_mutex_unlock(&mutex);
	return TCL_OK;
}

int cmdTelLoopResult(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) {
   my_pthread_mutex_lock(&mutex);
	Tcl_SetResult(interp,telthread->eval_result,TCL_VOLATILE);
	my_pthread_mutex_unlock(&mutex);
	return TCL_OK;
}

int cmdTelHaDec(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) {
   char ligne[2256],ligne2[2256];
   int result = TCL_OK;
   struct telprop *tel;
   char comment[]="Usage: %s %s ?goto|stop|move|coord|motor|init|state|?";
   if (argc<3) {
      sprintf(ligne,comment,argv[0],argv[1]);
      Tcl_SetResult(interp,ligne,TCL_VOLATILE);
      result = TCL_ERROR;
   } else {
      tel = (struct telprop*)clientData;
      if (strcmp(argv[2],"init")==0) {
         /* --- init ---*/
         if (argc>=4) {
				sprintf(ligne2,"mc_angle2deg [lindex {%s} 0]",argv[3]);
				Tcl_Eval(interp,ligne2);
				tel->ha0=atof(interp->result);
				sprintf(ligne2,"mc_angle2deg [lindex {%s} 1] 90",argv[3]);
				Tcl_Eval(interp,ligne2);
				tel->dec0=atof(interp->result);
				result=mytel_hadec_init(tel);
				Tcl_SetResult(interp,"",TCL_VOLATILE);
            result = TCL_OK;
         } else {
            sprintf(ligne,"Usage: %s %s %s {angle_ra angle_dec}",argv[0],argv[1],argv[2]);
            Tcl_SetResult(interp,ligne,TCL_VOLATILE);
            result = TCL_ERROR;
         }
      } else if (strcmp(argv[2],"coord")==0) {
			result=mytel_hadec_coord(tel,ligne);
			if ( result == TCL_OK) {                     
            sprintf(ligne2,"list [mc_angle2hms [lindex {%s} 0] 360 zero 2 auto string] [mc_angle2dms [lindex {%s} 1] 90 zero 1 + string]",ligne,ligne); 
				Tcl_Eval(interp,ligne2);
            Tcl_SetResult(interp,interp->result,TCL_VOLATILE);
            result = TCL_OK;
         }
      } else if (strcmp(argv[2],"goto")==0) {
         /* --- goto ---*/
         if (argc>=4) {
				sprintf(ligne2,"mc_angle2deg [lindex {%s} 0]",argv[3]);
				Tcl_Eval(interp,ligne2);
				tel->ha0=atof(interp->result);
				sprintf(ligne2,"mc_angle2deg [lindex {%s} 1] 90",argv[3]);
				Tcl_Eval(interp,ligne2);
				tel->dec0=atof(interp->result);
				result=mytel_hadec_goto(tel);
				Tcl_SetResult(interp,"",TCL_VOLATILE);
            result = TCL_OK;
         } else {
            sprintf(ligne,"Usage: %s %s %s {angle_ra angle_dec}",argv[0],argv[1],argv[2]);
            Tcl_SetResult(interp,ligne,TCL_VOLATILE);
            result = TCL_ERROR;
         }
      } else {
         /* --- sub command not found ---*/
         sprintf(ligne,comment,argv[0],argv[1]);
         Tcl_SetResult(interp,ligne,TCL_VOLATILE);
         result = TCL_ERROR;
      }
   }
   return result;
}

int cmdTelAdu(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]) {
   char ligne[2256];
   int result = TCL_OK;
   struct telprop *tel;
   char comment[]="Usage: %s %s ?goto|stop|move|coord|motor|init|state|?";
   if (argc<3) {
      sprintf(ligne,comment,argv[0],argv[1]);
      Tcl_SetResult(interp,ligne,TCL_VOLATILE);
      result = TCL_ERROR;
   } else {
      tel = (struct telprop*)clientData;
      if (strcmp(argv[2],"coord")==0) {
			result=mytel_adu_coord(tel,ligne);
			if ( result == TCL_OK) {                     
            Tcl_SetResult(interp,ligne,TCL_VOLATILE);
            result = TCL_OK;
         }
      } else if (strcmp(argv[2],"goto")==0) {
			/*
			result=mytel_adu_coord(tel,ligne);
			if ( result == TCL_OK) {                     
				// je convertis les angles en HMS et DMS
            sprintf(ligne,"list [mc_angle2hms [lindex {%s} 0] 360 zero 2 auto string] [mc_angle2dms [lindex {%s} 1] 90 zero 1 + string]",ligne,ligne); 
            Tcl_SetResult(interp,ligne,TCL_VOLATILE);
            result = TCL_OK;
         }
			*/
      } else {
         /* --- sub command not found ---*/
         sprintf(ligne,comment,argv[0],argv[1]);
         Tcl_SetResult(interp,ligne,TCL_VOLATILE);
         result = TCL_ERROR;
      }
   }
   return result;
}
