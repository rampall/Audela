#
# Fichier : acqdslr.tcl
# Description : Outil d'acquisition pour APN Canon
# Auteur : Raymond Zachantke
# Mise a jour $Id: acqdslr.tcl,v 1.1 2010-04-19 15:12:42 robertdelmas Exp $
#

#============================================================
# Declaration du namespace acqdslr
#    initialise le namespace
#============================================================
namespace eval ::acqdslr {
   package provide acqdslr 1.0
   package require audela 1.4.0

   #--- Chargement des captions pour recuperer le titre utilise par getPluginLabel
   source [ file join [file dirname [info script]]  acqdslr.cap ]

   #------------------------------------------------------------
   # getPluginTitle
   #    retourne le titre du plugin dans la langue de l'utilisateur
   #------------------------------------------------------------
   proc getPluginTitle { } {
      global caption

      return "$caption(acqdslr,title)"
   }

   #------------------------------------------------------------
   # getPluginHelp
   #    retourne le nom du fichier d'aide principal
   #------------------------------------------------------------
   proc getPluginHelp { } {
      return "acqdslr.htm"
   }

   #------------------------------------------------------------
   # getPluginType
   #    retourne le type de plugin
   #------------------------------------------------------------
   proc getPluginType { } {
      return "tool"
   }

   #------------------------------------------------------------
   # getPluginDirectory
   #    retourne le type de plugin
   #------------------------------------------------------------
   proc getPluginDirectory { } {
      return "acqdslr"
   }

   #------------------------------------------------------------
   # getPluginOS
   #    retourne le ou les OS de fonctionnement du plugin
   #------------------------------------------------------------
   proc getPluginOS { } {
      return [ list Windows Linux Darwin ]
   }

   #------------------------------------------------------------
   # getPluginProperty
   #    retourne la valeur de la propriete
   #
   # parametre :
   #    propertyName : nom de la propriete
   # return : valeur de la propriete ou "" si la propriete n'existe pas
   #------------------------------------------------------------
   proc getPluginProperty { propertyName } {
      switch $propertyName {
         function     { return "acquisition" }
         subfunction { return "acqdslr" }
         display      { return "panel" }
      }
   }

   #------------------------------------------------------------
   # initPlugin
   #    initialise le plugin
   #------------------------------------------------------------
   proc initPlugin { tkbase } {

   }

   #------------------------------------------------------------
   # createPluginInstance
   #    cree une nouvelle instance de l'outil
   #------------------------------------------------------------
   proc createPluginInstance { { in "" } { visuNo 1 } } {
      global panneau

      #--- Mise en place de l'interface graphique
      createPanel $in.acqdslr
   }

   #------------------------------------------------------------
   # deletePluginInstance
   #    suppprime l'instance du plugin
   #------------------------------------------------------------
   proc deletePluginInstance { visuNo } {

   }

   #------------------------------------------------------------
   # createPanel
   #    prepare la creation de la fenetre de l'outil
   #------------------------------------------------------------
   proc createPanel { this } {
      global audace panneau
      variable This

      #---
      set This $this

      #---
      set panneau(acqdslr,base)  "$this"

      acqdslrBuildIF $This
   }

   #------------------------------------------------------------
   # startTool
   #    affiche la fenetre de l'outil
   #------------------------------------------------------------
   proc startTool { visuNo } {
      variable This

      pack $This -side left -fill y
   }

   #------------------------------------------------------------
   # stopTool
   #    masque la fenetre de l'outil
   #------------------------------------------------------------
   proc stopTool { visuNo } {
      variable This

      pack forget $This
   }

#--   fin du namespace
}

#------------------------------------------------------------
# acqdslrBuildIF
#    cree la fenetre de l'outil
#------------------------------------------------------------
proc acqdslrBuildIF { This } {
   global audace panneau caption color
   variable Dslr

   #---
   frame $This -borderwidth 2 -relief groove
   pack $This
      #--- Frame du titre
     frame $This.fra1 -borderwidth 2 -relief groove
         #--- Bouton du titre
         Button $This.fra1.but -borderwidth 1 \
            -text "$caption(acqdslr,help_titre1)\n$caption(acqdslr,title)" \
            -command "::audace::showHelpPlugin [ ::audace::getPluginTypeDirectory [ ::acqdslr::getPluginType ] ] \
               [ ::acqdslr::getPluginDirectory ] [ ::acqdslr::getPluginHelp ]"
         pack $This.fra1.but -in $This.fra1 -anchor center -expand 1 -fill both -side top -ipadx 5
      DynamicHelp::add $This.fra1.but -text $caption(acqdslr,help_titre)
      pack $This.fra1 -side top -fill x

      #--   le frame cantonnant le DSLR et le bouton GO
      set Dslr $This.dslr

      searchInfoDslr

      #--- Mise ajour dynamique des couleurs
      ::confColor::applyColor $This
   }

# ==============================================================================================
# ==============================================================================================

   #########################################################################
   #--   Au lancement collecte les donnees sur la camera et les memorise   #
   #########################################################################
   proc searchInfoDslr {} {
      global audace panneau caption

      #--   ouvre la liste des types de declenchement
      set panneau(acqdslr,poseLabels) [ list "<30" ]

      #-- liste les stockages
      set panneau(acqdslr,stockLabels) \
         [ list "$caption(acqdslr,stock,cf)" "$caption(acqdslr,stock,home)" ]

      #--   complete l'etat
      lappend panneau(acqdslr,status) [ list "0" ]

      #--   valeur par defaut de la liste des formats
      set panneau(acqdslr,qualityLabels) [ list "RAW" ]

      #--   liste les modes de prise de vue
      set panneau(acqdslr,bracketLabels) \
         [ list "$caption(acqdslr,bracket,one)" \
            "$caption(acqdslr,bracket,serie)" \
            "$caption(acqdslr,bracket,continu)" ]

      #--   liste les pas du bracketing
      set panneau(acqdslr,stepLabels) \
         [ list "+6" "+5" "+4" "+3" "+2" "+1" \
            0 "-1" "-2" "-3" "-4" "-5" "-6" ]

      #--   liste les vitesses standards
      set panneau(acqdslr,exptimeLabels) [ list "30" "25" "20" "15" "13" \
         "10" "8" "6" "5" "4" "3.2" "2.5" "2.0" "1.6" "1.3" \
         "1.0"  "0.8" "0.6" "0.5" "0.4" "0.3" "1/4" "1/5" "1/6" "1/8" \
         "1/10" "1/13" "1/15" "1/20" "1/25" "1/30" "1/40" "1/50" "1/60" "1/80" \
         "1/100" "1/125" "1/160" "1/200" "1/250" "1/320" "1/400" "1/500" "1/640" "1/800" \
         "1/1000" "1/1250" "1/1600" "1/2000" "1/2500" "1/3200" "1/4000" "1/5000" "1/6400" "1/8000" ]

      set panneau(acqdslr,exptimeValues) [ list "30" "25" "20" \
         "15" "13" "10" "8" "6" "5" "4" "3.2" "2.5" "2" "1.6" "1.3" \
         "1" "0.8" "0.6" "0.5" "0.4" "0.3" "0.25" "0.2" "0.16667" "0.125" \
         "0.1" "0.076923" "0.06667" "0.05" "0.04" "0.03333" "0.025" "0.02" \
         "0.01667" "0.0125" "0.01" "0.008" "0.00625" "0.005" "0.004" \
         "0.003125" "0.0025" "0.002" "0.0015625" "0.00125" ".001" "0.0008" \
         "0.000625" "0.0005" "0.0004" "0.0003125" "0.00025" "0.0002" "0.00015625" "0.000125" ]

      setCamProperty

      apnBuildIF
   }

   ###################################################################
   #-- Complete le panneau d'acquisition                             #
   ###################################################################
   proc apnBuildIF {} {
      global panneau caption
      variable Dslr

      #--   le frame cantonnant le DSLR et le bouton GO
      frame $Dslr -borderwidth 1 -relief sunken
      pack $Dslr -side top -fill x

      ::blt::table $Dslr

      #---   construit les menubutton
      foreach var { stock quality step bracket pose } {
         buildMenuButton $var
      }
      $Dslr.step configure -textvar "" -text $caption(acqdslr,step) -width 2
      $Dslr.pose configure -width 2

      #--   label pour afficher le format de l'image
      label $Dslr.format -textvariable panneau(acqdslr,format) \
         -width 4 -borderwidth 1

      #--   construit le bouton 'Test'
      button $Dslr.test -relief raised -width 10 -borderwidth 2 \
         -text $caption(acqdslr,test) -command "testTime"

      #--   construit les entrees de donnees
      foreach var { nom nb_poses delai intervalle } {
         buildLabelEntry $var
      }
      $Dslr.nom configure -labelwidth 6 -width 10
      bind $Dslr.nom <Leave> { test_bracketing ; test_nom }

      #--   la combobox pour le temps de pose
      ComboBox $Dslr.exptime -borderwidth 1 -width 8 -relief sunken \
         -height 10 -justify center \
         -textvariable panneau(acqdslr,time) \
         -modifycmd "test_bracketing ; test_exptime"
      bind $Dslr.exptime <Leave> { test_bracketing ; test_exptime }

      #--   label pour afficher les etapes
      label $Dslr.state -textvariable panneau(acqdslr,action) \
         -width 14 -borderwidth 2 -relief sunken

      #--   configure le bouton de lancement d'acquisition
      button $Dslr.but1 -borderwidth 2 -text "$caption(acqdslr,go)" \
         -command "shoot"

      #--   packaging des widgets
      ::blt::table $Dslr \
         $Dslr.stock 0,0 -cspan 2 \
         $Dslr.format 1,0 \
         $Dslr.quality 1,1 \
         $Dslr.step 2,0 \
         $Dslr.bracket 2,1 \
         $Dslr.test 3,0 -cspan 2 \
         $Dslr.state 4,0 -cspan 2 \
         $Dslr.nom 5,0 -cspan 2 \
         $Dslr.nb_poses 6,0 -cspan 2 \
         $Dslr.pose 7,0 \
         $Dslr.exptime 7,1 \
         $Dslr.delai 8,0 -cspan 2 \
         $Dslr.intervalle 9,0 -cspan 2 \
         $Dslr.but1 10,0 -cspan 2 -ipadx 15 -ipady 3 -fill x
         ::blt::table configure $Dslr r* -pady 2

      #--   ajoute les bulles d'aide
      foreach child { step test nom nb_poses pose delai } {
         DynamicHelp::add $Dslr.$child -text $caption(acqdslr,help$child)
      }

      initPar

      #--   demarre sur la configuration 'Une image'
      configImg
   }

   ############################################################################
   #--   Initialise les variables                                             #
   ############################################################################
   proc initPar {} {
      global audace panneau caption

      lassign { "" "0" "0" "0" " " } ::acqdslr::nom ::acqdslr::delai \
         panneau(acqdslr,intervalle_mini) panneau(acqdslr,test) \
         panneau(acqdslr,action)

      #--   selectionne le mode de stockage de niveau le plus eleve
      set panneau(acqdslr,stock) [ lindex $panneau(acqdslr,stockLabels) end ]

      #--   selectionne le format actuel d'image
      set panneau(acqdslr,quality) [ lindex $panneau(acqdslr,qualityLabels) end ]

      #--   indique le format (jpeg ou fits)
      configFormat

      #--   selectionne le mode de prise de vue 'Une image"
      set panneau(acqdslr,bracket) [ lindex $panneau(acqdslr,bracketLabels) 0 ]

      #--   selectionne le nombre de pas "0"
      set panneau(acqdslr,step) [ lindex $panneau(acqdslr,stepLabels) 6 ]

      #--   selectionne le mode de declenchement de niveau le plus eleve
      set l [ llength  $panneau(acqdslr,poseLabels) ]
      incr l "-1"
      set panneau(acqdslr,pose) [ lindex $panneau(acqdslr,poseLabels) $l ]

      #--   selectionne la gamme de temps d'exposition
      configTimeScale $l

      #--   initalise le fichier log
      set rep "$audace(rep_images)"
      set nom "DSLR acqdslr "
      set date [clock format [clock seconds] -format "%A %d %B %Y"]
      append nom $date ".log"
      set panneau(acqdslr,log) [ file join $rep $nom ]
      writeLog $panneau(acqdslr,log) "$caption(acqdslr,ouvsess)"
   }

   ############################################################################
   #--   Module de construction des boutons de menu avec les commandes        #
   #  appelee par ApnBuildIF                                                  #
   ############################################################################
   proc buildMenuButton { var } {
      global panneau
      variable Dslr

      #--   raccourcis
      set data $panneau(acqdslr,${var}Labels)
      set camNo $panneau(acqdslr,camNo)

      menubutton $Dslr.$var -menu $Dslr.$var.m -relief raised \
         -width 10 -borderwidth 2 -textvar panneau(acqdslr,$var)
      menu $Dslr.$var.m -tearoff 0

      #--   specifie les commandes associees a chaque item du menu
      foreach label $data {
         set indice [ lsearch $data $label ]
         switch -exact $var {
          "bracket"     {  switch $indice {
                              "0"   {  set cmd "configImg" }
                              "1"   {  set cmd "configSerie" }
                              "2"   {  set cmd "configContinu" }
                              "3"   {  set cmd "configRafale" }
                           }
                        }
          "pose"        {  switch $indice {
                              "0"  {  set cmd "switchExpTime 0" }
                              "1"  {  set cmd "switchExpTime 1" }
                           }
                        }
          "quality"     {  if { $panneau(acqdslr,camNo) != "0" } {
                              set cmd "cam$camNo quality $label ; configFormat"
                           } else {
                              set cmd "configFormat"
                           }
                        }
          "step"        {  set cmd "test_bracketing" }
          "stock"       {  switch $indice {
                              "0"     { set cmd "switchStock 1" }
                              default { set cmd "switchStock 0" }
                           }
                        }
         }

         $Dslr.$var.m add radiobutton -label $label -indicatoron "1" \
            -value $label -variable panneau(acqdslr,$var) -command $cmd
         }
   }

   ######################################################################
   #--   Gere l'etat des widgets lies a DSLR                            #
   ######################################################################
   proc setWindowState { state } {
      global panneau
      variable This
      variable Dslr

      set children [ list stock quality step bracket test \
         nom nb_poses pose exptime delai intervalle ]

      foreach child $children {
         $Dslr.$child configure -state $state
      }
      $Dslr.but1 configure -state $state

      if { $state == "normal" } {
         set k [ lsearch $panneau(acqdslr,bracketLabels) $panneau(acqdslr,bracket) ]
         switch -exact  $k {
            "0"   { configImg }
            "1"   { configSerie }
            "2"   { configContinu }
            "3"   { configRafale }
         }
      }
   }

   #########################################################################
   #--   Configure le panneau pour une image seule                         #
   #  appelee par le bouton de mode de prise de vue et par setWindowState  #
   #########################################################################
   proc configImg { } {
      global panneau
      variable Dslr

      configStock
      configPose
      configIntervalle "1"

      foreach child { test nom } {
         $Dslr.$child configure -state normal
      }

      set ::acqdslr::nb_poses "1"
      foreach child { step nb_poses } {
         $Dslr.$child configure -state disabled
      }
      set panneau(acqdslr,test) "0"
   }

   #########################################################################
   #--   Configure le panneau pour une serie d'images                      #
   #  appelee par le bouton de mode de prise de vue et par setWindowState  #
   #########################################################################
   proc configSerie { } {
      global panneau
      variable Dslr

      configStock
      configPose
      configIntervalle "2"

      set ::acqdslr::nb_poses "2"
      foreach child { test nom nb_poses } {
         $Dslr.$child configure -state normal
      }

      set etat "normal"
      if { $panneau(acqdslr,pose) == ">31" } {
         set etat "disabled"
      }
      $Dslr.step configure -state $etat
      set panneau(acqdslr,test) "0"
   }

   #########################################################################
   #--   Configure le panneau pour une serie continue d'images             #
   #  appelee par le bouton de mode de prise de vue et par setWindowState  #
   #########################################################################
   proc configContinu { } {
      variable Dslr

      configStock
      configPose
      configIntervalle "3"

      set ::acqdslr::nb_poses "2"
      foreach child { step nb_poses } {
         $Dslr.$child configure -state normal
      }
      foreach child { test nom } {
         $Dslr.$child configure -state disabled
      }
   }

   #########################################################################
   #--   Configure le panneau pour une rafale d'images                     #
   #  appelee par le bouton de mode de prise de vue et par setWindowState  #
   #########################################################################
   proc configRafale { } {
      variable Dslr

      configStock
      configPose
      configIntervalle "4"

      foreach child { step test nom nb_poses } {
         $Dslr.$child configure -state disabled
      }
   }

   #########################################################################
   #--   Configure le bouton Stock                                         #
   #########################################################################
   proc configStock {} {
      global panneau caption
      variable Dslr

      set l [ llength $panneau(acqdslr,stockLabels) ]

      #--   si carte CF seule ou mode continu ou mode rafale
      if { $l == "1" \
         || $panneau(acqdslr,bracket) == "$caption(acqdslr,bracket,continu)" \
         || $panneau(acqdslr,bracket) == "$caption(acqdslr,bracket,rafale)" } {
         #--   active usecf 1
         $Dslr.stock.m invoke 0
         #--   gele le bouton
         $Dslr.stock configure -state disabled
      } else {
        $Dslr.stock configure -state normal
      }
   }

   #########################################################################
   #--   Configure le bouton USB/Longuepose                                #
   #########################################################################
   proc configPose {} {
      global panneau caption
      variable Dslr

      if { [ llength $panneau(acqdslr,poseLabels) ] == "2" \
         && $panneau(acqdslr,bracket) != "$caption(acqdslr,bracket,continu)" \
         && $panneau(acqdslr,bracket) != "$caption(acqdslr,bracket,rafale)" } {
            set etat "normal"
      } else {
            $Dslr.pose.m invoke 0
            set etat "disabled"
      }
      $Dslr.pose configure -state $etat
   }

   #########################################################################
   #--   Commute USB/Longuepose                                            #
   #  parametres : camNo choix ( si USB ul =0 sinon ul=1 )                 #
   #########################################################################
   proc switchExpTime { ul } {
      global panneau
      variable Dslr

      set etat_anterieur [ lindex $panneau(acqdslr,status) 0 ]

      if { $etat_anterieur != $ul } {
         set status [ cam$panneau(acqdslr,camNo) longuepose $ul ]
         #--   memorise le nouvel etat
         set panneau(acqdslr,status) \
            [ lreplace $panneau(acqdslr,status) 0 0 "$status" ]
      }

      #--   adapte l'entree du temps d'exposition
      if { $etat_anterieur != $ul || [ llength [ $Dslr.exptime cget -values ] ] == "0" } {
         configTimeScale $ul
         #--   transfert la valeur
         set ::acqdslr::exptime $panneau(acqdslr,time)
         update
      }
   }

   #########################################################################
   #--   Configure l'entree du temps d'exposition                          #
   #  parametre : camNo choix ( si USB s =0 sinon s=1 )                    #
   #########################################################################
   proc  configTimeScale { s } {
      global panneau
      variable Dslr

      switch $s {
         "0"   {  #--   met en place la liste standard
                  $Dslr.exptime configure -editable 0 \
                     -values "$panneau(acqdslr,exptimeLabels)" -height 10
                  #--   selectionne 1 seconde
                  $Dslr.exptime setvalue @15
               }
         "1"   {  #--   met en place la liste ouverte
                  $Dslr.exptime configure -editable 1 -values "31" -height 1
                  #--   selectionne 31
                  $Dslr.exptime setvalue @0
               }
      }

      #--   transfert la valeur
      set ::acqdslr::exptime $panneau(acqdslr,time)
      update
   }

   #########################################################################
   #--   Commute la memoire                                                #
   #  parametres : camNo stockage ( si CF sto =1 sinon sto=0 )             #
   #########################################################################
   proc switchStock { sto } {
      global panneau
      variable Dslr

      set camNo $panneau(acqdslr,camNo)

      if { $panneau(acqdslr,camNo) == "0" } {
         return
      }

      set etat_anterieur [ lindex $panneau(acqdslr,status) 1 ]

      if { $etat_anterieur != $sto } {
         cam$camNo autoload [ expr { 1-$sto } ]
         cam$camNo usecf $sto
         #--   memorise le nouvel etat
         set panneau(acqdslr,status) \
            [ lreplace $panneau(acqdslr,status) end end "$sto" ]
      }

      #--   si usage de CF
      if { $sto == "1" } {
         set ::acqdslr::nom ""
         set etat "disabled"
      } else {
         set etat "normal"
      }
      $Dslr.nom configure -state $etat
   }

   #########################################################################
   #--   Configure le format et l'extension                                #
   #########################################################################
   proc configFormat { } {
      global panneau conf

      if { $panneau(acqdslr,quality) == "RAW" } {
         set panneau(acqdslr,format) "fits"
         set panneau(acqdslr,extension) "$conf(extension,defaut)"
      } else {
         set panneau(acqdslr,format) "jpeg"
         set panneau(acqdslr,extension) ".jpg"
      }
   }

   ######################################################################
   #--   Configure l'entree Intervalle                                  #
   ######################################################################
   proc configIntervalle { n } {
      global panneau caption
      variable Dslr

      set var "intervalle"
      set state "disabled"
      set val " "

      if { $n == "4" } {
         set var "duree"
         set val "1"
      }
      set ::acqdslr::intervalle $val

      if { ( $n == "2" && $panneau(acqdslr,test) == "1" ) || $n == "4" } {
         set state "normal"
      }

      $Dslr.intervalle configure -label "$caption(acqdslr,$var)" -state $state
      DynamicHelp::add $Dslr.intervalle -text $caption(acqdslr,help$var)
      update
   }

   ######################################################################
   #--   Cree une entree avec un label appelee par ApnBuildIF           #
   #  parametres : nom descendant                                       #
   ######################################################################
   proc buildLabelEntry { child } {
      global caption
      variable Dslr

      LabelEntry $Dslr.$child -label $caption(acqdslr,$child) \
         -textvariable ::acqdslr::$child -labelanchor w -labelwidth 8 \
         -borderwidth 1 -relief flat -padx 2 -justify right \
         -width 8 -relief sunken
      bind $Dslr.$child <Leave> [ list test_$child ]
   }

   ######################################################################
   #--   Ecriture du fichier log                                        #
   #  parametres :   nom du fichier texte                               #
   ######################################################################
   proc writeLog { f m } {

      set err [ catch {
            set fd [ open $f a+ ]
            puts $fd $m
            close $fd
         } msg ]
      return $err
   }

   ######################################################################
   #--   Fenetre d'avertissement                                        #
   #  parametres :   variable de caption                                #
   ######################################################################
   proc avertiUser { v } {
      global caption

      tk_messageBox -title $caption(acqdslr,attention)\
         -icon error -type ok -message $caption(acqdslr,$v)
   }

   ######################################################################
   #-- Gere les prises de vue a partir des reglages de l'utilisateur    #
   ######################################################################
   proc shoot {} {
      global panneau caption

      #--   si pas de cam connectee
      if { $panneau(acqdslr,camNo) == "0" } {
         connectCam
         return
      }

      #--   gele les commandes
      setWindowState disabled

      #--   affiche le status 'Attente'
      set panneau(acqdslr,action) $caption(acqdslr,action,wait)

      #---  si la pose est differee, affichage du temps restant
      if { $::acqdslr::delai != 0 } {
         delay "delai"
      }

      if { $panneau(acqdslr,bracket) == "$caption(acqdslr,bracket,rafale)" } {
         #--   affiche le status 'Acquisition'
         set panneau(acqdslr,action) $caption(acqdslr,action,acq)
         shootRafale
         #--   complete le fichier log
         infLog "$caption(acqdslr,duree) $::acqdslr::intervalle sec."
      } else {
         shootImg
      }

      set panneau(acqdslr,action) " "

      #--   degele les commandes
      setWindowState normal
   }

   ######################################################################
   #-- Commande les prises de vue autres que le mode Rafale de l'APN    #
   ######################################################################
   proc shootImg { } {
      global panneau caption
      variable Dslr

      #--   raccourcis
      set camNo $panneau(acqdslr,camNo)
      set type [ lsearch $panneau(acqdslr,bracketLabels) $panneau(acqdslr,bracket) ]
      set stock "$panneau(acqdslr,stock)"
      set exptime $::acqdslr::exptime

      #--   memorise
      set nb_poses $acqdslr::nb_poses
      set timer $::acqdslr::intervalle
      set n [ lsearch $panneau(acqdslr,exptimeValues) $exptime ]
      set delta [ expr { -1*$panneau(acqdslr,step) } ]

      #--   compte les images
      set i 1

      while { $::acqdslr::nb_poses > 0 } {

         #--  affiche le status 'Acquisition'
         set panneau(acqdslr,action) $caption(acqdslr,action,acq)

         #--  regle le temps d'exposition
         cam$camNo exptime $exptime

         #--- le temps maintenant
         set time_now [ clock seconds ]

         #--- Alarme sonore de fin de pose
         ::camera::alarmeSonore $exptime

         #--  definit la commande
         if { $type == "2" } {

            #--   En Continu : retard pour valider le changement de temps d'exposition
            if { $delta != "0" } {
               after 1500
            }
            catch { cam$camNo acq -blocking } msg

         } else {

            #--   Une image ou Une serie
            catch {  cam$camNo acq
                     vwait status_cam$camNo } msg
         }

         if ![ regexp "Dialog error" $msg ] {
            if { $acqdslr::nom != "Test" } {
               set name $acqdslr::nom
               append name $i $panneau(acqdslr,extension)
               infLog "$name"
            }
        } else {
            avertiUser "cam_pb"
        }

         #--  charge et visualise l'image si stockage autre que carte CF
         if { $stock != "$caption(acqdslr,stock,cf)" } {
            loadandseeImg $i
         }

         #--- decremente et affiche le nombre de poses qui reste a prendre
         incr ::acqdslr::nb_poses "-1"

         #-- incremente l'index de l'image
         incr i

         #--   si ce n'est pas la derniere image
         if { $nb_poses >= $i && $type != 0 } {

            #--   recalcule et affiche exptime pour serie et rafale
            if { $delta != "0" } {

               #--   incremente l'index = regresse dans la liste
               incr n "$delta"

               #--   extrait le temps de pose
               set ::acqdslr::exptime [ lindex  $panneau(acqdslr,exptimeValues) $n ]

               #--   actualise le temps de pose sur le panneau
               $Dslr.exptime setvalue @$n
               update
            }

            #--   met a jour l'intervalle pour Une serie
            if { $type == 1 } {
               set d [ expr { $time_now + $timer -[ clock seconds ] } ]
               if { $d > 1 } {

                  #--   met a jour le timer
                  set ::acqdslr::intervalle $d

                  #--   decompte les secondes
                  delay "intervalle"
               }
            }
         }
      }
   }

   ######################################################################
   #-- Commande le mode Rafale de l'APN                                 #
   ######################################################################
   proc shootRafale {} {
      global panneau

      #--   raccourcis
      set camNo $panneau(acqdslr,camNo)
      set linkNo $panneau(acqdslr,linkNo)
      set bitNo $panneau(acqdslr,bitNo)

      #--  regle le temps d'exposition
      cam$camNo exptime $::acqdslr::exptime

      #--   passe en mode 'rafale'
      cam$camNo drivemode 1

      #--   prend une photo juste pour passer les parametres
      catch {  cam$camNo acq -blocking } msg

      if [ regexp "Dialog error" $msg ] {
            avertiUser "cam_pb"
      }

      #--   passe en mode longuepose
      cam$camNo longuepose 1

      #--   actionne le bit
      link$linkNo bit $bitNo $panneau(acqdslr,startvalue)
      after [ expr { $acqdslr::intervalle*1000 } ]
      link$linkNo bit $bitNo $panneau(acqdslr,stopvalue)

      #--   repasse en mode USB
      cam$camNo longuepose 0

      #--   repasse en mode normal
      cam$camNo drivemode 0
   }

   ######################################################################
   #-- Nomme, sauvegarde, transfert et affiche l'image                  #
   #-- parametre : index de l'image                                     #
   ######################################################################
   proc loadandseeImg { k } {
      global audace panneau caption

      #--   affiche le status 'Sauvegarde'
      set panneau(acqdslr,action) $caption(acqdslr,action,load)

      #--   visualise l'image
      confVisu::autovisu $audace(visuNo)

      #--   nomme l'image
      set name $acqdslr::nom
      if { $name == "" } { set name "tmp" }

      #---  donne un index et une extension a l'image
      append name $k $panneau(acqdslr,extension)

      #--   recupere les stats
      set stat [ stat ]

      #--   sauve l'image
      saveima [ file join $audace(rep_images) $name ]

      #--   envoie les valeurs vers la console pour une image RAW
      if { $panneau(acqdslr,format) == "fits" } {
         ::console::affiche_resultat "\n$name\n\
            $caption(acqdslr,maxi) [ lindex $stat 2 ]\n\
            $caption(acqdslr,moyenne) [ lindex $stat 4 ]\n\
            $caption(acqdslr,mini) [ lindex $stat 3 ]\n\n"
      }
   }

   ######################################################################
   #--  Mesure l'intervalle mini dans les conditions de reglages        #
   ######################################################################
   proc testTime { } {
      global audace panneau caption

      if { $panneau(acqdslr,camNo) == "0" } {
         return
      }

      #--   annule le delai
      set ::acqdslr::delai "0"

      #--   affiche le nom
      set ::acqdslr::nom "$caption(acqdslr,test)"

      #--   inhibe les commandes
      setWindowState disabled

      #--   fixe le nb de pose a 1
      set ::acqdslr::nb_poses "1"

      #--   le temps maintenant
      set t0 [clock milliseconds]

      #--   lance une acquisition
      shoot

      #--   calcule la duree de la sequence
      set duree [ expr { int(([clock milliseconds ]-$t0)/1000.0) } ]

      #--   memorise l'intervalle
      set panneau(remotecrtl,intervalle_mini) "$duree"

      #--   definit le nom du fichier
      set file ${::acqdslr::nom}1$panneau(acqdslr,extension)

      #--   supprime le nom affiche
      set ::acqdslr::nom " "

      #--   supprime le fichier test
      file delete [ file join $audace(rep_images) $file ]

      #--   memorise le test
      set panneau(acqdslr,test) "1"

      #--   libere les commandes
      setWindowState normal

      #--   fixe l'intervalle mini a afficher
      set ::acqdslr::intervalle "$duree"
   }

   ######################################################################
   #-- Verifie et Formate le temps en d�cimal                           #
   ######################################################################
   proc test_exptime {} {
      global panneau

      set exptime $panneau(acqdslr,time)

      if { $panneau(acqdslr,pose) == "<30" } {

         #--   en mode USB, cherche l'index de la valeur
         set i [ lsearch $panneau(acqdslr,exptimeLabels) $exptime ]

         #--   lit le resultat dans la liste de conversion
         set resultat [ lindex $panneau(acqdslr,exptimeValues) $i ]

      } else {

         #--   en mode Longuepose, teste l'entree
         #--   ote tout ce qui suit la virgule
         regsub -all {[^0-9]} $exptime {} resultat

         if { $exptime != $resultat || $exptime <= "30" } {
            #--   modifie l'affichage
            set $panneau(acqdslr,time) "31"
            set resultat "31"
         }
      }
      set ::acqdslr::exptime $resultat
   }

   ###################################################################
   #-- Ote tous les caracteres non alphanumeriques ou non underscore #
   ###################################################################
   proc test_nom {} {

      #-- seuls les caracteres alphanum�riques et le underscore sont autorises
      regsub -all {[^\w_]} $::acqdslr::nom {} ::acqdslr::nom

   }

   ###################################################################
   #-- Si le nombre de poses n'est pas un entier il est fixe a 1     #
   ###################################################################
   proc test_nb_poses {} {

      if ![ TestEntier $::acqdslr::nb_poses ] {
         set ::acqdslr::nb_poses "1"
      }
   }

   #######################################################################
   #-- Si l'intervalle n'est pas un entier ou est < l'intervalle minimum #
   #-- il est fixe � l'intervalle minimum                                #
   #######################################################################
   proc test_intervalle {} {
      global panneau

      regsub -all {[^0-9]} $::acqdslr::intervalle {} resultat
      if { $resultat < $panneau(acqdslr,intervalle_mini) } {
         set resultat $panneau(acqdslr,intervalle_mini)
      }
      set ::acqdslr::intervalle $resultat
   }

   ###################################################################
   #-- Si le delai n'est pas un entier il est fixe a 0               #
   ###################################################################
   proc test_delai {} {

      if ![ TestEntier $::acqdslr::delai ] {
         set ::acqdslr::delai "0"
      }
   }

   #########################################################################
   #-- Teste si la valeur d'exposition finale est dans la plage            #
   #-- appelee par le bouton Pas, le nb de poses et le temps d'exposition  #
   #-- s'applique uniquement a 'Une serie' ou 'En continu'                 #
   #########################################################################
   proc test_bracketing {} {
      global panneau caption

      if { $panneau(acqdslr,pose) == "<30" \
         &&  ( $panneau(acqdslr,bracket) == $caption(acqdslr,bracket,serie) \
         || $panneau(acqdslr,bracket) == $caption(acqdslr,bracket,continu))  } {

         #--   recherche l'indice du temps affiche
         set i_initial [ lsearch $panneau(acqdslr,exptimeLabels) $panneau(acqdslr,time) ]

         #--   calcule l'indice de la valeur finale
         set i_final [ expr { $i_initial+$panneau(acqdslr,step)*(1-$::acqdslr::nb_poses) } ]

         if { $i_final < 0 || $i_final > [ llength $panneau(acqdslr,exptimeLabels) ] } {

            #--   message d'alerte si hors plage
            avertiUser "out_of_limits"

            #--   remet pas a 0
            set panneau(acqdslr,step) 0
         }
      }
   }

   ######################################################################
   #-- Complete le fichier log                                          #
   #-- parametre : nom de l'image                                       #
   ######################################################################
   proc infLog { name } {
      global panneau

      set texte [ list \
            "$panneau(acqdslr,stock)" \
            "$panneau(acqdslr,bracket)" \
            "$panneau(acqdslr,quality)" \
            $::acqdslr::exptime \
            "$name" ]
      writeLog $panneau(acqdslr,log) $texte
   }

   ######################################################################
   #--   Decompteur de secondes                                         #
   #  parametre : nom de la variable a decompter (delai ou intervalle)  #
   ######################################################################
   proc delay { var } {

      upvar 1 ::acqdslr::$var t
      while { $t > "0" } {
            after 1000
            incr t "-1"
            update
      }
      set t "0"
      update
   }

   ######################################################################
   #--   propose de connecter la cam et de reconfigurer le panneau      #
   ######################################################################
   proc connectCam  { } {
      global audace panneau caption
      variable Dslr

      ::confCam::run
      ::confCam::selectNotebook A dslr
      vwait audace(camNo)

      #--   rapatrie les donnes sur la connexion
      setCamProperty

      if { $panneau(acqdslr,camNo) != "0" } {
      #--   reconfigure le panneau
         destroy $Dslr
         apnBuildIF

      }
   }

   ######################################################################
   #--   Recupere les donnees de la cam                                 #
   ######################################################################
   proc setCamProperty { } {
      global panneau caption

      set panneau(acqdslr,camNo) "0"
      set cam_list [ ::cam::list ]

      #--   arrete si pas de cam
      if { $cam_list == "" } {
         return
      }

      #--   cherche une cam DSLR
      foreach camNo $cam_list {
         if { [ cam$camNo name ] == "DSLR" } {
            set panneau(acqdslr,camNo) $camNo
         }
      }

      #--   arrete si pas de DSLR
      if { $panneau(acqdslr,camNo) == "0" } {
         return
      }

      set camNo $panneau(acqdslr,camNo)

      #--   fixe drivemode a 0
      cam$camNo drivemode 0

      #--   liste les formats possibles
      set panneau(acqdslr,qualityLabels) [ cam$camNo quality list ]
      set panneau(acqdslr,quality) [ cam$camNo quality ]

      #--   si la camera est connectee en mode longue pose
      if [ cam$camNo longuepose ] {

         #--   complete la liste des types de declenchement
         lappend panneau(acqdslr,poseLabels) ">30"

         set panneau(acqdslr,linkNo) [ cam$camNo longueposelinkno ]
         set panneau(acqdslr,bitNo) [ cam$camNo longueposelinkbit ]
         set panneau(acqdslr,startvalue) [ cam$camNo longueposestartvalue ]
         set panneau(acqdslr,stopvalue) [ cam$camNo longueposestopvalue ]

         #--   ajoute le mode Rafale si la longuepose existe
         lappend panneau(acqdslr,bracketLabels) "$caption(acqdslr,bracket,rafale)"

         #--   memorise l'etat
         set panneau(acqdslr,status) [ lreplace $panneau(acqdslr,status) 0 0 "1" ]
         lappend panneau(acqdslr,status) "0"
      }
   }