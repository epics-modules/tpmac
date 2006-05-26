/*
 *  Implementation of the Open/Close/Read/Write/Ioctl interface to
 *  PMAC DPRAM ASCII and PMAC Mailbox ASCII.
 *
 *  Author: Andy Foster (for Diamond)
 *  Date:   26th May 2006
 *
*/

typedef struct
{
  DEV_HDR devHdr;
  int     ctlr;
  int     openFlag;
  int     cancelFlag;
} PMAC_ASC_DEV;

typedef struct
{
  DEV_HDR devHdr;
  int     ctlr;
  int     openFlag;
} PMAC_MBX_DEV;

/* Function prototypes */

int  pmacOpenAsc( PMAC_ASC_DEV * );
int  pmacCloseAsc( PMAC_ASC_DEV * );
int  pmacReadAsc( PMAC_ASC_DEV *, char *, int );
int  pmacWriteAsc( PMAC_ASC_DEV *, char *, int );
int  pmacIoctlAsc( PMAC_ASC_DEV *, int, int * );
int  pmacOpenMbx( PMAC_MBX_DEV * );
int  pmacCloseMbx( PMAC_MBX_DEV * );
int  pmacReadMbx( PMAC_MBX_DEV *, char *, int );
int  pmacWriteMbx( PMAC_MBX_DEV *, char *, int );
int  pmacIoctlMbx( PMAC_MBX_DEV *, int, int * );
long pmacVmeConfigSim( int, unsigned long, unsigned long, unsigned int, unsigned int );
