/*
 ============================================================================
 Name        : TempMon.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Temperature Monitoring and Display
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "TempMon.h"

#define RED_LED_PIN    0x1000
#define YELLOW_LED_PIN 0x2000
#define GREEN_LED_PIN  0x3000

#define HARDWARE_SERIAL_NUMBER 0xABC1234

#define HIGH 1
#define LOW  0

enum {REV_A = 0,
	  REV_B,
	  NONE
     }hardware_revision;

void GPIO_Write(int PIN, int status)
{
	/* Write data into GPIO port PIN */
	PIN = &status;
}

void GPIO_Init(int PIN)
{
	/* Initialize 0 to the address PIN */
	int data = 0;
	PIN = &data;
}

int adcOutput;

void ADC_Init()
{
	adcOutput = 0;
}

void EEPROM_Init()
{
	hardware_revision = NONE;
}

int EEPROM_Read()
{
	if (HARDWARE_SERIAL_NUMBER == 0xABC1234)
	    return REV_A;
	else
		return REV_B;
}

void delay(int counter)
{
	int i;
	for(i = 0; (i < counter); i++)
		i++;
}

void TempMon_ISR(void) {
    /* Get ADC output from the port */
    adcOutput = ADC_Output();
    int temperature;

    if (hardware_revision == REV_A) {
        temperature = adcOutput;
    } else {
        temperature = adcOutput / 10;
    }

    /* Update LED based on temperature */
    if (temperature < 5 || temperature >= 105) {
        GPIO_Write(RED_LED_PIN, HIGH);
        GPIO_Write(YELLOW_LED_PIN, LOW);
        GPIO_Write(GREEN_LED_PIN, LOW);
    }
    else if (temperature >= 85) {
        GPIO_Write(RED_LED_PIN, LOW);
        GPIO_Write(YELLOW_LED_PIN, HIGH);
        GPIO_Write(GREEN_LED_PIN, LOW);
    }
    else {
        GPIO_Write(RED_LED_PIN, LOW);
        GPIO_Write(YELLOW_LED_PIN, LOW);
        GPIO_Write(GREEN_LED_PIN, HIGH);
    }
}


int main(void) {

    /* Initialize GPIO pins for LEDs */
    GPIO_Init(GREEN_LED_PIN);
    GPIO_Init(YELLOW_LED_PIN);
    GPIO_Init(RED_LED_PIN);

    /* Initialize ADC for temperature sensor reading */
    ADC_Init();

    /* Initialize EEPROM and read configuration */
    EEPROM_Init();
    int hardware_revision = EEPROM_Read(HARDWARE_SERIAL_NUMBER);

    /* Main control loop */
    while (1) {
    	/* Call Interrupt service routine to get ADC data */
    	TempMon_ISR();
        /* Wait for next sample (100µs) */
        delay(100);
    }
}
