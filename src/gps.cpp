#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>  // File control definitions
#include <termios.h> // POSIX terminal control definitions
#include <unistd.h>  // UNIX standard function definitions

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

    cout << "Reading GPS data..." << endl;

    char buffer[256];
    while (true) {
        int bytesRead = read(serialPort, buffer, sizeof(buffer) - 1);

        if (bytesRead > 0) {
            buffer[bytesRead] = '\0'; // Null-terminate the buffer
            cout << buffer;          // Print the raw GPS data
        } else {
            usleep(100000); // Sleep for 100 ms
        }
    }

    close(serialPort);
    return 0;
}