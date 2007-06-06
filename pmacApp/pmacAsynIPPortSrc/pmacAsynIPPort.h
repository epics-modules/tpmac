#ifndef asynInterposePmac_H
#define asynInterposePmac_H

#include <shareLib.h>
#include <epicsExport.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

epicsShareFunc int pmacAsynIPPortConfig(const char *portName,int addr);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* asynInterposePmac_H */
