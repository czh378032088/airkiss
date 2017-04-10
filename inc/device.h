
#ifndef __DEVICE_H
#define __DEVICE_H

#include <pthread.h>

enum DeviceState
{
    DeviceStateLogOut,
    DeviceStateLanding,
    DeviceStateLanded
};

class DeviceCtrl
{
public:
   DeviceCtrl();
   static DeviceCtrl *RunDeviceTask(void);
   static void *DeviceThread(void*arg);
   void EndThread(void);
   void WaitThreadEnd(void);

protected:
   void RunTask(void);
private:
   static pthread_t threadID;
   char *devid;
   volatile bool runFlag;
   volatile bool runEndFlag;
   DeviceState deviceState;
   int ledstate;
};

#endif

