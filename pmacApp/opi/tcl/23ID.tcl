#! /usr/bin/wish -f


  set Beamline 23d
  set bln	${Beamline}:
  set Control	staff
  set subTitle	(${Control})

  source ./config.tcl
  source ./config23ID.tcl

# All the variables below (myhome,sp,...) are defined in config.tcl:

# This must redirect BOTH of the standard and error IO streams (p.112):
  set errorFile >>&
  append errorFile " ${myhome}${sp}${Facility}.err"

  exec $medm -attach $varyFont -x &
# exec sleep 2

  source ./main.tcl
  source ./motion.tcl

