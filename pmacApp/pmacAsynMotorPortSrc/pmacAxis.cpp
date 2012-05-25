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

// These are the pmacAxis class methods
pmacAxis::pmacAxis(pmacController *pC, int axisNo)
  :   asynMotorAxis(pC, axisNo),
      pC_(pC)
{
  static const char *functionName = "pmacAxis::pmacAxis";
  char *index = NULL;
  int status = 0;
  
  pC_->myDebug(functionName); 

  /* Set an EPICS exit handler that will shut down polling before asyn kills the IP sockets */
  epicsAtExit(shutdownCallback, pC_);

  callParamCallbacks();

  /* Wake up the poller task which will make it do a poll, 
   * updating values for this axis to use the new resolution (stepSize_) */   
  pC_->wakeupPoller();
 
}

pmacAxis::~pmacAxis() 
{
  //Destructor
}


asynStatus pmacAxis::move(double position, int relative, double min_velocity, double max_velocity, double acceleration)
{
  char errorString[100] = {0};
  double deviceUnits = 0.0;
  int status = 0;
  static const char *functionName = "pmacAxis::move";

  pC_->myDebug(functionName);  

  return asynSuccess;
}


 
asynStatus pmacAxis::home(double min_velocity, double max_velocity, double acceleration, int forwards)
{
  int status = 0;
  char errorBuffer[100] = {0};
  static const char *functionName = "pmacAxis::home";

  pC_->myDebug(functionName); 

  return asynSuccess;

}

asynStatus pmacAxis::moveVelocity(double min_velocity, double max_velocity, double acceleration)
{
  int status = 0;
  double deviceVelocity = 0.0;
  double deviceAcceleration = 0.0;
  static const char *functionName = "pmacAxis::moveVelocity";

  pC_->myDebug(functionName);  

  return asynSuccess;
}


asynStatus pmacAxis::setPosition(double position)
{
  int status = 0;
  static const char *functionName = "pmacAxis::setPosition";
  
  pC_->myDebug(functionName);  

  return asynSuccess;
}

asynStatus pmacAxis::stop(double acceleration)
{
  int status = 0;
  static const char *functionName = "pmacAxis::stopAxis";

  pC_->myDebug(functionName); 

  return asynSuccess;
}

asynStatus pmacAxis::poll(bool *moving)
{
  int status = 0;
  int axisDone = 0;
  char message[pC_->m_MAXBUF];
  char command[pC_->m_MAXBUF];
  char response[pC_->m_MAXBUF];
  static const char *functionName = "pmacAxis::poll";

  sprintf(message, "%s: Polling axis: %d", functionName, this->axisNo_);
  pC_->myDebug(message); 
  
  sprintf(command, "#%d ???", this->axisNo_);
  this->pC_->lowLevelWriteRead(command, response);

  callParamCallbacks();
  return status ? asynError : asynSuccess;
}


