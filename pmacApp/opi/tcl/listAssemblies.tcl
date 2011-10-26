#! /usr/bin/wish -f
#! wish.exe -f
#-----------------------------------------------------------------------------------------
# This script creates a chart of all beamline assemblies. It must be called with beamline
# name argument; otherwise 23d is used by default.
#
#                         Author: Sergey Stepanov
#
# Version-1: 2007/03 -- calls caget programs to get assembly numbers
# Version-2: 2008/02 -- added using caTCL functions instead of caget (if caTCL exists)
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
#    puts "Using caTCL from ${CA_EXTENSIONS}"
     set auto_path [linsert $auto_path 0 $CA_EXTENSIONS]
     package require ca
}

#-----------------------------------------------------------------------------------------
proc pError {text} {
  puts stderr "${text}"
  flush stderr
  exit
}

#-----------------------------------------------------------------------------------------
proc chk_exit {} {
# set ans [tk_messageBox -type yesno -icon question -message "Are you sure you want to quit?"]
# if {$ans != "no"} {
     exit
# }
};			# chk_exit


#----------------------------------------------------------------------------------------
### main:
# proc main {Beamline} {

   if {$argc != 0 &&  $argc != 1}  {
      set txt [format "%s\n%s\n%s\n%s\n" "listAssemblies:" \
                                         "Incorrect number of command line arguments." \
                                         "The command line must specify beamline name as one of:" \
                                         "23i, 23o, 23b, 23d."]
      tk_messageBox -message $txt -type ok -icon error -title "listAssemblies Error"
      pError $txt
   }

   if {$argc != 0} {
      set Beamline [lindex $argv 0]
   } else {
      puts stderr "listAssemblies: no beamline specified.\n Using the development environment 23d !\n"
      set Beamline "23d"
   }

   if { $Beamline != "23i"  &&  $Beamline != "23o"  &&  $Beamline != "23b"  &&  $Beamline != "23d" } {
      set txt [format "%s\n%s\n%s\n" "listAssemblies:" \
                                     "Unknown beamline ${Beamline} in the command line." \
                                     "Must be one of the following: 23i, 23o, 23b, 23d."]
      tk_messageBox -message $txt -type ok -icon error -title "listAssemblies Error"
      pError $txt
   }

   if {[file exists /etc/passwd]} {set SYSTEM LINUX;  set sep  /;} \
   else                           {set SYSTEM WIN32;  set sep \\;}

   if {[info exists env(FASTSCANS)]} {
      set Fastscans $env(FASTSCANS)
   } else {
      if {${SYSTEM} == "LINUX"} {set Fastscans ${sep}home${sp}gmca${sep}epics${sep}fastscans;} \
      else                      {set Fastscans c:${sep}gmca${sep}fastscans;}
   }
### Produce tclGMCA dir name from Fastscans:
   set tclGMCA [regsub fastscans ${Fastscans} gmcaApp${sep}tcl${sep}]

#  set bln ${Beamline}:
   source ${tclGMCA}config.tcl
   source ${tclGMCA}config${Beamline}.tcl

### Sergey: intercept killing window:
   wm protocol . WM_DELETE_WINDOW chk_exit

   frame .frHdrs -borderwidth 2                    -background #FF0000
   label .lbBeamline -text "Beamline=${Beamline}" -background #FF0000 -foreground white
   pack .lbBeamline -in .frHdrs -side left  -anchor ne
   pack .frHdrs -side top  -padx 0 -pady 0 -fill x

   frame .frAssys -borderwidth 2                     -background #5959E5

   set pmacList {}

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
       set assembly ${Beamline}:${cmp}${assy}
       if {$CA_EXIST} {
 	  set timeout 0.5
          set assyNo($pmac) 0
          if {[pv linkw epics_var ${assembly}Pcs $timeout] != 0} {
             puts stderr "Error connecting to ${assembly}Pcs, code=${errorCode}"
             flush stderr
	  } else {
	     if {[pv getw epics_var  $timeout] != 0} {
                puts stderr "Error reading ${assembly}Pcs, code=${errorCode}"
                flush stderr
	     } else {
                set assyNo($pmac) $epics_var
	     }
	  }
	  pv unlink epics_var
       } else {
          catch {exec caget -t ${assembly}Pcs} assyNo($pmac)
#         puts "${assembly}Pcs -> $assyNo($pmac)"
### Filter output in case there is something else:
          if {[catch {scan $assyNo($pmac) "%d" assyNo($pmac)} err]} {
             set assyNo($pmac) 0
          }
       }
       set axesList {}
       if {[llength $axes] > 0} {
         foreach axis $axes {
	   set axis ${Beamline}:${cmp}${axis}
	   lappend axesList $axis
	 }
       } else {
         foreach axis $drvs {
	   set axis ${Beamline}:${cmp}${axis}
	   lappend axesList $axis
	 }
       }
       lappend assyList($pmac) [concat  $assyNo($pmac) ${Beamline}:${cmp}$assy " (" $axesList ")" ]
     }
   }
### Create panel
   foreach  pmac $pmacList {
     frame .fr$pmac -relief flat
     label .lb$pmac -text "$pmac" -justify left  \
	                 -background       #FFC0CE -foreground       black \
	                 -activebackground #FFA0AE -activeforeground black
     pack .lb$pmac -in .fr$pmac -ipadx 2 -ipady 0 -side top -anchor nw -fill x

     set assyList($pmac) [lsort -integer -index 0 -increasing $assyList($pmac)]
     foreach  unit $assyList($pmac) {
       set n [lindex $unit 0]
       set assy [lrange $unit 1 end]
       if {[winfo exists .lb$pmac$n]} {
         set txt [format "%s\n%s\n%s\n" "listAssemblies:" \
                                        "Duplicate widget name \'.lb$pmac$n\'." \
                                        "Possible config${Beamline}.tcl error."]
         tk_messageBox -message $txt -type ok -icon error -title "listAssemblies Error"
         pError $txt
       }
### -adobe-courier-medium-r-normal--11-80-100-100-m-60-iso10646-1
       if {$n<10} {label .lb$pmac$n -text "0${n}   ${assy}" -foreground black -font -adobe-times-medium-r-normal--13-*-75-75-*-*-*-* } \
       else       {label .lb$pmac$n -text  "${n}   ${assy}" -foreground black -font -adobe-times-medium-r-normal--13-*-75-75-*-*-*-* }
       pack  .lb$pmac$n -in .fr$pmac -ipadx 0 -ipady 0 -side top -anchor nw -fill y
     }
     pack .fr$pmac -in .frAssys -side left -padx 2 -pady 0 -anchor nw -fill y
   }


   pack .frAssys -side top  -padx 0 -pady 0 -fill x
#}
