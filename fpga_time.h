#ifndef __FPGA_TIME_H__
#define __FPGA_TIME_H__


#include "common.h"
extern void SetRbClockAlign_Once();

extern void GetFpgaRuningTime(TimeInternal *pTime);

extern void Control_LedRun(Boolean val);

extern void SetFpgaTime(Integer32 sec);
extern void SetFpgaAdjustPhase(Integer32 nsec);


extern void SetFpgaAddressVal(short offset,Uint8 val);
extern Uint8 GetFpgaAddressVal(short offset);

//extern void CollectAlarm(UartDevice *pUartDevice);
#endif
