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


extern "C" void shutdownCallback(void *pPvt)
{
  pmacController *pC = static_cast<pmacController *>(pPvt);

  pC->lock();
  pC->shuttingDown_ = 1;
  pC->unlock();
}

// These are the pmacAxis:: methods
pmacAxis::pmacAxis(pmacController *pC, int axisNo, double stepSize)
  :   asynMotorAxis(pC, axisNo),
      pC_(pC)
{
  static const char *functionName = "pmacAxis::pmacAxis";
  char *index;
  int status;
  
  /* Set an EPICS exit handler that will shut down polling before asyn kills the IP sockets */
  epicsAtExit(shutdownCallback, pC_);

  callParamCallbacks();

  /* Wake up the poller task which will make it do a poll, 
   * updating values for this axis to use the new resolution (stepSize_) */   
  pC_->wakeupPoller();

  printf("  %s\n", functionName); 
}


asynStatus pmacAxis::move(double position, int relative, double min_velocity, double max_velocity, double acceleration)
{
  char errorString[100];
  double deviceUnits;
  int status;
  static const char *functionName = "move";

  printf("  %s\n", functionName); 

  return asynSuccess;
}


 
asynStatus pmacAxis::home(double min_velocity, double max_velocity, double acceleration, int forwards)
{
  int status;
  int groupStatus;
  char errorBuffer[100];
  static const char *functionName = "home";

  printf("  %s\n", functionName); 

  return asynSuccess;

}

asynStatus pmacAxis::moveVelocity(double min_velocity, double max_velocity, double acceleration)
{
  int status;
  double deviceVelocity, deviceAcceleration;
  static const char *functionName = "moveVelocity";

  printf("  %s\n", functionName); 

  return asynSuccess;
}


asynStatus pmacAxis::setPosition(double position)
{
  
  static const char *functionName = "setPosition";
  
  printf("  %s\n", functionName); 

  return asynSuccess;
}

asynStatus pmacAxis::stop(double acceleration)
{
  int status;
  static const char *functionName = "stopAxis";

  printf("  %s\n", functionName); 

  return asynSuccess;
}

asynStatus pmacAxis::poll(bool *moving)
{
  int status;
  int axisDone;
  static const char *functionName = "pmacAxis::poll";

  printf("  %s\n", functionName); 

  callParamCallbacks();
  return status ? asynError : asynSuccess;
}


