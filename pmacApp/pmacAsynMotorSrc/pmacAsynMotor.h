#ifndef DRV_MOTOR_SIM_H
#define DRV_MOTOR_SIM_H

#ifdef __cplusplus
extern "C" {
#endif

int pmacAsynMotorCreate( char *port, int addr, int card, int nAxes );
int pmacSetMovingPollPeriod(int card, int movingPollPeriod);
int pmacSetIdlePollPeriod(int card, int idlePollPeriod);

#ifdef __cplusplus
}
#endif
#endif
