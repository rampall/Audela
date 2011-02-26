#
# Fichier : bddimages_xml.tcl
# Description : Manipulation des fichiers de config XML de bddimages
#
# Auteur : J. Berthier & F. Vachier
# Mise à jour $Id$
#

namespace eval ::bddimagesAdmin {
   package provide bddimagesAdmin 1.0
   global audace

   # Lecture des captions
   source [ file join [file dirname [info script]] bddimages_admin.cap ]

}

#--------------------------------------------------
#  ::bddimagesAdmin::sql_header { }
#--------------------------------------------------
# Permet de recuperer le nombre d'images dans la bddimages
# @return -nombre de header
#--------------------------------------------------
proc ::bddimagesAdmin::sql_nbimg { } {
   set sqlcmd ""
   append sqlcmd "SELECT count(*) FROM images;"
   set err [catch {set status [::bddimages_sql::sql query $sqlcmd]} msg]
   if {$err != 0} {
      ::console::affiche_erreur "ERREUR sql_nbimg\n"
      ::console::affiche_erreur "  SQL : <$sqlcmd>\n"
      ::console::affiche_erreur "  ERR : <$err>\n"
      ::console::affiche_erreur "  MSG : <$msg>\n"
      return "Table 'images' inexistantes"
   } else {
      return $msg
   }
}

#--------------------------------------------------
#  ::bddimagesAdmin::sql_header { }
#--------------------------------------------------
# Permet de recuperer le nombre de header dans la bddimages
# @return -nombre de header
#--------------------------------------------------
proc ::bddimagesAdmin::sql_header { } {
   set sqlcmd ""
   append sqlcmd "SELECT distinct idheader FROM header;"
   set err [catch {set status [::bddimages_sql::sql query $sqlcmd]} msg]
   if {$err != 0} {
      ::console::affiche_erreur "ERREUR sql_header\n"
      ::console::affiche_erreur "  SQL : <$sqlcmd>\n"
      ::console::affiche_erreur "  ERR : <$err>\n"
      ::console::affiche_erreur "  MSG : <$msg>\n"
      return "Table 'header' inexistantes"
   } else {
      return [llength $msg]
   }
}

#--------------------------------------------------
#  ::bddimagesAdmin::GetPassword { }
#--------------------------------------------------
# Demande d'un mot de passe utilisateur
# @param msg Message de demande du mot de passe
# @return -code err
#--------------------------------------------------
proc ::bddimagesAdmin::GetPassword { msg } {
   global getPassword
   # getPassword est un tableau qui va contenir 3 entrées:
   #   name   contient le nom de l'utilisateur
   #   passwd contient son mot de passe
   #   result contient 1 si et seulement si l'utilisateur a cliqué sur Ok
   set getPassword(result) 0
   set getPassword(passwd) ""

   toplevel .passwd
   wm title .passwd "Root password"
   wm positionfrom .passwd user
   wm sizefrom .passwd user
   frame .passwd.f -relief groove
   pack configure .passwd.f -side top -fill both -expand 1 -padx 10 -pady 10

   # Frame qui va contenir le label "Type your password:" et une entrée pour le rentrer
   frame .passwd.f.pass
   pack configure .passwd.f.pass -side top -fill x
     label .passwd.f.pass.e -text $msg
     pack configure .passwd.f.pass.e -side left -anchor c

   # L'option -show permet de masquer la véritable entrée, 
   # et de mettre une étoile à la place des caractères saisis
   frame .passwd.f.gpass
   pack configure .passwd.f.gpass -side top -fill x
     entry .passwd.f.gpass.v -textvariable getPassword(passwd) -show "*"
     pack configure .passwd.f.gpass.v -side bottom -anchor c

   # Frame qui va contenir les boutons Cancel et Ok
   frame .passwd.f.buttons
   pack configure .passwd.f.buttons -side top -fill x
     button .passwd.f.buttons.cancel -text Cancel -command {destroy .passwd}
     pack configure .passwd.f.buttons.cancel -side left
     button .passwd.f.buttons.ok -text Ok -command { set getPassword(result) 1; destroy .passwd }
     pack configure .passwd.f.buttons.ok -side right

   grab set .passwd
   tkwait window .passwd
   if {$getPassword(result)} {
      return -code 0 $getPassword(passwd)
   } else {
      return -code error ""
   }
}

#--------------------------------------------------
#  ::bddimagesAdmin::RAZBdd { }
#--------------------------------------------------
# Reinitialise la base de donnees bddimages
# @return -code err
#--------------------------------------------------
proc ::bddimagesAdmin::RAZBdd { } {
   global caption
   global bddconf
   
   set answer [tk_messageBox -title $caption(bddimages_admin,msg_prevent) -message $caption(bddimages_admin,msg_prevent2) \
           -icon question -type okcancel ]
   switch -- $answer {
      ok {
         if { [catch {::bddimagesAdmin::GetPassword $caption(bddimages_admin,mdprootsql)} passwd ] != 0 } {
            ::console::affiche_erreur "$caption(bddimages_admin,cancelRAZ)\n"
            return
         }
         # Supprime la BDD
         set status "ok"
         if { [catch {::mysql::connect -host $bddconf(server) -user root -password $passwd} dblink] != 0 } {
            ::console::affiche_erreur "$dblink\n"
            set status "Error: $dblink"
         } else {
            if {$status == "ok"} {
               set sqlcmd "DROP DATABASE IF EXISTS $bddconf(dbname);"
               set err [catch {::mysql::query $dblink $sqlcmd} msg]
               if {$err} {
                  set status "Failed : \n <$sqlcmd> \n <$err> \n <$msg>"
               }
            }
            if {$status == "ok"} {
               set sqlcmd "CREATE DATABASE IF NOT EXISTS $bddconf(dbname);"
               set err [catch {::mysql::query $dblink $sqlcmd} msg]
               if {$err} {
                  set status "Failed : \n <$sqlcmd> \n <$err> \n <$msg>"
               }
            }
            if {$status=="ok"} {
               set sqlcmd "GRANT ALL PRIVILEGES ON `bddimages` . * TO '$bddconf(login)'@'$bddconf(server)' WITH GRANT OPTION ;"
               set err [catch {::mysql::query $dblink $sqlcmd} msg]
               if {$err} {
                  set status "Failed : \n <$sqlcmd> \n <$err> \n <$msg>"
               }
            }
            # Fermeture connection
            ::mysql::close $dblink
            unset dblink
         }
         # Message 
         tk_messageBox -message "$caption(bddimages_admin,efface): $status" -type ok

         # Supprime le repertoire fits
         set errnum [catch {file delete -force $bddconf(dirfits)} msg]
         if {$errnum == 0} {
            ::console::affiche_resultat "Effacement du repertoire : $bddconf(dirfits) \n"
            set errnum [catch {file mkdir  $bddconf(dirfits)} msg]
            if {$errnum == 0} {
               ::console::affiche_resultat "Creation du repertoire : $bddconf(dirfits) \n"
            } else {
               ::console::affiche_resultat "ERREUR: Creation du repertoire : $bddconf(dirfits) impossible <$errnum>\n"
            }
         } else {
            ::console::affiche_resultat "ERREUR: Effacement du repertoire : $bddconf(dirfits) impossible <$errnum>\n"
         }
         
         # Supprime le repertoire logs
         set errnum [catch {file delete -force $bddconf(dirlog)} msg]
         if {$errnum == 0} {
            ::console::affiche_resultat "Effacement du repertoire : $bddconf(dirlog) \n"
            set errnum [catch {file mkdir  $bddconf(dirlog)} msg]
            if {$errnum == 0} {
               ::console::affiche_resultat "Creation du repertoire : $bddconf(dirlog) \n"
            } else {
               ::console::affiche_resultat "ERREUR: Creation du repertoire : $bddconf(dirlog) impossible <$errnum>\n"
            }
         } else {
            ::console::affiche_resultat "ERREUR: Effacement du repertoire : $bddconf(dirlog) impossible <$errnum>\n"
         }
         
         # Supprime le repertoire probleme
         set errnum [catch {file delete -force $bddconf(direrr)} msg]
         if {$errnum == 0} {
            ::console::affiche_resultat "Effacement du repertoire : $bddconf(direrr) \n"
            set errnum [catch {file mkdir  $bddconf(direrr)} msg]
            if {$errnum == 0} {
               ::console::affiche_resultat "Creation du repertoire : $bddconf(direrr) \n"
            } else {
               ::console::affiche_resultat "ERREUR: Creation du repertoire : $bddconf(direrr) impossible <$errnum>\n"
            }
         } else {
            ::console::affiche_resultat "ERREUR: Effacement du repertoire : $bddconf(direrr) impossible <$errnum>\n"
         }

      }
   }
}

#--------------------------------------------------
#  ::bddimagesAdmin::TestConnectBdd { }
#--------------------------------------------------
# Test la connection vers la base de donnees bddimages
# @return -code err
#--------------------------------------------------
proc ::bddimagesAdmin::TestConnectBdd { } {
   global bddconf
   global caption

   set status ""
   if { [catch {::mysql::connect -host $bddconf(server) -user $bddconf(login) -password $bddconf(pass) -db $bddconf(dbname)} dblink] != 0 } {
      set err 1
      set status "$caption(bddimages_admin,mysqlconnecterr)\n $dblink"
   } else {
      set sqlcmd "SHOW TABLES;"
      set err [catch {::mysql::query $dblink $sqlcmd} msg]
      if {$err != 0} {
         set status "$caption(bddimages_admin,mysqlshowerr)\n <$err>, <$msg>"
      } else {
         set status "$caption(bddimages_admin,mysqlok)"
      }
      # Fermeture connection
      ::mysql::close $dblink
      unset dblink
   }
   tk_messageBox -message "$status" -type ok
   return -code $err $status
}




#--------------------------------------------------
#  ::bddimagesAdmin::bdi_compatible { }
#--------------------------------------------------
# verifie la compatibilite de l image
# @return 1 si compatible 0 sinon
#--------------------------------------------------
   proc ::bddimagesAdmin::bdi_compatible { bufno } {
   
      set compat 0

      set key [buf$bufno getkwd "BDDIMAGES VERSION"]
      if {[lindex $key 1] != 1 } {
         return $compat
         }
      set key [buf$bufno getkwd "BDDIMAGES STATE"]
      if {[lindex $key 0] == "" } {
         return $compat
         }
      set key [buf$bufno getkwd "BDDIMAGES TYPE"]
      if {[lindex $key 0] == "" } {
         return $compat
         }
      set key [buf$bufno getkwd "BDDIMAGES WCS"]
      if {[lindex $key 0] == "" } {
         return $compat
         }
      set key [buf$bufno getkwd "BDDIMAGES NAMECATA"]
      if {[lindex $key 0] == "" } {
         return $compat
         }
      set key [buf$bufno getkwd "BDDIMAGES DATECATA"]
      if {[lindex $key 0] == "" } {
         return $compat
         }
      set key [buf$bufno getkwd "BDDIMAGES ASTROID"]
      if {[lindex $key 0] == "" } {
         return $compat
         }
      set key [buf$bufno getkwd "BDDIMAGES ASTROMETRY"]
      if {[lindex $key 0] == "" } {
         return $compat
         }
      set key [buf$bufno getkwd "BDDIMAGES CATAASTROM"]
      if {[lindex $key 0] == "" } {
         return $compat
         }
      set key [buf$bufno getkwd "BDDIMAGES PHOTOMETRY"]
      if {[lindex $key 0] == "" } {
         return $compat
         }
      set key [buf$bufno getkwd "BDDIMAGES CATAPHOTOM"]
      if {[lindex $key 0] == "" } {
         return $compat
         }
      
      return 1
      }


#--------------------------------------------------
#  ::bddimagesAdmin::bdi_setcompat { }
#--------------------------------------------------
# verifie la compatibilite de l image
# @return 1 si compatible 0 sinon
#--------------------------------------------------
   proc ::bddimagesAdmin::bdi_setcompat { bufno } {
   
      set key [buf$bufno getkwd "BDDIMAGES VERSION"]
      if {[lindex $key 1] != 1 } {
         buf$bufno setkwd [list "HIERARCH BDDIMAGES VERSION" "1" "int" "Compatibility version for bddimages" ""]
         }
      set key [buf$bufno getkwd "BDDIMAGES STATES"]
      if {[lindex $key 0] == "" } {
         buf$bufno setkwd [list "HIERARCH BDDIMAGES STATES" "unknown" "string" "RAW | CORR | CATA | Unknown" ""]
         }
      set key [buf$bufno getkwd "BDDIMAGES TYPE"]
      if {[lindex $key 0] == "" } {
         buf$bufno setkwd [list "HIERARCH BDDIMAGES TYPE" "unknown" "string" "IMG | FLAT | DARK | OFFSET | Unknown" ""]
         }
      set key [buf$bufno getkwd "BDDIMAGES WCS"]
      if {[lindex $key 0] == "" } {
         buf$bufno setkwd [list "HIERARCH BDDIMAGES WCS" "unknown" "string" "WCS performed" ""]
         }
      set key [buf$bufno getkwd "BDDIMAGES NAMECATA"]
      if {[lindex $key 0] == "" } {
         buf$bufno setkwd [list "HIERARCH BDDIMAGES NAMECATA" "unknown" "string" "Name file of the cata file" ""]
         }
      set key [buf$bufno getkwd "BDDIMAGES DATECATA"]
      if {[lindex $key 0] == "" } {
         buf$bufno setkwd [list "HIERARCH BDDIMAGES DATECATA" "unknown" "string" "Date iso when the cata file was modified" ""]
         }
      set key [buf$bufno getkwd "BDDIMAGES ASTROID"]
      if {[lindex $key 0] == "" } {
         buf$bufno setkwd [list "HIERARCH BDDIMAGES ASTROID" "unknown" "string" "ASTROID performed" ""]
         }
      set key [buf$bufno getkwd "BDDIMAGES ASTROMETRY"]
      if {[lindex $key 0] == "" } {
         buf$bufno setkwd [list "HIERARCH BDDIMAGES ASTROMETRY" "unknown" "string" "Astrometry performed" ""]
         }
      set key [buf$bufno getkwd "BDDIMAGES CATAASTROM"]
      if {[lindex $key 0] == "" } {
         buf$bufno setkwd [list "HIERARCH BDDIMAGES CATAASTROM" "unknown" "string" "Catalog used for astrometry" ""]
         }
      set key [buf$bufno getkwd "BDDIMAGES PHOTOMETRY"]
      if {[lindex $key 0] == "" } {
         buf$bufno setkwd [list "HIERARCH BDDIMAGES PHOTOMETRY" "unknown" "string" "Photometry performed" ""]
         }
      set key [buf$bufno getkwd "BDDIMAGES CATAPHOTOM"]
      if {[lindex $key 0] == "" } {
         buf$bufno setkwd [list "HIERARCH BDDIMAGES CATAPHOTOM" "unknown" "string" "Catalog used for photometry" ""]
         }
         
         
      # retire les mots cles         
         
      set key [buf$bufno getkwd "BDDIMAGES RAW"]
      if {[lindex $key 0] != "" } {
         buf$bufno delkwd "BDDIMAGES RAW"
         }
      set key [buf$bufno getkwd "BDDIMAGES FLAT"]
      if {[lindex $key 0] != "" } {
         buf$bufno delkwd "BDDIMAGES FLAT"
         }
      set key [buf$bufno getkwd "BDDIMAGES DARK"]
      if {[lindex $key 0] != "" } {
         buf$bufno delkwd "BDDIMAGES DARK"
         }
      set key [buf$bufno getkwd "BDDIMAGES OFFSET"]
      if {[lindex $key 0] != "" } {
         buf$bufno delkwd "BDDIMAGES OFFSET"
         }
      set key [buf$bufno getkwd "BDDIMAGES SFLAT"]
      if {[lindex $key 0] != "" } {
         buf$bufno delkwd "BDDIMAGES SFLAT"
         }
      set key [buf$bufno getkwd "BDDIMAGES SDARK"]
      if {[lindex $key 0] != "" } {
         buf$bufno delkwd "BDDIMAGES SDARK"
         }
      set key [buf$bufno getkwd "BDDIMAGES SOFFSET"]
      if {[lindex $key 0] != "" } {
         buf$bufno delkwd "BDDIMAGES SOFFSET"
         }




      return
      }


