#
# File : cmanimate.tcl
# Description : Animation/slides control panel for Cloud Monitor
# Author : Sylvain Rondi
# Release Date : April 2002
#
# Date de mise a jour : 14 janvier 2006
#
#****************************************************************
#
#
#
#Who		When		What
#--------	---------	----------------------------------------------------
#srondi	2002-07-XX  modified
#

#****************************************************************
#		NAME
#			cmanimate - Animate the FITS images from MASCOT
#
#		SYNOPSIS
#
#
#		DESCRIPTION
#			Permit the user to browse and animate the images from the
#                 Mini All Sky Cloud Observation Tool and to display some
#			information (positions of the UT's)
#
#------------------------------------------------------------------------------------

package provide cmanimate 1.0

namespace eval ::cmanim {
   variable This
   variable parametres
   global audace

   #--- Chargement des captions
   source [ file join $audace(rep_plugin) tool cmanimate cmanimate.cap ]

   proc init { { in "" } } {
      createPanel $in.cmanim
   }

   proc createPanel { this } {
      variable This
      global conf
      global audace
      global panneau
      global caption
      global cmconf
      global numero

      #--- Chargement des variables de configuration de cmaude
      set fichier_cmaude [ file join $audace(rep_plugin) tool cmaude cmaude_ini.tcl ]
      if { [ file exists $fichier_cmaude ] } {
         source $fichier_cmaude
      }
      #--- Chargement des variables de configuration
      set fichier_cmanimate [ file join $audace(rep_plugin) tool cmanimate cmanimate.ini ]
      if { [ file exists $fichier_cmanimate ] } {
         source $fichier_cmanimate
      }
      set This $this
      #---
      set panneau(menu_name,cmanim)       "$caption(cmanimate,titre)"
      set panneau(cmanim,aide)            "$caption(cmanimate,help_titre)"
      set panneau(cmanim,genericfilename) "$caption(cmanimate,nom_generique)"
      set panneau(cmanim,filename)        ""
      set panneau(cmanim,nbimages)        "$caption(cmanimate,nbre_images)"
      set panneau(cmanim,delayms)         "$caption(cmanimate,delai_ms)"
      set panneau(cmanim,nbloops)         "$caption(cmanimate,nbre_boucles)"
      set panneau(cmanim,goall)           "$caption(cmanimate,animation_totale)"
      set panneau(cmanim,golast)          "$caption(cmanimate,animation_fin)"
      set panneau(cmanim,nblast)          "10"
      set panneau(cmanim,lbllast)         "$caption(cmanimate,image)"
      set panneau(cmanim,gotolast)        "$caption(cmanimate,aller_derniere_image)"
      set panneau(cmanim,forw)            "$caption(cmanimate,image_suivante)"
      set panneau(cmanim,backw)           "$caption(cmanimate,image_precedante)"
      set panneau(cmanim,label_ima)       "$caption(cmanimate,status)"
      set panneau(cmanim,status)          "$caption(cmanimate,status1)"
      set panneau(cmanim,goimg)           "$caption(cmanimate,aller_image)"
      set panneau(cmanim,numimg)          "1"
      set numero                          "1"
      cmanimBuildIF $This
   }

   proc Chargement_Var { } {
      variable parametres
      global audace

      #--- Ouverture du fichier de parametres
      set fichier [ file join $audace(rep_plugin) tool cmanimate cmanimate.ini ]
      if { [ file exists $fichier ] } {
         source $fichier
      }
      if { ! [ info exists parametres(cmanim,position) ]} { set parametres(cmanim,position) "0" }
   }

   proc Enregistrement_Var { } {
      variable parametres
      global audace
      global panneau

      set parametres(cmanim,position) "$panneau(cmanim,position)"

      #--- Sauvegarde des parametres
      catch {
         set nom_fichier [ file join $audace(rep_plugin) tool cmanimate cmanimate.ini ]
         if [ catch { open $nom_fichier w } fichier ] {
            #---
         } else {
            foreach { a b } [ array get parametres ] { 
               puts $fichier "set parametres($a) \"$b\"" 
            }
            close $fichier
         }
      }
   }

   proc Adapt_Panneau_AcqFC { { a "" } { b "" } { c "" } } {
      variable This
      global panneau

      if { $panneau(cmanim,position) == "0" } {
         pack forget $This.frauts.case3
         pack $This.frauts.case2 -in $This.frauts -side bottom -fill none -ipadx 5 -ipady 2 -pady 2 -padx 2
         pack $This.frauts.lab.but1 -anchor center -expand 1 -fill both -side left
         pack forget $This.frauts.lab.but2
         pack forget $This.frauts.lab.but2.labURL1
         pack forget $This.frauts.lab.but2.labURL2
         pack forget $This.frauts.lab.but2.labURL3
         pack forget $This.frauts.lab.but2.labURL4
         pack forget $This.frauts.lab.but3
      } elseif { $panneau(cmanim,position) == "1" } {
         pack forget $This.frauts.case2
         pack $This.frauts.case3 -in $This.frauts -side bottom -fill none -ipadx 5 -ipady 2 -pady 2 -padx 2
         pack forget $This.frauts.lab.but1
         pack forget $This.frauts.lab.but2
         pack forget $This.frauts.lab.but2.labURL1
         pack forget $This.frauts.lab.but2.labURL2
         pack forget $This.frauts.lab.but2.labURL3
         pack forget $This.frauts.lab.but2.labURL4
         pack $This.frauts.lab.but3 -anchor center -expand 1 -fill both -side left
      } elseif { $panneau(cmanim,position) == "2" } {
         pack forget $This.frauts.case2
         pack $This.frauts.case3 -in $This.frauts -side bottom -fill none -ipadx 5 -ipady 2 -pady 2 -padx 2
         pack forget $This.frauts.lab.but1
         pack $This.frauts.lab.but2 -anchor center -expand 1 -fill both -side left
         pack $This.frauts.lab.but2.labURL1 -anchor center -expand 1 -fill both -side left
         pack $This.frauts.lab.but2.labURL2 -anchor center -expand 1 -fill both -side left
         pack $This.frauts.lab.but2.labURL3 -anchor center -expand 1 -fill both -side left
         pack $This.frauts.lab.but2.labURL4 -anchor center -expand 1 -fill both -side left
         pack forget $This.frauts.lab.but3
      }
   }

   proc startTool { visuNo } {
      variable This
      variable parametres
      global caption
      global panneau

      trace add variable ::panneau(cmanim,position) write ::cmanim::Adapt_Panneau_AcqFC
      ::cmanim::Chargement_Var
      set panneau(cmanim,position) "$parametres(cmanim,position)"
      pack $This -side left -fill y
      console::affiche_prompt "$caption(cmanimate,en_tete_1)"
      console::affiche_prompt "$caption(cmanimate,en_tete_2)"
      console::affiche_prompt "$caption(cmanimate,en_tete_3)"
      console::affiche_prompt "$caption(cmanimate,en_tete_4)"
      console::affiche_prompt "$caption(cmanimate,en_tete_5)"
      console::affiche_prompt "$caption(cmanimate,en_tete_6)"
      console::affiche_prompt "$caption(cmanimate,en_tete_7)"
      console::affiche_prompt "$caption(cmanimate,en_tete_8)"
      console::affiche_prompt "$caption(cmanimate,en_tete_9)"
      console::affiche_prompt "$caption(cmanimate,en_tete_10)"
   }

   proc stopTool { visuNo } {
      variable This
      global audace
      global panneau

      trace remove variable ::panneau(cmanim,position) write ::cmanim::Adapt_Panneau_AcqFC
      ::cmanim::Enregistrement_Var
      pack forget $This
      if { [ winfo exists $audace(base).erreurfichier ] } {
         destroy $audace(base).erreurfichier
      }
      if { [ winfo exists $audace(base).position_tel ] } {
         destroy $audace(base).position_tel
      }
   }

   proc cmdGoall { } {
      variable This
      global audace
      global panneau
      global caption

      #--- Clean the screen
      visu$audace(visuNo) clear
      #---
      if { $panneau(cmanim,encours) == "0" } {
         #--- Efface la position des telescopes
         $audace(hCanvas) delete uts
         set panneau(cmanim,encours) "1"
         grab $This.frago.but1
         $This.frago.but1 configure -relief groove
         set panneau(cmanim,status) "$caption(cmanimate,animation_en_cours)"
         $This.fra6.labURL2 configure -text "$panneau(cmanim,status)"
         update
         #--- In case of errors (wrong folder or doesn't exist)
         #--- If no error, function ANIMATE
         set num [ catch { animate_mascot $panneau(cmanim,filename) $panneau(cmanim,nbi) $panneau(cmanim,ms) \
            $panneau(cmanim,nbl) } msg ]
         if { $num == "1" } {
            ::cmanim::ErreurFichier
         }
         grab release $This.frago.but1
         $This.frago.but1 configure -relief raised
         update
         set panneau(cmanim,encours) "0"
         set panneau(cmanim,status) "$caption(cmanimate,animation_terminee)"
         $This.fra6.labURL2 configure -text "$panneau(cmanim,status)"
         cmdchkuts_1
      }
   }

   proc animate_mascot { filename nb {millisecondes 200} {nbtours 10} } {
      #--- filename : Nom generique des fichiers a animer
      #--- nb : Nombre d'images (1 a nb)
      #--- millisecondes : Timer entre chaque image affichee
      #--- nbtours : Nombre de boucles sur les nb images
      #
      global conf
      global audace
      global panneau
      global caption
      global color

      set len [string length $conf(rep_images)]
      set folder "$conf(rep_images)"
      if { $len> "0" } {
         set car [string index "$conf(rep_images)" [expr $len-1]]
         if { $car != "/" } {
            append folder "/"
         }
      }
      set basecanvas $audace(base).can1.canvas
      #--- Je sauvegarde le numero de l'image associe � la visu
      set imageNo [visu$audace(visuNo) image]
      #--- On va creer nb zones de visu a partir de 101
      set off 100
      #--- Cree les visu et les Tk_photoimage
      for { set k 1 } { $k <= $nb } { incr k } {
         set kk [expr $k+$off]
         #--- Cree l'image si elle n'existe pas et l'associe a la visu
         visu$audace(visuNo) image $kk
         buf$audace(bufNo) load "$folder$filename$k"
         #---
         if { $panneau(cmanim,drawposuts) == "1" } {
            if { $panneau(cmanim,position) == "2" } {
            #--- Positionnement pour l'option Paranal avec des images en binning 1x1
               set nom_image [ file join $audace(rep_images) $panneau(cmanim,filename)$k$conf(extension,defaut) ]
               catch {
                  set altut1($k) [lindex [buf$audace(bufNo) getkwd "ALTUT1"] 1]
                  set aziut1($k) [lindex [buf$audace(bufNo) getkwd "AZUT1"] 1]
                  set altut2($k) [lindex [buf$audace(bufNo) getkwd "ALTUT2"] 1]
                  set aziut2($k) [lindex [buf$audace(bufNo) getkwd "AZUT2"] 1]
                  set altut3($k) [lindex [buf$audace(bufNo) getkwd "ALTUT3"] 1]
                  set aziut3($k) [lindex [buf$audace(bufNo) getkwd "AZUT3"] 1]
                  set altut4($k) [lindex [buf$audace(bufNo) getkwd "ALTUT4"] 1]
                  set aziut4($k) [lindex [buf$audace(bufNo) getkwd "AZUT4"] 1]
               }
            } elseif { $panneau(cmanim,position) == "1" } {            
            #--- Positionnement pour l'option 'Votre instrument' avec des images en binning 1x1
               #--- A developper, recuperation des coordonnees de pointage (lx200, audecom, ouranos, etc.)
            }
         }
         #--- Affiche l'image associee a la visu
         ::audace::autovisu $audace(visuNo)
      }
      #--- Animation
      for { set t 1 } { $t <= $nbtours } { incr t } {   
         for { set k 1 } { $k <= $nb } { incr k } {
            set kk [expr $k+$off]
            #---
            if { $panneau(cmanim,drawposuts) == "1" } {
               if { $panneau(cmanim,position) == "2" } {
               #--- Positionnement pour l'option Paranal avec des images en binning 1x1
                  catch {
                     set altut(1) $altut1($k)
                     set aziut(1) $aziut1($k)
                     set altut(2) $altut2($k)
                     set aziut(2) $aziut2($k)
                     set altut(3) $altut3($k)
                     set aziut(3) $aziut3($k)
                     set altut(4) $altut4($k)
                     set aziut(4) $aziut4($k)
                     altaz2oval $altut(1) $aziut(1) "$caption(cmanimate,ut1)" "$color(red)" "1" "13" "uts"
                     altaz2oval $altut(2) $aziut(2) "$caption(cmanimate,ut2)" "$color(yellow)" "1" "13" "uts"
                     altaz2oval $altut(3) $aziut(3) "$caption(cmanimate,ut3)" "$color(green)" "1" "13" "uts"
                     altaz2oval $altut(4) $aziut(4) "$caption(cmanimate,ut4)" "$color(blue)" "1" "13" "uts"
                  }
               } elseif { $panneau(cmanim,position) == "1" } {
               #--- Positionnement pour l'option 'Votre instrument' avec des images en binning 1x1
                  #--- A developper, recuperation des coordonnees de pointage (lx200, audecom, ouranos, etc.)
               }
            }
            $basecanvas itemconfigure display -image image$kk
            #--- Change l'image associee a la visu
            visu$audace(visuNo) image $kk
            update
            after $millisecondes
            #--- Effacement des surimpressions
            $audace(hCanvas) delete uts
         }
      }
      #--- Detruit les Tk_photoimage
      for { set k 1 } { $k <= $nb } { incr k } {
         set kk [expr $k+$off]
         image delete image$kk
      }
      #--- Reconfigure pour Aud'ACE normal
      $basecanvas itemconfigure display -image image$imageNo
      update
      #--- Je restaure le numero de l'image associe a la visu
      visu$audace(visuNo) image $imageNo
      buf$audace(bufNo) load $folder${filename}1
      ::audace::autovisu $audace(visuNo)
   }

   proc cmdGolast { } {
      variable This
      global audace
      global conf
      global panneau
      global caption
      global numbrstart
      global numbrend

      set numbrend "0"
      set num "1"
      while { $num != "0" } {
         incr numbrend 1
         set nom_image [ file join $audace(rep_images) $panneau(cmanim,filename)$numbrend$conf(extension,defaut) ]
         set num [ file exists $nom_image ]
      }
      incr numbrend -1
      set numbrstart [expr $numbrend - $panneau(cmanim,nblast) + 1]
      if { $numbrstart <= "0" } {
         set numbrstart "1"
      }
      #--- Clean the screen
      visu$audace(visuNo) clear
      #---
      if { $panneau(cmanim,encours) == "0" } {
         #--- Efface la position des telescopes
         $audace(hCanvas) delete uts
         set panneau(cmanim,encours) "1"
         grab $This.frago.but1
         $This.frago.but5 configure -relief groove
         $This.frago.but1 configure -relief groove
         set panneau(cmanim,status) "$caption(cmanimate,animation_en_cours)"
         $This.fra6.labURL2 configure -text "$panneau(cmanim,status)"
         update
         #--- In case of errors (wrong folder or doesn't exist)
         #--- If no error, function ANIMATE
         set num [ catch { ::cmanim::animate_mascot_last $panneau(cmanim,filename) $numbrstart $numbrend \
            $panneau(cmanim,ms) $panneau(cmanim,nbl) } msg ]
         if { $num == "1" } {
            ::cmanim::ErreurFichier
         }
         grab release $This.frago.but1
         $This.frago.but1 configure -relief raised
         $This.frago.but5 configure -relief raised
         update
         set panneau(cmanim,encours) "0"
         set panneau(cmanim,status) "$caption(cmanimate,animation_terminee)"
         $This.fra6.labURL2 configure -text "$panneau(cmanim,status)"
         cmdchkuts_1
      }
   }

   proc animate_mascot_last { filename numbrstart numbrend millisecondes nbtours } {
      #--- filename : Nom generique des fichiers filename*.extension a animer
      #--- numbrstart : Numero d'image de debut
      #--- numbrend : Numero d'image de fin
      #--- millisecondes : Timer entre chaque image affichee
      #--- nbtours : Nombre de boucles sur les nb images
      global conf
      global audace
      global panneau
      global caption
      global color

      set len [string length $conf(rep_images)]
      set folder "$conf(rep_images)"
      if { $len > "0" } {
         set car [string index "$conf(rep_images)" [expr $len-1]]
         if { $car != "/" } {
            append folder "/"
         }
      }
      set basecanvas $audace(base).can1.canvas
      #--- Je sauvegarde le numero de l'image associe � la visu
      set imageNo [visu$audace(visuNo) image]
      #--- On va creer nb zones de visu a partir de 101
      set off "100"
      #--- Cree les visu et les Tk_photoimage
      for { set k $numbrstart } { $k <= $numbrend } { incr k } {
         set kk [expr $k+$off]
         #--- Cree l'image si elle n'existe pas et l'associe a la visu
         visu$audace(visuNo) image $kk
         buf$audace(bufNo) load "$folder$filename$k"
         #---
         if { $panneau(cmanim,drawposuts) == "1" } {
            if { $panneau(cmanim,position) == "2" } {
            #--- Positionnement pour l'option Paranal avec des images en binning 1x1
               set nom_image [ file join $audace(rep_images) $panneau(cmanim,filename)$k$conf(extension,defaut) ]
               catch {
                  set altut1($k) [lindex [buf$audace(bufNo) getkwd "ALTUT1"] 1]
                  set aziut1($k) [lindex [buf$audace(bufNo) getkwd "AZUT1"] 1]
                  set altut2($k) [lindex [buf$audace(bufNo) getkwd "ALTUT2"] 1]
                  set aziut2($k) [lindex [buf$audace(bufNo) getkwd "AZUT2"] 1]
                  set altut3($k) [lindex [buf$audace(bufNo) getkwd "ALTUT3"] 1]
                  set aziut3($k) [lindex [buf$audace(bufNo) getkwd "AZUT3"] 1]
                  set altut4($k) [lindex [buf$audace(bufNo) getkwd "ALTUT4"] 1]
                  set aziut4($k) [lindex [buf$audace(bufNo) getkwd "AZUT4"] 1]
               }
            } elseif { $panneau(cmanim,position) == "1" } {
            #--- Positionnement pour l'option 'Votre instrument' avec des images en binning 1x1
               #--- A developper, recuperation des coordonnees de pointage (lx200, audecom, ouranos, etc.)
            }
         }
         #--- Affiche l'image associee a la visu
         ::audace::autovisu $audace(visuNo)
      }
      #--- Animation
      for { set t 1 } { $t <= $nbtours } { incr t } {
         for { set k $numbrstart } { $k <= $numbrend } { incr k } {
            set kk [expr $k+$off]
            #---
            if { $panneau(cmanim,drawposuts) == "1" } {
               if { $panneau(cmanim,position) == "2" } {
               #--- Positionnement pour l'option Paranal avec des images en binning 1x1
                  catch {
                     set altut(1) $altut1($k)
                     set aziut(1) $aziut1($k)
                     set altut(2) $altut2($k)
                     set aziut(2) $aziut2($k)
                     set altut(3) $altut3($k)
                     set aziut(3) $aziut3($k)
                     set altut(4) $altut4($k)
                     set aziut(4) $aziut4($k)
                     altaz2oval $altut(1) $aziut(1) "$caption(cmanimate,ut1)" "$color(red)" "1" "13" "uts"
                     altaz2oval $altut(2) $aziut(2) "$caption(cmanimate,ut2)" "$color(yellow)" "1" "13" "uts"
                     altaz2oval $altut(3) $aziut(3) "$caption(cmanimate,ut3)" "$color(green)" "1" "13" "uts"
                     altaz2oval $altut(4) $aziut(4) "$caption(cmanimate,ut4)" "$color(blue)" "1" "13" "uts"
                  }
               } elseif { $panneau(cmanim,position) == "1" } {
               #--- Positionnement pour l'option 'Votre instrument' avec des images en binning 1x1
                  #--- A developper, recuperation des coordonnees de pointage (lx200, audecom, ouranos, etc.)
               }
            }
            $basecanvas itemconfigure display -image image$kk
            #--- Change l'image associee a la visu
            visu$audace(visuNo) image $kk
            update
            after $millisecondes
            #--- Effacement des surimpressions
            $audace(hCanvas) delete uts
         }
      }
      #--- Detruit les Tk_photoimage
      for { set k $numbrstart } { $k <= $numbrend } { incr k } {
         set kk [expr $k+$off]
         image delete image$kk
      }
      #--- Reconfigure pour Aud'ACE normal
      $basecanvas itemconfigure display -image image$imageNo
      update
      #--- Je restaure le numero de l'image associe a la visu
      visu$audace(visuNo) image $imageNo
      buf$audace(bufNo) load $folder${filename}1
      ::audace::autovisu $audace(visuNo)
   }

   proc cmdForw { } {
   #--- Push on FORWARD button, pass to the following image
      variable This
      global conf
      global audace
      global panneau
      global caption
      global numero

      incr numero 1
      set nom_image [ file join $audace(rep_images) $panneau(cmanim,filename)$numero$conf(extension,defaut) ]
      set num [ catch { loadima $nom_image } msg ]
      if { $num == "1" } {
         incr numero -1
         ::cmanim::ErreurFichier
      } else {
         set datefits [lindex [buf$audace(bufNo) getkwd DATE-OBS] 1]
         set panneau(cmanim,status) "$caption(cmanimate,image_numero)$numero - [string range $datefits 0 15]"
         $This.fra6.labURL2 configure -text "$panneau(cmanim,status)"
         cmdchkuts_1
      }
   }

   proc cmdBackw { } {
   #--- Push on BACKWARD button, pass to the previous image
      variable This
      global conf
      global audace
      global panneau
      global caption
      global numero

      incr numero -1
      if { $numero < "1" } {
         set numero "1"
         ::cmanim::ErreurFichier
      }
      set nom_image [ file join $audace(rep_images) $panneau(cmanim,filename)$numero$conf(extension,defaut) ]
      set num [ catch { loadima $nom_image } msg ]
      if { $num == "1" } then {
         incr numero 1
         ::cmanim::ErreurFichier
      } else {
         set datefits [lindex [buf$audace(bufNo) getkwd DATE-OBS] 1]
         set panneau(cmanim,status) "$caption(cmanimate,image_numero)$numero - [string range $datefits 0 15]"
         $This.fra6.labURL2 configure -text "$panneau(cmanim,status)"
         cmdchkuts_1
      }
   }

   proc cmdGotolast { } {
   #--- Push on Go To Last button, pass to the last image available
      variable This
      global conf
      global audace
      global panneau
      global caption
      global numero

      set numero "1"
      set num "1"
      while { $num != "0" } {
         incr numero 1
         set nom_image [ file join $audace(rep_images) $panneau(cmanim,filename)$numero$conf(extension,defaut) ]
         set num [ file exists $nom_image ]
      }
      incr numero -1
      set nom_image [ file join $audace(rep_images) $panneau(cmanim,filename)$numero$conf(extension,defaut) ]
      set num [catch {loadima $nom_image} msg]
      if { $num == "1" } then {
         ::cmanim::ErreurFichier
      } else {
         set datefits [lindex [buf$audace(bufNo) getkwd DATE-OBS] 1]
         set panneau(cmanim,status) "$caption(cmanimate,image_numero)$numero - [string range $datefits 0 15]"
         $This.fra6.labURL2 configure -text "$panneau(cmanim,status)"
         cmdchkuts_1
      }
   }

   proc cmdGoto { } {
   #--- Push on Go To button, pass to the image number "numero"
      variable This
      global conf
      global audace
      global panneau
      global caption
      global numero

      set numero "$panneau(cmanim,numimg)"
      set nom_image [ file join $audace(rep_images) $panneau(cmanim,filename)$numero$conf(extension,defaut) ]
      set num [catch {loadima $nom_image} msg]
      if { $num == "1" } then {
         ::cmanim::ErreurFichier
         set numero "1"
      } else {
         set datefits [lindex [buf$audace(bufNo) getkwd DATE-OBS] 1]
         set panneau(cmanim,status) "$caption(cmanimate,image_numero)$numero - [string range $datefits 0 15]"
         $This.fra6.labURL2 configure -text "$panneau(cmanim,status)"
         cmdchkuts_1
      }
   }

   proc cmdchkgrid { } {
      global audace
      global panneau
      global caption

      if { $panneau(cmanim,drawgrid) == "1" } then {
         console::affiche_erreur "$caption(cmanimate,dessine_grille)"
         cmdGrid
      } else {
         console::affiche_erreur "$caption(cmanimate,efface_grille)"
         $audace(hCanvas) delete thegrid
      }
   }

   proc cmdGrid { } {
      global conf
      global audace
      global caption
      global panneau
      global color
      global cmconf

      if { $panneau(cmanim,position) == "2" } {
      #--- Grille pour l'option Paranal avec des images en binning 1x1
         #--- Draw an AltAz grid over the image
         set centerx [lindex $cmconf(zenith11) 0]
         set centery [lindex $cmconf(zenith11) 1]
         foreach anglegrid { 0. 15. 30. 50. 70. } {
            #--- The radius of the image is 275 pixels
            set centeroff [expr 285.0 - ($anglegrid / 90.0 * 285.0)]
            set x1 [expr $centerx-$centeroff]
            set y1 [expr $centery-$centeroff]
            set x2 [expr $centerx+$centeroff]
            set y2 [expr $centery+$centeroff]
            set offsetxtxt [expr $x1+15.0]
            set offsetytxt [expr $centery-5.0]
            $audace(hCanvas) create oval $x1 $y1 $x2 $y2 -outline $color(violet) -tags thegrid
            $audace(hCanvas) create text $offsetxtxt $offsetytxt -text "$anglegrid$caption(cmanimate,degres)" \
               -fill $color(violet) -tags thegrid
         }
         set x1 [expr $centerx-295.0]
         set y1 [expr $centery-295.0]
         set x2 [expr $centerx+295.0]
         set y2 [expr $centery+285.0]
         $audace(hCanvas) create line $x1 $centery $x2 $centery -fill $color(violet) -tags thegrid
         $audace(hCanvas) create line $centerx $y1 $centerx $y2 -fill $color(violet) -tags thegrid
         for { set anglegrid2 20 } { $anglegrid2 < 361 } { incr anglegrid2 20 } {
            altaz2oval "20" $anglegrid2 $anglegrid2 "$color(violet)" "1" "0" "thegrid"
            altaz2oval "15" $anglegrid2 "" "$color(violet)" "1" "2" "thegrid"
         }
      } else {
      #--- Grille pour les 2 autres options avec des images en binning 1x1
         #--- A developper pour le cas specifique, car depend du champ et de l'orientation de la camera
      }
   }

   proc altaz2oval { altut aziut utID color_cmanimate width radius tag } {
      global cmconf
      global conf
      global audace
      global caption

      set centerx [lindex $cmconf(zenith11) 0]
      set centery [lindex $cmconf(zenith11) 1]
      #--- Convert the {altitude azimuth} position into the {x1 y1 x2 y2} oval coordinates
      #--- This is the radius from the center given by the altitude
      set altdeci [expr 285.0 - ($altut / 90.0 * 285.0)]
      #--- This is X and Y
      set xpos [expr $centerx + ($altdeci * sin ($aziut / 180. * 3.1415))]
      set ypos [expr $centery - ($altdeci * cos ($aziut / 180. * 3.1415))]
      set x1 [expr $xpos - $radius]
      set y1 [expr $ypos - $radius]
      set x2 [expr $xpos + $radius]
      set y2 [expr $ypos + $radius]
      $audace(hCanvas) create oval $x1 $y1 $x2 $y2 -outline $color_cmanimate -width $width -tags $tag
      $audace(hCanvas) create text $xpos $ypos -text "$utID$caption(cmanimate,degres)" -fill $color_cmanimate -tags $tag
   }

   proc cmdchkuts { } {
      global audace
      global panneau
      global caption

      if { $panneau(cmanim,drawposuts) == "1" } then {
         $audace(hCanvas) delete uts
         if { $panneau(cmanim,position) == "1" } {
            console::affiche_erreur "$caption(cmanimate,dessine_position)"
         } else {
            console::affiche_erreur "$caption(cmanimate,dessine_positions)"
         }
         catch { cmdDrawuts }
      } else {
         if { $panneau(cmanim,position) == "1" } {
            console::affiche_erreur "$caption(cmanimate,efface_position)"
         } else {
            console::affiche_erreur "$caption(cmanimate,efface_positions)"
         }
         $audace(hCanvas) delete uts
      }
   }

   proc cmdchkuts_1 { } {
      global audace
      global panneau
      global caption
      global color

      if { $panneau(cmanim,drawposuts) == "1" } then {
         $audace(hCanvas) delete uts
         catch { cmdDrawuts }
      } else {
         $audace(hCanvas) delete uts
      }
   }

   proc cmdDrawuts { } {
   #--- Draw the position of the UT's on the image
      global audace
      global panneau
      global caption
      global color

      if { $panneau(cmanim,position) == "2" } {
      #--- Positionnement pour l'option Paranal avec des images en binning 1x1
         #--- Prepare the datas
         #--- Erase the previous overlay
         #--- Put the position from the header into local variables (optionnal?)
         set altut1 [lindex [buf$audace(bufNo) getkwd "ALTUT1"] 1]
         set aziut1 [lindex [buf$audace(bufNo) getkwd "AZUT1"] 1]
         set altut2 [lindex [buf$audace(bufNo) getkwd "ALTUT2"] 1]
         set aziut2 [lindex [buf$audace(bufNo) getkwd "AZUT2"] 1]
         set altut3 [lindex [buf$audace(bufNo) getkwd "ALTUT3"] 1]
         set aziut3 [lindex [buf$audace(bufNo) getkwd "AZUT3"] 1]
         set altut4 [lindex [buf$audace(bufNo) getkwd "ALTUT4"] 1]
         set aziut4 [lindex [buf$audace(bufNo) getkwd "AZUT4"] 1]
         #--- Put the wind parameters
         set aziwind "340"
         set forcewind "5"
         #--- Draw the circles on the canvas
         #--- Options : alti, azim, name, color, thickness, radius
         altaz2oval $altut1 $aziut1 "$caption(cmanimate,ut1)" "$color(red)" "1" "13" "uts"
         altaz2oval $altut2 $aziut2 "$caption(cmanimate,ut2)" "$color(yellow)" "1" "13" "uts"
         altaz2oval $altut3 $aziut3 "$caption(cmanimate,ut3)" "$color(green)" "1" "13" "uts"
         altaz2oval $altut4 $aziut4 "$caption(cmanimate,ut4)" "$color(blue)" "1" "13" "uts"
         drawwind $aziwind $forcewind
      } elseif { $panneau(cmanim,position) == "1" } {
      #--- Positionnement pour l'option 'Votre instrument' avec des images en binning 1x1
         #--- A developper, recuperation des coordonnees de pointage (lx200, audecom, ouranos, etc.)
      }
   }

   proc drawwind { aziwind forcewind } {
      global audace
      global caption
      global color

      #--- Convert the {altitude azimuth} position into the {x1 y1 x2 y2} oval coordinates
      #--- This is the radius from the center given by the altitude
      #--- This is X and Y
      set xpos [expr ($forcewind * sin ($aziwind / 180. * 3.1415))]
      set ypos [expr ($forcewind * -1. * cos ($aziwind / 180. * 3.1415))]
      set x1 "545"
      set y1 "60"
      set x2 [expr 545. + $xpos]
      set y2 [expr 60. + $ypos]
      #--- 14m/s speed (pointing limit)
      $audace(hCanvas) create oval 536 51 564 79 -outline $color(cyan) -width 1 -tags uts
      #--- 18m/s speed (close dome)
      $audace(hCanvas) create oval 532 47 568 83 -outline $color(cyan) -width 1 -tags uts
      #--- 25m/s speed
      $audace(hCanvas) create oval 525 40 575 90 -outline $color(cyan) -width 1 -tags uts
      $audace(hCanvas) create line $x1 $y1 $x2 $y2 -fill $color(cyan) -width 1 -tags uts
      $audace(hCanvas) create text 515 45 -text "$caption(cmanimate,vent)" -fill $color(cyan) -tags uts
   }

   proc ErreurFichier { } {
   #--- Notice the user of a wrong folder or file
      global audace
      global caption

      if { [ winfo exists $audace(base).erreurfichier ] } {
         destroy $audace(base).erreurfichier
      }
      toplevel $audace(base).erreurfichier
      wm transient $audace(base).erreurfichier $audace(base)
      wm title $audace(base).erreurfichier "$caption(cmanimate,attention)"
      set posx_erreurfichier [ lindex [ split [ wm geometry $audace(base) ] "+" ] 1 ]
      set posy_erreurfichier [ lindex [ split [ wm geometry $audace(base) ] "+" ] 2 ]
      wm geometry $audace(base).erreurfichier +[ expr $posx_erreurfichier + 170 ]+[ expr $posy_erreurfichier + 102 ]
      wm resizable $audace(base).erreurfichier 0 0
      #--- Create the message
      label $audace(base).erreurfichier.lab1 -text "$caption(cmanimate,erreur1)"
      pack $audace(base).erreurfichier.lab1 -padx 10 -pady 2
      label $audace(base).erreurfichier.lab2 -text "$caption(cmanimate,erreur2)"
      pack $audace(base).erreurfichier.lab2 -padx 10 -pady 2
      label $audace(base).erreurfichier.lab3 -text "$caption(cmanimate,erreur3)"
      pack $audace(base).erreurfichier.lab3 -padx 10 -pady 2
      #--- New message window is on
      focus $audace(base).erreurfichier
      #--- Mise a jour dynamique des couleurs
      ::confColor::applyColor $audace(base).erreurfichier
   }

   proc Position_Tel { } {
   #--- Notice the user of a wrong folder or file
      global audace
      global caption
      global panneau

      if { [ winfo exists $audace(base).position_tel ] } {
         destroy $audace(base).position_tel
      }
      toplevel $audace(base).position_tel
      wm transient $audace(base).position_tel $audace(base)
      wm title $audace(base).position_tel "$caption(cmanimate,position_tel)"
      set posx_position_tel [ lindex [ split [ wm geometry $audace(base) ] "+" ] 1 ]
      set posy_position_tel [ lindex [ split [ wm geometry $audace(base) ] "+" ] 2 ]
      wm geometry $audace(base).position_tel 280x90+[ expr $posx_position_tel + 170 ]+[ expr $posy_position_tel + 102 ]
      wm resizable $audace(base).position_tel 0 0
      #--- Create the message
      radiobutton $audace(base).position_tel.rad1 -anchor nw -highlightthickness 0 -padx 0 -pady 0 \
         -text "$caption(cmanimate,non)" -value 0 -variable panneau(cmanim,position) -command {  }
      pack $audace(base).position_tel.rad1 -padx 10 -pady 5
      radiobutton $audace(base).position_tel.rad2 -anchor nw -highlightthickness 0 -padx 0 -pady 0 \
         -text "$caption(cmanimate,oui)" -value 1 -variable panneau(cmanim,position) -command {  }
      pack $audace(base).position_tel.rad2 -padx 10 -pady 5
      radiobutton $audace(base).position_tel.rad3 -anchor nw -highlightthickness 0 -padx 0 -pady 0 \
         -text "$caption(cmanimate,paranal)" -value 2 -variable panneau(cmanim,position) -command {  }
      pack $audace(base).position_tel.rad3 -padx 10 -pady 5
      #--- New message window is on
      focus $audace(base).position_tel
      #--- Mise a jour dynamique des couleurs
      ::confColor::applyColor $audace(base).position_tel
   }

   proc edit_nom_image { } {
      global audace
      global panneau

      #--- Fenetre parent
      set fenetre "$audace(base)"
      #--- Ouvre la fenetre de choix des images
      set filename [ ::tkutil::box_load $fenetre $audace(rep_images) $audace(bufNo) "1" ]
      #--- Extraction du nom generique
      set panneau(cmanim,filename) [ lindex [ decomp $filename ] 1 ]
   }

   proc Nom_gene { } {
      global audace
      global caption

      if { [ winfo exists $audace(base).nom_gene ] } {
         destroy $audace(base).nom_gene
      }
      toplevel $audace(base).nom_gene
      wm transient $audace(base).nom_gene $audace(base)
      wm title $audace(base).nom_gene "$caption(cmanimate,attention)"
      set posx_nom_gene [ lindex [ split [ wm geometry $audace(base) ] "+" ] 1 ]
      set posy_nom_gene [ lindex [ split [ wm geometry $audace(base) ] "+" ] 2 ]
      wm geometry $audace(base).nom_gene +[ expr $posx_nom_gene + 170 ]+[ expr $posy_nom_gene + 102 ]
      wm resizable $audace(base).nom_gene 0 0

      #--- Cree l'affichage du message
      label $audace(base).nom_gene.lab1 -text "$caption(cmanimate,message1)"
      pack $audace(base).nom_gene.lab1 -padx 10 -pady 2
      label $audace(base).nom_gene.lab2 -text "$caption(cmanimate,message2)"
      pack $audace(base).nom_gene.lab2 -padx 10 -pady 2
      label $audace(base).nom_gene.lab3 -text "$caption(cmanimate,message3)"
      pack $audace(base).nom_gene.lab3 -padx 10 -pady 2

      #--- La nouvelle fenetre est active
      focus $audace(base).nom_gene

      #--- Mise a jour dynamique des couleurs
      ::confColor::applyColor $audace(base).nom_gene
   }

#--- End of the procedures
}

proc cmanimBuildIF { This } {
#======================
#=== Panel graphism ===
#======================
   global audace
   global panneau
   global caption
   global color

   #--- Initialisation des variables du panneau
   set panneau(cmanim,encours)  "0"
   if { [ info exists panneau(cmanim,filename) ] == "0" } { set panneau(cmanim,filename) "" }
   if { [ info exists panneau(cmanim,nbi) ] == "0" }      { set panneau(cmanim,nbi)      "3" }
   if { [ info exists panneau(cmanim,ms) ] == "0" }       { set panneau(cmanim,ms)       "100" }
   if { [ info exists panneau(cmanim,nbl) ] == "0" }      { set panneau(cmanim,nbl)      "2" }

   frame $This -borderwidth 2 -relief groove

      #--- Title frame
      frame $This.fra1 -borderwidth 2 -relief groove

         #--- Label for title
         Button $This.fra1.but -borderwidth 1 -text $panneau(menu_name,cmanim) \
            -command {
               ::audace::showHelpPlugin tool cmanimate cmanimate.htm
            }
         pack $This.fra1.but -in $This.fra1 -anchor center -expand 1 -fill both -side top -ipadx 5
         DynamicHelp::add $This.fra1.but -text $panneau(cmanim,aide)

      pack $This.fra1 -side top -fill x

      #--- Frame for generic name
      frame $This.fra2 -borderwidth 1 -relief groove

         #--- Label for generic name
         label  $This.fra2.lab1 -text "$panneau(cmanim,genericfilename)" -relief flat
         pack   $This.fra2.lab1 -in $This.fra2 -anchor center -fill none -padx 4 -pady 2

         #--- Entry for generic name
         entry  $This.fra2.ent1 -font $audace(font,arial_8_b) -textvariable panneau(cmanim,filename) -relief groove
         pack   $This.fra2.ent1 -in $This.fra2 -anchor center -fill none -padx 4 -pady 4
         bind $This.fra2.ent1 <Enter> { ::cmanim::Nom_gene }
         bind $This.fra2.ent1 <Leave> { destroy $audace(base).nom_gene }

      pack $This.fra2 -side top -fill x

      #--- Frame for the number of images
      frame $This.fra3 -borderwidth 1 -relief groove

         #--- Label for the number of images
         label  $This.fra3.lab1 -text "$panneau(cmanim,nbimages)" -relief flat
         pack   $This.fra3.lab1 -in $This.fra3 -anchor center -expand true -fill none -side left

         #--- Entry for the number of images
         entry  $This.fra3.ent1 -font $audace(font,arial_8_b) -textvariable panneau(cmanim,nbi) -relief groove \
            -width 4 -justify center
         pack   $This.fra3.ent1 -in $This.fra3 -anchor center -expand true -fill none -side left -pady 4

      pack $This.fra3 -side top -fill x

      #--- Frame for the delay
      frame $This.fra4 -borderwidth 1 -relief groove

         #--- Label for the delay
         label  $This.fra4.lab1 -text "$panneau(cmanim,delayms)" -relief flat
         pack   $This.fra4.lab1 -in $This.fra4 -anchor center -expand true -fill none -side left

         #--- Entry for the delay
         entry  $This.fra4.ent1 -font $audace(font,arial_8_b) -textvariable panneau(cmanim,ms) -relief groove \
            -width 5 -justify center
         pack   $This.fra4.ent1 -in $This.fra4 -anchor center -expand true -fill none -side left -pady 4

      pack $This.fra4 -side top -fill x

      #--- Frame for the number of loops
      frame $This.fra5 -borderwidth 1 -relief groove

         #--- Label for the number of loops
         label  $This.fra5.lab1 -text "$panneau(cmanim,nbloops)" -relief flat
         pack   $This.fra5.lab1 -in $This.fra5 -anchor center -expand true -fill none -side left

         #--- Entry pour le nb de boucles
         entry  $This.fra5.ent1 -font $audace(font,arial_8_b) -textvariable panneau(cmanim,nbl) -relief groove \
            -width 2 -justify center
         pack   $This.fra5.ent1 -in $This.fra5 -anchor center -expand true -fill none -side left -pady 4

      pack $This.fra5 -side top -fill x

      #--- Frame of commands
      frame $This.frago -borderwidth 1 -relief groove

         #--- Bouton GO animate all
         button  $This.frago.but1 -borderwidth 2 -text "$panneau(cmanim,goall)" \
            -command { ::cmanim::cmdGoall }
         pack   $This.frago.but1 -in $This.frago -side top -fill none -padx 2 -pady 8 -ipadx 5 -ipady 8

         #--- Bouton GO animate last X images
         button  $This.frago.but5 -borderwidth 2 -text "$panneau(cmanim,golast)" \
            -command { ::cmanim::cmdGolast }
         pack   $This.frago.but5 -in $This.frago -side left -expand true -pady 8 -ipadx 3 -ipady 3

         #--- Entry for the number of last images
         entry  $This.frago.ent2 -font $audace(font,arial_8_b) -textvariable panneau(cmanim,nblast) -relief groove \
            -width 3 -justify center
         pack   $This.frago.ent2 -in $This.frago -side left -expand true

         #--- Label for the number of last images
         label  $This.frago.lab1 -text "$panneau(cmanim,lbllast)" -relief flat
         pack   $This.frago.lab1 -in $This.frago -side left -expand true

      pack $This.frago -side top -fill x

      #--- Frame of image browser
      frame $This.frabrowse -borderwidth 1 -relief groove

         #--- Bouton Forward
         button $This.frabrowse.but2 -borderwidth 2 -text "$panneau(cmanim,forw)" \
            -command { ::cmanim::cmdForw }
         pack   $This.frabrowse.but2 -in $This.frabrowse -anchor center -fill none -padx 4 -pady 8 -ipadx 3 -ipady 3

         #--- Bouton Backward
         button $This.frabrowse.but3 -borderwidth 2 -text "$panneau(cmanim,backw)" \
            -command { ::cmanim::cmdBackw }
         pack   $This.frabrowse.but3 -in $This.frabrowse -anchor center -fill none -padx 4 -pady 8 -ipadx 3 -ipady 3

         #--- Button go to last image
         button $This.frabrowse.but5 -borderwidth 2 -text "$panneau(cmanim,gotolast)" \
            -command { ::cmanim::cmdGotolast }
         pack   $This.frabrowse.but5 -in $This.frabrowse -anchor center -fill none -padx 4 -pady 8 -ipadx 3 -ipady 3

         #--- Bouton Go To image
         button $This.frabrowse.but4 -borderwidth 2 -text "$panneau(cmanim,goimg)" \
            -command { ::cmanim::cmdGoto }
         pack   $This.frabrowse.but4 -in $This.frabrowse -side left -expand true -pady 8 -ipadx 3 -ipady 3

         #--- Entry for the Go To image number
         entry  $This.frabrowse.ent1 -font $audace(font,arial_8_b) -textvariable panneau(cmanim,numimg) -relief groove \
            -width 4 -justify center
         pack   $This.frabrowse.ent1 -in $This.frabrowse -side left -expand true

      pack $This.frabrowse -side top -fill x

      #--- Frame of UT's overlay
      frame $This.frauts -borderwidth 1 -relief groove

         #--- Checkcase for coordinate grid overlay
         checkbutton $This.frauts.case1 -pady 0 -text "$caption(cmanimate,grille_sur_image)" \
            -variable panneau(cmanim,drawgrid) -command { ::cmanim::cmdchkgrid }
         pack   $This.frauts.case1 -in $This.frauts -side top -fill none -padx 2 -pady 2 -ipadx 5 -ipady 2

         #--- Checkcase for position of the UT's
         checkbutton $This.frauts.case2 -pady 0 -text "$caption(cmanimate,telescope_sur_image)" \
            -state disabled
         pack   $This.frauts.case2 -in $This.frauts -side bottom -fill none -padx 2 -pady 2 -ipadx 5 -ipady 2

         #--- Checkcase for position of the UT's
         checkbutton $This.frauts.case3 -pady 0 -text "$caption(cmanimate,telescope_sur_image)" \
            -variable panneau(cmanim,drawposuts) -command { ::cmanim::cmdchkuts }
         pack   $This.frauts.case3 -in $This.frauts -side bottom -fill none -padx 2 -pady 2 -ipadx 5 -ipady 2

         #--- Labels color of the UT's
         frame $This.frauts.lab -borderwidth 0 -height 100 -relief groove
            button $This.frauts.lab.but1 -borderwidth 2 -text "$caption(cmanimate,pas_instrument)" \
               -font $audace(font,arial_10_b) -command { ::cmanim::Position_Tel }
            pack $This.frauts.lab.but1 -in $This.frauts.lab -anchor center -fill both -side left -expand true
            button $This.frauts.lab.but2 -borderwidth 2 -font $audace(font,arial_10_b) -state disabled
            pack $This.frauts.lab.but2 -in $This.frauts.lab -anchor center -fill both -side left -expand true
            label $This.frauts.lab.but2.labURL1 -borderwidth 2 -text "$caption(cmanimate,ut1)" \
               -font $audace(font,arial_10_b) -fg $color(red)
            pack $This.frauts.lab.but2.labURL1 -in $This.frauts.lab.but2 -anchor center -fill both -side left -expand true
            label $This.frauts.lab.but2.labURL2 -borderwidth 2 -text "$caption(cmanimate,ut2)" \
               -font $audace(font,arial_10_b) -fg $color(yellow)
            pack $This.frauts.lab.but2.labURL2 -in $This.frauts.lab.but2 -anchor center -fill both -side left -expand true
            label $This.frauts.lab.but2.labURL3 -borderwidth 2 -text "$caption(cmanimate,ut3)" \
               -font $audace(font,arial_10_b) -fg $color(green)
            pack $This.frauts.lab.but2.labURL3 -in $This.frauts.lab.but2 -anchor center -fill both -side left -expand true
            label $This.frauts.lab.but2.labURL4 -borderwidth 2 -text "$caption(cmanimate,ut4)" \
               -font $audace(font,arial_10_b) -fg $color(blue)
            pack $This.frauts.lab.but2.labURL4 -in $This.frauts.lab.but2 -anchor center -fill both -side left -expand true
            button $This.frauts.lab.but3 -borderwidth 2 -text "$caption(cmanimate,instrument)" \
               -font $audace(font,arial_10_b) -command { ::cmanim::Position_Tel }
            pack $This.frauts.lab.but3 -in $This.frauts.lab -anchor center -expand 1 -fill both -side left \
               -expand true
         pack   $This.frauts.lab -in $This.frauts -side top -fill none -padx 2 -pady 2 -ipadx 4 -ipady 2

      pack $This.frauts -side top -fill x

      #--- Frame for image infos
      frame $This.fra6 -borderwidth 2 -relief groove

         #--- Label for title
         label $This.fra6.lab1 -borderwidth 1 -text "$panneau(cmanim,label_ima)" -font $audace(font,arial_10_b)
         pack $This.fra6.lab1 -in $This.fra6 -anchor center -expand 1 -fill both -side top

         #--- Label for images name
         label  $This.fra6.labURL2 -text "$panneau(cmanim,status)" -font $audace(font,arial_7_b) -fg $color(red) -relief flat
         pack   $This.fra6.labURL2 -in $This.fra6 -anchor center -expand 1 -fill both -padx 4 -pady 1

      pack $This.fra6 -side top -fill x

      #--- Binding pour afficher le nom generique des images et le positionnement des instruments
      bind $This.fra2.ent1 <Key-Escape> { ::cmanim::edit_nom_image }
      catch {
         bind $This.frauts.lab.but2.labURL1 <ButtonPress-1> { ::cmanim::Position_Tel }
         bind $This.frauts.lab.but2.labURL2 <ButtonPress-1> { ::cmanim::Position_Tel }
         bind $This.frauts.lab.but2.labURL3 <ButtonPress-1> { ::cmanim::Position_Tel }
         bind $This.frauts.lab.but2.labURL4 <ButtonPress-1> { ::cmanim::Position_Tel }
      }

      #--- Mise a jour dynamique des couleurs
      ::confColor::applyColor $This
}

#================================
#=== Intialisation of pannel  ===
#================================
global audace

::cmanim::init $audace(base)

#=== End of file cmanimate.tcl ===

