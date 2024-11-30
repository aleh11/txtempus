#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>  // File control definitions
#include <termios.h> // POSIX terminal control definitions
#include <unistd.h>  // UNIX standard function definitions
#include <chrono>    // For time management
#include <thread>    // For sleep_for

using namespace std;

int main() {
    const char* device = "/dev/serial0"; // UART device
    int serialPort = open(device, O_RDWR | O_NOCTTY | O_NDELAY);

    if (serialPort == -1) {
        cerr << "Failed to open serial port!" << endl;
        return 1;
    }

    // Configure the serial port
    struct termios options;
    tcgetattr(serialPort, &options);
    cfsetispeed(&options, B9600); // Set baud rate to 9600
    cfsetospeed(&options, B9600);
    options.c_cflag |= (CLOCAL | CREAD); // Enable receiver and set local mode
    options.c_cflag &= ~CSIZE;          // Mask the character size bits
    options.c_cflag |= CS8;             // 8 data bits
    options.c_cflag &= ~PARENB;         // No parity
    options.c_cflag &= ~CSTOPB;         // 1 stop bit
    tcsetattr(serialPort, TCSANOW, &options);

    cout << "Reading GPS data every 30 seconds..." << endl;

    char buffer[256];
    while (true) {
        string gpsData = ""; // Buffer to store a complete NMEA sentence

        // Read data until a newline (end of an NMEA sentence)
        while (true) {
            int bytesRead = read(serialPort, buffer, 1); // Read one byte at a time
            if (bytesRead > 0) {
                if (buffer[0] == '\n') {
                    break; // End of NMEA sentence
                }
                gpsData += buffer[0]; // Append byte to gpsData
            }
        }

        // Check if the sentence is valid and display it
        if (!gpsData.empty() && gpsData.find("$GPGGA") != string::npos) { // Filter for GPGGA
            cout << "GPS Reading: " << gpsData << endl;
        }

        // Wait for 30 seconds before the next reading
        this_thread::sleep_for(chrono::seconds(30));
    }

    close(serialPort);
    return 0;
}
