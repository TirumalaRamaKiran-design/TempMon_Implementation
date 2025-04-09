/*
 * ADC.c
 *
 *  Created on: 05-Apr-2025
 *      Author: LENOVO
 */


#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* DA structures, prototypes & variables */
unsigned eoc;
extern unsigned base;
int AD_Enabled = 0;
bool FilterDebounceDiagnosticsDone = false;
int CurrentChannel = 0;
unsigned int base = 0x2000;

/* Analog Channel definition */
struct anachan
{
  int data;
  int status;
} AnalogChannel[8];

/* Analog Channel states */
enum anastat
{
  INACTIVE,
  START_CONVERSION,
  DATA_READY
};

unsigned InitializeAnalog(void);
int TurnOnAnalog(int channel);
int TurnOffAnalog(int channel);
int GetChannelData(int channel)
{
	return AnalogChannel[channel].data;
}

void write_data(int x, int data)
{
    x = data;
}

int read_data(int x)
{
    return x;
}

/* find hardware port if one exists */
unsigned get_port(void)
{
  int x;
  static unsigned local_port;
  int portAddress;
  unsigned int base, not_ready_count,ready_count;

  if(local_port == 32767)
    return 0;

  for(x=0x200; x<0x3c0; x+=0x40)   {
      not_ready_count = 32767;
      ready_count = 32767;

      /* start conversion */
      write_data(x,0);
      while((read_data(x+0x18) & 0x80) && --not_ready_count); /* wait for not ready */
      while(!(read_data(x+0x18) & 0x80) && --ready_count); /* wait for ready */
      if(ready_count < 32767 && ready_count > 0)
    {
      local_port = base = x;
      portAddress = local_port + 0x18;

      return portAddress;
    }
  }
}

double get_frequency()
{
    return 200.0;
}

unsigned InitializeAnalog(void)
{
  base = get_port();
  eoc = base + 0x18;

  if(!base)
    return 0;

  memset(&AnalogChannel, 0, sizeof(AnalogChannel));

  CurrentChannel = 0;

  return base;
}

int TurnOnAnalog(int channel)
{
  if(channel < 0 || channel > 7)
    return -1;

  AnalogChannel[channel].status = START_CONVERSION;

  AD_Enabled = 1;

  return channel;
}

int TurnOffAnalog(int channel)
{
  int x;

  if(channel < 0 || channel > 7)
    return -1;

  AnalogChannel[channel].status = INACTIVE;

  return AnalogChannel[channel].data;
}

void new_timer(int timer_counter)
{
  int x;

  /* Is ADC channel enabled? */
  if(AD_Enabled)
  {
    /* look for start conversion or data ready status */
    while(AnalogChannel[CurrentChannel].status != INACTIVE
    		&& (timer_counter != 0))
    {
    	/* Fetch data from channel with DATA_READY status  */
		switch(AnalogChannel[CurrentChannel].status)
		{
		  case START_CONVERSION:
			/* will be ready at next interrupt */
			AnalogChannel[CurrentChannel].status = DATA_READY;
			break;

		  case DATA_READY:
			/* check eoc even though it's probably already ready */
			while(!(read_data(eoc) & 0x80));

			/* load data into structure */
			AnalogChannel[CurrentChannel].data = read_data(base);

			/* set up for next */
			AnalogChannel[CurrentChannel].status = START_CONVERSION;

			break;
		  case INACTIVE:
			break;
		} /* end switch(AnnalogChannel[CurrentChannel].status) */

		/* check the next channel for data */
    	CurrentChannel++;

    	/* decrement counter */
    	timer_counter--;
   } /* end while loop */
  } /* end if(AD_Enabled) */
}

void SampleData(void)
{
  int x;
  new_timer(100.0);

  x = (int)InitializeAnalog();

  printf("init ana = %X\n",x);

  x = TurnOnAnalog(CurrentChannel);
  printf("TurnOnAnalog(CurrentChannel); = %d\n",x);

  x = GetChannelData(CurrentChannel);
  printf("%4d",x);

  /* Perform filtering, debounce and Diagnostics on raw data */
  FilterDebounceDiagnosticsDone = true;
}


char* ADC_Raw()
{
  unsigned ADC_Chan0,dac1,eoc;
  int count;
  char data[300];

  ADC_Chan0 = get_port();

  if(ADC_Chan0 == 0)
  {
    printf("no hardware found\n");
    return 0;
  }

  SampleData();

  dac1 = ADC_Chan0 + 8;
  eoc = ADC_Chan0 + 0x18;

  printf("ADC Channel 0 after get_port = %X\n",ADC_Chan0);

  for(count=0; count<300; count++)
  {
	/* wait for ready */
	while(!(read_data(eoc) & 0x80))
	  data[count] = (char)read_data(dac1);
  }

    return data;
}

int ADC_Output()
{
	if(FilterDebounceDiagnosticsDone == true)
	{
		/* Conversion of raw signal to Temperature value */
		if (ADC_Raw() != '\0')
			return 0;
		else
			return ADC_Raw();
	}
}
