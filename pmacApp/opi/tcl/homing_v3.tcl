#! /usr/bin/wish -f
#! wish.exe -f
#-----------------------------------------------------------------------------------------
# This is a frontend to homing script. It must be called with the "beamline name" argument;
# otherwise 23d is used by default.
#
#                                 Author: Sergey Stepanov
# Version-1    /2006.12/
# Version-2    /2007.06/ added loading nominal positions from snapshot file.
# Version-3    /2008.01/ added button to mark not homed directories
# Version-3.1  /2008.02/ added using caTCL functions instead of caget (if caTCL exists)
#-----------------------------------------------------------------------------------------
  set CA_EXIST 0
  if { [info exists env(EPICS_HOST_ARCH)] } {
     set arch $env(EPICS_HOST_ARCH)
     if     {$arch == "win32-x86"} {set CA_EXTENSIONS "c:/epics/extensions/bin/${arch}";} \
     elseif {$arch == "linux-x86"} {set CA_EXTENSIONS "/usr/local/epics/extensions/lib/${arch}";} \
     else                          {set CA_EXTENSIONS "";}
#    puts stdout "CA_EXTENSIONS = ${CA_EXTENSIONS}"
     if {[file exists "${CA_EXTENSIONS}/pkgIndex.tcl"]} {set CA_EXIST 1;}
  }
  if {$CA_EXIST} {
     set auto_path [linsert $auto_path 0 $CA_EXTENSIONS]
     package require ca
}

#-----------------------------------------------------------------------------------------
proc pDoitPmac {pmacname coordlist} {
#   puts stdout "pDoitPmac: coordlist=${coordlist}"
    flush stdout
    global var$pmacname
    foreach coord $coordlist {
        set coordname [lindex $coord 1]
        global var$coordname
        set var$coordname [set var$pmacname]
    }
}

#-----------------------------------------------------------------------------------------
proc pDoitIoc {iocname pmaclist} {
#   puts stdout "pDoitIoc: pmaclist=${pmaclist}"
    flush stdout
    global var$iocname
    foreach pmac $pmaclist {
	set pmacname [lindex $pmac 0]
        global var$pmacname
	set var$pmacname [set var$iocname]
        set coordlist [lindex $pmac 1]
        pDoitPmac $pmacname $coordlist
    }
}

#-----------------------------------------------------------------------------------------
proc pDoitHome {} {
    global Beamline xterm homeScript varOpt varMedmHome varMedmMove IocPmacList nominalFile
#   puts stdout "pDoitHome: pmaclist=${pmaclist}"
    flush stdout

    switch -exact ${varOpt} {
	Sequential {
	set lhome []
# 1. Loop over VME crates
	    foreach ioc $IocPmacList {
	        set iocname [lindex $ioc 0]
                global var$iocname
	        set var$iocname 0
	        set pmaclist [lindex $ioc 1]
# 2. Loop over PMAC cards
                foreach pmac $pmaclist {
	            set pmacname [lindex $pmac 0]
                    global var$pmacname
	            set var$pmacname 0
                    set coordlist [lindex $pmac 1]
# 3. Loop over coordinate systems in a PMAC:
                    foreach coord $coordlist {
                        set coordname [lindex $coord 1]
                        global var$coordname
                        if { [set var$coordname] } {
                            lappend lhome $coordname
                            .cbFunc$coordname config -foreground #007700
                            set var$coordname 0
			}
                    }
                }
            }
            if { $lhome != "" } {
                set CmdString "${lhome} nominal=${nominalFile}"
                if {$varMedmHome == 0} {set CmdString "${CmdString} noMedmHome"}
                if {$varMedmMove == 0} {set CmdString "${CmdString} noMedmMove"}
#               puts "${homeScript} ${Beamline} ${CmdString}"
                eval "exec $xterm perl ${homeScript} ${Beamline} ${CmdString} &"
            }
        }
	Parallel {
# 1. Loop over VME crates
	    foreach ioc $IocPmacList {
	        set iocname [lindex $ioc 0]
                global var$iocname
	        set var$iocname 0
	        set pmaclist [lindex $ioc 1]
# 2. Loop over PMAC cards
                foreach pmac $pmaclist {
	            set pmacname [lindex $pmac 0]
                    global var$pmacname
	            set var$pmacname 0
                    set coordlist [lindex $pmac 1]
# 3. Loop over coordinate systems in a PMAC:
                    set i 0
                    foreach coord $coordlist {
                        set coordname [lindex $coord 1]
                        global var$coordname
                        if { [set var$coordname] } {
                            incr i
                            set CmdString "${coordname} nominal=${nominalFile}"
                            if {$varMedmHome == 0} {set CmdString "${CmdString} noMedmHome"}
                            if {$varMedmMove == 0} {set CmdString "${CmdString} noMedmMove"}
                            if {$i == 1} {
                              set CmdString "${CmdString} autoMacroReset"
                            } else {
                              set CmdString "${CmdString} noUnlink32 waitMacroReset"
                            }
#                           puts "${homeScript} ${Beamline} ${CmdString}"
                            eval "exec ${xterm} perl ${homeScript} ${Beamline} ${CmdString} &"
                            .cbFunc$coordname config -foreground #007700
                            set var$coordname 0
			}
                    }
                }
            }
	}
    }
}

#-----------------------------------------------------------------------------------------
proc pMarkNotHomed {} {
    global Beamline IocPmacList CA_EXIST epics_var errorCode
#   puts stdout "pMarkNotHomed: pmaclist=${pmaclist}"
    flush stdout

# 1. Loop over VME crates
    foreach ioc $IocPmacList {
       set pmaclist [lindex $ioc 1]
# 2. Loop over PMAC cards
       foreach pmac $pmaclist {
          set coordlist [lindex $pmac 1]
# 3. Loop over coordinate systems in a PMAC:
          foreach coord $coordlist {
             set coordname [lindex $coord 1]
# Get homed status from EPICS. On error presume that assy is homed:
             if {$CA_EXIST} {
 	        set timeout 0.5
		set output "NotHomed"
                if {[pv linkw epics_var ${Beamline}:${coordname}Homed $timeout] != 0} {
                   puts stderr "Error connecting to ${Beamline}:${coordname}Homed, code=${errorCode}"
                   flush stderr
		} else {
                   if {[pv getw epics_var $timeout] != 0} {
                      puts stderr "Error reading ${Beamline}:${coordname}Homed, code=${errorCode}"
                      flush stderr
		   } else {
                      set output $epics_var
		   }
		}
            	pv unlink epics_var
             } else {
                catch {exec caget -t -w 0.2 ${Beamline}:${coordname}Homed} output
	     }
#            puts "pMarkNotHomed: ${Beamline}:${coordname}Homed -> $output"
	     if {[regexp {^NotHomed} $output] == 1} {set homedStatus 0;} \
             else {set homedStatus 1;}
	     if {$homedStatus == 0} {.cbFunc$coordname config -foreground #ff0000;}
          }
       }
    }
}

#-----------------------------------------------------------------------------------------
proc pClearHomed {} {
    global IocPmacList

# 1. Loop over VME crates
    foreach ioc $IocPmacList {
       set pmaclist [lindex $ioc 1]
# 2. Loop over PMAC cards
       foreach pmac $pmaclist {
          set coordlist [lindex $pmac 1]
# 3. Loop over coordinate systems in a PMAC:
          foreach coord $coordlist {
             set coordname [lindex $coord 1]
             .cbFunc$coordname config -foreground #000000
          }
       }
    }
}

#-----------------------------------------------------------------------------------------
proc pDoitFile {} {
    global nominalFile nominalDir
    set newSnap [tk_getOpenFile -title {Nominal Positions: Open SNAP File} \
	           -initialdir ${nominalDir} -filetypes {{"SNAP files" {.snap}} {"ALL files" {*}}}]
    if { $newSnap != "" && $newSnap != $nominalFile } {
      set nominalFile $newSnap
      .lbNominal config -text "Nominal=${nominalFile}"
    }
}

#-----------------------------------------------------------------------------------------
proc pError {text} {
  puts stderr "${text}"
  flush stderr
  exit
}

#-----------------------------------------------------------------------------------------
proc chk_exit {} {
  set ans [tk_messageBox -type yesno -icon question -message "Are you sure you want to quit?"]
  if {$ans != "no"} {
     exit
  }
};				# chk_exit


#----------------------------------------------------------------------------------------
### main:
# proc main {Beamline} {

   if { $argc != 0 &&  $argc != 1 }  {
      set txt "HomeMenu: Incorrect number of command line arguments. The command line must specify beamline name as one of: 23i, 23o, 23b, 23d.\n"
#     message .msg -width 10c -justify left -text "${txt}"
#     button  .ok  -text "OK" -command "pError {${txt}}"
#     pack .msg
#     pack .ok
      tk_messageBox -message $txt -type ok -icon error -title "HomeMenu Error"
      pError $txt
   }

   if { $argc != 0 } {
      set Beamline [lindex $argv 0]
   } else {
      puts stderr "HomeMenu: no beamline specified. Using the development environment 23d !"
      set Beamline "23d"
   }

   if { $Beamline != "23i"  &&  $Beamline != "23o"  &&  $Beamline != "23b"  &&  $Beamline != "23d" } {
      set txt "HomeMenu: Unknown beamline ${Beamline} in the command line -- must be one of the following: 23i, 23o, 23b, 23d.\n"
#     message .msg -width 10c -justify left -text "${txt}"
#     button  .ok  -text "OK" -command "pError {${txt}}"
#     pack .msg
#     pack .ok
      tk_messageBox -message $txt -type ok -icon error -title "HomeMenu Error"
      pError $txt
   }

   if { [ file exists /etc/passwd ] } {
      set SYSTEM LINUX
      set sep /
   } else {
      set SYSTEM WIN32
      set sep \\
   }

   if { [ info exists env(FASTSCANS) ] } {
      set Fastscans $env(FASTSCANS)
   } else {
      if ( ${SYSTEM} == "LINUX" ) {
	 set Fastscans ${sep}home${sp}gmca${sep}epics${sep}fastscans
      } else {
	 set Fastscans c:${sep}gmca${sep}fastscans
      }
   }
### Produce tclGMCA dir name from Fastscans:
   set tclGMCA [regsub fastscans ${Fastscans} gmcaApp${sep}tcl${sep}]

#  set bln ${Beamline}:
   source ${tclGMCA}config.tcl
   source ${tclGMCA}config${Beamline}.tcl

### Produce home script name from Fastscans:
   set homeScript [regsub fastscans ${Fastscans} ${sep}pezca${sep}home23_v2.pl]
### Default data file with nominal positions:
   set nominalDir ${Fastscans}${sep}data
   set nominalFile ${nominalDir}${sep}nominal_${Beamline}.snap

### Sergey: intercept killing window:
   wm protocol . WM_DELETE_WINDOW chk_exit

   frame .fRack

# 1. Loop over VME crates
   foreach ioc $IocPmacList {
      set iocname [lindex $ioc 0]
      set pmaclist [lindex $ioc 1]
      frame .fIoc$iocname -relief ridge -borderwidth 2
#     checkbutton .cbFunc$iocname -text $bln$iocname -width 9
      checkbutton .cbFunc$iocname -text $iocname                       \
                     -background       #FFC0CE -foreground       black \
                     -activebackground #FFA0AE -activeforeground black \
                     -variable var$iocname                             \
                     -command "pDoitIoc $iocname [list $pmaclist]"
      pack .cbFunc$iocname -in .fIoc$iocname -ipadx 2 -ipady 0 -side left -fill y
# 2. Loop over PMAC cards
      foreach pmac $pmaclist {
	 set pmacname [lindex $pmac 0]
	 set coordlist [lindex $pmac 1]
	 frame .fPmac$pmacname -relief flat
#	 checkbutton .cbFunc$pmacname -text $bln$pmacname -width 10
	 checkbutton .cbFunc$pmacname -text "$pmacname    " -justify left  \
	                 -background       #FFC0CE -foreground       black \
	                 -activebackground #FFA0AE -activeforeground black \
                         -variable var$pmacname                            \
 			 -command "pDoitPmac $pmacname [list $coordlist]"
	 pack .cbFunc$pmacname -in .fPmac$pmacname -ipadx 2 -ipady 0 -side top -anchor nw -fill y
# 3. Loop over coordinate systems in a PMAC:
	 foreach coord $coordlist {
	    set coordname [lindex $coord 1]
#	    checkbutton .cbFunc$coordname -text $bln$coordname
	    checkbutton .cbFunc$coordname -text $coordname \
	                    -foreground       black        \
			    -activeforeground red          \
                            -variable var$coordname
	    pack .cbFunc$coordname -in .fPmac$pmacname -ipadx 0 -ipady 0 -side top -anchor nw -fill y
	 }
# 3. End of loop over coordinate systems in a PMAC
	 pack .fPmac$pmacname -in .fIoc$iocname -side left -padx 2 -pady 0 -anchor nw -fill y
      }
# 2. End of loop over PMAC cards
      pack .fIoc$iocname -in .fRack -side left -fill x -fill y -expand 1 -anchor nw
   }
# 1. End of loop over VME crates

   frame .fHome -borderwidth 2
   frame .fOpts -relief sunken -borderwidth 2
   label       .lbOptHead -text "Homing mode:" -underline 7
   radiobutton .rbOptSeq  -text "Sequential" -variable varOpt -value "Sequential" -relief flat
   radiobutton .rbOptPrl  -text "Parallel"   -variable varOpt -value "Parallel"   -relief flat
   pack .lbOptHead .rbOptSeq .rbOptPrl -in .fOpts -side top -anchor w
   frame .fMedm -borderwidth 2
   checkbutton .cbOptMedmHome -text "Show Motor MEDM" -underline 0 -variable varMedmHome
   checkbutton .cbOptMedmMove -text "Show Assy  MEDM" -underline 0 -variable varMedmMove
   pack .cbOptMedmHome .cbOptMedmMove -in .fMedm -side top -anchor w
   button        .bFuncHome -text "HOME" -underline 0 -width 16     \
                 -background       #EEEE00 -foreground       black \
                 -activebackground #FFFF00 -activeforeground black \
                 -command "pDoitHome"
   button        .bFuncMark -text "Mark not homed" -underline 0 -width 16     \
                 -background       #EE5555 -foreground       black \
                 -activebackground #FF7777 -activeforeground black \
                 -command "pMarkNotHomed"
   button        .bFuncClear -text "Clear homed" -underline 0 -width 16     \
                 -background       #999999 -foreground       black \
                 -activebackground #AAAAAA -activeforeground black \
                 -command "pClearHomed"
   pack .fOpts .fMedm .bFuncHome .bFuncMark .bFuncClear -in .fHome -side top -padx 5 -pady 5

   frame .fHdrs -borderwidth 2                    -background #FF0000
   label .lbBeamline -text "Beamline=${Beamline}" -background #FF0000 -foreground white
   pack .lbBeamline -in .fHdrs -side left  -anchor ne
   pack .fHdrs -side top  -padx 0 -pady 0 -fill x

   frame .fNmnl -borderwidth 2                     -background #5959E5
   label .lbNominal -text "Nominal=${nominalFile}" -background #5959E5 -foreground white
   button        .bFuncFile -text "Change" -underline 0 -width 9     \
                 -background       #00C4FF -foreground       #000000 \
                 -activebackground #00A4FF -activeforeground #000000 \
                 -command "pDoitFile"
   pack .lbNominal -in .fNmnl -side left  -anchor w
   pack .bFuncFile -in .fNmnl -side right -anchor e

   pack .fNmnl -side bottom -padx 0 -pady 0 -fill x

   pack .fHome .fRack -side left -padx 4 -pady 4


   set varOpt  "Sequential"
   set varMedmHome 1
   set varMedmMove 1
   pMarkNotHomed
#}						# End of main proc.
