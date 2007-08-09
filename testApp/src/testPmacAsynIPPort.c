#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cantProceed.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <epicsStdio.h>
#include <epicsString.h>
#include <epicsThread.h>
#include <epicsAssert.h>
#include <asynDriver.h>
#include <asynOctet.h>
#include <asynOctetSyncIO.h>
#include <drvAsynIPPort.h>
#include <iocsh.h>
#include <registryFunction.h>
#include <epicsExport.h>
#include "testPmacAsynIPPort.h"

#define BUFFER_SIZE 80
/*#define READ_TIMEOUT -1.0  - this would wait forever */
#define READ_TIMEOUT 5.0
#define WRITE_TIMEOUT 2.0

#define MAX_CMD 20

typedef struct myData {
    epicsMutexId mutexId;
    char         *portName;
    double       readTimeout;
    asynOctet    *pasynOctet;
    void         *octetPvt;
    void         *registrarPvt;
    char         *cmd[MAX_CMD];
}myData;


/*** pmac commands for axis 1 ***/
/* Initiate an absolute move to position (zz) with velocity (vv) and acceleration (aa): Ixx22=vv Ixx20=aa #xx J=zz */
char* cmdMoveA="I122=1 I120=0.5 #1 J=10";
/* Initiate a relative move of distance (zz) with velocity (vv) and acceleration (aa): Ixx22=vv Ixx20=aa #xx J^zz */
char* cmdMoveR="I122=1 I120=0.5 #1 J^10";
/* Initiate a forwards velocity move with velocity (vv) and acceleration (aa): Ixx22=vv Ixx20=aa #xx J+ */
char* cmdJF="I122=1 I120=0.5 #1 J+";
/* Initiate a reverse velocity move with velocity (vv) and acceleration (aa): Ixx22=vv Ixx20=aa #xx J- */
char* cmdJR="I122=1 I120=0.5 #1 J-";
/* re-enable limits if limits: Ixx24=Ixx24&$FDFFFF */ 
char* cmdEnLim="I124=I124&$FDFFFF";
/* Homing with velocity (vv) and acceleration (aa): Ixx23=vv Ixx20=aa #xx HOME */
char* cmdHome="I123=1 I120=0.5 #1 HOME";
/* Homing with velocity (vv) and acceleration (aa) and temporarily disabling limits: Ixx23=vv Ixx20=aa #xx HOME Ixx24=Ixx24|$20000 */
char* cmdHomeNL="I123=1 I120=0.5 #1 HOME I124=I124|$20000";
/* Stopping any motion command: #xx J/ */
char* cmdStop="#1 J/";
/* Determining homing and limits status - yy is the macro node (i.e. (xx-1)*2-mod((xx-1),2)): MSyy,I912 MSyy,I913 Ixx24 Ixx23 Ixx26 */
char* cmdHLStatus="MS0,I912 MS0,I913 I124 I123 I126";
/* Polling status: #xx ? P F V */
char* cmdPollStatus="#1 ? P F V";
/* Determining the initial status: Ixx13 Ixx14 Ixx30 Ixx31 Ixx33 */
char* cmdPosStatus="I113 I114 I130 I131 I133";
/* Enable axis: Ix00=1 */
char* cmdEnable="I100=1";
/* Disable axis: Ix00=0 */
char* cmdDisable="I100=0";
/* Definining the current position to be a new fixed value (zz=value *32+0.5): #xx K Mxx61=zz*Ixx08 Mxx62=zz*Ixx08 J/ */
char* cmdDefPos="#1 K M161=640.5*I108 M162=640.5*I108 J/";
/* Set axis low limit to value (zz): Ixx14=zz */
char* cmdSetLoLim="I114=-1000";
/* Set axis high limit to value (zz): Ixx13=zz */
char* cmdSetHiLim="I113=1000";
/* Set axis proportional gain to value (zz): Ixx30=zz */
char* cmdSetPGain="I130=2000";
/* Set axis differential gain to value (zz): Ixx31=zz */
char* cmdSetDGain="I131=0";
/* Set axis integral gain to value (zz): Ixx33=zz */
char* cmdSetIGain="I133=0";
/* Report status words for all motors */
char* cmdCTRLB="\2";
/* Report all coordinate system status words */
char* cmdCTRLC="\3";
/* Report following errors for all motors */
char* cmdCTRLF="\6";
/* Report global status word */
char* cmdCTRLG="\7";
/* Report position of all motors */
char* cmdCTRLP="\16";
/* Report velocity of all motors */
char* cmdCTRLV="\22";

asynStatus sendCmd(asynUser *pasynUser, myData *pPvt, char* cmd )
{
    char buffer[BUFFER_SIZE];
    int nread, nwrite, eomReason;
    asynStatus status;

        buffer[0] = 0;

        status = pasynOctetSyncIO->writeRead( pasynUser,
                                          cmd, strlen(cmd),
                                          buffer, BUFFER_SIZE,
                                          pPvt->readTimeout,
                                          &nwrite, &nread, &eomReason );
        switch (status) {
        case asynTimeout:
            asynPrint(pasynUser, ASYN_TRACE_ERROR,
                      "testPmacAsynIPPort: TIMEOUT\n");
            /* no break - fall throught to asynSuccess */
            
        case asynSuccess:
             asynPrintIO(pasynUser,ASYN_TRACEIO_DEVICE,cmd,strlen(cmd),
                 "testPmacAsynIPPort: %s write %d:\n",pPvt->portName,strlen(cmd));
             asynPrintIO(pasynUser,ASYN_TRACEIO_DEVICE,buffer,nread,
                 "testPmacAsynIPPort: %s read %d:\n",pPvt->portName,nread);
            break;

        default:
            asynPrint(pasynUser, ASYN_TRACE_ERROR,
                      "testPmacAsynIPPort: writeRead error on: %s: write %d: %s, status=%d error=%s\n", 
                      pPvt->portName, strlen(cmd), cmd, status, pasynUser->errorMessage);
            break;
        }
        return status;
}

void testHandler(myData *pPvt)
{
    asynUser *pasynUser;
    asynStatus status;
    int i;

    status = pasynOctetSyncIO->connect(pPvt->portName, 0, &pasynUser, NULL);
    if (status) {
        asynPrint(pasynUser, ASYN_TRACE_ERROR,
                  "echoHandler: unable to connect to port %s\n", 
                  pPvt->portName);
        return;
    }

    asynPrint(pasynUser, ASYN_TRACE_FLOW, 
              "testPmacAsynIPPort: testHandler, portName=%s\n", pPvt->portName);
            epicsThreadSleep(0.01); /* Let the writer thread get time to run */
    
/***  EOS setup is not required - its already done by pmacAsynIPPort interpose interface
    status = pasynOctetSyncIO->setInputEos(pasynUser, "\6", 1);
    if (status) {
        asynPrint(pasynUser, ASYN_TRACE_ERROR,
                  "testPmacAsynIPPort: unable to set input EOS on %s: %s\n", 
                  pPvt->portName, pasynUser->errorMessage);
        return;
    }
    status = pasynOctetSyncIO->setOutputEos(pasynUser, "\r", 1);
    if (status) {
        asynPrint(pasynUser, ASYN_TRACE_ERROR,
                  "testPmacAsynIPPort: unable to set output EOS on %s: %s\n", 
                  pPvt->portName, pasynUser->errorMessage);
        return;
    }
***/

    /*** send the test commands ***/
    status = asynSuccess;
    pPvt->cmd[0]=cmdPollStatus; 
    pPvt->cmd[1]=cmdPosStatus;
    pPvt->cmd[2]=cmdEnable;
    pPvt->cmd[3]=cmdEnLim;
    pPvt->cmd[4]=cmdMoveA;
    pPvt->cmd[5]=cmdStop;
    pPvt->cmd[6]=cmdMoveR;
    pPvt->cmd[7]=cmdStop;
    pPvt->cmd[8]=cmdJF;
    pPvt->cmd[9]=cmdStop;
    pPvt->cmd[10]=cmdHomeNL;
    pPvt->cmd[11]=cmdStop;
    pPvt->cmd[12]=cmdSetLoLim;
    pPvt->cmd[13]=cmdSetHiLim;
    pPvt->cmd[14]=cmdHLStatus;
    pPvt->cmd[15]=NULL;

    for(i=0; i<MAX_CMD && pPvt->cmd[i] && status==asynSuccess; i++) {
        status = sendCmd(pasynUser, pPvt, pPvt->cmd[i] );
    /*   epicsThreadSleep(1); */
    }
    
    if (status==asynSuccess)
        printf("**********TEST PASSED ********\n");
    else    
        printf("**********TEST FAILED ********\n");
    return;
}

                         
static void connectionCallback(void *drvPvt, asynUser *pasynUser, char *portName, size_t len, int eomReason)
{
/*    myData     *pPvt = (myData *)drvPvt;
    myData     *newPvt = calloc(1, sizeof(myData));
*/
    asynPrint(pasynUser, ASYN_TRACE_FLOW, 
              "testPmacAsynIPPort: connectionCallback, portName=%s\n", portName);

}

epicsShareFunc int testPmacAsynIPPort(const char *portName, int readTimeout)
{
    myData        *pPvt;
    asynUser      *pasynUser;
    asynStatus    status;
    int           addr=0;
    asynInterface *pasynInterface;

    pPvt = (myData *)callocMustSucceed(1, sizeof(myData), "testPmacAsynIPPort");
    pPvt->mutexId = epicsMutexCreate();
    pPvt->portName = epicsStrDup(portName);
    pasynUser = pasynManager->createAsynUser(0,0);
    pasynUser->userPvt = pPvt;
    status = pasynManager->connectDevice(pasynUser,portName,addr);
    if(status!=asynSuccess) {
        printf("testPmacAsynIPPort: can't connect to port %s: %s\n", portName, pasynUser->errorMessage);
        return -1;
    }
    pasynInterface = pasynManager->findInterface(
       pasynUser,asynOctetType,1);
    if(!pasynInterface) {
        printf("testPmacAsynIPPort: %s driver not supported\n",asynOctetType);
        return -1;
    }
    if (readTimeout == 0) 
        pPvt->readTimeout = READ_TIMEOUT;
    else 
        pPvt->readTimeout = (double)readTimeout;
    pPvt->pasynOctet = (asynOctet *)pasynInterface->pinterface;
    pPvt->octetPvt = pasynInterface->drvPvt;
    status = pPvt->pasynOctet->registerInterruptUser(
                 pPvt->octetPvt, pasynUser,
                 connectionCallback,pPvt,&pPvt->registrarPvt);
    if(status!=asynSuccess) {
        printf("testPmacAsynIPPort: devAsynOctet registerInterruptUser %s\n",
               pasynUser->errorMessage);
        return -1;       
    }

    asynPrint(pasynUser,ASYN_TRACE_FLOW, "Done testPmacAsynIPPort OK\n");
    
    testHandler(pPvt);
    return 0;
}

static const iocshArg testPmacAsynIPPortArg0 = {"port", iocshArgString};
static const iocshArg testPmacAsynIPPortArg1 = {"read timeout", iocshArgInt};
static const iocshArg *const testPmacAsynIPPortArgs[] = {
    &testPmacAsynIPPortArg0,
    &testPmacAsynIPPortArg1};
static const iocshFuncDef testPmacAsynIPPortDef = {"testPmacAsynIPPort", 2, testPmacAsynIPPortArgs};
static void testPmacAsynIPPortCall(const iocshArgBuf * args) 
{ 
    testPmacAsynIPPort(args[0].sval, args[1].ival);
}

static void testPmacAsynIPPortRegister(void)
{
    static int firstTime = 1;
    if(!firstTime) return;
    firstTime = 0;
    iocshRegister(&testPmacAsynIPPortDef,testPmacAsynIPPortCall);
}
epicsExportRegistrar(testPmacAsynIPPortRegister);
