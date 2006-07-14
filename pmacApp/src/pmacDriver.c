/*
 *  Implementation of the Open/Close/Read/Write/Ioctl interface to
 *  PMAC DPRAM ASCII and PMAC Mailbox ASCII.
 *
 *  Author: Andy Foster (for Diamond)
 *  Date:   26th May 2006
 *
*/

/* vxWorks headers */
#include <vxWorks.h>
#include <stdio.h>
#include <string.h>
#include <ioLib.h>
#include <iosLib.h>
#include <logLib.h>
#include <semLib.h>
#include <sioLib.h>
#include <taskLib.h>

/* EPICS headers */
#include <epicsRingBytes.h>
#include <epicsTypes.h>
#include <epicsEvent.h>
#include <cantProceed.h>
#include <devLib.h>

/* PMAC headers */
#include <pmacVme.h>
#include <pmacDriver.h>

#if 0 /* vxWorks */
#include <cacheLib.h>

/** Makes writing to a register on the VME board a bit simpler - or at least
 * 	makes the code a bit more readable. */
#define getReg(location)          (cacheInvalidate( DATA_CACHE, (char *) &(location), sizeof(location)), location )
#define setReg(location, value)   (location) = (value); cacheFlush( DATA_CACHE, (char *) &(location), sizeof(value))
#else

/** Makes writing to a register on the VME board a bit simpler - or at least
 * 	makes the code a bit more readable. */
#define getReg(location)          (location)
#define setReg(location, value)   (location) = (value)
#endif

#define PMAC_BASE_ASC_REGS_OUT (160)

int    pmacDrvNumAsc  = 0;     /* DPRAM ASCII driver number   */
int    pmacDrvNumMbx  = 0;     /* Mailbox ASCII driver number */
int    replyQueueSize = 2048;  /* Size of ring buffer         */

typedef struct
{
    DEV_HDR devHdr;
    int     ctlr;
    int     openFlag;
    int     cancelFlag;
    epicsRingBytesId replyQ;
    epicsEventId ioReadmeId;
} PMAC_DEV;

/* Function prototypes */

static int  pmacOpen( PMAC_DEV *pPmacDev, char * remainder, int mode );
static int  pmacClose( PMAC_DEV * );
static int  pmacRead( PMAC_DEV *, char *, int );
static int  pmacWriteAsc( PMAC_DEV *, char *, int );
static int  pmacIoctl( PMAC_DEV *, int, int * );
static int  pmacOpenMbx( PMAC_DEV * );
static int  pmacCloseMbx( PMAC_DEV * );
static int  pmacReadMbx( PMAC_DEV *, char *, int );
static int  pmacWriteMbx( PMAC_DEV *, char *, int );
static int  pmacIoctlMbx( PMAC_DEV *, int, int * );
static void pmacAscInISR( PMAC_DEV *pPmacAscDev );

PMAC_DEV     pmacAscDev[PMAC_MAX_CTLRS];
PMAC_DEV     pmacMbxDev[PMAC_MAX_CTLRS];

IMPORT int       pmacVmeConfigLock;
IMPORT PMAC_CTLR pmacVmeCtlr[PMAC_MAX_CTLRS];


/* This routine installs the DPRAM ASCII driver and the Mailbox ASCII driver.
   It adds a DRPAM ASCII device and a Mailbox ASCII device for every PMAC
   card that has been configured.
   This routine is called from "drvPmac_init" in "drvPmac.c"
   which in turn is called from EPICS "iocInit()" */

STATUS pmacDrv(void)
{
  STATUS ret;
  int    installedAsc;
  int    installedMbx;
  int    i;
  char   devNameAsc[32];
  char   devNameMbx[32];
  long   status;

  ret          = OK;
  installedAsc = FALSE;
  installedMbx = FALSE;

  /* For the DPRAM ASCII driver */

  /* check if driver already installed */

  if (pmacDrvNumAsc > 0)
  {
    installedAsc = TRUE;
    ret          = OK;
  }

  if( !installedAsc )
  {
    for( i=0; i<PMAC_MAX_CTLRS; i++ )
    {
      pmacAscDev[i].openFlag   = FALSE;
      pmacAscDev[i].cancelFlag = FALSE;
    }

    /* printf("pmacDrv: Installing ASC: pmacVmeNumCtlrs = %d\n", pmacVmeNumCtlrs); */

    pmacDrvNumAsc = iosDrvInstall( 0, 0, pmacOpen,  pmacClose, pmacRead,
                                         pmacWriteAsc, pmacIoctl );

    /* Add a DPRAM ASCII device for every configured card */
    for( i=0; i < PMAC_MAX_CTLRS; i++ )
    {
        pmacAscDev[i].ctlr =  pmacVmeCtlr[i].ctlr;
        pmacAscDev[i].openFlag   = 0;
        pmacAscDev[i].cancelFlag = 0;

	if ( pmacVmeCtlr[i].configured )
        {
            sprintf( devNameAsc, "/dev/pmac/%d/asc", pmacVmeCtlr[i].ctlr );           
            ret = iosDevAdd( &pmacAscDev[i].devHdr, devNameAsc, pmacDrvNumAsc);
            if( ret == ERROR ) cantProceed("pmacDrv: Error adding: /dev/pmac/n/asc device" );

            pmacAscDev[i].replyQ = epicsRingBytesCreate(replyQueueSize);
            if( !pmacAscDev[i].replyQ )
                cantProceed("pmacDrv: Failed to create ring buffer");

            pmacAscDev[i].ioReadmeId = epicsEventMustCreate( epicsEventEmpty ); 

            status = devConnectInterrupt (intVME, pmacVmeCtlr[i].irqVector + 1,
                                          (void *)pmacAscInISR, (void *) &(pmacAscDev[i]));
            if (!RTN_SUCCESS(status))
                cantProceed("pmacDrv: Failed to connect to DPRAM ASCII interrupt");
        }
    }
  }

  /* For the Mailbox ASCII driver */

  /* check if driver already installed */

  if(pmacDrvNumMbx > 0)
  {
    installedMbx = TRUE;
    ret          = OK;
  }

  if( !installedMbx )
  {
    for( i=0; i<PMAC_MAX_CTLRS; i++ )
      pmacMbxDev[i].openFlag = FALSE;

    /* printf("pmacDrv: Installing MBX: pmacVmeNumCtlrs = %d\n", pmacVmeNumCtlrs); */

    pmacDrvNumMbx = iosDrvInstall( 0, 0, pmacOpenMbx,  pmacCloseMbx, pmacReadMbx,
                                         pmacWriteMbx, pmacIoctlMbx );

    /* Add a DPRAM ASCII device for every configured card */
    for( i=0; i < PMAC_MAX_CTLRS; i++ )
    {
        pmacMbxDev[i].ctlr =  pmacVmeCtlr[i].ctlr;
        pmacMbxDev[i].openFlag   = 0;
        pmacMbxDev[i].cancelFlag = 0;

	if ( pmacVmeCtlr[i].configured )
        {
            sprintf( devNameMbx, "/dev/pmac/%d/mbx", pmacVmeCtlr[i].ctlr );           
            ret = iosDevAdd( &pmacMbxDev[i].devHdr, devNameMbx, pmacDrvNumMbx);
            if( ret == ERROR ) cantProceed("pmacDrv: Error adding: /dev/pmac/n/asc device" );

            pmacMbxDev[i].replyQ = epicsRingBytesCreate(replyQueueSize);
            if( !pmacMbxDev[i].replyQ )
                cantProceed("pmacDrv: Failed to create ring buffer");

            pmacMbxDev[i].ioReadmeId = epicsEventMustCreate( epicsEventEmpty ); 

/*
             status = devConnectInterrupt (intVME, pmacVmeCtlr[i].irqVector + 1,
                                           (void *)pmacMbxInISR, (void *) &(pmacMbxDev[i]));

            if (!RTN_SUCCESS(status))
                cantProceed("pmacDrv: Failed to connect to Mailbox interrupt");
*/
        }
    }
  }
  return( ret );
}


static int pmacOpen( PMAC_DEV *pPmacDev, char * remainder, int mode )
{
    if (remainder[0] != 0 || pPmacDev->openFlag )
        return ERROR;
    else
        pPmacDev->openFlag = TRUE;

    return( (int) pPmacDev );
}


static int pmacClose( PMAC_DEV *pPmacDev )
{
  /* printf("pmacCloseAsc called\n"); */

  if( pPmacDev->openFlag )
      pPmacDev->openFlag = FALSE;

  return( OK );
}


static int pmacRead( PMAC_DEV *pPmacDev, char *buffer, int nBytes )
{
  int       numRead;

  numRead    = epicsRingBytesGet(pPmacDev->replyQ, buffer, nBytes);

  if( numRead == 0 )  /* The buffer was empty */
  {
      epicsEventWait( pPmacDev->ioReadmeId );
      if( pPmacDev->cancelFlag )
      {
        pPmacDev->cancelFlag = FALSE;
      }
      else
      {
        numRead    = epicsRingBytesGet(pPmacDev->replyQ, buffer, nBytes);
      }
  }

  return( numRead );
}


static int pmacWriteAsc( PMAC_DEV *pPmacAscDev, char *buffer, int nBytes )
{
  int        i;
  int        ctlr;
  int        numWritten;
  static int totalWritten=0;
  PMAC_DPRAM *dpramAsciiOut;
  PMAC_DPRAM *dpramAsciiOutControl;

  ctlr                 = pPmacAscDev->ctlr;
  dpramAsciiOut        = pmacRamAddr(ctlr,0x0EA0);
  dpramAsciiOutControl = pmacRamAddr(ctlr,0x0E9C);
  i                    = 0;
  numWritten           = 0;


  for( i=0; (i < nBytes) && (i < PMAC_BASE_ASC_REGS_OUT); i++ )
  {
      if ( buffer[i] == '\r' )
      {
          /* Send command to PMAC */
          setReg( dpramAsciiOut[totalWritten], (char) 0 );
          setReg( *dpramAsciiOutControl, (char) 1 );
          totalWritten = 0;
      }
      else
      {
          if ( totalWritten == 0 )
          {
              /* Line termination just sent - ensure PMAC is ready */
              int count = 0;
              while( getReg( *dpramAsciiOutControl ) != 0x0 )
              {
                  taskDelay(1);
                  count++;
                  if( count > 10 )
                  printf( "pmacWriteAsc: Stuck in while loop\n" );
              }
          }

          setReg( dpramAsciiOut[totalWritten], buffer[i] );
          totalWritten++;
      }
      numWritten++;
  }

  return(numWritten);
}


static int pmacIoctl( PMAC_DEV *pPmacDev, int request, int *arg )
{
  int       ret=0;

  switch( request )
  {
    case FIONREAD:
      *arg = epicsRingBytesUsedBytes( pPmacDev->replyQ );
      break;

    case FIORFLUSH:
      epicsRingBytesFlush( pPmacDev->replyQ );
      break;

    case FIOCANCEL:
      pPmacDev->cancelFlag = TRUE;
      epicsEventSignal( pPmacDev->ioReadmeId );
      break;

    case SIO_HW_OPTS_GET:
      /* printf("SIO_HW_OPTS_GET: Get hardware options - not implemented\n"); */
      break;

    case SIO_BAUD_GET:
      /* printf("SIO_BAUD_GET: Get the baud rate - not implemented\n"); */
      break;

    case SIO_BAUD_SET:
      /* printf("SIO_BAUD_SET: Set the baud rate - not implemented\n"); */
      break;

    case FIOBAUDRATE:
      /* printf("FIOBAUDRATE: Set serial baud rate - not implemented\n"); */
      break;

    default:
      break;
  }
  return( ret );
}


/* pmacAscInISR - Interrupt Service Routine which is called when
                  PMAC issues an interrupt to tell us the DPRAM ASCII buffer
                  can be read.
                  Note: There is 1 interrupt per line of the response and
                  1 interrupt for the ACK at the end.
                  Note: Errors returned by PMAC are not being handled here
                  and they probably should be. */

static void pmacAscInISR( PMAC_DEV *pPmacAscDev )
{
  int        i;
  int        ctlr;
  int        pushOK;
  int        length;
  PMAC_DPRAM *dpramAsciiInControl;
  PMAC_DPRAM *dpramAsciiIn;
  epicsUInt16 control;
  char       terminator;

  ctlr                = pPmacAscDev->ctlr;
  dpramAsciiInControl = pmacRamAddr(ctlr, 0x0F40);
  dpramAsciiIn        = pmacRamAddr(ctlr, 0x0F44);
  length              = getReg( *pmacRamAddr(ctlr, 0x0F42) ) - 1;
  terminator          = getReg( *(dpramAsciiInControl) );
  control             = getReg( *(dpramAsciiInControl+1));

  if (control == 0)
  {
    for( i=0; i<length; i++ )
    {
        char c = getReg(*dpramAsciiIn);
        pushOK = epicsRingBytesPut( pPmacAscDev->replyQ, &c, 1);
        if( !pushOK )
            logMsg("PMAC reply ring buffer full\n", 0, 0, 0, 0, 0, 0);
        dpramAsciiIn++;
    }
    pushOK = epicsRingBytesPut( pPmacAscDev->replyQ, &terminator, 1);
    if( !pushOK )
      logMsg("PMAC reply ring buffer full\n", 0, 0, 0, 0, 0, 0);
  }
  else
  {
    /* Build a "ERRnnn" string from the BCD error code in dpramAsciiInControl */
    char response[]={'E','R','R','0','0','0',PMAC_TERM_BELL,PMAC_TERM_ACK};

    /* Convert the BCD encoded error number to its ASCII equivalent */
    response[3] += ((control ) & 0xF );
    response[4] += ((terminator >> 4 ) & 0xF );
    response[5] += ((terminator ) & 0xF );

    /* Push the data the onto the ring buffer */
    pushOK = epicsRingBytesPut( pPmacAscDev->replyQ, response, sizeof(response));
    if( !pushOK )
      logMsg("PMAC reply ring buffer full\n", 0, 0, 0, 0, 0, 0);
  }

  setReg( *((volatile epicsUInt16 *) dpramAsciiInControl), (epicsUInt16) 0 );
  control = getReg( *((volatile epicsUInt16 *) dpramAsciiInControl));
  epicsEventSignal( pPmacAscDev->ioReadmeId );
  return;
}


/* Mailbox ASCII Open/Close/Read/Write/Ioctl routines */

static int pmacOpenMbx( PMAC_DEV *pPmacMbxDev )
{
  int ret;

  /* printf("pmacOpenMbx: name = %s, number = %d\n", 
          pPmacMbxDev->devHdr.name,  pPmacMbxDev->ctlr); */

  if( pPmacMbxDev->openFlag )
    ret = ERROR;
  else
  {
    pPmacMbxDev->openFlag = TRUE;
    ret                   = (int)pPmacMbxDev;
  }

  return( ret );
}

static int pmacCloseMbx( PMAC_DEV *pPmacMbxDev )
{
  /*  printf("pmacCloseMbx called\n"); */

  if( pPmacMbxDev->openFlag )
    pPmacMbxDev->openFlag = FALSE;

  return( OK );
}

static int pmacReadMbx( PMAC_DEV *pPmacMbxDev, char *buffer, int nBytes )
{
  return(0);
}

static int pmacWriteMbx( PMAC_DEV *pPmacMbxDev, char *buffer, int nBytes )
{
  return(0);
}

static int pmacIoctlMbx( PMAC_DEV *pPmacMbxDev, int request, int *arg )
{
  return(0);
}


/* This is a test routine for testing Open/Close of multiple PMAC devices
   without having PMAC cards in the crate */

long pmacVmeConfigSim( int ctlrNumber, unsigned long addrBase, unsigned long addrDpram,
                       unsigned int irqVector, unsigned int irqLevel )
{
  PMAC_CTLR *pPmacCtlr;
	
  if( pmacVmeConfigLock != 0 )
  {
    printf( "pmacVmeConfigSim: Cannot change configuration after initialization\n" );
    return(ERROR);
  }
  	
  if( (ctlrNumber < 0) | (ctlrNumber >= PMAC_MAX_CTLRS) )
  {
    printf( "pmacVmeConfigSim: Controller number %d invalid -- must be 0 to %d.\n",
             ctlrNumber, PMAC_MAX_CTLRS-1 );
    return(ERROR);
  }
  
  if( pmacVmeCtlr[ctlrNumber].configured )
  {
    printf( "pmacVmeConfigSim: Controller %d already configured -- request ignored.\n",
            ctlrNumber );
    return(ERROR);
  }
	
  pPmacCtlr                = &pmacVmeCtlr[ctlrNumber];
  pPmacCtlr->ctlr          = ctlrNumber;
  pPmacCtlr->vmebusBase    = addrBase;
  pPmacCtlr->irqVector     = irqVector;
  pPmacCtlr->irqLevel      = irqLevel;
  pPmacCtlr->enabled       = FALSE;
  pPmacCtlr->present       = FALSE;
  pPmacCtlr->active        = FALSE;
  pPmacCtlr->enabledBase   = TRUE;
  pPmacCtlr->presentBase   = TRUE;
  pPmacCtlr->activeBase    = FALSE;
  pPmacCtlr->enabledDpram  = TRUE;
  pPmacCtlr->presentDpram  = TRUE;
  pPmacCtlr->activeDpram   = FALSE;
  pPmacCtlr->enabledGather = TRUE;
  pPmacCtlr->activeGather  = FALSE;
  pPmacCtlr->vmebusDpram   = addrDpram;
  if( addrDpram == 0 )
    pPmacCtlr->enabledDpram = FALSE;

  pPmacCtlr->present    = pPmacCtlr->presentBase | pPmacCtlr->presentDpram;
  pPmacCtlr->enabled    = pPmacCtlr->enabledBase | pPmacCtlr->enabledDpram;
  pPmacCtlr->configured = TRUE;
	
  return(0);
}
