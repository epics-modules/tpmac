#! /usr/bin/wish -f
#! wish82.exe -f

#-----------------------------------------------------------------------------------------
#  This is a GUI frontend to BURT software aimed at saving/resotring motors calibrations
#
#    Original version: Thomas Coleman, ANL
#  Current maintainer: Sergey Stepanov, ANL
#
#  Version 1.1: added grouped save/restore to/from a set of files
#  Version 2.0: added file dates on the buttons.
#-----------------------------------------------------------------------------------------
proc pDoitCoord {coordname} {
#   puts stdout "pDoitCoord: coordname=${coordname}"
    flush stdout
    global sCmd burtRestore burtSave burtDIR requestDIR Beamline
    set operand ${Beamline}_${coordname}
    switch -exact ${sCmd} {
       Quit {
          exit
       }
       Restore {
# This replaces all ":" by "_" to make the files PC-friendly:
          regsub -all : ${operand} _ snapFile
#	  puts "${burtRestore} -f ${burtDIR}${snapFile}.snap"
#	  exec  ${burtRestore} -f ${burtDIR}${snapFile}.snap
          if { [ catch {exec  ${burtRestore} -f ${burtDIR}${snapFile}.snap} err ] } {
# Ignore styrings like: "error waiting for process to exit: child process lost (is SIGCHLD ignored or trapped?)"
	     if { [ string match *SIGCHLD* $err ] == 0 } {
	        puts "exec: '$err'"
	        tk_messageBox -message $err -type ok -icon error -title "Application Error"
	     }
	  }
       }
       Save {
# This replaces all ":" by "_" to make the files PC-friendly:
          regsub -all : ${operand} _ snapFile
          regsub  ${Beamline}_ ${snapFile} {} requestFile
          set snapFile    ${burtDIR}${snapFile}.snap
          set requestFile ${requestDIR}${requestFile}.req
#         puts "${burtSave} -f ${requestFile} -o ${snapFile} -DBEAMLINE=${Beamline}"
#         exec  ${burtSave} -f ${requestFile} -o ${snapFile} -DBEAMLINE=${Beamline}
	  if { [ catch {exec ${burtSave} -f ${requestFile} -o ${snapFile} -DBEAMLINE=${Beamline}} err ] } {
# Ignore styrings like: "error waiting for process to exit: child process lost (is SIGCHLD ignored or trapped?)"
             if { [ string match *SIGCHLD* $err ] == 0 } {
                puts "exec: '$err'"
                tk_messageBox -message $err -type ok -icon error -title "Application Error"
             }
          }
          if {[file exists $snapFile]} {
             file stat $snapFile snapStat
             set datestamp [clock format $snapStat(mtime) -format "%Y/%m/%d"]
          } else {
             set datestamp "       No file"
          }
	  set lbl ${Beamline}:${coordname}
	  set len [string length $lbl]
	  for {set i $len} {$i <= 16} {incr i} {set lbl "${lbl} ";}
          .bFunc$coordname configure -text "${lbl} ${datestamp}"
       }
    }
}

#-----------------------------------------------------------------------------------------
proc pDoitPmac {coordlist} {
#   puts stdout "pDoitPmac: coordlist=${coordlist}"
    flush stdout
    foreach coord $coordlist {
        set coordname [lindex $coord 1]
        pDoitCoord $coordname
    }
}

#-----------------------------------------------------------------------------------------
proc pDoitIoc {pmaclist} {
#   puts stdout "pDoitIoc: pmaclist=${pmaclist}"
    flush stdout
    foreach pmac $pmaclist {
        set coordlist [lindex $pmac 1]
        pDoitPmac $coordlist
    }
}

#-----------------------------------------------------------------------------------------
proc pError {text} {
	puts stderr "${text}"
        flush stderr
	exit
}

#----------------------------------------------------------------------------------------
# main:

if { [ file exists /etc/passwd ] } {
    set SYSTEM LINUX
    set tclGMCA  /home/gmca/epics/gmcaApp/tcl/
} else {
    set SYSTEM WIN32
    set tclGMCA  {c:\\gmca\\gmcaApp\\tcl\\}
}

### Password protection for *ANY* account.
### Added by Sergey 2006/09/19.
catch {exec ${tclGMCA}/authorize.tcl } txt
if { $txt != "0" } {
  pError "Authorization failed (${txt})"
}

if { $argc != 0 && $argc != 1 }  {
    set txt "CalibSave: Incorrect number of command line argumentts. The command line must specify beamline name as one of: 23i, 23o, 23b, 23d.\n"
    message .msg -width 10c -justify left -text "${txt}"
    button  .ok  -text "OK" -command "pError {${txt}}"
    pack .msg
    pack .ok
} else {
    if { $argc != 0 } {
       set Beamline [lindex $argv 0]
    } else {
       puts stderr "CalibSave: no beamline specified. Using the development environment 23d !"
       set Beamline "23d"
    }
    if { $Beamline != "23i"  &&  $Beamline != "23o"  &&  $Beamline != "23b"  &&  $Beamline != "23d" } {
	set txt "CalibSave: Unknown beamline ${Beamline} in the command line -- must be one of the following: 23i, 23o, 23b, 23d.\n"
	message .msg -width 10c -justify left -text "${txt}"
	button  .ok  -text "OK" -command "pError {${txt}}"
	pack .msg
	pack .ok
    } else {
	set bln ${Beamline}:
	source ${tclGMCA}config.tcl
	source ${tclGMCA}config${Beamline}.tcl
	set burtDIR    ${burtGMCA}${Beamline}${sp}
	set requestDIR ${burtGMCA}request${sp}

	frame .fRack

# 1. Loop over VME crates
	foreach ioc $IocPmacList {
	    set iocname [lindex $ioc 0]
	    set pmaclist [lindex $ioc 1]
	    frame .fIoc$iocname -relief ridge -borderwidth 2
	    button .bFunc$iocname -text $bln$iocname -width 9             \
	                   -background pink -foreground black             \
	    		   -activebackground red -activeforeground yellow \
 	    		   -command "pDoitIoc [list $pmaclist]"
	    pack .bFunc$iocname -in .fIoc$iocname -ipadx 8 -side left -fill y
# 2. Loop over PMAC cards
	    foreach pmac $pmaclist {
	    	set pmacname [lindex $pmac 0]
	    	set coordlist [lindex $pmac 1]
	    	frame .fPmac$pmacname -relief flat
	    	button .bFunc$pmacname -text $bln$pmacname -width 11      \
	                   -background pink -foreground black             \
			   -activebackground red -activeforeground yellow \
 			   -command "pDoitPmac [list $coordlist]"
	    	pack .bFunc$pmacname -in .fPmac$pmacname -side top
# 3. Loop over coordinate systems in a PMAC:
	    	foreach coord $coordlist {
		    set coordname [lindex $coord 1]
		    set snapFile ${burtDIR}${bln}${coordname}.snap
	            regsub -all : ${snapFile} _ snapFile
                    if {[file exists $snapFile]} {
		      file stat $snapFile snapStat
		      set datestamp [clock format $snapStat(mtime) -format "%Y/%m/%d"]
		    } else {
		      set datestamp "       No file"
		    }
		    set lbl ${bln}${coordname}
		    set len [string length $lbl]
		    for {set i $len} {$i <= 16} {incr i} {set lbl "${lbl} ";}
		    button .bFunc$coordname -text "${lbl} ${datestamp}" -command "pDoitCoord $coordname"
		    pack .bFunc$coordname -in .fPmac$pmacname -side top -fill x
	    	}
# 3. End of loop over coordinate systems in a PMAC
	    	pack .fPmac$pmacname -in .fIoc$iocname -side left -padx 2 -pady 2 -anchor n
	    }
# 2. End of loop over PMAC cards
  	    pack .fIoc$iocname -in .fRack -side left -fill x -fill y -expand 1 -anchor nw
    	}
# 1. End of loop over VME crates

	frame .fCmds -relief sunken -borderwidth 2
	if { ${SYSTEM} != "WIN32" } {
	    radiobutton .rbCmdQui -text "Quit"    -variable sCmd -value "Quit"    -relief flat
	    radiobutton .rbCmdRes -text "Restore" -variable sCmd -value "Restore" -relief flat
	    radiobutton .rbCmdSav -text "Save"    -variable sCmd -value "Save"    -relief flat
	    pack .rbCmdQui .rbCmdRes .rbCmdSav -in .fCmds -side top -anchor w
	} else {
	    radiobutton .rbCmdQui -text "Quit"    -variable sCmd -value "Quit"    -relief flat
	    label       .lbCmdRes -text "        Restore"
	    label       .lbCmdSav -text "        Save"
	    pack .rbCmdQui .lbCmdRes .lbCmdSav -in .fCmds -side top -anchor w
	}

  	frame .fHdrs -borderwidth 2                    -background #FF0000
  	label	.lbBeamline -text "Beamline=${Beamline}" -background #FF0000 -foreground #FFFFFF
  	label	.lbDir	    -text "SaveDir=${burtDIR}"   -background #FF0000 -foreground #FFFFFF
  	pack .lbBeamline -in .fHdrs -side left  -anchor ne
  	pack .lbDir      -in .fHdrs -side right -anchor nw
  	pack .fHdrs -side top  -padx 0 -pady 0 -fill x

	pack .fCmds .fRack -side left -padx 4 -pady 4

	set sCmd "Quit"
    }
}
