#
# Fichier : acqfcSetup.tcl
# Description : Configuration de certains parametres de l'outil Acquisition
# Auteur : Robert DELMAS
# Mise a jour $Id: acqfcSetup.tcl,v 1.7 2007-11-29 22:10:26 robertdelmas Exp $
#

namespace eval acqfcSetup {

   #
   # acqfcSetup::init
   # Chargement des captions
   #
   proc init { } {
      global audace

      #--- Chargement des captions
      source [ file join $audace(rep_plugin) tool acqfc acqfcSetup.cap ]
   }

   #
   # acqfcSetup::initToConf
   # Initialisation des variables de configuration
   #
   proc initToConf { visuNo } {
      variable parametres

      #--- Creation des variables de la boite de configuration si elles n'existent pas
      if { ! [ info exists ::acqfc::parametres(acqfc,$visuNo,messages) ] }      { set ::acqfc::parametres(acqfc,$visuNo,messages)      "1" }
      if { ! [ info exists ::acqfc::parametres(acqfc,$visuNo,save_file_log) ] } { set ::acqfc::parametres(acqfc,$visuNo,save_file_log) "1" }
   }

   #
   # acqfcSetup::confToWidget
   # Charge la configuration dans des variables locales
   #
   proc confToWidget { visuNo } {
      variable parametres
      global panneau

      #--- confToWidget
      set panneau(acqfc,$visuNo,messages)      $::acqfc::parametres(acqfc,$visuNo,messages)
      set panneau(acqfc,$visuNo,save_file_log) $::acqfc::parametres(acqfc,$visuNo,save_file_log)
   }

   #
   # acqfcSetup::widgetToConf
   # Acquisition de la configuration, c'est a dire isolation des differentes variables dans le tableau conf(...)
   #
   proc widgetToConf { visuNo } {
      variable parametres
      global panneau

      #--- widgetToConf
      set ::acqfc::parametres(acqfc,$visuNo,messages)      $panneau(acqfc,$visuNo,messages)
      set ::acqfc::parametres(acqfc,$visuNo,save_file_log) $panneau(acqfc,$visuNo,save_file_log)
   }

   #
   # acqfcSetup::run
   # Cree la fenetre de configuration de l'affichage des messages sur la Console
   # et de l'enregistrement des dates dans le fichier log
   #
   proc run { visuNo this } {
      global audace panneau

      #--- Determination de la fenetre parente
      if { $visuNo == "1" } {
         set base "$audace(base)"
      } else {
         set base ".visu$visuNo"
      }

      #---
      set panneau(acqfc,$visuNo,acqfcSetup) $this
      ::confGenerique::run $visuNo "$panneau(acqfc,$visuNo,acqfcSetup)" "::acqfcSetup" -modal 0
      set posx_config [ lindex [ split [ wm geometry $base ] "+" ] 1 ]
      set posy_config [ lindex [ split [ wm geometry $base ] "+" ] 2 ]
      wm geometry $panneau(acqfc,$visuNo,acqfcSetup) +[ expr $posx_config + 165 ]+[ expr $posy_config + 55 ]
   }

   #
   # acqfcSetup::ok
   # Fonction appellee lors de l'appui sur le bouton 'OK' pour appliquer la configuration
   # et fermer la fenetre du choix de l'affichage ou non de messages sur la Console et de
   # l'enregistrement ou non des dates dans le fichier log
   #
   proc ok { visuNo } {
      ::acqfcSetup::apply $visuNo
      ::acqfcSetup::closeWindow
   }

   #
   # acqfcSetup::apply
   # Fonction 'Appliquer' pour memoriser et appliquer la configuration
   #
   proc apply { visuNo } {
      ::acqfcSetup::widgetToConf $visuNo
   }

   #
   # acqfcSetup::showHelp
   # Fonction appellee lors de l'appui sur le bouton 'Aide'
   #
   proc showHelp { } {
      ::audace::showHelpPlugin [ ::audace::getPluginTypeDirectory [ ::acqfc::getPluginType ] ] \
         [ ::acqfc::getPluginDirectory ] acqfcSetup.htm
   }

   #
   # acqfcSetup::closeWindow
   # Fonction appellee lors de l'appui sur le bouton 'Fermer'
   #
   proc closeWindow { visuNo } {
   }

   #
   # acqfcSetup::getLabel
   # Retourne le nom de la fenetre de configuration
   #
   proc getLabel { } {
      global caption

      return "$caption(acqfcSetup,titre)"
   }

   #
   # acqfcSetup::fillConfigPage
   # Creation de l'interface graphique
   #
   proc fillConfigPage { frm visuNo } {
      global caption panneau

      #--- Charge la configuration de la vitesse de communication dans une variable locale
      ::acqfcSetup::confToWidget $visuNo

      #--- Frame pour les commentaires
      frame $panneau(acqfc,$visuNo,acqfcSetup).frame3 -borderwidth 1 -relief raise

         #--- Frame pour le commentaire 1
         frame $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame4 -borderwidth 0

            #--- Cree le label pour le commentaire 1
            frame $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame4.frame6
               label $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame4.frame6.lab1 -text "$caption(acqfcSetup,texte1)"
               pack $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame4.frame6.lab1 -side left -fill both \
                  -expand 0 -padx 5 -pady 5
            pack $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame4.frame6 -side left -fill both -expand 1

            #--- Cree le checkbutton pour le commentaire 1
            frame $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame4.frame7 -borderwidth 0
               checkbutton $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame4.frame7.check1 -highlightthickness 0 \
                  -variable panneau(acqfc,$visuNo,messages)
               pack $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame4.frame7.check1 -side right -padx 5 -pady 0
            pack $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame4.frame7 -side right -fill both -expand 1

         pack $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame4 -side top -fill both -expand 1

         #--- Frame pour le commentaire 2
         frame $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame5 -borderwidth 0

            #--- Cree le label pour le commentaire 2
            frame $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame5.frame8 -borderwidth 0
               label $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame5.frame8.lab2 -text "$caption(acqfcSetup,texte2)"
               pack $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame5.frame8.lab2 -side left -fill both \
                  -expand 0 -padx 5 -pady 5
            pack $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame5.frame8 -side left -fill both -expand 1

            #--- Cree le checkbutton pour le commentaire 2
            frame $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame5.frame9 -borderwidth 0
               checkbutton $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame5.frame9.check2 -highlightthickness 0 \
                  -variable panneau(acqfc,$visuNo,save_file_log)
               pack $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame5.frame9.check2 -side right -padx 5 -pady 0
            pack $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame5.frame9 -side right -fill both -expand 1

         pack $panneau(acqfc,$visuNo,acqfcSetup).frame3.frame5 -side top -fill both -expand 1

      pack $panneau(acqfc,$visuNo,acqfcSetup).frame3 -side top -fill both -expand 1

   }

}

#--- Initialisation au demarrage
::acqfcSetup::init

