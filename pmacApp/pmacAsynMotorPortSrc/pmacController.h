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

  virtual ~pmacController();

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
  asynUser* m_LowLevelPortUser;
  epicsUInt32 debugFlag;
  asynStatus lowLevelWriteRead(char *command, char *response);
  int lowLevelPortConnect(const char *port, int addr, asynUser **ppasynUser, char *inputEos, char *outputEos);
  void myDebug(const char *message);

  static const epicsUInt32 m_MAXBUF;
  static const epicsUInt32 m_TIMEOUT;

  friend class pmacAxis;

};


#endif /* pmacController_H */
