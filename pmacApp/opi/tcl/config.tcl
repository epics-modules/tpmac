# config.tcl

if { [ file exists /etc/passwd ] } {
   set SYSTEM LINUX
} else {
   set SYSTEM WIN32
}

if { ${SYSTEM} == "LINUX" } {
#
    set et       wish
    set sp       /
    set myhome   $env(HOME)
    set topdir   .
    set topdir2  ${topdir}
    set adldir   adl
    set tcldir   tcl
    set burtdir  burt
    set xterm    "xterm +cm -sb -sl 5000"
#
} else {
    set et       wish
    set sp       \\\\
    set myhome   .
    set topdir   ${myhome}
    set topdir   ${myhome}
    set adldir   adl
    set tcldir   tcl
    set burtdir  burt
    set xterm    xterm.exe
}

  set adlMTR      ${topdir}${sp}..${sp}${adldir}${sp}
  set adlPMAC     ${topdir}${sp}..${sp}${adldir}${sp}
  set icondir     .${sp}

  set medm        medm
  set varyFont    { -displayFont alias }
