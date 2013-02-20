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

#include <iostream>
using std::cout;
using std::endl;

#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsExport.h>
#include <epicsString.h>
#include <iocsh.h>
#include <drvSup.h>
#include <registryFunction.h>

#include "asynOctetSyncIO.h"

#include "pmacController.h"

static const char *driverName = "pmacController";

const epicsUInt32 pmacController::PMAC_MAXBUF_ = PMAC_MAXBUF;
const epicsFloat64 pmacController::PMAC_TIMEOUT_ = 5.0;

const epicsUInt32 pmacController::PMAC_STATUS1_MAXRAPID_SPEED    = (0x1<<0);
const epicsUInt32 pmacController::PMAC_STATUS1_ALT_CMNDOUT_MODE  = (0x1<<1);
const epicsUInt32 pmacController::PMAC_STATUS1_SOFT_POS_CAPTURE  = (0x1<<2);
const epicsUInt32 pmacController::PMAC_STATUS1_ERROR_TRIGGER     = (0x1<<3);
const epicsUInt32 pmacController::PMAC_STATUS1_FOLLOW_ENABLE     = (0x1<<4);
const epicsUInt32 pmacController::PMAC_STATUS1_FOLLOW_OFFSET     = (0x1<<5);
const epicsUInt32 pmacController::PMAC_STATUS1_PHASED_MOTOR      = (0x1<<6);
const epicsUInt32 pmacController::PMAC_STATUS1_ALT_SRC_DEST      = (0x1<<7);
const epicsUInt32 pmacController::PMAC_STATUS1_USER_SERVO        = (0x1<<8);
const epicsUInt32 pmacController::PMAC_STATUS1_USER_PHASE        = (0x1<<9);
const epicsUInt32 pmacController::PMAC_STATUS1_HOMING            = (0x1<<10);
const epicsUInt32 pmacController::PMAC_STATUS1_BLOCK_REQUEST     = (0x1<<11);
const epicsUInt32 pmacController::PMAC_STATUS1_DECEL_ABORT       = (0x1<<12);
const epicsUInt32 pmacController::PMAC_STATUS1_DESIRED_VELOCITY_ZERO = (0x1<<13);
const epicsUInt32 pmacController::PMAC_STATUS1_DATABLKERR        = (0x1<<14);
const epicsUInt32 pmacController::PMAC_STATUS1_DWELL             = (0x1<<15);
const epicsUInt32 pmacController::PMAC_STATUS1_INTEGRATE_MODE    = (0x1<<16);
const epicsUInt32 pmacController::PMAC_STATUS1_MOVE_TIME_ON      = (0x1<<17);
const epicsUInt32 pmacController::PMAC_STATUS1_OPEN_LOOP         = (0x1<<18);
const epicsUInt32 pmacController::PMAC_STATUS1_AMP_ENABLED       = (0x1<<19);
const epicsUInt32 pmacController::PMAC_STATUS1_X_SERVO_ON        = (0x1<<20);
const epicsUInt32 pmacController::PMAC_STATUS1_POS_LIMIT_SET     = (0x1<<21);
const epicsUInt32 pmacController::PMAC_STATUS1_NEG_LIMIT_SET     = (0x1<<22);
const epicsUInt32 pmacController::PMAC_STATUS1_MOTOR_ON          = (0x1<<23);

const epicsUInt32 pmacController::PMAC_STATUS2_IN_POSITION       = (0x1<<0);
const epicsUInt32 pmacController::PMAC_STATUS2_WARN_FOLLOW_ERR   = (0x1<<1);
const epicsUInt32 pmacController::PMAC_STATUS2_ERR_FOLLOW_ERR    = (0x1<<2);
const epicsUInt32 pmacController::PMAC_STATUS2_AMP_FAULT         = (0x1<<3);
const epicsUInt32 pmacController::PMAC_STATUS2_NEG_BACKLASH      = (0x1<<4);
const epicsUInt32 pmacController::PMAC_STATUS2_I2T_AMP_FAULT     = (0x1<<5);
const epicsUInt32 pmacController::PMAC_STATUS2_I2_FOLLOW_ERR     = (0x1<<6);
const epicsUInt32 pmacController::PMAC_STATUS2_TRIGGER_MOVE      = (0x1<<7);
const epicsUInt32 pmacController::PMAC_STATUS2_PHASE_REF_ERR     = (0x1<<8);
const epicsUInt32 pmacController::PMAC_STATUS2_PHASE_SEARCH      = (0x1<<9);
const epicsUInt32 pmacController::PMAC_STATUS2_HOME_COMPLETE     = (0x1<<10);
const epicsUInt32 pmacController::PMAC_STATUS2_POS_LIMIT_STOP    = (0x1<<11);
const epicsUInt32 pmacController::PMAC_STATUS2_DESIRED_STOP      = (0x1<<12);
const epicsUInt32 pmacController::PMAC_STATUS2_FORE_IN_POS       = (0x1<<13);
const epicsUInt32 pmacController::PMAC_STATUS2_NA14              = (0x1<<14);
const epicsUInt32 pmacController::PMAC_STATUS2_ASSIGNED_CS       = (0x1<<15);

/*Global status ???*/
const epicsUInt32 pmacController::PMAC_GSTATUS_CARD_ADDR             = (0x1<<0);
const epicsUInt32 pmacController::PMAC_GSTATUS_ALL_CARD_ADDR         = (0x1<<1);
const epicsUInt32 pmacController::PMAC_GSTATUS_RESERVED              = (0x1<<2);
const epicsUInt32 pmacController::PMAC_GSTATUS_PHASE_CLK_MISS        = (0x1<<3);
const epicsUInt32 pmacController::PMAC_GSTATUS_MACRO_RING_ERRORCHECK = (0x1<<4);
const epicsUInt32 pmacController::PMAC_GSTATUS_MACRO_RING_COMMS      = (0x1<<5);
const epicsUInt32 pmacController::PMAC_GSTATUS_TWS_PARITY_ERROR      = (0x1<<6);
const epicsUInt32 pmacController::PMAC_GSTATUS_CONFIG_ERROR          = (0x1<<7);
const epicsUInt32 pmacController::PMAC_GSTATUS_ILLEGAL_LVAR          = (0x1<<8);
const epicsUInt32 pmacController::PMAC_GSTATUS_REALTIME_INTR         = (0x1<<9);
const epicsUInt32 pmacController::PMAC_GSTATUS_FLASH_ERROR           = (0x1<<10);
const epicsUInt32 pmacController::PMAC_GSTATUS_DPRAM_ERROR           = (0x1<<11);
const epicsUInt32 pmacController::PMAC_GSTATUS_CKSUM_ACTIVE          = (0x1<<12);
const epicsUInt32 pmacController::PMAC_GSTATUS_CKSUM_ERROR           = (0x1<<13);
const epicsUInt32 pmacController::PMAC_GSTATUS_LEADSCREW_COMP        = (0x1<<14);
const epicsUInt32 pmacController::PMAC_GSTATUS_WATCHDOG              = (0x1<<15);
const epicsUInt32 pmacController::PMAC_GSTATUS_SERVO_REQ             = (0x1<<16);
const epicsUInt32 pmacController::PMAC_GSTATUS_DATA_GATHER_START     = (0x1<<17);
const epicsUInt32 pmacController::PMAC_GSTATUS_RESERVED2             = (0x1<<18);
const epicsUInt32 pmacController::PMAC_GSTATUS_DATA_GATHER_ON        = (0x1<<19);
const epicsUInt32 pmacController::PMAC_GSTATUS_SERVO_ERROR           = (0x1<<20);
const epicsUInt32 pmacController::PMAC_GSTATUS_CPUTYPE               = (0x1<<21);
const epicsUInt32 pmacController::PMAC_GSTATUS_REALTIME_INTR_RE      = (0x1<<22);
const epicsUInt32 pmacController::PMAC_GSTATUS_RESERVED3             = (0x1<<23);

const epicsUInt32 pmacController::PMAC_HARDWARE_PROB = (PMAC_GSTATUS_MACRO_RING_ERRORCHECK | PMAC_GSTATUS_MACRO_RING_COMMS | PMAC_GSTATUS_REALTIME_INTR | PMAC_GSTATUS_FLASH_ERROR | PMAC_GSTATUS_DPRAM_ERROR | PMAC_GSTATUS_CKSUM_ERROR | PMAC_GSTATUS_WATCHDOG | PMAC_GSTATUS_SERVO_ERROR);

const epicsUInt32 pmacController::PMAX_AXIS_GENERAL_PROB1 = 0;
const epicsUInt32 pmacController::PMAX_AXIS_GENERAL_PROB2 = (PMAC_STATUS2_DESIRED_STOP | PMAC_STATUS2_AMP_FAULT);


//C function prototypes, for the functions that can be called on IOC shell
extern "C" {
  asynStatus pmacCreateController(const char *portName, const char *lowLevelPortName, int lowLevelPortAddress, 
					 int numAxes, int movingPollPeriod, int idlePollPeriod);
  
  asynStatus pmacCreateAxis(const char *pmacName, int axis);

  asynStatus pmacCreateAxes(const char *pmacName, int numAxes);
  
  asynStatus pmacDisableLimitsCheck(const char *controller, int axis, int allAxes);
  
  asynStatus pmacSetAxisScale(const char *controller, int axis, int scale);
  
  asynStatus pmacSetOpenLoopEncoderAxis(const char *controller, int axis, int encoder_axis);

  
}

pmacController::pmacController(const char *portName, const char *lowLevelPortName, int lowLevelPortAddress, 
			       int numAxes, double movingPollPeriod, double idlePollPeriod)
  : asynMotorController(portName, numAxes+1, NUM_MOTOR_DRIVER_PARAMS,
			0, // No additional interfaces
			0, // No addition interrupt interfaces
			ASYN_CANBLOCK | ASYN_MULTIDEVICE, 
			1, // autoconnect
			0, 0)  // Default priority and stack size
{
  static const char *functionName = "pmacController::pmacController";

  printf("  Constructor: %s\n", functionName);

  //Initialize non static data members
  lowLevelPortUser_ = NULL;
  debugFlag_ = 0;
  movesDeferred_ = 0;

  pAxes_ = (pmacAxis **)(asynMotorController::pAxes_);

  // Create controller-specific parameters
  createParam(PMAC_C_GlobalStatusString,       asynParamInt32, &PMAC_C_GlobalStatus_);
  createParam(PMAC_C_CommsErrorString,         asynParamInt32, &PMAC_C_CommsError_);

  // Connect our Asyn user to the low level port that is a parameter to this constructor
  if (lowLevelPortConnect(lowLevelPortName, lowLevelPortAddress, &lowLevelPortUser_, "\006", "\r") != asynSuccess) {
    printf("%s: Failed to connect to low level asynOctetSyncIO port %s\n", functionName, lowLevelPortName);
        setIntegerParam(PMAC_C_CommsError_, 1);
  } else {
    /* Create the poller thread for this controller
     * NOTE: at this point the axis objects don't yet exist, but the poller tolerates this */
    setIntegerParam(PMAC_C_CommsError_, 0);
  }
  startPoller(movingPollPeriod, idlePollPeriod, 10);

  callParamCallbacks();
 
}


pmacController::~pmacController(void) 
{
  //Destructor
}


/**
 * Connect to the underlying low level Asyn port that is used for comms.
 * This uses the asynOctetSyncIO interface, and also sets the input and output terminators.
 */
int pmacController::lowLevelPortConnect(const char *port, int addr, asynUser **ppasynUser, char *inputEos, char *outputEos)
{
  asynStatus status = asynSuccess;
 
  static const char *functionName = "pmacController::lowLevelPortConnect";

  debugFlow(functionName);

  status = pasynOctetSyncIO->connect( port, addr, ppasynUser, NULL);
  if (status) {
    asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
	      "pmacController::motorAxisAsynConnect: unable to connect to port %s\n", 
	      port);
    return status;
  }

  //Do I want to disconnect below? If the IP address comes up, will the driver recover
  //if the poller functions are running? Might have to use asynManager->isConnected to
  //test connection status of low level port (in the pollers). But then autosave 
  //restore doesn't work (and we would save wrong positions). So I need to 
  //have a seperate function(s) to deal with connecting after IOC init.

  status = pasynOctetSyncIO->setInputEos(*ppasynUser, inputEos, strlen(inputEos) );
  if (status) {
    asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
	      "pmacController: unable to set input EOS on %s: %s\n", 
	      port, (*ppasynUser)->errorMessage);
    pasynOctetSyncIO->disconnect(*ppasynUser);
    //Set my low level pasynUser pointer to NULL
    *ppasynUser = NULL;
    return status;
  }
  
  status = pasynOctetSyncIO->setOutputEos(*ppasynUser, outputEos, strlen(outputEos));
  if (status) {
    asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
	      "pmacController: unable to set output EOS on %s: %s\n", 
	      port, (*ppasynUser)->errorMessage);
    pasynOctetSyncIO->disconnect(*ppasynUser);
    //Set my low level pasynUser pointer to NULL
    *ppasynUser = NULL;
    return status;
  }
  
  return status;
}

/**
 * Utilty function to print the connected status of the low level asyn port.
 */
asynStatus pmacController::printConnectedStatus()
{
  asynStatus status = asynSuccess;
  int asynManagerConnected = 0;
  
  if (lowLevelPortUser_) {
    status = pasynManager->isConnected(lowLevelPortUser_, &asynManagerConnected);
      if (status) {
      asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
	      "pmacController: Error calling pasynManager::isConnected.\n");
      return asynError;
    } else {
	printf("pmacController::printConnectedStatus: isConnected: %d\n", asynManagerConnected);
    }
  }
  return asynSuccess;
}

/**
 * Wrapper for asynOctetSyncIO write/read functions.
 * @param command - String command to send.
 * @response response - String response back.
 */
asynStatus pmacController::lowLevelWriteRead(const char *command, char *response)
{
  asynStatus status = asynSuccess;

   int eomReason;
   size_t nwrite = 0;
   size_t nread = 0;
   int commsError = 0;
   static const char *functionName = "pmacController::lowLevelWriteRead";

   debugFlow(functionName);

   if (!lowLevelPortUser_) {
     setIntegerParam(this->motorStatusCommsError_, 1);
     return asynError;
   }

   asynPrint(lowLevelPortUser_, ASYN_TRACEIO_DRIVER, "%s: command: %s\n", functionName, command);
   debugFlow("Sending: ");
   debugFlow(command);

   //Make sure the low level port is connected before we attempt comms
   //Use the controller-wide param PMAC_C_CommsError_
   getIntegerParam(PMAC_C_CommsError_, &commsError);

   if (!commsError) {
     status = pasynOctetSyncIO->writeRead(lowLevelPortUser_ ,
					  command, strlen(command),
					  response, PMAC_MAXBUF_,
					  PMAC_TIMEOUT_,
					  &nwrite, &nread, &eomReason );
     
     if (status) {
       asynPrint(lowLevelPortUser_, ASYN_TRACE_ERROR, "%s: Error from pasynOctetSyncIO->writeRead. command: %s\n", functionName, command);
       setIntegerParam(this->motorStatusCommsError_, 1);
     } else {
       setIntegerParam(this->motorStatusCommsError_, 0);
     }
   }

   asynPrint(lowLevelPortUser_, ASYN_TRACEIO_DRIVER, "%s: response: %s\n", functionName, response); 
   debugFlow("Received: ");
   debugFlow(response);

  return status;
}

void pmacController::debugFlow(const char *message)
{
  if (debugFlag_ == 1) {
    printf("  %s\n", message);
  }

  asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "%s\n", message);

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
      if (!pAxis) continue;
      fprintf(fp, "  axis %d\n"
                  "    scale = %d\n", 
              pAxis->axisNo_,
              pAxis->scale_);
    }
  }

  // Call the base class method
  asynMotorController::report(fp, level);
}


asynStatus pmacController::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
  int function = pasynUser->reason;
  asynStatus status = asynError;
  pmacAxis *pAxis = NULL;
  char command[64] = {0};
  char response[64] = {0};
  double encRatio = 1.0;
  epicsInt32 encposition = 0;
	
  static const char *functionName = "pmacController::writeFloat64";

  debugFlow(functionName);

  pAxis = this->getAxis(pasynUser);
  if (!pAxis) {
    return asynError;
  }

  /* Set the parameter and readback in the parameter library. */
  status = pAxis->setDoubleParam(function, value);

  if (function == motorPosition_) {
    /*Set position on motor axis.*/            
    epicsInt32 position = (epicsInt32) floor(value*32/pAxis->scale_ + 0.5);
    
    sprintf(command, "#%dK M%d61=%d*I%d08 M%d62=%d*I%d08",
	     pAxis->axisNo_,
	     pAxis->axisNo_, position, pAxis->axisNo_, 
	     pAxis->axisNo_, position, pAxis->axisNo_ );

    asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, 
	      "%s: Set axis %d on controller %s to position %f\n", 
	      functionName, pAxis->axisNo_, portName, value);
    
    if ( command[0] != 0 && status == asynSuccess) {
      status = lowLevelWriteRead(command, response);
    }
                
    sprintf(command, "#%dJ/", pAxis->axisNo_);

    if (command[0] != 0 && status == asynSuccess) {
      status = lowLevelWriteRead(command, response);
    }

    /*Now set position on encoder axis, if one is in use.*/
                
    if (pAxis->encoder_axis_) {
      getDoubleParam(motorEncoderRatio_,  &encRatio);
      encposition = (epicsInt32) floor((position*encRatio) + 0.5);
                  
      sprintf(command, "#%dK M%d61=%d*I%d08 M%d62=%d*I%d08",
	      pAxis->encoder_axis_,
	      pAxis->encoder_axis_, encposition, pAxis->encoder_axis_, 
	      pAxis->encoder_axis_, encposition, pAxis->encoder_axis_ );
                  
      asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, 
		"%s: Set encoder axis %d on controller %s to position %f\n", 
		functionName, pAxis->axisNo_, portName, value);
                  
      if (command[0] != 0 && status == asynSuccess) {
	status = lowLevelWriteRead(command, response);
      }
                  
      sprintf(command, "#%dJ/", pAxis->encoder_axis_);
      //The lowLevelWriteRead will be done at the end of this function.
    }

    /*Now do an update, to get the new position from the controller.*/
    bool moving = true;
    pAxis->getAxisStatus(&moving);
  } 
  else if (function == motorLowLimit_) {
    sprintf(command, "I%d14=%f", pAxis->axisNo_, value/pAxis->scale_);
    asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW,
	      "%s: Setting low limit on controller %s, axis %d to %f\n",
	      functionName, portName, pAxis->axisNo_, value);
  }
  else if (function == motorHighLimit_) {
    sprintf(command, "I%d13=%f", pAxis->axisNo_, value/pAxis->scale_);
    asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW,
	      "%s: Setting high limit on controller %s, axis %d to %f\n",
	      functionName, portName, pAxis->axisNo_, value);
  } 

  //Execute the command.
  if (command[0] != 0 && status == asynSuccess) {
    status = lowLevelWriteRead(command, response);
  }

  //Call base class method
  //This will handle callCallbacks even if the function was handled here.
  status = asynMotorController::writeFloat64(pasynUser, value);

  return status;

}


asynStatus pmacController::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
  int function = pasynUser->reason;
  asynStatus status = asynError;
  pmacAxis *pAxis = NULL;
  static const char *functionName = "pmacController::writeInt32";

  debugFlow(functionName);

  pAxis = this->getAxis(pasynUser);
  if (!pAxis) {
    return asynError;
  }
  
  /* Set the parameter and readback in the parameter library.  This may be overwritten when we read back the
   * status at the end, but that's OK */
  status = pAxis->setIntegerParam(function, value);

  if (function == motorDeferMoves_) {
    asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, "%s: Setting deferred move mode on PMAC %s to %d\n", functionName, portName, value);
    if (value == 0 && this->movesDeferred_ != 0) {
      status = this->processDeferredMoves();
    }
    this->movesDeferred_ = value;
  }

  //Call base class method
  //This will handle callCallbacks even if the function was handled here.
  status = asynMotorController::writeInt32(pasynUser, value);

  return status;

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
  epicsUInt32 globalStatus = 0;
  static const char *functionName = "pmacController::poll";

  debugFlow(functionName);

  if (!lowLevelPortUser_) {
    setIntegerParam(this->motorStatusCommsError_, 1);
    return asynError;
  }

  //Set any controller specific parameters. 
  //Some of these may be used by the axis poll to set axis problem bits.
  globalStatus = getGlobalStatus();
  setIntegerParam(this->PMAC_C_GlobalStatus_, ((globalStatus & PMAC_HARDWARE_PROB) != 0));
  
  callParamCallbacks();

  return asynSuccess;
}


/**
 * Read the PMAC global status integer (using a ??? )
 * @return int The global status integer (23 active bits)
 */
epicsUInt32 pmacController::getGlobalStatus(void)
{
  char command[32];
  char response[PMAC_MAXBUF_];
  int nvals = 0;
  epicsUInt32 pmacStatus = 0;
  static const char *functionName = "pmacController::getGlobalStatus";

  debugFlow(functionName);

  sprintf(command, "???");
  if (lowLevelWriteRead(command, response) != asynSuccess) {
    asynPrint(lowLevelPortUser_, ASYN_TRACE_ERROR, "%s: Error reading global status. command: %s\n", functionName, command);
    setIntegerParam(this->motorStatusCommsError_, 1);
  } else {
    setIntegerParam(this->motorStatusCommsError_, 0);
  }
  
  nvals = sscanf(response, "%6x", &pmacStatus);

  return pmacStatus;

}

/**
 * Disable the check in the axis poller that reads ix24 to check if hardware limits
 * are disabled. By default this is enabled for safety reasons. It sets the motor
 * record PROBLEM bit in MSTA, which results in the record going into MAJOR/STATE alarm.
 * @param axis Axis number to disable the check for.
 */
asynStatus pmacController::pmacDisableLimitsCheck(int axis) 
{
  pmacAxis *pA = NULL;
  static const char *functionName = "pmacController::pmacDisableLimitsCheck";

  debugFlow(functionName);

  this->lock();
  pA = getAxis(axis);
  if (pA) {
    pA->limitsCheckDisable_ = 1;
    asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, 
              "%s. Disabling hardware limits disable check on controller %s, axis %d\n", 
              functionName, portName, pA->axisNo_);
  } else {
    asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
	      "%s: Error: axis %d has not been configured using pmacCreateAxis.\n", functionName, axis);
    return asynError;
  }
  this->unlock();
  return asynSuccess;
}

/**
 * Disable the check in the axis poller that reads ix24 to check if hardware limits
 * are disabled. By default this is enabled for safety reasons. It sets the motor
 * record PROBLEM bit in MSTA, which results in the record going into MAJOR/STATE alarm.
 * This function will disable the check for all axes on this controller.
 */
asynStatus pmacController::pmacDisableLimitsCheck(void) 
{
  pmacAxis *pA = NULL;
  static const char *functionName = "pmacController::pmacDisableLimitsCheck";

  debugFlow(functionName);

  this->lock();
  for (int i=0; i<numAxes_; i++) {
    pA = getAxis(i);
    if (!pA) continue;
    pA->limitsCheckDisable_ = 1;
    asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, 
              "%s. Disabling hardware limits disable check on controller %s, axis %d\n", 
              functionName, portName, pA->axisNo_);
  }
  this->unlock();
  return asynSuccess;
}


/**
 * Set the PMAC axis scale factor to increase resolution in the motor record.
 * Default value is 1.
 * @param axis Axis number to set the PMAC axis scale factor.
 * @param scale Scale factor to set
 */
asynStatus pmacController::pmacSetAxisScale(int axis, int scale) 
{
  pmacAxis *pA = NULL;
  static const char *functionName = "pmacController::pmacSetAxisScale";

  debugFlow(functionName);

  if (scale < 1) {
    asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s: Error: scale factor must be >=1.\n", functionName);
    return asynError;
  }

  this->lock();
  pA = getAxis(axis);
  if (pA) {
    pA->scale_ = scale;
    asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, 
              "%s. Setting scale factor of &d on axis %d, on controller %s.\n", 
              functionName, pA->scale_, pA->axisNo_, portName);

  } else {
    asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
	      "%s: Error: axis %d has not been configured using pmacCreateAxis.\n", functionName, axis);
    return asynError;
  }
  this->unlock();
  return asynSuccess;
}


/**
 * If we have an open loop axis that has an encoder coming back on a different channel
 * then the encoder readback axis number can be set here. This ensures that the encoder
 * will be used for the position readback. It will also ensure that the encoder axis
 * is set correctly when performing a set position on the open loop axis.
 *
 * To use this function, the axis number used for the encoder must have been configured
 * already using pmacCreateAxis.
 *
 * @param controller The Asyn port name for the PMAC controller.
 * @param axis Axis number to set the PMAC axis scale factor.
 * @param encoder_axis The axis number that the encoder is fed into.  
 */
asynStatus pmacController::pmacSetOpenLoopEncoderAxis(int axis, int encoder_axis)
{
  pmacAxis *pA = NULL;
  static const char *functionName = "pmacController::pmacSetOpenLoopEncoderAxis";

  debugFlow(functionName);

  this->lock();
  pA = getAxis(axis);
  if (pA) {
    //Test that the encoder axis has also been configured
    if (getAxis(encoder_axis) == NULL) {
      asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
		"%s: Error: encoder axis %d has not been configured using pmacCreateAxis.\n", functionName, encoder_axis);
      return asynError;
    }
    pA->encoder_axis_ = encoder_axis;
    asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW, 
              "%s. Setting encoder axis &d for axis %d, on controller %s.\n", 
              functionName, pA->encoder_axis_, pA->axisNo_, portName);

  } else {
    asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, 
	      "%s: Error: axis %d has not been configured using pmacCreateAxis.\n", functionName, axis);
    return asynError;
  }
  this->unlock();
  return asynSuccess;
}


asynStatus pmacController::processDeferredMoves(void)
{
  asynStatus status = asynError;
  char command[PMAC_MAXBUF_] = {0};
  char response[32] = {0};
  pmacAxis *pAxis = NULL;
  static const char *functionName = "pmacController::processDeferredMoves";

  debugFlow(functionName);

  //Build up combined move command for all axes involved in the deferred move.
  for (int axis=0; axis<numAxes_; axis++) {
    pAxis = getAxis(axis);
    if (pAxis != NULL) {
      if (pAxis->deferredMove_) {
	sprintf(command, "%s #%d%s%.2f", command, pAxis->axisNo_,
		pAxis->deferredRelative_ ? "J^" : "J=",
		pAxis->deferredPosition_);
      }
    }
  }
  
  //Execute the deferred move
  status = lowLevelWriteRead(command, response);

  //Clear deferred move flag for the axes involved.
  for (int axis=0; axis<numAxes_; axis++) {
    pAxis = getAxis(axis);
    if (pAxis!=NULL) {
      if (pAxis->deferredMove_) {
	pAxis->deferredMove_ = 0;
      }
    }
  }
     
  return status;
}





/*************************************************************************************/
/** The following functions have C linkage, and can be called directly or from iocsh */

extern "C" {

/**
 * C wrapper for the pmacController constructor.
 * See pmacController::pmacController.
 *
 */
asynStatus pmacCreateController(const char *portName, const char *lowLevelPortName, int lowLevelPortAddress, 
				int numAxes, int movingPollPeriod, int idlePollPeriod)
{

    pmacController *ppmacController
      = new pmacController(portName, lowLevelPortName, lowLevelPortAddress, numAxes, movingPollPeriod/1000., idlePollPeriod/1000.);
    ppmacController = NULL;

    return asynSuccess;
}

/**
 * C wrapper for the pmacAxis constructor.
 * See pmacAxis::pmacAxis.
 *
 */
asynStatus pmacCreateAxis(const char *pmacName,         /* specify which controller by port name */
			  int axis)                    /* axis number (start from 1). */
{
  pmacController *pC;
  pmacAxis *pAxis;

  static const char *functionName = "pmacCreateAxis";

  pC = (pmacController*) findAsynPortDriver(pmacName);
  if (!pC) {
    printf("%s:%s: Error port %s not found\n",
           driverName, functionName, pmacName);
    return asynError;
  }
  
  pC->lock();
  pAxis = new pmacAxis(pC, axis);
  pAxis = NULL;
  pC->unlock();
  return asynSuccess;
}

/**
 * C Wrapper function for pmacAxis constructor.
 * See pmacAxis::pmacAxis.
 * This function allows creation of multiple pmacAxis objects with axis numbers 1 to numAxes.
 * @param pmacName Asyn port name for the controller (const char *)
 * @param numAxes The number of axes to create, starting at 1.
 *
 */
asynStatus pmacCreateAxes(const char *pmacName,        
			  int numAxes)                   
{
  pmacController *pC;
  pmacAxis *pAxis;

  static const char *functionName = "pmacCreateAxis";

  pC = (pmacController*) findAsynPortDriver(pmacName);
  if (!pC) {
    printf("%s:%s: Error port %s not found\n",
           driverName, functionName, pmacName);
    return asynError;
  }
  
  pC->lock();
  for (int axis=1; axis<=numAxes; axis++) {
    pAxis = new pmacAxis(pC, axis);
    pAxis = NULL;
  }
  pC->unlock();
  return asynSuccess;
}


/**
 * Disable the check in the axis poller that reads ix24 to check if hardware limits
 * are disabled. By default this is enabled for safety reasons. It sets the motor
 * record PROBLEM bit in MSTA, which results in the record going into MAJOR/STATE alarm.
 * @param controller Asyn port name for the controller (const char *)
 * @param axis Axis number to disable the check for.
 * @param allAxes Set to 0 if only dealing with one axis. 
 *                Set to 1 to do all axes (in which case the axis parameter is ignored).
 */
asynStatus pmacDisableLimitsCheck(const char *controller, int axis, int allAxes)
{
  pmacController *pC;
  static const char *functionName = "pmacDisableLimitsCheck";

  pC = (pmacController*) findAsynPortDriver(controller);
  if (!pC) {
    cout << driverName << "::" << functionName << " Error port " << controller << " not found." << endl;
    return asynError;
  }

  if (allAxes == 1) {
    return pC->pmacDisableLimitsCheck();
  } else if (allAxes == 0) {
    return pC->pmacDisableLimitsCheck(axis);
  }

  return asynError;
}


/**
 * Set the PMAC axis scale factor to increase resolution in the motor record.
 * Default value is 1.
 * @param controller The Asyn port name for the PMAC controller.
 * @param axis Axis number to set the PMAC axis scale factor.
 * @param scale Scale factor to set
 */
asynStatus pmacSetAxisScale(const char *controller, int axis, int scale)
{
  pmacController *pC;
  static const char *functionName = "pmacSetAxisScale";

  pC = (pmacController*) findAsynPortDriver(controller);
  if (!pC) {
    cout << driverName << "::" << functionName << " Error: port " << controller << " not found." << endl;
    return asynError;
  }
    
  return pC->pmacSetAxisScale(axis, scale);
}

  
/**
 * If we have an open loop axis that has an encoder coming back on a different channel
 * then the encoder readback axis number can be set here. This ensures that the encoder
 * will be used for the position readback. It will also ensure that the encoder axis
 * is set correctly when performing a set position on the open loop axis.
 *
 * To use this function, the axis number used for the encoder must have been configured
 * already using pmacCreateAxis.
 *
 * @param controller The Asyn port name for the PMAC controller.
 * @param axis Axis number to set the PMAC axis scale factor.
 * @param encoder_axis The axis number that the encoder is fed into.  
 */
asynStatus pmacSetOpenLoopEncoderAxis(const char *controller, int axis, int encoder_axis)
{
  pmacController *pC;
  static const char *functionName = "pmacSetOpenLoopEncoderAxis";

  pC = (pmacController*) findAsynPortDriver(controller);
  if (!pC) {
    cout << driverName << "::" << functionName << " Error: port " << controller << " not found." << endl;
    return asynError;
  }
    
  return pC->pmacSetOpenLoopEncoderAxis(axis, encoder_axis);
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
static const iocshFuncDef configpmacCreateController = {"pmacCreateController", 6, pmacCreateControllerArgs};
static void configpmacCreateControllerCallFunc(const iocshArgBuf *args)
{
  pmacCreateController(args[0].sval, args[1].sval, args[2].ival, args[3].ival, args[4].ival, args[5].ival);
}


/* pmacCreateAxis */
static const iocshArg pmacCreateAxisArg0 = {"Controller port name", iocshArgString};
static const iocshArg pmacCreateAxisArg1 = {"Axis number", iocshArgInt};
static const iocshArg * const pmacCreateAxisArgs[] = {&pmacCreateAxisArg0,
                                                     &pmacCreateAxisArg1};
static const iocshFuncDef configpmacAxis = {"pmacCreateAxis", 2, pmacCreateAxisArgs};

static void configpmacAxisCallFunc(const iocshArgBuf *args)
{
  pmacCreateAxis(args[0].sval, args[1].ival);
}

/* pmacCreateAxes */
static const iocshArg pmacCreateAxesArg0 = {"Controller port name", iocshArgString};
static const iocshArg pmacCreateAxesArg1 = {"Num Axes", iocshArgInt};
static const iocshArg * const pmacCreateAxesArgs[] = {&pmacCreateAxesArg0,
                                                     &pmacCreateAxesArg1};
static const iocshFuncDef configpmacAxes = {"pmacCreateAxes", 2, pmacCreateAxesArgs};

static void configpmacAxesCallFunc(const iocshArgBuf *args)
{
  pmacCreateAxes(args[0].sval, args[1].ival);
}


/* pmacDisableLimitsCheck */
static const iocshArg pmacDisableLimitsCheckArg0 = {"Controller port name", iocshArgString};
static const iocshArg pmacDisableLimitsCheckArg1 = {"Axis number", iocshArgInt};
static const iocshArg pmacDisableLimitsCheckArg2 = {"All Axes", iocshArgInt};
static const iocshArg * const pmacDisableLimitsCheckArgs[] = {&pmacDisableLimitsCheckArg0,
							      &pmacDisableLimitsCheckArg1,
							      &pmacDisableLimitsCheckArg2};
static const iocshFuncDef configpmacDisableLimitsCheck = {"pmacDisableLimitsCheck", 3, pmacDisableLimitsCheckArgs};

static void configpmacDisableLimitsCheckCallFunc(const iocshArgBuf *args)
{
  pmacDisableLimitsCheck(args[0].sval, args[1].ival, args[2].ival);
}



/* pmacSetAxisScale */
static const iocshArg pmacSetAxisScaleArg0 = {"Controller port name", iocshArgString};
static const iocshArg pmacSetAxisScaleArg1 = {"Axis number", iocshArgInt};
static const iocshArg pmacSetAxisScaleArg2 = {"Scale", iocshArgInt};
static const iocshArg * const pmacSetAxisScaleArgs[] = {&pmacSetAxisScaleArg0,
							      &pmacSetAxisScaleArg1,
							      &pmacSetAxisScaleArg2};
static const iocshFuncDef configpmacSetAxisScale = {"pmacSetAxisScale", 3, pmacSetAxisScaleArgs};

static void configpmacSetAxisScaleCallFunc(const iocshArgBuf *args)
{
  pmacSetAxisScale(args[0].sval, args[1].ival, args[2].ival);
}

/* pmacSetOpenLoopEncoderAxis */
static const iocshArg pmacSetOpenLoopEncoderAxisArg0 = {"Controller port name", iocshArgString};
static const iocshArg pmacSetOpenLoopEncoderAxisArg1 = {"Axis number", iocshArgInt};
static const iocshArg pmacSetOpenLoopEncoderAxisArg2 = {"Encoder Axis", iocshArgInt};
static const iocshArg * const pmacSetOpenLoopEncoderAxisArgs[] = {&pmacSetOpenLoopEncoderAxisArg0,
								  &pmacSetOpenLoopEncoderAxisArg1,
								  &pmacSetOpenLoopEncoderAxisArg2};
static const iocshFuncDef configpmacSetOpenLoopEncoderAxis = {"pmacSetOpenLoopEncoderAxis", 3, pmacSetOpenLoopEncoderAxisArgs};

static void configpmacSetOpenLoopEncoderAxisCallFunc(const iocshArgBuf *args)
{
  pmacSetOpenLoopEncoderAxis(args[0].sval, args[1].ival, args[2].ival);
}


static void pmacControllerRegister(void)
{
  iocshRegister(&configpmacCreateController,   configpmacCreateControllerCallFunc);
  iocshRegister(&configpmacAxis,               configpmacAxisCallFunc);
  iocshRegister(&configpmacAxes,               configpmacAxesCallFunc);
  iocshRegister(&configpmacDisableLimitsCheck, configpmacDisableLimitsCheckCallFunc);
  iocshRegister(&configpmacSetAxisScale, configpmacSetAxisScaleCallFunc);
  iocshRegister(&configpmacSetOpenLoopEncoderAxis, configpmacSetOpenLoopEncoderAxisCallFunc);
}
epicsExportRegistrar(pmacControllerRegister);

#ifdef vxWorks
  //VxWorks register functions
  epicsRegisterFunction(pmacCreateController);
  epicsRegisterFunction(pmacCreateAxis);
  epicsRegisterFunction(pmacCreateAxes);
  epicsRegisterFunction(pmacDisableLimitsCheck);
  epicsRegisterFunction(pmacSetAxisScale);
  epicsRegisterFunction(pmacSetOpenLoopEncoderAxis);
#endif
} // extern "C"

