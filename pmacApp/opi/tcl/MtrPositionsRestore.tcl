#! /usr/bin/wish -f
#! wish82.exe -f

#-----------------------------------------------------------------------------------------
proc pReadSnap {snapFile} {

   global dateStamp user Beamline
   set restoreList {}

#  puts "Opening ${snapFile}"
   set SNAP [open $snapFile r]

   set line 1
   if { [gets $SNAP fileString] < 0 || [regexp {^\# This file was created on } $fileString] == 0 } {
       set txt "*** mtrPositionsRestoreMenu: \n"
       set txt "${txt}Snap file \[${snapFile}\] has incorrect date stamp field.\n"
       tk_messageBox -message $txt -type ok -icon error -title "mtrPositionsRestoreMenu Error"
       pError $txt
   }
   regsub {^\# This file was created on } $fileString {} dateStamp

   incr line
   if { [gets $SNAP fileString] < 0 || [regexp {^\# Producer=} $fileString] == 0 } {
       set txt "*** mtrPositionsRestoreMenu: \n"
       set txt "${txt}Snap file \[${snapFile}\] has incorrect producer field. \n"
       tk_messageBox -message $txt -type ok -icon error -title "mtrPositionsRestoreMenu Error"
       pError $txt
   }

   incr line
   if { [gets $SNAP fileString] < 0 || [regexp {^\# User=} $fileString] == 0 } {
       set txt "*** mtrPositionsRestoreMenu: \n"
       set txt "${txt}Snap file \[${snapFile}\] has incorrect user field. \n"
       tk_messageBox -message $txt -type ok -icon error -title "mtrPositionsRestoreMenu Error"
       pError $txt
   }
   regsub {^\# User=} $fileString {} user

   while { [gets $SNAP fileString] >= 0 } {
      incr line
      regsub -all {^\#.*$} $fileString {}  fileString
      regsub -all {\t}     $fileString { } fileString
      regsub -all { +}     $fileString { } fileString
      regsub -all {^ *}    $fileString {}  fileString
      regsub -all { *$}    $fileString {}  fileString
      if { [string length $fileString] > 0 } {
         set assyList [split $fileString { }]
         set na [llength $assyList]
         incr na -2
         set nAxis [lindex $assyList 0]
         if { $na != [expr 2*$nAxis] } {
            close $SNAP
            set na [expr $na/2.]
            set txt "*** mtrPositionsRestoreMenu: \n"
            set txt "${txt}Snap file \[${snapFile}\] \n"
            set txt "${txt}has incorrect record at line=${line}. \n"
            set txt "${txt}Expected nAxis=${nAxis}, found=${na} \n"
            tk_messageBox -message $txt -type ok -icon error -title "mtrPositionsRestoreMenu Error"
            pError $txt
         }
         set assy [lindex $assyList 1]
#        puts "nAxis=${nAxis}  assy=${assy}"
### Check if the SNAP belongs
### to requested beamline:
         if { [regexp "^${Beamline}:" $assy] == 0 } {
            set bln_ [scan $assy %3s]
            set txt "*** mtrPositionsRestoreMenu: \n"
            set txt "${txt}the SNAP file belongs to beamline ${bln_}, not to ${Beamline}\n"
            tk_messageBox -message $txt -type ok -icon error -title "mtrPositionsRestoreMenu Error"
	    break
	 }
### Skip beamline extras: generic XY positioners and small Huber slits:
### (this is to reduce the list length; in the future we may need scrolling)
#        if { [regexp "^${Beamline}:XY:P" $assy] == 0 && \
#             [regexp "^${Beamline}:HS:A" $assy] == 0 } {
            set assyParsed [concat $nAxis $assy]
            for {set i 0} {$i< $nAxis} {incr i} {
               set ia [expr 2+$i]
               set ip [expr 2+$nAxis+$i]
               set axis [concat [lindex $assyList $ia] [lindex $assyList $ip]]
               lappend assyParsed $axis
            }
#           puts $assyParsed
            lappend restoreList $assyParsed
#	 }
      }
   }
#  puts $restoreList
   close $SNAP
   return $restoreList
}

#-----------------------------------------------------------------------------------------

proc pDrawRack {} {

   global restoreList

### the .fRack frame holds the canvas and scrollbar
   frame .fRack -relief sunken -borderwidth 2
   canvas .fRack.canvas -yscrollcommand {.fRack.yscroll set} -bg LightYellow2
   scrollbar .fRack.yscroll -command {.fRack.canvas yview} -orient vertical

### pack the frame and its canvas and scrollbar
   pack .fRack.yscroll -side right -fill y
   pack .fRack.canvas -side left -fill both -expand true
   pack .fRack -side left -padx 4 -pady 4 -anchor nw -expand true -fill both

### .fRack.f holds the checkboxes and is displayed by the canvas .fRack.canvas
   frame .fRack.f -bd 0 -bg LightYellow2
   .fRack.canvas create window 0 0 -anchor nw -window .fRack.f

### Loop over assemblies (coordinate systems)
   foreach restore $restoreList {
      set nAxis [lindex $restore 0]
      set assy  [lindex $restore 1]
      set txt $assy
      for {set i 0} {$i< $nAxis} {incr i} {
      	 set axis [lindex $restore [expr 2+$i]]
	 set axisName [lindex $axis 0]
	 set axisDest [format "%-.3f" [lindex $axis 1]]
         set txt "${txt}      ${axisName}=${axisDest}"
      }
      global var$assy
      destroy .fRack.f..cbFunc$assy
      checkbutton .fRack.f.cbFunc$assy -text $txt          \
	  -foreground       black \
	  -activeforeground red   \
	  -bg LightYellow2 \
	  -variable var$assy
      grid .fRack.f.cbFunc$assy -sticky w
   }
### End of loop over coordinate systems

### tkwait is needed so the bbox calculation will be correct
   if {[llength $restoreList] > 0} {
     set firstAssy [lindex [lindex $restoreList 0] 1]
     set child .fRack.f.cbFunc$firstAssy
     tkwait visibility $child
   }

### Make the canvas just big enough to hold the checkboxes
   set bbox [grid bbox .fRack.f 0 0]
   set incr [lindex $bbox 3]
   set width [winfo reqwidth .fRack.f]
   set height [winfo reqheight .fRack.f]
   .fRack.canvas config -scrollregion "0 0 $width $height"
   .fRack.canvas config -yscrollincrement $incr
   set max [llength $restoreList]
   set maxScroll 35
   if {$max > $maxScroll} {
       set max $maxScroll
   }

   set height [expr $incr * $max]
   .fRack.canvas config -width $width -height $height
}

#-----------------------------------------------------------------------------------------
proc pRestore {} {

   global Beamline xterm restoreScript restoreList

   set CmdString {}
### Loop over coordinate systems:
   foreach restore $restoreList {
      set assy [lindex $restore 1]
      global var$assy
      if { [set var$assy] } {
         lappend CmdString [join $restore]
         .fRack.f..cbFunc$assy config -foreground #007700
         set var$assy 0
      }
   }
   if { [llength $CmdString] > 0 } {
      set Cmd [join $CmdString {" "}]
#     puts "exec $xterm perl ${restoreScript} ${Beamline} \"${Cmd}\" &"
      eval "exec $xterm perl ${restoreScript} ${Beamline} \"${Cmd}\" &"
   }
}

#-----------------------------------------------------------------------------------------
proc pSelect {action} {

   global restoreList

### action: 0=unselect 1=select
   if { $action != 0 } {
      set action 1
   }
### Loop over coordinate systems:
   foreach restore $restoreList {
      set assy [lindex $restore 1]
      global var$assy
      set var$assy $action
   }
}

#-----------------------------------------------------------------------------------------
proc pChangeSnap {} {

   global snapdir snapFile restoreList dateStamp user

   set newSnap [tk_getOpenFile -title {Restore Positions: Open SNAP File} \
	             -initialdir ${snapdir} -filetypes {{SNAP {.snap}} {ALL {*}}}]
   if { $newSnap != "" && $newSnap != $snapFile } {
      set snapFile $newSnap
      set restoreList [pReadSnap $snapFile]
      destroy .fRack
      .lbSnap      config -text $snapFile
      .lbLabelDate config -text "Date = ${dateStamp}"
      .lbLabelUser config -text "User = ${user}"
      pDrawRack
      pSelect 1
   }
}

#-----------------------------------------------------------------------------------------
proc pError {text} {
   puts stderr "${text}"
   flush stderr
   exit
}

#----------------------------------------------------------------------------------------
### main:
# proc main {Beamline} {

   global dateStamp user restoreList

   if { $argc != 0 &&  $argc != 1 }  {
      set txt "*** mtrPositionsRestoreMenu: \n"
      set txt "${txt}Incorrect number of command line arguments. \n"
      set txt "${txt}The command line must specify beamline name as one of: \n"
      set txt "${txt}23i, 23o, 23b, 23d.\n"
      tk_messageBox -message $txt -type ok -icon error -title "mtrPositionsRestoreMenu Error"
      pError $txt
   }

   if { $argc != 0 } {
      set Beamline [lindex $argv 0]
   } else {
      puts stderr "mtrPositionsRestoreMenu: no beamline specified. Using the development environment 23d !"
      set Beamline "23d"
   }

   if { $Beamline != "23i"  &&  $Beamline != "23o"  &&  $Beamline != "23b"  &&  $Beamline != "23d" } {
      set txt "*** mtrPositionsRestoreMenu: \n"
      set txt "${txt}Unknown beamline ${Beamline} in the command line \n"
      set txt "${txt}Beamline must be one of the following: \n"
      set txt "${txt}23i, 23o, 23b, 23d.\n"
      tk_messageBox -message $txt -type ok -icon error -title "mtrPositionsRestoreMenu Error"
      pError $txt
   }

   if { [ file exists /etc/passwd ] } {
      set SYSTEM LINUX
      set sep /
   } else {
      set SYSTEM WIN32
      set sep \\
   }

   set tclGMCA [pwd]${sep}
   set restoreScript mtrPositionsRestore2.pl

   source ${tclGMCA}config.tcl
   source ${tclGMCA}config${Beamline}.tcl
   set snapFile ""
   set dateStamp ""
   set user ""

   frame .fHdrs -borderwidth 2   -background #FF0000
   label .lbSnap -text $snapFile -background #FF0000 -foreground white
   pack .lbSnap -in .fHdrs -side left  -anchor ne

   pack .fHdrs -side top  -padx 0 -pady 0 -fill x

   frame .fRestore -borderwidth 2
   frame .fLabels -relief sunken -borderwidth 2
   label .lbLabelDate -text "Date = ${dateStamp}"
   label .lbLabelUser -text "User = ${user}"
   label .lbLabelBeam -text "Line = ${Beamline}"
   pack .lbLabelDate .lbLabelUser .lbLabelBeam -in .fLabels -side top -anchor w
   button .bFuncSelect -text "Select All" -underline 0 -width 9     \
	      -background       #FFFF00 -foreground       black     \
              -activebackground #EEEE00 -activeforeground black     \
              -command "pSelect 1"
   button .bFuncUnselect -text "Unselect All" -underline 0 -width 9 \
	      -background       #FFFF00 -foreground       black     \
              -activebackground #EEEE00 -activeforeground black     \
              -command "pSelect 0"
   button .bFuncNewSnap -text "Another File" -underline 0 -width 9  \
	      -background       #FFFF00 -foreground       black     \
              -activebackground #EEEE00 -activeforeground black     \
              -command "pChangeSnap"
   button .bFuncRestore -text "RESTORE" -underline 0 -width 9       \
	      -background       #FFFF00 -foreground       black     \
              -activebackground #EEEE00 -activeforeground black     \
              -command "pRestore"
   pack .fLabels .bFuncSelect .bFuncUnselect .bFuncNewSnap .bFuncRestore -in .fRestore -side top -padx 5 -pady 5

   pack .fRestore -side left -padx 4 -pady 4 -anchor n

   set snapFile [tk_getOpenFile -title {Restore Positions: Open SNAP File} \
                -initialdir ${snapdir} -filetypes {{SNAP {.snap}} {ALL {*}}}]
   if {$snapFile == ""} { exit; }
   set restoreList [pReadSnap $snapFile]
   .lbSnap      config -text $snapFile
   .lbLabelDate config -text "Date = ${dateStamp}"
   .lbLabelUser config -text "User = ${user}"
   pDrawRack
   pSelect 1

#}						# End of main proc.
