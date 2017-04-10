

#ifndef __GPIO_H
#define __GPIO_H

#define DIR_OUT 0
#define DIR_IN  1

class GpioCtrl
{
public:
   GpioCtrl(int index);
   int GetGpioDirection(void);
   int SetGpioDirection(int dir);
   int SetGpio(int state);
   int GetGpio(void);
protected:
   int GetAndCreatFile(void);
private:
   int gpioIndx;
};

#endif
