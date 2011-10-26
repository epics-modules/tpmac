# motion_staff.tcl
menuMain "MOTION" "motion.xbm"

#***********************************************************
#  proc menuPMC (pmac)

proc menuPMC {widget bln pmc} {
  global medm varyFont errorFile adlPMAC adlSTD adlGMCA tclGMCA2 Beamline burtSTD acc65eList acc59eList xterm sp
  set thisMenu "${widget}.m${pmc}"
  ${widget} add cascade -label "${bln}${pmc}" -menu ${thisMenu}
  menu ${thisMenu}
  menuMedm ${thisMenu} "View/Start Servo Clock"    ${adlPMAC}PmacClock.adl    pmac=${bln}${pmc}
  menuMedm ${thisMenu} "Command String"            ${adlPMAC}PmacCommand.adl  pmac=${bln}${pmc}
# menuMedm ${thisMenu} "Tcl/Tk Strings"            ${adlPMAC}PmacTcl.adl      pmac=${bln}${pmc}
  ${thisMenu} add separator
  if { $bln == "23i:" && $pmc == "pmac10:"} {
    menuMedm ${thisMenu} "RON905_1" ${adlPMAC}Channel32.adl    pmac=${bln}${pmc},mtr=${bln}${pmc}re1:,mtrNo=13,node=-1
    menuMedm ${thisMenu} "RON905_2" ${adlPMAC}Channel32.adl    pmac=${bln}${pmc},mtr=${bln}${pmc}re2:,mtrNo=14,node=-1
    menuMedm ${thisMenu} "RON905_3" ${adlPMAC}Channel32.adl    pmac=${bln}${pmc},mtr=${bln}${pmc}re3:,mtrNo=15,node=-1
    menuMedm ${thisMenu} "RON905_4" ${adlPMAC}Channel32.adl    pmac=${bln}${pmc},mtr=${bln}${pmc}re4:,mtrNo=16,node=-1
    menuMedm ${thisMenu} "RON905_S" ${adlPMAC}Channel32.adl    pmac=${bln}${pmc},mtr=${bln}${pmc}reS:,mtrNo=11,node=-1
    ${thisMenu} add separator
  }
  if { $bln == "23o:" && $pmc == "pmac10:"} {
    menuMedm ${thisMenu} "RON905_1" ${adlPMAC}Channel32.adl    pmac=${bln}${pmc},mtr=${bln}${pmc}re1:,mtrNo=13,node=-1
    menuMedm ${thisMenu} "RON905_2" ${adlPMAC}Channel32.adl    pmac=${bln}${pmc},mtr=${bln}${pmc}re2:,mtrNo=14,node=-1
    menuMedm ${thisMenu} "RON905_3" ${adlPMAC}Channel32.adl    pmac=${bln}${pmc},mtr=${bln}${pmc}re3:,mtrNo=15,node=-1
    menuMedm ${thisMenu} "RON905_4" ${adlPMAC}Channel32.adl    pmac=${bln}${pmc},mtr=${bln}${pmc}re4:,mtrNo=16,node=-1
    menuMedm ${thisMenu} "RON905_S" ${adlPMAC}Channel32.adl    pmac=${bln}${pmc},mtr=${bln}${pmc}sm12:,mtrNo=12,node=-1
    ${thisMenu} add separator
  }
  if { $bln == "23b:" && $pmc == "pmac10:"} {
    menuMedm ${thisMenu} "RON905_1" ${adlPMAC}Channel32.adl    pmac=${bln}${pmc},mtr=${bln}${pmc}re1:,mtrNo=25,node=-1
    menuMedm ${thisMenu} "RON905_2" ${adlPMAC}Channel32.adl    pmac=${bln}${pmc},mtr=${bln}${pmc}re2:,mtrNo=26,node=-1
    menuMedm ${thisMenu} "RON905_3" ${adlPMAC}Channel32.adl    pmac=${bln}${pmc},mtr=${bln}${pmc}re3:,mtrNo=27,node=-1
    menuMedm ${thisMenu} "RON905_4" ${adlPMAC}Channel32.adl    pmac=${bln}${pmc},mtr=${bln}${pmc}re4:,mtrNo=28,node=-1
    menuMedm ${thisMenu} "RON905_S" ${adlPMAC}Channel32.adl    pmac=${bln}${pmc},mtr=${bln}${pmc}reS:,mtrNo=31,node=-1
    ${thisMenu} add separator
  }
  if { $bln == "23b:" && ($pmc == "pmac10:" ||$pmc == "pmac11:") } {
     menuMedm ${thisMenu} "Motor#16 View/Configure" ${adlPMAC}Channel32.adl    pmac=${bln}${pmc},mtr=${bln}${pmc}mo:,mtrNo=16,node=-1
     set optcmd "exec ${xterm} ${tclGMCA2}pezca23id unlink32.pl ${pmc} 16 &"
     ${thisMenu} add command -label "Motor#16 Unlink from PCS" -command "${optcmd}"
  } else {
     menuMedm ${thisMenu} "Motor#32 View/Configure" ${adlPMAC}Channel32.adl    pmac=${bln}${pmc},mtr=${bln}${pmc}mo:,mtrNo=32,node=-1
     set optcmd "exec ${xterm} ${tclGMCA2}pezca23id unlink32.pl ${pmc} 32 &"
     ${thisMenu} add command -label "Motor#32 Unlink from PCS" -command "${optcmd}"
  }

  if { [ info exists acc59eList ] } {
    set acc59e [ lsearch $acc59eList "${pmc}" ]
    if {$acc59e != -1} {
      regsub {:$} $pmc {} p
      menuMedm ${thisMenu} "Acc 59e DAC"          ${adlPMAC}8_DACs_comment.adl      P=${bln}${pmc}59e,BL=${Beamline},SAVE_DIR=${burtSTD},PMAC=${p}
      menuMedm ${thisMenu} "Acc 59e ADC"          ${adlPMAC}8_ADCs_comment.adl      P=${bln}${pmc}59e,BL=${Beamline},SAVE_DIR=${burtSTD},PMAC=${p}
    }
  }
  if { [ info exists acc65eList ] } {
    set acc65e [ lsearch $acc65eList "${pmc}" ]
    if {$acc65e != -1} {
###   Remove ":" at the end of "pmacXX:" and place the stripped string into $p:
      regsub {:$} $pmc {} p
      ${thisMenu} add separator
#     menuMedm ${thisMenu} "Digital I/O"                   ${adlGMCA}Acc_11E.adl           pmac=${bln}${pmc}
      menuMedm ${thisMenu} "Digital I/O (single word)"     ${adlGMCA}Acc_65E.adl           pmac=${bln}${pmc}
      menuMedm ${thisMenu} "Digital I/O (individual bits)" ${adlSTD}digital_IO_pmac24.adl  P=${bln}${pmc}acc65e,BL=${Beamline},SAVE_DIR=${burtSTD},PMAC=${p}
    }
  }
  ${thisMenu} add separator
# Remove ":" at the end of "pmacXX:" and place the stripped string into $p:
  regsub {:$} $pmc {} p
  set optcmd "exec ${xterm} ${tclGMCA2}pezca23id mtrBackup.pl ${p} 32 NONINTERACTIVE &"
# set optcmd "exec ${xterm} ${tclGMCA2}pezca23id mtrBackup.pl ${p} 32 &"
  ${thisMenu} add command -label "Dump all motors parametes" -command "${optcmd}"

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
  menuMedm ${thisMenu} "View All Motor Status Bits" ${adlPMAC}MtrServoStatus.adl mtr=${bln}${cmp}${mtr},mtrNo=${mtrNo},pmac=${bln}${pmac}
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
  menuMedm ${thisMenu} "View Detailed Status" ${adlPMAC}PrgExeStatus.adl  pcs=${bln}${cmp}${asy}
  menuMedm ${thisMenu} "View Short Status"    ${adlPMAC}Bckgnd1.adl       pcs=${bln}${cmp}${asy}
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
  global medm varyFont errorFile adlMTR adlGMCA Beamline sp SYSTEM

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

# if       { [regexp {pmac1[0-9]} $pmac] != 0 } {
#   set ioc "${bln}ioc1"
# } elseif { [regexp {pmac2[0-9]} $pmac] != 0 } {
#   set ioc "${bln}ioc2"
# } elseif { [regexp {pmac3[0-9]} $pmac] != 0 } {
#   set ioc "${bln}ioc3"
# } else {
#   set ioc "unknown"
# }
  set ioc ${bln}ioc[string index $pmac 4]
  append macro ",ioc=${ioc}"

  if {$cmp == "CCD:"} {
     if       {$asy == "St:"} {
	         append macro ",VFM_angle=${bln}KB:VA:"
     } elseif {$asy == "Lp:"} {
	         append macro ",HFM_angle=${bln}KB:HA:"
	         append macro ",assySt=${bln}${cmp}St:"
	         append macro ",x1St=${bln}${cmp}D:"
	         append macro ",x2St=${bln}${cmp}2Q:"
     }
  } elseif {$cmp == "SH:" && $asy == "Ps:"} {
     global shutterOpenedMvar shutterClosedMvar
	         append macro ",MvarOpened=${shutterOpenedMvar}"
	         append macro ",MvarClosed=${shutterClosedMvar}"
  }
  menuMedm ${thisMenu} "Calibrate" ${adlMTR}Calib${calib}.adl  ${macro}
  menuMedm ${thisMenu} "Configure" ${adlMTR}Config${calib}.adl ${macro}
  if {$cmp == "COL:" && $asy == "St:"} {
    append macro ",ax1=${bln}${cmp}H:,ax2=${bln}${cmp}V:,pre=${Beamline}:bi:,header=Collimator Positions Presets"
    menuMedm ${thisMenu} "Setup Presets" ${adlMTR}Setup_5presets_collimator.adl ${macro}
### See this menu in main.tcl:
    menuCollimatorPresetsBURT ${thisMenu}
  } elseif {$cmp == "BS:" && $asy == "Ps:"} {
    append macro ",ax1=${bln}${cmp}PT:,header=BeamStop In/Out Setup"
    menuMedm ${thisMenu} "Setup In/Out" ${adlMTR}SetupInOut.adl ${macro}
    regsub ",ax1=.*" $macro ",ax1=${bln}${cmp}D:,name=critical,header=BeamStop Collision Distance Setup" macro
    menuMedm ${thisMenu} "Setup Collision Distance" ${adlMTR}SetupCollisionDistance.adl ${macro}
  } elseif {$cmp == "PIN:" && $asy == "St:"} {
    append macro ",ax1=${bln}${cmp}P:,header=Pin Diode In/Out Setup"
    menuMedm ${thisMenu} "Setup In/Out" ${adlMTR}SetupInOut.adl ${macro}
  } elseif {$cmp == "BP:" && $asy == "Mo:"} {
    append macro ",ax1=${bln}${cmp}MOZ:,header=Mono BPM presets"
    menuMedm ${thisMenu} "Setup Position Presets" ${adlMTR}Setup_3presets.adl ${macro}
  } elseif {$cmp == "BP:" && $asy == "Hd1:"} {
    append macro ",ax1=${bln}${cmp}H1Z:,header=HDM1 BPM presets"
    menuMedm ${thisMenu} "Setup Position Presets" ${adlMTR}Setup_4presets.adl ${macro}
  } elseif {$cmp == "BP:" && $asy == "Hd3:"} {
    append macro ",ax1=${bln}${cmp}H3Z:,header=HDM1a BPM presets"
    menuMedm ${thisMenu} "Setup Position Presets" ${adlMTR}Setup_4presets.adl ${macro}
  } elseif {$cmp == "BP:" && $asy == "Kb:"} {
    append macro ",ax1=${bln}${cmp}KBZ:,header=KBM BPM presets"
    menuMedm ${thisMenu} "Setup Position Presets" ${adlMTR}Setup_3presets.adl ${macro}
  } elseif {$cmp == "TV:" && $asy == "V1:"} {
    menuMedm ${thisMenu} "Zoom to Backlight calibration" \
             ${adlGMCA}zoom2backlightCalibration.adl P=${bln},C=${cmp}${asy}lightcalc,SDIS=${cmp}${asy}lightSDIS,ioc=${ioc}
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
  global adlMTR adlGMCA ID Beamline
  set name  [lindex $component 0]
  set cmp   [lindex $component 1]
  set units [lindex $component 2]

  set thisMenu "${widget}.m${bln}${cmp}"
  ${widget} add cascade -label ${name} -menu ${thisMenu}

  menu ${thisMenu}

  if {$cmp == "MO:"} {
     if {$ID != "none"} {
        menuMedm ${thisMenu} "Everything"           ${adlMTR}Mono.adl assy=${bln}${cmp},xx=${ID}
        ${thisMenu} add separator
     } else {
        menuMedm ${thisMenu} "Everything"           ${adlMTR}MonoBM.adl assy=${bln}${cmp}
        ${thisMenu} add separator
     }
  } elseif {$cmp == "HD:"} {
     menuMedm ${thisMenu} "Horiz.Deflecting Mirrors Everything" ${adlMTR}MirrorHDM.adl assy=${bln}${cmp}
     ${thisMenu} add separator
     menuMedm ${thisMenu} "Kill / Enable KBM and HDM motors" ${adlGMCA}kill_kbm_hdm.adl P=${bln}
     ${thisMenu} add separator
  } elseif {$cmp == "KB:"} {
     menuMedm ${thisMenu} "Horiz.Mirror Everything" ${adlMTR}MirrorHKB.adl assy=${bln}${cmp}
     menuMedm ${thisMenu} "Vert. Mirror Everything" ${adlMTR}MirrorVKB.adl assy=${bln}${cmp}
     ${thisMenu} add separator
     if { $bln == "23i:" } {
        menuMedm ${thisMenu} "Kill / Enable KBM motors"         ${adlGMCA}kill_kbm.adl P=${bln}
     } elseif { $bln == "23o:" } {
        menuMedm ${thisMenu} "Kill / Enable KBM and HDM motors" ${adlGMCA}kill_kbm_hdm.adl P=${bln}
     }
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
     if { $bln != "23b:" } {
        menuMedm ${thisMenu} "Everything, no  Kappa" ${adlMTR}GonioOCS_ID.adl  assy=${bln}${cmp},Beamline=${Beamline},st=St
#       menuMedm ${thisMenu} "Everything with Kappa" ${adlMTR}GonioOKCS_ID.adl assy=${bln}${cmp},Beamline=${Beamline},st=St
     } else {
        menuMedm ${thisMenu} "Everything, no  Kappa" ${adlMTR}GonioOCS_BM.adl  assy=${bln}${cmp},Beamline=${Beamline},st=StB
#       menuMedm ${thisMenu} "Everything with Kappa" ${adlMTR}GonioOKCS_BM.adl assy=${bln}${cmp},Beamline=${Beamline},st=StB
     }
     ${thisMenu} add separator
  } elseif {$cmp == "RB:"} {
     menuMedm ${thisMenu} "Everything"               ${adlMTR}Robot.adl assy=${bln}${cmp}
     ${thisMenu} add separator
  } elseif {$cmp == "SH:"} {
     menuMedm ${thisMenu} "Shutter Control"          ${adlMTR}Shutter.adl   mtr=${bln}${cmp}mp:,assy=${bln}${cmp}Ps:,BL=${bln}
     ${thisMenu} add separator
  } elseif {$cmp == "HS:"} {
     menuMedm ${thisMenu} "Everything"               ${adlMTR}Slits.adl assy=${bln}${cmp},txt=Huber
     ${thisMenu} add separator
  } elseif {$cmp == "BP:" } {
     if {$bln == "23i:" || $bln == "23o:"} {
        menuMedm ${thisMenu} "Mono BPM Presets"      ${adlMTR}Move1_3presets.adl assy=${bln}${cmp}Mo:,ax1=${bln}${cmp}MOZ:
        if {$bln == "23o:"} {
           menuMedm ${thisMenu} "HDM1 BPM Presets"   ${adlMTR}Move1_4presets.adl assy=${bln}${cmp}Hd1:,ax1=${bln}${cmp}H1Z:
        }
        menuMedm ${thisMenu} "KBM  BPM Presets"      ${adlMTR}Move1_3presets.adl assy=${bln}${cmp}Kb:,ax1=${bln}${cmp}KBZ:
     }
     if { $bln == "23b:" } {
        menuMedm ${thisMenu} "VFM BPM Presets"       ${adlMTR}Move1_3presets.adl assy=${bln}${cmp}Vf:,ax1=${bln}${cmp}VFZ:
     }
     ${thisMenu} add separator
  }

  foreach unit $units {menuUNT ${thisMenu} ${bln} ${cmp} ${unit}}
}

#***********************************************************
#  proc menuCLOCKS

proc menuCLOCKS {widget bln} {
  global IocPmacList adlPMAC
  set thisMenu $widget

### Compile pmac cards list:
  set pmacs {}
  foreach ioc $IocPmacList {
    foreach pmac [lindex $ioc 1] {
      lappend pmacs [lindex $pmac 0]
    }
  }
  set npmacs [llength $pmacs]
  set str "bl=${bln}"
  set i 0
  foreach pmc $pmacs {
    incr i
    set str "${str},pmac${i}=${bln}${pmc}"
  }
  menuMedm ${thisMenu} "PMAC clocks" ${adlPMAC}PmacClock_view${npmacs}.adl  $str
}

#***********************************************************
#  proc menuPMACS: create PMACs Submenu

proc menuPMACS {widget bln} {
  global IocPmacList adlPMAC
  set thisMenu ${widget}.pmac
  ${widget} add cascade -label "PMAC cards" -menu ${thisMenu}
  menu ${thisMenu}
  foreach ioc $IocPmacList {
     foreach pmac [lindex $ioc 1] {
	menuPMC ${thisMenu} ${bln} [lindex $pmac 0]
     }
     ${thisMenu} add separator
  }
}

#***********************************************************
#  proc menuBURT (backup/restore menu)

proc menuBURT {widget} {
  global Beamline snapdir wish tclGMCA tclGMCA2 errorFile

  set thisMenu "${widget}.mburt"
  ${widget} add cascade -label "Backup/Restore positions" -menu ${thisMenu}

  menu ${thisMenu}

  ${thisMenu} add command -label "Backup motor positions" -command {\
    set dateStamp [clock format [clock seconds] -format {%Y-%m-%d-%H%M}]; \
    set initialFile "${Beamline}-${dateStamp}.snap"; \
    set snapFile [tk_getSaveFile -title {Backup Positions: Specify SNAP File} -initialdir ${snapdir} -initialfile ${initialFile} -filetypes {{"SNAP files" {.snap}} {"ALL files" {*}}} -defaultextension {.snap}]; \
    if {$snapFile != ""} {\
      if {[file exists $snapFile] == 1} {tk_messageBox -message "Overwriting is not allowed!" -type ok -icon error -title "File Exists"; \
      } else {eval "exec ${xterm} ${tclGMCA2}pezca23id mtrPositionsBackup.pl ${snapFile} ${Beamline} &";} \
    } \
  }

  ${thisMenu} add command -label "Restore motor positions" \
      -command "exec ${wish} -f ${tclGMCA}MtrPositionsRestore.tcl ${Beamline} $errorFile & "
}

#***********************************************************
# Create Main Motion Menu

set thisMenu .mbMOTION.m
menu ${thisMenu}
menuMedm ${thisMenu} "All motors at a glance" ${adlMTR}Glance${Beamline}.adl bl=${bln},id=${ID}
${thisMenu} add command -label "Motors assignment" \
      -command "exec ${wish} -f ${tclGMCA}listMotors2.tcl ${Beamline} $errorFile & "
${thisMenu} add command -label "Assemblies assignment" \
      -command "exec ${wish} -f ${tclGMCA}listAssemblies.tcl ${Beamline} $errorFile & "

${thisMenu} add separator
foreach component $ComponentList {menuCMP ${thisMenu} ${bln} ${component}}

${thisMenu} add separator
menuCLOCKS ${thisMenu} $bln
menuPMACS  ${thisMenu} $bln

${thisMenu} add separator
menuBURT ${thisMenu}
if { ${SYSTEM} == "LINUX" } {
  if { ($Beamline == "23i" && ($HOST == "bl1ws3" || $HOST == "bl1dl380lower" || $HOST == "bl1dl380upper")) \
    || ($Beamline == "23o" && ($HOST == "bl2ws3" || $HOST == "bl2dl380lower" || $HOST == "bl2dl380upper")) \
    || ($Beamline == "23b" && ($HOST == "bl3ws3" || $HOST == "bl3dl380lower" || $HOST == "bl3dl380upper")) \
    || ($Beamline == "23d" && [regexp {^px} $HOST]) \
    || $HOST == "www" } {
# To make colored text, use, e.g:  -foreground Red
    ${thisMenu} add command -label "Backup/Restore calibrations" \
       -command "exec ${wish} -f ${tclGMCA}CalibSave.tcl ${Beamline} $errorFile & "
  }
}

if { ( $Beamline == "23i" && $HOST == "keithley1" )     || \
     ( $Beamline == "23i" && $HOST == "bl1ws2" )        || \
     ( $Beamline == "23i" && $HOST == "bl1ws3" )        || \
     ( $Beamline == "23i" && $HOST == "bl1dl380upper" ) || \
     ( $Beamline == "23o" && $HOST == "keithley2" )     || \
     ( $Beamline == "23o" && $HOST == "bl2ws2" )        || \
     ( $Beamline == "23o" && $HOST == "bl2ws3" )        || \
     ( $Beamline == "23o" && $HOST == "bl2dl380upper" ) || \
     ( $Beamline == "23b" && $HOST == "keithley3" )     || \
     ( $Beamline == "23b" && $HOST == "bl3ws2" )        || \
     ( $Beamline == "23b" && $HOST == "bl3ws3" )        || \
     ( $Beamline == "23b" && $HOST == "bl3dl380upper" ) || \
     ( $Beamline == "23d" && [regexp {^px} $HOST] )     || \
     ( $HOST == "www" ) } {
  ${thisMenu} add separator
  ${thisMenu} add command -label "Home Motors" \
      -command "exec ${wish} -f ${tclGMCA}homing_v3.tcl ${Beamline} $errorFile & "
}

if { ( $Beamline == "23i" && ($HOST == "bl1ws3" || $HOST == "bl1ws2") ) || \
     ( $Beamline == "23o" && ($HOST == "bl2ws3" || $HOST == "bl1ws2") ) || \
     ( $Beamline == "23b" && ($HOST == "bl3ws3" || $HOST == "bl1ws2") ) || \
     ( $Beamline == "23d" && [regexp {^px} $HOST])                      || \
     ( $HOST == "www" ) } {
  ${thisMenu} add separator
  ${thisMenu} add command -label "Calibrate Mono" \
      -command "exec ${wish} -f ${tclGMCA}calibrateMono.tcl ${Beamline} & "
}

### Sergey, 2006/09/18: we do not support xxx anymore since we stopped compiling et_wish:
#if { ${SYSTEM} != "WIN32" } {
#  ${thisMenu} add separator
#  ${thisMenu} add command -label "Open Ixxx (new)" \
#     -command "exec ${et} -f ${tclMTR}Ixxx${Beamline}_new.tcl $errorFile &"
#}

