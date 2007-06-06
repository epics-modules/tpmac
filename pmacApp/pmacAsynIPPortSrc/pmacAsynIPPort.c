
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <cantProceed.h>
#include <epicsAssert.h>
#include <epicsStdio.h>
#include <epicsString.h>
#include <iocsh.h>

#include <epicsExport.h>
#include "asynDriver.h"
#include "asynOctet.h"
#include "pmacAsynIPPort.h"

#include <netinet/in.h>

#define ETHERNET_DATA_SIZE 1492
#define INPUT_SIZE        (ETHERNET_DATA_SIZE+1)  /* +1 to allow space to add terminating ACK */
#define ACK '\6'
#define CTRLB '\2'
#define CTRLC '\3'
#define CTRLF '\6'
#define CTRLG '\7'
#define CTRLP '\16'
#define CTRLV '\22'
#define CTRLX '\24'

/* PMAC ethernet command structure */
#pragma pack(1)
typedef struct tagEthernetCmd
{
    unsigned char RequestType;
    unsigned char Request;
    unsigned short wValue;
    unsigned short wIndex;
    unsigned short wLength; /* length of bData */
    unsigned char bData[ETHERNET_DATA_SIZE];
} ethernetCmd;
#pragma pack()

#define ETHERNET_CMD_HEADER ( sizeof(ethernetCmd) - ETHERNET_DATA_SIZE )

/* PMAC ethernet commands - RequestType field */
#define VR_UPLOAD   0xC0
#define VR_DOWNLOAD 0x40

/* PMAC ethernet commands - Request field */
#define VR_PMAC_SENDLINE    0xB0
#define VR_PMAC_GETLINE     0xB1
#define VR_PMAC_FLUSH       0xB3
#define VR_PMAC_GETMEM      0xB4
#define VR_PMAC_SETMEN      0xB5
#define VR_PMAC_SETBIT      0xBA
#define VR_PMAC_SETBITS     0xBB
#define VR_PMAC_PORT        0xBE
#define VR_PMAC_GETRESPONSE 0xBF
#define VR_PMAC_READREADY   0xC2
#define VR_CTRL_RESPONSE    0xC4
#define VR_PMAC_GETBUFFER   0xC5
#define VR_PMAC_WRITEBUFFER 0xC6
#define VR_PMAC_WRITEERROR  0xC7
#define VR_FWDOWNLOAD       0xCB
#define VR_IPADDRESS        0xE0

/* PMAC control character commands (VR_CTRL_RESPONSE cmd) */
static char ctrlCommands[] = { CTRLB,CTRLC,CTRLF,CTRLG,CTRLP,CTRLV };

typedef struct pmacPvt {
    char          *portName;
    int           addr;
    asynInterface pmacInterface;
    asynOctet     *poctet;  /* The methods we're overriding */
    void          *octetPvt;
    asynUser      *pasynUser;     /* For connect/disconnect reporting */
    ethernetCmd   *poutCmd;
    ethernetCmd   *pinCmd;
    char          *inBuf;
    unsigned int  inBufHead;
    unsigned int  inBufTail;
    } pmacPvt;

/* NOTES
   use asynUser.timeout to specify I/O request timeouts in seconds
   asynStatus may return asynSuccess,asynTimeout,asynOverflow or asynError
   asynError indicates that asynUser.errorMessage has been populated by epicsSnprintf(). 
*/  

/* Connect/disconnect handling */
static void pmacInExceptionHandler(asynUser *pasynUser,asynException exception);

/* asynOctet methods */
static asynStatus writeIt(void *ppvt,asynUser *pasynUser,
    const char *data,size_t numchars,size_t *nbytesTransfered);
static asynStatus writeRaw(void *ppvt,asynUser *pasynUser,
    const char *data,size_t numchars,size_t *nbytesTransfered);
static asynStatus readIt(void *ppvt,asynUser *pasynUser,
    char *data,size_t maxchars,size_t *nbytesTransfered,int *eomReason);
static asynStatus readRaw(void *ppvt,asynUser *pasynUser,
    char *data,size_t maxchars,size_t *nbytesTransfered,int *eomReason);
static asynStatus flushIt(void *ppvt,asynUser *pasynUser);
static asynStatus registerInterruptUser(void *ppvt,asynUser *pasynUser,
    interruptCallbackOctet callback, void *userPvt,void **registrarPvt);
static asynStatus cancelInterruptUser(void *drvPvt,asynUser *pasynUser,
     void *registrarPvt);
static asynStatus setInputEos(void *ppvt,asynUser *pasynUser,
    const char *eos,int eoslen);
static asynStatus getInputEos(void *ppvt,asynUser *pasynUser,
    char *eos,int eossize ,int *eoslen);
static asynStatus setOutputEos(void *ppvt,asynUser *pasynUser,
    const char *eos,int eoslen);
static asynStatus getOutputEos(void *ppvt,asynUser *pasynUser,
    char *eos,int eossize,int *eoslen);
static asynOctet octet = {
    writeIt,writeRaw,readIt,readRaw,flushIt,
    registerInterruptUser, cancelInterruptUser,
    setInputEos,getInputEos,setOutputEos,getOutputEos
};

static asynStatus readResponse(pmacPvt *pPmacPvt, asynUser *pasynUser, size_t *nbytesTransfered, int *eomReason );
static int pmacReadReady(pmacPvt *pPmacPvt, asynUser *pasynUser );
static int pmacFlush(pmacPvt *pPmacPvt, asynUser *pasynUser );

epicsShareFunc int pmacAsynIPPortConfig(const char *portName,int addr)
{
    pmacPvt       *pPmacPvt;
    asynInterface *plowerLevelInterface;
    asynStatus    status;
    asynUser      *pasynUser;
    size_t        len;

    len = sizeof(pmacPvt) + strlen(portName) + 1;
    pPmacPvt = callocMustSucceed(1,len,"asynInterposePmacConfig");
    pPmacPvt->portName = (char *)(pPmacPvt+1);
    strcpy(pPmacPvt->portName,portName);
    pPmacPvt->addr = addr;
    pPmacPvt->pmacInterface.interfaceType = asynOctetType;
    pPmacPvt->pmacInterface.pinterface = &octet;
    pPmacPvt->pmacInterface.drvPvt = pPmacPvt;
    pasynUser = pasynManager->createAsynUser(0,0);
    pPmacPvt->pasynUser = pasynUser;
    pPmacPvt->pasynUser->userPvt = pPmacPvt;
    status = pasynManager->connectDevice(pasynUser,portName,addr);
    if(status!=asynSuccess) {
        printf("%s connectDevice failed\n",portName);
        pasynManager->freeAsynUser(pasynUser);
        free(pPmacPvt);
        return -1;
    }
    status = pasynManager->exceptionCallbackAdd(pasynUser,pmacInExceptionHandler);
    if(status!=asynSuccess) {
        printf("%s exceptionCallbackAdd failed\n",portName);
        pasynManager->freeAsynUser(pasynUser);
        free(pPmacPvt);
        return -1;
    }
    status = pasynManager->interposeInterface(portName,addr,
       &pPmacPvt->pmacInterface,&plowerLevelInterface);
    if(status!=asynSuccess) {
        printf("%s interposeInterface failed\n",portName);
        pasynManager->exceptionCallbackRemove(pasynUser);
        pasynManager->freeAsynUser(pasynUser);
        free(pPmacPvt);
        return -1;
    }
    pPmacPvt->poctet = (asynOctet *)plowerLevelInterface->pinterface;
    pPmacPvt->octetPvt = plowerLevelInterface->drvPvt;

    pPmacPvt->poutCmd = callocMustSucceed(1,sizeof(ethernetCmd),"asynInterposePmacConfig");
    pPmacPvt->pinCmd = callocMustSucceed(1,sizeof(ethernetCmd),"asynInterposePmacConfig");
    
    pPmacPvt->inBuf = callocMustSucceed(1,INPUT_SIZE,"asynInterposePmacConfig");
    
    pPmacPvt->inBufHead = 0;
    pPmacPvt->inBufTail = 0;

    return(0);
}

static void pmacInExceptionHandler(asynUser *pasynUser,asynException exception)
{
    pmacPvt *pPmacPvt = (pmacPvt *)pasynUser->userPvt;
    asynPrint(pasynUser,ASYN_TRACE_FLOW, "pmacInExceptionHandler\n");

    if (exception == asynExceptionConnect) {
        pPmacPvt->inBufHead = 0;
        pPmacPvt->inBufTail = 0;
    }
}

/*
    Read reponse data from PMAC into cyclic pmacPvt.inBuf. If there is no data in the asyn buffer then issue
    GETBUFFER command to get any outstanding data still on the PMAC.
*/
static asynStatus readResponse(pmacPvt *pPmacPvt, asynUser *pasynUser, size_t *nbytesTransfered, int *eomReason )
{
    ethernetCmd* inCmd;
    asynStatus status = asynSuccess;
    size_t thisRead;
    *nbytesTransfered = 0;

    status = pPmacPvt->poctet->readRaw(pPmacPvt->octetPvt,
         pasynUser,pPmacPvt->inBuf,INPUT_SIZE,&thisRead,eomReason);
         
    if(status==asynSuccess) {
        asynPrintIO(pasynUser,ASYN_TRACE_FLOW,pPmacPvt->inBuf,thisRead,
             "%s read\n",pPmacPvt->portName);
             
        if (thisRead == 0 && *eomReason == ASYN_EOM_END) {
            /* failed to read any characters into the input buffer, 
               check for more response data on the PMAC */
            if ( pmacReadReady(pPmacPvt,pasynUser )) { 
                /* response data is available on the PMAC */
                /* issue a getbuffer command */
                inCmd = pPmacPvt->pinCmd;
                inCmd->RequestType = VR_UPLOAD;
                inCmd->Request = VR_PMAC_GETBUFFER;
                inCmd->wValue = 0;
                inCmd->wIndex = 0;
                inCmd->wLength = htons(0);
                status = pPmacPvt->poctet->writeRaw(pPmacPvt->octetPvt,
                    pasynUser,(char*)pPmacPvt->pinCmd,ETHERNET_CMD_HEADER,nbytesTransfered);
                    
                asynPrintIO(pasynUser,ASYN_TRACE_FLOW,(char*)pPmacPvt->pinCmd,ETHERNET_CMD_HEADER,
                    "%s write GETBUFFER\n",pPmacPvt->portName);
                
                status = pPmacPvt->poctet->readRaw(pPmacPvt->octetPvt,
                    pasynUser,pPmacPvt->inBuf,INPUT_SIZE,&thisRead,eomReason);
                    
                if(status==asynSuccess) {
                    asynPrintIO(pasynUser,ASYN_TRACE_FLOW,pPmacPvt->inBuf,thisRead,
                       "%s read\n",pPmacPvt->portName);
                }
            }
        }
    }    
         
    if(status==asynSuccess) {
        *nbytesTransfered = thisRead;   
        pPmacPvt->inBufTail = 0;
        pPmacPvt->inBufHead = thisRead;
    }
    
    return status; 
}


/*
   Send ReadReady command to PMAC to discover if there is any data to read from it.
   Returns: 0 - no data available
            1 - data available
*/            
static int pmacReadReady(pmacPvt *pPmacPvt, asynUser *pasynUser )
{
    ethernetCmd cmd;
    unsigned char data[2];
    asynStatus status;
    size_t thisRead;
    size_t nbytesTransfered = 0;
    int eomReason = 0;
    int retval = 0;

    cmd.RequestType = VR_UPLOAD;
    cmd.Request = VR_PMAC_READREADY;
    cmd.wValue = 0;
    cmd.wIndex = 0;
    cmd.wLength = htons(2);
    status = pPmacPvt->poctet->writeRaw(pPmacPvt->octetPvt,
        pasynUser,(char*)&cmd,ETHERNET_CMD_HEADER,&nbytesTransfered);
    
    asynPrintIO(pasynUser,ASYN_TRACE_FLOW,(char*)&cmd,ETHERNET_CMD_HEADER,
        "%s write pmacReadReady\n",pPmacPvt->portName);
     
    status = pPmacPvt->poctet->readRaw(pPmacPvt->octetPvt,
         pasynUser,data,2,&thisRead,&eomReason);

    if(status==asynSuccess) {
         if (thisRead==2 && data[0] != 0) {
             retval = 1;
         }    
         asynPrintIO(pasynUser,ASYN_TRACE_FLOW,data,thisRead,
             "%s read pmacReadReady OK\n",pPmacPvt->portName);
    } else {
        asynPrint(pasynUser,ASYN_TRACE_FLOW, "%s pmacReadReady failed",pPmacPvt->portName);
    }
    return retval;            
}

/*
   Send Flush command to PMAC and wait for confirmation ctrlX to be returned. 
   Returns: 0 - failed
            1 - success
*/            
static int pmacFlush(pmacPvt *pPmacPvt, asynUser *pasynUser )
{
    ethernetCmd cmd;
    unsigned char data[2];
    asynStatus status = asynSuccess;
    size_t thisRead;
    size_t nbytesTransfered = 0;
    int eomReason = 0;
    int retval = 0;

    cmd.RequestType = VR_DOWNLOAD;
    cmd.Request = VR_PMAC_FLUSH;
    cmd.wValue = 0;
    cmd.wIndex = 0;
    cmd.wLength = 0;
    status = pPmacPvt->poctet->writeRaw(pPmacPvt->octetPvt,
        pasynUser,(char*)&cmd,ETHERNET_CMD_HEADER,&nbytesTransfered);
    
    asynPrintIO(pasynUser,ASYN_TRACE_FLOW,(char*)&cmd,ETHERNET_CMD_HEADER,
        "%s write FLUSH\n",pPmacPvt->portName);
    
    do { 
        status = pPmacPvt->poctet->readRaw(pPmacPvt->octetPvt,
         pasynUser,data,1,&thisRead,&eomReason);
    } while (status==asynSuccess && data[0] != CTRLX); 

    if(status==asynSuccess) {
         asynPrintIO(pasynUser,ASYN_TRACE_FLOW,data,thisRead,
             "%s read pmacFlush OK\n",pPmacPvt->portName);
         retval = 1;    
    } else {
        asynPrint(pasynUser,ASYN_TRACE_FLOW, "%s read pmacFlush failed\n",pPmacPvt->portName);
    }

    pPmacPvt->inBufTail = 0;
    pPmacPvt->inBufHead = 0;
    
    return retval;            
}


/* asynOctet methods */

static asynStatus writeIt(void *ppvt,asynUser *pasynUser,
    const char *data,size_t numchars,size_t *nbytesTransfered)
{
    asynStatus status;
    ethernetCmd* outCmd;
    pmacPvt *pPmacPvt = (pmacPvt *)ppvt;
    size_t nbytesActual = 0;
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "interposePmac::writeIt\n" );
    assert(pPmacPvt);
    
    /* TODO need to scan the data buffer for control commands and do PMAC_GETRESPONSE and CTRL_RESPONSE commands
       as necessary, currently we assume control characters arrive as individual calls to this routine */
    outCmd = pPmacPvt->poutCmd;
    if (numchars==1 && strchr(ctrlCommands,data[0])){
        outCmd->RequestType = VR_UPLOAD;
        outCmd->Request = VR_CTRL_RESPONSE;
        outCmd->wValue = data[0];
        outCmd->wIndex = 0;
        outCmd->wLength = htons(0);
        status = pPmacPvt->poctet->writeRaw(pPmacPvt->octetPvt,
        pasynUser,(char*)pPmacPvt->poutCmd,ETHERNET_CMD_HEADER,&nbytesActual);
        *nbytesTransfered = (nbytesActual==ETHERNET_CMD_HEADER) ? numchars : 0;
    } else {
        if (numchars > ETHERNET_DATA_SIZE) {
            /* TODO call write multiple times for large data blocks */
            numchars = ETHERNET_DATA_SIZE;
            asynPrint( pasynUser, ASYN_TRACE_FLOW, "writeIt - ERROR TRUNCATED\n" );
        }
        outCmd->RequestType = VR_DOWNLOAD;
        outCmd->Request = VR_PMAC_GETRESPONSE;
        outCmd->wValue = 0;
        outCmd->wIndex = 0;
        outCmd->wLength = htons(numchars);
        memcpy(outCmd->bData,data,numchars);
        status = pPmacPvt->poctet->writeRaw(pPmacPvt->octetPvt,
            pasynUser,(char*)pPmacPvt->poutCmd,numchars+ETHERNET_CMD_HEADER,&nbytesActual);
        *nbytesTransfered = (nbytesActual>ETHERNET_CMD_HEADER) ? (nbytesActual - ETHERNET_CMD_HEADER) : 0;
    }
        
    asynPrintIO(pasynUser,ASYN_TRACE_FLOW,(char*)pPmacPvt->poutCmd,numchars+ETHERNET_CMD_HEADER,
            "%s write\n",pPmacPvt->portName);
                    
    return status;
}


static asynStatus writeRaw(void *ppvt,asynUser *pasynUser,
    const char *data,size_t numchars,size_t *nbytesTransfered)
{
    pmacPvt *pPmacPvt = (pmacPvt *)ppvt;
    asynStatus status = asynSuccess;
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "interposePmac::writeRaw\n" );
    assert(pPmacPvt);

    status = pPmacPvt->poctet->writeRaw(pPmacPvt->octetPvt,
        pasynUser,data,numchars,nbytesTransfered);
        
    return status;    
}

static asynStatus readIt(void *ppvt,asynUser *pasynUser,
    char *data,size_t maxchars,size_t *nbytesTransfered,int *eomReason)
{
    pmacPvt *pPmacPvt = (pmacPvt *)ppvt;
    asynStatus status = asynSuccess;
    size_t thisRead;
    int nRead = 0;
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "interposePmac::readIt\n" );
    assert(pPmacPvt);
    
    if(eomReason) *eomReason = 0;
    for (;;) {
        if ((pPmacPvt->inBufTail != pPmacPvt->inBufHead)) {
            *data++ = pPmacPvt->inBuf[pPmacPvt->inBufTail++];
            nRead++;
            if (nRead >= maxchars) break;
            continue;
        }
        if(eomReason && *eomReason) break;
        status = readResponse(pPmacPvt, pasynUser, &thisRead, eomReason);
        
        if (*eomReason == ASYN_EOM_END){
            /* append <ACK> terminator */
            assert(pPmacPvt->inBufHead < INPUT_SIZE);
            pPmacPvt->inBuf[pPmacPvt->inBufHead++] = ACK;
            
        if(status!=asynSuccess || thisRead==0) break;
        }        
    }
    *nbytesTransfered = nRead;
        
    return status;
}


static asynStatus readRaw(void *ppvt,asynUser *pasynUser,
    char *data,size_t maxchars,size_t *nbytesTransfered,int *eomReason)
{
    pmacPvt *pPmacPvt = (pmacPvt *)ppvt;
    asynStatus status = asynSuccess;
    size_t thisRead;
    int nRead = 0;
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "interposePmac::writeRaw\n" );
    assert(pPmacPvt);

    if(eomReason) *eomReason = 0;
    for (;;) {
        if ((pPmacPvt->inBufTail != pPmacPvt->inBufHead)) {
            *data++ = pPmacPvt->inBuf[pPmacPvt->inBufTail++];
            nRead++;
            if (nRead >= maxchars) break;
            continue;
        }
        if(eomReason && *eomReason) break;
        status = readResponse(pPmacPvt, pasynUser, &thisRead, eomReason);
        
        if(status!=asynSuccess || thisRead==0) break;
    }
    *nbytesTransfered = nRead;
        
    return status;
}


static asynStatus flushIt(void *ppvt,asynUser *pasynUser)
{
    pmacPvt *pPmacPvt = (pmacPvt *)ppvt;
    asynStatus status = asynSuccess;
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "interposePmac::flushIt\n" );
    assert(pPmacPvt);

    pmacFlush (pPmacPvt, pasynUser);
    status = pPmacPvt->poctet->flush(pPmacPvt->octetPvt,pasynUser);
    return asynSuccess;
}

static asynStatus registerInterruptUser(void *ppvt,asynUser *pasynUser,
    interruptCallbackOctet callback, void *userPvt,void **registrarPvt)
{
    pmacPvt *pPmacPvt = (pmacPvt *)ppvt;
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "interposePmac::registerInterruptUser\n" );
    assert(pPmacPvt);

    return pPmacPvt->poctet->registerInterruptUser(pPmacPvt->octetPvt,
        pasynUser,callback,userPvt,registrarPvt);
} 

static asynStatus cancelInterruptUser(void *drvPvt,asynUser *pasynUser,
     void *registrarPvt)
{
    pmacPvt *pPmacPvt = (pmacPvt *)drvPvt;
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "interposePmac::cancelInterruptUser\n" );
    assert(pPmacPvt);

    return pPmacPvt->poctet->cancelInterruptUser(pPmacPvt->octetPvt,
        pasynUser,registrarPvt);
} 

static asynStatus setInputEos(void *ppvt,asynUser *pasynUser,
    const char *eos,int eoslen)
{
    pmacPvt *pPmacPvt = (pmacPvt *)ppvt;
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "interposePmac::setInputEos\n" );
    assert(pPmacPvt);

    return pPmacPvt->poctet->setInputEos(pPmacPvt->octetPvt,pasynUser,
       eos,eoslen);
}

static asynStatus getInputEos(void *ppvt,asynUser *pasynUser,
    char *eos,int eossize,int *eoslen)
{
    pmacPvt *pPmacPvt = (pmacPvt *)ppvt;
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "interposePmac::getInputEos\n" );
    assert(pPmacPvt);

    return pPmacPvt->poctet->getInputEos(pPmacPvt->octetPvt,pasynUser,
       eos,eossize,eoslen);
}

static asynStatus setOutputEos(void *ppvt,asynUser *pasynUser,
    const char *eos, int eoslen)
{
    pmacPvt *pPmacPvt = (pmacPvt *)ppvt;
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "interposePmac::setOutputEos\n" );
    assert(pPmacPvt);

    return pPmacPvt->poctet->setOutputEos(pPmacPvt->octetPvt,
        pasynUser,eos,eoslen);

}

static asynStatus getOutputEos(void *ppvt,asynUser *pasynUser,
    char *eos,int eossize,int *eoslen)
{
    pmacPvt *pPmacPvt = (pmacPvt *)ppvt;
    asynPrint( pasynUser, ASYN_TRACE_FLOW, "interposePmac::getOutputEos\n" );
    assert(pPmacPvt);

    return pPmacPvt->poctet->getOutputEos(pPmacPvt->octetPvt,
        pasynUser,eos,eossize,eoslen);

}


/* register pmacAsynIPPortConfig*/
static const iocshArg pmacAsynIPPortConfigArg0 =
    { "portName", iocshArgString };
static const iocshArg pmacAsynIPPortConfigArg1 =
    { "addr", iocshArgInt };
static const iocshArg *pmacAsynIPPortConfigArgs[] = 
    {&pmacAsynIPPortConfigArg0,&pmacAsynIPPortConfigArg1};
static const iocshFuncDef pmacAsynIPPortConfigFuncDef =
    {"pmacAsynIPPortConfig", 2, pmacAsynIPPortConfigArgs};
static void pmacAsynIPPortConfigCallFunc(const iocshArgBuf *args)
{
    pmacAsynIPPortConfig(args[0].sval,args[1].ival);
}

static void pmacAsynIPPortRegister(void)
{
    static int firstTime = 1;
    if (firstTime) {
        firstTime = 0;
        iocshRegister(&pmacAsynIPPortConfigFuncDef, pmacAsynIPPortConfigCallFunc);
    }
}
epicsExportRegistrar(pmacAsynIPPortRegister);
