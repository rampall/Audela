#
# Fichier : webcam.tcl
# Description : Configuration des cameras WebCam
# Auteurs : Michel PUJOL et Robert DELMAS
# Mise a jour $Id: webcam.tcl,v 1.12 2007-05-30 17:16:10 robertdelmas Exp $
#

namespace eval ::webcam {
}

#
# ::webcam::init
#    Initialise les variables conf(webcam,$camItem,...) et les captions
#
proc ::webcam::init { } {
   global audace conf
   variable private

   #--- Charge le fichier caption
   source [ file join $audace(rep_plugin) camera webcam webcam.cap ]

   #--- Initialise les variables de la webcams A
   foreach camItem { A B C } {
      if { ! [ info exists conf(webcam,$camItem,longuepose) ] }            { set conf(webcam,$camItem,longuepose)           "0" }
      if { ! [ info exists conf(webcam,$camItem,longueposeport) ] }        { set conf(webcam,$camItem,longueposeport)       "LPT1:" }
      if { ! [ info exists conf(webcam,$camItem,longueposelinkbit) ] }     { set conf(webcam,$camItem,longueposelinkbit)    "0" }
      if { ! [ info exists conf(webcam,$camItem,longueposestartvalue) ] }  { set conf(webcam,$camItem,longueposestartvalue) "0" }
      if { ! [ info exists conf(webcam,$camItem,mirh) ] }                  { set conf(webcam,$camItem,mirh)                 "0" }
      if { ! [ info exists conf(webcam,$camItem,mirv) ] }                  { set conf(webcam,$camItem,mirv)                 "0" }
      if { ! [ info exists conf(webcam,$camItem,channel) ] }               { set conf(webcam,$camItem,channel)              "0" }
      if { ! [ info exists conf(webcam,$camItem,ccd_N_B) ] }               { set conf(webcam,$camItem,ccd_N_B)              "0" }
      if { ! [ info exists conf(webcam,$camItem,dim_ccd_N_B) ] }           { set conf(webcam,$camItem,dim_ccd_N_B)          "1/4''" }
      if { ! [ info exists conf(webcam,$camItem,ccd) ] }                   { set conf(webcam,$camItem,ccd)                  "" }
      if { ! [ info exists conf(webcam,$camItem,videoformat) ] }           { set conf(webcam,$camItem,videoformat)          "QCIF" }
      if { ! [ info exists conf(webcam,$camItem,port) ] }                  { set conf(webcam,$camItem,port)                 "/dev/video0" }

      if { $::tcl_platform(os) == "Linux" } {
         if { ! [ info exists conf(webcam,$camItem,configWindowPosition)]} { set conf(webcam,$camItem,configWindowPosition) "+0+0"  }
         if { ! [ info exists conf(webcam,$camItem,framerate) ] }          { set conf(webcam,$camItem,framerate)            "5"  }
         if { ! [ info exists conf(webcam,$camItem,shutter) ] }            { set conf(webcam,$camItem,shutter)              "1/25"  }
         if { ! [ info exists conf(webcam,$camItem,gain) ] }               { set conf(webcam,$camItem,gain)                 "50"  }
         if { ! [ info exists conf(webcam,$camItem,autoShutter) ] }        { set conf(webcam,$camItem,autoShutter)          "1"  }
         if { ! [ info exists conf(webcam,$camItem,autoGain) ] }           { set conf(webcam,$camItem,autoGain)             "1"  }
      }
   }

   #--- definition des format video
   #--- Attention : les valeurs de private(videoFormatLabels) et private(videoFormatNames)
   #---             doivent etre dans le meme ordre
   set private(videoFormatLabels) [ list \
      "VGA - 640 x 480"  \
      "CIF - 352 x 288"  \
      "SIF - 320 x 240"  \
      "SSIF - 240 x 176" \
      "QCIF - 176 x 144" \
      "QSIF - 160 x 120" \
      "SQCIF - 128 x 96" \
   ]
   set private(videoFormatNames) [ list "VGA" "CIF" "SIF" "SSIF" "QCIF" "QSIF" "SQCIF" ]

   set private(portList) ""
}

#
# ::webcam::confToWidget
#    Copie les variables de configuration dans des variables locales
#
proc ::webcam::confToWidget { } {
   variable private
   global conf

   #--- Recupere la configuration de la WebCam dans le tableau private($camItem,...)
   foreach camItem { A B C } {
      set private($camItem,longuepose)           $conf(webcam,$camItem,longuepose)
      set private($camItem,longueposeport)       $conf(webcam,$camItem,longueposeport)
      set private($camItem,longueposelinkbit)    $conf(webcam,$camItem,longueposelinkbit)
      set private($camItem,longueposestartvalue) $conf(webcam,$camItem,longueposestartvalue)
      set private($camItem,mirh)                 $conf(webcam,$camItem,mirh)
      set private($camItem,mirv)                 $conf(webcam,$camItem,mirv)
      set private($camItem,channel)              $conf(webcam,$camItem,channel)
      set private($camItem,ccd_N_B)              $conf(webcam,$camItem,ccd_N_B)
      set private($camItem,dim_ccd_N_B)          $conf(webcam,$camItem,dim_ccd_N_B)
      set private($camItem,ccd)                  $conf(webcam,$camItem,ccd)

      #--- je copie le port correspondant de la camera
      set private($camItem,port)                 $conf(webcam,$camItem,port)

      #--- je copie le label correspondant au format
      set formatIndex [lsearch -exact $private(videoFormatNames) $conf(webcam,$camItem,videoformat)]
      set private($camItem,videoformat) [lindex $private(videoFormatLabels) $formatIndex]
   }
}

#
# ::webcam::widgetToConf
#    Copie les variables locales dans des variables de configuration
#
proc ::webcam::widgetToConf { camItem } {
   variable private
   global conf

   #--- Memorise la configuration de la WebCam dans le tableau conf(webcam,$camItem,...)
   set conf(webcam,$camItem,longuepose)           $private($camItem,longuepose)
   set conf(webcam,$camItem,longueposeport)       $private($camItem,longueposeport)
   set conf(webcam,$camItem,longueposelinkbit)    $private($camItem,longueposelinkbit)
   set conf(webcam,$camItem,longueposestartvalue) $private($camItem,longueposestartvalue)
   set conf(webcam,$camItem,mirh)                 $private($camItem,mirh)
   set conf(webcam,$camItem,mirv)                 $private($camItem,mirv)
   set conf(webcam,$camItem,channel)              $private($camItem,channel)
   set conf(webcam,$camItem,ccd_N_B)              $private($camItem,ccd_N_B)
   set conf(webcam,$camItem,dim_ccd_N_B)          $private($camItem,dim_ccd_N_B)
   set conf(webcam,$camItem,ccd)                  $private($camItem,ccd)

   #--- je copie la camera correspondant au port
   set conf(webcam,$camItem,port)                 $private($camItem,port)

   #--- je copie la format correspondant au label
   set formatIndex [lsearch -exact $private(videoFormatLabels) $private($camItem,videoformat)]
   set conf(webcam,$camItem,videoformat) [lindex $private(videoFormatNames) $formatIndex]
}

#
# ::webcam::fillConfigPage
#    Interface de configuration de la WebCam
#
proc ::webcam::fillConfigPage { frm camItem } {
   variable private
   global audace caption color

   #--- Initialise une variable locale
   set private(frm) $frm

   #--- confToWidget
   ::webcam::confToWidget

   #--- Supprime tous les widgets de l'onglet
   foreach i [ winfo children $frm ] {
      destroy $i
   }

   #--- Creation des differents frames
   frame $frm.frame1 -borderwidth 0 -relief raised
   pack $frm.frame1 -side top -fill both -expand 1

   frame $frm.frame3 -borderwidth 0 -relief raised
   pack $frm.frame3 -in $frm.frame1 -side left -fill x -expand 1 -anchor nw

   frame $frm.frame4 -borderwidth 0 -relief raised
   pack $frm.frame4 -in $frm.frame1 -side left -fill none -anchor ne

   frame $frm.frame5 -borderwidth 1 -relief solid
   pack $frm.frame5 -in $frm.frame4 -side top -fill none -anchor e

   frame $frm.frame6 -borderwidth 0 -relief raised
   pack $frm.frame6 -in $frm.frame3 -side bottom -fill x -pady 5

   frame $frm.frame7 -borderwidth 0 -relief raised
   pack $frm.frame7 -in $frm.frame3 -side bottom -fill x -pady 5

   frame $frm.frame8 -borderwidth 0 -relief raised
   pack $frm.frame8 -in $frm.frame3 -side left -fill x -pady 5

   frame $frm.frame9 -borderwidth 0 -relief raised
   pack $frm.frame9 -in $frm.frame3 -side left -fill x -padx 20

   frame $frm.frame10 -borderwidth 0 -relief raised
   pack $frm.frame10 -in $frm.frame5 -side top -fill x -pady 5

   frame $frm.frame11 -borderwidth 0 -relief raised
   pack $frm.frame11 -in $frm.frame5 -side top -fill x -pady 5

   frame $frm.frame12 -borderwidth 0 -relief raised
   pack $frm.frame12 -in $frm.frame5 -side top -fill x -pady 5

   frame $frm.frame13 -borderwidth 0 -relief raised
   pack $frm.frame13 -in $frm.frame5 -side top -fill x -pady 5

   frame $frm.frame14 -borderwidth 0 -relief raised
   pack $frm.frame14 -in $frm.frame4 -side bottom -fill x -pady 5

   frame $frm.frame15 -borderwidth 0 -relief raised
   pack $frm.frame15 -in $frm.frame14 -side right -fill x -pady 5

   #--- Definition du canal USB
   label $frm.lab1 -text "$caption(webcam,canal_usb)"
   pack $frm.lab1 -in $frm.frame8 -anchor center -side left -padx 10

   #--- Je constitue la liste des canaux USB
   set list_combobox [ list 0 1 2 3 4 5 6 7 8 9 ]

   #--- Choix du canal USB
   ComboBox $frm.port \
      -width 5        \
      -height [ llength $list_combobox ] \
      -relief sunken  \
      -borderwidth 1  \
      -textvariable ::webcam::private($camItem,channel) \
      -editable 0     \
      -values $list_combobox
   pack $frm.port -in $frm.frame8 -anchor center -side left -padx 0

   #--- Miroir en x et en y
   checkbutton $frm.mirx -text "$caption(webcam,miroir_x)" -highlightthickness 0 \
      -variable ::webcam::private($camItem,mirh)
   pack $frm.mirx -in $frm.frame9 -anchor w -side top -padx 20 -pady 10

   checkbutton $frm.miry -text "$caption(webcam,miroir_y)" -highlightthickness 0 \
      -variable ::webcam::private($camItem,mirv)
   pack $frm.miry -in $frm.frame9 -anchor w -side top -padx 20 -pady 10

   #--- Boutons de configuration de la source
   if { $::tcl_platform(os) == "Linux" } {
      label $frm.frame7.portLabel -text "Port"
      pack $frm.frame7.portLabel -anchor nw -side top -padx 10
      listbox $frm.frame7.portList -listvariable ::webcam::private(portList) -state normal -height 3
      pack $frm.frame7.portList -anchor center -padx 10 -pady 5 -ipadx 10 -expand true
      bind $frm.frame7.portList <<ListboxSelect>> "::webcam::selectPort $camItem $frm.frame7.portList"
   } else {
      button $frm.frame7.conf_webcam -text "$caption(webcam,conf_source)"
      pack $frm.frame7.conf_webcam -in $frm.frame7 -anchor center -padx 10 -pady 5 -ipadx 10 -ipady 5 -expand true
   }
   #--- Boutons de configuration du format video
   if { $::tcl_platform(os) == "Linux" } {
      label $frm.frame6.videoFormatLabel -text "$caption(webcam,format_video)"
      pack $frm.frame6.videoFormatLabel -anchor nw -side top -padx 10
      ComboBox $frm.frame6.videoFormatList \
         -width 20         \
         -height [ llength $private(videoFormatLabels) ] \
         -relief sunken    \
         -borderwidth 1    \
         -editable 0       \
         -textvariable ::webcam::private($camItem,videoformat) \
         -values $private(videoFormatLabels)
      pack $frm.frame6.videoFormatList -anchor center -padx 10 -pady 5 -ipadx 10 -expand true
   } else {
      button $frm.frame6.format_webcam -text "$caption(webcam,format_video)"
      pack $frm.frame6.format_webcam -in $frm.frame6 -anchor center -padx 10 -pady 5 -ipadx 10 -ipady 5 -expand true
   }

   #--- Option longue pose avec lien au site web de Steve Chambers
   checkbutton $frm.longuepose -highlightthickness 0 -variable ::webcam::private($camItem,longuepose) \
      -command "::webcam::checkConfigLonguePose $camItem"
   pack $frm.longuepose -in $frm.frame10 -anchor center -side left -pady 3

   set labelName [::confCam::createUrlLabel $frm.frame10 "$caption(webcam,longuepose)" "$caption(webcam,site_web_chambers)"]
   pack $labelName -anchor center -side left -pady 3

   label $frm.lab2 -text "$caption(webcam,longueposeport)"
   pack $frm.lab2 -in $frm.frame11 -anchor center -side left -padx 3 -pady 5

   #--- Bouton de configuration des liaisons
   button $frm.configure -text "$caption(webcam,configurer)" -relief raised \
      -command "::webcam::configureLinkLonguePose $camItem ; \
         ::confLink::run ::webcam::private($camItem,longueposeport) \
         { parallelport quickremote } \"- $caption(webcam,longuepose1) - $caption(webcam,camera)\""
   pack $frm.configure -in $frm.frame11 -side left -pady 0 -ipadx 10 -ipady 1

   #--- Je constitue la liste des liaisons pour la longuepose
   set list_combobox [ ::confLink::getLinkLabels { "parallelport" "quickremote" } ]

   #--- Je verifie le contenu de la liste
   if { [ llength $list_combobox ] > 0 } {
      #--- Si la liste n'est pas vide,
      #--- Je verifie que la valeur par defaut existe dans la liste
      if { [ lsearch -exact $list_combobox $private($camItem,longueposeport) ] == -1 } {
         #--- Si la valeur par defaut n'existe pas dans la liste,
         #--- Je la remplace par le premier item de la liste
         set private($camItem,longueposeport) [ lindex $list_combobox 0 ]
      }
   } else {
      #--- Si la liste est vide
      #--- Je desactive l'option longue pose
      set private($camItem,longueposeport) ""
      set private($camItem,longuepose) 0
      #--- J'empeche de selectionner l'option longue
      $frm.longuepose configure -state disable
   }

   #--- Choix du port ou de la liaison
   ComboBox $frm.lpport \
      -width 13         \
      -height [ llength $list_combobox ] \
      -relief sunken    \
      -borderwidth 1    \
      -editable 0       \
      -textvariable ::webcam::private($camItem,longueposeport) \
      -values $list_combobox \
      -modifycmd "::webcam::configureLinkLonguePose $camItem"
   pack $frm.lpport -in $frm.frame11 -anchor center -side right -padx 10 -pady 5

   label $frm.lab3 -text "$caption(webcam,longueposebit)"
   pack $frm.lab3 -in $frm.frame12 -anchor center -side left -padx 3 -pady 5

   set list_combobox [ list 0 1 2 3 4 5 6 7 ]
   ComboBox $frm.longueposelinkbit \
      -width 5                     \
      -height [ llength $list_combobox ] \
      -relief sunken               \
      -borderwidth 1               \
      -textvariable ::webcam::private($camItem,longueposelinkbit) \
      -editable 0                  \
      -values $list_combobox
   pack $frm.longueposelinkbit -in $frm.frame12 -anchor center -side right -padx 10 -pady 5

   label $frm.lab4 -text "$caption(webcam,longueposestart)"
   pack $frm.lab4 -in $frm.frame13 -anchor center -side left -padx 3 -pady 5

   #entry $frm.longueposestartvalue -width 4 -textvariable ::webcam::private($camItem,longueposestartvalue) -justify center
   set longuePoseStartList [list "0" "1"]
   ComboBox $frm.longueposestartvalue \
      -width 4         \
      -height [ llength $longuePoseStartList ] \
      -relief sunken    \
      -borderwidth 1    \
      -editable 0       \
      -textvariable ::webcam::private($camItem,longueposestartvalue) \
      -values $longuePoseStartList
   pack $frm.longueposestartvalue -in $frm.frame13 -anchor center -side right -padx 10 -pady 5

   #--- WebCam modifiee avec un capteur Noir et Blanc
   checkbutton $frm.ccd_N_B -text "$caption(webcam,ccd_N_B)" -highlightthickness 0 \
      -variable ::webcam::private($camItem,ccd_N_B) -command "::webcam::checkConfigCCDN&B $camItem"
   pack $frm.ccd_N_B -in $frm.frame14 -anchor center -side left -pady 3 -pady 8

   set list_combobox [ list 1/4'' 1/3'' 1/2'' ]
   ComboBox $frm.dim_ccd_N_B \
      -width 5               \
      -height [ llength $list_combobox ] \
      -relief sunken         \
      -borderwidth 1         \
      -editable 0            \
      -textvariable ::webcam::private($camItem,dim_ccd_N_B) \
      -modifycmd "::webcam::checkConfigCCDN&B $camItem" \
      -values $list_combobox
   pack $frm.dim_ccd_N_B -in $frm.frame15 -anchor center -side right -padx 10 -pady 5

   #--- Frame du site web web officiel des WebCams
   frame $frm.frame2 -borderwidth 0 -relief raised

      label $frm.frame2.lab103 -text "$caption(webcam,titre_site_web)"
      pack $frm.frame2.lab103 -side top -fill x -pady 2

      set labelName [::confCam::createUrlLabel $frm.frame2 "$caption(webcam,site_web_ref)" \
         "$caption(webcam,site_web_ref)"]
      pack $labelName -side top -fill x -pady 2

   pack $frm.frame2 -side bottom -fill x -pady 2

   #--- Gestion des widgets actifs/inactifs
   ::webcam::ConfigWebCam $camItem

   #--- Mise a jour dynamique des couleurs
   ::confColor::applyColor $frm
}

#
# ::webcam::configureCamera
#    Configure la WebCam en fonction des donnees contenues dans les variables conf(webcam,$camItem,...)
#
proc ::webcam::configureCamera { camItem } {
   global caption conf confCam

   set camNo [ cam::create webcam "$conf(webcam,$camItem,port)" -channel $conf(webcam,$camItem,channel) \
      -lpport $conf(webcam,$camItem,longueposeport) -name WEBCAM -ccd $conf(webcam,$camItem,ccd) ]
   console::affiche_erreur "$caption(webcam,canal_usb) ($caption(webcam,camera))\
      $caption(webcam,2points) $conf(webcam,$camItem,channel)\n"
   console::affiche_erreur "$caption(webcam,longuepose) $caption(webcam,2points)\
      $conf(webcam,$camItem,longuepose)\n"
   console::affiche_saut "\n"
   set confCam($camItem,camNo) $camNo
   #--- J'associe le buffer de la visu
   set bufNo [visu$confCam($camItem,visuNo) buf]
   cam$camNo buf $bufNo
   #--- Je configure l'oriention des miroirs par defaut
   cam$camNo mirrorh $conf(webcam,$camItem,mirh)
   cam$camNo mirrorv $conf(webcam,$camItem,mirv)

   #--- Je configure le format video (pour Linux uniquement)
   if { $::tcl_platform(os) == "Linux" } {
      cam$camNo videoformat $conf(webcam,$camItem,videoformat)
      cam$camNo framerate $conf(webcam,$camItem,framerate)
   }
   #--- Je cree la thread dediee a la camera
   set confCam($camItem,threadNo) [::confCam::createThread $camNo $bufNo $confCam($camItem,visuNo)]

   #--- Parametrage des longues poses
   if { $conf(webcam,$camItem,longuepose) == "1" } {
      switch [ ::confLink::getLinkNamespace $conf(webcam,$camItem,longueposeport) ] {
         parallelport {
            #--- Je cree la liaison longue pose
            set linkNo [ ::confLink::create $conf(webcam,$camItem,longueposeport) "cam$camNo" "longuepose" "bit $conf(webcam,$camItem,longueposelinkbit)" ]
            #---
            cam$camNo longuepose 1
            cam$camNo longueposelinkno $linkNo
            cam$camNo longueposelinkbit $conf(webcam,$camItem,longueposelinkbit)
            cam$camNo longueposestartvalue $conf(webcam,$camItem,longueposestartvalue)
            #--- si  longueposestartvalue=0 alors longueposestovalue=1
            #--- si  longueposestartvalue=1 alors longueposestovalue=0
            cam$camNo longueposestopvalue [expr $conf(webcam,$camItem,longueposestartvalue)==0]
         }
         quickremote {
            #--- Je cree la liaison longue pose
            set linkNo [ ::confLink::create $conf(webcam,$camItem,longueposeport) "cam$camNo" "longuepose" "bit $conf(webcam,$camItem,longueposelinkbit)" ]
            #---
            cam$camNo longuepose 1
            cam$camNo longueposelinkno $linkNo
            cam$camNo longueposelinkbit $conf(webcam,$camItem,longueposelinkbit)
            cam$camNo longueposestartvalue $conf(webcam,$camItem,longueposestartvalue)
            #--- si  longueposestartvalue=0 alors longueposestovalue=1
            #--- si  longueposestartvalue=1 alors longueposestovalue=0
            cam$camNo longueposestopvalue [expr $conf(webcam,$camItem,longueposestartvalue)==0]
         }
      }

      #--- J'ajoute la commande de liaison longue pose dans la thread de la camera
      if { $confCam($camItem,threadNo) != 0 && [cam$camNo longueposelinkno] != 0} {
         thread::copycommand $confCam($camItem,threadNo) "link[cam$camNo longueposelinkno]"
      }

   } else {
      #--- Pas de liaison longue pose
      cam$camNo longuepose 0
   }

   #---
   ::confVisu::visuDynamix $confCam($camItem,visuNo) 255 -255
}

#------------------------------------------------------------
#  selectPort
#     selectionne un port
#
#  return null
#------------------------------------------------------------
proc ::webcam::selectPort { camItem tklist } {
   variable private

   set index [$tklist curselection]
   set private($camItem,port) [lindex [lindex $private(portList) $index] 0]
}

#
# ::webcam::stop
#    Arrete la WebCam
#
proc ::webcam::stop { camNo camItem } {
   global conf
   variable private

   if { [ info exists private(frm)] } {
      set frm $private(frm)
      #--- Boutons de configuration de la WebCam inactif
      if { [ winfo exists $frm] } {
         if { $::tcl_platform(os) == "Linux" } {
            $frm.frame6.videoFormatList configure -state disabled
         } else {
            $frm.frame7.conf_webcam configure -state disabled
            $frm.frame6.format_webcam configure -state disabled
         }
      }
      #--- Je ferme la liaison longuepose
      if { $conf(webcam,$camItem,longuepose) == 1 } {
         ::confLink::delete $conf(webcam,$camItem,longueposeport) "cam$camNo" "longuepose"
      }
   }
}

#
# ::webcam::ConfigWebCam
#    Configure les widgets de configuration de la WebCam
#
proc ::webcam::ConfigWebCam { camItem } {
   variable private
   global audace conf confCam

   if { [ info exists private(frm)] } {
      set frm $private(frm)
      if { [ winfo exists $frm ] } {
         if { [ ::confCam::getProduct $confCam($camItem,camNo) ] == "webcam" } {
            #--- Boutons de configuration de la WebCam actif
            if { $::tcl_platform(os) == "Linux" } {
               $frm.frame6.videoFormatList configure -state normal
            } else {
               $frm.frame7.conf_webcam configure -state normal -command "cam$confCam($camItem,camNo) videosource"
               $frm.frame6.format_webcam configure -state normal -command "cam$confCam($camItem,camNo) videoformat window"
            }
         } else {
            #--- Boutons de configuration de la WebCam inactif
            if { $::tcl_platform(os) == "Linux" } {
               $frm.frame6.videoFormatList configure -state disabled
            } else {
               $frm.frame7.conf_webcam configure -state disabled
               $frm.frame6.format_webcam configure -state disabled
            }
         }
         #--- Configure les widgets associes a la longue pose
         ::webcam::checkConfigLonguePose $camItem
         #--- Configure les widgets associes au choix du CCD
         ::webcam::checkConfigCCDN&B $camItem

         #--- actualise la liste des ports
         if { $::tcl_platform(os) == "Linux" } {
            #--- je remplis la liste avec la liste des ports
            if { "$private(portList)" == "" } {
               set private(portList) [lsort -dictionary [ glob -nocomplain /dev/video? ]]
            }
            #--- je selectionne le port courant
             set index [lsearch $private(portList) $private($camItem,port)]
             if { $index != -1 } {
                $frm.frame7.portList selection set $index
             }
         }
      }
   }
}

#
# ::webcam::checkConfigLonguePose
#    Configure les widgets de configuration de la longue pose
#
proc ::webcam::checkConfigLonguePose { camItem } {
   variable private

   if { [ info exists private(frm)] } {
      set frm $private(frm)
      if { [ winfo exists $frm ] } {
         if { $private($camItem,longuepose) == "1" } {
            #--- Widgets de configuration de la longue pose actifs
            $frm.lpport configure -state normal
            $frm.configure configure -state normal
            $frm.longueposelinkbit configure -state normal
            $frm.longueposestartvalue configure -state normal
         } else {
            #--- Widgets de configuration de la longue pose inactifs
            $frm.lpport configure -state disabled
            $frm.configure configure -state disabled
            $frm.longueposelinkbit configure -state disabled
            $frm.longueposestartvalue configure -state disabled
         }
      }
   }
}

#
# ::webcam::checkConfigCCDN&B
#    Configure les widgets de configuration du choix du CCD
#
proc ::webcam::checkConfigCCDN&B { camItem } {
   variable private
   global audace

   if { [ info exists private(frm)] } {
      set frm $private(frm)
      if { [ winfo exists $frm ] } {
         if { $::webcam::private($camItem,ccd_N_B) == "1" } {
            if { $::webcam::private($camItem,dim_ccd_N_B) == "1/4''" } {
               set ::webcam::private($camItem,ccd) "ICX098BL-6"
            } elseif { $::webcam::private($camItem,dim_ccd_N_B) == "1/3''" } {
               set ::webcam::private($camItem,ccd) "ICX424AL-6"
            } elseif { $::webcam::private($camItem,dim_ccd_N_B) == "1/2''" } {
               set ::webcam::private($camItem,ccd) "ICX414AL-6"
            }
            pack $frm.frame15 -in $frm.frame14 -side right -fill x -pady 5
         } else {
            set ::webcam::private($camItem,ccd) "ICX098BQ-A"
            pack forget $frm.frame15
         }
      }
   }
}

#
# ::webcam::configureLinkLonguePose
#    Positionne la liaison sur celle qui vient d'etre selectionnee pour la longue pose
#
proc ::webcam::configureLinkLonguePose { camItem } {
   variable private

   #--- Je positionne startvalue par defaut en fonction du type de liaison
   if { [ ::confLink::getLinkNamespace $private($camItem,longueposeport) ] == "parallelport" } {
      set private($camItem,longueposestartvalue) "0"
   } elseif { [ ::confLink::getLinkNamespace $private($camItem,longueposeport) ] == "quickremote" } {
      set private($camItem,longueposestartvalue) "1"
   }
}

#
# ::webcam::getBinningList
#    Retourne la liste des binnings disponibles de la camera
#
proc ::webcam::getBinningList { } {
   set binningList { 1x1 }
   return $binningList
}

#
# ::webcam::getBinningListScan
#    Retourne la liste des binnings disponibles pour les scans de la camera
#
proc ::webcam::getBinningListScan { } {
   set binningListScan { }
   return $binningListScan
}

#
# ::webcam::getLongExposure
#    Retourne 1 si le mode longue pose est activ�
#    Sinon retourne 0
#
proc ::webcam::getLongExposure { camItem } {
   return $::conf(webcam,$camItem,longuepose)
}

#
# ::webcam::getShutterOption
#    Retourne le mode de fonctionnement de l'obturateur (O : Ouvert , F : Ferme , S : Synchro)
#
proc ::webcam::getShutterOption { } {
   set ShutterOptionList { }
   return $ShutterOptionList
}

#
# ::webcam::hasCapability
#    Retourne "la valeur de la propriete"
#
#  Parametres :
#     camNo      : Numero de la camera
#     capability : Fonctionnalite de la camera
#
proc ::webcam::hasCapability { camNo capability } {
   switch $capability {
      window { return 0 }
   }
}

#
# ::webcam::hasLongExposure
#    Retourne le mode longue pose de la camera (1 : oui , 0 : non)
#
proc ::webcam::hasLongExposure { } {
   return 1
}

#
# ::webcam::hasVideo
#    Retourne le mode video de la camera (1 : oui , 0 : non)
#
proc ::webcam::hasVideo { } {
   return 1
}

#
# ::webcam::hasScan
#    Retourne le mode scan de la camera (1 : Oui , 0 : Non)
#
proc ::webcam::hasScan { } {
   return 0
}

#
# ::webcam::hasShutter
#    Retourne la presence d'un obturateur (1 : Oui , 0 : Non)
#
proc ::webcam::hasShutter { } {
   return 0
}

# ========= Namespace de la fenetre de configuration ========

namespace eval ::webcam::config {
}

#------------------------------------------------------------
# ::webcam::config::run
#    affiche la fenetre de configuration de l'autoguidage
#------------------------------------------------------------
proc ::webcam::config::run { visuNo camItem } {
   variable private

   if { $::tcl_platform(os) == "Linux" } {
      set private($visuNo,toplevel) "[confVisu::getBase $visuNo].webcamconfig"
      set private($visuNo,camItem) $camItem

      set private(frameRateList) [list "5" "10" "15" "20" "25" "30" "50"]
      set private(shutterList)   [list "1/5" "1/10" "1/15" "1/20" "1/25" "1/33" "1/50" "1/100" "1/250" "1/500" "1/1000" "1/2500" "1/5000" "1/10000"]

      #--- j'affiche la fenetre de configuration
      if { [winfo exists $private($visuNo,toplevel)] == 0 } {
         ::confGenerique::run $visuNo $private($visuNo,toplevel) "::webcam::config" $visuNo nomodal
         wm transient $private($visuNo,toplevel) [winfo parent $private($visuNo,toplevel) ]
         wm geometry $private($visuNo,toplevel) $::conf(webcam,$camItem,configWindowPosition)
      } else {
         focus $private($visuNo,toplevel)
      }
      set result 0
   } else {
      set result [ catch { after 10 "cam[ ::confVisu::getCamNo $visuNo ] videosource" } ]
   }
   return $result
}

#------------------------------------------------------------
# ::webcam::config::closeWindow
#    ferme la fenetre de configuration
#------------------------------------------------------------
proc ::webcam::config::closeWindow { visuNo } {
   variable private

   #--- j'enregistre la position de la fentre de configuration
   set geometry [ wm geometry $private($visuNo,toplevel)]
   set deb [ expr 1 + [ string first + $geometry ] ]
   set fin [ string length $geometry ]
   set ::conf(webcam,$private($visuNo,camItem),configWindowPosition) "+[ string range $geometry $deb $fin ]"
}

#------------------------------------------------------------
# ::webcam::config::getLabel
#    retourne le nom de la fenetre de configuration
#------------------------------------------------------------
proc ::webcam::config::getLabel { } {
   global caption

   return "$caption(webcam,configurer) $caption(webcam,camera)"
}

#------------------------------------------------------------
# ::webcam::config::fillConfigPage { }
#    fenetre de configuration de la camera
#    return rien
#------------------------------------------------------------
proc ::webcam::config::fillConfigPage { frm visuNo } {
   variable private
   global caption

   set private($visuNo,This) $frm

   #--- j'initialise les variables des widgets
   set private($visuNo,framerate)   $::conf(webcam,$private($visuNo,camItem),framerate)
   set private($visuNo,shutter)     $::conf(webcam,$private($visuNo,camItem),shutter)
   set private($visuNo,gain)        $::conf(webcam,$private($visuNo,camItem),gain)
   set private($visuNo,autoShutter) $::conf(webcam,$private($visuNo,camItem),autoShutter)
   set private($visuNo,autoGain)    $::conf(webcam,$private($visuNo,camItem),autoGain)

   TitleFrame $frm.shutter -borderwidth 2 -relief ridge -text "$caption(webcam,shutter)"
      checkbutton $frm.shutter.auto -text "$caption(webcam,auto)" -highlightthickness 0 \
         -variable ::webcam::config::private($visuNo,autoShutter) \
         -command "::webcam::config::onSetAutoShutter $visuNo $frm.shutter.scale"
      pack $frm.shutter.auto -in [$frm.shutter getframe] -anchor w -side top -fill none -expand 0
      #listbox $frm.shutter.list -state normal -width [llength $private(shutterList)] -listvariable ::webcam::config::private(shutterList)
      scale $frm.shutter.scale -from "0." -to "100." \
         -orient vertical -showvalue true -bigincrement 10 -tickinterval 5 -resolution 1 -width 8 \
         -borderwidth 1 -relief groove \
         -variable ::webcam::config::private($visuNo,shutter) \
         -command "::webcam::config::onSelectShutter $visuNo $frm.shutter.scale"
      pack $frm.shutter.scale -in [$frm.shutter getframe] -anchor w -side top -fill y -expand 1
      #bind $frm.shutter.list <<ListboxSelect>> "::webcam::config::onSelectShutter $visuNo $frm.shutter.list"
   pack $frm.shutter -anchor w -side left -fill y -expand 0

   TitleFrame $frm.gain -borderwidth 2 -relief ridge -text "$caption(webcam,gain)"
      checkbutton $frm.gain.auto -text "$caption(webcam,auto)" -highlightthickness 0 \
         -variable ::webcam::config::private($visuNo,autoGain) \
         -command "::webcam::config::onSetAutoGain $visuNo $frm.gain.scale"
      pack $frm.gain.auto -in [$frm.gain getframe] -anchor w -side top -fill none -expand 0
      scale $frm.gain.scale -from "0" -to "100" \
         -orient vertical -showvalue true -tickinterval 1 -resolution 1 -width 8 \
         -borderwidth 1 -relief groove \
         -variable ::webcam::config::private($visuNo,gain) \
         -command "::webcam::config::onSelectGain $visuNo $frm.gain.scale"
      pack $frm.gain.scale -in [$frm.gain getframe] -anchor w -side top -fill y -expand 1
   pack $frm.gain -anchor w -side left -fill y -expand 0

   TitleFrame $frm.framerate -borderwidth 2 -relief ridge -text "$caption(webcam,framerate)"
      listbox $frm.framerate.list -state normal  -width 8 -listvariable ::webcam::config::private(frameRateList)
      pack $frm.framerate.list -in [$frm.framerate getframe] -anchor w -side top -fill y -expand 0
      bind $frm.framerate.list <<ListboxSelect>> "::webcam::config::onSelectFrameRate $visuNo $frm.framerate.list"
   pack $frm.framerate -anchor w -side left -fill y -expand 0

   pack $frm -fill y -expand 1

   #--- Mise a jour dynamique des couleurs
   ::confColor::applyColor $frm

   $frm.framerate.list selection set [lsearch $private(frameRateList) $private($visuNo,framerate)]
   $frm.shutter.scale set $private($visuNo,shutter)
   $frm.gain.scale    set $private($visuNo,gain)
   ::webcam::config::onSetAutoShutter $visuNo $frm.shutter.scale
   ::webcam::config::onSetAutoGain $visuNo $frm.gain.scale
}

#------------------------------------------------------------
# ::webcam::config::onSelectFrameRate
#    selectionne le nombre d'images par seconde
#
#  return null
#------------------------------------------------------------
proc ::webcam::config::onSelectFrameRate { visuNo tklist } {
   variable private

   #--- je copie la valeur selectionnee dans la variable private
   set index [$tklist curselection]
   if { $index != "" } {
      set private($visuNo,framerate) [$tklist get $index ]
      set camItem $private($visuNo,camItem)
      set camNo $::confCam($camItem,camNo)
      if { $private($visuNo,framerate) != $::conf(webcam,$camItem,framerate) } {
         set catchResult [ catch { cam$camNo framerate $private($visuNo,framerate) } catchMessage ]
         if { $catchResult == 0 } {
            set ::conf(webcam,$private($visuNo,camItem),framerate) $private($visuNo,framerate)
         } else {
            tk_messageBox -message "$catchMessage" -title [::webcam::config::getLabel] -icon error
            set private($visuNo,framerate) $::conf(webcam,$private($visuNo,camItem),framerate)
            $private($visuNo,This).framerate.list selection clear 0 end
            $private($visuNo,This).framerate.list selection set [lsearch $private(frameRateList) $private($visuNo,framerate)]
         }
      }
   }
}

#------------------------------------------------------------
# ::webcam::config::onSelectShutter
#    selectionne la vitesse d'obturation
#
#    A negative value sets the shutter speed to automatic
#    (controlled by the camera's firmware).
#    A value of 0..65535 will set manual mode, where the values
#    have been calibrated such that 65535 is the longest possible
#    exposure time. It is not a linear scale, where a value of '1'
#    is 1/65536th of a second, etc.
#
#  return null
#------------------------------------------------------------
proc ::webcam::config::onSelectShutter { visuNo tklist value } {
   variable private

   set camItem $private($visuNo,camItem)
   if { $private($visuNo,shutter) != $::conf(webcam,$camItem,shutter) } {
      set camNo $::confCam($camItem,camNo)
      if { $private($visuNo,autoShutter) == 0 } {
         #--- j'ajoute un point pour transformer en valeur decimale
         append value "."
         set value [expr $value]
         #--- je convertis le pourcentage en fraction de 65535
         set value [expr int( $value * 65535. / 100. ) ]
         cam$camNo setvideoparameter -shutter $value
         #--- j'attends un peu pour ne pas saturer
         after 100
      } else {
         cam$camNo setvideoparameter -shutter "-1"
      }
      set ::conf(webcam,$camItem,shutter) $private($visuNo,shutter)
   }
}

#------------------------------------------------------------
# ::webcam::config::onSelectGain
#    selectionne le gain
#
#  return null
#------------------------------------------------------------
proc ::webcam::config::onSelectGain { visuNo tkscale value } {
   variable private

   set camItem $private($visuNo,camItem)

   if { $private($visuNo,gain) != $::conf(webcam,$camItem,gain) } {
      set camNo $::confCam($camItem,camNo)
      if { $private($visuNo,autoGain) == 0 } {
         #--- j'ajoute un point pour transformer en valeur decimale
         append value "."
         #--- je convertis en fraction de 65535
         set value [expr int( 65535. * $value / 100.) ]
         cam$camNo setvideoparameter -gain $value
         #--- j'attends un peu pour ne pas saturer
         after 100
      } else {
         cam$camNo setvideoparameter -gain "-1"
      }
      set ::conf(webcam,$camItem,gain) $private($visuNo,gain)
   }
}

#------------------------------------------------------------
# ::webcam::config::onSetAutoShutter
#    change le mode automatique du gain
#
#  return null
#------------------------------------------------------------
proc ::webcam::config::onSetAutoShutter { visuNo tklist } {
   variable private

   if { $private($visuNo,autoShutter) == 0 } {
      $tklist configure -state normal
   } else {
      $tklist configure -state disabled
   }

   set camItem $private($visuNo,camItem)

   if { $private($visuNo,autoShutter) != $::conf(webcam,$camItem,autoShutter)  } {
      set camNo $::confCam($camItem,camNo)
      if { $private($visuNo,autoShutter) == 0 } {
         #--- j'ajoute un point pour transformer en valeur decimale
         set value $::conf(webcam,$camItem,shutter)
         append value "."
         #--- je convertis le pourcentage en fraction de 65535
         set value [expr $value]
         set value [expr int( $value * 65535. / 100. ) ]
         cam$camNo setvideoparameter -shutter $value
      } else {
         cam$camNo setvideoparameter -shutter "-1"
      }
      set ::conf(webcam,$camItem,autoShutter) $private($visuNo,autoShutter)
   }
}

#------------------------------------------------------------
# ::webcam::config::onSetGainAuto
#    change le mode automatique du gain
#
#  return null
#------------------------------------------------------------
proc ::webcam::config::onSetAutoGain { visuNo tkscale } {
   variable private

   if { $private($visuNo,autoGain) == 0 } {
      $tkscale configure -state normal
   } else {
      $tkscale configure -state disabled
   }

   set camItem $private($visuNo,camItem)

    if { $private($visuNo,autoGain) != $::conf(webcam,$camItem,autoGain) } {
      set camNo $::confCam($camItem,camNo)
      if { $private($visuNo,autoGain) == 0 } {
         #--- j'ajoute un point pour transformer en valeur decimale
         set value $::conf(webcam,$camItem,autoGain)
         append value "."
         #--- je convertis en fraction de 65535
         set value [expr int( $value * 65535./ 100.) ]
         cam$camNo setvideoparameter -gain $value
      } else {
         cam$camNo setvideoparameter -gain "-1"
      }
      set ::conf(webcam,$camItem,autoGain) $private($visuNo,autoGain)
   }
}

