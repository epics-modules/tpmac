/* pmacVme.c -  EPICS Device Driver Library for Turbo PMAC2-VME Ultralite
 *
 * Author       Oleg A. Makarov
 *              Thomas A. Coleman's PMAC-VME driver library was used as a prototype 
 * Date:        2003/08/19
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 */

/*
*****************************************************************
                          COPYRIGHT NOTIFICATION
*****************************************************************

THE FOLLOWING IS A NOTICE OF COPYRIGHT, AVAILABILITY OF THE CODE,
AND DISCLAIMER WHICH MUST BE INCLUDED IN THE PROLOGUE OF THE CODE
AND IN ALL SOURCE LISTINGS OF THE CODE.
 
(C)  COPYRIGHT 1995 UNIVERSITY OF CHICAGO
 
Argonne National Laboratory (ANL), with facilities in the States of 
Illinois and Idaho, is owned by the United States Government, and
operated by the University of Chicago under provision of a contract
with the Department of Energy.

Portions of this material resulted from work developed under a U.S.
Government contract and are subject to the following license:  For
a period of five years from March 30, 1993, the Government is
granted for itself and others acting on its behalf a paid-up,
nonexclusive, irrevocable worldwide license in this computer
software to reproduce, prepare derivative works, and perform
publicly and display publicly.  With the approval of DOE, this
period may be renewed for two additional five year periods. 
Following the expiration of this period or periods, the Government
is granted for itself and others acting on its behalf, a paid-up,
nonexclusive, irrevocable worldwide license in this computer
software to reproduce, prepare derivative works, distribute copies
to the public, perform publicly and display publicly, and to permit
others to do so.

*****************************************************************
                                DISCLAIMER
*****************************************************************

NEITHER THE UNITED STATES GOVERNMENT NOR ANY AGENCY THEREOF, NOR
THE UNIVERSITY OF CHICAGO, NOR ANY OF THEIR EMPLOYEES OR OFFICERS,
MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LEGAL
LIABILITY OR RESPONSIBILITY FOR THE ACCURACY, COMPLETENESS, OR
USEFULNESS OF ANY INFORMATION, APPARATUS, PRODUCT, OR PROCESS
DISCLOSED, OR REPRESENTS THAT ITS USE WOULD NOT INFRINGE PRIVATELY
OWNED RIGHTS.  

*****************************************************************
LICENSING INQUIRIES MAY BE DIRECTED TO THE INDUSTRIAL TECHNOLOGY
DEVELOPMENT CENTER AT ARGONNE NATIONAL LABORATORY (630-252-2000).
*/

/*
 * Modification History:
 * ---------------------
 * .01  6-7-95        tac     initial
 * .02  8-19-03        oam     Turbo PMAC2-VME Ultralite initial
 */

/*
 * DESCRIPTION:
 * ------------
 * This module drives PMAC-VME.
 *
 */

/*
 * INCLUDES
 */

/* VxWorks Includes */

#include	<vxWorks.h>
#include	<vxLib.h>
#include	<sysLib.h>
#include	<taskLib.h>
#include	<iv.h>
#include	<math.h>
#include	<stdio.h>	/* Sergey */
#include	<string.h>	/* Sergey */
#define __PROTOTYPE_5_0         /* Sergey */
#include        <logLib.h>      /* Sergey */

/* EPICS Includes */

#include 	<devLib.h>
#include 	<errMdef.h>

#include <pmacVme.h>

/* ajf: globals */

LOCAL int pmacDrvNumAsc = 0;  /* DPRAM ASCII driver number   */
LOCAL int pmacDrvNumMbx = 0;  /* Mailbox ASCII driver number */

#define PMAC_BASE_ASC_REGS_OUT  (160)

/*
 * DEFINES
 */

#define PMAC_DIAGNOSTICS TRUE
#define PMAC_PRIVATE FALSE

#if PMAC_PRIVATE
#define PMAC_LOCAL LOCAL
#else
#define PMAC_LOCAL
#endif

#if PMAC_DIAGNOSTICS
#define PMAC_MESSAGE	logMsg
#define PMAC_DEBUG(level,code)       { if (pmacVmeDebug >= (level)) { code } }
#else
#define PMAC_DEBUG(level,code)      ;
#endif

#define NO_ERR_STATUS	(-1)

#define PMAC_BUFSIZE	(80)

#define PMAC_MEM_SIZE_BASE	(0x200)		/* Size of base registers */
#define PMAC_MEM_SIZE_DPRAM	(0x4000)	/* Size of DPRAM */

#define PMAC_BASE_MBX_REGS_OUT	(15)
#define PMAC_BASE_MBX_REGS_IN	(16)

#define BYTESWAP(x) (MSB(x) | (LSB(x) << 8))

/*
 * GLOBALS
 */

char * pmacVmeVersion = "@(#) pmacVme.c 1.6 97/05/06";

#if PMAC_DIAGNOSTICS
volatile int	pmacVmeDebug = 0;		/* must be > 0 to see messages */
#endif

/*
 * LOCALS
 */

LOCAL int		pmacVmeConfigLock = 0;
PMAC_LOCAL int		pmacVmeNumCtlrs = 0;
PMAC_LOCAL PMAC_CTLR	pmacVmeCtlr[PMAC_MAX_CTLRS];
PMAC_LOCAL PMAC_ASC_DEV	pmacAscDev[PMAC_MAX_CTLRS];
PMAC_LOCAL PMAC_MBX_DEV	pmacMbxDev[PMAC_MAX_CTLRS];

/*******************************************************************************
 *
 * pmacVmeConfig - Configure PMAC-VME Controller Addresses and Interrupts
 *
 * This routine is to be called in the startup script in order to init the
 * controller addresses and the associated IRQ vectors and levels.
 *
 * By default there are no controllers configured.
 *
 */
long pmacVmeConfig
(
	int		ctlrNumber,
	unsigned long	addrBase,
	unsigned long	addrDpram,
	unsigned int	irqVector,
	unsigned int	irqLevel
)
{
	char *		MyName = "pmacVmeConfig";
	int		i;
	long		val;
	char		block;
	volatile char	*pBlock;
	long		status;
	PMAC_CTLR *	pPmacCtlr;
        char            readbuf[32]; /* ajf */
        char            errmsg[32];  /* ajf */
	
	if (pmacVmeConfigLock != 0)
	{
		printf ( "%s: Cannot change configuration after initialization -- request ignored.\n",
			MyName);
		return (ERROR);
	}
  
	if ( pmacVmeNumCtlrs == 0 )
	{
		for (i=0; i < PMAC_MAX_CTLRS; i++ )
		{
			pmacVmeCtlr[i].configured = FALSE;
		}
	}
	
	if ( (ctlrNumber < 0) | (ctlrNumber >= PMAC_MAX_CTLRS) )
	{
		printf ("%s: Controller number %d invalid -- must be 0 to %d.\n",
			MyName, ctlrNumber, PMAC_MAX_CTLRS - 1);
		return(ERROR);
	}
  
	if (pmacVmeCtlr[ctlrNumber].configured)
	{
		printf ("%s: Controller %d already configured -- request ignored.\n",
			MyName, ctlrNumber);
		return(ERROR);
	}
	
	PMAC_DEBUG
	(	1,
		printf ("%s: Initializing controller %d.\n", MyName, ctlrNumber);
	)

	pPmacCtlr = &pmacVmeCtlr[ctlrNumber];
	pPmacCtlr->ctlr = ctlrNumber;
	pPmacCtlr->vmebusBase = addrBase;
	pPmacCtlr->irqVector = irqVector;
	pPmacCtlr->irqLevel = irqLevel;
	
	pPmacCtlr->enabled = FALSE;
	pPmacCtlr->present = FALSE;
	pPmacCtlr->active = FALSE;
	pPmacCtlr->enabledBase = TRUE;
	pPmacCtlr->presentBase = FALSE;
	pPmacCtlr->activeBase = FALSE;
	pPmacCtlr->enabledDpram = TRUE;
	pPmacCtlr->presentDpram = FALSE;
	pPmacCtlr->activeDpram = FALSE;
	pPmacCtlr->enabledGather = TRUE;
	pPmacCtlr->activeGather = FALSE;

	pPmacCtlr->vmebusDpram = addrDpram;
	if ( addrDpram == 0 )
	{
		pPmacCtlr->enabledDpram = FALSE;
	}
			
/*	status = devRegisterAddress ("PMAC BASE", atVMEA32, */
	status = devRegisterAddress ("PMAC BASE", atVMEA24,
				pPmacCtlr->vmebusBase, PMAC_MEM_SIZE_BASE,
				(void *) &(pPmacCtlr->pBase));
	if (!RTN_SUCCESS(status))
	{
		printf ("%s: Failure registering controller %d base address A24 %#010x.\n",
			MyName, pPmacCtlr->ctlr, (int)pPmacCtlr->vmebusBase);
		return (status);
	}
	
	/*Oleg: selecting lines A19-A14 for the DPRAM */
	pPmacCtlr->pBase->mailbox.MB[144].data = 0x3F & (pPmacCtlr->vmebusBase>>14);
		
#if 0
  ajf: This line causes an MVME-5500 to hang.

	status = vxMemProbe ((char*) &pPmacCtlr->pBase->mailbox.MB[0].data,
				VX_READ, 1, (char*)&val);
	if (status != OK)
	{
		printf ("%s: Failure probing for base address.\n",
			MyName);
		return (status);
	}
#endif

	pPmacCtlr->presentBase = TRUE;

	if ( pPmacCtlr->enabledDpram )
	{
/*		status = devRegisterAddress ("PMAC DPRAM", atVMEA32, */
		status = devRegisterAddress ("PMAC DPRAM", atVMEA24,
					pPmacCtlr->vmebusDpram, PMAC_MEM_SIZE_DPRAM,
					(void *) &(pPmacCtlr->pDpramBase));
		if (!RTN_SUCCESS(status))
		{
			printf ("%s: Failure registering controller %d DPRAM address A24 %#010x.\n",
				MyName, (int)pPmacCtlr->ctlr, (int)pPmacCtlr->vmebusDpram);
			return (status);
		}
	
		block = (char) ((pPmacCtlr->vmebusDpram & 0x000fc000) >> 14);
		pBlock = ((char *) pPmacCtlr->pBase) + 0x121;

		PMAC_DEBUG
		(	1,
			printf ("%s: Setting DPRAM mapping addr %#010lx val %d\n",
				MyName, (long)pBlock, block);
		)
			
		*pBlock = block;
			
		status = vxMemProbe ( (char *) pPmacCtlr->pDpramBase, VX_READ, 2, (char*)&val);
		if (status != OK)
		{
			printf ("%s: Failure probing for DPRAM address: 0x%x\n",
				MyName, (int) *pPmacCtlr->pDpramBase);
			return (status);
		}
		pPmacCtlr->presentDpram = TRUE;
	}

/* ajf */

        pPmacCtlr->ioAscReadmeSem = semBCreate (SEM_Q_FIFO, SEM_FULL);
        if ( pPmacCtlr->ioAscReadmeSem == NULL)
        {
          status = S_dev_internal;
          printf ("%s: Failure creating ioAscReadmeSem binary semaphore.\n", MyName);
          return (status);
        }
	
	pPmacCtlr->ioMbxReceiptSem = semBCreate (SEM_Q_FIFO, SEM_EMPTY);
	if ( pPmacCtlr->ioMbxReceiptSem == NULL)
	{
		status = S_dev_internal;
		printf ("%s: Failure creating ioMbxReceiptSem binary semaphore.\n",
			MyName);
		return (status);
	}

	pPmacCtlr->ioMbxReadmeSem = semBCreate (SEM_Q_FIFO, SEM_EMPTY);
	if ( pPmacCtlr->ioMbxReadmeSem == NULL)
	{
		status = S_dev_internal;
		printf ("%s: Failure creating ioMbxReadmeSem binary semaphore.\n",
			MyName);
		return (status);
	}

	pPmacCtlr->ioMbxLockSem = semBCreate (SEM_Q_FIFO, SEM_EMPTY);
	if ( pPmacCtlr->ioMbxLockSem == NULL)
	{
		status = S_dev_internal;
		printf ("%s: Failure creating ioMbxLockSem binary semaphore.\n",
			MyName);
		return (status);
	}
			
	PMAC_DEBUG
	(	1,
		printf ("%s: Connecting to interrupt vector %d\n",
				MyName, pPmacCtlr->irqVector - 1);
	)

	status = devConnectInterrupt (intVME, pPmacCtlr->irqVector - 1,
				pmacMbxReceiptISR, (void *) pPmacCtlr);
	if (!RTN_SUCCESS(status))
	{
		printf ("%s: Failure to connect interrupt.\n",
			MyName);
		return (status);
	}
			
	PMAC_DEBUG
	(	1,
		printf ("%s: Connecting to interrupt vector %d\n",
				MyName, pPmacCtlr->irqVector);
	)

	status = devConnectInterrupt (intVME, pPmacCtlr->irqVector,
				pmacMbxReadmeISR, (void *) pPmacCtlr);
	if (!RTN_SUCCESS(status))
	{
		printf ("%s: Failure to connect interrupt.\n",
			MyName);
		return (status);
	}

	PMAC_DEBUG
	(	1,
		printf ("%s: Connecting to interrupt vector %d\n",
				MyName, pPmacCtlr->irqVector + 1);
	)

/* ajf */
        status = devConnectInterrupt (intVME, pPmacCtlr->irqVector + 1,
                                      pmacAscInISR, (void *) pPmacCtlr);
        if (!RTN_SUCCESS(status))
        {
          printf ("%s: Failure to connect interrupt.\n", MyName);
          return (status);
        }

	PMAC_DEBUG
	(	1,
		printf ("%s: Enabling interrupt level %d\n",
				MyName, pPmacCtlr->irqLevel);
	)

        status = devEnableInterruptLevel (intVME, pPmacCtlr->irqLevel);
	if (!RTN_SUCCESS(status))
	{
		printf ("%s: Failure to enable interrupt level.\n",
			MyName);
		return (status);
	}
			
	pPmacCtlr->present = pPmacCtlr->presentBase | pPmacCtlr->presentDpram;
	pPmacCtlr->enabled = pPmacCtlr->enabledBase | pPmacCtlr->enabledDpram;
	pPmacCtlr->configured = TRUE;
	pmacVmeNumCtlrs++;
	
	PMAC_DEBUG
	(	1,
		printf ("%s: presentbase = %d, presentDpram=%d, therfore present=%d\n",
				MyName,
			        pPmacCtlr->presentBase,
			        pPmacCtlr->presentDpram,
			        pPmacCtlr->present);
	)

	PMAC_DEBUG
	(	1,
		printf ("%s: enabledBase = %d, enabledDpram=%d, therefore enabled=%d\n",
				MyName,
			        pPmacCtlr->enabledBase,
			        pPmacCtlr->enabledDpram,
			        pPmacCtlr->enabled);
	)

	PMAC_DEBUG
	(	1,
		printf ("%s: pmacVmeNumCtlrs =  %d and  PMAC_MAX_CTLRS = %d\n",
			       	MyName,
			       	pmacVmeNumCtlrs,
			        PMAC_MAX_CTLRS);
	)

        /* ajf: Just read any characters currently in the buffer */
        pmacAscRead(ctlrNumber, readbuf, errmsg);

	return(0);
}

/*******************************************************************************
 *
 * pmacVmeInit - Initialize PMAC-VME Hardware Configuration
 *
 */
PMAC_LOCAL long pmacVmeInit (void)
{
	/* char *	MyName = "pmacVmeInit"; */
	int		i;
	PMAC_CTLR	*pPmacCtlr;
	STATUS		semStatus;
	
	pmacVmeConfigLock = 1;

	if ( pmacVmeNumCtlrs == 0 )
	{
		return (0);
	}
	
	for ( i=0; i < PMAC_MAX_CTLRS; i++)
	{
		pPmacCtlr = &pmacVmeCtlr[i];

		if ( pPmacCtlr->configured )
		{
			if ( pPmacCtlr->presentBase & pPmacCtlr->enabledBase )
			{
				semStatus=semGive (pPmacCtlr->ioMbxLockSem);
				printf ("pmacVmeInit: semStatus=%d, card=%d \n",semStatus, i);	
				pPmacCtlr->activeBase = TRUE;
			}
			
			if ( pPmacCtlr->presentDpram & pPmacCtlr->enabledDpram )
			{
				pPmacCtlr->activeDpram = TRUE;
			}

			if ( pPmacCtlr->activeDpram & pPmacCtlr->enabledGather )
			{
				pPmacCtlr->activeGather = TRUE;
			}

			pPmacCtlr->active = pPmacCtlr->activeBase | pPmacCtlr->activeDpram;
		}
	}	

	return (0);
}

/*******************************************************************************
 *
 * pmacMbxReceiptISR - interrupt service routine for mailbox receipt acknowledge
 *
 */
PMAC_LOCAL void pmacMbxReceiptISR
(
	PMAC_CTLR	*pPmacCtlr
)
{
	char *	MyName = "pmacMbxReceiptISR";
	STATUS	semStatus;

	PMAC_DEBUG
	(	10,
		PMAC_MESSAGE ("%s: PMAC IRQ MbxReceipt for ctlr %d\n", MyName, pPmacCtlr->ctlr,0,0,0,0);
	)
	
	semStatus=semGive (pPmacCtlr->ioMbxReceiptSem);
	if ( semStatus ) {
	   PMAC_MESSAGE ("%s: semStatus=%d card=%d\n", MyName, semStatus, pPmacCtlr->ctlr,0,0,0); 
	}
	
	return;
}

/*******************************************************************************
 *
 * pmacMbxReadmeISR - interrupt service routine for mailbox message arrival
 *
 */
PMAC_LOCAL void pmacMbxReadmeISR
(
	PMAC_CTLR	*pPmacCtlr
)
{
	char *	MyName = "pmacMbxReadmeISR";

	PMAC_DEBUG
	(	10,
		PMAC_MESSAGE ("%s: PMAC IRQ MbxReadme for ctlr %d\n", MyName, pPmacCtlr->ctlr,0,0,0,0);
	)
	
	semGive (pPmacCtlr->ioMbxReadmeSem);
	
	return;
}

/*******************************************************************************
 *
 * pmacMbxOut - put characters in PMAC mailbox
 *
 */
PMAC_LOCAL char pmacMbxOut
(
	int	ctlr,
	char	*writebuf
)
{
    /* char *	MyName = "pmacMbxOut"; */
    int		i;
    int		length;
    char	firstcharacter;
    char	termination;
    PMAC_CTLR	*pPmacCtlr;

    pPmacCtlr = &pmacVmeCtlr[ctlr];
    termination = 0;
    length = strlen (writebuf);

    firstcharacter = writebuf[0];

    for (i = 1; (i < length) && (i < PMAC_BASE_MBX_REGS_OUT); i++)
    {
	pPmacCtlr->pBase->mailbox.MB[i+1].data = writebuf[i];
    }

    if ((i == length) && (i < PMAC_BASE_MBX_REGS_OUT))
    {
	termination = PMAC_TERM_CR;
	pPmacCtlr->pBase->mailbox.MB[i+1].data = PMAC_TERM_CR;
    }
    else if (i > length)
    {
	termination = PMAC_TERM_CR;
	firstcharacter = PMAC_TERM_CR;
    }
    pPmacCtlr->pBase->mailbox.MB[0].data = firstcharacter;

    return (termination);

}   

/*******************************************************************************
 *
 * pmacMbxIn - get characters from PMAC mailbox
 *
 */
PMAC_LOCAL char pmacMbxIn
(
	int	ctlr,
	char	*readbuf,
	char	*errmsg
)
{
    /* char *	MyName = "pmacMbxIn"; */
    int		i;
    int		j;
    char	chr;
    char	terminator;
    char	terminext;
    PMAC_CTLR	*pPmacCtlr;

    pPmacCtlr = &pmacVmeCtlr[ctlr];
    
    terminator = 0;
    terminext = 0;
    errmsg[0] = NULL;

    for (i = 0; (i < PMAC_BASE_MBX_REGS_IN) && (terminator == 0); i++)
    {
	chr = pPmacCtlr->pBase->mailbox.MB[i].data;

	if (chr == PMAC_TERM_CR || chr == PMAC_TERM_ACK || chr == PMAC_TERM_BELL)
	{
	    terminator = chr;
	    readbuf[i] = NULL;
	    if (terminator == PMAC_TERM_BELL)
	    {
		for (j=0, i++; (i < PMAC_BASE_MBX_REGS_IN) && (terminext == 0); j++, i++)
		{
		    chr = pPmacCtlr->pBase->mailbox.MB[i].data;
		    if (chr == PMAC_TERM_CR)
		    {
			terminext = chr;
			errmsg[j] = NULL;
		    }
		    else
		    {
			errmsg[j] = chr;
		    }
		}
	    }
	}
	else
	{
	     readbuf[i] = chr;
	}
    }

    return (terminator);
}



/*******************************************************************************
 *
 * pmacMbxRead - read response from PMAC mailbox
 *
 */
PMAC_LOCAL char pmacMbxRead
(
	int	ctlr,
	char	*readbuf,
	char	*errmsg
)
{
    char *	MyName = "pmacMbxRead";/*  */
    int		i;
    char	terminator;
    PMAC_CTLR	*pPmacCtlr;
    STATUS stat;
    
    pPmacCtlr = &pmacVmeCtlr[ctlr];
    
    i = 0;
    terminator = 0;

    while ( terminator == 0)
    {
	pPmacCtlr->pBase->mailbox.MB[1].data = 0;
	stat = semTake( pPmacCtlr->ioMbxReadmeSem, WAIT_TIMEOUT);
	if (stat == S_objLib_OBJ_TIMEOUT) PMAC_MESSAGE ("%s: PMAC MBX README FAILED, controller: %d\n", MyName, pPmacCtlr->ctlr,0,0,0,0);
	terminator = pmacMbxIn (ctlr, &readbuf[i], errmsg);
	i += PMAC_BASE_MBX_REGS_IN;
    }

    return (terminator);
}



/*******************************************************************************
 *
 * pmacMbxWrite - write command to PMAC mailbox
 *
 */
PMAC_LOCAL char pmacMbxWrite
(
	int	ctlr,
	char	*writebuf
)
{
    char *	MyName = "pmacMbxWrite";/*  */
    int		i;
    char	terminator;
    PMAC_CTLR	*pPmacCtlr;
    STATUS stat;
    
    pPmacCtlr = &pmacVmeCtlr[ctlr];

    i = 0;
    terminator = 0;

    while ( terminator == 0 ) 
    {
	terminator = pmacMbxOut (ctlr, &writebuf[i]);
	stat = semTake (pPmacCtlr->ioMbxReceiptSem, WAIT_TIMEOUT);
	if (stat == S_objLib_OBJ_TIMEOUT) PMAC_MESSAGE ("%s: PMAC MBX RECEIPT FAILED, controller: %d\n", MyName, pPmacCtlr->ctlr,0,0,0,0);
	i += PMAC_BASE_MBX_REGS_OUT;
    }

    return (terminator);

}   

/*******************************************************************************
 *
 * pmacMbxLock - Lock PMAC mailbox for ctlr
 *
 */
long pmacMbxLock
(
	int	ctlr
)
{
    char *	MyName = "pmacMbxLock";
    STATUS stat;
    PMAC_CTLR	*pPmacCtlr;

    pPmacCtlr = &pmacVmeCtlr[ctlr];
	PMAC_DEBUG
	(	7,
		PMAC_MESSAGE ("%s: PMAC MBX LOCK %d\n", MyName, pPmacCtlr->ctlr,0,0,0,0);
	)
    
    stat = semTake (pPmacCtlr->ioMbxLockSem, WAIT_TIMEOUT);
    if (stat == S_objLib_OBJ_TIMEOUT) PMAC_MESSAGE ("%s: PMAC MBX LOCK FAILED, controller: %d\n", MyName, pPmacCtlr->ctlr,0,0,0,0);
    return (0);
}

/*******************************************************************************
 *
 * pmacMbxUnlock - Unlock PMAC mailbox for ctlr
 *
 */
long pmacMbxUnlock
(
	int	ctlr
)
{
    char *	MyName = "pmacMbxUnlock";
    PMAC_CTLR	*pPmacCtlr;

    pPmacCtlr = &pmacVmeCtlr[ctlr];
	PMAC_DEBUG
	(	7,
		PMAC_MESSAGE ("%s: PMAC MBX UNLOCK %d\n", MyName, pPmacCtlr->ctlr,0,0,0,0);
	)
    
    semGive (pPmacCtlr->ioMbxLockSem);
    return (0);
}

/*******************************************************************************
 *
 * pmacVmeWriteC - write character
 *
 */
char pmacVmeWriteC
(
	char *addr,
	char val
)
{
	*addr = val;
	return 0;
}

/*******************************************************************************
 *
 * pmacVmeReadC - read character
 *
 */
char pmacVmeReadC
(
	char *addr
)
{
	return (*addr);
}

/*******************************************************************************
 *
 * pmacRamAddr - get DPRAM address
 *
 */
PMAC_DPRAM * pmacRamAddr
(
	int	ctlr,
	int	offset
)
{
	PMAC_CTLR *	pCtlr = &pmacVmeCtlr[ctlr];
	PMAC_DPRAM *	pDpram = (PMAC_DPRAM *) (pCtlr->pDpramBase + offset);
	
	PMAC_DEBUG
	(	2,
		PMAC_MESSAGE ("pmacRamAddr:  Controller #%d, at base %#X with offset %#X = address a24 %#010X \n",
			      ctlr, pCtlr->pDpramBase, offset, pDpram,0,0);
	)
/* This is a workaround to restore PMAC clock synchronization in case of 2 PMACS -- Sergey, Oleg 2006.01.30 */
/* (At the end of IOC startup script set pmacVmeDebug=1 for about 2s and 6s after booting IOC) */
	PMAC_DEBUG
	(	1,
		PMAC_MESSAGE ("pmacRamAddr:  Controller #%d\n",ctlr,0,0,0,0,0);
		/* PMAC_MESSAGE ("\n",0,0,0,0,0,0); */
		/* printf ("pmacRamAddr:  Controller #%d\n",ctlr); */
		/* printf ("pmacRamAddr:  Controller #%d, at base %#X with offset %#X = address a24 %#010X \n",
			      ctlr, (int)pCtlr->pDpramBase, offset, (int)pDpram); */
	)


	return (pDpram);
}
	
/*******************************************************************************
 *
 * pmacRamGet16 - get DPRAM 16 bits
 *
 */
PMAC_LOCAL long pmacRamGet16
(
	PMAC_DPRAM *	pDpram,
	long *		pVal
)
{
	union {
	      char	ram[4];
	      long	val0;
	} lval;
	
	/* Read PMAC DPRAM */
	lval.ram[0] = (char)0;
	lval.ram[1] = (char)0;
	lval.ram[2] = pDpram[1];
	lval.ram[3] = pDpram[0];
	
	/* Return value */
	*pVal = lval.val0;
	
	return (0);
}

/*******************************************************************************
 *
 * pmacRamPut16 - put DPRAM 16 bits
 *
 */
PMAC_LOCAL long pmacRamPut16
(
	PMAC_DPRAM *	pDpram,
	long 		val
)
{
	union {
	      char	ram[4];
	      long	val0;
	} lval;
	
	/* User value */
	lval.val0 = val;
	
	/* Write PMAC DPRAM */
	pDpram[1] = lval.ram[2];
	pDpram[0] = lval.ram[3];
	
	return (0);
}

/*******************************************************************************
 *
 * pmacRamGet24U - get DPRAM 24 bits unsigned
 *
 */
PMAC_LOCAL long pmacRamGet24U
(
	PMAC_DPRAM *	pDpram,
	long * 		pVal
)
{
	union {
	      char	ram[4];
	      long	val0;
	} lval;

	/* Read PMAC DPRAM */
	lval.ram[0] = (char)0;
	lval.ram[1] = pDpram[2];
	lval.ram[2] = pDpram[1];
	lval.ram[3] = pDpram[0];
		
	/* Return value */
	*pVal = lval.val0;
	
	return (0);
}

/*******************************************************************************
 *
 * pmacRamGet24 - get DPRAM 24 bits
 *
 */
PMAC_LOCAL long pmacRamGet24
(
	PMAC_DPRAM *	pDpram,
	long *		pVal
)
{
	long	val0;

	/* Read PMAC DPRAM */
	pmacRamGet24U (pDpram, &val0);

	if (val0 & 0x800000) val0 |= 0xff000000;
		
	/* Return value */
	*pVal = val0;
	
	return (0);
}

/*******************************************************************************
 *
 * pmacRamPut32 - put DPRAM 32 bits
 *
 */
PMAC_LOCAL long pmacRamPut32
(
	PMAC_DPRAM *	pDpram,
	long 		val
)
{
	union {
	      char	ram[4];
	      long	val0;
	} lval;

	/* User value */
	lval.val0 = val;	

	
	/* Write PMAC DPRAM */
	pDpram[3] = lval.ram[0];
	pDpram[2] = lval.ram[1];
	pDpram[1] = lval.ram[2];
	pDpram[0] = lval.ram[3];

	return (0);
}

/*******************************************************************************
 *
 * pmacRamGetF - get DPRAM F word
 *
 */
PMAC_LOCAL long pmacRamGetF
(
	PMAC_DPRAM *	pDpram,
	double *	pVal
)
{
	union {
	      char	ram[4];
	      float	valF;
	} lval;

	/* Read PMAC DPRAM */
	lval.ram[0] = pDpram[3];
	lval.ram[1] = pDpram[2];
	lval.ram[2] = pDpram[1];
	lval.ram[3] = pDpram[0];

	/* Return value */
	*pVal = (double) lval.valF;

	return (0);
}

/*******************************************************************************
 *
 * pmacRamPutF - put DPRAM F word
 *
 */
PMAC_LOCAL long pmacRamPutF
(
	PMAC_DPRAM *	pDpram,
	double		val
)
{
	union {
	      char	ram[4];
	      float	valF;
	} lval;

	/* User value */
	lval.valF = (float) val;
	
	/* Write PMAC DPRAM */
	pDpram[3] = lval.ram[0];
	pDpram[2] = lval.ram[1];
	pDpram[1] = lval.ram[2];
	pDpram[0] = lval.ram[3];

	return (0);
}

/*******************************************************************************
 *
 * pmacRamGetD - get DPRAM D word
 *
 */
PMAC_LOCAL long pmacRamGetD
(
	PMAC_DPRAM *	pDpram,
	double *	pVal
)
{

	long	val0;
	long	val1;
	
	pmacRamGet24U (pDpram, &val0);
	pmacRamGet24 (&(pDpram[4]), &val1);

	/* Convert 48 bit fixed-point format */
	/* Return value */
	*pVal = ((double)val1) * 0x1000000 + (double) val0;

	return (0);
}

/*******************************************************************************
 *
 * pmacRamGetL - get DPRAM L word
 *
 * mantissa/2^35 * 2^(exp-$7ff)
 */
PMAC_LOCAL long pmacRamGetL
(
	PMAC_DPRAM *	pDpram,
	double *	pVal
)
{
	long	val0;
	long	val1;
	double		mantissa	= 0.0;
	long int	exponent	= 0;
	double	valD;

	/* Read PMAC DPRAM */
	pmacRamGet24U (pDpram, &val0);
	pmacRamGet24U (&(pDpram[4]), &val1);
	
	/* Convert 48 bit floating point format */
	mantissa = ((double)(val1 & 0x00000fff)) * 0x1000000 + (double) val0;
	exponent = ((val1 >> 12) & 0x00000fff) - 2082;  /* 0x7ff + 35 */
	
	if (mantissa == 0.0)
	{
		valD = 0.0;
	}
	else
	{
		valD = mantissa * pow (2.0, (double) exponent);
	}

	/* Return value */
	*pVal = valD;

	return (0);
}


/* ajf: Required DPRAM ASCII routines for Open/Close/Read/Write driver */

/* pmacAscIn - get characters from PMAC DPRAM ASCII */

PMAC_LOCAL int pmacAscIn( int ctlr, char *readbuf, char	*errmsg, int *numChar )
{
  int        i;
  int        status;
  int        length;
  int        errorFirst;
  int        errorSecond;
  int        errorThird;
  PMAC_CTLR  *pPmacCtlr;
  PMAC_DPRAM *dpramAsciiInControl;
  PMAC_DPRAM *dpramAsciiIn;
    
  dpramAsciiInControl = pmacRamAddr(ctlr,0x0F40);
  dpramAsciiIn        = pmacRamAddr(ctlr,0x0F44);
  pPmacCtlr           = &pmacVmeCtlr[ctlr];
  status              = 0; 
  errorFirst          = 0;
  errorSecond         = 0;
  errorThird          = 0;
  errmsg[0]           = NULL;
    
  if( *(dpramAsciiInControl+1)>>7 )  /* An error occurred */
  {
    errorFirst  = *dpramAsciiInControl      & 0xF;	
    errorSecond = (*dpramAsciiInControl>>4) & 0xF;
    errorThird  = *(dpramAsciiInControl+1)  & 0xF;
	
    sprintf(errmsg, "%d%d%d", errorThird, errorSecond, errorFirst);
	
    *numChar                 = length;
    *dpramAsciiInControl     = 0x0;
    *(dpramAsciiInControl+1) = 0x0;
    status                   = -2;
  }
  else
  {
    if( *dpramAsciiInControl != PMAC_TERM_CR )  /*only ack*/
      status = 1;
    else
    {
      pmacAscInCount(ctlr,&length);

      for( i=0; i<length-1; i++ )
      {
        readbuf[i]=*dpramAsciiIn;
        dpramAsciiIn++;
      }
      readbuf[i] = 0;
    }
    
    *numChar                 = length;
    *dpramAsciiInControl     = 0x0;
    *(dpramAsciiInControl+1) = 0x0;
  }
  return( status );
}


/* pmacAscRead - read response from PMAC DPRAM ASCII */

PMAC_LOCAL int pmacAscRead( int	ctlr, char *readbuf, char *errmsg )
{
  int       i;
  int       status;
  int       numChar;
  PMAC_CTLR *pPmacCtlr;

  i         = 0;
  status    = 0;
  pPmacCtlr = &pmacVmeCtlr[ctlr];

  while( status == 0 )
  {
    if( semTake( pPmacCtlr->ioAscReadmeSem, 120 ) )
    {
      printf( "pmacAscRead: semTake failed\n" );
      status = -1;
      break;
    }
    status  = pmacAscIn(ctlr, &readbuf[i], errmsg, &numChar);
    i      += numChar;
  }

  /* printf("pmacAscRead: Reading %s  from ASCII DPRAM\n", readbuf); */

  return( status );
}


 /* pmacAscWrite - write command to PMAC DPRAM ASCII */

PMAC_LOCAL int pmacAscWrite( int ctlr, char *writebuf )
{
  int        i;
  int        length;
  PMAC_CTLR  *pPmacCtlr;
  PMAC_DPRAM *dpramAsciiOut; 
  PMAC_DPRAM *dpramAsciiOutControl;

  /* printf("\npmacAscWrite: writing %s to DPRAM\n", writebuf); */

  pPmacCtlr            = &pmacVmeCtlr[ctlr];
  dpramAsciiOut        = pmacRamAddr(ctlr,0x0EA0);
  dpramAsciiOutControl = pmacRamAddr(ctlr,0x0E9C);
  length               = strlen( writebuf );
  i                    = 0;
    
  while( *dpramAsciiOutControl != 0x0 )
  {
    taskDelay(1);
    i++;
    if( i > 10 )
      printf( "pmacAscWrite: Stuck in while loop\n" );
  }

  for( i=0; (i < length) && (i < PMAC_BASE_ASC_REGS_OUT); i++ )
  {
    *dpramAsciiOut = writebuf[i];
    dpramAsciiOut++;
  }

  /* End of command */	
  *dpramAsciiOut = 0;
	
  /* Set output control bit to 1 for PMAC to read the command */
  *dpramAsciiOutControl = 1;

  return(0);
}


/* pmacAscInISR - Interrupt Service Routine which is called when
                  PMAC issues interrupt to say the DPRAM ASCII buffer
                  can be read */

PMAC_LOCAL void pmacAscInISR( PMAC_CTLR	*pPmacCtlr )
{
  semGive( pPmacCtlr->ioAscReadmeSem );
  return;
}


/* pmacAscInCount - Determine the number of characters in DPRAM ASCII buffer */

PMAC_LOCAL long pmacAscInCount( int ctlr, int *pVal )
{
  PMAC_DPRAM *pRam;

  pRam  = (PMAC_DPRAM *)((long) pmacVmeCtlr[ctlr].pDpramBase + 0xF42);
  *pVal = (int) *pRam;

  return(0);
}


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
      pmacAscDev[i].openFlag = FALSE;

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
  char errmsg[32];

  pmacAscRead( pPmacAscDev->ctlr, buffer, errmsg );

  return(0);
}


int pmacWriteAsc( PMAC_ASC_DEV *pPmacAscDev, char *buffer, int nBytes )
{
  pmacAscWrite( pPmacAscDev->ctlr, buffer );

  return(0);
}


int pmacIoctlAsc( PMAC_ASC_DEV *pPmacAscDev, int request, int *arg )
{
  int ret = 0;

  switch( request )
  {
    case FIONREAD:
      pmacAscInCount( pPmacAscDev->ctlr, arg );
      break;

    case FIORFLUSH:
      printf("FIORFLUSH: Flush any chars in buffers\n");
      break;

    case FIOCANCEL:
      printf("FIOCANCEL: Cancel read or write on the device\n");
      break;

    case SIO_HW_OPTS_GET:
      printf("SIO_HW_OPTS_GET: Get hardware options - not required\n");
      break;

    case SIO_BAUD_GET:
      printf("SIO_BAUD_GET: Get the baud rate - not required\n");
      break;

    case SIO_BAUD_SET:
      printf("SIO_BAUD_SET: Set the baud rate, why different to FIOBAUDRATE? - not required\n");
      break;

    case FIOBAUDRATE:
      printf("FIOBAUDRATE: Set serial baud rate\n");
      break;

    default:
      break;
  }
  return( ret );
}


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


long pmacVmeConfigSim
(
	int		ctlrNumber,
	unsigned long	addrBase,
	unsigned long	addrDpram,
	unsigned int	irqVector,
	unsigned int	irqLevel
)
{
	char *		MyName = "pmacVmeConfig";
	int		i;
	PMAC_CTLR *	pPmacCtlr;
	
	if (pmacVmeConfigLock != 0)
	{
		printf ( "%s: Cannot change configuration after initialization -- request ignored.\n",
			MyName);
		return (ERROR);
	}
  
	if ( pmacVmeNumCtlrs == 0 )
	{
		for (i=0; i < PMAC_MAX_CTLRS; i++ )
		{
			pmacVmeCtlr[i].configured = FALSE;
		}
	}
	
	if ( (ctlrNumber < 0) | (ctlrNumber >= PMAC_MAX_CTLRS) )
	{
		printf ("%s: Controller number %d invalid -- must be 0 to %d.\n",
			MyName, ctlrNumber, PMAC_MAX_CTLRS - 1);
		return(ERROR);
	}
  
	if (pmacVmeCtlr[ctlrNumber].configured)
	{
		printf ("%s: Controller %d already configured -- request ignored.\n",
			MyName, ctlrNumber);
		return(ERROR);
	}
	
	PMAC_DEBUG
	(	1,
		printf ("%s: Initializing controller %d.\n", MyName, ctlrNumber);
	)

	pPmacCtlr = &pmacVmeCtlr[ctlrNumber];
	pPmacCtlr->ctlr = ctlrNumber;
	pPmacCtlr->vmebusBase = addrBase;
	pPmacCtlr->irqVector = irqVector;
	pPmacCtlr->irqLevel = irqLevel;
	
	pPmacCtlr->enabled = FALSE;
	pPmacCtlr->present = FALSE;
	pPmacCtlr->active = FALSE;
	pPmacCtlr->enabledBase = TRUE;
	pPmacCtlr->presentBase = TRUE;
	pPmacCtlr->activeBase = FALSE;
	pPmacCtlr->enabledDpram = TRUE;
	pPmacCtlr->presentDpram = TRUE;
	pPmacCtlr->activeDpram = FALSE;
	pPmacCtlr->enabledGather = TRUE;
	pPmacCtlr->activeGather = FALSE;

	pPmacCtlr->vmebusDpram = addrDpram;
	if ( addrDpram == 0 )
	{
		pPmacCtlr->enabledDpram = FALSE;
	}
			
	pPmacCtlr->present = pPmacCtlr->presentBase | pPmacCtlr->presentDpram;
	pPmacCtlr->enabled = pPmacCtlr->enabledBase | pPmacCtlr->enabledDpram;
	pPmacCtlr->configured = TRUE;
	pmacVmeNumCtlrs++;
	
	return(0);
}
