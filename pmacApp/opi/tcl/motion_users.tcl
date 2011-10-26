# motion_users.tcl
menuMain "MOTION" "motion.xbm"

#***********************************************************
#  proc menuPMC (pmac)

proc menuPMC {widget bln pmc} {
  global medm varyFont errorFile adlPMAC adlSTD adlGMCA tclGMCA Beamline burtSTD acc65eList xterm sp
  set thisMenu "${widget}.m${pmc}"
  ${widget} add cascade -label "${bln}${pmc}" -menu ${thisMenu}
  menu ${thisMenu}
  menuMedm ${thisMenu} "View Servo Clock"    ${adlPMAC}PmacClock_view.adl    pmac=${bln}${pmc}

  set acc65e [ lsearch $acc65eList "${pmc}" ]
  if {$acc65e != -1} {
#   Remove ":" at the end of "pmacXX:" and place the stripped string into $p:
    regsub {:$} $pmc {} p
    ${thisMenu} add separator
#   menuMedm ${thisMenu} "Digital I/O"                   ${adlGMCA}Acc_11E.adl           pmac=${bln}${pmc}
    menuMedm ${thisMenu} "Digital I/O (individual bits)" ${adlSTD}digital_IO_pmac24.adl  P=${bln}${pmc}acc65e,BL=${Beamline},SAVE_DIR=${burtSTD},PMAC=${p}
  }
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
# (this version uses ComponentUsers)

proc menuUNT {widget bln cmp unit} {
  set name   [lindex $unit 0]
  set asy    [lindex $unit 1]
  set axes   [lindex $unit 2]
# puts "${name}"
# puts "${asy}"
# puts "${axes}"
  global medm varyFont errorFile adlMTR sp Beamline

  set thisMenu "${widget}"

  set num [llength ${axes}]
  set macro [macroMOV ${bln} ${cmp} ${asy} ${axes}]
  if {$cmp == "COL:"} {
    menuMedm ${thisMenu} ${name} ${adlMTR}Move_collimator_users_5presets.adl  Beamline=${Beamline},${macro}
  } else {
    menuMedm ${thisMenu} ${name} ${adlMTR}Move${num}_large.adl ${macro}
#   menuMedm ${thisMenu} ${name} ${adlMTR}Move${num}_medium.adl ${macro}
#   menuMedm ${thisMenu} ${name} ${adlMTR}Move${num}_short.adl ${macro}
  }
}

#***********************************************************
#  proc menuUNT_older (unit or PMAC assembly menu)
# (this version uses ComponentList)

proc menuUNT_older {widget bln cmp unit} {
  set name   [lindex $unit 0]
  set asy    [lindex $unit 1]
  set axes   [lindex $unit 5]
  set drives [lindex $unit 6]
# puts "${name}"
# puts "${asy}"
# puts "${axes}"
# puts "${drives}"
  global medm varyFont errorFile adlMTR sp

  set thisMenu "${widget}"
# set thisMenu "${widget}.m${asy}"
# ${widget} add cascade -label ${name} -menu ${thisMenu}
# menu ${thisMenu}

  set num [llength ${axes}]
  if {$num > 0} {
    set macro [macroMOV ${bln} ${cmp} ${asy} ${axes}]
    menuMedm ${thisMenu} ${name} ${adlMTR}Move${num}_large.adl ${macro}
#   menuMedm ${thisMenu} ${name} ${adlMTR}Move${num}_medium.adl ${macro}
#   menuMedm ${thisMenu} ${name} ${adlMTR}Move${num}_short.adl ${macro}
  } else {
    set num [llength ${drives}]
    set macro [macroMOV ${bln} ${cmp} ${asy} ${drives}]
    menuMedm ${thisMenu} ${name} ${adlMTR}Move${num}_large.adl ${macro}
#   menuMedm ${thisMenu} ${name} ${adlMTR}Move${num}_medium.adl ${macro}
#   menuMedm ${thisMenu} ${name} ${adlMTR}Move${num}_short.adl ${macro}
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
        menuMedm ${thisMenu} "Everything"           ${adlMTR}Mono_users.adl assy=${bln}${cmp},xx=${ID}
        ${thisMenu} add separator
     } else {
        menuMedm ${thisMenu} "Everything"           ${adlMTR}MonoBM_users.adl assy=${bln}${cmp}
        ${thisMenu} add separator
     }
  } elseif {$cmp == "HD:"} {
     menuMedm ${thisMenu} "Kill / Enable KBM and HDM motors" ${adlGMCA}kill_kbm_hdm.adl P=${bln}
     ${thisMenu} add separator
  } elseif {$cmp == "KB:"} {
     if { $bln == "23i:" } {
        menuMedm ${thisMenu} "Kill / Enable KBM motors"         ${adlGMCA}kill_kbm.adl P=${bln}
     } elseif { $bln == "23o:" } {
        menuMedm ${thisMenu} "Kill / Enable KBM and HDM motors" ${adlGMCA}kill_kbm_hdm.adl P=${bln}
     }
     ${thisMenu} add separator
  } elseif {$cmp == "GS:"} {
     menuMedm ${thisMenu} "Everything"              ${adlMTR}Slits.adl assy=${bln}${cmp},txt=Guard
     ${thisMenu} add separator
  } elseif {$cmp == "GO:"} {
     if { $bln != "23b:" } {set ST "St";} else {set ST "StB";}
     menuMedm ${thisMenu} "Everything "             ${adlMTR}GonioOCS_USERS.adl Beamline=${Beamline},assy=${bln}${cmp},st=${ST}
     ${thisMenu} add separator
  } elseif {$cmp == "SH:"} {
     menuMedm ${thisMenu} "Shutter Control"         ${adlMTR}Shutter.adl   mtr=${bln}${cmp}mp:,assy=${bln}${cmp}Ps:,BL=${bln}
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
#  proc menuBACKUP

proc menuBACKUP {widget} {
  global Beamline snapdir wish tclGMCA2 errorFile

  set thisMenu ${widget}

  ${thisMenu} add command -label "Backup motor positions" -command {\
    set dateStamp [clock format [clock seconds] -format {%Y-%m-%d-%H%M}]; \
    set initialFile "${Beamline}-${dateStamp}.snap"; \
    set snapFile [tk_getSaveFile -title {Backup Positions: Specify SNAP File} -initialdir ${snapdir} -initialfile ${initialFile} -filetypes {{"SNAP files" {.snap}} {"ALL files" {*}}} -defaultextension {.snap}]; \
    if {$snapFile != ""} {\
      if {[file exists $snapFile] == 1} {tk_messageBox -message "Overwriting is not allowed!" -type ok -icon error -title "File Exists"; \
      } else {eval "exec ${xterm} ${tclGMCA2}pezca23id mtrPositionsBackup.pl ${snapFile} ${Beamline} &";} \
    } \
  }
}

#***********************************************************
# Create Main Motion Menu

set thisMenu .mbMOTION.m
menu ${thisMenu}
menuMedm ${thisMenu} "All motors at a glance" ${adlMTR}Glance${Beamline}.adl bl=${bln},id=${ID}

${thisMenu} add separator
foreach component $ComponentUsers {menuCMP ${thisMenu} ${bln} ${component}}

${thisMenu} add separator
menuCLOCKS ${thisMenu} $bln

${thisMenu} add separator
menuBACKUP ${thisMenu}
