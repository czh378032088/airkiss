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

#include"airkisstask.h"
#include "gpio.h"


#define GetClockMs()   (clock() * 1000 / CLOCKS_PER_SEC)

pthread_t AirkissTask::threadID = -1;

AirkissTask::AirkissTask()
{
   this->runFlag = false;
   this->sendRandom = 0;
   this->sendDis = 0;
   this->sendTimes = 0;
}

AirkissTask* AirkissTask::RunAirkissTask(void)
{
   AirkissTask *pAirkissTask = new AirkissTask();
   int ret = pthread_create(&threadID,NULL,AirkissThread,pAirkissTask); 
   if (ret != 0)
   {
      delete pAirkissTask;
      return NULL;
   }
       
   return pAirkissTask;
}

void *AirkissTask::AirkissThread(void*arg)
{
   AirkissTask *pAirkissTask = (AirkissTask *)arg;
   pAirkissTask->RunTask();
   return 0;
}

void AirkissTask::EndThread(void)
{
    runEndFlag = false;
    runFlag = false;
}

void AirkissTask::WaitThreadEnd(void)
{
    while(!runEndFlag)
    {
        usleep(1000);
    }
}


void AirkissTask::RunTask(void)
{
   runFlag = true;
   airkiss_context_t akcontext;
   airkiss_config_t config;
   char recBuf[2048],sendBuff[2048]; 
   struct sockaddr_in adr_inet;  
   struct sockaddr_in adr_clnt; 

   config.memcpy = &memcpy;
   config.memset = &memset;
   config.memcmp = &memcmp;
   config.printf = &printf;

   airkiss_init(&akcontext, &config);
  
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);  
  if (sockfd == -1)  
  {  
      perror("socket 出错");  
      exit(1);  
  }  

  adr_inet.sin_family = AF_INET;  
  adr_inet.sin_port   = htons(12476);  
  adr_inet.sin_addr.s_addr = htonl(INADDR_ANY);  
  bzero(&(adr_inet.sin_zero),8);  
  int len = sizeof(adr_clnt);  

  int z = bind (sockfd, (struct sockaddr *) &adr_inet, sizeof (adr_inet));   
 
  if (z == -1)  
  {  
      perror("bind出错");  
      exit(1);  
  }    

  int sockfdSend = socket(AF_INET, SOCK_DGRAM, 0);
  char appid[] = "gh_1683dbbff1fc";
  char devid[] = "gh_1683dbbff1fc_4b58275c2b2572e8";//"gh_1683dbbff1fc_865663e3df14ab23";
  
  //SetUdpBroadcast(87);

   while(runFlag)
   {
      if(GetEnterConfigEvent())
      {
         printf("Run Config wifi\n");
         RunConfigWifi(&akcontext,&config);
      }
      int recLen = recvfrom(sockfd, recBuf, sizeof(recBuf), MSG_DONTWAIT, (struct sockaddr *)&adr_clnt, (socklen_t*)&len); 

      //printf("recLen = %d\n",recLen);

      if(recLen > 0)
      {
         recBuf[recLen] = 0;  
      //printf("接收:%s\n",buf);  
         printf("adr_clnt.sin_family=%x\n",adr_clnt.sin_family);
         printf("adr_clnt.sin_port=%x\n",adr_clnt.sin_port);
         printf("adr_clnt.sin_addr.s_addr=%x\n",adr_clnt.sin_addr.s_addr);

         int ret = airkiss_lan_recv(recBuf, recLen, &config);
         if(ret == AIRKISS_LAN_SSDP_REQ)
         {
            unsigned short  sendSize = 2048;
            ret = airkiss_lan_pack(AIRKISS_LAN_SSDP_RESP_CMD, appid,devid,0, 0, sendBuff, &sendSize, &config);
            if(ret == AIRKISS_LAN_PAKE_READY)
            {
               printf("sendSize = %d\n",sendSize);
               sendto (sockfdSend, sendBuff, sendSize, 0, (struct sockaddr *)&adr_clnt, sizeof(adr_clnt)); 
            }
            else
            {
               printf("ret = %d\n",ret);
            }
         }
      } 
      UdpBroadcast();
   }
   runEndFlag = true;
   close(sockfd);
   usleep(10000);
}

bool AirkissTask::GetEnterConfigEvent(void)
{
   static clock_t nowTime = 0;
   static int downCount = 0;   
   if(GetClockMs() - nowTime > 100)
   {
      GpioCtrl gpio(38);
      if(gpio.GetGpio() == 0)
         downCount ++;
      else 
         downCount = 0;
      if(downCount >= 20)
      {
          downCount = 0;
          return true;
      }  
      nowTime = GetClockMs();
   }
   return false;
}

void AirkissTask::RunConfigWifi(airkiss_context_t* context, const airkiss_config_t* config)
{
   struct ifreq ifr;
   struct sockaddr_ll sll;  
   char data[2048];
   airkiss_result_t ak_result;

   system("ifconfig wlan0 down");
   usleep(1000);
   system("iwconfig wlan0 mode monitor");
   usleep(1000);
   system("ifconfig wlan0 up");
       
   int socket_id = socket(PF_PACKET, SOCK_RAW, htons (ETH_P_ALL));
   if(socket_id < 0)
   {
	    printf("error::Open socket error!\n");
		return ;
   }
  /// printf("1\n");
   int caplen = 65535;
   setsockopt(socket_id,SOL_SOCKET,SO_RCVBUF, &caplen, sizeof(caplen));
    
  // printf("2\n");
   strcpy (ifr.ifr_name, "wlan0");  
   ioctl(socket_id, SIOCGIFINDEX, &ifr) ;
        
   // printf("3\n");
   
   memset( &sll, 0, sizeof(sll) );  
   sll.sll_family = AF_PACKET;  
   sll.sll_ifindex = ifr.ifr_ifindex;  
   sll.sll_protocol = htons(ETH_P_ALL); 
   bind(socket_id, (struct sockaddr *)&sll, sizeof(sll));

   usleep(1000);
  // printf("4\n");

   clock_t nowTime = GetClockMs();
   ChangeChannel(context,config);
    //printf("5\n");
   int lock = 0;
   while(runFlag)
   {
    if(GetClockMs() - nowTime >= 200 && lock == 0)
    {
        nowTime = GetClockMs(); 
        ChangeChannel(context,config);
    }
    //printf("6\n");
	int ret = recv(socket_id, data, 2048, MSG_DONTWAIT);
	if(ret <= 64)
	{
        //printf("nodata\n");
	     continue;
	}
	else
	{
       printf("len = %d: ",ret);
	  /* for(int i = 0;i<ret;i++)
	    {
		printf("%02X ",(unsigned char)data[i]);
	    }*/
	    printf("\n");

	   ret = airkiss_recv(context,data + 30,ret - 30);
	   if(ret == AIRKISS_STATUS_CHANNEL_LOCKED)
	   {
	      lock = 1;		  
	      printf("channel locked\n");
	   }
	   else if(ret == AIRKISS_STATUS_COMPLETE)
	   {
	      if(airkiss_get_result(context , &ak_result) < 0)
	      {
	          printf("airkiss get result fail\n");
		  break;
	      }
	      else
	      {
	          printf("result ok!ssid is %s , key is %s\n" , ak_result.ssid , ak_result.pwd);
		  break;
	      }
	   }
	 }
   }
    close(socket_id);

	system("ifconfig wlan0 down");
    printf("ifconfig wlan0 down\n");
	usleep(1000);
	system("iwconfig wlan0 mode managed");
    printf("iwconfig wlan0 mode managed\n");
	usleep(1000);
    if(runFlag) 
        WriteConfigfile(ak_result.ssid,ak_result.pwd); 
        
	system("ifconfig wlan0 up");
    printf("ifconfig wlan0 up\n");
    usleep(1000);
        
    system("wifi reload");   
    usleep(1000);     

   if(runFlag) 
   {
      SetUdpBroadcast(ak_result.random);
      UdpBroadcast();
   }
  
  
}

int AirkissTask::WriteConfigfile(const char *ssid,const char *pwr)
{
   FILE *fp = fopen("/etc/config/wireless","w");
   fputs("config wifi-device \'radio0\'\n\r",fp);
   fputs("\toption type \'mac80211\'\n\r",fp);
   fputs("\toption hwmode \'11g\'\n\r",fp);
   fputs("\toption path \'platform/10300000.wmac\'\n\r",fp);
   fputs("\toption htmode \'HT20\'\n\r",fp);
   fputs("\toption disabled \'0\'\n\r",fp);
   fputs("\toption channel \'auto\'\n\r",fp);
   fputs("\toption country \'00\'\n\r",fp);
   fputs("\n\r",fp);
   fputs("config wifi-iface\n\r",fp);
   fputs("\toption network \'wwan\'\n\r",fp);
   fprintf(fp,"\toption ssid \'%s\'\n\r",ssid);
   fputs("\toption device \'radio0\'\n\r",fp);
   fputs("\toption mode \'sta\'\n\r",fp);
   fputs("\toption encryption \'psk-mixed\'\n\r",fp);
   fprintf(fp,"\toption key \'%s\'\n\r",pwr);
   fclose(fp);
/*
   fp = fopen("/etc/wpa_supplicant.conf","w");
   fputs("ctrl_interface=/var/run/wpa_supplicant\n\r",fp);
   fputs("update_config=1\n\r",fp);

   fprintf(fp,"network={\nssid=\"%s\"\nkey_mgmt=WPA-PSK\nproto=WPA\npairwise=TKIP\ngroup=TKIP\npsk=\"%s\"\npriority=1\n}",ssid,pwr);
   fclose(fp);
*/
   return 0;
}

 void  AirkissTask::ChangeChannel(airkiss_context_t* context, const airkiss_config_t* config)
 {
    static int cur_channel = 0;
    char cmd[128] = "\0";

    if(cur_channel >= 13)
	    cur_channel = 1;
	else
	    cur_channel ++;
	snprintf(cmd,128,"iwconfig wlan0 channel %d\n",cur_channel);
    printf(cmd);
	system(cmd);
	airkiss_change_channel(context);
 }

void AirkissTask::SetUdpBroadcast(unsigned char random,int dis,int times)
{
   this->sendRandom = random;
   this->sendDis = dis;
   this->sendTimes = times;
}

void AirkissTask::UdpBroadcast(void)
{
    int fd = -1;
    static clock_t nowTime = 0;

    int enabled = 1;
    int err;
    struct sockaddr_in addr;
    
    if(this->sendTimes <= 0)
       return;
    
    if(GetClockMs() - nowTime > this->sendDis)
    {
     if(fd == -1)
     {
       addr.sin_family = AF_INET;
       addr.sin_addr.s_addr = INADDR_BROADCAST;
       addr.sin_port = htons(10000);
    
       fd = socket(AF_INET, SOCK_DGRAM, 0);
       if (fd < 0)
       {
           printf("get socket err\n");
           return;
       } 
    
       err = setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char *) &enabled, sizeof(enabled));
       if(err == -1)
       {
           printf("setsockopt err\n");
           close(fd);
           fd = -1;
           return;
       }
    
       printf("Sending random to broadcast..\n");
    }

       sendto(fd, (unsigned char *)&this->sendRandom, 1, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr));
       printf("fd = %x,send sendRandom %d ,%d\n",fd,this->sendRandom,this->sendTimes);
       this->sendTimes --;
       if(this->sendTimes <= 0)
       {
         // close(fd);
         // fd = -1;
       }
       nowTime = GetClockMs();
    }    

    close(fd);
/*
    if(!runFlag && fd != -1)
    {
        close(fd);
        fd = -1;
    }
       */
      
       
}
