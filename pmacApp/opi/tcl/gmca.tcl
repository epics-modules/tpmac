#! /usr/bin/wish -f
#! /usr/local/epics/extensions/bin/solaris/et -f
#! wish82.exe -f

#-----------------------------------------------------------------------------------------
proc pError {text} {
	puts stderr "${text}"
	exit
}

#-----------------------------------------------------------------------------------------
# main:

set tclGMCA  [pwd]/
if { [ file exists /etc/passwd ] } {
   set SYSTEM LINUX
} else {
   set SYSTEM WIN32
### For Windows you must have cygwin Xwin.exe installed:
### Set this environment system-wide:
#  set env(DISPLAY) "locaslhost:0.0"
   set xwin  [exec perl ${tclGMCA}xwin_detect.pl]
   if { $xwin == "0" } {
      exec XWin.exe -multiwindow -clipboard &
#     exec sleep 2
      after 2000
   }
}

if { $argc != 2 }  {
  set txt "gmca: Incorrect number of command line argumentts. The command line must specify beamline name as one of: 23i, 23o, 23b, 23d.\n"
  tk_messageBox -message $txt -type ok -icon error -title "Application Error"
  pError $txt
}

set Beamline [lindex $argv 0]
if { $Beamline != "23i"  &&  $Beamline != "23o"  &&  $Beamline != "23b"  &&  $Beamline != "23d" } {
  set txt "gmca: Unknown beamline \[${Beamline}\] in the command line; expect: 23i, 23o, 23b, 23d.\n"
  tk_messageBox -message $txt -type ok -icon error -title "Application Error"
  pError $txt
}

set Control [lindex $argv 1]
if { $Control != "staff"  &&  $Control != "users" } {
  set txt "gmca: Unknown control \[${Control}\] in the command line; expect: staff, users.\n"
  tk_messageBox -message $txt -type ok -icon error -title "Application Error"
  pError $txt
}

  set bln ${Beamline}:
  set subTitle (${Control})
  source ${tclGMCA}config.tcl
  source ${tclGMCA}config${Beamline}.tcl

### All the variables below (myhome,sp,...) are defined in config.tcl:

### This must redirect BOTH of the standard and error IO streams (p.112):
  set errorFile >>&
  append errorFile " ${myhome}${sp}gmca${Beamline}.err"

# exec $medm -attach $varyFont -x -cmap &
  exec $medm -attach $varyFont -x       &
# exec sleep 2

  source ${tclGMCA}main.tcl
# source ${tclGMCA}aps.tcl
  source ${tclGMCA}motion_${Control}.tcl
# source ${tclGMCA}scan.tcl
# source ${tclGMCA}tools.tcl

