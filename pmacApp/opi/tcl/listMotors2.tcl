#! /usr/bin/wish -f
#! wish.exe -f
#-----------------------------------------------------------------------------------------
# This creates a chart of all beamline motors. It must be called with beamline name argument;
# otherwise 23d is used by default.
# Version-2, 2007/02/24: unlike Version-1, this shows unused channels in PMAC.
#-----------------------------------------------------------------------------------------

#-----------------------------------------------------------------------------------------
proc pError {text} {
	puts stderr "${text}"
        flush stderr
	exit
}

#----------------------------------------------------------------------------------------
### main:
# proc main {Beamline} {

   if { $argc != 0 &&  $argc != 1 }  {
      set txt "listMotors: Incorrect number of command line arguments. The command line must specify beamline name as one of: 23i, 23o, 23b, 23d.\n"
#     message .msg -width 10c -justify left -text "${txt}"
#     button  .ok  -text "OK" -command "pError {${txt}}"
#     pack .msg
#     pack .ok
      tk_messageBox -message $txt -type ok -icon error -title "listMotors Error"
      pError $txt
   }

   if { $argc != 0 } {
      set Beamline [lindex $argv 0]
   } else {
      puts stderr "listMotors: no beamline specified. Using the development environment 23d !"
      set Beamline "23d"
   }

   if { $Beamline != "23i"  &&  $Beamline != "23o"  &&  $Beamline != "23b"  &&  $Beamline != "23d" } {
      set txt "listMotors: Unknown beamline ${Beamline} in the command line -- must be one of the following: 23i, 23o, 23b, 23d.\n"
#     message .msg -width 10c -justify left -text "${txt}"
#     button  .ok  -text "OK" -command "pError {${txt}}"
#     pack .msg
#     pack .ok
      tk_messageBox -message $txt -type ok -icon error -title "listMotors Error"
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

   frame .frHdrs -borderwidth 2                    -background #FF0000
   label .lbBeamline -text "Beamline=${Beamline}" -background #FF0000 -foreground white
   pack .lbBeamline -in .frHdrs -side left  -anchor ne
   pack .frHdrs -side top  -padx 0 -pady 0 -fill x

   frame .frMtrs -borderwidth 2                     -background #5959E5

   set pmacList {}
#  set mtrList {}

   foreach component $ComponentList {
     set name  [lindex $component 0]
     set cmp   [lindex $component 1]
     set units [lindex $component 2]
     foreach unit $units {
# { "Vertical   Aperture" Av:  HSAp pmac10: 08XY3  {ZC: ZS:} {ZT: ZB:} {{ 1 mt: } { 2 mb: }}   }
       set lbl   [lindex $unit 0]
       set assy  [lindex $unit 1]
       set tsub  [lindex $unit 2]
       set pmac  [lindex $unit 3]
       set dpram [lindex $unit 4]
       set axes  [lindex $unit 5]
       set drvs  [lindex $unit 6]
       set mtrs  [lindex $unit 7]
       if {[lsearch $pmacList $pmac] == -1} {lappend pmacList $pmac}
       foreach nmpair $mtrs {
         set n [lindex $nmpair 0]
         set m [lindex $nmpair 1]
         set motor ${Beamline}:${cmp}${m}
	 lappend mtrList($pmac) [list  $n $motor]
#        label .lb$motor -text "${pmac}   ${motor}   ${n}" -background #5959E5 -foreground white
#        pack  .lb$motor -in .frMtrs -side top -anchor nw -fill y
       }
     }
   }
# source "listmotors.tcl"
   foreach  pmac $pmacList {
     frame .fr$pmac -relief flat
     label .lb$pmac -text "$pmac" -justify left  \
	                 -background       #FFC0CE -foreground       black \
	                 -activebackground #FFA0AE -activeforeground black
     pack .lb$pmac -in .fr$pmac -ipadx 2 -ipady 0 -side top -anchor nw -fill x

     set mtrList($pmac) [lsort -integer -index 0 -increasing $mtrList($pmac)]
     for {set n 1} {$n <= 32} {incr n} {
       set mtrList2($pmac,$n) " "
     }
     foreach  nmpair $mtrList($pmac) {
       set n [lindex $nmpair 0]
       set m [lindex $nmpair 1]
       if {$mtrList2($pmac,$n) == " "} {
         set mtrList2($pmac,$n) "${m}"
       } else {
         set txt "listMotors: Duplicate motor $n name \'$m\'. Possible config${Beamline}.tcl error.\n"
         tk_messageBox -message $txt -type ok -icon error -title "listMotors Error"
         pError $txt
       }
     }
     if {$mtrList2($pmac,32) == " "} {set mtrList2($pmac,32) "reserved out"}
     for {set n 1} {$n <= 32} {incr n} {
       set m $mtrList2($pmac,$n)
       if {[winfo exists .lb$pmac$n]} {
         set txt "listMotors: Duplicate widget name \'.lb$pmac$n\'. Possible config${Beamline}.tcl error.\n"
         tk_messageBox -message $txt -type ok -icon error -title "listMotors Error"
         pError $txt
       }
       if {$n<10} {label .lb$pmac$n -text "0${n}   ${m}" -foreground black} \
       else       {label .lb$pmac$n -text  "${n}   ${m}" -foreground black}
       pack  .lb$pmac$n -in .fr$pmac -ipadx 0 -ipady 0 -side top -anchor nw -fill y
     }
     pack .fr$pmac -in .frMtrs -side left -padx 2 -pady 0 -anchor nw -fill y
   }


   pack .frMtrs -side top  -padx 0 -pady 0 -fill x
#}
