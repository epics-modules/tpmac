#ifndef DRV_MOTOR_SIM_H
#define DRV_MOTOR_SIM_H

#ifdef __cplusplus
extern "C" {
#endif

int pmacAsynCoordCreate( char *port, int addr, int cs, int ref, int program );
int pmacSetCoordMovingPollPeriod(int cs, int movingPollPeriod);
int pmacSetCoordIdlePollPeriod(int cs, int idlePollPeriod);

#ifdef __cplusplus
}
#endif
#endif
