/*
 ============================================================================
 Name        : TempMon.cpp
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Temperature Monitoring and Display
 ============================================================================
*/

#include <iostream>
#include <thread>
#include <chrono>
#include "TempMon.hpp"

#define RED_LED_PIN    0x1000
#define YELLOW_LED_PIN 0x2000
#define GREEN_LED_PIN  0x3000

#define HARDWARE_SERIAL_NUMBER 0xABC1234

#define HIGH 1
#define LOW  0

enum HardwareRevision { REV_A = 0, REV_B, NONE };

class TempMon {
private:
    int adcOutput;
    int temperature;
    HardwareRevision hardware_revision;

public:
    TempMon() : adcOutput(0), temperature(0), hardware_revision(NONE) {}

    void GPIO_Write(int PIN, int status) {
        // Write data into GPIO port PIN
        // Since we're using C++, we assume this is just a placeholder function.
        std::cout << "GPIO " << std::hex << PIN << " set to " << status << std::dec << std::endl;
    }

    void GPIO_Init(int PIN) {
        // Initialize 0 to the address PIN
        int data = 0;
        std::cout << "GPIO " << std::hex << PIN << " initialized to " << data << std::dec << std::endl;
    }

    void ADC_Init() {
        adcOutput = 0;
    }

    void EEPROM_Init() {
    	// sample code for initialize
        hardware_revision = NONE;
    }

    int EEPROM_Read() {
        if (HARDWARE_SERIAL_NUMBER == 0xABC1234)
            return REV_A;
        else
            return REV_B;
    }

    // dummy delay function
    void delay(int counter) {
    	int i;
    	for(i = 0; (i < counter); i++)
    		i++;
    }

    int ADC_Output() {
        // Placeholder for actual ADC output function
        return adcOutput;
    }

    void TempMon_ISR() {
        /* Get ADC output from the port */
        adcOutput = ADC_Output();

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
        } else if (temperature >= 85) {
            GPIO_Write(RED_LED_PIN, LOW);
            GPIO_Write(YELLOW_LED_PIN, HIGH);
            GPIO_Write(GREEN_LED_PIN, LOW);
        } else {
            GPIO_Write(RED_LED_PIN, LOW);
            GPIO_Write(YELLOW_LED_PIN, LOW);
            GPIO_Write(GREEN_LED_PIN, HIGH);
        }
    }

    void MainLoop() {
        /* Initialize GPIO pins for LEDs */
        GPIO_Init(GREEN_LED_PIN);
        GPIO_Init(YELLOW_LED_PIN);
        GPIO_Init(RED_LED_PIN);

        /* Initialize ADC for temperature sensor reading */
        ADC_Init();

        /* Initialize EEPROM and read configuration */
        EEPROM_Init();
        hardware_revision = static_cast<HardwareRevision>(EEPROM_Read());

        /* Main control loop */
        while (true) {
            /* Call Interrupt service routine to get ADC data */
            TempMon_ISR();

            /* Wait for next sample (100µs) */
            delay(100);
        }
    }
};

int main() {
    TempMon tempMon;
    tempMon.MainLoop();
    return 0;
}
