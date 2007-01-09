# config.tcl

if { [info exists env(HOST)] } {
   set HOST $env(HOST)
} elseif { [info exists env(HOSTNAME)] } {
   set HOST $env(HOSTNAME)
} else {
   set HOST "unknown"
   puts "!!! gmca: unknown host"
}

if { [ file exists /etc/passwd ] } {
   set SYSTEM LINUX
} else {
   set SYSTEM WIN32
}

if { ${SYSTEM} == "LINUX" } {
#
    set wish     wish
    set sp       /
    set myhome   $env(HOME)
    set topdir   [pwd]/..
    set topdir2  ${topdir}
    set snapdir  ${myhome}
    set adldir   adl
    set tcldir   tcl
    set burtdir  burt
    set xterm    "gnome-terminal --hide-menubar -x "
#
} else {
#
    set wish     wish
    set sp       /
    set myhome   [pwd]/..
    set topdir   ${myhome}
    set topdir2  ${topdir}
    set snapdir  ${myhome}
    set adldir   adl
    set tcldir   tcl
    set burtdir  burt
    set xterm    "xterm +cm -sb -rightbar -sl 5000 -e"
    if { $env(EPICS_HOST_ARCH) == "win32-x86-cygwin" } {
### Cygwin-specific:
        set topdir2  /cygdrive/${topdir2}
	regsub -all :  ${topdir2} / topdir2
	regsub -all \\ ${topdir2} / topdir2
    }
}

  set adlGMCA     ${topdir2}${sp}${adldir}${sp}
  set adlMCA      ${topdir2}${sp}${adldir}${sp}
  set adlMTR      ${topdir2}${sp}${adldir}${sp}
  set adlPMAC     ${topdir2}${sp}${adldir}${sp}
  set adlSTD      ${topdir2}${sp}${adldir}${sp}

  set tclGMCA     ${topdir}${sp}${tcldir}${sp}
  set tclMTR      ${topdir}${sp}${tcldir}${sp}
  set icondir     ${tclGMCA}

  set burtGMCA    $topdir${sp}${burtdir}${sp}
  set burtSTD     $topdir${sp}${burtdir}${sp}

  set medm        medm
  set varyFont    { -displayFont alias }
  set burtSave    burtrb
  set burtRestore burtwb
  set burtgooey   ${tclGMCA}burtgui
  set stripTool   StripTool
  set perl	  perl

