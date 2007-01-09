#!/usr/bin/perl -W
use strict;

#--------------------------------------------------------------
# This is a PERL script to produce DPRAM map files of M3300 and
# Q70 variables from the map of PMAC Coordinate Systems (PCS)
#
# Originally written by Satish Devarapalli Sep 23, 2005
# Modified and maintained by Sergey Stepanov.
#--------------------------------------------------------------

my $m3300Address     = 0x60450;
my $m3300LineAddress = 3300;
my $q70Address       = 3300;
my $q70LineAddress   = 70;
my $maxPCS           = 16;
my $PCSnum           = 1;
my $PCSname          = "";

my $pcsInputFile;
my $m3300OutputFile;
my $q70OutputFile;

my @all_lines;
my $no_of_lines;
my $prev_line;
my $i;
my $coordinate;
my $temp;

############################## User Input ##############################

my $narg = $#ARGV + 1;
  if ( $narg != 1 ) {
     print STDOUT "Syntax: mapDPRAM.pl  pcsInputFilename...\n";
     exit;
  }

#print "Please enter input file: ";
#chop($pcsInputFile=<STDIN>);
$pcsInputFile=$ARGV[0];

# print "Please enter m3300 output file: ";
# chop($m3300OutputFile=<STDIN>);
$m3300OutputFile=$pcsInputFile;
if    ( $m3300OutputFile =~ /^pcs/ ) {
  $m3300OutputFile =~ s/^pcs/m3300/;	
} 
elsif ( $m3300OutputFile =~ /^1_pcs/ ) {
  $m3300OutputFile =~ s/^1_pcs/2_m3300/;	
} 
elsif ( $m3300OutputFile =~ /.pmc$/ ) {
  $m3300OutputFile =~ s/.pmc$/_m3300.pmc/;
}
else  {
  $m3300OutputFile = "${m3300OutputFile}_m3300";
}
while ( -f $m3300OutputFile ) {
  $m3300OutputFile = "${m3300OutputFile}~";	#if file exists!
}

# print "Please enter q70 output file: ";
# chop($q70OutputFile=<STDIN>);
$q70OutputFile=$pcsInputFile;
if    ( $q70OutputFile =~ /^pcs/ ) {
  $q70OutputFile =~ s/^pcs/q70/;	
} 
elsif ( $q70OutputFile =~ /^1_pcs/ ) {
  $q70OutputFile =~ s/^1_pcs/3_q70/;	
} 
elsif ( $q70OutputFile =~ /.pmc$/ ) {
  $q70OutputFile =~ s/.pmc$/_q70.pmc/;
}
else  {
  $q70OutputFile = "${q70OutputFile}_q70";
}
while ( -f $q70OutputFile ) {
  $q70OutputFile = "${q70OutputFile}~";	#if file exists!
}

############################## Open Input and Output Files ##############################

print STDOUT "Opening ${pcsInputFile} for reading..\n";
open(DAT, $pcsInputFile) or die "Can't open $pcsInputFile: $!";
@all_lines=<DAT>;
close(DAT);

#m3300 output file
print STDOUT "Opening ${m3300OutputFile} for writing..\n";
open(M3300,">$m3300OutputFile") || die("Cannot Open $m3300OutputFile File");

#q70 output file
print STDOUT "Opening ${q70OutputFile} for writing..\n";
open(Q70,">$q70OutputFile") || die("Cannot Open $q70OutputFile File");

#Write M3300 File Header
print M3300 "; M-variable Definitions\n";
print M3300 "; point to motion program parameters in the DPRAM\n";
print M3300 "; use together with ${pcsInputFile} and ${q70OutputFile}\n";
print M3300 "\n";
print M3300 "CLOSE	; To ensure commands are on-line\n";
print M3300 "M3300..3540->*	; To clear existing definitions\n";

#Write Q70 File Header
print Q70 "; Q-variable Definitions\n";
print M3300 "; use together with ${pcsInputFile} and ${m3300OutputFile}\n";
print Q70 "\n";
print Q70 "CLOSE	; To ensure commands are on-line\n";


############################## Convert Files to M3300 and Q70 Format ##############################

$no_of_lines=scalar(@all_lines);

for($i=0;$i<$no_of_lines;$i++)
{
	chomp($all_lines[$i]);#truncate new line character
	$temp = $all_lines[$i];
	
	$all_lines[$i] =~ s/^[\s]+//g; #delete white spaces at the beginning of the line
	#$all_lines[$i] =~ s/\r+//g;   #delete carriage return \r (or <cr>) at the end of the line???
	
	#print " abc123\rxyz123\ref123\raaa"; #Interesting: Try printing \r in the output and see!!


	#skip blank lines and comment lines (lines starting with ';'
	if( $all_lines[$i] =~ m/^[\s]*$/ || substr($all_lines[$i],0,1) eq ";" )
	{
		#print "Skipping line: $all_lines[$i]\n";
		$prev_line=$all_lines[$i];
		
	} elsif ( substr($all_lines[$i],0,1) eq "&" ) {
		
		#; (1) WS:Av: ---> this is the previous line. PCSname=WS:Av
		#&1
		#1->X  ; WS:mt:
		#2->Y  ; WS:mb:

		if ( $prev_line =~ m/(\S*:\S*)/ ) 
		{
			$PCSname=$1;
		} else {
			$PCSname="";
		}
				
		#######  M3300 #######
		print M3300 "\n";
		
		print M3300 "; &$PCSnum DPRAM Program Parameters Set      $PCSname \n";
		
		printf M3300 "M$m3300LineAddress->DP:\$%x        ; TA0\n",$m3300Address;
		$m3300Address = $m3300Address + 1;
		$m3300LineAddress = $m3300LineAddress + 1;
		
		printf M3300 "M$m3300LineAddress->DP:\$%x        ; TS0\n",$m3300Address;
		$m3300Address = $m3300Address + 1;
		$m3300LineAddress = $m3300LineAddress + 1;
		
		####### Q70 #######
		print Q70 "\n";
		
		$q70LineAddress = 70;
		
		print Q70 "&$PCSnum 	;Coordinate System      $PCSname \n";
		print Q70 "Q70..99=3540	; Default for non-existent parameters\n";
		print Q70 "\n";
		
		print Q70 "Q$q70LineAddress=$PCSnum      \t; CS#\n";
		$q70LineAddress	= $q70LineAddress + 1;
		
		print Q70 "Q$q70LineAddress=$q70Address  \t; TA0  3300+(PREVIOUS RAM)\n";
		$q70LineAddress	= $q70LineAddress + 1;
		$q70Address	= $q70Address + 1;
		
		print Q70 "Q$q70LineAddress=$q70Address  \t; TS0\n";
		$q70LineAddress	= $q70LineAddress + 1;
		$q70Address	= $q70Address + 1;
				

		#####################
				
		$PCSnum = $PCSnum + 1;
		
	} elsif ( $all_lines[$i] =~m/->([XYZ])/ ) {
		
		$coordinate=$1;
		#print "Co-ordinate is $coordinate\n";
		
		printf M3300 "M$m3300LineAddress->DP:\$%x  \t; ${1}0\n",$m3300Address;
		$m3300Address = $m3300Address + 1;
		$m3300LineAddress = $m3300LineAddress + 1;
		
		printf M3300 "M$m3300LineAddress->F:\$%x   \t; ${1}1\n",$m3300Address;
		$m3300Address = $m3300Address + 1;
		$m3300LineAddress = $m3300LineAddress + 1;
		
		printf M3300 "M$m3300LineAddress->F:\$%x   \t; ${1}2\n",$m3300Address;
		$m3300Address = $m3300Address + 1;
		$m3300LineAddress = $m3300LineAddress + 1;
		
		######### Q70 #######
		print Q70 "Q$q70LineAddress=$q70Address  \t; ${1}0\n";
		$q70LineAddress	= $q70LineAddress + 1;
		$q70Address	= $q70Address + 1;
		
		print Q70 "Q$q70LineAddress=$q70Address  \t; ${1}1\n";
		$q70LineAddress	= $q70LineAddress + 1;
		$q70Address	= $q70Address + 1;
		
		print Q70 "Q$q70LineAddress=$q70Address  \t; ${1}2\n";
		$q70LineAddress	= $q70LineAddress + 1;
		$q70Address	= $q70Address + 1;
		
	} else {
		
		print "!!!Skipping line in [${pcsInputFile}]:\n$all_lines[$i]\n"
	}
}

############################## Unused Coordinates ##############################

print Q70 "\n; Unused Coordinate Systems\n\n";

for($i=$PCSnum;$i<=$maxPCS;$i++)
{
	print Q70 "&$i	; Coordinate System\n";
	print Q70 "Q70..99=3540	; Default for non-existent parameters\n";
	print Q70 "\n";
}

############################## Close Output Files ##############################
close(M3300);
close(Q70);
exit;
