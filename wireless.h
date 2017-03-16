
#ifndef WIRELESS_H_
#define WIRELESS_H_

#define AUTO_MODE      0
#define AD_HOC_MODE    1
#define MANAGED_MODE   2
#define MASTER_MODE    3
#define REPEATER_MODE  4
#define SECONDARY_MODE 5
#define MONITOR_MODE   6

class WirelessCtl
{
public:
  WirelessCtl(const char *infName);
  int Open();
  int Close();
  int SetPromisc(int flag);
  int GetRangeInfo(struct iw_range *range);
  int SetFreq(struct iw_freq *freq,int fixed);
  int GetFreq(struct iw_freq *freq);
  int GetData(void *buff,int size);
  int SetMode(int mode);
  int GetMode(void);
private:
  char interfaceName[32];
  int sockfd;
};

#endif

