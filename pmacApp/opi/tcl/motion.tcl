# motion_staff.tcl
menuMain "MOTION" "motion.xbm"

#***********************************************************
#  proc menuPMC (pmac)

proc menuPMC {widget bln pmc} {
  global medm varyFont errorFile adlPMAC adlSTD tclGMCA Beamline burtSTD acc65eList xterm sp
  set thisMenu "${widget}.m${pmc}"
  ${widget} add cascade -label "${bln}${pmc}" -menu ${thisMenu}
  menu ${thisMenu}
  menuMedm ${thisMenu} "View/Start Servo Clock"    ${adlPMAC}PmacClock.adl    pmac=${bln}${pmc}
  menuMedm ${thisMenu} "Command String"            ${adlPMAC}PmacCommand.adl  pmac=${bln}${pmc}
  menuMedm ${thisMenu} "Tcl/Tk Strings"            ${adlPMAC}PmacTcl.adl      pmac=${bln}${pmc}

  set acc65e [ lsearch $acc65eList "${pmc}" ]
  if {$acc65e != -1} {
    ${thisMenu} add separator
#   menuMedm ${thisMenu} "Digital I/O"                   ${adlPMAC}Acc_11E.adl           pmac=${bln}${pmc}
    menuMedm ${thisMenu} "Digital I/O (single word)"     ${adlPMAC}Acc_65E.adl           pmac=${bln}${pmc}
    menuMedm ${thisMenu} "Digital I/O (individual bits)" ${adlPMAC}digital_IO_pmac24.adl  P=${bln}${pmc}acc65e:,BL=${Beamline},SAVE_DIR=.
  }

return $thisMenu
}

#***********************************************************
#  proc menuMTR (motor)

proc menuMTR {widget bln cmp pmac mtr mtrNo} {
  global medm varyFont errorFile adlPMAC adlMTR brushlessList geodrivesList sp
  set thisMenu "${widget}.m${mtr}"
  ${widget} add cascade -label "${bln}${cmp}${mtr} (${pmac} mtr${mtrNo})" -menu ${thisMenu}
  menu ${thisMenu}
  set brushless [ lsearch $brushlessList "${cmp}${mtr}" ]
  set geodrive  [ lsearch $geodrivesList "${cmp}${mtr}" ]
  if {$geodrive != -1} {
    set geonode [lindex $geodrivesList [expr ${geodrive} + 1]]
  } else {
    set geonode -1
  }
# puts "${cmp}${mtr} brushless=${brushless} geo=${geodrive} node=${geonode}"
  if       { $brushless != -1 && $geodrive != -1 } {
    menuMedm ${thisMenu} "View Status / Jog / Home brushless-geo" ${adlPMAC}MtrStatus_brushgeo.adl mtr=${bln}${cmp}${mtr},mtrNo=${mtrNo},pmac=${bln}${pmac},node=${geonode}
  } elseif { $brushless != -1 && $geodrive == -1 } {
    menuMedm ${thisMenu} "View Status / Jog / Home brushless"     ${adlPMAC}MtrStatus_brush.adl    mtr=${bln}${cmp}${mtr},mtrNo=${mtrNo},pmac=${bln}${pmac},node=${geonode}
  } elseif { $brushless == -1 && $geodrive != -1 } {
    menuMedm ${thisMenu} "View Status / Jog / Home geodrive"      ${adlPMAC}MtrStatus_geo.adl      mtr=${bln}${cmp}${mtr},mtrNo=${mtrNo},pmac=${bln}${pmac},node=${geonode}
  } else {
    menuMedm ${thisMenu} "View Status / Jog / Home motor"         ${adlPMAC}MtrStatus.adl          mtr=${bln}${cmp}${mtr},mtrNo=${mtrNo},pmac=${bln}${pmac},node=${geonode}
  }
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

proc menuDRV {widget bln cmp drv comment} {
  global medm varyFont errorFile adlMTR sp
  set thisMenu "${widget}.m${drv}"
  ${widget} add cascade -label "${bln}${cmp}${drv} ${comment}" -menu ${thisMenu}
  menu ${thisMenu}
  menuMedm ${thisMenu} "Modify Range" ${adlMTR}LoHi1.adl ax1=${bln}${cmp}${drv}
  return $thisMenu
}

#***********************************************************
#  proc menuPCS (pmac coordinate system)

proc menuPCS {widget bln cmp asy dpram comment} {
  global medm varyFont errorFile adlPMAC sp
  set thisMenu "${widget}.m${asy}"
  ${widget} add cascade -label "${bln}${cmp}${asy} ${comment}" -menu ${thisMenu}
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
#
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
#
  set mac "assy=${bln}${cmp}${asy}"
  set x 1
  foreach drv $drives {
	lappend mac "ax$x=${bln}${cmp}${drv}"
	incr x
  }
  return [join $mac ","]
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
# puts "${name}"
# puts "${asy}"
# puts "${calib}"
# puts "${pmac}"
# puts "${dpram}"
# puts "${axes}"
# puts "${drives}"
# puts "${MOTORS}"
  global medm varyFont errorFile adlMTR sp

  set motors {}
  set motorsNo {}
  foreach motor $MOTORS {
        set mtrNo   [lindex $motor 0]
        set mtr     [lindex $motor 1]
	lappend motorsNo "${mtrNo}"
	lappend motors  "${mtr}"
  }
# puts "${motorsNo}"
# puts "${motors}"

  set thisMenu "${widget}.m${asy}"
  ${widget} add cascade -label ${name} -menu ${thisMenu}
  menu ${thisMenu}

# menuASY ${thisMenu} ${bln} ${cmp} ${asy} ${calib} ${axes} ${drives} ${motors}
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
## proc menuASY {widget bln cmp asy calib axes drives motors } {
##   global medm varyFont errorFile adlMTR sp
##   set thisMenu "${widget}.mAssy"
##
##   ${widget} add cascade -label "Assembly" -menu ${thisMenu}
##   menu ${thisMenu}

  set num [llength ${axes}]
  if {$num > 0} {
    set macro [macroMOV ${bln} ${cmp} ${asy} ${axes}]
    menuMedm ${thisMenu} "Move Combined Axes"   ${adlMTR}Move${num}_large_limits.adl ${macro}
  }

  set num [llength ${drives}]
  set macro [macroMOV ${bln} ${cmp} ${asy} ${drives}]
  menuMedm ${thisMenu} "Move Drives"   ${adlMTR}Move${num}_large_limits.adl ${macro}
# menuMedm ${thisMenu} "Move Drives (medium)" ${adlMTR}Move${num}_medium_limits.adl ${macro}
# menuMedm ${thisMenu} "Move Drives (short)"  ${adlMTR}Move${num}_short_limits.adl ${macro}

  set num [llength ${motors}]
  set macro [macroMOV ${bln} ${cmp} ${asy} ${motors}]
  menuMedm ${thisMenu} "Move Motors"          ${adlMTR}Move${num}_large_limits.adl ${macro}

  set num [llength ${axes}]
  if {$num > 0} {
    set num [llength [concat ${axes} ${drives}]]
    set macro [macroMOV ${bln} ${cmp} ${asy} [concat ${axes} ${drives}]]
    menuMedm ${thisMenu} "Move Axes & Drives" ${adlMTR}Move${num}_large_limits.adl ${macro}
  }

  set num [llength [concat ${drives} ${motors}]]
  set macro [macroMOV ${bln} ${cmp} ${asy} [concat ${drives} ${motors}]]
  menuMedm ${thisMenu} "Move Drives & Motors" ${adlMTR}Move${num}_large_limits.adl ${macro}

  set num [llength ${axes}]
  if {$num > 0} {
    set num [llength [concat ${axes} ${drives} ${motors}]]
    set macro [macroMOV ${bln} ${cmp} ${asy} [concat ${axes} ${drives} ${motors}]]
    menuMedm ${thisMenu} "Move everything" ${adlMTR}Move${num}_large_limits.adl ${macro}
  }

  ${thisMenu} add separator

  set macro [macroCAL ${bln} ${cmp} ${asy} ${axes} ${drives} ${motors}]
  if {$cmp == "CCD:"} {
     if       {$asy == "St:"} {
	         append macro ",VFM_angle=${bln}KB:VA:"
     } elseif {$asy == "Lp:"} {
	         append macro ",HFM_angle=${bln}KB:HA:"
	         append macro ",assySt=${bln}${cmp}St:"
	         append macro ",x1St=${bln}${cmp}D:"
	         append macro ",x2St=${bln}${cmp}2Q:"
     }
  }
  menuMedm ${thisMenu} "Calibrate" ${adlMTR}Calib${calib}.adl  ${macro}
  menuMedm ${thisMenu} "Configure" ${adlMTR}Config${calib}.adl ${macro}
  if {$cmp == "COL:" && $asy == "St:"} {
    append macro ",ax1=${bln}${cmp}V:"
    menuMedm ${thisMenu} "Setup In/Out" ${adlMTR}SetupCOLSt.adl ${macro}
  }
## }
#~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
#
  ${thisMenu} add separator
  menuPCS ${thisMenu} ${bln} ${cmp} ${asy} ${dpram} "(coord.sys.)"
  set num [llength ${axes}]
  if {$num > 0} {
    ${thisMenu} add separator
    foreach axs $axes {menuDRV ${thisMenu} ${bln} ${cmp} ${axs} "(comb.axis)"}
  }
  ${thisMenu} add separator
  foreach drv $drives {menuDRV ${thisMenu} ${bln} ${cmp} ${drv} "(drive)"}
  ${thisMenu} add separator
  foreach motor $MOTORS {
      set mtrNo   [lindex $motor 0]
      set mtr     [lindex $motor 1]
#     puts "${mtr} ${mtrNo}"
      menuMTR ${thisMenu} ${bln} ${cmp} ${pmac} ${mtr} ${mtrNo}
  }
}

#***********************************************************
#  proc menuCMP (components menu: slits, mono, mirror...)

proc menuCMP {widget bln component} {
  global adlMTR ID Beamline
  set name  [lindex $component 0]
  set cmp   [lindex $component 1]
  set units [lindex $component 2]

  set thisMenu "${widget}.m${bln}${cmp}"
  ${widget} add cascade -label ${name} -menu ${thisMenu}

  menu ${thisMenu}

if {$cmp == "MO:"} {
  if {$ID != "none"} {
    menuMedm ${thisMenu} "Everything"              ${adlMTR}Mono.adl assy=${bln}${cmp},xx=${ID}
    ${thisMenu} add separator
  } else {
    menuMedm ${thisMenu} "Everything"              ${adlMTR}MonoBM.adl assy=${bln}${cmp}
    ${thisMenu} add separator
  }
} elseif {$cmp == "HD:"} {
    menuMedm ${thisMenu} "Horiz.Deflecting Mirrors Everything" ${adlMTR}MirrorHDM.adl assy=${bln}${cmp}
    ${thisMenu} add separator
} elseif {$cmp == "KB:"} {
    menuMedm ${thisMenu} "Horiz.Mirror Everything" ${adlMTR}MirrorHKB.adl assy=${bln}${cmp}
    menuMedm ${thisMenu} "Vert. Mirror Everything" ${adlMTR}MirrorVKB.adl assy=${bln}${cmp}
    ${thisMenu} add separator
} elseif {$cmp == "WS:"} {
    menuMedm ${thisMenu} "Everything"              ${adlMTR}Slits.adl assy=${bln}${cmp},txt=White
    ${thisMenu} add separator
} elseif {$cmp == "CS:"} {
    menuMedm ${thisMenu} "Everything"              ${adlMTR}Slits.adl assy=${bln}${cmp},txt=Monochromatic
    ${thisMenu} add separator
} elseif {$cmp == "GS:"} {
    menuMedm ${thisMenu} "Everything"              ${adlMTR}Slits.adl assy=${bln}${cmp},txt=Guard
    ${thisMenu} add separator
} elseif {$cmp == "BD:"} {
    menuMedm ${thisMenu} "Everything"              ${adlMTR}Delivery.adl assy=${bln}${cmp}
    ${thisMenu} add separator
} elseif {$cmp == "GO:"} {
    menuMedm ${thisMenu} "Everything, no  Kappa" ${adlMTR}GonioOCS.adl  assy=${bln}${cmp},Beamline=${Beamline}
    menuMedm ${thisMenu} "Everything with Kappa" ${adlMTR}GonioOKCS.adl assy=${bln}${cmp},Beamline=${Beamline}
    ${thisMenu} add separator
} elseif {$cmp == "SH:"} {
    menuMedm ${thisMenu} "Shutter Control"         ${adlMTR}Shutter.adl   mtr=${bln}${cmp}mp:,assy=${bln}${cmp}Ps:
    ${thisMenu} add separator
} elseif {$cmp == "HS:"} {
    menuMedm ${thisMenu} "Everything"              ${adlMTR}Slits.adl assy=${bln}${cmp},txt=Huber
    ${thisMenu} add separator
}

  foreach unit $units {menuUNT ${thisMenu} ${bln} ${cmp} ${unit}}
}

#***********************************************************
#  proc menuBURT (backup/restore menu)

proc menuBURT {widget} {
  global Beamline myhome

  set thisMenu "${widget}.mburt"
  ${widget} add cascade -label "Backup/Restore positions" -menu ${thisMenu}
}

#***********************************************************
# Create Motion Menu

set thisMenu .mbMOTION.m
menu ${thisMenu}

foreach component $ComponentList {menuCMP ${thisMenu} ${bln} ${component}}

if { ${SYSTEM} == "LINUX" } {
  if { $HOST == "px0" || $HOST == "www" \
    || $HOST == "bl1ws3" || $HOST == "bl1dl380lower" || $HOST == "bl1dl380upper" \
    || $HOST == "bl2ws3" || $HOST == "bl2dl380lower" || $HOST == "bl2dl380upper" } {
    ${thisMenu} add separator
    ${thisMenu} add command -label "Calibration Save/Restore" \
       -command "exec ${et} -f ${tclGMCA}CalibSave.tcl ${Beamline} $errorFile & "
  }
}

${thisMenu} add separator
${thisMenu} add cascade -label "PMAC cards" -menu ${thisMenu}.pmac

#***********************************************************
# Create PMAC Submenu
#
set thisMenu .mbMOTION.m.pmac
menu ${thisMenu}

foreach ioc $IocPmacList {
	foreach pmac [lindex $ioc 1] {
		menuPMC ${thisMenu} ${bln} [lindex $pmac 0]
	}
	${thisMenu} add separator
}
