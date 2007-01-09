#!/usr/bin/perl
#----------------------------------------------------------------
# This script detects whether X-server is running on Windows.
# Please set the environment variable:
#               DISPLAY=localhost:0.0
#----------------------------------------------------------------
  if ( (-f "/etc/passwd") && (! -d "/cygdrive/c") ) {
# On UNIX/Linux the X-server is always running:
	print STDOUT "1";
	exit;
  }


   $xwin   = `ps -e -W | grep /usr/X11R6/bin/XWin`;
   $exceed = `ps -e -W | grep exceed.exe`;
   if ( $xwin =~ "XWin" || $exceed =~ "exceed.exe") {
	  print STDOUT "1";
   } else {
	  print STDOUT "0";
   }
   exit;
