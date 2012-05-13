#
# Fichier : magnifier.tcl
# Description : Affiche une loupe sur l'image
# Auteur : Raymond Zachantke
# Mise à jour $Id$
#

#============================================================
# Declaration du namespace Magnifier
#    initialise le namespace
#============================================================
namespace eval ::Magnifier {
   package provide Magnifier 1.0

   array set widget { }

   #--- Chargement des captions pour recuperer le titre utilise par getPluginLabel
   source [ file join [file dirname [info script]] magnifier.cap ]

   #------------------------------------------------------------
   # getPluginTitle
   #    retourne le titre du plugin dans la langue de l'utilisateur
   #------------------------------------------------------------
   proc getPluginTitle { } {
      global caption

      return "$caption(magnifier,titre)"
   }

   #------------------------------------------------------------
   # getPluginHelp
   #    retourne le nom du fichier d'aide principal
   #------------------------------------------------------------
   proc getPluginHelp { } {
      return "magnifier.htm"
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
      return "magnifier"
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
         function     { return "display" }
         subfunction1 { return "magnifier" }
         display      { return "window" }
         multivisu    { return 1 }
      }
   }

   #------------------------------------------------------------
   # initPlugin
   #    initialise le plugin au demarrage d'AudeLA
   #    Il ne faut utiliser cette procedure que si on a besoin d'initialiser des
   #    des variables ou de creer des procedure des le demarrage d'AudeLA.
   #    Sinon il vaut mieux utiliser createPluginInstance qui est appelee lors de
   #    la premiere utilisation de l'outil.
   #    Cela evite ainsi d'alourdir le demarrage d'AudeLA et d'occuper de la
   #    memoire pour rien si l'outil n'est pas utilise.
   #------------------------------------------------------------
   proc initPlugin { tkbase } {
   }

   #------------------------------------------------------------
   # createPluginInstance
   #    cree une nouvelle instance de l'outil
   #------------------------------------------------------------
   proc createPluginInstance { { in "" } { visuNo 1 } } {
      #variable private

      #--- j'initialise les parametres dans le tableau conf()
      #set private(imageSize) " "

      return [namespace current]
   }

   #------------------------------------------------------------
   # deletePluginInstance
   #    suppprime l'instance du plugin
   #------------------------------------------------------------
   proc deletePluginInstance { visuNo } {
      if { [ winfo exists [ confVisu::getBase $visuNo ].confMagnifier ] } {
         #--- je ferme la fenetre si l'utilisateur ne l'a pas deja fait
         ::Magnifier::closeWindow $visuNo
      }
   }

   #------------------------------------------------------------
   # startTool
   #    affiche la fenetre de l'outil
   #------------------------------------------------------------
   proc startTool { visuNo } {
      #--- J'ouvre la fenetre
      ::confGenerique::run $visuNo [confVisu::getBase $visuNo].confMagnifier ::Magnifier -modal 0
   }

   #------------------------------------------------------------
   # stopTool
   #    masque la fenetre de l'outil
   #------------------------------------------------------------
   proc stopTool { visuNo } {
      #--- Rien a faire, car la fenetre est fermee par l'utilisateur
   }

   #------------------------------------------------------------
   # getLabel
   #    retourne le nom et le label du reticule
   #
   #  return "Titre de l'onglet (dans la langue de l'utilisateur)"]
   #------------------------------------------------------------
   proc getLabel { } {
      return [ ::Magnifier::getPluginTitle ]
   }

   #------------------------------------------------------------
   # apply { }
   #    copie les variable des widgets dans le tableau conf()
   #------------------------------------------------------------
   proc apply { visuNo } {
      variable widget
      global conf

      set conf(visu,magnifier,color) $widget(color)
      set conf(visu,magnifier,defaultstate) $widget(defaultstate)

      ::confVisu::setMagnifier $visuNo $widget($visuNo,currentstate)
   }

   #------------------------------------------------------------
   # closeWindow
   #    Procedure correspondant a l'appui sur le bouton Fermer
   #------------------------------------------------------------
   proc closeWindow { visuNo } {
      variable wbase

      #--- Detruit la fenetre
      destroy [confVisu::getBase $visuNo].confMagnifier
   }

   #------------------------------------------------------------
   # fillConfigPage { }
   #    fenetre de configuration du reticule
   #
   #  return rien
   #
   #------------------------------------------------------------
   proc fillConfigPage { frm visuNo } {
      variable widget
      global caption conf

      #--- je memorise la reference de la frame
      set widget(frm) $frm

      #--- j'initialise les valeurs
      set widget(color)                $conf(visu,magnifier,color)
      set widget(defaultstate)         $conf(visu,magnifier,defaultstate)
      set widget($visuNo,currentstate) [::confVisu::getMagnifier $visuNo]

      #--- creation des differents frames
      frame $frm.frameState -borderwidth 1 -relief raised
      pack $frm.frameState -side top -fill both -expand 1

      frame $frm.frameColor -borderwidth 1 -relief raised
      pack $frm.frameColor -side top -fill both -expand 1

      #--- current state
      checkbutton $frm.frameState.currentstate -text "$caption(magnifier,current_state_label)" \
         -highlightthickness 0 -variable ::Magnifier::widget($visuNo,currentstate)
      pack $frm.frameState.currentstate -in $frm.frameState -anchor center -side left -padx 10 -pady 5

      #--- default state
      checkbutton $frm.frameState.defaultstate -text "$caption(magnifier,default_state_label)" \
         -highlightthickness 0 -variable Magnifier::widget(defaultstate)
      pack $frm.frameState.defaultstate -in $frm.frameState -anchor center -side left -padx 10 -pady 5

      #--- color
      label $frm.frameColor.labColor -text "$caption(magnifier,color_crosshair)" -relief flat
      pack $frm.frameColor.labColor -in $frm.frameColor -anchor center -side left -padx 10 -pady 10

      button $frm.frameColor.butColor_color_invariant -relief raised -width 6 -bg $widget(color) \
         -activebackground $widget(color) -command "::Magnifier::changeColor $frm"
      pack $frm.frameColor.butColor_color_invariant -in $frm.frameColor -anchor center -side left -padx 10 -pady 5 -ipady 5
   }

   #==============================================================
   # Fonctions specifiques
   #==============================================================

   #------------------------------------------------------------
   # showHelp
   #    aide
   #
   #------------------------------------------------------------
   proc showHelp { } {
      ::audace::showHelpPlugin [ ::audace::getPluginTypeDirectory [ ::Magnifier::getPluginType ] ] \
         [ ::Magnifier::getPluginDirectory ] [ ::Magnifier::getPluginHelp ]
   }

   #------------------------------------------------------------
   # changeColor
   #    change la couleur de la loupe
   #
   #------------------------------------------------------------
   proc changeColor { frm } {
      variable widget
      global caption

      set temp [tk_chooseColor -initialcolor $::Magnifier::widget(color) -parent ${Magnifier::widget(frm)} \
         -title ${caption(magnifier,color_crosshair)} ]
      if  { "$temp" != "" } {
         set Magnifier::widget(color) "$temp"
         ${Magnifier::widget(frm)}.frameColor.butColor_color_invariant configure \
            -bg $::Magnifier::widget(color) -activebackground $::Magnifier::widget(color)
      }
   }

}
