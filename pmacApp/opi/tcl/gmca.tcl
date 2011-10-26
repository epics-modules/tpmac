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

if { [ file exists /etc/passwd ] } {
   set SYSTEM LINUX
   set tclGMCA  /home/gmca/epics/gmcaApp/tcl/
} else {
   set SYSTEM WIN32
#  set tclGMCA  {c:\\gmca\\gmcaApp\\tcl\\}
   set tclGMCA  {c:/gmca/gmcaApp/tcl/}
### Set this system-wide:
#  set env(DISPLAY) "locaslhost:0.0"
   set xwin  [exec perl ${tclGMCA}xwin_detect.pl]
   if { $xwin == "0" } {
#     exec XWin.exe -multiwindow -clipboard -screen 0 @1 -screen 0 @2
#     exec XWin.exe -multiwindow -clipboard -screen 0 1280x1024@1 -screen 0 1600x1200@2 &
      exec XWin.exe -multiwindow -clipboard -multiplemonitors &
#     exec XWin.exe -multiwindow -clipboard &
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

if { $Control == "staff" && $SYSTEM != "WIN32" } {
  if { $env(USER) != "gmca"         && \
       $env(USER) != "user0"        && \
       $env(USER) != "dyoder"       && \
       $env(USER) != "makarov"      && \
       $env(USER) != "mbecker"      && \
       $env(USER) != "mhilgart"     && \
       $env(USER) != "nukri"        && \
       $env(USER) != "ogata"        && \
       $env(USER) != "rbenn"        && \
       $env(USER) != "rfischetti"   && \
       $env(USER) != "sergey"       && \
       $env(USER) != "spothineni"   && \
       $env(USER) != "vnagas"       && \
       $env(USER) != "root" } {
    catch {exec ${tclGMCA}/authorize.tcl } txt
    if { $txt != "0" } {
      pError "Authorization failed (${txt})"
    }
  }
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
# if { $SYSTEM != "WIN32" } {
#    source ${tclGMCA}px.tcl
# }

