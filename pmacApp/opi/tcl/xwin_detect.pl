#!/usr/bin/perl -W
#----------------------------------------------------------------
# This script detects whether X-server is running on Windows.
# Please set the environment variable:
#               DISPLAY=localhost:0.0
#----------------------------------------------------------------
  my ($xwin1, $xwin2, $exceed);

  if ( (-f '/etc/passwd') && (! -d '/cygdrive/c') ) {
# On UNIX/Linux the X-server is always running:
	print STDOUT '1';
	exit;
  }

  $xwin1  = `ps -e -W | grep -i /usr/X11R6/bin/XWin`;
  $xwin2  = `ps -e -W | grep -i /usr/bin/XWin`;
  $exceed = `ps -e -W | grep -i exceed`;
  if ($xwin1 =~ /XWin/i ||
      $xwin2 =~ /XWin/i ||
      $exceed =~ /exceed/i) {print STDOUT '1';}
  else                      {print STDOUT '0';}
  exit;
