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

#ifndef pmacAxis_H
#define pmacAxis_H

#include "asynMotorController.h"
#include "asynMotorAxis.h"

class pmacController;

class pmacAxis : public asynMotorAxis
{
  public:
  /* These are the methods we override from the base class */
  pmacAxis(pmacController *pController, int axisNo, double stepSize);
  asynStatus move(double position, int relative, double min_velocity, double max_velocity, double acceleration);
  asynStatus moveVelocity(double min_velocity, double max_velocity, double acceleration);
  asynStatus home(double min_velocity, double max_velocity, double acceleration, int forwards);
  asynStatus stop(double acceleration);
  asynStatus poll(bool *moving);
  asynStatus setPosition(double position);
  
  private:
  pmacController *pC_;
  
  double setpointPosition_;
  double encoderPosition_;
  double currentVelocity_;
  double velocity_;
  double accel_;
  double highLimit_;
  double lowLimit_;
  double stepSize_;
  int axisStatus_;
  double deferredPosition_;
  int deferredMove_;
  int deferredRelative_;

  friend class pmacController;
};

#endif /* pmacAxis_H */
