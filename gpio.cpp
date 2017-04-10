
#include "gpio.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

GpioCtrl::GpioCtrl(int index)
{
   this->gpioIndx = index;
}

int GpioCtrl::GetGpioDirection(void)
{
   if(GetAndCreatFile())
      return -1;
    char buff[64];
    sprintf(buff,"/sys/class/gpio/gpio%d/direction",gpioIndx);
    FILE* fp = fopen(buff,"r");
    if(fp == NULL)
       return -1;
    if(fgets(buff, 5, fp) == NULL)
    {
        fclose(fp);
        return -2;
    }
    if(strcmp(buff,"in") == 0)
       return DIR_IN;
    else if(strcmp(buff,"out") == 0)
       return DIR_OUT;
    else 
       return -3;

    fclose(fp);
}

int GpioCtrl::SetGpioDirection(int dir)
{
   if(GetAndCreatFile())
      return -1;
    char buff[64];
    sprintf(buff,"/sys/class/gpio/gpio%d/direction",gpioIndx);
    FILE* fp = fopen(buff,"w");
    if(fp == NULL)
       return -1;
    int ret;
    if(dir == DIR_IN)
       ret = fputs("in",fp);
    else if(dir == DIR_OUT)
       ret = fputs("out",fp);
    else 
       ret = -1;
    if(ret < 0)
    {
        fclose(fp);
        return ret;
    }
    return fclose(fp);
}

int GpioCtrl::SetGpio(int state)
{
   if(GetAndCreatFile())
      return -1;
       char buff[64];
    sprintf(buff,"/sys/class/gpio/gpio%d/value",gpioIndx);
    FILE* fp = fopen(buff,"w");
    if(fp == NULL)
       return -1;

    if(fprintf(fp,"%d",state) < 0)
    {
        fclose(fp);
        return -1;
    }
    return fclose(fp);
}

int GpioCtrl::GetGpio(void)
{
   if(GetAndCreatFile())
      return -1;
      char buff[64];
    sprintf(buff,"/sys/class/gpio/gpio%d/value",gpioIndx);
    FILE* fp = fopen(buff,"r");
    if(fp == NULL)
       return -1;
    if(fgets(buff, 2, fp) == NULL)
    {
        fclose(fp);
        return -2;
    }

    fclose(fp);
    return (buff[0] - '0');
}

int GpioCtrl::GetAndCreatFile(void)
{
    char buff[64];
    sprintf(buff,"/sys/class/gpio/gpio%d",gpioIndx);
    if(access(buff,F_OK) == 0)
       return 0;
    FILE* fp = fopen("/sys/class/gpio/export","w");
    if(fp == NULL)
       return -1;
    if(fprintf(fp,"%d",gpioIndx) < 0)
    {
        fclose(fp);
        return -2;
    }   
    return fclose(fp);
}

