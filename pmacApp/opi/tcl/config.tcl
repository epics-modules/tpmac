# config.tcl

if { [info exists env(HOST)] } {
   set HOST $env(HOST)
#  puts "gmca: HOST=${HOST}"
} elseif { [info exists env(HOSTNAME)] } {
   set HOST $env(HOSTNAME)
#  puts "gmca: HOSTNAME=${HOST}"
} else {
   set HOST "unknown"
   puts "!!! config.tcl: unknown host"
}
regsub ".*$" $HOST "" HOST

if { [ file exists /etc/passwd ] } {
   set SYSTEM LINUX
} else {
   set SYSTEM WIN32
}

# -------------------------------------- LINUX ------------------------------------------
if { ${SYSTEM} == "LINUX" } {
#
   set wish     /usr/bin/wish
   set et       $wish
   set sp       /
   set myhome   $env(HOME)
#  set topdir   /home/gmca/epics
   set topdir   ../
   set topdir2  ${topdir}
   set snapdir  ${topdir}${sp}logfiles${sp}
   if {![file exists $snapdir]} {catch "file mkdir ${snapdir}"}
   if {![file exists $snapdir] || ![file isdirectory $snapdir] || ![file writable $snapdir]} {
      puts "\n\n"
      puts "!!! config.tcl: directory ${snapdir} cannot be created or is not writable."
      puts "!!! config.tcl: snapshots will default to ${myhome}"
      puts "\n\n"
      set snapdir $myhome
   }
   set adldir    adl
   set tcldir    tcl
   set burtdir   burt
### Add "-hold" option to diagnose script problems:
#  set xterm    "xterm +cm -sb -rightbar -sl 5000 -e"
   set xterm    "gnome-terminal --hide-menubar --disable-factory -x "
#
} else {
# -------------------------------------- WINDOWS ----------------------------------------
   set wish     wish
   set et       $wish
   set sp       /
   set myhome   ..${sp}
   set topdir   $myhome
   set topcyg   ${topdir}
   set topcyg   [regsub : ${topcyg} {}]
   set topcyg   [regsub $sp ${topcyg} /]
### Cygwin/Win32 specific:
   if {$env(EPICS_HOST_ARCH) == "win32-x86-cygwin"} {set topdir2  $topcyg} \
   else                                             {set topdir2  $topdir}
   set snapdir  $topdir${sp}shared${sp}
   set adldir    adl
   set tcldir    tcl
   set burtdir   burt
### Add "-hold" option to diagnose script problems:
   set xterm    "xterm +cm -sb -rightbar -sl 5000 -e"
### Use "cmd" version calls ".bat" files, while the "xterm" version calls csh scripts:
#  set xterm    "cmd /c"
}
# --------------------------------END-LINUX/WINDOWS--------------------------------------

# set adlAPS      ${topdir2}${sp}${adldir}APS${sp}
  set adlGMCA     ${topdir2}${sp}${adldir}GMCA${sp}
# set adlMCA      ${topdir2}${sp}${adldir}MCA${sp}
  set adlMTR      ${topdir2}${sp}${adldir}MTR${sp}
  set adlPMAC     ${topdir2}${sp}${adldir}PMAC${sp}
  set adlSTD      ${topdir2}${sp}${adldir}STD${sp}

  set tclGMCA     ${topdir}${sp}${tcldir}${sp}
### If we are xterm on Windows, we need to pass Windows-type path to wrappers:
  if {${SYSTEM} == "WIN32" && [regexp -nocase ^xterm $xterm]} {set tclGMCA2 ${topcyg}/${tcldir}/} \
  else                                                        {set tclGMCA2 ${tclGMCA}}
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

# if {![info exists env(FASTSCANS)]} {set env(FASTSCANS) "${topdir}${sp}fastscans";}
# if {![info exists env(PEZCA)]}     {set env(PEZCA) "${topdir}${sp}pezca";}
  set env(FASTSCANS) $tclGMCA
  set env(PEZCA)     $tclGMCA

  set PEZCA $env(PEZCA)
  set FASTSCANS $env(FASTSCANS)
