/* @(#) pmacVme.c 1.6 97/05/06 */

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

/* Local Includes */

#include	<pmacVme.h>

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

#define PMAC_BASE_MBX_REGS	(16)
#define PMAC_BASE_MBX_REGS_OUT	(15)
#define PMAC_BASE_MBX_REGS_IN	(16)

#define PMAC_STRLEN_FWVER	(31)

#define BYTESWAP(x) (MSB(x) | (LSB(x) << 8))

/*
 * TYPEDEFS
 */

typedef volatile char	PMAC_DPRAM_BASE;

typedef struct  /* PMAC_MBX_BASE */
{
	struct pmacBaseMbxStruct
	{
		struct { unsigned char unused; unsigned char data; } MB[PMAC_BASE_MBX_REGS];
		
	} mailbox;
} PMAC_MBX_BASE;

typedef struct  /* PMAC_CTLR */
{
	int		ctlr;
	int		configured;
	int		present;
	int		enabled;
	int		active;
	int		presentBase;
	int		enabledBase;
	int		activeBase;
	int		presentDpram;
	int		enabledDpram;
	int		activeDpram;
	int		enabledGather;
	int		activeGather;
	PMAC_MBX_BASE *		pBase;
	PMAC_DPRAM_BASE *	pDpramBase;
	unsigned	irqVector;
	unsigned	irqLevel;
	unsigned long	vmebusBase;
	unsigned long	vmebusDpram;
	SEM_ID		ioMbxLockSem;
	SEM_ID		ioMbxReceiptSem;
	SEM_ID		ioMbxReadmeSem;
	SEM_ID		ioAscLockSem;
	SEM_ID		ioAscReadmeSem;
	SEM_ID		ioGatBufferSem;
	char		firmwareVersion[PMAC_STRLEN_FWVER];
} PMAC_CTLR;


/*
 * FORWARD DECLARATIONS
 */

PMAC_LOCAL void pmacMbxReceiptISR (PMAC_CTLR * pPmacCtlr);
PMAC_LOCAL void pmacMbxReadmeISR (PMAC_CTLR * pPmacCtlr);
PMAC_LOCAL void pmacAscInISR (PMAC_CTLR * pPmacCtlr);
PMAC_LOCAL void pmacGatBufferISR (PMAC_CTLR * pPmacCtlr);

/*
 * GLOBALS
 */

char * pmacVmeVersion = "@(#) pmacVme.c 1.6 97/05/06";

#if PMAC_DIAGNOSTICS
volatile int	pmacVmeDebug = 2;		/* must be > 0 to see messages */
#endif

/*
 * LOCALS
 */

LOCAL int		pmacVmeConfigLock = 0;
PMAC_LOCAL int		pmacVmeNumCtlrs = 0;
PMAC_LOCAL PMAC_CTLR	pmacVmeCtlr[PMAC_MAX_CTLRS];

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
			
	status = devRegisterAddress ("PMAC BASE", atVMEA24,
				(void *) pPmacCtlr->vmebusBase, PMAC_MEM_SIZE_BASE,
				(void *) &(pPmacCtlr->pBase));
	if (!RTN_SUCCESS(status))
	{
		printf ("%s: Failure registering controller %d base address A24 %#010x.\n",
			MyName, pPmacCtlr->ctlr, (int)pPmacCtlr->vmebusBase);
		return (status);
	}
	
	/*Oleg: selecting lines A19-A14 for the DPRAM */
	pPmacCtlr->pBase->mailbox.MB[144].data = 0x3F & (pPmacCtlr->vmebusBase>>14);
		
	status = vxMemProbe ((char*) &pPmacCtlr->pBase->mailbox.MB[0].data,
				VX_READ, 1, (char*)&val);
	if (status != OK)
	{
		printf ("%s: Failure probing for base address.\n",
			MyName);
		return (status);
	}

	pPmacCtlr->presentBase = TRUE;

	if ( pPmacCtlr->enabledDpram )
	{
		status = devRegisterAddress ("PMAC DPRAM", atVMEA24,
					(void *) pPmacCtlr->vmebusDpram, PMAC_MEM_SIZE_DPRAM,
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
	
	pPmacCtlr->ioMbxReceiptSem = semBCreate (SEM_Q_FIFO, SEM_EMPTY);
	if ( pPmacCtlr->ioMbxReceiptSem == NULL)
	{
		status = S_dev_internal;
		printf ("%s: Failure creating binary semaphore.\n",
			MyName);
		return (status);
	}

	pPmacCtlr->ioMbxReadmeSem = semBCreate (SEM_Q_FIFO, SEM_EMPTY);
	if ( pPmacCtlr->ioMbxReceiptSem == NULL)
	{
		status = S_dev_internal;
		printf ("%s: Failure creating binary semaphore.\n",
			MyName);
		return (status);
	}

	pPmacCtlr->ioMbxLockSem = semBCreate (SEM_Q_FIFO, SEM_EMPTY);
	if ( pPmacCtlr->ioMbxLockSem == NULL)
	{
		status = S_dev_internal;
		printf ("%s: Failure creating binary semaphore.\n",
			MyName);
		return (status);
	}
			
	pPmacCtlr->ioAscReadmeSem = semBCreate (SEM_Q_FIFO, SEM_EMPTY);
	if ( pPmacCtlr->ioAscReadmeSem == NULL)
	{
		status = S_dev_internal;
		printf ("%s: Failure creating binary semaphore.\n",
			MyName);
		return (status);
	}

	pPmacCtlr->ioAscLockSem = semBCreate (SEM_Q_FIFO, SEM_EMPTY);
	if ( pPmacCtlr->ioAscLockSem == NULL)
	{
		status = S_dev_internal;
		printf ("%s: Failure creating binary semaphore.\n",
			MyName);
		return (status);
	}
			
	pPmacCtlr->ioGatBufferSem = semBCreate (SEM_Q_FIFO, SEM_EMPTY);
	if ( pPmacCtlr->ioGatBufferSem == NULL)
	{
		status = S_dev_internal;
		printf ("%s: Failure creating binary semaphore.\n",
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

	status = devConnectInterrupt (intVME, pPmacCtlr->irqVector + 1,
				pmacAscInISR, (void *) pPmacCtlr);
	if (!RTN_SUCCESS(status))
	{
		printf ("%s: Failure to connect interrupt.\n",
			MyName);
		return (status);
	}
			
	PMAC_DEBUG
	(	1,
		printf ("%s: Connecting to interrupt vector %d\n",
				MyName, pPmacCtlr->irqVector + 2);
	)

	status = devConnectInterrupt (intVME, pPmacCtlr->irqVector + 2,
				pmacGatBufferISR, (void *) pPmacCtlr);
	if (!RTN_SUCCESS(status))
	{
		printf ("%s: Failure to connect interrupt.\n",
			MyName);
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
		printf ("%s: enabledBase = %d, enabledDpram=%d, therfore enabled=%d\n",
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
				semGive (pPmacCtlr->ioMbxLockSem);	
				semGive (pPmacCtlr->ioAscLockSem);
			
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

	PMAC_DEBUG
	(	10,
		PMAC_MESSAGE ("%s: PMAC IRQ MbxReceipt for ctlr %d\n", MyName, pPmacCtlr->ctlr);
	)
	
	semGive (pPmacCtlr->ioMbxReceiptSem);
	
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
		PMAC_MESSAGE ("%s: PMAC IRQ MbxReadme for ctlr %d\n", MyName, pPmacCtlr->ctlr);
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
    /* char *	MyName = "pmacMbxRead"; */
    int		i;
    char	terminator;
    PMAC_CTLR	*pPmacCtlr;
    
    pPmacCtlr = &pmacVmeCtlr[ctlr];
    
    i = 0;
    terminator = 0;

    while ( terminator == 0)
    {
	pPmacCtlr->pBase->mailbox.MB[1].data = 0;
	semTake( pPmacCtlr->ioMbxReadmeSem, WAIT_FOREVER);
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
    /* char *	MyName = "pmacMbxWrite"; */
    int		i;
    char	terminator;
    PMAC_CTLR	*pPmacCtlr;
    
    pPmacCtlr = &pmacVmeCtlr[ctlr];

    i = 0;
    terminator = 0;

    while ( terminator == 0 ) 
    {
	terminator = pmacMbxOut (ctlr, &writebuf[i]);
	semTake (pPmacCtlr->ioMbxReceiptSem, WAIT_FOREVER);
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
    PMAC_CTLR	*pPmacCtlr;

    pPmacCtlr = &pmacVmeCtlr[ctlr];
	PMAC_DEBUG
	(	7,
		PMAC_MESSAGE ("%s: PMAC MBX LOCK %d\n", MyName, pPmacCtlr->ctlr);
	)
    
    semTake (pPmacCtlr->ioMbxLockSem, WAIT_FOREVER);
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
		PMAC_MESSAGE ("%s: PMAC MBX UNLOCK %d\n", MyName, pPmacCtlr->ctlr);
	)
    
    semGive (pPmacCtlr->ioMbxLockSem);
    return (0);
}

/*******************************************************************************
 *
 * pmacAscLock - Lock PMAC ASCII buffer for ctlr
 *
 */
long pmacAscLock
(
	int	ctlr
)
{
    /* char *	MyName = "pmacAscLock"; */
    PMAC_CTLR	*pPmacCtlr;

    pPmacCtlr = &pmacVmeCtlr[ctlr];
    
    semTake (pPmacCtlr->ioAscLockSem, WAIT_FOREVER);
    return (0);
}

/*******************************************************************************
 *
 * pmacAscUnlock - Unlock PMAC ASCII buffer for ctlr
 *
 */
long pmacAscUnlock
(
	int	ctlr
)
{
    /* char *	MyName = "pmacAscUnlock"; */
    PMAC_CTLR	*pPmacCtlr;

    pPmacCtlr = &pmacVmeCtlr[ctlr];
    
    semGive (pPmacCtlr->ioAscLockSem);
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
			      ctlr, pCtlr->pDpramBase, offset, pDpram);
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


/*******************************************************************************
 *
 * pmacAscInISR - ASCII Host-Input Buffer Ready Interrupt Service Routine
 *
 */
PMAC_LOCAL void pmacAscInISR
(
	PMAC_CTLR	*pPmacCtlr
)
{
	char *	MyName = "pmacAscInISR";

	PMAC_DEBUG
	(	10,
		PMAC_MESSAGE ("%s: PMAC IRQ AscIn for ctlr %d\n", MyName, pPmacCtlr->ctlr);
	)
	
	semGive (pPmacCtlr->ioAscReadmeSem);
	
	return;
}


/*******************************************************************************
 *
 * pmacAscH2p - Put ASCII Word
 *
 */
PMAC_LOCAL long pmacAscH2p
(
	int	ctlr,
	long	off,
	int	val
)
{
	PMAC_DPRAM * pRam;
	
	pRam = (PMAC_DPRAM *) ((long) pmacVmeCtlr[ctlr].pDpramBase + off);
	*pRam = (short) val;
	return (0);
}

/*******************************************************************************
 *
 * pmacAscP2h - Get Ascii word
 *
 */
PMAC_LOCAL long pmacAscP2h
(
	int	ctlr,
	long	off,
	int *	pVal
)
{
	PMAC_DPRAM * pRam;
	
	pRam = (PMAC_DPRAM *) ((long) pmacVmeCtlr[ctlr].pDpramBase + off);
	*pVal = (int) *pRam;
	return (0);
}

/*******************************************************************************
 *
 * pmacAscOutStatus - Read ASCII Host-Output-Control Flag
 *
 */
PMAC_LOCAL long pmacAscOutStatus
(
	int	ctlr,
	int *	pVal
)
{
	PMAC_DPRAM * pRam;
	
	pRam = (PMAC_DPRAM *) ((long) pmacVmeCtlr[ctlr].pDpramBase + 0xE9C);
	*pVal = (int) ((*pRam) & 0x0001);
	return (0);
}

/*******************************************************************************
 *
 * pmacAscOutComplete - Set ASCII Host Output Complete Flag
 *
 */
PMAC_LOCAL long pmacAscOutComplete
(
	int	ctlr
)
{
	PMAC_DPRAM * pRam;
	
	pRam = (PMAC_DPRAM *) ((long) pmacVmeCtlr[ctlr].pDpramBase + 0xE9C);
	*pRam = (short) 0x0001;
	return (0);
}

/*******************************************************************************
 *
 * pmacAscOutCtrlChar - Send ASCII Host-Output Control Character
 *
 */
PMAC_LOCAL long pmacAscOutCtrlChar
(
	int	ctlr,
	int	ctrlchar
)
{
	PMAC_DPRAM * pRam;
	
	pRam = (PMAC_DPRAM *) ((long) pmacVmeCtlr[ctlr].pDpramBase + 0xE9E);
	*pRam = ((short) ctrlchar) & 0x00ff;
	return (0);
}

/*******************************************************************************
 *
 * pmacAscOutString - Write ASCII Host-Output Transfer Buffer
 *
 */
PMAC_LOCAL long pmacAscOutString
(
	int	ctlr,
	char *	string
)
{
	PMAC_DPRAM *	pRam;
	short		charPair;
	short		buffer[PMAC_ASC_IN_BUFLEN/2];
	int		terminated;
	int		i;
	
	pRam = (PMAC_DPRAM *) ((long) pmacVmeCtlr[ctlr].pDpramBase + 0xEA0);

	buffer[(PMAC_ASC_OUT_BUFLEN/2)-1] = 0x0000;
	strncpy ( (char *) buffer, string, PMAC_ASC_OUT_BUFLEN - 1);

	terminated = FALSE;
	for (i=0; !terminated; i++)
	{
		charPair = buffer[i];
		pRam[i] = BYTESWAP (charPair);
		if ( (LSB(charPair) == NULL) || (MSB(charPair) == NULL) )
		{
			terminated = TRUE;
		}
	}

	return (0);
}

/*******************************************************************************
 *
 * pmacAscOutBuffer - Read ASCII Host-Output Transfer Buffer
 *
 */
PMAC_LOCAL long pmacAscOutBuffer
(
	int	ctlr,
	char *	string
)
{
	int		i;
	PMAC_DPRAM *	pRam;
	short		charPair;
	short		buffer[PMAC_ASC_OUT_BUFLEN/2];
	int		terminated;
	
	pRam = (PMAC_DPRAM *) ((long) pmacVmeCtlr[ctlr].pDpramBase + 0xEA0);
	
	terminated = FALSE;
	for (i=0; !terminated; i++)
	{
		charPair = pRam[i];
		buffer[i] = BYTESWAP (charPair);
		if ( (LSB(charPair) == NULL) || (MSB(charPair) == NULL) )
		{
			terminated = TRUE;
		}
	}
	strcpy (string, (char *) buffer);
	return (0);
}

/*******************************************************************************
 *
 * pmacAscInStatus - Read ASCII Host-Input Control Word
 *
 */
PMAC_LOCAL long pmacAscInStatus
(
	int	ctlr,
	int *	pVal
)
{
	PMAC_DPRAM * pRam;
	
	pRam = (PMAC_DPRAM *) ((long) pmacVmeCtlr[ctlr].pDpramBase + 0xF40);
	*pVal = (int) *pRam;
	return (0);
}

/*******************************************************************************
 *
 * pmacAscInEnable - Set ASCII Host-Input Enable
 *
 */
PMAC_LOCAL long pmacAscInEnable
(
	int	ctlr
)
{
	PMAC_DPRAM * pRam;
	
	pRam = (PMAC_DPRAM *) ((long) pmacVmeCtlr[ctlr].pDpramBase + 0xF40);
	*pRam = 0;
	return (0);
}

/*******************************************************************************
 *
 * pmacAscInCount - Determine Number Of ASCII Characters In Host-Input Buffer
 *
 */
PMAC_LOCAL long pmacAscInCount
(
	int	ctlr,
	int *	pVal
)
{
	PMAC_DPRAM * pRam;
	
	pRam = (PMAC_DPRAM *) ((long) pmacVmeCtlr[ctlr].pDpramBase + 0xF42);
	*pVal = (int) *pRam;
	return (0);
}

/*******************************************************************************
 *
 * pmacAscInString - Read ASCII Host-Input Transfer Buffer
 *
 */
PMAC_LOCAL long pmacAscInString
(
	int	ctlr,
	char *	string
)
{
	int		i;
	PMAC_DPRAM *	pRam;
	short		charPair;
	short		buffer[PMAC_ASC_IN_BUFLEN/2];
	int		terminated;
	
	pRam = (PMAC_DPRAM *) ((long) pmacVmeCtlr[ctlr].pDpramBase + 0xF44);
	
	terminated = FALSE;
	for (i=0; !terminated; i++)
	{
		charPair = pRam[i];
		buffer[i] = BYTESWAP (charPair);
		if ( (LSB(charPair) == NULL) || (MSB(charPair) == NULL) )
		{
			terminated = TRUE;
		}
	}
	strcpy (string, (char *) buffer);
	return (0);
}

/*******************************************************************************
 *
 * pmacGatBufferISR - interrupt service routine for DPRAM Data Gathering Buffer
 *
 */
PMAC_LOCAL void pmacGatBufferISR
(
	PMAC_CTLR	*pPmacCtlr
)
{
	char *	MyName = "pmacGatBufferISR";

	PMAC_DEBUG
	(	10,
		PMAC_MESSAGE ("%s: PMAC IRQ GatBuffer for ctlr %d\n", MyName, pPmacCtlr->ctlr);
	)
	
	semGive (pPmacCtlr->ioGatBufferSem);
	
	return;
}

/*******************************************************************************
 *
 * pmacGatBufferSem - semaphorel routine for DPRAM Data Gathering Buffer
 *
 */
PMAC_LOCAL void pmacGatBufferSem
(
	int	ctlr
)
{
	/* char * MyName = "pmacGatBufferSem"; */
	PMAC_CTLR	*pPmacCtlr;

	pPmacCtlr = &pmacVmeCtlr[ctlr];

	semTake( pPmacCtlr->ioGatBufferSem, WAIT_FOREVER);
	
	return;
}
