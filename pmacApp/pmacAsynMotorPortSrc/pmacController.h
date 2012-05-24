/********************************************
 *  pmacController.h 
 * 
 *  PMAC Asyn motor based on the 
 *  asynMotorController class.
 * 
 *  Matthew Pearson
 *  23 May 2012
 * 
 ********************************************/

#ifndef pmacController_H
#define pmacController_H

#include "asynMotorController.h"
#include "asynMotorAxis.h"
#include "pmacAxis.h"

class pmacController : public asynMotorController {

 public:
  pmacController(const char *portName, const char *lowLevelPortName, int lowLevelPortAddress, int numAxes, double movingPollPeriod, 
		 double idlePollPeriod);

  /* These are the methods that we override */
  asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
  asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
  void report(FILE *fp, int level);
  pmacAxis* getAxis(asynUser *pasynUser);
  pmacAxis* getAxis(int axisNo);
  asynStatus poll();

 protected:
  pmacAxis **pAxes_;       /**< Array of pointers to axis objects */

 private:
  asynUser* lowLevelPortUser;
  asynStatus writeRead(const char *command, char *response);
  int motorAxisAsynConnect(const char *port, int addr, asynUser **ppasynUser, char *inputEos, char *outputEos);

  static int _MAXBUF;

};


#endif /* pmacController_H */
