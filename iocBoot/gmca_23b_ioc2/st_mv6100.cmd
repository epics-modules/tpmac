### See also: bootShow, bootChange
###
### vxWorks startup script
  loginUserAdd "gmca", "RceeQSdRSb"
# loginUserDelete "vw5"
### Set shell prompt
  shellPromptSet "23b:ioc2> "

ipAttach 1, "geisc"
ifAddrSet "geisc1", "192.168.0.1"
ifMaskSet "geisc1", 0xffffffC0
hostAdd "ICB", "192.168.0.1"

### This is routeAdd, hostAdd, hostShow, nfsMount...
#  < ../nfsCommands
# nfsAuthUnixSet "bl3dl380upper", 500, 100
# nfsMount("bl3dl380upper", "/home/gmca/epics_synApps/synApps_5_2", "/ioc")
  nfsAuthUnixSet "bl3dl380lower", 500, 100
  nfsMount("bl3dl380lower", "/home/gmca/epics_synApps/synApps_5_2", "/ioc")

### Add gateway to be visible from 23ID-in network:
# routeAdd     "destaddr",      "gateaddr"
# Route from BM-net to lab:
  routeAdd  "164.54.210.0",   "164.54.210.193"
# Route from BM-net to ID-in:
  routeAdd  "164.54.210.64",  "164.54.210.193"
# routeShow

  location  = "23-BM"
  engineer  = "O.Makarov"

  < iocEnv

  putenv ("EPICS_TS_NTP_INET=164.54.210.2")
### 215=bl3ws6 (directnet) 217=bl3ioc1 218=bl3ioc2 219=keithley3 220=mar3
### putenv ("EPICS_CA_ADDR_LIST=164.54.210.2 164.54.210.215 164.54.210.217")
  putenv ("EPICS_CA_ADDR_LIST=164.54.210.2")
  putenv ("EPICS_CA_AUTO_ADDR_LIST=YES")
  printf "EPICS_CA_ADDR_LIST=%s\n",getenv("EPICS_CA_ADDR_LIST")
################################################################################

### If the VxWorks kernel was built using the project facility, the following must
### be added before any C++ code is loaded (see SPR #28980).
  sysCplusEnable=1

################################################################################
### Load EPICS base software
  cd appbin
  ld < xxx.munch

# Increase size of buffer for error logging from default 1256
  errlogInit(5000)
################################################################################
### Register all support components
  cd startup
  dbLoadDatabase("../../dbd/iocxxxVX.dbd")
  iocxxxVX_registerRecordDeviceDriver(pdbbase)

  iocLogDisable=1

################################################################################
### IP stuff:
  < st_ip.cmd

################################################################################
### NIM BIN stuff - fluorescent detector, HVPS, etc.:
 < st_icb6100.cmd

##############################################################################
### save_restore setup
### status-PV prefix
  save_restoreSet_status_prefix("23b:ioc2")
### Debug-output level
  save_restoreSet_Debug(0)

### Ok to save/restore save sets with missing values (no CA connection to PV)?
  save_restoreSet_IncompleteSetsOk(1)
### Save dated backup files?
  save_restoreSet_DatedBackupFiles(0)

### Number of sequenced backup files to write
  save_restoreSet_NumSeqFiles(3)
### Time interval between sequenced backups
  save_restoreSet_SeqPeriodInSeconds(600)

### If you want save_restore to manage its own NFS mount, specify the name and
### IP address of the file server to which save files should be written.
### This currently is supported only on vxWorks.
### save_restoreSet_NFSHost("oxygen", "164.54.52.4")

### specify where save files should be
  set_savefile_path(startup, "autosave")

### specify what save files should be restored.  Note these files must be
### in the directory specified in set_savefile_path(), or, if that function
### has not been called, from the directory current when iocInit is invoked
### Specify which save files are to be restored before record initialization (pass 0)
### and which are to be restored after record initialization (pass 1)
### This is the default: restore positios before record init and settings both before and after:
# set_pass0_restoreFile("auto_positions.sav")
  set_pass0_restoreFile("auto_settings.sav")
  set_pass1_restoreFile("auto_settings.sav")
# set_pass0_restoreFile("triggered_settings.sav")

### specify directories in which to to search for included request files
  set_requestfile_path(startup, "dbSave")

### Save_restore status database:
  dbLoadRecords("$(AUTOSAVE)/save_restoreStatus.db", "P=23b:ioc2")
  dbLoadRecords("$(GMCA)/generic_bo.db", "BL=23b,NAME=ioc2:saveTrigger")

################################################################################
### Oleg (XYcom-542 DAC/ADC card):
###xy542Configure(
###    int         cardNum,        /* card number as used in INP fields */
###    void        *base,          /* base address of the board */
###    int         vectorNum,      /* interrupt vector number. If 0 then one */
###				   /* will be found */
###    int         itrLevel,       /* board's interrupt level */
###    int         itrState,       /* whether or not interrupts should be */
###				   /* enabled at init time */
###    int	 aiType		   /* analog input signal type (differential=0 */
###				   /* or single-ended=1 */
###)
### base address 0x0000; address range: 0x0000 - 0x03ff, interrupt level 5
#   xy542Configure(0, 0xff0000, 0x??, 2, 1, 0)
#   dbLoadTemplate("../../db/XY542.substitutions")

###############################################################################
### Scan-support software
### crate-resident scan.  This executes 1D, 2D, 3D, and 4D scans, and caches
### 1D data, but it doesn`t store anything to disk.  (See 'saveData' below
### for that.)
  dbLoadRecords("$(SSCAN)/scan.db","P=23b:2:,MAXPTS1=2000,MAXPTS2=20,MAXPTS3=1,MAXPTS4=1,MAXPTSH=2000")

###############################################################################
### Struck 7201 multichannel scaler (same as SIS 3806 multichannel scaler)

###STR7201Setup(int numCards, int baseAddress, int interruptVector, int interruptLevel)
  STR7201Setup(1, 0xA0000000, 0xDC, 6)
# mcaRecordDebug = 10
# devSTR7201Debug = 20
# drvSTR7201Debug = 10
# devSiStrParmDebug = 0

### STR7201Config(int card, int maxSignals, int maxChans,
###               int 1=enable internal 25MHZ clock,
###               int 1=enable initial software channel advance in MCS external advance mode)
  STR7201Config(0, 32, 4000, 1, 1)
  dbLoadRecords("$(MCA)/Struck32.db", "P=23b:s1:,M=c")

### In the following make sure to use: P=23b:s1, M=mca#, CHANS=1024 (or CHANS=2048)
# dbLoadTemplate("mcs_2048.substitutions")
  dbLoadTemplate("mcs_4000.substitutions")

### Struck Scaler working as a Joerger (16 inputs only):
  dbLoadRecords("$(MCA)/STR7201scaler.db", "P=23b:,S=scaler1,C=0")

### Reduce delay before starting autocount (info from Tim Mooney):
  scaler_wait_time=1

###############################################################################
### Joerger VS
### scalerVS_Setup(int num_cards,	/* maximum number of cards in crate */
###	char *addrs,		/* address (0x800-0xf800, 2048-byte (0x800) boundary) */
###	unsigned vector,	/* valid vectors(96-255 or 0x60-0xff) */
###	int intlevel)
  scalerVS_Setup(1, 0x2000, 0xCE, 5)
# devScaler_VSDebug=5
# scalerRecordDebug=0
  dbLoadRecords("$(STD)/scaler32.db","P=23b:,S=scaler3,C=0, DTYP=Joerger VS, FREQ=10000000, OUT=#C0 S0 @")

### Reduce delay before starting autocount (info from Tim Mooney):
  scaler_wait_time=1

###############################################################################
### Acromag AVME9440 setup parameters:
### devAvem9440Config (ncards,a16base,intvecbase)
  devAvme9440Config(1, 0x2800, 0xE0)

### Acromag 9440 digital IO records:
### - Outpot Channels 0--15 can provide digital output
### - Input  Channels 0--7 are scanned in I/O interrupt mode
### - Input  Channels 8--15 are scanned in passive mode
### - Input  Channels 16--31 provide digital output readback
### ATTENTION: to use interrupts, set interrupt level to 5 (jumper J4: 1&2, 5&6 IN))

  dbLoadTemplate("digital_IO.substitutions")

# devAvme9440Debug=5

###############################################################################
dbLoadRecords("$(VXSTATS)/vxStats-template.db", "IOCNAME=23b:2")
### medm -x -attach -macro P=23b:2 /home/gmca/epics_synApps/synApps_5_1/std/2-5/stdApp/op/adl/IOC_Status_full.adl &

###vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
### Load PMAC databases:
   < st_pmac.cmd

###^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
### Robot databases:
  dbLoadRecords("$(GMCA)/robot.db","bln=23b:")

### Blu-Ice Databases
  dbLoadTemplate("bluiceCollect.substitutions")
  dbLoadTemplate("bluiceConfig.substitutions")
  dbLoadTemplate("bluiceScan.substitutions")
  dbLoadTemplate("bluiceHutch.substitutions")
### This is temporary in the keithley IOC:
# dbLoadTemplate("bluiceScreening.substitutions")

### Mostab Databases (contact Derek)
  dbLoadRecords("$(GMCA)/mostab.db", "P=23b:mostab:,PP=23b:postab:")
  dbLoadRecords("$(GMCA)/generic_IncrDecr.db","PV=23b:postab:set_v,PREC=4,INIVAL=0.01")
  dbLoadRecords("$(GMCA)/generic_IncrDecr.db","PV=23b:postab:set_h,PREC=4,INIVAL=0.01")
### Local copy of SRcurrent (should be in the same place with the PID record)
  dbLoadRecords("$(GMCA)/SRcurrent.db","BL=23b")

# iocInit "/ioc/xxx2/iocBoot/iocgmca2/resource.def"
  iocInit

### Start up the autosave task and tell it what to do.
### The task is actually named "save_restore".
### (See also, 'initHooks' above, which is the means by which the values that
### will be saved by the task we`re starting here are going to be restored.
### Note that you can reload these sets after creating them: e.g.,
# reload_monitor_set("auto_settings.req",30,"P=23b:2")

  taskDelay 240
### save positions and settings every NN seconds
# create_monitor_set("auto_positions.req", 5, "P=23b:ioc2")
# create_monitor_set("auto_settings.req", 600, "P=23b:ioc2")
# create_periodic_set("auto_settings.req", 600, "P=23b:ioc2")
# create_triggered_set("triggered_settings.req", "23b:ioc2:saveTrigger.PROC", "P=23b:ioc2")
  create_monitor_set("auto_positions.req", 1800, "P=23b:ioc2")
  create_monitor_set("auto_settings.req", 18000, "P=23b:ioc2")
  create_triggered_set("auto_settings.req", "23b:ioc2:saveTrigger.PROC", "P=23b:ioc2")

# Reset Macro (now moved to home script based on I39 status):
# < st_clearmacro.cmd

  taskDelay 30
     dbpf ("23b:pmac20:StrCmd","msclrf0")
  taskDelay 30
     dbpf ("23b:pmac20:StrCmd","msclrf24")
  taskDelay 30
     dbpf ("23b:pmac20:StrCmd","msclrf28")
  taskDelay 30
     dbpf ("23b:pmac20:StrCmd","msclrf32")
  taskDelay 30

  taskDelay 30
     dbpf ("23b:SH:mp:OpenPos.PROC", "1")
  taskDelay 30
     dbpf ("23b:SH:mp:ClosePos.PROC","1")
  taskDelay 30

  taskDelay 30
     dbpf ("23b:pmac21:StrCmd","msclrf0")
  taskDelay 30
     dbpf ("23b:pmac21:StrCmd","msclrf32")
  taskDelay 30

### Disable BeamStop-Out if fast shuter is open:
    dbpf ("23b:BS:PT:RqsPosRcl2.SDIS","23b:SH:mp:bo:rbk PP MS")

### This is now moved into the DB -- Sergey:
# seq &frame_vx,"name=BluIce,frm=23b:FPE"

  seq &robot, "name=SMR, unit=23b:"
  taskDelay 30

### Disable Telnet:
# ts tTelnetd
######################## 23b:ioc2: ALL DONE! #################################
### Enter ^X or "reboot" to reboot!
### Debugging: dbcar(0,5), i, dbgrep
### Also use: memShow, bootChange, lkup, dbgrep, i, ts, tr

