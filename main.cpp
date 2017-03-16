#include <time.h>
#include <stdio.h>
#include "airkiss.h"
#include <stdlib.h>
#include <string.h>
#include "wireless.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <sys/ioctl.h> 
#include <linux/if_packet.h>
#include <linux/wireless.h>

#define ASSKEY  "123456"

airkiss_context_t airkiss_context;

const airkiss_config_t akconf = 
{ 	
(airkiss_memset_fn)&memset, 	
(airkiss_memcpy_fn)&memcpy, 	
(airkiss_memcmp_fn)&memcmp, 	
(airkiss_printf_fn)&printf 
};

WirelessCtl wirelessCtrl("wlp3s0");


int main(int argc, char *argv[])
{
  struct iw_range range;
  int ret;
  unsigned char buffer[1518] = {0};

  ret = airkiss_init(&airkiss_context,&akconf);
  if(ret < 0)
  {
     printf("airkiss_init failed:%d \n",ret);
     return ret;
  }
 
  printf(airkiss_version());
  printf("\n");

  ret = airkiss_set_key(&airkiss_context, (const unsigned char *)ASSKEY, strlen(ASSKEY));
  if(ret < 0)
  {
     printf("airkiss_set_key failed:%d \n",ret);
     return ret;
  }

  ret = wirelessCtrl.Open();
  if(ret != 0)
  {
     printf("wireless open failed:%d \n",ret);
     return ret;
  }
  ret = wirelessCtrl.SetPromisc(1);
  if(ret != 0)
  {
     printf("wireless set promisc failed:%d \n",ret);
     return ret;
  }


  ret = wirelessCtrl.GetRangeInfo(&range);
  if(ret != 0)
  {
     printf("wireless get range info failed:%d \n",ret);
     return ret;
  }

  printf("%d channels in total; available frequencies :\n",range.num_channels);
  for(int i = 0 ; i < range.num_frequency ; i ++)
  {
     printf("m=%d,e=%d,i=%d,flags=%d\n",range.freq[i].m,range.freq[i].e,range.freq[i].i,range.freq[i].flags);
  }
  
  struct iw_freq freq;

  ret = wirelessCtrl.GetFreq(&freq); 
  if(ret != 0)
  {
     printf("wireless get freq failed:%d \n",ret);
     return ret;
  }  
  printf("now freq:m=%d,e=%d,i=%d,flags=%d\n",freq.m,freq.e,freq.i,freq.flags);

  ret = wirelessCtrl.SetMode(AD_HOC_MODE);
  if(ret != 0)
  {
     printf("wireless set mode AD_HOC_MODE failed:%d \n",ret);
     return ret;
  } 
  

  int channelIndex = 8;
  clock_t nowTime = clock();
  bool lockFlag = false;

  ret = wirelessCtrl.SetFreq(range.freq + channelIndex,1);
  if(ret != 0)
  {
     printf("wireless set freq failed:%d \n",ret);
     return ret;
  } 
  


  while(1)
  {
  /*  if(!lockFlag && clock() - nowTime >= 100)
    {
      channelIndex ++;
      if(channelIndex >= 11)//range.num_frequency)
      {
        channelIndex = 0;
      }
        ret = wirelessCtrl.SetFreq(range.freq + channelIndex,1);
       if(ret != 0)
       {
          printf("wireless set freq failed:%d \n",ret);
          return ret;
       } 
      airkiss_change_channel(&airkiss_context)  ;
      printf("channelIndex = %d\n",channelIndex);
    }*/
    int length = wirelessCtrl.GetData(buffer,sizeof(buffer));
    if(length <= 0)
       continue;
    printf ("recview package length : %d\n", length);
    ret = airkiss_recv(&airkiss_context,buffer,length);
    if(ret == AIRKISS_STATUS_CHANNEL_LOCKED)
    {
      printf("lockFlag = true");
      lockFlag = true;
    }
    else if(ret == AIRKISS_STATUS_COMPLETE)
    {
      airkiss_result_t result;
      ret = airkiss_get_result(&airkiss_context,&result);
      if(ret == 0)
      {
        printf("SSID=%s,PWD=%s",result.ssid,result.pwd);
      }
      break;
    }
    else 
    {
      lockFlag = false;
    }
  }




  
  ret = wirelessCtrl.GetFreq(&freq); 
  if(ret != 0)
  {
     printf("wireless get freq failed:%d \n",ret);
     return ret;
  }  
  printf("now freq:m=%d,e=%d,i=%d,flags=%d\n",freq.m,freq.e,freq.i,freq.flags);

  ret = wirelessCtrl.SetMode(MANAGED_MODE);
  if(ret != 0)
  {
     printf("wireless set mode AD_HOC_MODE failed:%d \n",ret);
     return ret;
  } 

  wirelessCtrl.Close();

#if 0

int ret = 0;
unsigned char buffer[1518] = {0};
unsigned char *eth_head = NULL;

if ((sockfd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0)
{
   printf ("create socket failed\n");
   return -1;
}

if (0 != set_promisc ("wlp3s0", sockfd))
{
   printf ("Failed to set interface promisc mode\n");
}

while (1)
{
   memset (buffer, 0x0, sizeof (buffer));
   ret = recvfrom (sockfd, buffer, sizeof (buffer), 0, NULL, NULL);
   printf ("recview package length : %d\n", ret);

   eth_head = buffer;
   printf ("PACKAGE START\n");
/* get source and dectination mac address */
   printf ("dectination mac:%2x-%2x-%2x-%2x-%2x-%2x,"
"source mac:%2x-%2x-%2x-%2x-%2x-%2x;\n", eth_head[0],
   eth_head[1], eth_head[2], eth_head[3], eth_head[4],
   eth_head[5], eth_head[6], eth_head[7], eth_head[8],
   eth_head[9], eth_head[10], eth_head[11]);
   printf ("eth_type:%02x%02x\n", eth_head[12], eth_head[13]);

/* ARP proptocol flag */
   if (0x08 == eth_head[12] && 0x06 == eth_head[13])
   {
     printf ("ARP source ip:%d.%d.%d.%d,destination ip:%d.%d.%d.%d;\n",
     eth_head[28], eth_head[29], eth_head[30], eth_head[31],
     eth_head[38], eth_head[39], eth_head[40], eth_head[41]);
   }
/* IPv4 proptocol flag */
   else if (0x08 == eth_head[12] && 0x00 == eth_head[13])
   {
     if (0x45 == eth_head[14])
     {
       printf ("IPv4 source ip:%d.%d.%d.%d,destination ip:%d.%d.%d."
      "%d;\n", eth_head[26], eth_head[27], eth_head[28],
       eth_head[29], eth_head[30], eth_head[31],
       eth_head[32], eth_head[33]);
     }
     else
     {
       printf ("p_head:%02x\n", eth_head[14]);
     }
   }
   printf ("PACKAGE END\n");
}
#endif
return 0;
}


