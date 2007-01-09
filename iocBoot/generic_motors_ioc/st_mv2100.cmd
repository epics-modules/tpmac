### vxWorks startup script
  shellPromptSet "ioc1> "

### This is routeAdd, hostAdd, hostShow, nfsMount...
  nfsAuthUnixSet "px0", 501, 100
  nfsMount("px0", "/home/gmca/epics/synApps", "/ioc")

  startup   = "/ioc/xxx1/iocBoot/ioc1"
  appbin    = "/ioc/xxx1/bin/vxWorks-ppc603"

  putenv ("TPMAC=/ioc/tpmac/2-2/pmacApp/Db")

################################################################################

### If the VxWorks kernel was built using the project facility, the following must
### be added before any C++ code is loaded (see SPR #28980).
  sysCplusEnable=1

### Load EPICS software
  cd appbin
  ld < oam.munch

##############################################################################
### dbrestore setup
# ok to restore a save set that had missing values (no CA connection to PV)?
  reboot_restoreDebug = 0
  save_restoreDebug = 0
  sr_restore_incomplete_sets_ok = 1
  reboot_restoreDatedBU = 0
  set_savefile_path(startup, "autosave")
  set_requestfile_path(startup, "dbSave")


### override address, interrupt vector, etc. information in module_types.h
#  module_types()

### need more entries in wait/scan-record channel-access queue?
#  recDynLinkQsize = 1024

  cd startup

#vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
### Load PMAC databases:
# < st_pmac10_23d.cmd
  < st_pmac_14xy-2hs.cmd
#^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  iocLogDisable=1

  iocInit

  taskDelay 160
  create_monitor_set("auto_positions.req", 5, "P=23o:1")
  create_monitor_set("auto_settings.req", 30, "P=23o:1")

### Clear Macro:
  taskDelay 10
  dbpf ("23o:pmac10:StrCmd","ms$$$0")
  taskDelay 60
  dbpf ("23o:pmac10:StrCmd","ms$$$32")

############################# END ioc23ID ##########################
### ALL DONE!
### Enter ^X or "reboot" to reboot!
###

