/********************************************
 *  pmacController.cpp
 * 
 *  PMAC Asyn motor based on the 
 *  asynMotorController class.
 * 
 *  Matthew Pearson
 *  23 May 2012
 * 
 ********************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsExport.h>
#include <epicsString.h>
#include <iocsh.h>

#include "pmacController.h"
#include "asynOctetSyncIO.h"
#include "asynCommonSyncIO.h"


static const char *driverName = "pmacController";

static const int _MAXBUF = 256;


pmacController::pmacController(const char *portName, const char *lowLevelPortName, int lowLevelPortAddress, 
			       int numAxes, double movingPollPeriod, double idlePollPeriod)
  : asynMotorController(portName, numAxes, NUM_MOTOR_DRIVER_PARAMS,
			0, // No additional interfaces
			0, // No addition interrupt interfaces
			ASYN_CANBLOCK | ASYN_MULTIDEVICE, 
			1, // autoconnect
			0, 0)  // Default priority and stack size
{
  static const char *functionName = "pmacController::pmacController";

  printf("  Calling %s\n", functionName);

  pAxes_ = (pmacAxis **)(asynMotorController::pAxes_);

  // Connect our Asyn user to the low level port that is a parameter to this constructor
  //if(pasynOctetSyncIO->connect(lowLevelPortName, lowLevelPortAddress, &lowLevelPortUser, NULL) != asynSuccess) {
  //   printf("%s: Failed to connect to low level asynOctetSyncIO port %s\n", functionName, lowLevelPortName);
  //}
  if (motorAxisAsynConnect(lowLevelPortName, lowLevelPortAddress, &lowLevelPortUser, "\006", "\r") != asynSuccess) {
    printf("%s: Failed to connect to low level asynOctetSyncIO port %s\n", functionName, lowLevelPortName);
  }
  

  // Create controller-specific parameters


  /* Create the poller thread for this controller
   * NOTE: at this point the axis objects don't yet exist, but the poller tolerates this */
  startPoller(movingPollPeriod, idlePollPeriod, 10);
  

}

int pmacController::motorAxisAsynConnect(const char *port, int addr, asynUser **ppasynUser, char *inputEos, char *outputEos)
{
  asynStatus status = asynSuccess;
 
  status = pasynOctetSyncIO->connect( port, addr, ppasynUser, NULL);
    if (status) {
        asynPrint(lowLevelPortUser, ASYN_TRACE_ERROR,
                  "pmacController::motorAxisAsynConnect: unable to connect to port %s\n", 
                  port);
        return status;
    }

    status = pasynOctetSyncIO->setInputEos(*ppasynUser, inputEos, strlen(inputEos) );
    if (status) {
        asynPrint(*ppasynUser, ASYN_TRACE_ERROR,
                  "drvPmacCreate: unable to set input EOS on %s: %s\n", 
                  port, (*ppasynUser)->errorMessage);
        pasynOctetSyncIO->disconnect(*ppasynUser);
        return status;
    }

    status = pasynOctetSyncIO->setOutputEos(*ppasynUser, outputEos, strlen(outputEos));
    if (status) {
        asynPrint(*ppasynUser, ASYN_TRACE_ERROR,
                  "drvPmacCreate: unable to set output EOS on %s: %s\n", 
                  port, (*ppasynUser)->errorMessage);
        pasynOctetSyncIO->disconnect(*ppasynUser);
        return status;
    }

    return status;
}

/**
 * Wrapper for asynOctetSyncIO write/read functions.
 * @param command - String command to send.
 * @response response - String response back.
 */
asynStatus pmacController::writeRead(const char *command, char *response)
{
  asynStatus status = asynSuccess;

   int eomReason;
   size_t bytesOut;
   size_t bytesIn;
   printf("Tx> %s\n", command);
   status = pasynOctetSyncIO->writeRead(lowLevelPortUser, command, strlen(command),
						   response, 256, /*timeout=*/1.0, &bytesOut, &bytesIn, &eomReason);
   printf("Rx< %s\n", response);
   
  return status;
}


void pmacController::report(FILE *fp, int level)
{
  int axis = 0;
  pmacAxis *pAxis = NULL;

  fprintf(fp, "pmac motor driver %s, numAxes=%d, moving poll period=%f, idle poll period=%f\n", 
          this->portName, numAxes_, movingPollPeriod_, idlePollPeriod_);

  if (level > 0) {
    for (axis=0; axis<numAxes_; axis++) {
      pAxis = getAxis(axis);
      fprintf(fp, "  axis %d\n"
                  "    name = %s\n"
                  "    step size = %g\n"
                  "    status = %d\n", 
              pAxis->axisNo_,
              pAxis->stepSize_,  
              pAxis->axisStatus_);
    }
  }

  // Call the base class method
  asynMotorController::report(fp, level);
}


asynStatus pmacController::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
  int function = pasynUser->reason;
  int status = asynSuccess;
  pmacAxis *pAxis = NULL;
  double deviceValue = 0.0;
  static const char *functionName = "pmacController::writeFloat64";
  
  printf("  Calling %s\n", functionName);

  pAxis = this->getAxis(pasynUser);
  if (!pAxis) return asynError;

  /* Set the parameter and readback in the parameter library. */
  status = pAxis->setDoubleParam(function, value);


  if (function == motorResolution_)
  {
    pAxis->stepSize_ = value;
    asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, 
              "%s:%s: Set pmac %s, axis %d stepSize to %f\n", 
               driverName, functionName, portName, pAxis->axisNo_, value);
  }
  else {
    /* Call base class method */
    status = asynMotorController::writeFloat64(pasynUser, value);
  }

  return (asynStatus)status;

}


asynStatus pmacController::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
  int function = pasynUser->reason;
  int status = asynSuccess;
  pmacAxis *pAxis = NULL;
  static const char *functionName = "pmacController::writeInt32";

  printf("  Calling %s\n", functionName);

  pAxis = this->getAxis(pasynUser);
  if (!pAxis) return asynError;
  
  /* Set the parameter and readback in the parameter library.  This may be overwritten when we read back the
   * status at the end, but that's OK */
  status = pAxis->setIntegerParam(function, value);

  if (function == motorSetClosedLoop_) {

  }
  else {
    /* Call base class method */
    status = asynMotorController::writeInt32(pasynUser, value);
  }
  return (asynStatus)status;

}


/** Returns a pointer to an pmacAxis object.
  * Returns NULL if the axis number encoded in pasynUser is invalid.
  * \param[in] pasynUser asynUser structure that encodes the axis index number. */
pmacAxis* pmacController::getAxis(asynUser *pasynUser)
{
  int axisNo = 0;
    
  getAddress(pasynUser, &axisNo);
  return getAxis(axisNo);
}



/** Returns a pointer to an pmacAxis object.
  * Returns NULL if the axis number is invalid.
  * \param[in] axisNo Axis index number. */
pmacAxis* pmacController::getAxis(int axisNo)
{
  if ((axisNo < 0) || (axisNo >= numAxes_)) return NULL;
  return pAxes_[axisNo];
}


/** Polls the controller, rather than individual axis.*/
asynStatus pmacController::poll()
{
  int status = 0;
  char *command = NULL;
  char response[256];
  static const char *functionName = "pmacController::poll";

  printf("  Calling %s\n", functionName);
  
  /* Read ver as a test.*/
  command = "ver";
  writeRead(command, response);
  printf("Response from ver: %s\n", response);

  if (status) return asynError;
  
  callParamCallbacks();
  return asynSuccess;
}



/** The following functions have C linkage, and can be called directly or from iocsh */

extern "C" {

asynStatus pmacCreateController(const char *portName, const char *lowLevelPortName, int lowLevelPortAddress, 
				int numAxes, int movingPollPeriod, int idlePollPeriod)
{
    pmacController *ppmacController
      = new pmacController(portName, lowLevelPortName, lowLevelPortAddress, numAxes, movingPollPeriod/1000., idlePollPeriod/1000.);
    ppmacController = NULL;
    static const char *functionName = "pmacCreateController";
    
    printf("Calling %s.\n", functionName);

    return asynSuccess;
}

asynStatus pmacCreateAxis(const char *pmacName,         /* specify which controller by port name */
                         int axis,                    /* axis number 0-7 */
                         const char *stepsPerUnit)    /* steps per user unit */
{
  pmacController *pC;
  pmacAxis *pAxis;
  double stepSize;
  static const char *functionName = "pmacCreateAxis";

  printf("Calling %s.\n", functionName);

  /* pC = (pmacController*) findAsynPortDriver(pmacName);
  if (!pC) {
    printf("%s:%s: Error port %s not found\n",
           driverName, functionName, pmacName);
    return asynError;
  }
  errno = 0;
  stepSize = strtod(stepsPerUnit, NULL);
  if (errno != 0) {
    printf("%s:%s: Error invalid steps per unit=%s\n",
           driverName, functionName, stepsPerUnit);
    return asynError;
  }
  
  pC->lock();
  pAxis = new pmacAxis(pC, axis, 1./stepSize);
  pAxis = NULL;
  pC->unlock();*/
  return asynSuccess;
}


/* Code for iocsh registration */

/* pmacCreateController */
static const iocshArg pmacCreateControllerArg0 = {"Controller port name", iocshArgString};
static const iocshArg pmacCreateControllerArg1 = {"Low level port name", iocshArgString};
static const iocshArg pmacCreateControllerArg2 = {"Low level port address", iocshArgInt};
static const iocshArg pmacCreateControllerArg3 = {"Number of axes", iocshArgInt};
static const iocshArg pmacCreateControllerArg4 = {"Moving poll rate (ms)", iocshArgInt};
static const iocshArg pmacCreateControllerArg5 = {"Idle poll rate (ms)", iocshArgInt};
static const iocshArg * const pmacCreateControllerArgs[] = {&pmacCreateControllerArg0,
							    &pmacCreateControllerArg1,
							    &pmacCreateControllerArg2,
							    &pmacCreateControllerArg3,
							    &pmacCreateControllerArg4,
							    &pmacCreateControllerArg5};
static const iocshFuncDef configpmac = {"pmacCreateController", 6, pmacCreateControllerArgs};
static void configpmacCallFunc(const iocshArgBuf *args)
{
  pmacCreateController(args[0].sval, args[1].sval, args[2].ival, args[3].ival, args[4].ival, args[5].ival);
}


/* pmacCreateAxis */
static const iocshArg pmacCreateAxisArg0 = {"Controller port name", iocshArgString};
static const iocshArg pmacCreateAxisArg1 = {"Axis number", iocshArgInt};
static const iocshArg pmacCreateAxisArg2 = {"Steps per unit", iocshArgInt};
static const iocshArg * const pmacCreateAxisArgs[] = {&pmacCreateAxisArg0,
                                                     &pmacCreateAxisArg1,
                                                     &pmacCreateAxisArg2};
static const iocshFuncDef configpmacAxis = {"pmacCreateAxis", 3, pmacCreateAxisArgs};

static void configpmacAxisCallFunc(const iocshArgBuf *args)
{
  pmacCreateAxis(args[0].sval, args[1].ival, args[2].sval);
}

static void pmacRegister3(void)
{
  iocshRegister(&configpmac,            configpmacCallFunc);
  iocshRegister(&configpmacAxis,        configpmacAxisCallFunc);
}
epicsExportRegistrar(pmacRegister3);


} // extern "C"
