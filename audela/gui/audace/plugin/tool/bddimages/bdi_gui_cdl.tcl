## \file bdi_gui_cdl.tcl
#  \brief     Creation des courbes de lumiere 
#  \details   Ce namepsace se restreint a tout ce qui est gestion des sources
#             pour la selection des refereence en vue de faire la courbe de 
#             Lumiere.
#  \author    Frederic Vachier
#  \version   1.0
#  \date      2013
#  \copyright GNU Public License.
#  \par Ressource 
#  \code  source [file join $audace(rep_install) gui audace plugin tool bddimages bdi_gui_cdl.tcl]
#  \endcode
#  \todo      normaliser les noms des fichiers sources 

#--------------------------------------------------
#
# source [ file join $audace(rep_plugin) tool bddimages bdi_gui_cdl.tcl ]
#
#--------------------------------------------------
#
# Mise à jour $Id: bdi_tools_cdl.tcl 6858 2011-03-06 14:19:15Z fredvachier $
#
#--------------------------------------------------

## Declaration du namespace \c bdi_gui_cdl.
#  @pre       Chargement a partir d'Audace
#  @bug       Probleme de memoire sur les exec
#  @warning   Appel GUI
namespace eval bdi_gui_cdl {

   
   variable fen      ; # Variable definissant la racine de la fenetre de l'outil
   variable dataline ; # table des donnees des etoiles de reference
   
}


   #----------------------------------------------------------------------------
   ## Initialisation des variables de namespace
   #  \details   Si la variable n'existe pas alors on va chercher
   #             dans la variable globale \c conf
   proc ::bdi_gui_cdl::inittoconf {  } {
      
      
   }

   #----------------------------------------------------------------------------
   ## Fermeture de la fenetre .
   # Les variables utilisees sont affectees a la variable globale
   # \c conf
   proc ::bdi_gui_cdl::fermer {  } {

      destroy $::bdi_gui_cdl::fen
      
      unset ::bdi_gui_cdl::fen
      
   }

   #----------------------------------------------------------------------------
   ## Relance l outil
   # Les variables utilisees sont affectees a la variable globale
   # \c conf
   proc ::bdi_gui_cdl::relance {  } {

      ::bddimages::ressource
      ::bdi_gui_cdl::fermer
      ::bdi_gui_cdl::run
      foreach img $::tools_cata::img_list {
         gren_info "IMG = [llength $img]\n"
         break
      }
      ::bdi_tools_cdl::get_memory

   }


   # Structure ASTROID :
   #  0    "xsm" 
   #  1    "ysm" 
   #  2    "err_xsm" 
   #  3    "err_ysm" 
   #  4    "fwhmx" 
   #  5    "fwhmy" 
   #  6    "fwhm" 
   #  7    "flux" 
   #  8    "err_flux" 
   #  9    "pixmax"
   #  10   "intensity" 
   #  11   "sky" 
   #  12   "err_sky" 
   #  13   "snint" 
   #  14   "radius" 
   #  15   "rdiff" 
   #  16   "err_psf" 
   #  17   "ra" 
   #  18   "dec"
   #  19   "res_ra" 
   #  20   "res_dec" 
   #  21   "omc_ra" 
   #  22   "omc_dec" 
   #  23   "mag" 
   #  24   "err_mag" 
   #  25   "name" 
   #  26   "flagastrom" 
   #  27   "flagphotom" 
   #  28   "cataastrom"
   #  29   "cataphotom"
   proc ::bdi_gui_cdl::affiche_data { } {

      set tt0 [clock clicks -milliseconds]

      # Onglet References
      $::bdi_gui_cdl::dataline delete 0 end
      for {set id 1} {$id<=$::tools_cata::nb_img_list} {incr id} {
         set sources [lindex  $::gui_cata::cata_list($id) 1]
         foreach s $sources {
            set othf [::bdi_tools_psf::get_astroid_othf_from_source $s]
            set name [::bdi_tools_psf::get_val othf "name"]
            if {$name == ""} {continue}
            set nbimg 1
            set mag [::bdi_tools_psf::get_val othf "mag"]
            $::bdi_gui_cdl::dataline insert end [list "0" $name $nbimg $mag]
         }
      }

      set tt [format "%.3f" [expr ([clock clicks -milliseconds] - $tt0)/1000.]]
      gren_info "Affichage complet en $tt sec \n"

   }


   proc ::bdi_gui_cdl::cmdButton1Click_dataline { w args } {

   }

   #----------------------------------------------------------------------------
   ## Demarrage de l'outil
   #  \param img_list structure de liste d'images
   proc ::bdi_gui_cdl::run { } {

      ::bdi_gui_cdl::inittoconf
      
      ::bdi_gui_cdl::create_dialog
   }

   #----------------------------------------------------------------------------
   ## Creation de la boite de dialogue.
   proc ::bdi_gui_cdl::create_dialog { } {

      set ::bdi_gui_cdl::fen .photometry
      if { [winfo exists $::bdi_gui_cdl::fen] } {
         wm withdraw $::bdi_gui_cdl::fen
         wm deiconify $::bdi_gui_cdl::fen
         focus $::bdi_gui_cdl::fen
         return
      }
      toplevel $::bdi_gui_cdl::fen -class Toplevel
      set posx_config [ lindex [ split [ wm geometry $::bdi_gui_cdl::fen ] "+" ] 1 ]
      set posy_config [ lindex [ split [ wm geometry $::bdi_gui_cdl::fen ] "+" ] 2 ]
      wm geometry $::bdi_gui_cdl::fen +[ expr $posx_config + 165 ]+[ expr $posy_config + 55 ]
      wm resizable $::bdi_gui_cdl::fen 1 1
      wm title $::bdi_gui_cdl::fen "Photometrie V3"
      wm protocol $::bdi_gui_cdl::fen WM_DELETE_WINDOW "::bdi_gui_cdl::fermer"

      set frm $::bdi_gui_cdl::fen.appli
      #--- Cree un frame general
      frame $frm  -cursor arrow -relief groove
      pack $frm -in $::bdi_gui_cdl::fen -anchor s -side top -expand yes -fill both -padx 10 -pady 5

         set onglets [frame $frm.onglets]
         pack $onglets -in $frm  -expand yes -fill both

            pack [ttk::notebook $onglets.nb] -expand yes -fill both 
            set f_dataline [frame $onglets.nb.f_dataline]
            set f_starstar [frame $onglets.nb.f_starstar]
            set f_classif  [frame $onglets.nb.f_classif]
            set f_timeline [frame $onglets.nb.f_timeline]

            $onglets.nb add $f_dataline -text "References"
            $onglets.nb add $f_starstar -text "Variations"
            $onglets.nb add $f_classif  -text "Classification"
            $onglets.nb add $f_timeline -text "Timeline"

            $onglets.nb select $f_dataline
            ttk::notebook::enableTraversal $onglets.nb

         set results [frame $f_dataline.dataline  -borderwidth 10 -relief groove]
         pack $results -in $f_dataline -expand yes -fill both
            
            set cols [list 0 " "       left  \
                           0 "Name"    left  \
                           0 "Nb img"  right \
                           0 "moy Mag" right \
                     ]
            # Table
            set ::bdi_gui_cdl::dataline $results.table
            tablelist::tablelist $::bdi_gui_cdl::dataline \
              -columns $cols \
              -labelcommand tablelist::sortByColumn \
              -xscrollcommand [ list $results.hsb set ] \
              -yscrollcommand [ list $results.vsb set ] \
              -selectmode extended \
              -activestyle none \
              -stripebackground "#e0e8f0" \
              -showseparators 1
    
            # Scrollbar
            scrollbar $results.hsb -orient horizontal -command [list $::bdi_gui_cdl::dataline xview]
            pack $results.hsb -in $results -side bottom -fill x
            scrollbar $results.vsb -orient vertical -command [list $::bdi_gui_cdl::dataline yview]
            pack $results.vsb -in $results -side left -fill y 

            # Pack la Table
            pack $::bdi_gui_cdl::dataline -in $results -expand yes -fill both

            # Popup
            menu $results.popupTbl -title "Actions"

               $results.popupTbl add command -label "Voir l'objet dans une image" \
                   -command "" -state disabled

            # Binding
            bind $::bdi_gui_cdl::dataline <<ListboxSelect>> [ list ::bdi_gui_cdl::cmdButton1Click_dataline %W ]
            bind [$::bdi_gui_cdl::dataline bodypath] <ButtonPress-3> [ list tk_popup $results.popupTbl %X %Y ]

            # tri des colonnes (ascii|asciinocase|command|dictionary|integer|real)
            #    Ascii
            foreach ncol [list "Name"] {
               set pcol [expr int ([lsearch $cols $ncol]/3)]
               $::bdi_gui_cdl::dataline columnconfigure $pcol -sortmode ascii
            }
            #    Reel
            foreach ncol [list "Nb img" "moy Mag"] {
               set pcol [expr int ([lsearch $cols $ncol]/3)]
               $::bdi_gui_cdl::dataline columnconfigure $pcol -sortmode real
            }

         #--- Cree un frame 
         set pb [frame $frm.pb  -borderwidth 0 -cursor arrow -relief groove]
         pack $pb  -in $frm -anchor s -side top -expand 0 -fill x -padx 10 -pady 5

             set ::bdi_tools_cdl::progress 0
             set    pf [ ttk::progressbar $pb.p -variable ::bdi_tools_cdl::progress -orient horizontal -mode determinate]
             pack   $pf -in $pb -side left -expand 1 -fill x 

         #--- Cree un frame pour afficher les boutons
         set center [frame $frm.info  -borderwidth 2 -cursor arrow -relief groove]
         pack $center  -in $frm -anchor s -side bottom -expand 0 -fill x -padx 10 -pady 5

         #--- Cree un frame pour afficher les boutons
         set info [frame $center.info  -borderwidth 0 -cursor arrow -relief groove]
         pack $info  -in $center -anchor s -side top -expand 0 -fill y -padx 10 -pady 5

             label $info.labmem -text "Mem :" -width 10 -justify left
             label $info.valmem -textvariable ::bdi_tools_cdl::mem -width 10 -justify left
             grid $info.labmem $info.valmem -sticky w
             
         #--- Cree un frame pour afficher les boutons
         set center [frame $frm.actions  -borderwidth 2 -cursor arrow -relief groove]
         pack $center  -in $frm -anchor s -side bottom -expand 0 -fill x -padx 10 -pady 5

         #--- Cree un frame pour afficher les boutons
         set actions [frame $center.actions  -borderwidth 4 -cursor arrow -relief groove]
         pack $actions  -in $center -anchor s -side top -expand 0 -fill y -padx 10 -pady 5

              button $actions.ressource -text "Ressource" -borderwidth 2 -takefocus 1 \
                 -command "::bddimages::ressource"
              button $actions.relance -text "Relance" -borderwidth 2 -takefocus 1 \
                 -command "::bdi_gui_cdl::relance"
              button $actions.clean -text "Clean" -borderwidth 2 -takefocus 1 \
                 -command "console::clear"

              button $actions.chargexml -text "Charge XML" -borderwidth 2 -takefocus 1 \
                 -command "::bdi_tools_cdl::charge_cata_xml"
              button $actions.chargelist -text "Charge LIST" -borderwidth 2 -takefocus 1 \
                 -command "::bdi_tools_cdl::charge_cata_list"
              button $actions.voir -text "Voir" -borderwidth 2 -takefocus 1 \
                 -command "::bdi_gui_cdl::affiche_data"

              button $actions.fermer -text "Fermer" -borderwidth 2 -takefocus 1 \
                 -command "::bdi_gui_cdl::fermer"
              button $actions.aide -text "Aide" -borderwidth 2 -takefocus 1 \
                 -command "" -state disabled
              
              grid $actions.ressource -row 0 -column 0 -sticky news
              grid $actions.relance   -row 0 -column 1 -sticky news
              grid $actions.clean     -row 0 -column 2 -sticky news

              grid $actions.chargexml  -row 1 -column 0 -sticky news
              grid $actions.chargelist -row 1 -column 1 -sticky news
              grid $actions.voir       -row 1 -column 2 -sticky news

              grid $actions.aide      -row 2 -column 0 -sticky news
              grid $actions.fermer    -row 2 -column 1 -sticky news

   }
