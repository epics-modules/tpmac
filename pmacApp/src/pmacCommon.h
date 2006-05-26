/*
 *  Implementation of the Open/Close/Read/Write/Ioctl interface to
 *  PMAC DPRAM ASCII and PMAC Mailbox ASCII.
 *
 *  Author: Andy Foster (for Diamond)
 *  Date:   26th May 2006
 *
*/

/* These routines are needed by pmacDriver.c and pmacVme.c */

STATUS pmacDrv();
void   pmacAscInISR( PMAC_CTLR * );
