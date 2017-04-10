
#ifndef __AIRKISSTASK_H
#define __AIRKISSTASK_H

#include <pthread.h>
#include "airkiss.h"

class AirkissTask
{
public:
   AirkissTask();
   static AirkissTask *RunAirkissTask(void);
   static void *AirkissThread(void*arg);
   void EndThread(void);
   void WaitThreadEnd(void);

protected:
   void RunTask(void);
   bool GetEnterConfigEvent(void);
   void RunConfigWifi(airkiss_context_t* context, const airkiss_config_t* config);
   int  WriteConfigfile(const char *ssid,const char *pwr);
   void ChangeChannel(airkiss_context_t* context, const airkiss_config_t* config);
   void SetUdpBroadcast(unsigned char random,int dis = 100,int times = 600);
   void UdpBroadcast(void);
private:
   static pthread_t threadID;
   volatile bool runFlag;
   volatile bool runEndFlag;
   unsigned char sendRandom;
   int sendDis,sendTimes;
};


#endif
