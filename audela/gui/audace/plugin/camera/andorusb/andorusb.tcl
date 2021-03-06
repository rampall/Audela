#
# Fichier : andorusb.tcl
# Description : Configuration de la camera Andor
# Auteur : Frederic VACHIER
# Mise à jour $Id$
#

namespace eval ::andorusb {
   package provide andorusb 1.0

   #--- Charge le fichier caption
   source [ file join [file dirname [info script]] andorusb.cap ]
}

#
# getPluginTitle
#    Retourne le label du plugin dans la langue de l'utilisateur
#
proc ::andorusb::getPluginTitle { } {
   global caption

   return "$caption(andorusb,camera)"
}

#
# getPluginHelp
#    Retourne la documentation du plugin
#
proc ::andorusb::getPluginHelp { } {
   return "andorusb.htm"
}

#
# getPluginType
#    Retourne le type du plugin
#
proc ::andorusb::getPluginType { } {
   return "camera"
}

#
# getPluginOS
#    Retourne le ou les OS de fonctionnement du plugin
#
proc ::andorusb::getPluginOS { } {
   return [ list Linux ]
}

#
# getCamNo
#    Retourne le numero de la camera
#
proc ::andorusb::getCamNo { camItem } {
   variable private

   return $private($camItem,camNo)
}

#
# isReady
#    Indique que la camera est prete
#    Retourne "1" si la camera est prete, sinon retourne "0"
#
proc ::andorusb::isReady { camItem } {
   variable private

   if { $private($camItem,camNo) == "0" } {
      #--- Camera KO
      return 0
   } else {
      #--- Camera OK
      return 1
   }
}

#
# initPlugin
#    Initialise les variables conf(andorusb,...)
#
proc ::andorusb::initPlugin { } {
   variable private
   global audace conf caption

   #--- Initialise les variables de la camera AndorUSB
   if { ! [ info exists conf(andorusb,cool) ] }        { set conf(andorusb,cool)        "0" }
   if { ! [ info exists conf(andorusb,foncobtu) ] }    { set conf(andorusb,foncobtu)    "2" }
   if { ! [ info exists conf(andorusb,config) ] }      { set conf(andorusb,config)      [ file join $audace(rep_install) bin ] }
   if { ! [ info exists conf(andorusb,mirh) ] }        { set conf(andorusb,mirh)        "0" }
   if { ! [ info exists conf(andorusb,mirv) ] }        { set conf(andorusb,mirv)        "0" }
   if { ! [ info exists conf(andorusb,temp) ] }        { set conf(andorusb,temp)        "-50" }
   if { ! [ info exists conf(andorusb,ouvert_obtu) ] } { set conf(andorusb,ouvert_obtu) "0" }
   if { ! [ info exists conf(andorusb,ferm_obtu) ] }   { set conf(andorusb,ferm_obtu)   "30" }

   #--- Initialisation
   set private(A,camNo) "0"
   set private(B,camNo) "0"
   set private(C,camNo) "0"
   set private(ccdTemp) "$caption(andorusb,temperature_CCD)"
}

#
# confToWidget
#    Copie les variables de configuration dans des variables locales
#
proc ::andorusb::confToWidget { } {
   variable private
   global caption conf

   #--- Recupere la configuration de la camera Andor dans le tableau private(...)
   set private(cool)        $conf(andorusb,cool)
   set private(foncobtu)    [ lindex "$caption(andorusb,obtu_ouvert) $caption(andorusb,obtu_ferme) $caption(andorusb,obtu_synchro)" $conf(andorusb,foncobtu) ]
   set private(config)      $conf(andorusb,config)
   set private(mirh)        $conf(andorusb,mirh)
   set private(mirv)        $conf(andorusb,mirv)
   set private(temp)        $conf(andorusb,temp)
   set private(ouvert_obtu) $conf(andorusb,ouvert_obtu)
   set private(ferm_obtu)   $conf(andorusb,ferm_obtu)
}

#
# widgetToConf
#    Copie les variables locales dans des variables de configuration
#
proc ::andorusb::widgetToConf { camItem } {
   variable private
   global caption conf

   #--- Memorise la configuration de la camera Andor dans le tableau conf(andorusb,...)
   set conf(andorusb,cool)        $private(cool)
   set conf(andorusb,foncobtu)    [ lsearch "$caption(andorusb,obtu_ouvert) $caption(andorusb,obtu_ferme) $caption(andorusb,obtu_synchro)" "$private(foncobtu)" ]
   set conf(andorusb,config)      $private(config)
   set conf(andorusb,mirh)        $private(mirh)
   set conf(andorusb,mirv)        $private(mirv)
   set conf(andorusb,temp)        $private(temp)
   set conf(andorusb,ouvert_obtu) $private(ouvert_obtu)
   set conf(andorusb,ferm_obtu)   $private(ferm_obtu)
}

#
# fillConfigPage
#    Interface de configuration de la camera Andor
#
proc ::andorusb::fillConfigPage { frm camItem } {
   variable private
   global audace caption

   #--- Initialise une variable locale
   set private(frm) $frm

   #--- confToWidget
   ::andorusb::confToWidget

   #--- Supprime tous les widgets de l'onglet
   foreach i [ winfo children $frm ] {
      destroy $i
   }

   #--- Frame du repertoire des fichiers de configuration
   frame $frm.frame1 -borderwidth 0 -relief raised

      #--- Definition du repertoire des fichiers de configuration
      label $frm.frame1.lab2 -text "$caption(andorusb,config)"
      pack $frm.frame1.lab2 -anchor center -side left -padx 10

      entry $frm.frame1.host -width 40 -textvariable ::andorusb::private(config)
      pack $frm.frame1.host -anchor center -side left -padx 10

      button $frm.frame1.explore -text "$caption(andorusb,parcourir)" -width 1 -command "::andorusb::explore"
      pack $frm.frame1.explore -side left -padx 10 -pady 5 -ipady 5

   pack $frm.frame1 -side top -fill both -expand 1

   #--- Frame des miroirs en x et en y, du refroidissement, de la temperature du capteur CCD et de l'obturateur
   frame $frm.frame2 -borderwidth 0 -relief raised

      #--- Frame du mode de fonctionnement, du delai d'ouverture et de fermeture de l'obturateur
      frame $frm.frame2.frame4 -borderwidth 0 -relief raised

         #--- Frame du mode de fonctionnement de l'obturateur
         frame $frm.frame2.frame4.frame7 -borderwidth 0 -relief raised

            #--- Mode de fonctionnement de l'obturateur
            label $frm.frame2.frame4.frame7.lab3 -text "$caption(andorusb,fonc_obtu)"
            pack $frm.frame2.frame4.frame7.lab3 -anchor center -side left -padx 10 -pady 5

            set list_combobox [ list $caption(andorusb,obtu_ouvert) $caption(andorusb,obtu_ferme) $caption(andorusb,obtu_synchro) ]
            ComboBox $frm.frame2.frame4.frame7.foncobtu \
               -width [ ::tkutil::lgEntryComboBox $list_combobox ] \
               -height [ llength $list_combobox ] \
              -relief sunken       \
               -borderwidth 1      \
               -editable 0         \
               -textvariable ::andorusb::private(foncobtu) \
               -values $list_combobox
            pack $frm.frame2.frame4.frame7.foncobtu -anchor center -side left -padx 10 -pady 5

         pack $frm.frame2.frame4.frame7 -side top -fill x -expand 1

         #--- Frame du delai d'ouverture de l'obturateur
         frame $frm.frame2.frame4.frame8 -borderwidth 0 -relief raised

            #--- Delai d'ouverture de l'obturateur
            label $frm.frame2.frame4.frame8.lab4 -text "$caption(andorusb,ouvert_obtu)"
            pack $frm.frame2.frame4.frame8.lab4 -anchor center -side left -padx 10 -pady 5

            entry $frm.frame2.frame4.frame8.ouvert_obtu -textvariable ::andorusb::private(ouvert_obtu) \
               -width 4 -justify center \
               -validate all -validatecommand { ::tkutil::validateNumber %W %V %P %s integer 0 1000 }
            pack $frm.frame2.frame4.frame8.ouvert_obtu -anchor center -side left -padx 5 -pady 5

            label $frm.frame2.frame4.frame8.lab5 -text "$caption(andorusb,ms)"
            pack $frm.frame2.frame4.frame8.lab5 -side left -fill x -padx 0 -pady 5

         pack $frm.frame2.frame4.frame8 -side top -fill x -expand 1

         #--- Frame du delai de fermeture de l'obturateur
         frame $frm.frame2.frame4.frame9 -borderwidth 0 -relief raised

            #--- Delai de fermeture de l'obturateur
            label $frm.frame2.frame4.frame9.lab6 -text "$caption(andorusb,ferm_obtu)"
            pack $frm.frame2.frame4.frame9.lab6 -anchor center -side left -padx 10 -pady 5

            entry $frm.frame2.frame4.frame9.ferm_obtu -textvariable ::andorusb::private(ferm_obtu) \
               -width 4 -justify center \
               -validate all -validatecommand { ::tkutil::validateNumber %W %V %P %s integer 0 1000 }
            pack $frm.frame2.frame4.frame9.ferm_obtu -anchor center -side left -padx 5 -pady 5

            label $frm.frame2.frame4.frame9.lab7 -text "$caption(andorusb,ms)"
            pack $frm.frame2.frame4.frame9.lab7 -side left -fill x -padx 0 -pady 5

         pack $frm.frame2.frame4.frame9 -side top -fill x -expand 1

      pack $frm.frame2.frame4 -side bottom -fill both -expand 1

      #--- Frame des miroirs en x et en y
      frame $frm.frame2.frame5 -borderwidth 0 -relief raised

         #--- Miroir en x et en y
         checkbutton $frm.frame2.frame5.mirx -text "$caption(andorusb,miroir_x)" -highlightthickness 0 \
            -variable ::andorusb::private(mirh)
         pack $frm.frame2.frame5.mirx -anchor w -side top -padx 20 -pady 10

         checkbutton $frm.frame2.frame5.miry -text "$caption(andorusb,miroir_y)" -highlightthickness 0 \
            -variable ::andorusb::private(mirv)
         pack $frm.frame2.frame5.miry -anchor w -side top -padx 20 -pady 10

      pack $frm.frame2.frame5 -anchor n -side left -fill x -padx 20

      #--- Frame du refroidissement et de la temperature du capteur CCD
      frame $frm.frame2.frame6 -borderwidth 0 -relief raised

         #--- Frame du refroidissement
         frame $frm.frame2.frame6.frame10 -borderwidth 0 -relief raised

            #--- Definition du refroidissement
            checkbutton $frm.frame2.frame6.frame10.cool -text "$caption(andorusb,refroidissement)" \
               -highlightthickness 0 -variable ::andorusb::private(cool) \
               -command "::andorusb::checkConfigRefroidissement"
            pack $frm.frame2.frame6.frame10.cool -anchor center -side left -padx 0 -pady 5

            entry $frm.frame2.frame6.frame10.temp -textvariable ::andorusb::private(temp) -width 4 \
               -justify center \
               -validate all -validatecommand { ::tkutil::validateNumber %W %V %P %s double -274 50 }
            pack $frm.frame2.frame6.frame10.temp -anchor center -side left -padx 5 -pady 5

            label $frm.frame2.frame6.frame10.tempdeg \
               -text "$caption(andorusb,refroidissement_1)"
            pack $frm.frame2.frame6.frame10.tempdeg -side left -fill x -padx 0 -pady 5

         pack $frm.frame2.frame6.frame10 -side top -fill x -padx 10

         #--- Frame de la temperature du capteur CCD
         frame $frm.frame2.frame6.frame11 -borderwidth 0 -relief raised

            #--- Definition de la temperature du capteur CCD
            label $frm.frame2.frame6.frame11.ccdtemp -textvariable ::andorusb::private(ccdTemp)
            pack $frm.frame2.frame6.frame11.ccdtemp -side left -fill x -padx 20 -pady 5

         pack $frm.frame2.frame6.frame11 -side top -fill x -padx 30

      pack $frm.frame2.frame6 -side left -fill x -expand 0

   pack $frm.frame2 -side top -fill x -expand 0

   #--- Frame du site web officiel de la Andor
   frame $frm.frame3 -borderwidth 0 -relief raised

      label $frm.frame3.lab103 -text "$caption(andorusb,titre_site_web)"
      pack $frm.frame3.lab103 -side top -fill x -pady 2

      set labelName [ ::confCam::createUrlLabel $frm.frame3 "$caption(andorusb,site_web_ref)" \
         "$caption(andorusb,site_web_ref)" ]
      pack $labelName -side top -fill x -pady 2

   pack $frm.frame3 -side bottom -fill x -pady 2

   #--- Gestion des widgets actifs/inactifs
   ::andorusb::checkConfigRefroidissement

   #--- Mise a jour dynamique des couleurs
   ::confColor::applyColor $frm
}

#
# configureCamera
#    Configure la camera Andor en fonction des donnees contenues dans les variables conf(andorusb,...)
#
proc ::andorusb::configureCamera { camItem bufNo } {
   variable private
   global caption conf

   set catchResult [ catch {
      #--- je verifie que la camera n'est deja utilisee
      if { $private(A,camNo) != 0 || $private(B,camNo) != 0 || $private(C,camNo) != 0  } {
         error "" "" "CameraUnique"
      }
      #--- Je mets conf(andorusb,config) entre guillemets pour le cas ou le nom du repertoire contient des espaces
      #--- Je cree la camera
      set camNo [ cam::create andorusb USB $conf(andorusb,config) -debug_directory $::audace(rep_log) ]
      console::affiche_entete "$caption(andorusb,port_camera) ([ cam$camNo name ]) \
         $caption(andorusb,2points) $conf(andorusb,config)\n"
      console::affiche_saut "\n"
      #--- Je change de variable
      set private($camItem,camNo) $camNo
      #--- Je configure l'obturateur
      switch -exact -- $conf(andorusb,foncobtu) {
         0 {
            cam$camNo shutter "opened"
         }
         1 {
            cam$camNo shutter "closed"
         }
         2 {
            cam$camNo shutter "synchro"
         }
      }
      #--- Je configure le refroidissement
      if { $conf(andorusb,cool) == "1" } {
         cam$camNo cooler on
         cam$camNo cooler check $conf(andorusb,temp)
      } else {
         cam$camNo cooler off
      }
      #--- J'associe le buffer de la visu
      cam$camNo buf $bufNo
      #--- Je configure l'oriention des miroirs par defaut
      cam$camNo mirrorh $conf(andorusb,mirh)
      cam$camNo mirrorv $conf(andorusb,mirv)
      #--- Delais d'ouverture et de fermeture de l'obturateur
      cam$camNo openingtime $conf(andorusb,ouvert_obtu)
      cam$camNo closingtime $conf(andorusb,ferm_obtu)
      #--- Je mesure la temperature du capteur CCD
      if { [ info exists private(aftertemp) ] == "0" } {
         ::andorusb::dispTempAndor $camItem
      }
   } ]

   if { $catchResult == "1" } {
      #--- En cas d'erreur, je libere toutes les ressources allouees
      ::andorusb::stop $camItem
      #--- Je transmets l'erreur a la procedure appelante
      return -code error -errorcode $::errorCode -errorinfo $::errorInfo
   }
}

#
# stop
#    Arrete la camera Andor
#
proc ::andorusb::stop { camItem } {
   variable private

   #--- J'arrete la camera
   if { $private($camItem,camNo) != 0 } {
      cam::delete $private($camItem,camNo)
      set private($camItem,camNo) 0
   }
}

#
# dispTempAndor
#    Affiche la temperature du CCD
#
proc ::andorusb::dispTempAndor { camItem } {
   variable private
   global caption

   if { [ catch { set temp_ccd [ cam$private($camItem,camNo) temperature ] } ] == "0" } {
      set temp_ccd [ format "%+5.2f" $temp_ccd ]
      set private(ccdTemp)   "$caption(andorusb,temperature_CCD) $temp_ccd $caption(andorusb,deg_c)"
      set private(aftertemp) [ after 5000 ::andorusb::dispTempAndor $camItem ]
   } else {
      set temp_ccd ""
      set private(ccdTemp) "$caption(andorusb,temperature_CCD) $temp_ccd"
      if { [ info exists private(aftertemp) ] == "1" } {
         unset private(aftertemp)
      }
   }
}

#
# checkConfigRefroidissement
#    Configure le widget de la consigne en temperature
#
proc ::andorusb::checkConfigRefroidissement { } {
   variable private

   if { [ info exists private(frm) ] } {
      set frm $private(frm)
      if { [ winfo exists $frm ] } {
         if { $::andorusb::private(cool) == "1" } {
            pack $frm.frame2.frame6.frame10.temp -anchor center -side left -padx 5 -pady 5
            pack $frm.frame2.frame6.frame10.tempdeg -side left -fill x -padx 0 -pady 5
            $frm.frame2.frame6.frame11.ccdtemp configure -state normal
         } else {
            pack forget $frm.frame2.frame6.frame10.temp
            pack forget $frm.frame2.frame6.frame10.tempdeg
            $frm.frame2.frame6.frame11.ccdtemp configure -state disabled
         }
      }
   }
}

#
# setTempCCD
#    Procedure pour retourner la consigne de temperature du CCD
#
proc ::andorusb::setTempCCD { camItem } {
   global conf

   return "$conf(andorusb,temp)"
}

#
# setShutter
#    Procedure pour la commande de l'obturateur
#
proc ::andorusb::setShutter { camItem shutterState ShutterOptionList } {
   variable private
   global caption conf

   set conf(andorusb,foncobtu) $shutterState
   set camNo $private($camItem,camNo)

   #--- Gestion du mode de fonctionnement
   switch -exact -- $shutterState {
      0  {
         #--- j'envoie la commande a la camera
         cam$camNo shutter "opened"
         #--- je mets a jour le widget dans la fenetre de configuration si elle est ouverte
         set private(foncobtu) $caption(andorusb,obtu_ouvert)
      }
      1  {
         #--- j'envoie la commande a la camera
         cam$camNo shutter "closed"
         #--- je mets a jour le widget dans la fenetre de configuration si elle est ouverte
         set private(foncobtu) $caption(andorusb,obtu_ferme)
      }
      2  {
         #--- j'envoie la commande a la camera
         cam$camNo shutter "synchro"
         #--- je mets a jour le widget dans la fenetre de configuration si elle est ouverte
         set private(foncobtu) $caption(andorusb,obtu_synchro)
      }
   }
}

#
# explore
#    Procedure pour designer les fichiers de configuration
#
proc ::andorusb::explore { } {
   variable private
   global audace caption

   set private(config) [ tk_chooseDirectory -title "$caption(andorusb,dossier)" \
      -initialdir [ file join $audace(rep_install) bin ] -parent [ winfo toplevel $private(frm) ] ]
}

#
# getPluginProperty
#    Retourne la valeur de la propriete
#
# Parametre :
#    propertyName : Nom de la propriete
# return : Valeur de la propriete ou "" si la propriete n'existe pas
#
# binningList :      Retourne la liste des binnings disponibles
# binningXListScan : Retourne la liste des binnings en x disponibles en mode scan
# binningYListScan : Retourne la liste des binnings en y disponibles en mode scan
# dynamic :          Retourne la liste de la dynamique haute et basse
# hasBinning :       Retourne l'existence d'un binning (1 : Oui, 0 : Non)
# hasFormat :        Retourne l'existence d'un format (1 : Oui, 0 : Non)
# hasLongExposure :  Retourne l'existence du mode longue pose (1 : Oui, 0 : Non)
# hasScan :          Retourne l'existence du mode scan (1 : Oui, 0 : Non)
# hasShutter :       Retourne l'existence d'un obturateur (1 : Oui, 0 : Non)
# hasTempSensor      Retourne l'existence du capteur de temperature (1 : Oui, 0 : Non)
# hasSetTemp         Retourne l'existence d'une consigne de temperature (1 : Oui, 0 : Non)
# hasVideo :         Retourne l'existence du mode video (1 : Oui, 0 : Non)
# hasWindow :        Retourne la possibilite de faire du fenetrage (1 : Oui, 0 : Non)
# longExposure :     Retourne l'etat du mode longue pose (1: Actif, 0 : Inactif)
# multiCamera :      Retourne la possibilite de connecter plusieurs cameras identiques (1 : Oui, 0 : Non)
# name :             Retourne le modele de la camera
# product :          Retourne le nom du produit
# shutterList :      Retourne l'etat de l'obturateur (O : Ouvert, F : Ferme, S : Synchro)
#
proc ::andorusb::getPluginProperty { camItem propertyName } {
   variable private

   switch $propertyName {
      binningList      { return [ list 1x1 2x2 3x3 4x4 5x5 6x6 ] }
      binningXListScan { return [ list "" ] }
      binningYListScan { return [ list "" ] }
      dynamic          { return [ list 65535 0 ] }
      hasBinning       { return 1 }
      hasFormat        { return 0 }
      hasLongExposure  { return 0 }
      hasScan          { return 0 }
      hasShutter       { return 1 }
      hasTempSensor    { return 1 }
      hasSetTemp       { return 1 }
      hasVideo         { return 0 }
      hasWindow        { return 1 }
      longExposure     { return 1 }
      multiCamera      { return 0 }
      name             {
         if { $private($camItem,camNo) != "0" } {
            return [ cam$private($camItem,camNo) name ]
         } else {
            return ""
         }
      }
      product          {
         if { $private($camItem,camNo) != "0" } {
            return [ cam$private($camItem,camNo) product ]
         } else {
            return ""
         }
      }
      shutterList      { return [ list $::caption(andorusb,obtu_ouvert) $::caption(andorusb,obtu_ferme) $::caption(andorusb,obtu_synchro) ] }
   }
}

