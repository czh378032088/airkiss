
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

#include <unistd.h>


#include "airkiss.h"

#if WIRELESS_EXT <= 11
#ifndef SIOCDEVPRIVATE
#define SIOCDEVPRIVATE      0x8BE0
#endif
#define SIOCIWFIRSTPRIV      SIOCDEVPRIVATE
#endif
#define RTPRIV_IOCTL_GET_80211_DATA	(SIOCIWFIRSTPRIV + 0x1D)
#define RTPRIV_IOCTL_SHOW_CONNSTATUS	(SIOCIWFIRSTPRIV + 0x1B)

#define RX_80211_PKT_SIZE   (1500)
#define RX_BUF_SIZE         (3000)


typedef struct _RX_PKT
{
	unsigned int  length;
	unsigned char  data[RX_80211_PKT_SIZE];
}RX_PKT;


airkiss_context_t akcontext;
uint8_t cur_channel = 1;
airkiss_config_t config;
airkiss_result_t ak_result;

int write_configfile(unsigned char *ssid,unsigned char *pwr)
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

int udp_broadcast(unsigned char random, int port)
{
    int fd;
    int enabled = 1;
    int err;
    struct sockaddr_in addr;
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_BROADCAST;
    addr.sin_port = htons(port);
    
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        printf("get socket err\n");
        return 1;
    } 
    
    err = setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char *) &enabled, sizeof(enabled));
    if(err == -1)
    {
        close(fd);
        return 1;
    }
    
    printf("Sending random to broadcast..\n");
    int i;
    unsigned int usecs = 1000*100;
    for(i=0;i<50;i++)
    {
        sendto(fd, (unsigned char *)&random, 1, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr));
        usleep(usecs);
    }

    close(fd);
}

int set_channel(int channel)
{
        char cmd[128] = "\0";
	snprintf(cmd,128,"iwconfig wlan0 channel %d\n",channel);
        printf(cmd);
	system(cmd);
	return 0;
}

void time_callback(int signo)
{
        if(cur_channel >= 13)
	    cur_channel = 1;
	else
	    cur_channel ++;
	set_channel(cur_channel);
	airkiss_change_channel(&akcontext);
}

static void exit_airkiss(int sig)
{
	int ret;
	char data[64];
	
	system("ifconfig wlan0 down");
	usleep(1000);
	system("iwconfig wlan0 mode managed");
	usleep(1000);
	system("ifconfig wlan0 up");

	exit(1);
}

static void signal_handle(void)
{
    signal(SIGPIPE, &exit_airkiss);//pipe broken
    signal(SIGINT,  &exit_airkiss);//ctrl+c
    signal(SIGTERM, &exit_airkiss);//kill
    signal(SIGSEGV, &exit_airkiss);//segmentfault
    signal(SIGBUS,  &exit_airkiss);//bus error/**/
}

int main(int argc, char* argv[])
{
	int length;
	RX_PKT * mp;
	int ret= -1;
	int i;
        int socket_id;
        struct iwreq wrq;
	char data[RX_BUF_SIZE];
	struct itimerval tick;

	signal_handle();

	config.memcpy = memcpy;
	config.memset = memset;
	config.memcmp = memcmp;
	config.printf = printf;

	printf("version=%s\n",airkiss_version());
	usleep(100000);
	if(airkiss_init(&akcontext, &config))
	{
		printf("airkiss init fail\n");
		return -1;
	}

	printf("start monitor mode\n");

	system("ifconfig wlan0 down");
	usleep(1000);
	system("iwconfig wlan0 mode monitor");
	usleep(1000);
	system("ifconfig wlan0 up");
       
       	socket_id = socket(PF_PACKET, SOCK_RAW, htons (ETH_P_ALL));
	if(socket_id < 0)
	{
	        printf("error::Open socket error!\n");
		return -1;
	}
    int caplen = 65535;
    setsockopt(socket_id,SOL_SOCKET,SO_RCVBUF, &caplen, sizeof(caplen));
    
	struct ifreq ifr;
        strcpy (ifr.ifr_name, "wlan0");  
	ioctl(socket_id, SIOCGIFINDEX, &ifr) ;
        

       struct sockaddr_ll sll;  
       memset( &sll, 0, sizeof(sll) );  
       sll.sll_family = AF_PACKET;  
       sll.sll_ifindex = ifr.ifr_ifindex;  
        sll.sll_protocol = htons(ETH_P_ALL); 
	bind(socket_id, (struct sockaddr *)&sll, sizeof(sll));

       usleep(1000);


       clock_t nowTime = clock();
       time_callback(0);
   
    int lock = 0;

    while(1)
    {
        if(clock() - nowTime >= CLOCKS_PER_SEC / 5 && lock == 0)
       {
            nowTime = clock(); 
            time_callback(0);
       }

	ret = recv(socket_id, data, RX_BUF_SIZE, MSG_DONTWAIT);
	if(ret <= 64)
	{
             //printf("nodata\n");
	     continue;
	}
	else
	{
          /* printf("len = %d: ",ret);
	   for(i=0;i<ret;i++)
	    {
		printf("%02X ",(unsigned char)data[i]);
	    }
	    printf("\n");*/

	   ret = airkiss_recv(&akcontext,data + 30,ret - 30);
	   if(ret == AIRKISS_STATUS_CHANNEL_LOCKED)
	   {
	      lock = 1;		  
	      printf("channel locked\n");
	   }
	   else if(ret == AIRKISS_STATUS_COMPLETE)
	   {
	      if(airkiss_get_result(&akcontext , &ak_result) < 0)
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
	usleep(1000);
	system("iwconfig wlan0 mode managed");
	usleep(1000);
        
        write_configfile(ak_result.ssid,ak_result.pwd); 
        
	system("ifconfig wlan0 up");
        usleep(100000);
        
        system("wifi reload");   
        usleep(1000000);     

    udp_broadcast(ak_result.random, 10000);
	
	return 0;
}
