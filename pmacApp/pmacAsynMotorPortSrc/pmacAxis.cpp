/********************************************
 *  pmacAxis.cpp
 * 
 *  PMAC Asyn motor based on the 
 *  asynMotorAxis class.
 * 
 *  Matthew Pearson
 *  23 May 2012
 * 
 ********************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsExport.h>
#include <epicsExit.h>
#include <epicsString.h>
#include <iocsh.h>

#include "pmacController.h"

static const char *driverName = "pmacAxis";

/////////////////replace with a runtime function that can be called on IOC shell.////////////////////////
/////////////////Or, provide an overloaded constructor with this as an argument.////////////////////////
/* This #define affects the behaviour of the driver.

   REMOVE_LIMITS_ON_HOME removes the limits protection when homing if
    - ms??,i912 indicates you are homing onto a limit
    - ms??,i913 and the home velocity indicate that the limit you trigger.
      for home detection is the one you are homing towards.
    - any home offset is in the opposite sense to the home velocity.
*/

#define REMOVE_LIMITS_ON_HOME

extern "C" void shutdownCallback(void *pPvt)
{
  pmacController *pC = static_cast<pmacController *>(pPvt);

  pC->lock();
  pC->shuttingDown_ = 1;
  pC->unlock();
}

// These are the pmacAxis class methods
pmacAxis::pmacAxis(pmacController *pC, int axisNo)
  :   asynMotorAxis(pC, axisNo),
      pC_(pC)
{
  static const char *functionName = "pmacAxis::pmacAxis";

  pC_->debugFlow(functionName); 

  //Initialize non-static data members
  setpointPosition_ = 0.0;
  encoderPosition_ = 0.0;
  currentVelocity_ = 0.0;
  velocity_ = 0.0;
  accel_ = 0.0;
  highLimit_ = 0.0;
  lowLimit_ = 0.0;
  limitsDisabled_ = 0;
  stepSize_ = 1; //Don't need?
  deferredPosition_ = 0.0;
  deferredMove_ = 0;
  deferredRelative_ = 0;
  scale_ = 1;
  previous_position_ = 0.0;
  previous_direction_ = 0;
  amp_enabled_ = 0;
  fatal_following_ = 0;
  encoder_axis_ = 0;
  limitsCheckDisable_ = 0;
  errorPrintCount_ = 0;
  errorPrintFlag_ = 0;

  /* Set an EPICS exit handler that will shut down polling before asyn kills the IP sockets */
  epicsAtExit(shutdownCallback, pC_);

  //Do an initial poll to get some values from the PMAC
  if (getAxisInitialStatus() != asynSuccess) {
    asynPrint(pC_->pasynUserSelf, ASYN_TRACE_ERROR,
	      "%s: getAxisInitialStatus failed to return asynSuccess. Controller: %s, Axis: %d.\n", functionName, pC_->portName, axisNo_);
  }

  callParamCallbacks();

  /* Wake up the poller task which will make it do a poll, 
   * updating values for this axis to use the new resolution (stepSize_) */   
  pC_->wakeupPoller();
 
}

asynStatus pmacAxis::getAxisInitialStatus(void)
{
  char command[pC_->PMAC_MAXBUF_];
  char response[pC_->PMAC_MAXBUF_];
  int cmdStatus = 0;
  double low_limit = 0.0;
  double high_limit = 0.0;
  double pgain = 0.0;
  double igain = 0.0;
  double dgain = 0.0;
  int nvals = 0;

  static const char *functionName = "pmacAxis::getAxisInitialStatus";

  sprintf(command, "I%d13 I%d14 I%d30 I%d31 I%d33", axisNo_, axisNo_, axisNo_, axisNo_, axisNo_);
  cmdStatus = pC_->lowLevelWriteRead(command, response);
  nvals = sscanf(response, "%lf %lf %lf %lf %lf", &high_limit, &low_limit, &pgain, &dgain, &igain);

  if (cmdStatus || nvals != 5) {
    asynPrint(pC_->pasynUserSelf, ASYN_TRACE_ERROR, "%s: Error: initial status poll failed on axis %d.\n", functionName, axisNo_);
    return asynError;
  } else {
    setDoubleParam(pC_->motorLowLimit_,  low_limit*scale_);
    setDoubleParam(pC_->motorHighLimit_, high_limit*scale_);
    setDoubleParam(pC_->motorPgain_,     pgain);
    setDoubleParam(pC_->motorIgain_,     igain);
    setDoubleParam(pC_->motorDgain_,     dgain);
    setIntegerParam(pC_->motorStatusHasEncoder_, 1);
  }

  return asynSuccess;
}


pmacAxis::~pmacAxis() 
{
  //Destructor
}


asynStatus pmacAxis::move(double position, int relative, double min_velocity, double max_velocity, double acceleration)
{
  asynStatus status = asynError;
  static const char *functionName = "pmacAxis::move";

  pC_->debugFlow(functionName);  

  char acc_buff[32]="\0";
  char vel_buff[32]="\0";
  char command[128] = {0};
  char response[32] = {0};

  if (max_velocity != 0) {
    sprintf(vel_buff, "I%d22=%f ", axisNo_, (max_velocity / (scale_ * 1000.0) ));
  }
  if (acceleration != 0) {
    if (max_velocity != 0) {
      sprintf(acc_buff, "I%d20=%f ", axisNo_, (fabs(max_velocity/acceleration) * 1000.0));
    }
  }

  if (pC_->movesDeferred_ == 0) {
    sprintf( command, "%s%s#%d %s%.2f", vel_buff, acc_buff, axisNo_,
	     (relative?"J^":"J="), position/scale_ );
  } else { /* deferred moves */
    sprintf( command, "%s%s", vel_buff, acc_buff);
    deferredPosition_ = position/scale_;
    deferredMove_ = 1;
    deferredRelative_ = relative;
  }
        
#ifdef REMOVE_LIMITS_ON_HOME
  if (limitsDisabled_) {
    char buffer[32];
    /* Re-enable limits */
    sprintf( buffer, " i%d24=i%d24&$FDFFFF", axisNo_, axisNo_);
    strcat( command, buffer );
    limitsDisabled_ = 0;
  }
#endif
  status = pC_->lowLevelWriteRead(command, response);
  
  return status;
}


 
asynStatus pmacAxis::home(double min_velocity, double max_velocity, double acceleration, int forwards)
{
  asynStatus status = asynError;
  char command[128] = {0};
  char response[128] = {0};
  static const char *functionName = "pmacAxis::home";

  pC_->debugFlow(functionName); 

  sprintf(command, "#%d HOME", axisNo_);
  
#ifdef REMOVE_LIMITS_ON_HOME
 /* If homing onto an end-limit and home velocity is in the right direction, clear limits protection */
  int macro_station = ((axisNo_-1)/2)*4 + (axisNo_-1)%2;
  int home_type = 0;
  int home_flag = 0;
  int flag_mode = 0;
  int nvals = 0;
  int home_offset = 0;
  int controller_type = 0;
  double home_velocity = 0.0;
  char buffer[128] = {0};

  /* Discover type of controller */
  strcpy(buffer, "cid");
  status = pC_->lowLevelWriteRead(buffer, response);
  nvals  = sscanf(response, "%d", &controller_type);
  
  if (controller_type == 603382) {
    printf("%s. This is a Geobrick LV\n", functionName);
  } else if (controller_type == 602413) {
    printf("%s. This is a Turbo PMAC 2 Ultralite\n", functionName);
  } else {
    printf("%s. Error. Unknown controller type = %d\n", functionName, controller_type);
    return asynError;
  }

  if (controller_type == 603382) {
    /* Read home flags and home direction from Geobrick LV */ 
    if (axisNo_ < 5) {
      sprintf(buffer, "I70%d2 I70%d3 i%d24 i%d23 i%d26", axisNo_, axisNo_, axisNo_, axisNo_, axisNo_);
    } else {
      sprintf(buffer, "I71%d2 I71%d3 i%d24 i%d23 i%d26", axisNo_-4, axisNo_-4, axisNo_, axisNo_, axisNo_);
    }
    printf("%s\n", buffer);
    status = pC_->lowLevelWriteRead(buffer, response);
    nvals = sscanf(response, "%d %d $%x %lf %d", &home_type, &home_flag, &flag_mode, &home_velocity, &home_offset);
  }
  
  if( controller_type == 602413 ) {
    /* Read home flags and home direction from VME PMAC */ 
    sprintf(buffer, "ms%d,i912 ms%d,i913 i%d24 i%d23 i%d26", macro_station, macro_station, axisNo_, axisNo_, axisNo_);
    status = pC_->lowLevelWriteRead(buffer, response);
    nvals = sscanf(response, "$%x $%x $%x %lf %d", &home_type, &home_flag, &flag_mode, &home_velocity, &home_offset);
  }

  if (status || (nvals !=5)) {
    asynPrint(pC_->pasynUserSelf, ASYN_TRACE_ERROR,
	      "%s: Error: Cannot read home flags. status=%d, nvals=%d. Controller: %s, Axis: %d.\n", 
	      functionName, status, nvals, pC_->portName, axisNo_);
    return asynError;
  }

  printf("home_type = %d, home_flag = %d, flag_mode = %x, home_velocity = %f, home_offset = %d\n", home_type, home_flag, flag_mode, home_velocity, home_offset );
  printf("status = %d, nvals = %d\n", status, nvals);
  
  if (max_velocity != 0) {
    home_velocity = (forwards?1:-1)*(fabs(max_velocity) / 1000.0);
  }

  if ( ( home_type <= 15 )      && 
       ( home_type % 4 >= 2 )   &&
       !( flag_mode & 0x20000 ) &&
       (( home_velocity > 0 && home_flag == 1 && home_offset <= 0 ) || 
	( home_velocity < 0 && home_flag == 2 && home_offset >= 0 )    )   ) {
      sprintf(buffer, " i%d24=i%d24|$20000", axisNo_, axisNo_ );
      strcat(command, buffer);
      limitsDisabled_ = 1;
      asynPrint(pC_->pasynUserSelf, ASYN_TRACE_FLOW, 
		"%s. Disabling limits whilst homing PMAC controller %s, axis %d, type:%d, flag:$%x, vel:%f\n",
		functionName, pC_->portName, axisNo_, home_type, home_flag, home_velocity);
  } else {
    asynPrint(pC_->pasynUserSelf, ASYN_TRACE_ERROR,
	      "%s: Error: Cannot disable limits to home PMAC controller %s, axis %d, type:%x, flag:$%d, vel:%f, mode:0x%x, offset: %d\n", 
	      functionName, pC_->portName, axisNo_, home_type, home_flag, home_velocity, flag_mode, home_offset);
  }
#endif
  status = pC_->lowLevelWriteRead(command, response);

  return status;
}

asynStatus pmacAxis::moveVelocity(double min_velocity, double max_velocity, double acceleration)
{
  asynStatus status = asynError;
  char acc_buff[32] = "\0";
  char vel_buff[32] = "\0";
  char command[128] = {0};
  char response[32] = {0};
  static const char *functionName = "pmacAxis::moveVelocity";

  pC_->debugFlow(functionName);  

  if (max_velocity != 0) {
    sprintf(vel_buff, "I%d22=%f ", axisNo_, (fabs(max_velocity) / (scale_ * 1000.0)));
  }
  if (acceleration != 0) {
    if (max_velocity != 0) {
      sprintf(acc_buff, "I%d20=%f ", axisNo_, (fabs(max_velocity/acceleration) * 1000.0));
    }
  }
  sprintf(command, "%s%s#%d %s", vel_buff, acc_buff, axisNo_, (max_velocity < 0 ? "J-": "J+") );

#ifdef REMOVE_LIMITS_ON_HOME
  if (limitsDisabled_) {
    char buffer[32];
    /* Re-enable limits */
    sprintf(buffer, " i%d24=i%d24&$FDFFFF", axisNo_, axisNo_);
    strcat(command, buffer);
    limitsDisabled_ = 0;
  }
#endif
  status = pC_->lowLevelWriteRead(command, response);

  return status;
}


asynStatus pmacAxis::setPosition(double position)
{
  //int status = 0;
  static const char *functionName = "pmacAxis::setPosition";
  
  pC_->debugFlow(functionName);  

  return asynSuccess;
}

asynStatus pmacAxis::stop(double acceleration)
{
  asynStatus status = asynError;
  static const char *functionName = "pmacAxis::stopAxis";

  pC_->debugFlow(functionName); 

  char command[128] = {0};
  char response[32] = {0};

  /*Only send a J/ if the amplifier output is enabled. When we send a stop, 
    we don't want to power on axes that have been powered off for a reason.*/
  if ((amp_enabled_ == 1) || (fatal_following_ == 1)) {
    sprintf( command, "#%d J/ M%d40=1",  axisNo_, axisNo_ );
  } else {
    /*Just set the inposition bit in this case.*/
    sprintf( command, "M%d40=1",  axisNo_ );
  }
  deferredMove_ = 0;

  status = pC_->lowLevelWriteRead(command, response);
  return status;
}

asynStatus pmacAxis::poll(bool *moving)
{
  int status = 0;
  //int axisDone = 0;
  int globalStatus = 0;
  char message[pC_->PMAC_MAXBUF_];
  //char command[pC_->PMAC_MAXBUF_];
  //char response[pC_->PMAC_MAXBUF_];
  static const char *functionName = "pmacAxis::poll";

  sprintf(message, "%s: Polling axis: %d", functionName, this->axisNo_);
  pC_->debugFlow(message); 

  if (!pC_->lowLevelPortUser_) {
    setIntegerParam(pC_->motorStatusCommsError_, 1);
    return asynError;
  }
  
  //Set axis problem bits based on the controller status (obtained in the controller poll).
  if (pC_->getIntegerParam(pC_->PMAC_C_GlobalStatus_, &globalStatus)) {
    asynPrint(pC_->pasynUserSelf, ASYN_TRACE_ERROR, "%s: Could not read controller %s global status.\n", functionName, pC_->portName);
  }
  setIntegerParam(pC_->motorStatusProblem_, globalStatus);
      
  //Now poll axis status
  if ((status = getAxisStatus(moving)) != asynSuccess) {
    asynPrint(pC_->pasynUserSelf, ASYN_TRACE_ERROR,
	      "%s: getAxisStatus failed to return asynSuccess. Controller: %s, Axis: %d.\n", functionName, pC_->portName, axisNo_);
  }
  
  callParamCallbacks();
  return status ? asynError : asynSuccess;
}



asynStatus pmacAxis::getAxisStatus(bool *moving)
{
    char command[pC_->PMAC_MAXBUF_];
    char response[pC_->PMAC_MAXBUF_];
    int cmdStatus = 0;; 
    int done = 0;
    double position = 0; 
    double enc_position = 0;
    int nvals = 0;
    epicsUInt32 status[2] = {0};
    int axisProblemFlag = 0;
    int limitsDisabledBit = 0;
    int errorPrintMin = 10000;
    
    /* Keep error messages from being printed each poll.*/
    if (errorPrintCount_ > errorPrintMin) {
      errorPrintCount_ = 0;
      errorPrintFlag_ = 0;
    }
    errorPrintCount_++;
            
    /* Read all the status for this axis in one go */
    if (encoder_axis_ != 0) {
      /* Encoder position comes back on a different axis */
      sprintf( command, "#%d ? P #%d P", axisNo_,  encoder_axis_);
    } else {
      /* Encoder position comes back on this axis - note we initially read 
	 the following error into the position variable */
      sprintf( command, "#%d ? F P", axisNo_);
    }
    
    cmdStatus = pC_->lowLevelWriteRead(command, response);
    nvals = sscanf( response, "%6x%6x %lf %lf", &status[0], &status[1], &position, &enc_position );
	
    if ( cmdStatus || nvals != 4) {
      asynPrint(pC_->pasynUserSelf, ASYN_TRACE_ERROR,
		"drvPmacAxisGetStatus: not all status values returned. Status: %d\nCommand :%s\nResponse:%s",
		cmdStatus, command, response );
    } else {
      int homeSignal = ((status[1] & pC_->PMAC_STATUS2_HOME_COMPLETE) != 0);
      int direction = 0;
      
      /* For closed loop axes, position is actually following error up to this point */ 
      if ( encoder_axis_ == 0 ) {
	position += enc_position;
      }
      
      position *= scale_;
      enc_position  *= scale_;
      
      setDoubleParam(pC_->motorPosition_, position);
      setDoubleParam(pC_->motorEncoderPosition_, enc_position);
      
      /* Use previous position and current position to calculate direction.*/
      if ((position - previous_position_) > 0) {
	direction = 1;
      } else if (position - previous_position_ == 0.0) {
	direction = previous_direction_;
      } else {
	direction = 0;
      }
      setIntegerParam(pC_->motorStatusDirection_, direction);
      /*Store position to calculate direction for next poll.*/
      previous_position_ = position;
      previous_direction_ = direction;

      if(deferredMove_) {
	done = 0; 
      } else {
	done = (((status[1] & pC_->PMAC_STATUS2_IN_POSITION) != 0) || ((status[0] & pC_->PMAC_STATUS1_MOTOR_ON) == 0)); 
	/*If we are not done, but amp has been disabled, then set done (to stop when we get following errors).*/
	if ((done == 0) && ((status[0] & pC_->PMAC_STATUS1_AMP_ENABLED) == 0)) {
	  done = 1;
	}
      }

      if (!done) {
	*moving = true;
      } else {
	*moving = false;
      }

      setIntegerParam(pC_->motorStatusDone_, done);
      setIntegerParam(pC_->motorStatusHighLimit_, ((status[0] & pC_->PMAC_STATUS1_POS_LIMIT_SET) != 0) );
      setIntegerParam(pC_->motorStatusHomed_, homeSignal);
      /*If desired_vel_zero is false && motor activated (ix00=1) && amplifier enabled, set moving=1.*/
      setIntegerParam(pC_->motorStatusMoving_, ((status[0] & pC_->PMAC_STATUS1_DESIRED_VELOCITY_ZERO) == 0) && ((status[0] & pC_->PMAC_STATUS1_MOTOR_ON) != 0) && ((status[0] & pC_->PMAC_STATUS1_AMP_ENABLED) != 0) );
      setIntegerParam(pC_->motorStatusLowLimit_, ((status[0] & pC_->PMAC_STATUS1_NEG_LIMIT_SET)!=0) );
      setIntegerParam(pC_->motorStatusFollowingError_,((status[1] & pC_->PMAC_STATUS2_ERR_FOLLOW_ERR) != 0) );
      fatal_following_ = ((status[1] & pC_->PMAC_STATUS2_ERR_FOLLOW_ERR) != 0);

      axisProblemFlag = 0;
      /*Set any axis specific general problem bits.*/
      if ( ((status[0] & pC_->PMAX_AXIS_GENERAL_PROB1) != 0) || ((status[1] & pC_->PMAX_AXIS_GENERAL_PROB2) != 0) ) {
	axisProblemFlag = 1;
      }
      /*Check limits disabled bit in ix24, and if we haven't intentially disabled limits
	because we are homing, set the motorAxisProblem bit. Also check the limitsCheckDisable
	flag, which the user can set to disable this feature.*/
      if (!limitsCheckDisable_) {
	/*Check we haven't intentially disabled limits for homing.*/
	if (!limitsDisabled_) {
	  sprintf( command, "i%d24", axisNo_);
	  cmdStatus = pC_->lowLevelWriteRead(command, response);
	  if (cmdStatus == asynSuccess) {
	    sscanf(response, "$%x", &limitsDisabledBit);
	    limitsDisabledBit = ((0x20000 & limitsDisabledBit) >> 17);
	    if (limitsDisabledBit) {
	      axisProblemFlag = 1;
	      if (errorPrintFlag_ == 0) {
		asynPrint(pC_->pasynUserSelf, ASYN_TRACE_ERROR, "*** WARNING *** Limits are disabled on controller %s, axis %d\n", pC_->portName, axisNo_);
		errorPrintFlag_ = 1;
	      }
	    }
	  }
	}
      }
      setIntegerParam(pC_->motorStatusProblem_, axisProblemFlag);
      
      /* Clear error print flag for this axis if problem has been removed.*/
      if (axisProblemFlag == 0) {
	errorPrintFlag_ = 0;
      }
            

    }

#ifdef REMOVE_LIMITS_ON_HOME
    if (limitsDisabled_ && (status[1] & pC_->PMAC_STATUS2_HOME_COMPLETE) && (status[0] & pC_->PMAC_STATUS1_DESIRED_VELOCITY_ZERO) ) {
      /* Re-enable limits */
      sprintf( command, "i%d24=i%d24&$FDFFFF", axisNo_, axisNo_);
      cmdStatus = pC_->lowLevelWriteRead(command, response);
      limitsDisabled_ = (cmdStatus != 0);
    }
#endif
    /*Set amplifier enabled bit.*/
    if ((status[0] & pC_->PMAC_STATUS1_AMP_ENABLED) != 0) {
      amp_enabled_ = 1;
    } else {
      amp_enabled_ = 0;
    }
    
    return asynSuccess;
}

