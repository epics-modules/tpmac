/* drvPmac.c -  EPICS Device Driver Support for Turbo PMAC2-VME Ultralite
 * Author       Oleg A. Makarov
 *              Thomas A. Coleman's PMAC-VME driver was used as a prototype 
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
 * .02  7-3-97        wfl     added include of stdioLib.h
 * .03  7-15-03       oam     rewrited fot Turbo PMAC2 Ultralite
 * .04  26th May 2006 ajf     Addition of Open/Close/Read/Write/Ioctl interface.
 *                            Changed WAIT_TIMEOUT to WAIT_FOREVER in
 *                            semTake of Mailbox semaphore.
 */

/*
 * DESCRIPTION:
 * ------------
 * This module implements EPICS Device Driver Support for Turbo PMAC-VME Ultralite.
 *
 */

/*
 * INCLUDES
 */

/* VxWorks Includes */

#include	<vxWorks.h>
#include	<vxLib.h>
#include	<stdioLib.h>
#include	<sysLib.h>
#include	<taskLib.h>
#include	<iv.h>
#include	<math.h>
#include	<rngLib.h>
#include	<string.h>	/* Sergey */
#define __PROTOTYPE_5_0		/* Sergey */
#include	<logLib.h>	/* Sergey */


/* EPICS Includes */

#include	<dbDefs.h>
#include	<recSup.h>
#include	<devLib.h>
#include	<devSup.h>
#include	<drvSup.h>
#include	<errMdef.h>
#include	<taskwd.h>
#include	<callback.h>

#define PMAC_ASYN
#ifdef PMAC_ASYN
#include "asynDriver.h"
#include "asynOctetSyncIO.h"
#endif

/* local includes */

#include	<drvPmac.h>
#include "epicsExport.h"


/*
 * DEFINES
 */

#define PMAC_DIAGNOSTICS TRUE
#define PMAC_PRIVATE FALSE
#define vxTicksPerSecond (sysClkRateGet())	/*clock ticks per second*/

#if PMAC_PRIVATE
#define PMAC_LOCAL LOCAL
#else
#define PMAC_LOCAL
#endif

#if PMAC_DIAGNOSTICS
#define PMAC_MESSAGE	logMsg
#define PMAC_DEBUG(level,code)	{ if (drvPmacDebug >= (level)) { code } }
#else
#define PMAC_DEBUG(level,code)      ;
#endif

#define NO_ERR_STATUS	(-1)

#define FILE_TEXT_BUFLEN	(256)

#define PMAC_MEM_SIZE	0x7B31F
#define PMAC_DPRAM	0x60000

#define PMAC_MAX_CARDS	(PMAC_MAX_CTLRS)

#define PMAC_MAX_MTR	(256) /* 32 motors x 8 values = 256 */
#define	PMAC_MAX_BKG	(343) /* 16 C.S. x 18 values + 5 global = 293 */
#define	PMAC_MAX_VAR	(128) /* max allowed */
#define	PMAC_MAX_OPN	(480) /* */

#define PMAC_TASKNAME_LEN	15

#define PMAC_MBX_QUEUE_SIZE 1000

#define PMAC_MBX_SCAN		"pmacMbx"
#define PMAC_MBX_PRI		(45)
#define PMAC_MBX_OPT		(VX_FP_TASK)
#define PMAC_MBX_STACK		(8000)

#define PMAC_MTR_SCAN		"pmacMtr"
#define PMAC_MTR_PRI		(44)		/* OLeg 2005/11/15 (was 45) */
#define PMAC_MTR_RATE		(vxTicksPerSecond/10)
#define PMAC_MTR_OPT		(VX_FP_TASK)
#define PMAC_MTR_STACK		(8000)

#define PMAC_BKG_SCAN		"pmacBkg"
#define PMAC_BKG_PRI		(45)
#define PMAC_BKG_RATE		(vxTicksPerSecond/10)
#define PMAC_BKG_OPT		(VX_FP_TASK)
#define PMAC_BKG_STACK		(8000)

#define PMAC_VAR_SCAN		"pmacVar"
#define PMAC_VAR_PRI		(45)
#define PMAC_VAR_RATE		(vxTicksPerSecond/10)
#define PMAC_VAR_OPT		(VX_FP_TASK)
#define PMAC_VAR_STACK		(8000)

#define PMAC_OPN_SCAN		"pmacOpn"
#define PMAC_OPN_PRI		(46)		/* OLeg 2005/11/15 (was 45) */
#define PMAC_OPN_RATE		(vxTicksPerSecond*1)
#define PMAC_OPN_OPT		(VX_FP_TASK)
#define PMAC_OPN_STACK		(8000)

#define PMAC_DPRAM_MTR		1
#define PMAC_DPRAM_BKG		2
#define	PMAC_DPRAM_VAR		3
#define PMAC_DPRAM_OPN		4
#define PMAC_DPRAM_NONE		(-1)

#define PMAC_MEMTYP_Y		1
#define PMAC_MEMTYP_X		2
#define PMAC_MEMTYP_SY		3
#define PMAC_MEMTYP_SX		4
#define PMAC_MEMTYP_DP		5
#define PMAC_MEMTYP_D		6
#define PMAC_MEMTYP_F		7
#define PMAC_MEMTYP_L		8
#define PMAC_MEMTYP_HY		9
#define PMAC_MEMTYP_HX		10
#define PMAC_MEMTYP_NONE	(-1)

/* PMAC Hardware Constants */

#define PMAC_VARTYP_Y		0x00
#define PMAC_VARTYP_L		0x10
#define PMAC_VARTYP_X		0x20
#define PMAC_VARTYP_NONE	(-1)

/*
 * TYPEDEFS
 */

typedef struct  /* PMAC_DRVET */
{
	long		number;
	DRVSUPFUN	report;
	DRVSUPFUN	init;
} PMAC_DRVET;

typedef struct  /* PMAC_CARD */
{
	int		card;
	int		ctlr;
	int		configured;
	int		present;
	int		enabled;

	int		enabledMbx;
	int		enabledRam;
	int		enabledMtr;
	int		enabledBkg;
	int		enabledVar;
	int		enabledOpn;

	SEM_ID		scanMbxSem;

	RING_ID		scanMbxQ;

	volatile int	scanMtrRate;
	volatile int	scanBkgRate;
	volatile int	scanVarRate;
	volatile int	scanOpnRate;

	int		scanMbxTaskId;
	int		scanMtrTaskId;
	int		scanBkgTaskId;
	int		scanVarTaskId;
	int		scanOpnTaskId;

	char		scanMbxTaskName[PMAC_TASKNAME_LEN];
	char		scanMtrTaskName[PMAC_TASKNAME_LEN];
	char		scanBkgTaskName[PMAC_TASKNAME_LEN];
	char		scanVarTaskName[PMAC_TASKNAME_LEN];
	char		scanOpnTaskName[PMAC_TASKNAME_LEN];

	int		numMtrIo;
	int		numBkgIo;
	int		numVarIo;
	int		numOpnIo;
	int		numtimMT;
	int		numtimCS;
	int		numtimVB;

	PMAC_RAM_IO	MtrIo[PMAC_MAX_MTR];
	PMAC_RAM_IO	BkgIo[PMAC_MAX_BKG];
	PMAC_RAM_IO	VarIo[PMAC_MAX_VAR];
	PMAC_RAM_IO	OpnIo[PMAC_MAX_OPN];
	PMAC_RAM_IO	timMT[10];
	PMAC_RAM_IO	timCS[10];
	PMAC_RAM_IO	timVB[10];

#ifdef PMAC_ASYN
        asynUser *      pasynUser;
#endif
} PMAC_CARD;

/*
 * FORWARD DECLARATIONS
 */

PMAC_LOCAL long		drvPmacRamGetData (PMAC_RAM_IO * pRamIo);
PMAC_LOCAL long		drvPmacRamPutData (PMAC_RAM_IO * pRamIo);

PMAC_LOCAL long		drvPmac_report (int level);
PMAC_LOCAL long		drvPmac_init (void);

PMAC_LOCAL int		drvPmacMtrScan();
PMAC_LOCAL int		drvPmacBkgScan();
PMAC_LOCAL int		drvPmacVarScan();

PMAC_LOCAL void		drvPmacMtrScanInit (int card);
int			drvPmacMtrTask (int card);

PMAC_LOCAL void		drvPmacBkgScanInit (int card);
int			drvPmacBkgTask (int card);

PMAC_LOCAL void		drvPmacVarScanInit (int card);
int			drvPmacVarTask (int card);

PMAC_LOCAL void		drvPmacMbxScanInit (int card);
int			drvPmacMbxTask (int card);
char			drvPmacMbxWriteRead (int card, char * writebuf, char * readbuf, char * errmsg);

/*
 * GLOBALS
 */

char * drvPmacVersion = "@(#) drvPmac.c 2.0 2003/08/27";

#if PMAC_DIAGNOSTICS
volatile int	drvPmacDebug = 0;		/* must be > 0 to see messages */
#endif

/* EPICS Driver Support Entry Table */

PMAC_DRVET drvPmac =
{
	2,
	drvPmac_report,
	drvPmac_init,
};
epicsExportAddress(drvet,drvPmac);

/*
 * LOCALS
 */

LOCAL int		drvPmacConfigLock = 0;
PMAC_LOCAL int		drvPmacNumCards = 0;
PMAC_LOCAL PMAC_CARD	drvPmacCard[PMAC_MAX_CARDS];

/*******************************************************************************
 *
 * pmacDrvConfig - Configure PMAC-VME Driver
 *
 * This routine is to be called in the startup script in order to init the
 * driver options.
 *
 * By default there are no cards configured.
 *
 */
long pmacDrvConfig
(
	int		cardNumber,
	int		scanMtrRate,
	int		scanBkgRate,
	int		scanVarRate,
#ifdef PMAC_ASYN
	char *		asynMbxPort
#else
	int		disableMbx
#endif
)
{
	char *		MyName = "pmacDrvConfig";
	int		i;
	/* long		status; */
	PMAC_CARD	*pCard;

	if (drvPmacConfigLock != 0)
	{
		printf ( "%s: Cannot change configuration after initialization -- request ignored.",
			MyName);
		return (ERROR);
	}

	PMAC_DEBUG
       	(	1,
		PMAC_MESSAGE ("%s: drvPmacNumCards = %d  PMAC_MAX_CARDS = %d\n",
			      MyName, drvPmacNumCards, PMAC_MAX_CARDS,0,0,0);
	)

	if ( drvPmacNumCards == 0 )
	{
		for (i=0; i < PMAC_MAX_CARDS; i++ )
		{
			drvPmacCard[i].configured = FALSE;
		}
	}

	if ( (cardNumber < 0) | (cardNumber >= PMAC_MAX_CARDS) )
	{
		printf ("%s: Controller number %d invalid -- must be 0 to %d.",
			MyName, cardNumber, PMAC_MAX_CARDS - 1);
		return(ERROR);
	}

	if (drvPmacCard[cardNumber].configured)
	{
		printf ("%s: Controller %d already configured -- request ignored.",
			MyName, cardNumber);
		return(ERROR);
	}

	PMAC_DEBUG
	(	1,
		printf ("%s: Initializing card %d.\n", MyName, cardNumber);
	)

	pCard = &drvPmacCard[cardNumber];
	pCard->card = cardNumber;
	pCard->ctlr = cardNumber;
	pCard->numMtrIo = 0;
	pCard->numBkgIo = 0;
	pCard->numVarIo = 0;
	pCard->numOpnIo = 0;
	pCard->numtimCS = 0;
	pCard->numtimMT = 0;
	pCard->numtimVB = 0;

	pCard->scanMtrRate = PMAC_MTR_RATE;
	pCard->scanBkgRate = PMAC_BKG_RATE;
	pCard->scanVarRate = PMAC_VAR_RATE;
	pCard->scanOpnRate = PMAC_OPN_RATE;

	pCard->enabledMbx = TRUE;
	pCard->enabledRam = TRUE;
	pCard->enabledMtr = TRUE;
	pCard->enabledBkg = TRUE;
	pCard->enabledVar = TRUE;
	pCard->enabledOpn = TRUE;

	pCard->configured = TRUE;
	drvPmacNumCards++;
	
	/* clear motor masks for data reporting buffer */
	pmacRamPut16 ( pmacRamAddr(cardNumber,0x70), 0 );
	pmacRamPut16 ( pmacRamAddr(cardNumber,0x72), 0 );
	
	/* clear max C.S. number for C.S. data reporting buffer */
	pmacRamPut16 ( pmacRamAddr(cardNumber,0x674), 0 );

#ifdef PMAC_ASYN
        /* Setup connection to MBX port */
        pCard->enabledMbx = FALSE;
        if ( asynMbxPort != NULL )
        {
            asynStatus status;
            const char ack = PMAC_TERM_ACK;
 
            status = pasynOctetSyncIO->connect( asynMbxPort, 0, &(pCard->pasynUser), NULL);
            if (status) {
                printf( "pmacDrvConfig: unable to connect to asynPort %s\n", asynMbxPort );
                return ERROR;
            }

            status = pasynOctetSyncIO->setInputEos( pCard->pasynUser, &ack, 1 );
            if (status) {
                printf( "pmacDrvConfig: unable to set input EOS on %s: %s\n", 
                        asynMbxPort, pCard->pasynUser->errorMessage);
                pasynOctetSyncIO->disconnect(pCard->pasynUser);
                return ERROR;
            }

            status = pasynOctetSyncIO->setOutputEos( pCard->pasynUser, "\r", 1 );
            if (status) {
                printf( "pmacDrvConfig: unable to set output EOS on %s: %s\n", 
                        asynMbxPort, pCard->pasynUser->errorMessage);
                pasynOctetSyncIO->disconnect(pCard->pasynUser);
                return ERROR;
            }

            pCard->enabledMbx = TRUE;
        }
#endif

	return(0);
}

/*******************************************************************************
 *
 * drvPmac_report - print driver report information
 *
 */
PMAC_LOCAL long drvPmac_report
(
	int	level
)
{
	/* char *	MyName = "drvPmac_report"; */
	short int	i;
	/* short int	j; */
	PMAC_CARD *	pCard;

	if (level > 0)
	{
	        printf ("PMAC-VME driver: %s\n", drvPmacVersion);
		printf ("Number of cards configured = %d.\n", drvPmacNumCards);
        	for (i = 0; i < PMAC_MAX_CARDS; i++)
        	{
        		if ( drvPmacCard[i].configured )
              		{
				pCard = &drvPmacCard[i];

				printf ("card = %d  ctlr = %d  present = %d  enabled = %d\n",
					pCard->card, pCard->ctlr, pCard->present, pCard->enabled);
				printf ("    enabledMbx = %d  enabledRam = %d\n",
					pCard->enabledMbx, pCard->enabledRam);
				printf ("    enabledMtr = %d  enabledBkg = %d  enabledVar = %d\n",
					pCard->enabledMtr, pCard->enabledBkg, pCard->enabledVar);
				printf ("    enabledOpn = %d\n", pCard->enabledOpn);
				printf ("    numMtrIo = %d  numBkgIo = %d  numVarIo = %d  numOpnIo = %d\n",
					drvPmacCard[i].numMtrIo, drvPmacCard[i].numBkgIo,
					drvPmacCard[i].numVarIo, drvPmacCard[i].numOpnIo);
 			}

		}
 	}

 	if (level > 1)
 	{
 		printf ("Print even more info. The user may be prompted for options.\n");
 	}

 	return (0);
 }

/*******************************************************************************
 *
 * drvPmac_init - initialize PMAC driver
 *
 */
PMAC_LOCAL long drvPmac_init (void)
{
	/* char *	MyName = "drvPmac_init"; */
	int		status;

	drvPmacConfigLock = 1;

	status = pmacVmeInit ();

        /* ajf: Addition of the Open/Close/Read/Write/Ioctl interface */
/*         status = pmacDrv(); */

	return (status);
}

/*******************************************************************************
 *
 * drvPmacStartup - startup PMAC driver
 *
 */
PMAC_LOCAL long drvPmacStartup (void)
{
	char *		MyName = "drvPmacStartup";
	int		i;

	static	long	status = 0;
	static	int	oneTimeOnly = 0;

	if (oneTimeOnly != 0) return (status);

	oneTimeOnly = -1;

        for (i = 0; i < PMAC_MAX_CARDS; i++)
        {
                if ( drvPmacCard[i].configured )
		{

			PMAC_DEBUG
			(	1,
				PMAC_MESSAGE ("%s: Starting tasks for card %d.\n", MyName, i,0,0,0,0);
			)

			if ( drvPmacCard[i].enabledMbx )
			{
				drvPmacMbxScanInit (i);
			}

			if ( drvPmacCard[i].enabledMtr )
			{
				drvPmacMtrScanInit (i);
			}

			if ( drvPmacCard[i].enabledBkg )
			{
				drvPmacBkgScanInit (i);
			}

			if ( drvPmacCard[i].enabledVar )
			{
				drvPmacVarScanInit (i);
			}

		}
	}

	return (status);
}

/*******************************************************************************
 *
 * drvPmacRamDisable - Disable PMAC card DPRAM
 *
 */
long drvPmacRamDisable
(
	int	card
)
{
    /* char * MyName = "drvPmacRamDisable"; */
    PMAC_CARD	*pCard;

    pCard = &drvPmacCard[card];
    pCard->enabledRam = FALSE;

    return (0);
}

/*******************************************************************************
 *
 * drvPmacRamEnable - Enable PMAC card DPRAM
 *
 */
long drvPmacRamEnable
(
	int	card
)
{
    /* char * MyName = "drvPmacRamEnable"; */
    PMAC_CARD	*pCard;

    pCard = &drvPmacCard[card];
    pCard->enabledRam = pCard->configured;

    return (0);
}

/*******************************************************************************
 *
 * drvPmacMemSpecParse - parse a PMAC address specification into values
 *
 * specification is of the form <memType>:$<hexAddr> where valid <memType>
 * strings are Y, X, SY, SX, DP, D, F, L, HY and HX and <hexAddr> must be
 * in the range 0000 -> PMAC_MEM_SIZE.
 *
 * For example, "SX:$8000" is memory type "SX" and Hex address 0x8000.
 *
 */
long drvPmacMemSpecParse
(
	char	*pmacAdrSpec,
	int	*memType,
	int	*pmacAdr
)
{
	char *	MyName = "drvPmacMemSpecParse";
	int	parmCount;

	long	status;
	char	memTypeStr[11];

	parmCount = 0;
	memTypeStr[0] = '\0';
	*memType = PMAC_MEMTYP_NONE;
	*pmacAdr = 0;

	parmCount = sscanf (pmacAdrSpec, "%[^:]:$%x", memTypeStr, pmacAdr);

	PMAC_DEBUG
	(	1,
		PMAC_MESSAGE ("%s: parse '%s' results parmCount %d\n",
			MyName, pmacAdrSpec, parmCount,0,0,0);
		PMAC_MESSAGE ("%s: memTypeStr '%c' pmacAdr %x\n",
			MyName, memTypeStr[0], *pmacAdr,0,0,0);
	)

	if ( (parmCount != 2) )
	{
		status = S_dev_badInit;
		errPrintf (status, __FILE__, __LINE__,
			"%s: Improper address specification '%s'.",
			MyName, pmacAdrSpec);
		return (status);
	}

	if ( strcmp (memTypeStr,"Y") == 0)
	{
		*memType = PMAC_MEMTYP_Y;
	}
	else if ( strcmp (memTypeStr,"X") == 0 )
	{
		*memType = PMAC_MEMTYP_X;
	}
	else if ( strcmp (memTypeStr,"SY") == 0 )
	{
		*memType = PMAC_MEMTYP_SY;
	}
	else if ( strcmp (memTypeStr,"SX") == 0 )
	{
		*memType = PMAC_MEMTYP_SX;
	}
	else if ( strcmp (memTypeStr,"HY") == 0 )
	{
		*memType = PMAC_MEMTYP_HY;
	}
	else if ( strcmp (memTypeStr,"HX") == 0 )
	{
		*memType = PMAC_MEMTYP_HX;
	}
	else if ( strcmp (memTypeStr,"DP") == 0 )
	{
		*memType = PMAC_MEMTYP_DP;
	}
	else if ( strcmp (memTypeStr,"F") == 0 )
	{
		*memType = PMAC_MEMTYP_F;
	}
	else if ( strcmp (memTypeStr,"D") == 0 )
	{
		*memType = PMAC_MEMTYP_D;
	}
	else if ( strcmp (memTypeStr,"L") == 0 )
	{
		*memType = PMAC_MEMTYP_L;
	}
	else
	{
		status = S_dev_badInit;
		errPrintf (status, __FILE__, __LINE__,
			"%s: Illegal address type '%s' for '%s'.",
			MyName, memTypeStr, pmacAdrSpec);
		return (status);
	}

	PMAC_DEBUG
	(	1,
		PMAC_MESSAGE ("%s: memType %d\n", MyName, *memType,0,0,0,0);
	)

	if ( (*pmacAdr < 0) || (*pmacAdr > PMAC_MEM_SIZE) )
	{
		status = S_dev_badInit;
		errPrintf (status, __FILE__, __LINE__,
			"%s: Address %x out of range for '%s'.",
			MyName, (unsigned int)pmacAdr, pmacAdrSpec);
		return (status);
	}

	return (0);

}

/*******************************************************************************
 *
 * drvPmacDpramRequest - add PMAC DPRAM address to scan list
 *
 */
long drvPmacDpramRequest
(
	short	card,
	short	pmacAdrOfs,
	char	*pmacAdrSpec,
	void	(*pFunc)(void *),
	void	*pParm,
	PMAC_RAM_IO ** ppRamIo
)
{
	char *	MyName = "drvPmacDpramRequest";
	int	i;
	long	status;
	int	hostOfs;

	PMAC_CARD *	pCard;
	PMAC_RAM_IO *	pMtrIo;
	PMAC_RAM_IO *	pBkgIo;
	PMAC_RAM_IO *	pVarIo;
	PMAC_RAM_IO *	pOpnIo;
	PMAC_RAM_IO *	ptimCS;
	PMAC_RAM_IO *	ptimMT;
	PMAC_RAM_IO *	ptimVB;

	int	memType;
	int	pmacAdr;
	long 	num, mask;

	pCard = &drvPmacCard[card];

	/* Parse PMAC Address Specification */
	status = drvPmacMemSpecParse (pmacAdrSpec, &memType, &pmacAdr);
	if (!RTN_SUCCESS(status))
	{
		return (status);
	}

	/* Add PMAC Address Offset */
	pmacAdr += (long) pmacAdrOfs;
	hostOfs = 4 * (pmacAdr - PMAC_DPRAM);
	if (memType == PMAC_MEMTYP_HX) {
	  hostOfs += 2;
	}


	/* If Motor Data Reporting Buffer */
	if ( (pmacAdr >= PMAC_DPRAM + 0x1D) && (pmacAdr < PMAC_DPRAM + 0x19D) )
	{
		/* set motor mask registers */
		num = (short)(pmacAdr-PMAC_DPRAM-0x1D)/0xC;
		if (num < 16) { 
		  mask = 1 << num;
		  pmacRamGet16 ( pmacRamAddr(card,0x70), &status );
		  status |= mask;
		  pmacRamPut16 ( pmacRamAddr(card,0x70), status );
		}
		else {
		  mask = 1 << (num - 16);
		  pmacRamGet16 ( pmacRamAddr(card,0x72), &status );
		  status |= mask;
		  pmacRamPut16 ( pmacRamAddr(card,0x72), status );
		}
		
		i = pCard->numMtrIo;
		pMtrIo = &pCard->MtrIo[i];
		*ppRamIo = pMtrIo;

		pMtrIo->memType = memType;
		pMtrIo->pmacAdr = pmacAdr;
		pMtrIo->hostOfs = hostOfs;
		pMtrIo->pAddress = pmacRamAddr(card,pMtrIo->hostOfs);
		pMtrIo->pFunc = pFunc;
		pMtrIo->pParm = pParm;

		PMAC_DEBUG
		(	1,
			PMAC_MESSAGE ( "%s: Mtr -- index %d memType %d hostOfs %x pAddress %#010lx\n",
				MyName,
				i, pMtrIo->memType, pMtrIo->hostOfs, pMtrIo->pAddress,0);
		)

		pCard->numMtrIo++;
		if (pCard->numMtrIo >= PMAC_MAX_MTR) {
		  printf("%s: too many records refer to MtrIo %d \n",MyName,pCard->numMtrIo);
		  return -1;
		}
	}

	/* If Background Fixed Data Buffer */
	else if ( (pmacAdr >= PMAC_DPRAM + 0x19E) && (pmacAdr < PMAC_DPRAM + 0x3A7) )
	{
	
		/* update max CS number */
		num = (pmacAdr-PMAC_DPRAM-0x1A7)/0x20;
		pmacRamGet16 ( pmacRamAddr(card,0x674), &status );
		if (num >= (status & 0x1f)) {
		  status = num + 1;
		  pmacRamPut16 ( pmacRamAddr(card,0x674), status );
		}
		
		i = pCard->numBkgIo;
		pBkgIo = &pCard->BkgIo[i];
		*ppRamIo = pBkgIo;

		pBkgIo->memType = memType;
		pBkgIo->pmacAdr = pmacAdr;
		pBkgIo->hostOfs = hostOfs;
		pBkgIo->pAddress = pmacRamAddr(card,pBkgIo->hostOfs);
		pBkgIo->pFunc = pFunc;
		pBkgIo->pParm = pParm;

		PMAC_DEBUG
		(	1,
			PMAC_MESSAGE ( "%s: Bkg -- index %d memType %d hostOfs %x pAddress %#010lx\n",
				MyName,
				i, pBkgIo->memType, pBkgIo->hostOfs, pBkgIo->pAddress,0);
		)

		pCard->numBkgIo++;
		if (pCard->numBkgIo >= PMAC_MAX_BKG) {
		  printf("%s: too many records refer to BkgIo %d \n",MyName,pCard->numBkgIo);
		  return -1;
		}
	}

	/* If Background Variable Data Buffer -- Outside of DPRAM */
	else if ( (pmacAdr < PMAC_DPRAM) || (pmacAdr > PMAC_DPRAM + 0xFFF) )
	{
		i = pCard->numVarIo;
		pVarIo = &pCard->VarIo[i];
		*ppRamIo = pVarIo;

		pVarIo->memType = memType;
		pVarIo->pmacAdr = pmacAdr;
		pVarIo->hostOfs = 0;		/* Unknown at this time */
		pVarIo->pAddress = pmacRamAddr(card,pVarIo->hostOfs);
		pVarIo->pFunc = pFunc;
		pVarIo->pParm = pParm;

		PMAC_DEBUG
		(	1,
			PMAC_MESSAGE ( "%s: Var -- index %d memType %d\n",
				MyName, i, pVarIo->memType,0,0,0);
		)

		pCard->numVarIo++;
		if (pCard->numVarIo >= PMAC_MAX_VAR) {
		  printf("%s: too many records refer to VarIo %d \n",MyName,pCard->numVarIo);
		  return -1;
		}
	}
	/* If Background data reporting time stamp */
	else if (pmacAdr == PMAC_DPRAM + 0x19E)
	{
		i = pCard->numtimCS;
		ptimCS = &pCard->timCS[i];
		*ppRamIo = ptimCS;

		ptimCS->memType = memType;
		ptimCS->pmacAdr = pmacAdr;
		ptimCS->hostOfs = hostOfs;
		ptimCS->pAddress = pmacRamAddr(card,ptimCS->hostOfs);
		ptimCS->pFunc = pFunc;
		ptimCS->pParm = pParm;

		PMAC_DEBUG
		(	1,
			PMAC_MESSAGE ( "%s: timCS -- index %d memType %d hostOfs %x pAddress %#010lx\n",
				MyName,
				i, ptimCS->memType, ptimCS->hostOfs, ptimCS->pAddress,0);
		)

		pCard->numtimCS++;
		if (pCard->numtimCS >= 10) {
		  printf("%s: too many records refer to timCS %d \n",MyName,pCard->numtimCS);
		  return -1;
		}
	}
	/* If Motor data reporting time stamp */
	else if (pmacAdr == PMAC_DPRAM + 0x1B)
	{
		i = pCard->numtimMT;
		ptimMT = &pCard->timMT[i];
		*ppRamIo = ptimMT;

		ptimMT->memType = memType;
		ptimMT->pmacAdr = pmacAdr;
		ptimMT->hostOfs = hostOfs;
		ptimMT->pAddress = pmacRamAddr(card,ptimMT->hostOfs);
		ptimMT->pFunc = pFunc;
		ptimMT->pParm = pParm;

		PMAC_DEBUG
		(	1,
			PMAC_MESSAGE ( "%s: timMT -- index %d memType %d hostOfs %x pAddress %#010lx\n",
				MyName,
				i, ptimMT->memType, ptimMT->hostOfs, ptimMT->pAddress,0);
		)

		pCard->numtimMT++;
		if (pCard->numtimMT >= 10) {
		  printf("%s: too many records refer to timMT %d \n",MyName,pCard->numtimMT);
		  return -1;
		}
	}
	/* If Variable Buffer reporting time stamp */
	else if (pmacAdr == PMAC_DPRAM + 0x411)
	{
		i = pCard->numtimVB;
		ptimVB = &pCard->timVB[i];
		*ppRamIo = ptimVB;

		ptimVB->memType = memType;
		ptimVB->pmacAdr = pmacAdr;
		ptimVB->hostOfs = hostOfs;
		ptimVB->pAddress = pmacRamAddr(card,ptimVB->hostOfs);
		ptimVB->pFunc = pFunc;
		ptimVB->pParm = pParm;

		PMAC_DEBUG
		(	1,
			PMAC_MESSAGE ( "%s: timMT -- index %d memType %d hostOfs %x pAddress %#010lx\n",
				MyName,
				i, ptimVB->memType, ptimVB->hostOfs, ptimVB->pAddress,0);
		)

		pCard->numtimVB++;
		if (pCard->numtimVB >= 10) {
		  printf("%s: too many records refer to timVB %d \n",MyName,pCard->numtimVB);
		  return -1;
		}
	}

	/* If 	Control Panel Function (does not yet existing) Or 
		Motor Data Reporting Buffer Control Or
		Background Data reporting Buffer Control Or
		DPRAM ASCII Buffers Or
		Background Variable Read & Write Buffer Control Or
		Binary Rotary Program Buffer Control Or
		Data Gathering Control Or
		Variable-Sized Buffers / Open Use DPRAM */
	else if ( ( (pmacAdr >= PMAC_DPRAM) && (pmacAdr < PMAC_DPRAM + 0x1D) ) ||
		  ( (pmacAdr >= PMAC_DPRAM + 0x19D) && (pmacAdr < PMAC_DPRAM + 0x19F) ) ||
		  ( (pmacAdr >= PMAC_DPRAM + 0x3A7) && (pmacAdr < PMAC_DPRAM + 0x1000) ) )
	{
		i = pCard->numOpnIo;
		pOpnIo = &pCard->OpnIo[i];
		*ppRamIo = pOpnIo;

		pOpnIo->memType = memType;
		pOpnIo->pmacAdr = pmacAdr;
		pOpnIo->hostOfs = hostOfs;
		pOpnIo->pAddress = pmacRamAddr(card,pOpnIo->hostOfs);
		pOpnIo->pFunc = pFunc;
		pOpnIo->pParm = pParm;

		PMAC_DEBUG
		(	1,
			PMAC_MESSAGE ( "%s: Opn -- index %d memType %d hostOfs %x pAddress %#010lx\n",
				MyName,
				i, pOpnIo->memType, pOpnIo->hostOfs, pOpnIo->pAddress,0);
		)

		pCard->numOpnIo++;
		if (pCard->numOpnIo >= PMAC_MAX_OPN) {
		  printf("%s: too many records refer to OpnIo %d \n",MyName,pCard->numOpnIo);
		  return -1;
		}
	}
	else
	{
		status = S_dev_badRequest;
		errPrintf (status, __FILE__, __LINE__,
			"%s: Unsupported DPRAM address range %#06x.",
			MyName, pmacAdr);
		return (status);
	}

	return (0);
}

/*******************************************************************************
 *
 * drvPmacVarSetup - enable operation of background variable data buffer
 *
 */
long drvPmacVarSetup
(
	int	card
)
{
	char *	MyName = "drvPmacVarSetup";
	int	i;
	long	status;
	PMAC_CARD *	pCard	= &drvPmacCard[card];
	PMAC_RAM_IO *	pVarIo;
	int	hostOfs;
	int	configOfs;
	long	configFormat	= PMAC_VARTYP_NONE;

	/* Start Config Table At PMAC_DPRAM + 0x540 */
	/* Maximum 128 addresses + 128 Data Words */
	/* End of data at PMAC_DPRAM + 0xFFF */

	/* Determine First Data Location */
	configOfs = 4 * 0x540;
	hostOfs = 4 * (0x540 + pCard->numVarIo );

	/* Set Size Of Buffer To Zero Before Changing Buffer Configuration */
	status = pmacRamPut16 ( pmacRamAddr(card,0x1048), 0);

	/* Configure Starting Address Offset Of Buffer 0x540 = 0x450 + 0xF0*/
	status = pmacRamPut16 ( pmacRamAddr(card,0x104A), 0xF0);

	/* For Each Background Variable */
	for (i=0; i < pCard->numVarIo; i++)
	{
		/* Determine Location */
		pVarIo = &pCard->VarIo[i];
		pVarIo->hostOfs = hostOfs;
		pVarIo->pAddress = pmacRamAddr(card,pVarIo->hostOfs);

		/* Determine Memory Format */
		switch (pVarIo->memType)
		{
			case (PMAC_MEMTYP_Y) :
			case (PMAC_MEMTYP_SY) :
				configFormat = PMAC_VARTYP_Y;
				break;
			case (PMAC_MEMTYP_X) :
			case (PMAC_MEMTYP_SX) :
				configFormat = PMAC_VARTYP_X;
				break;
			case (PMAC_MEMTYP_D) :
			case (PMAC_MEMTYP_L) :
				configFormat = PMAC_VARTYP_L;
				break;
			default :
				status = ERROR;
				/* Oleg */
				errPrintf (status, __FILE__, __LINE__,
					"%s: Illegal address type '%d' at address 0x%x (index %d of %d).",
						MyName, pVarIo->memType,*pVarIo->pAddress,i,pCard->numVarIo);
				return (status);
				break;
		}

		/* Configure PMAC Address To Be Copied Into Buffer */
		status = pmacRamPut16 ( pmacRamAddr(card,configOfs), pVarIo->pmacAdr );
		status = pmacRamPut16 ( pmacRamAddr(card,configOfs + 2), configFormat );
/*testing for D:$208 - CmdPos of the motor #4 
                if (pVarIo->pmacAdr == 0x208) {
		  printf ("drvPmacVarSetup: i = %d, PMAC addr = $%x, config addr = 0x%x, location Addr Offset= $%x \n",
		  i, pVarIo->pmacAdr, (int) pmacRamAddr(card,configOfs), pVarIo->hostOfs / 4);
		}*/
		PMAC_DEBUG
		(	1,
			PMAC_MESSAGE ( "%s: configFormat %d memType %d hostOfs %x pAddress %#010lx\n",
				MyName,
				configFormat, pVarIo->memType,
				pVarIo->hostOfs, pVarIo->pAddress,0);
		)

		/* Determine Location Of Next Variable */
		configOfs += 4;
		if (configFormat == PMAC_VARTYP_L)
		{
			hostOfs += 8;
		}
		else
		{
			hostOfs += 4;
		}

	}

	/* Clear Data Ready Bit For Next Data Fill */
	status = pmacRamPut16 ( pmacRamAddr(card,0x1044), 0 );

	/* Configure Size Of Buffer */
	status = pmacRamPut16 ( pmacRamAddr(card,0x1048), pCard->numVarIo);

	return (0);
}

/*******************************************************************************
 *
 * drvPmacMtrRead - read fixed motor data buffer
 *
 */
long drvPmacMtrRead
(
	int	card
)
{
	char *	MyName = "drvPmacMtrRead";
	int	i;
	long	status;
	long	pmacStatus;
	PMAC_CARD *	pCard	= &drvPmacCard[card];
	PMAC_RAM_IO *	pMtrIo;
	PMAC_RAM_IO *	ptimMT;

	/* Set Host Busy Bit */
	status = pmacRamPut16 ( pmacRamAddr(card,0x6A), 0x8000 );
		/* Check PMAC Busy Bit */
		status = pmacRamGet16 ( pmacRamAddr(card,0x6E), &pmacStatus );

		PMAC_DEBUG
		(	5,
			PMAC_MESSAGE ("%s: PMAC waiting status %x\n", MyName, pmacStatus,0,0,0,0);
		)
/*	do
	{

	}
	while ( (pmacStatus & 0x00008000) != 0);*/

	/* Read PMAC Motor Fixed Data Buffer */
 	for (i=0; i < pCard->numMtrIo; i++)
 	{
		pMtrIo = &pCard->MtrIo[i];
		status = drvPmacRamGetData (pMtrIo);
	}

 	for (i=0; i < pCard->numtimMT; i++)
 	{
		ptimMT = &pCard->timMT[i];
		status = drvPmacRamGetData (ptimMT);
	}

	/* Clear Host Busy Bit */
	status = pmacRamPut16 ( pmacRamAddr(card,0x06A), 0);

	/* Notify Requester Of New Data */
	for (i=0; i < pCard->numMtrIo; i++)
	{
		if ( pCard->MtrIo[i].pFunc != (void *) NULL )
		{
			(*pCard->MtrIo[i].pFunc)(pCard->MtrIo[i].pParm);
		}
	}

	for (i=0; i < pCard->numtimMT; i++)
	{
		if ( pCard->timMT[i].pFunc != (void *) NULL )
		{
			(*pCard->timMT[i].pFunc)(pCard->timMT[i].pParm);
		}
	}

	return (0);
}

/*******************************************************************************
 *
 * drvPmacBkgRead - read PMAC card background fixed data buffer
 *
 */
long drvPmacBkgRead
(
	int	card
)
{
	char *	MyName = "drvPmacBkgRead";
	int	i;
	long	status;
	long	pmacStatus;
	PMAC_CARD *	pCard	= &drvPmacCard[card];
	PMAC_RAM_IO *	pBkgIo;
	PMAC_RAM_IO *	ptimCS;

	/* Check for PMAC Data Ready */
	status = pmacRamGet16 ( pmacRamAddr(card,0x067A), &pmacStatus );

	PMAC_DEBUG
	(	5,
		PMAC_MESSAGE ("%s: PMAC status %x\n", MyName, pmacStatus,0,0,0,0);
	)

	/* If No Data Ready Then Return Without Reading */
	if ( (pmacStatus & 0x8000) == 0)
	{
		return (0);
	}

	/* Read PMAC Background Fixed Data Buffer */
	for (i=0; i<pCard->numBkgIo; i++)
	{
		pBkgIo = &pCard->BkgIo[i];
		status = drvPmacRamGetData (pBkgIo);
	}

	for (i=0; i<pCard->numtimCS; i++)
	{
		ptimCS = &pCard->timCS[i];
		status = drvPmacRamGetData (ptimCS);
	}

	/* Clear Data Ready Bit For Next Data */
	status = pmacRamPut16 ( pmacRamAddr(card,0x67A), 0);

	/* Notify Requester Of New Data */
 	for (i=0; i < pCard->numBkgIo; i++)
	{
		if ( pCard->BkgIo[i].pFunc != (void *) NULL )
		{
			(*pCard->BkgIo[i].pFunc)(pCard->BkgIo[i].pParm);
		}
	}

 	for (i=0; i < pCard->numtimCS; i++)
	{
		if ( pCard->timCS[i].pFunc != (void *) NULL )
		{
			(*pCard->timCS[i].pFunc)(pCard->timCS[i].pParm);
		}
	}

	return (0);
}

/*******************************************************************************
 *
 * drvPmacVarRead - read PMAC card background var data buffer
 */
long drvPmacVarRead
(
	int	card
)
{
	char *	MyName = "drvPmacVarRead";
	int	i;
	long	status;
	long	pmacStatus;
	PMAC_CARD *	pCard	= &drvPmacCard[card];
	PMAC_RAM_IO *	pVarIo;
	PMAC_RAM_IO *	ptimVB;

	/* Check For PMAC Data Ready */
	status = pmacRamGet16 ( pmacRamAddr(card,0x1044), &pmacStatus );

	PMAC_DEBUG
	(	5,
		PMAC_MESSAGE ("%s: PMAC status %x\n", MyName, pmacStatus,0,0,0,0);
	)

	/* If No Data Ready Then Return Without Reading */
	if ( (pmacStatus & 0x0001) != 1) {
		return (0);
	}

	/* Read PMAC Background Variable Data Buffer */
	for (i=0; i<pCard->numVarIo; i++) {
		pVarIo = &pCard->VarIo[i];
		status = drvPmacRamGetData (pVarIo);
	}
	/* Read PMAC Background Variable Data Buffer time stamp*/
	for (i=0; i<pCard->numtimVB; i++) {
		ptimVB = &pCard->timVB[i];
		status = drvPmacRamGetData (ptimVB);
	}

	/* Clear PMAC Data Ready Bit For Next Data */
	status = pmacRamPut16 ( pmacRamAddr(card,0x1044), 0);

	/* Notify Requester Of New Data */
 	for (i=0; i < pCard->numVarIo; i++) {
		if ( pCard->VarIo[i].pFunc != (void *) NULL ) {
			(*pCard->VarIo[i].pFunc)(pCard->VarIo[i].pParm);
		}
	}
 	for (i=0; i < pCard->numtimVB; i++) {
		if ( pCard->timVB[i].pFunc != (void *) NULL ) {
			(*pCard->timVB[i].pFunc)(pCard->timVB[i].pParm);
		}
	}

	return (0);
}

/*******************************************************************************
 *
 * pmacMtrShow - print motor fixed data scan information
 *
 */
int pmacMtrShow
(
	int	card,
	int	index
)
{
	char *		MyName = "pmacMtrShow";
	PMAC_CARD *	pCard	= &drvPmacCard[card];
	PMAC_RAM_IO *	pMtrIo	= &pCard->MtrIo[index];

	printf ("%s: memType %d pmacAdr %X \n",
		MyName,
		pMtrIo->memType,
		pMtrIo->pmacAdr);
	printf ("%s: hostOfs %#x pAddress %#010x\n",
		MyName,
		pMtrIo->hostOfs,
		(int)pMtrIo->pAddress);
	printf ("%s: valLong %d valDouble %f\n",
		MyName,
		(int)pMtrIo->valLong,
		pMtrIo->valDouble );
	printf ("%s: pFunc %#010lx pParm %#010x\n",
		MyName,
		(long)pMtrIo->pFunc,
		(int)pMtrIo->pParm );

	return (0);
}


/*******************************************************************************
 *
 * pmacBkgShow - print background scan information
 *
 */
int pmacBkgShow
(
	int	card,
	int	index
)
{
	char *		MyName = "pmacBkgShow";
	PMAC_CARD *	pCard	= &drvPmacCard[card];
	PMAC_RAM_IO *	pBkgIo	= &pCard->BkgIo[index];

	printf ("%s: memType %d pmacAdr %X \n",
		MyName,
		pBkgIo->memType,
		pBkgIo->pmacAdr);
	printf ("%s: hostOfs %#x pAddress %#010x\n",
		MyName,
		pBkgIo->hostOfs,
		(int)pBkgIo->pAddress);
	printf ("%s: valLong %d valDouble %f\n",
		MyName,
		(int)pBkgIo->valLong,
		pBkgIo->valDouble );
	printf ("%s: pFunc %#010lx pParm %#010x\n",
		MyName,
		(long)pBkgIo->pFunc,
		(int)pBkgIo->pParm );

	return (0);
}


/*******************************************************************************
 *
 * pmacVarShow - print background scan information
 *
 */
int pmacVarShow
(
	int	card,
	int	index
)
{
	char *		MyName = "pmacVarShow";
	PMAC_CARD *	pCard	= &drvPmacCard[card];
	PMAC_RAM_IO *	pVarIo	= &pCard->VarIo[index];

	printf ("%s: memType %d pmacAdr %X \n",
		MyName,
		pVarIo->memType,
		pVarIo->pmacAdr);
	printf ("%s: hostOfs %#x pAddress %#010x\n",
		MyName,
		pVarIo->hostOfs,
		(int)pVarIo->pAddress);
	printf ("%s: valLong %d valDouble %f\n",
		MyName,
		(int)pVarIo->valLong,
		pVarIo->valDouble );
	printf ("%s: pFunc %#010lx pParm %#010x\n",
		MyName,
		(long)pVarIo->pFunc,
		(int)pVarIo->pParm );

	return (0);
}

/*******************************************************************************
 *
 * pmacOpnShow - print background scan information
 *
 */
int pmacOpnShow
(
	int	card,
	int	index
)
{
	char *		MyName = "pmacOpnShow";
	PMAC_CARD *	pCard	= &drvPmacCard[card];
	PMAC_RAM_IO *	pOpnIo	= &pCard->OpnIo[index];

	printf ("%s: memType %d pmacAdr %X \n",
		MyName,
		pOpnIo->memType,
		pOpnIo->pmacAdr);
	printf ("%s: hostOfs %#x pAddress %#010x\n",
		MyName,
		pOpnIo->hostOfs,
		(int)pOpnIo->pAddress);
	printf ("%s: valLong %d valDouble %f\n",
		MyName,
		(int)pOpnIo->valLong,
		pOpnIo->valDouble );
	printf ("%s: pFunc %#010lx pParm %#010x\n",
		MyName,
		(long)pOpnIo->pFunc,
		(int)pOpnIo->pParm );

	return (0);
}


/*******************************************************************************
 * drvPmacRamGetData - read data from PMAC DPRAM
 */
PMAC_LOCAL long drvPmacRamGetData ( PMAC_RAM_IO *	pRamIo ) {
	/* char * MyName = "drvPmacRamGetData"; */
	long	status;

	switch (pRamIo->memType) {
		case (PMAC_MEMTYP_Y) :
		case (PMAC_MEMTYP_X) :
			status = pmacRamGet24U (pRamIo->pAddress,
						&pRamIo->valLong);
			pRamIo->valDouble = (double) pRamIo->valLong;
			break;
		case (PMAC_MEMTYP_SY) :
		case (PMAC_MEMTYP_SX) :
			status = pmacRamGet24 (pRamIo->pAddress,
						&pRamIo->valLong);
			pRamIo->valDouble = (double) pRamIo->valLong;
			break;
		case (PMAC_MEMTYP_HY) :
		case (PMAC_MEMTYP_HX) :
			status = pmacRamGet16 (pRamIo->pAddress,
						&pRamIo->valLong);
			pRamIo->valDouble = (double) pRamIo->valLong;
			break;
		case (PMAC_MEMTYP_DP) :
			status = pmacRamGet24 (pRamIo->pAddress,
						&pRamIo->valLong);
			pRamIo->valDouble = (double) pRamIo->valLong;
			break;
		case (PMAC_MEMTYP_F) :
			status = pmacRamGetF (pRamIo->pAddress,
						&pRamIo->valDouble);
			pRamIo->valLong = 0;
			break;
		case (PMAC_MEMTYP_D) :
			status = pmacRamGetD (pRamIo->pAddress,
						&pRamIo->valDouble);
			pRamIo->valLong = 0;
			break;
		case (PMAC_MEMTYP_L) :
			status = pmacRamGetL (pRamIo->pAddress,
						&pRamIo->valDouble);
			pRamIo->valLong = 0;
			break;
	}

	return (0);
}

/*******************************************************************************
 * drvPmacRamPutData - write data to PMAC DPRAM
 */
PMAC_LOCAL long drvPmacRamPutData ( PMAC_RAM_IO *	pRamIo ) {
	/* char * MyName = "drvPmacRamPutData"; */
	long	status;

	switch (pRamIo->memType) {
		case (PMAC_MEMTYP_Y) :
		case (PMAC_MEMTYP_X) :
		case (PMAC_MEMTYP_SY) :
		case (PMAC_MEMTYP_SX) :
			status = pmacRamPut32 (pRamIo->pAddress,
						pRamIo->valLong);
			break;
		case (PMAC_MEMTYP_HY) :
		case (PMAC_MEMTYP_HX) :
			status = pmacRamPut16 (pRamIo->pAddress,
						pRamIo->valLong);
			break;
		case (PMAC_MEMTYP_DP) :
			status = pmacRamPut32 (pRamIo->pAddress,
						pRamIo->valLong);
			break;
		case (PMAC_MEMTYP_F) :
			status = pmacRamPutF (pRamIo->pAddress,
						pRamIo->valDouble);
			break;
	}

	return (0);
}

/*******************************************************************************
 *
 * drvPmacMbxWriteRead - write command and read response in PMAC mailbox
 *
 */
char drvPmacMbxWriteRead (
	int	card,
	char	*writebuf,
	char	*readbuf,
	char	*errmsg
) {
    /* char * MyName = "drvPmacMbxWriteRead"; */
    char	terminator;

#ifdef PMAC_ASYN
    asynStatus status;
    const double timeout=1.0;
    size_t nwrite, nread;
    int eomReason;
    asynUser * pasynUser = drvPmacCard[card].pasynUser;
    char * pEnd;

    status = pasynOctetSyncIO->writeRead( pasynUser,
                                          writebuf, strlen(writebuf),
                                          readbuf, PMAC_MBX_IN_BUFLEN,
                                          timeout,
                                          &nwrite, &nread, &eomReason );

    pEnd = &readbuf[strlen(readbuf)-1];
    
    if ( *pEnd != PMAC_TERM_CR || status )
    {
        if (status) asynPrintIO( pasynUser,
                                 ASYN_TRACE_ERROR,
                                 readbuf, nread,
                                 "Asyn read/write error to PMAC card %d, command=%s. Status=%d, Error=%s, Response:\n",
                                 card, writebuf, status, pasynUser->errorMessage);
        else asynPrintIO( pasynUser,
                          ASYN_TRACE_ERROR,
                          readbuf, nread,
                          "PMAC error on card %d, command=%s, Error=%s, Response:\n",
                          card, writebuf, pmacError( readbuf ) );

        strcpy( errmsg, readbuf );
        *readbuf = 0;
        terminator = PMAC_TERM_BELL;
    }
    else terminator = PMAC_TERM_ACK;
    
    if (*pEnd == PMAC_TERM_CR || *pEnd == PMAC_TERM_ACK || *pEnd == PMAC_TERM_BELL)
    {
	/* terminator = *pEnd; */
	*pEnd = NULL;
    }
        
#else
    char buffer[PMAC_MBX_IN_BUFLEN];

    pmacMbxLock (card);

    terminator = pmacMbxWrite (card, writebuf);
    terminator = pmacMbxRead (card, readbuf, errmsg);
    while ( terminator == PMAC_TERM_CR ) {
	 terminator = pmacMbxRead (card, buffer, errmsg);
    }

    pmacMbxUnlock (card);
#endif

    return (terminator);
}

/*******************************************************************************
 *
 * drvPmacMbxScan - put PMAC MBX request on queue
 *
 */
void drvPmacMbxScan
(
	PMAC_MBX_IO *	pMbxIo
)
{
	char *	MyName = "drvPmacMbxScan";
	PMAC_CARD *	pCard;

	pCard = &drvPmacCard[pMbxIo->card];

	if ( rngBufPut(pCard->scanMbxQ,(void *)&pMbxIo,sizeof(pMbxIo)) != sizeof(pMbxIo) )
	{
		errMessage (0,"drvPmacMbxScan: rngBufPut overflow.");
	}

	PMAC_DEBUG
	(	9,
		PMAC_MESSAGE ("%s: rngBufPut completed.\n", MyName,0,0,0,0,0);
	)

	semGive (pCard->scanMbxSem);
	return;
}

/*******************************************************************************
 *
 * drvPmacMbxScanInit - initialize PMAC MBX scan task
 *
 */
PMAC_LOCAL void drvPmacMbxScanInit
(
	int	card
)
{
	/* char * MyName = "drvPmacMbxScanInit"; */
	/* long	status; */

	PMAC_CARD *	pCard = &drvPmacCard[card];

	pCard->scanMbxQ = rngCreate(sizeof(void *) * PMAC_MBX_QUEUE_SIZE);

	if ( pCard->scanMbxQ == NULL )
	{
		errMessage (0, "drvPmacMbxScanInit: rngCreate failed");
		exit(1);
	}

	pCard->scanMbxSem = semBCreate(SEM_Q_FIFO,SEM_EMPTY);
	if ( pCard->scanMbxSem == NULL )
	{
		errMessage (0, "drvPmacMbxScanInit: semBcreate failed.");
	}
	else
	{
		sprintf ( pCard->scanMbxTaskName, "%s%d", PMAC_MBX_SCAN, pCard->card);
		pCard->scanMbxTaskId = taskSpawn ( pCard->scanMbxTaskName,
					PMAC_MBX_PRI, PMAC_MBX_OPT, PMAC_MBX_STACK,
					(FUNCPTR)drvPmacMbxTask,
					pCard->card,0,0,0,0,0,0,0,0,0 );
		taskwdInsert ((void*)pCard->scanMbxTaskId, NULL, 0L);

             /* epicsPrintf( "***** %s: created queue size of: %d\n",
                             pCard->scanMbxTaskName, PMAC_MBX_QUEUE_SIZE ); */
	}

	return;
}

/*******************************************************************************
 *
 * drvPmacMbxTask - task for PMAC MBX input/output
 *
 */
int drvPmacMbxTask
(
	int	card
)
{
	char *	MyName = "drvPmacMbxTask";
	/* long	status; */

	PMAC_CARD *	pCard = &drvPmacCard[card];

	PMAC_MBX_IO *		pMbxIo;

	FOREVER
	{
                /*ajf: Change WAIT_TIMEOUT to WAIT_FOREVER           */
                /*     to avoid repeated messages when we do not     */
                /*     have a database record connected to the ASCII */
                /*     mailboxes                                     */
		if ( semTake(pCard->scanMbxSem,WAIT_FOREVER) != OK )
		{
			errMessage(0,"drvPmacMbxTask: semTake returned error.");
		}

		while ( rngNBytes(pCard->scanMbxQ) >= sizeof(pMbxIo) )
		{
			if( rngBufGet(pCard->scanMbxQ,(void *)&pMbxIo,sizeof(pMbxIo)) != sizeof(pMbxIo) )
			{
				errMessage (0,"drvPmacMbxTask: rngBufGet returned error.");
			}
			else
			{
				PMAC_DEBUG
				(	6,
					PMAC_MESSAGE ("%s: rngBufGet completed.\n", MyName,0,0,0,0,0);
					PMAC_MESSAGE ("%s: card=%d command=[%s]\n", MyName,
						pMbxIo->card, pMbxIo->command,0,0,0);
				)

				pMbxIo->terminator = drvPmacMbxWriteRead (pMbxIo->card,
								pMbxIo->command, pMbxIo->response,
								pMbxIo->errmsg);

				if ( pMbxIo->terminator == PMAC_TERM_BELL )
				{
					PMAC_MESSAGE ("%s: PMAC Error=[%s]\n", MyName, pMbxIo->errmsg,0,0,0,0);
					PMAC_MESSAGE ("%s: card=%d command=[%s]\n", MyName,
						pMbxIo->card, pMbxIo->command,0,0,0);
					PMAC_MESSAGE ("%s: response=[%s]\n", MyName, pMbxIo->response,0,0,0,0);
				}

				PMAC_DEBUG
				(	6,
					PMAC_MESSAGE ("%s: response=[%s]\n", MyName, pMbxIo->response,0,0,0,0);
				)

				callbackRequest (&pMbxIo->callback);

				PMAC_DEBUG
				(	6,
					PMAC_MESSAGE ("%s: Callback requested.\n", MyName,0,0,0,0,0);
				)


			}

		}
	}
}

/*******************************************************************************
 *
 * drvPmacMtrTask - perform motor fixed buffer scanning
 *
 */
int drvPmacMtrTask
(
	int	card
)
{
	/* char * MyName = "drvPmacMtrTask"; */
	PMAC_CARD *	pCard = &drvPmacCard[card];

	FOREVER
	{
		if ( pCard->enabledMtr )
		{
			drvPmacMtrRead (pCard->card);
		}
		taskDelay (pCard->scanMtrRate);
	}
}

/*******************************************************************************
 *
 * drvPmacMtrScanInit - spawn task to perform scanning of the motor fixed buffer
 *
 */
PMAC_LOCAL void drvPmacMtrScanInit
(
	int	card
)
{
	/* char * MyName = "drvPmacMtrScanInit"; */
	/* long	status; */

	PMAC_CARD *	pCard = &drvPmacCard[card];

	sprintf ( pCard->scanMtrTaskName, "%s%d", PMAC_MTR_SCAN, pCard->card);
	pCard->scanMtrTaskId = taskSpawn ( pCard->scanMtrTaskName,
				PMAC_MTR_PRI, PMAC_MTR_OPT, PMAC_MTR_STACK,
				drvPmacMtrTask,
				pCard->card,0,0,0,0,0,0,0,0,0 );
	taskwdInsert ((void*)pCard->scanMtrTaskId, NULL, NULL);

	return;
}

/*******************************************************************************
 *
 * drvPmacBkgTask - perform background scanning
 *
 */
int drvPmacBkgTask
(
	int	card
)
{
	/* char * MyName = "drvPmacBkgTask"; */
	PMAC_CARD *	pCard = &drvPmacCard[card];

	FOREVER
	{
		if ( pCard->enabledBkg )
		{
			drvPmacBkgRead (pCard->card);
		}
		taskDelay (pCard->scanBkgRate);
	}
}

/*******************************************************************************
 *
 * drvPmacBkgScanInit - spawn task to perform background scanning
 *
 */
PMAC_LOCAL void drvPmacBkgScanInit
(
	int	card
)
{
	/* char * MyName = "drvPmacBkgScanInit"; */
	/* long	status; */

	PMAC_CARD *	pCard = &drvPmacCard[card];

	sprintf ( pCard->scanBkgTaskName, "%s%d", PMAC_BKG_SCAN, pCard->card);
	pCard->scanBkgTaskId = taskSpawn ( pCard->scanBkgTaskName,
				PMAC_BKG_PRI, PMAC_BKG_OPT, PMAC_BKG_STACK,
				drvPmacBkgTask,
				pCard->card,0,0,0,0,0,0,0,0,0 );
	taskwdInsert ((void*)pCard->scanBkgTaskId, NULL, NULL);

	return;
}

/*******************************************************************************
 *
 * drvPmacVarTask - perform background variable scanning
 *
 */
int drvPmacVarTask
(
	int	card
)
{
	/* char * MyName = "drvPmacVarTask"; */
	PMAC_CARD *	pCard = &drvPmacCard[card];

	FOREVER
	{
		if ( pCard->enabledVar )
		{
			drvPmacVarRead (pCard->card);
		}
		taskDelay (pCard->scanVarRate);
	}
}

/*******************************************************************************
 *
 * drvPmacVarScanInit - spawn task to perform background scanning
 *
 */
PMAC_LOCAL void drvPmacVarScanInit
(
	int	card
)
{
	/* char * MyName = "drvPmacVarScanInit"; */
	long	status;

	PMAC_CARD *	pCard = &drvPmacCard[card];

	status = drvPmacVarSetup (pCard->card);

	sprintf ( pCard->scanVarTaskName, "%s%d", PMAC_VAR_SCAN, pCard->card);
	pCard->scanVarTaskId = taskSpawn ( pCard->scanVarTaskName,
				PMAC_VAR_PRI, PMAC_VAR_OPT, PMAC_VAR_STACK,
				drvPmacVarTask,
				pCard->card,0,0,0,0,0,0,0,0,0 );
	taskwdInsert ((void*)pCard->scanVarTaskId, NULL, NULL);

	return;
}
