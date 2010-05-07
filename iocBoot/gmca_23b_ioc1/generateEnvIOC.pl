#!/usr/bin/perl -W
# eval 'exec perl -S $0 ${1+"$@"}'  # -*- Mode: perl -*-
#     if $running_under_some_shell; # convertRelease.pl
#*************************************************************************
# Copyright (c) 2002 The University of Chicago, as Operator of Argonne
#     National Laboratory.
# Copyright (c) 2002 The Regents of the University of California, as
#     Operator of Los Alamos National Laboratory.
# EPICS BASE Versions 3.13.7
# and higher are distributed subject to a Software License Agreement found
# in file LICENSE that is included with this distribution.
#*************************************************************************
#
# convertRelease.pl,v 1.14.2.16 2005/11/30 21:44:55 jba Exp
#
# Parse configure/RELEASE file(s) and generate a derived output file.
#

use Cwd qw(cwd abs_path);
use Getopt::Std;

# ################################################################################
# print "Usage: generateEnvIOC.pl [-a arch] [-t ioctop] [AUTOSAVE CALC MCA ...]\n";
# -- here the last arguments constitute an optional mask on to include
#   into iocEnv. If it is empty, all definitions from local RELEASE are included.
# ################################################################################
$outfile = "iocEnv";

$cwd = UnixPath(cwd());

getopt "at";
unless (@ARGV == 0) {@envMask = @ARGV;}

if ($opt_a) {
    $arch = $opt_a;
} else {		# Look for O.<arch> in current path
    $_ = $cwd;
    ($arch) = /.*\/O.([\w-]+)$/;
}

# This approach is only possible under iocBoot/* and configure/*
$top = $cwd;
$top =~ s/\/iocBoot.*$//;
$top =~ s/\/configure.*$//;

# The IOC may need a different path to get to $top
if ($opt_t) {
    $iocroot = $opt_t;
    $root = $top;
    while (substr($iocroot, -1, 1) eq substr($root, -1, 1)) {
	chop $iocroot;
	chop $root;
    }
}

# TOP refers to this application
%macros = (TOP => LocalPath($top));
@apps   = (TOP);	# Records the order of definitions in RELEASE file

# Read the RELEASE file(s)
$relfile = "$top/configure/RELEASE";
die "Can't find $relfile" unless (-f $relfile);
&readReleaseFiles($relfile, \%macros, \@apps);
&expandRelease(\%macros, \@apps);

&makeEnv;
exit;

#
# Parse all relevent configure/RELEASE* files and includes
#
sub readReleaseFiles {
    my ($relfile, $Rmacros, $Rapps) = @_;

    return unless (-r $relfile);
    &readRelease($relfile, $Rmacros, $Rapps);
    if ($arch) {
	my ($hrelfile) = "$relfile.$arch";
	&readRelease($hrelfile, $Rmacros, $Rapps) if (-r $hrelfile);
	my ($crelfile) = "$relfile.Common.$arch";
	&readRelease($crelfile, $Rmacros, $Rapps) if (-r $crelfile);
	my ($arelfile) = "$relfile.$arch.$arch";
	&readRelease($arelfile, $Rmacros, $Rapps) if (-r $arelfile);
    }
}

#
# Parse a configure/RELEASE file and its includes.
#
# NB: This subroutine also appears in base/src/makeBaseApp/makeBaseApp.pl
# If you make changes here, they will be needed there as well.
#
sub readRelease {
    my ($file, $Rmacros, $Rapps) = @_;
    # $Rmacros is a reference to a hash, $Rapps a ref to an array
    my ($pre, $var, $post, $macro, $path);
    local *IN;
    open(IN, $file) or die "Can't open $file: $!\n";
    while (<IN>) {
	chomp;
	s/\r$//;		# Shouldn't need this, but sometimes...
	s/\s*#.*$//;		# Remove trailing comments
	s/\s+$//;		# Remove trailing whitespace
	next if /^\s*$/;	# Skip blank lines

	# Expand all already-defined macros in the line:
	while (($pre,$var,$post) = /(.*)\$\((\w+)\)(.*)/) {
	    last unless (exists $Rmacros->{$var});
	    $_ = $pre . $Rmacros->{$var} . $post;
	}

	# Handle "<macro> = <path>"
	($macro, $path) = /^\s*(\w+)\s*=\s*(.*)/;
	if ($macro ne "") {
		$macro="TOP" if $macro =~ /^INSTALL_LOCATION/ ;
		if (exists $Rmacros->{$macro}) {
			delete $Rmacros->{$macro};
		} else {
			push @$Rapps, $macro;
		}
	    $Rmacros->{$macro} = $path;
	    next;
	}
	# Handle "include <path>" syntax
	($path) = /^\s*include\s+(.*)/;
	&readRelease($path, $Rmacros, $Rapps) if (-r $path);
    }
    close IN;
}

sub expandRelease {
    my ($Rmacros, $Rapps) = @_;
    # Expand any (possibly nested) macros that were defined after use
    while (($macro, $path) = each %$Rmacros) {
	while (($pre,$var,$post) = $path =~ /(.*)\$\((\w+)\)(.*)/) {
	    $path = $pre . $Rmacros->{$var} . $post;
	    $Rmacros->{$macro} = $path;
	}
    }
}

sub makeEnv {
    die "Architecture not set (use -a option)" unless ($arch);
    @includes = grep !/^TEMPLATE_TOP$/, @apps;

    unlink($outfile);
    open(OUT,">$outfile") or die "$! creating $outfile";

    $startup = $cwd;
    $startup =~ s/^$root/$iocroot/o if ($opt_t);
    $appbin  = $top."/bin/".$arch;
    $appbin =~ s/^$root/$iocroot/o if ($opt_t);

    print OUT "startup = \"$startup\"\n";
    print OUT "appbin = \"$appbin\"\n" if (-d "$top/bin/$arch");

    $ioc = $cwd;
    $ioc =~ s/^.*\///;	# iocname is last component of directory name

#   print OUT "putenv \"ARCH=$arch\"\n";
#   print OUT "putenv \"IOC=$ioc\"\n";

    foreach $app (@includes) {
	$iocpath = $path = $macros{$app};
	$iocpath =~ s/^$root/$iocroot/o if ($opt_t);
        $incApp  = 1;
        if (defined @envMask) {
            if (!grep(/^${app}$/, @envMask)) {$incApp = 0;}
        }
        if ($path =~ /^$root/ && $incApp) {
            if  (-d "$path/db") {
                print OUT "putenv \"$app=$iocpath/db\"\n";
            } else {
                &readDir($path, \@arrayApp);
                $nApp = @arrayApp;
                if ($nApp > 1) {
                    print "!!! Found ${nApp} xxxApp directories for $path\n";
                    print "!!! Names: @{arrayApp} \n";
                    print "!!! Ignoring...\n";
                }
                elsif ($nApp == 0) {
                    print "!!! No xxxApp directories for $path\n";
                }
	        else {
                    if (-d "$path/${arrayApp[0]}/Db") {
                        print OUT "putenv \"$app=$iocpath/${arrayApp[0]}/Db\"\n";
                    }
                }
	    }
        }
    }
    close OUT;
}

# Path rewriting rules for various OSs
# These functions are duplicated in src/makeBaseApp/makeBaseApp.pl
sub UnixPath {
    my ($newpath) = @_;
    if ($^O eq "cygwin") {
	$newpath =~ s|\\|/|go;
	$newpath =~ s%^([a-zA-Z]):/%/cygdrive/$1/%;
    } elsif ($^O eq 'sunos') {
	$newpath =~ s(^\/tmp_mnt/)(/);
    }
    return $newpath;
}

sub LocalPath {
    my ($newpath) = @_;
    if ($^O eq "cygwin") {
	$newpath =~ s%^/cygdrive/([a-zA-Z])/%$1:/%;
    } elsif ($^O eq "darwin") {
	# These rules are likely to be site-specific
	$newpath =~ s%^/private/var/auto\.home/%/home/%;    # APS
    }
    return $newpath;
}

sub readDir {
  my $path     = shift @_, 
  my $arrayApp = shift @_;
  @$arrayApp = ();
  opendir(DIRHANDLE, $path);
  if (!defined \*DIRHANDLE) {return;}
  @list = readdir DIRHANDLE;
  closedir DIRHANDLE;
  foreach $dir (@list) {
    if ($dir =~ /App$/) {push @$arrayApp, $dir;}
  }
}
