# motion.tcl
menuMain "MOTION" "motion.xbm"

#***********************************************************
#  proc menuPMC (pmac)

proc menuPMC {widget bln pmc} {
global medm varyFont errorFile adlPMAC sp
set thisMenu "${widget}.m${pmc}"
${widget} add cascade -label "${bln}${pmc}" -menu ${thisMenu}
menu ${thisMenu}
menuMedm ${thisMenu} "View/Start Servo Clock" ${adlPMAC}PmacClock.adl    pmac=${bln}${pmc}
menuMedm ${thisMenu} "Command String"         ${adlPMAC}PmacCommand.adl  pmac=${bln}${pmc}
return $thisMenu
}

#***********************************************************
#  proc menuMTR (motor)

proc menuMTR {widget bln cmp pmac mtr mtrNo} {
global medm varyFont errorFile adlPMAC adlMTR sp
set thisMenu "${widget}.m${mtr}"
${widget} add cascade -label "${bln}${cmp}${mtr}" -menu ${thisMenu}
menu ${thisMenu}
menuMedm ${thisMenu} "View Status / Jog / Home " ${adlPMAC}MtrStatus.adl  mtr=${bln}${cmp}${mtr},mtrNo=${mtrNo},pmac=${bln}${pmac}
${thisMenu} add separator
menuMedm ${thisMenu} "Modify Range"    ${adlMTR}LoHi1.adl    ax1=${bln}${cmp}${mtr}
${thisMenu} add separator
menuMedm ${thisMenu} "Run Open-loop"   ${adlPMAC}OpnLp.adl      mtr=${bln}${cmp}${mtr},mtrNo=${mtrNo},pmac=${bln}${pmac}
menuMedm ${thisMenu} "Command String"  ${adlPMAC}MtrCommand.adl mtr=${bln}${cmp}${mtr}
${thisMenu} add separator
menuMedm ${thisMenu} "Configure Motion" ${adlPMAC}IxMtrMv.adl  mtr=${bln}${cmp}${mtr}
return $thisMenu
}

#***********************************************************
#  proc menuDRV (drive)

proc menuDRV {widget bln cmp drv} {
global medm varyFont errorFile adlMTR sp
set thisMenu "${widget}.m${drv}"
${widget} add cascade -label "${bln}${cmp}${drv}" -menu ${thisMenu}
menu ${thisMenu}
menuMedm ${thisMenu} "Modify Range" ${adlMTR}LoHi1.adl ax1=${bln}${cmp}${drv}
return $thisMenu
}

#***********************************************************
#  proc menuPCS (pmac coordinate system)

proc menuPCS {widget bln cmp asy dpram} {
global medm varyFont errorFile adlPMAC sp
set thisMenu "${widget}.m${asy}"
${widget} add cascade -label "${bln}${cmp}${asy}" -menu ${thisMenu}
menu ${thisMenu}
menuMedm ${thisMenu} "View Status"        ${adlPMAC}Bckgnd1.adl  pcs=${bln}${cmp}${asy}
${thisMenu} add separator
# menuMedm ${thisMenu} "Program Parameters" ${adlPMAC}MotProgParams.adl assy=${bln}${cmp}${asy}
menuMedm ${thisMenu} "Debug DPRAM"        ${adlPMAC}Ram${dpram}.adl   assy=${bln}${cmp}${asy}
menuMedm ${thisMenu} "Command String"     ${adlPMAC}PcsCommand.adl    pcs=${bln}${cmp}${asy}
return $thisMenu
}

#***********************************************************
#  proc macroCAL (make list for Calibrate screens)

proc macroCAL {bln cmp asy axes drives motors } {

set mac "assy=${bln}${cmp}${asy}"
set x 1
foreach axs $axes {
	lappend mac "x$x=${bln}${cmp}${axs}"
	incr x
}
set x 1
foreach drv $drives {
	lappend mac "d$x=${bln}${cmp}${drv}"
	incr x
}
set x 1
foreach mtr $motors {
	lappend mac "m$x=${bln}${cmp}${mtr}"
	incr x
}
return [join $mac ","]
}

#***********************************************************
#  proc macroMOV (make list for Move screens)

proc macroMOV {bln cmp asy drives } {

set mac "assy=${bln}${cmp}${asy}"
set x 1
foreach drv $drives {
	lappend mac "ax$x=${bln}${cmp}${drv}"
	incr x
}
return [join $mac ","]
}

#***********************************************************
#  proc menuASY (assembly menu)

proc menuASY {widget bln cmp asy calib axes drives motors } {
global medm varyFont errorFile adlMTR sp
set thisMenu "${widget}.mAssy"

${widget} add cascade -label "Assembly" -menu ${thisMenu}
menu ${thisMenu}

set num [llength ${axes}]
if { ${num} > 0 } {
  set macro [macroMOV ${bln} ${cmp} ${asy} ${axes}]
  menuMedm ${thisMenu} "Move Combined Axes"   ${adlMTR}Move${num}_large_limits.adl ${macro}
}

set num [llength ${drives}]
set macro [macroMOV ${bln} ${cmp} ${asy} ${drives}]
  menuMedm ${thisMenu} "Move Drives"   ${adlMTR}Move${num}_large_limits.adl ${macro}

set num [llength ${motors}]
set macro [macroMOV ${bln} ${cmp} ${asy} ${motors}]
menuMedm ${thisMenu} "Move Motors"          ${adlMTR}Move${num}_large_limits.adl ${macro}

set num [llength ${axes}]
if { ${num} > 0 } {
  set num [llength [concat ${axes} ${drives}]]
  set macro [macroMOV ${bln} ${cmp} ${asy} [concat ${axes} ${drives}]]
  menuMedm ${thisMenu} "Move Axes & Drives" ${adlMTR}Move${num}_large_limits.adl ${macro}
}

set num [llength [concat ${drives} ${motors}]]
set macro [macroMOV ${bln} ${cmp} ${asy} [concat ${drives} ${motors}]]
menuMedm ${thisMenu} "Move Drives & Motors" ${adlMTR}Move${num}_large_limits.adl ${macro}

set num [llength ${axes}]
if { ${num} > 0 } {
  set num [llength [concat ${axes} ${drives} ${motors}]]
  set macro [macroMOV ${bln} ${cmp} ${asy} [concat ${axes} ${drives} ${motors}]]
  menuMedm ${thisMenu} "Move everything" ${adlMTR}Move${num}_large_limits.adl ${macro}
}

${thisMenu} add separator

set macro [macroCAL ${bln} ${cmp} ${asy} ${axes} ${drives} ${motors}]
menuMedm ${thisMenu} "Calibrate" ${adlMTR}Calib${calib}.adl  ${macro}
menuMedm ${thisMenu} "Configure" ${adlMTR}Config${calib}.adl ${macro}
}

#***********************************************************
#  proc menuUNT (unit or PMAC assembly menu)

proc menuUNT {widget bln cmp unit} {
set name   [lindex $unit 0]
set asy    [lindex $unit 1]
set calib  [lindex $unit 2]
set pmac   [lindex $unit 3]
set dpram  [lindex $unit 4]
set axes   [lindex $unit 5]
set drives [lindex $unit 6]
set MOTORS [lindex $unit 7]
#puts "${name}"
#puts "${asy}"
#puts "${calib}"
#puts "${pmac}"
#puts "${dpram}"
#puts "${axes}"
#puts "${drives}"
#puts "${MOTORS}"

set motors {}
set motorsNo {}
foreach motor $MOTORS {
        set mtrNo   [lindex $motor 0]
        set mtr     [lindex $motor 1]
	lappend motorsNo "${mtrNo}"
	lappend motors  "${mtr}"
}
#puts "${motorsNo}"
#puts "${motors}"

set thisMenu "${widget}.m${asy}"
${widget} add cascade -label ${name} -menu ${thisMenu}
menu ${thisMenu}

menuASY ${thisMenu} ${bln} ${cmp} ${asy} ${calib} ${axes} ${drives} ${motors}
${thisMenu} add separator
menuPCS ${thisMenu} ${bln} ${cmp} ${asy} ${dpram}
${thisMenu} add separator
set num [llength ${axes}]
if { ${num} > 0 } {
  foreach axs $axes {menuDRV ${thisMenu} ${bln} ${cmp} ${axs}}
}
foreach drv $drives {menuDRV ${thisMenu} ${bln} ${cmp} ${drv}}
${thisMenu} add separator
foreach motor $MOTORS {
        set mtrNo   [lindex $motor 0]
        set mtr     [lindex $motor 1]
#       puts "${mtr} ${mtrNo}"
        menuMTR ${thisMenu} ${bln} ${cmp} ${pmac} ${mtr} ${mtrNo} }
}

#***********************************************************
#  proc menuCMP (components menu: slits, mono, mirror...)

proc menuCMP {widget bln component} {
set name  [lindex $component 0]
set cmp   [lindex $component 1]
set units [lindex $component 2]
set thisMenu "${widget}.m${bln}${cmp}"
${widget} add cascade -label ${name} -menu ${thisMenu}

menu ${thisMenu}
foreach unit $units {menuUNT ${thisMenu} ${bln} ${cmp} ${unit}}
}

#***********************************************************
# Create Motion Menu

set thisMenu .mbMOTION.m
menu ${thisMenu}

foreach component $ComponentList {menuCMP ${thisMenu} ${bln} ${component}}

${thisMenu} add separator
${thisMenu} add cascade -label "PMAC cards" -menu ${thisMenu}.pmac

#***********************************************************
# Create PMAC Submenu
#
set thisMenu .mbMOTION.m.pmac
menu ${thisMenu}

foreach ioc $IocPmacList {
	${thisMenu} add separator
	foreach pmac [lindex $ioc 1] {
		menuPMC ${thisMenu} ${bln} [lindex $pmac 0]
	}
}
