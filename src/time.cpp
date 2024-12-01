#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>  // File control definitions
#include <termios.h> // POSIX terminal control definitions
#include <unistd.h>  // UNIX standard function definitions
#include <sstream>   // For string stream
#include <chrono>    // For time management
#include <thread>    // For sleep_for

using namespace std;

// Function to parse time from a GNRMC sentence
string parseTime(const string& nmeaSentence) {
    if (nmeaSentence.substr(0, 6) == "$GNRMC") {
        stringstream ss(nmeaSentence);
        string token;
        int fieldIndex = 0;
        string time, validity;

        // Split the sentence by commas
        while (getline(ss, token, ',')) {
            if (fieldIndex == 1) { // Time field
                time = token;
            } else if (fieldIndex == 2) { // Validity field
                validity = token;
                break;
            }
            ++fieldIndex;
        }

        if (validity == "A") { // Only use time if validity is "A"
            return time;
        }
    }
    return ""; // Return empty if invalid
}

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

    cout << "Reading GPS data every 10 seconds..." << endl;

    char buffer[256];
    string nmeaSentence;

    while (true) {
        // Read data from the GPS module
        int bytesRead = read(serialPort, buffer, sizeof(buffer) - 1);

        if (bytesRead > 0) {
            buffer[bytesRead] = '\0'; // Null-terminate the buffer
            nmeaSentence += buffer;  // Append to the current sentence

            // Check if we have a complete NMEA sentence (ends with \n)
            size_t newlinePos = nmeaSentence.find('\n');
            if (newlinePos != string::npos) {
                string sentence = nmeaSentence.substr(0, newlinePos); // Extract the sentence
                nmeaSentence = nmeaSentence.substr(newlinePos + 1);  // Remove processed part

                // Parse and display the time if it's a GNRMC sentence
                string time = parseTime(sentence);
                if (!time.empty()) {
                    // Parse HHMMSS.sss format into a readable HH:MM:SS
                    string hours = time.substr(0, 2);
                    string minutes = time.substr(2, 2);
                    string seconds = time.substr(4, 2);

                    cout << "Current UTC Time: " << hours << ":" << minutes << ":" << seconds << endl;
                }
            }
        }

        // Wait for 10 seconds before the next reading
}
    close(serialPort);
    return 0;
}
