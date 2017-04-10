
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

#include "airkisstask.h"
#include "airkiss.h"
#include "device.h"


int main(int argc, char* argv[])
{

    AirkissTask* airkissTsk = AirkissTask::RunAirkissTask();
    DeviceCtrl *deviceCtrl = DeviceCtrl::RunDeviceTask();
    while(1)
    {
        char ch = getchar();
        if(ch == 'q')
        {  
            deviceCtrl->EndThread();
            airkissTsk->EndThread();
            deviceCtrl->WaitThreadEnd();
            airkissTsk->WaitThreadEnd();
            delete airkissTsk;
            delete deviceCtrl;
            break;
        }
    }
	return 0;
}
