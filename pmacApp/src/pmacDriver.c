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
#include <epicsTypes.h>
#include <epicsRingBytes.h>
#include <cantProceed.h>

/* PMAC headers */
#include <pmacVme.h>
#include <pmacCommon.h>
#include <pmacDriver.h>

#define PMAC_BASE_ASC_REGS_OUT (160)

int    pmacDrvNumAsc  = 0;     /* DPRAM ASCII driver number   */
int    pmacDrvNumMbx  = 0;     /* Mailbox ASCII driver number */
int    replyQueueSize = 2048;  /* Size of ring buffer         */

PMAC_ASC_DEV     pmacAscDev[PMAC_MAX_CTLRS];
PMAC_MBX_DEV     pmacMbxDev[PMAC_MAX_CTLRS];
epicsRingBytesId replyQ[PMAC_MAX_CTLRS];

IMPORT int       pmacVmeConfigLock;
IMPORT int       pmacVmeNumCtlrs;
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

  ret          = OK;
  installedAsc = FALSE;
  installedMbx = FALSE;

  /* For the DPRAM ASCII driver */

  /* check if driver already installed */

  if(pmacDrvNumAsc > 0)        
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

    pmacDrvNumAsc = iosDrvInstall( 0, 0, pmacOpenAsc,  pmacCloseAsc, pmacReadAsc,
                                         pmacWriteAsc, pmacIoctlAsc );
    /* Add a DPRAM ASCII device for every configured card */
    for( i=0; i<pmacVmeNumCtlrs; i++ )
    {
      sprintf( devNameAsc, "/dev/pmac/%d/asc", i );
      pmacAscDev[i].ctlr = i;
      ret                = iosDevAdd( &pmacAscDev[i].devHdr, devNameAsc, pmacDrvNumAsc);
      if( ret == ERROR )
      {
        printf("Error adding: \"%s\" device\n", devNameAsc);
        break;
      }
      replyQ[i] = epicsRingBytesCreate(replyQueueSize);
      if( !replyQ[i] )
        cantProceed("pmacDrv: Failed to create ring buffer");
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
    /* Add a Mailbox ASCII device for every configured card */
    for( i=0; i<pmacVmeNumCtlrs; i++ )
    {
      sprintf( devNameMbx, "/dev/pmac/%d/mbx", i );
      pmacMbxDev[i].ctlr = i;
      ret                = iosDevAdd( &pmacMbxDev[i].devHdr, devNameMbx, pmacDrvNumMbx);
      if( ret == ERROR )
      {
        printf("Error adding: \"%s\" device\n", devNameMbx);
        break;
      }
    }
  }
  return( ret );
}


int pmacOpenAsc( PMAC_ASC_DEV *pPmacAscDev )
{
  int ret;

  /* printf("pmacOpenAsc: name = %s, number = %d\n", 
          pPmacAscDev->devHdr.name,  pPmacAscDev->ctlr); */

  if( pPmacAscDev->openFlag )
    ret = ERROR;
  else
  {
    pPmacAscDev->openFlag = TRUE;
    ret                   = (int)pPmacAscDev;
  }

  return( ret );
}


int pmacCloseAsc( PMAC_ASC_DEV *pPmacAscDev )
{
  /* printf("pmacCloseAsc called\n"); */

  if( pPmacAscDev->openFlag )
    pPmacAscDev->openFlag = FALSE;

  return( OK );
}


int pmacReadAsc( PMAC_ASC_DEV *pPmacAscDev, char *buffer, int nBytes )
{
  int       numRead;
  int       ctlr;
  PMAC_CTLR *pPmacCtlr;

  ctlr      = pPmacAscDev->ctlr;
  pPmacCtlr = &pmacVmeCtlr[ctlr];

  numRead    = epicsRingBytesGet(replyQ[ctlr], buffer, nBytes);

  if( numRead == 0 )  /* The buffer was empty */
  {
      /* printf("semTake called\n"); */
      semTake(pPmacCtlr->ioAscReadmeSem, WAIT_FOREVER);
      if( pPmacAscDev->cancelFlag )
      {
        pPmacAscDev->cancelFlag = FALSE;
      }
      else
      {
        numRead    = epicsRingBytesGet(replyQ[ctlr], buffer, nBytes);
      }
  }

  /* printf("pmacReadAsc: Read %d chars: %s\n", numRead, buffer); */

  return( numRead );
}


int pmacWriteAsc( PMAC_ASC_DEV *pPmacAscDev, char *buffer, int nBytes )
{
  int        i;
  int        ctlr;
  int        numWritten;
  static int totalWritten=0;
  PMAC_DPRAM *dpramAsciiOut;
  PMAC_DPRAM *dpramAsciiOutControl;

 /* printf("pmacWriteAsc: writing %s to DPRAM, length %d\n", buffer, nBytes); */

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
          dpramAsciiOut[totalWritten] = 0;
          *dpramAsciiOutControl = 1;
          totalWritten = 0;
      }
      else
      {
          if ( totalWritten == 0 )
          {
              /* Line termination just sent - ensure PMAC is ready */
              int count = 0;
              while( *dpramAsciiOutControl != 0x0 )
              {
                  taskDelay(1);
                  count++;
                  if( count > 10 )
                  printf( "pmacWriteAsc: Stuck in while loop\n" );
              }
          }

          dpramAsciiOut[totalWritten] = buffer[i];
          totalWritten++;
      }
      numWritten++;
  }

  return(numWritten);
}


int pmacIoctlAsc( PMAC_ASC_DEV *pPmacAscDev, int request, int *arg )
{
  int       ret;
  int       ctlr;
  PMAC_CTLR *pPmacCtlr;

  ret       = 0;
  ctlr      = pPmacAscDev->ctlr;
  pPmacCtlr = &pmacVmeCtlr[ctlr];

  switch( request )
  {
    case FIONREAD:
      *arg = epicsRingBytesUsedBytes( replyQ[ctlr] );
      break;

    case FIORFLUSH:
      epicsRingBytesFlush( replyQ[ctlr] );
      break;

    case FIOCANCEL:
      pPmacAscDev->cancelFlag = TRUE;
      semGive( pPmacCtlr->ioAscReadmeSem );
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

void pmacAscInISR( PMAC_CTLR *pPmacCtlr )
{
  int         ctlr;
  int         pushOK;
  int         length;
  PMAC_DPRAM  *dpramAsciiInControl;
  PMAC_DPRAM  *dpramAsciiIn;
  epicsUInt16 control;
  char        response[16] = "ERR000\0x7\0x6";

  ctlr                = pPmacCtlr->ctlr;
  dpramAsciiInControl = pmacRamAddr(ctlr, 0x0F40);
  dpramAsciiIn        = pmacRamAddr(ctlr, 0x0F44);
  length              = *pmacRamAddr(ctlr, 0x0F42) - 1;
  control             = *(dpramAsciiInControl) & ((*(dpramAsciiInControl+1))<<8);

  if (control == 0)
  {
      /* Copy the bytes over from the DPRAM ascii buffer */
      pushOK = epicsRingBytesPut( replyQ[ctlr], (char *)dpramAsciiIn, length);
      if( pushOK ) pushOK = epicsRingBytesPut( replyQ[ctlr], (char *)dpramAsciiInControl, 1);
  }
  else
  {
      /* Build a "ERRnnn" string from the BCD error code in dpramAsciiInControl */
      response[3] = ((control >> 8 ) & 0xF ) + '0';
      response[4] = ((control >> 4 ) & 0xF ) + '0';
      response[5] = ((control >> 0 ) & 0xF ) + '0';
      pushOK      = epicsRingBytesPut( replyQ[ctlr], response, 8);
  }

  if( !pushOK ) logMsg("PMAC reply ring buffer full\n", 0, 0, 0, 0, 0, 0);

  *dpramAsciiInControl     = 0x0;
  *(dpramAsciiInControl+1) = 0x0;
  semGive( pPmacCtlr->ioAscReadmeSem );
  return;
}


/* Mailbox ASCII Open/Close/Read/Write/Ioctl routines */

int pmacOpenMbx( PMAC_MBX_DEV *pPmacMbxDev )
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

int pmacCloseMbx( PMAC_MBX_DEV *pPmacMbxDev )
{
  /*  printf("pmacCloseMbx called\n"); */

  if( pPmacMbxDev->openFlag )
    pPmacMbxDev->openFlag = FALSE;

  return( OK );
}

int pmacReadMbx( PMAC_MBX_DEV *pPmacMbxDev, char *buffer, int nBytes )
{
  return(0);
}

int pmacWriteMbx( PMAC_MBX_DEV *pPmacMbxDev, char *buffer, int nBytes )
{
  return(0);
}

int pmacIoctlMbx( PMAC_MBX_DEV *pPmacMbxDev, int request, int *arg )
{
  return(0);
}


/* This is a test routine for testing Open/Close of multiple PMAC devices
   without having PMAC cards in the crate */

long pmacVmeConfigSim( int ctlrNumber, unsigned long addrBase, unsigned long addrDpram,
                       unsigned int irqVector, unsigned int irqLevel )
{
  int       i;
  PMAC_CTLR *pPmacCtlr;
	
  if( pmacVmeConfigLock != 0 )
  {
    printf( "pmacVmeConfigSim: Cannot change configuration after initialization\n" );
    return(ERROR);
  }
  
  if( pmacVmeNumCtlrs == 0 )
  {
    for( i=0; i < PMAC_MAX_CTLRS; i++ )
      pmacVmeCtlr[i].configured = FALSE;
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
  pmacVmeNumCtlrs++;
	
  return(0);
}
