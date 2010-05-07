#!/usr/bin/wish
#
# script for displaying variables/registers of the Turbo PMAC controllers
# author Oleg A. Makarov
# August 8 2003 - initial version
#
### Sergey: include EPICS channel access
set auto_path [linsert $auto_path 0 /usr/local/epics/extensions/lib/$env(HOST_ARCH)]
package require ca

set cards {23d:pmac20: 23d:pmac21:}
set pmac [lindex $cards 0]
set vtypes {I P M M-> &1Q &2Q &3Q &4Q &5Q &6Q &7Q &8Q &9Q &10Q &11Q &12Q &13Q &14Q &15Q &16Q X:$ Y:$ L:$}
set vtype "I"
set f_100K 0
set f_10K  0
set f_1K   0
set f_100  0
set f_10   0
set remLines 0
set startdelay 1000
set repeatdelay 1000
frame .fSelection
frame .fStrings       -relief ridge  -borderwidth 2
frame .fPmac          -relief groove -borderwidth 2
frame .fType          -relief groove -borderwidth 2
frame .fScales        -relief groove -borderwidth 2
foreach card $cards {
	radiobutton 	.rbPmac${card} -text "${card}" \
			-variable pmac -value "${card}" -relief flat
	pack .rbPmac${card} -padx 2m -side top -anchor w -in .fPmac
}
foreach type $vtypes {
	radiobutton 	.rbType${type} -text "${type}" \
			-variable vtype -value "${type}" -relief flat
	pack .rbType${type} -padx 2m -side top -anchor w -in .fType
}
scale .scl_100K       -orient vertical -from 0 -to 0xf -length 9c -tickinterval 1 -width 20
scale .scl_10K        -orient vertical -from 0 -to 0xf -length 9c -tickinterval 1 -width 20
scale .scl_1K         -orient vertical -from 0 -to 8   -length 9c -tickinterval 1 -width 20
scale .scl_100        -orient vertical -from 0 -to 9   -length 9c -tickinterval 1 -width 20
scale .scl_10         -orient vertical -from 0 -to 9   -length 9c -tickinterval 1 -width 20
pack .scl_1K .scl_100 .scl_10 -pady 2m -side left -in .fScales
pack .fPmac .fType .fScales -side left -padx 2m -pady 2m -in .fSelection

foreach i {0 1 2 3 4 5 6 7 8 9 A B C D E F} {
	frame .fLine$i
	label .lVar$i -width 10 -textvariable var$i
	entry .eDec$i -width 20 -textvariable dec$i -relief sunken -bd 2
	entry .eHex$i -width 20 -textvariable hex$i -relief sunken -bd 2
	pack .lVar$i .eDec$i .eHex$i -side left -padx 2m -pady 2m -in .fLine$i
}

bind .eDec0 <Return> {set cmd0 "${out0}${dec0}"; pv put cmd0; set cmd0 "${inp0}"; pv put cmd0}
bind .eDec1 <Return> {set cmd1 "${out1}${dec1}"; pv put cmd1; set cmd1 "${inp1}"; pv put cmd1}
bind .eDec2 <Return> {set cmd2 "${out2}${dec2}"; pv put cmd2; set cmd2 "${inp2}"; pv put cmd2}
bind .eDec3 <Return> {set cmd3 "${out3}${dec3}"; pv put cmd3; set cmd3 "${inp3}"; pv put cmd3}
bind .eDec4 <Return> {set cmd4 "${out4}${dec4}"; pv put cmd4; set cmd4 "${inp4}"; pv put cmd4}
bind .eDec5 <Return> {set cmd5 "${out5}${dec5}"; pv put cmd5; set cmd5 "${inp5}"; pv put cmd5}
bind .eDec6 <Return> {set cmd6 "${out6}${dec6}"; pv put cmd6; set cmd6 "${inp6}"; pv put cmd6}
bind .eDec7 <Return> {set cmd7 "${out7}${dec7}"; pv put cmd7; set cmd7 "${inp7}"; pv put cmd7}
bind .eDec8 <Return> {set cmd8 "${out8}${dec8}"; pv put cmd8; set cmd8 "${inp8}"; pv put cmd8}
bind .eDec9 <Return> {set cmd9 "${out9}${dec9}"; pv put cmd9; set cmd9 "${inp9}"; pv put cmd9}
bind .eDecA <Return> {set cmdA "${outA}${decA}"; pv put cmdA; set cmdA "${inpA}"; pv put cmdA}
bind .eDecB <Return> {set cmdB "${outB}${decB}"; pv put cmdB; set cmdB "${inpB}"; pv put cmdB}
bind .eDecC <Return> {set cmdC "${outC}${decC}"; pv put cmdC; set cmdC "${inpC}"; pv put cmdC}
bind .eDecD <Return> {set cmdD "${outD}${decD}"; pv put cmdD; set cmdD "${inpD}"; pv put cmdD}
bind .eDecE <Return> {set cmdE "${outE}${decE}"; pv put cmdE; set cmdE "${inpE}"; pv put cmdE}
bind .eDecF <Return> {set cmdF "${outF}${decF}"; pv put cmdF; set cmdF "${inpF}"; pv put cmdF}

bind .eHex0 <Return> {set cmd0 "${out0}${hex0}"; pv put cmd0; set cmd0 "${inp0}"; pv put cmd0}
bind .eHex1 <Return> {set cmd1 "${out1}${hex1}"; pv put cmd1; set cmd1 "${inp1}"; pv put cmd1}
bind .eHex2 <Return> {set cmd2 "${out2}${hex2}"; pv put cmd2; set cmd2 "${inp2}"; pv put cmd2}
bind .eHex3 <Return> {set cmd3 "${out3}${hex3}"; pv put cmd3; set cmd3 "${inp3}"; pv put cmd3}
bind .eHex4 <Return> {set cmd4 "${out4}${hex4}"; pv put cmd4; set cmd4 "${inp4}"; pv put cmd4}
bind .eHex5 <Return> {set cmd5 "${out5}${hex5}"; pv put cmd5; set cmd5 "${inp5}"; pv put cmd5}
bind .eHex6 <Return> {set cmd6 "${out6}${hex6}"; pv put cmd6; set cmd6 "${inp6}"; pv put cmd6}
bind .eHex7 <Return> {set cmd7 "${out7}${hex7}"; pv put cmd7; set cmd7 "${inp7}"; pv put cmd7}
bind .eHex8 <Return> {set cmd8 "${out8}${hex8}"; pv put cmd8; set cmd8 "${inp8}"; pv put cmd8}
bind .eHex9 <Return> {set cmd9 "${out9}${hex9}"; pv put cmd9; set cmd9 "${inp9}"; pv put cmd9}
bind .eHexA <Return> {set cmdA "${outA}${hexA}"; pv put cmdA; set cmdA "${inpA}"; pv put cmdA}
bind .eHexB <Return> {set cmdB "${outB}${hexB}"; pv put cmdB; set cmdB "${inpB}"; pv put cmdB}
bind .eHexC <Return> {set cmdC "${outC}${hexC}"; pv put cmdC; set cmdC "${inpC}"; pv put cmdC}
bind .eHexD <Return> {set cmdD "${outD}${hexD}"; pv put cmdD; set cmdD "${inpD}"; pv put cmdD}
bind .eHexE <Return> {set cmdE "${outE}${hexE}"; pv put cmdE; set cmdE "${inpE}"; pv put cmdE}
bind .eHexF <Return> {set cmdF "${outF}${hexF}"; pv put cmdF; set cmdF "${inpF}"; pv put cmdF}

pack .fLine0 .fLine1 .fLine2 .fLine3 .fLine4 .fLine5 .fLine6 .fLine7 \
     .fLine8 .fLine9 -side top -in .fStrings
pack .fSelection .fStrings -side left -padx 2m -pady 2m
update

proc newVal junkVar {
	global 	hexAddr varNum \
		varPre varPost inpPre inpPost outPre outPost \
		var0 var1 var2 var3 var4 var5 var6 var7 var8 var9 varA varB varC varD varE varF \
		inp0 inp1 inp2 inp3 inp4 inp5 inp6 inp7 inp8 inp9 inpA inpB inpC inpD inpE inpF \
		out0 out1 out2 out3 out4 out5 out6 out7 out8 out9 outA outB outC outD outE outF \
		cmd0 cmd1 cmd2 cmd3 cmd4 cmd5 cmd6 cmd7 cmd8 cmd9 cmdA cmdB cmdC cmdD cmdE cmdF

        set val $varNum
	foreach i {0 1 2 3 4 5 6 7 8 9 A B C D E F} {
		switch $hexAddr { 	0 {set xxx $val}
					1 {set xxx [format "%X" $val]}
		}
#		after 50
		set var$i "${varPre}${xxx}${varPost}"
		set inp$i "${inpPre}[set var$i]${inpPost}"
		set out$i "${outPre}[set var$i]${outPost}"
		set cmd$i [set inp$i]; pv put cmd$i; update
		incr val
	}
}
proc newType {name element op} {
	global 	hexAddr varNum remLines \
		varPre varPost inpPre inpPost outPre outPost \
		rsp0 rsp1 rsp2 rsp3 rsp4 rsp5 rsp6 rsp7 rsp8 rsp9 rspA rspB rspC rspD rspE rspF
	upvar $name vt
	switch $vt {   "M->" { set hexAddr 0
		               set varPre "M"; set varPost "->"
		               set inpPre "";  set inpPost ""
		               set outPre "";  set outPost ""
		       }
		       "X:$" -
		       "Y:$" -
		       "L:$" { set hexAddr 1
		               set varPre "$vt"; set varPost ""
		               set inpPre "R";   set inpPost ""
		               set outPre "W";   set outPost ","
		       }
		       default { set hexAddr 0
		                 set varPre "$vt"; set varPost ""
		                 set inpPre "";    set inpPost ""
		                 set outPre "";    set outPost "="
		       }
	}
	switch $hexAddr {
		0 {	  if [expr $remLines] {
			  	  set remLines 0
				  pack forget  .fLine0 .fLine1 .fLine2 .fLine3 .fLine4. fLine5 .fLine6 .fLine7 \
					       .fLine8 .fLine9 .fLineA .fLineB .fLineC .fLineD .fLineE .fLineF .fStrings \
					       .scl_100K .scl_10K .scl_1K .scl_100 .scl_10 .fScales
			  	  set f_100K 0
			  	  set f_10K  0
			  	  .scl_1K  configure -to 8 -tickinterval 1
			  	  .scl_100 configure -to 9 -tickinterval 1
			  	  .scl_10  configure -to 9 -tickinterval 1
				  chgVarNum 0
			          pack .fLine0 .fLine1 .fLine2 .fLine3 .fLine4 \
			               .fLine5 .fLine6 .fLine7 .fLine8 .fLine9 -side top -in .fStrings
			          pack .scl_1K .scl_100 .scl_10 -pady 2m -side left -in .fScales
			  }
		}
		default {
			  if [expr $remLines]==0 {
				  pack forget  .fLine0 .fLine1 .fLine2 .fLine3 .fLine4. fLine5 .fLine6 .fLine7 \
					       .fLine8 .fLine9 .fStrings \
					       .scl_1K .scl_100 .scl_10 .fScales
				  .scl_1K  configure -to 0xf -tickinterval 1
				  .scl_100 configure -to 0xf -tickinterval 1
				  .scl_10  configure -to 0xf -tickinterval 1
				  chgVarNum 0
				  pack .fLine0 .fLine1 .fLine2 .fLine3 .fLine4 .fLine5 .fLine6 .fLine7 \
				       .fLine8 .fLine9 .fLineA .fLineB .fLineC .fLineD .fLineE .fLineF -side top -in .fStrings
				  pack .scl_100K .scl_10K .scl_1K .scl_100 .scl_10 -pady 2m -side left -in .fScales
			  }
			  set remLines 1
		}
	}
	newVal 0
	pack .fPmac .fType .fScales -side left -padx 2m -pady 2m -in .fSelection
	pack .fSelection .fStrings -side left -padx 2m -pady 2m
}

proc response_Post {rsp dec hex} {
	upvar $dec d
	upvar $hex h
	set d $rsp
	if [string is integer $d]&&([string length $d]>0) {set h [format "$%X" $d]} else {set h ""}
	if [string match "\$*" $d]  {set h $d; set d "see Hex ->"}
	if [string match "*\:*" $d] {set h "<-- var definition"}
}

proc newPmac {name element op} {
	global	startdelay vtype pvStatus \
		cmd0 cmd1 cmd2 cmd3 cmd4 cmd5 cmd6 cmd7 cmd8 cmd9 cmdA cmdB cmdC cmdD cmdE cmdF \
		rsp0 rsp1 rsp2 rsp3 rsp4 rsp5 rsp6 rsp7 rsp8 rsp9 rspA rspB rspC rspD rspE rspF \
		dec0 dec1 dec2 dec3 dec4 dec5 dec6 dec7 dec8 dec9 decA decB decC decD decE decF \
		hex0 hex1 hex2 hex3 hex4 hex5 hex6 hex7 hex8 hex9 hexA hexB hexC hexD hexE hexF
	upvar $name pmac

	foreach i {0 1 2 3 4 5 6 7 8 9 A B C D E F} {
		pv cmon rsp$i; set psp$i "N/A"; set dec$i "N/A"; set hex$i "N/A"; update
		pv link rsp$i "${pmac}TclRsp$i"; update
		pv link cmd$i "${pmac}TclCmd$i"; update
	}

	set num 0
	while 1 {
		after ${startdelay}
		set rdy 0
		incr num
		foreach i {0 1 2 3 4 5 6 7 8 9 A B C D E F} {
			pv stat cmd$i pvStatus; update
			if [string match ${pvStatus(state)} "OK"] {incr rdy;}
			pv stat rsp$i pvStatus; update
			if [string match ${pvStatus(state)} "OK"] \
				{incr rdy; pv get rsp$i; response_Post "[set rsp$i]" dec$i hex$i}
		}
		if (($rdy>31)||($num>10)) break
	}

	pv umon rsp0 {response_Post $rsp0 dec0 hex0}
	pv umon rsp1 {response_Post $rsp1 dec1 hex1}
	pv umon rsp2 {response_Post $rsp2 dec2 hex2}
	pv umon rsp3 {response_Post $rsp3 dec3 hex3}
	pv umon rsp4 {response_Post $rsp4 dec4 hex4}
	pv umon rsp5 {response_Post $rsp5 dec5 hex5}
	pv umon rsp6 {response_Post $rsp6 dec6 hex6}
	pv umon rsp7 {response_Post $rsp7 dec7 hex7}
	pv umon rsp8 {response_Post $rsp8 dec8 hex8}
	pv umon rsp9 {response_Post $rsp9 dec9 hex9}
	pv umon rspA {response_Post $rspA decA hexA}
	pv umon rspB {response_Post $rspB decB hexB}
	pv umon rspC {response_Post $rspC decC hexC}
	pv umon rspD {response_Post $rspD decD hexD}
	pv umon rspE {response_Post $rspE decE hexE}
	pv umon rspF {response_Post $rspF decF hexF}
	newType vtype "" "w"
}
proc chgVarNum junkVar {
	global	timedelay hexAddr f_100K f_10K f_1K f_100 f_10 varNum
	switch $hexAddr {0 	 {set varNum [expr $f_100K*  100000 + $f_10K*  10000 + $f_1K*  1000 + $f_100*  100 + $f_10*  10]}
			 default {set varNum [expr $f_100K*0x100000 + $f_10K*0x10000 + $f_1K*0x1000 + $f_100*0x100 + $f_10*0x10]}
	}
	newVal 0
}

set varNum 0
newPmac pmac "" "w"
.scl_100K 	configure -variable f_100K 	-command chgVarNum
.scl_10K 	configure -variable f_10K 	-command chgVarNum
.scl_1K 	configure -variable f_1K 	-command chgVarNum
.scl_100 	configure -variable f_100 	-command chgVarNum
.scl_10 	configure -variable f_10 	-command chgVarNum
trace variable vtype w newType
trace variable pmac w newPmac
while 1 {update; after ${repeatdelay}
       newVal 0
}
