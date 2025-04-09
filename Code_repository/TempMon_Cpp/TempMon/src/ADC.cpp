#include <iostream>
#include <cstring>
#include <stdexcept>

using namespace std;

/* AD structures, prototypes & variables */
unsigned eoc;                 // End of conversion
unsigned int base = 0x2000;   // Base address
bool AD_Enabled = false;      // Analog to Digital conversion enable flag
// Filtering of raw signal, debounce and Diagnostics flag
bool FilterDebounceDiagnosticsDone = false;
int CurrentChannel = 0;       // current active channel

/* Analog Channel definition */
struct Anachan {
    int data;
    int status;
};

// Define 8 Analog channels
Anachan AnalogChannel[8];

/* Analog Channel states */
enum Anastat {
    INACTIVE,
    START_CONVERSION,
    DATA_READY
};

// Main class to capture data and function definitions
class ADC {
public:
    ADC() {
        base = 0;
        AD_Enabled = false;
        CurrentChannel = 0;
        memset(&AnalogChannel, 0, sizeof(AnalogChannel));
    }

    // Initialize analog channel and set the base address
    unsigned InitializeAnalog() {
        base = get_port();
        eoc = base + 0x18;

        if (base == 0)
            return 0;

        // return base address
        return base;
    }

    // Set the active channel to START_CONVERSION and get the channel
    int TurnOnAnalog(int channel) {
        if (channel < 0 || channel > 7)
            return -1;

        AnalogChannel[channel].status = START_CONVERSION;
        AD_Enabled = true;

        return channel;
    }

    // Set the inactive channel to INACTIVE and get the last active data
    int TurnOffAnalog(int channel) {
        if (channel < 0 || channel > 7)
            return -1;

        AnalogChannel[channel].status = INACTIVE;

        return AnalogChannel[channel].data;
    }

    // Periodic timer interrupt. Checks and handles the status of the analog channels
    void new_timer(int timer_counter) {
    	// check if Analog to digital conversion is enabled
        if (AD_Enabled) {
        	//  look for start conversion or data ready status
            while (AnalogChannel[CurrentChannel].status != INACTIVE
            		&& (timer_counter != 0)) {
				// update the data and status of Analog channels
				switch (AnalogChannel[CurrentChannel].status) {
					case START_CONVERSION:
						AnalogChannel[CurrentChannel].status = DATA_READY;
						break;

					case DATA_READY:
						while (!(read_data(eoc) & 0x80));

						// Get data from Analog channel
						AnalogChannel[CurrentChannel].data = read_data(base);
						AnalogChannel[CurrentChannel].status = START_CONVERSION;

						break;
					case INACTIVE:
						break;
				}
				// increment to next channel
                CurrentChannel++;
				// decrement the counter until it becomes 0
				timer_counter--;
            }
        }
    }

    void SampleData() {
    	// Capture samples based on timer counter 100
        new_timer(100);

        unsigned x = InitializeAnalog();
        cout << "init ana = " << hex << x << endl;

        x = TurnOnAnalog(CurrentChannel);
        cout << "TurnOnAnalog(CurrentChannel); = " << x << endl;

        x = GetChannelData(CurrentChannel);
        cout << "GetChannelValue(CurrentChannel); = " << x << endl;

        // Perform filtering, debounce and Diagnostics on raw data
        FilterDebounceDiagnosticsDone = true;
    }

    char* ADC_Raw() {
        unsigned ADC_Chan0, dac1, eoc;
        int count;
        static char data[300];

        // Open the port and get the data
        ADC_Chan0 = get_port();

        if (ADC_Chan0 == 0) {
            cout << "no hardware found" << endl;
            return nullptr;
        }

        // Sample signals based on the timer counter
        SampleData();

        // set base and end of conversion address and offset
        dac1 = ADC_Chan0 + 8;
        eoc = ADC_Chan0 + 0x18;

        cout << "ADC Channel 0 after get_port = " << hex << ADC_Chan0 << endl;

        // capture the data from the channel and update buffer
        for (count = 0; count < 300; count++) {
            while (!(read_data(eoc) & 0x80))
            data[count] = (char) read_data(dac1);
        }

        return data;
    }

    // Get the data output from ADC
    int ADC_Output() {
    	if(FilterDebounceDiagnosticsDone == true)
    	{
            if (ADC_Raw() != nullptr)
                return 0;
            else
                return reinterpret_cast<int>(ADC_Raw());
    	}
    }

private:
    // Get the port address
    unsigned get_port() {
        static unsigned local_port;
        int x, portAddress;
        unsigned int not_ready_count, ready_count;

        if (local_port == 32767)
            return 0;

        for (x = 0x200; x < 0x3c0; x += 0x40) {
        	 not_ready_count = 32767;
			ready_count = 32767;
			write_data(x, 0);

			// Wait until the port address is updated. Polling
			while ((read_data(x + 0x18) & 0x80) && --not_ready_count);
			while (!(read_data(x + 0x18) & 0x80) && --ready_count);
			if (ready_count < 32767 && ready_count > 0) {
				local_port = base = x;
				portAddress = local_port + 0x18;
				return portAddress;
			}
		}
	}

	double get_frequency() {
		return 200.0;
	}

	int read_data(int x) {
		return x;
	}

	void write_data(int x, int data) {
		x = data;
	}

	int GetChannelData(int channel) {
		return AnalogChannel[channel].data;
	}
};

// main function to update ADC output
int ADC_main() {
	ADC adc;
	adc.SampleData();
	int output = adc.ADC_Output();
	return output;
}
