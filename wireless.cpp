
#include "wireless.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <sys/ioctl.h> 
#include <linux/if_packet.h>
#include <unistd.h>
#include <linux/wireless.h>

WirelessCtl::WirelessCtl(const char *infName)
{
  sockfd = -1;
  strcpy(interfaceName,infName);
}

int WirelessCtl::Open()
{
  struct ifreq ifr;
  int caplen = 65535;  
  if ((sockfd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0)
  {
     return -1;
  }
  if(setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF, &caplen, sizeof(caplen)) < 0)
  {
     return -2;
  } 
  strcpy(ifr.ifr_name,interfaceName);
  if(ioctl(sockfd, SIOCGIFINDEX, &ifr) != 0)
  {
    return -3;
  } 

  sockaddr_ll sll;  
  memset( &sll, 0, sizeof(sll) );  
  sll.sll_family = AF_PACKET;  
  sll.sll_ifindex = ifr.ifr_ifindex;  
  sll.sll_protocol = htons(ETH_P_ALL);  
     
  if(bind(sockfd, (struct sockaddr *)&sll, sizeof(sll)) != 0)
    return -4;
  return 0;  
}

int WirelessCtl::Close()
{
   int ret = 0;
   if(sockfd != -1)
     ret = close(sockfd);

   sockfd = -1;
   return ret;
}

int WirelessCtl::SetPromisc(int flag)
{
   struct ifreq ifr;

   strcpy (ifr.ifr_name, interfaceName);
   if (ioctl (sockfd, SIOCGIFFLAGS, &ifr) != 0)
   {
     return -1;
   }
   
   if(flag)
      ifr.ifr_flags |= IFF_PROMISC;
   else 
      ifr.ifr_flags &= (~IFF_PROMISC); 

   if (ioctl (sockfd, SIOCSIFFLAGS, &ifr) != 0)
   {
      return -2;
   }
   return 0;
}

int WirelessCtl::GetRangeInfo(struct iw_range *range)
{
  struct iwreq		wrq;
  char			buffer[sizeof(struct iw_range) * 2];
  union iw_range_raw *	range_raw;

  bzero(buffer, sizeof(buffer));
  
  wrq.u.data.pointer = (caddr_t) buffer;
  wrq.u.data.length = sizeof(buffer);
  wrq.u.data.flags = 0;
  strcpy(wrq.ifr_name, interfaceName);

  if(ioctl(sockfd, SIOCGIWRANGE, &wrq) < 0)
    return -1;

  memcpy((char *) range, buffer, sizeof(struct iw_range));

  return(0);
}

int WirelessCtl::SetFreq(struct iw_freq *freq,int fixed)
{
  struct iwreq		wrq;
  wrq.u.freq = *freq;

  strcpy(wrq.ifr_name, interfaceName);
  if(fixed)  
     wrq.u.freq.flags = IW_FREQ_FIXED;
  else
     wrq.u.freq.flags = 0;
 
  if(ioctl(sockfd, SIOCSIWFREQ, &wrq) < 0)
    return -1;

  return 0;
}

int WirelessCtl::GetFreq(struct iw_freq *freq)
{ 
  struct iwreq		wrq;

  strcpy(wrq.ifr_name, interfaceName);
 
  if(ioctl(sockfd, SIOCGIWFREQ, &wrq) < 0)
    return -1;

  *freq = wrq.u.freq;
  return 0;
}

int WirelessCtl::GetData(void *buff,int size)
{
  return recvfrom(sockfd, buff, size, MSG_DONTWAIT, NULL, NULL);
}

int WirelessCtl::SetMode(int mode)
{
  struct iwreq		wrq;
  strcpy(wrq.ifr_name, interfaceName);
  
  wrq.u.mode = mode;
  if(ioctl(sockfd, SIOCSIWMODE, &wrq) < 0)
    return -1;
  return 0;
}

int WirelessCtl::GetMode()
{
  struct iwreq		wrq;

  strcpy(wrq.ifr_name, interfaceName);
 
  if(ioctl(sockfd, SIOCGIWMODE, &wrq) < 0)
    return -1;

  return wrq.u.mode;
}


