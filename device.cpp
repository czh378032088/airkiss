
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include "pthread.h"
#include <sys/socket.h>
#include <linux/wireless.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <sys/socket.h>
#include <linux/if_packet.h>

#include "gpio.h"
#include "device.h"
#include "json.h"

#define GetClockMs()   (clock() * 1000 / CLOCKS_PER_SEC)

pthread_t DeviceCtrl::threadID = -1;
char constDevid[] = "gh_1683dbbff1fc_4b58275c2b2572e8";

DeviceCtrl::DeviceCtrl()
{
   GpioCtrl gpio(11);
   devid = constDevid;
   deviceState = DeviceStateLogOut;
   ledstate = 0;
   gpio.SetGpioDirection(DIR_OUT);
}

DeviceCtrl *DeviceCtrl::RunDeviceTask(void)
{
   DeviceCtrl *pDeviceCtrl = new DeviceCtrl();
   int ret = pthread_create(&threadID,NULL,DeviceThread,pDeviceCtrl); 
   if (ret != 0)
   {
      delete pDeviceCtrl;
      return NULL;
   }
       
   return pDeviceCtrl;
}

void *DeviceCtrl::DeviceThread(void*arg)
{
    DeviceCtrl *pDeviceCtrl = (DeviceCtrl *) arg;
    pDeviceCtrl->RunTask();
    return 0;
}

void DeviceCtrl::EndThread(void)
{
   runEndFlag = false;
   runFlag = false;
}

void DeviceCtrl::WaitThreadEnd(void)
{
    while(!runEndFlag)
    {
        usleep(1000);
    }
}

void DeviceCtrl::RunTask(void)
{
    int fd = -1;
    struct sockaddr_in adr_inet;
    char sendBuff[2048];
    char recBuff[2048];
    int  sendLenth = 0; 
    int  recLenth = 0;
    clock_t lastSendClock = 0;
    
    fd = socket(AF_INET, SOCK_DGRAM, 0);  
    if(fd < 0)
    {
      perror("socket 出错\n");  
      exit(1);  
    }

    bzero(&adr_inet,sizeof(adr_inet));

    adr_inet.sin_family = AF_INET;  
    adr_inet.sin_port   = htons(20000);  
    adr_inet.sin_addr.s_addr = inet_addr("192.168.11.198");  

    int ret = connect(fd,(struct sockaddr*)&adr_inet,sizeof(adr_inet));
    if(ret != 0)
    {
        printf("connect error\n");
    }
    
    runFlag = true;
    while(runFlag)
    {
       sendLenth = 0;
       recLenth = recv(fd,recBuff ,2048,MSG_DONTWAIT);
       ///if(recLenth > 0)
       //    printf("recv:%s\n",recBuff);
       switch(deviceState)
       {
          case DeviceStateLogOut:
            {
                JsonNode mainNode;
                mainNode.AddStringNode("deviceId",devid,5);
                mainNode.AddStringNode("msg_type","Login",5);
                sendLenth = mainNode.ToString(sendBuff,2048);
                deviceState = DeviceStateLanding;
            }
            break;
          case DeviceStateLanding:
            {
                if(recLenth > 0)
                {
                    JsonNode mainNode,*pNode;
                    printf("Login\n");
                    mainNode.LoadFromString(recBuff);
                    pNode = mainNode.FindChildNodeName("return_code");
                    if(pNode == NULL)
                    {
                        break;
                    }
                    long long temp ;
                    int ret = pNode->GetIntValue(&temp);
                    if(ret == 0 && temp == 0)
                       deviceState = DeviceStateLanded;
                }
                else if(GetClockMs() - lastSendClock >= 1000)
                {
                    deviceState = DeviceStateLogOut;
                }
            }
            break;
          case DeviceStateLanded:
            {
              if(GetClockMs() - lastSendClock >= 2000)
              {
                JsonNode mainNode;
                mainNode.AddStringNode("deviceId",devid);
                mainNode.AddStringNode("msg_type","Notify",5);
                if(ledstate)
                   mainNode.AddStringNode("led_state","ON",5);
                else 
                   mainNode.AddStringNode("led_state","OFF",5);
                sendLenth = mainNode.ToString(sendBuff,2048);
              }
              if(recLenth > 0)
              {
                  JsonNode mainNode,*pNode;
                  char buff[16];
                  mainNode.LoadFromString(recBuff);
                  pNode = mainNode.FindChildNodeName("msg_type");
                  if(pNode == NULL)
                  {
                      break;
                  }
                  pNode->GetStringValue(buff);
                  if(strcmp(buff,"Get") == 0)
                  {
                     if(ledstate)
                         mainNode.AddStringNode("led_state","ON",5);
                     else 
                         mainNode.AddStringNode("led_state","OFF",5);
                     sendLenth = mainNode.ToString(sendBuff,2048);    
                  }
                  else if(strcmp(buff,"Set") == 0)
                  {
                     pNode = mainNode.FindChildNodeName("led_state");
                     if(pNode == NULL)
                     {
                        break;
                     }
                      pNode->GetStringValue(buff);
                       
                      
                      if(strcmp(buff,"ON") == 0)
                      {
                         ledstate = 1;
                      }
                      else if(strcmp(buff,"OFF") == 0)
                      {
                         ledstate = 0;
                      }
                      GpioCtrl gpio(11);
                      gpio.SetGpio(ledstate);
                      mainNode.AddIntNode("return_code",0);
                      sendLenth = mainNode.ToString(sendBuff,2048);
                  }
              }
            }
            break;
          default:
            break;
       }
       if(sendLenth > 0)
       {
           send(fd,sendBuff ,sendLenth,MSG_DONTWAIT);
           //printf("send:%s\n",sendBuff);
           lastSendClock = GetClockMs();
       }
       usleep(10000);    
    }
    JsonNode mainNode;
    mainNode.AddStringNode("deviceId",devid,5);
    mainNode.AddStringNode("msg_type","Logout",5);
    sendLenth = mainNode.ToString(sendBuff,2048);
    send(fd,sendBuff ,sendLenth,0);

    runEndFlag = true;
}
