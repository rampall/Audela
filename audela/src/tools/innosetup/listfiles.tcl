#!/usr/local/bin/tclsh8.4
#
# Fichier : listfiles.tcl
# Description : genere un fichier texte pour Inno setup
# Auteur : Alain KLOTZ
# Date de MAJ : 2 jiun 2006
#
# source [pwd]/../src/tools/innosetup/listfiles.tcl

set version 1.4.0

   proc analdir { base } {
      global tab result resultfile f base0
      set listfiles ""
      set a [catch {set listfiles [glob ${base}/*]} msg]
      if {$a==0} {
         # --- tri des fichiers dans l'ordre chrono decroissant
         set listdatefiles ""
         foreach thisfile $listfiles {
            set a [file isdirectory $thisfile]
            if {$a==0} {
               set datename [file mtime $thisfile]
               lappend listdatefiles [list $datename $thisfile]
            }
         }
         set listdatefiles [lsort -decreasing $listdatefiles]
         # --- affiche les fichiers
         foreach thisdatefile $listdatefiles {
            set thisfile [lindex $thisdatefile 1]
            set a [file isdirectory $thisfile]
            if {$a==0} {
               set shortname [file tail "$thisfile"]
               set dirname [file dirname "$thisfile"]
               set sizename [expr 1+int([file size "$thisfile"]/1000)]
               set datename [file mtime "$thisfile"]
               if {$datename==-1} {
                  set datename 0000-00-00T00:00:00
               } else {
                  set datename [clock format [file mtime $thisfile] -format %Y-%m-%dT%H:%M:%S ]
               }
               #for {set k 0} {$k<$tab} {incr k} {
               #   append result " "
               #}
               regsub -all / "$thisfile" \\ name1
               regsub ${base0} "$dirname" "" temp
               regsub -all / "$temp" \\ name2
               set name2 "{app}$name2"
               if {$shortname=="PortTalk.sys"} {
                  append result "Source: \"$name1\"; DestDir: \"$name2\"; \n"
               }
               set extension [file extension "$thisfile"]
               if {($extension==".vxd")||($extension==".VXD")} {
                  set name2 "{sys}"
               }
               if {($extension==".sys")} {
                  set name2 "{sys}\\drivers"
               }
               append result "Source: \"$name1\"; DestDir: \"$name2\"; \n"
               #set a [string length $shortname]
               #set a [expr 20-$a]
               #if {$a<0} {
               #   set a 0
               #}
               #for {set k 0} {$k<$a} {incr k} {
               #   append result " "
               #}
               #append result "$datename ($sizename Ko)\n"
            }
         }
         set f [open $resultfile a]
         puts -nonewline $f "$result"
         close $f
         set result ""
         foreach thisfile $listfiles {
            set a [file isdirectory $thisfile]
            if {$a==1} {
               incr tab 1
               set shortname [file tail $thisfile]
               set datename [file mtime $thisfile]
               if {$datename==-1} {
                  set datename 0000-00-00T00:00:00
               } else {
                  set datename [clock format [file mtime $thisfile] -format %Y-%m-%dT%H:%M:%S ]
               }
               #append result "\n"
               #for {set k 0} {$k<$tab} {incr k} {
               #   append result " "
               #}
               #append result "Directory of $thisfile\n"
				#analdir $thisfile
				if {([file tail $thisfile] != "CVS") && ([file tail $thisfile] != "src") && ([file tail $thisfile] != "dev")} {
					analdir $thisfile
				}
               #incr tab -1
            }
         }
         #set f [open $resultfile a]
         #puts -nonewline $f "$result"
         #close $f
         #set result ""
      }
   }

   #::console::affiche_resultat "[glob [file dirname [info script]]/../../../* ]\n"
      #set base c:/d/audela-${version}/audela
      set base "[file dirname [info script]]/../../../"
      global tab result resultfile f base0
      set base0 "$base"
      set tab 0
      if {$base=="."} {
         set base [pwd]
      }
      file delete -force "${base}/bin/audela.log"
      file delete _force "${base}/bin/audace.txt"
      set resultfile "${base}/src/tools/innosetup/audela-${version}.iss"

set result    "; Script generated by the Inno Setup Script Wizard.\n"
append result "; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!\n"
append result "\n"
append result "\[Setup\]\n"
append result "AppName=AudeLA\n"
append result "AppVerName=Audela-${version}\n"
append result "AppPublisher=My Company, Inc.\n"
append result "AppPublisherURL=http://software.audela.free.fr\n"
append result "AppSupportURL=http://software.audela.free.fr\n"
append result "AppUpdatesURL=http://software.audela.free.fr\n"
append result "DefaultDirName=C:\\audela-${version}\n"
append result "DefaultGroupName=Audela\n"
append result "LicenseFile=licence.txt\n"
append result "InfoBeforeFile=before.txt\n"
append result "InfoAfterFile=after.txt\n"
append result "; uncomment the following line if you want your installation to run on NT 3.51 too.\n"
append result "; MinVersion=4,3.51\n"
append result "\n"
append result "\[Tasks\]\n"
append result "Name: \"desktopicon\"; Description: \"Create a &desktop icon\"; GroupDescription: \"Additional icons:\"; MinVersion: 4,4\n"
append result "\n"
append result "\[Files\]\n"

      set f [open $resultfile w]
      puts -nonewline $f "$result"
      close $f
      set result ""
      analdir $base

set result    "\n"
append result "\[Icons\]\n"
append result "Name: \"{group}\\audela\"; Filename: \"{app}\\bin\\audela.exe\" ; WorkingDir: \"{app}\\bin\" \n"
append result "Name: \"{userdesktop}\\audela\"; Filename: \"{app}\\bin\\audela.exe\" ; WorkingDir: \"{app}\\bin\" ; MinVersion: 4,4; Tasks: desktopicon\n"
append result "\n"
append result "\[Run\]\n"
append result "Filename: \"{app}\\bin\\audela.exe\"; WorkingDir: \"{app}\\bin\" ; Description: \"Launch audela\"; Flags: nowait postinstall skipifsilent\n"

      set f [open $resultfile a]
      puts -nonewline $f "$result"
      close $f
