#ifndef testPmacAsynIPPort_H
#define testPmacAsynIPPort_H

#include <shareLib.h>
#include <epicsExport.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

epicsShareFunc int testPmacAsynIPPort(const char *portName,int addr);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* testPmacAsynIPPort_H */
