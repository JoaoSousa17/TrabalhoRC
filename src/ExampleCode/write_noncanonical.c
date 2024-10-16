// Write to serial port in non-canonical mode
//
// Modified by: Eduardo Nuno Almeida [enalmeida@fe.up.pt]

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include "../../include/bcc1_utils.h"

// Baudrate settings are defined in <asm/termbits.h>, which is
// included by <termios.h>
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

#define FALSE 0
#define TRUE 1

#define BUF_SIZE 256



//TODO - verificar se estes valores estão certos

#define FLAG 0x7E // Synchronisation: start or end of frame

#define A_TRANSMITTER 0x03 // Address field in frames that are commands sent by the Transmitter or replies sent by the Receiver
#define A_RECEIVER 0X01 // Address field in frames that are commands sent by the Receiver or replies sent by the Transmitter

#define SET 0x03 // SET frame: sent by the transmitter to initiate a connection
#define UA 0x07 // UA frame: confirmation to the reception of a valid supervision frame


#define FRAME_SIZE 5 //Size of SET and UA frames


volatile int STOP = FALSE;

int main(int argc, char *argv[])
{
    // Program usage: Uses either COM1 or COM2
    const char *serialPortName = argv[1];

    if (argc < 2)
    {
        printf("Incorrect program usage\n"
               "Usage: %s <SerialPort>\n"
               "Example: %s /dev/ttyS1\n",
               argv[0],
               argv[0]);
        exit(1);
    }

    // Open serial port device for reading and writing, and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    int fd = open(serialPortName, O_RDWR | O_NOCTTY);

    if (fd < 0)
    {
        perror(serialPortName);
        exit(-1);
    }

    struct termios oldtio;
    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 1; // Inter-character timer unused
    newtio.c_cc[VMIN] = 0;  // Blocking read until 5 chars received

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");


    //Creating BCC1
    unsigned char BCC1_transmitter = calculate_bcc1(A_TRANSMITTER, SET);
    unsigned char BCC1_receiver = calculate_bcc1(A_RECEIVER, UA) ;



    //Creating set frame
    unsigned char setFrame[] = {FLAG, A_TRANSMITTER, SET, BCC1_transmitter, FLAG};


    //Sending SET frame
    int bytesWritten = write(fd, setFrame, FRAME_SIZE);
    if (bytesWritten < 0) {
        perror("Error writing to the serial port");
        close(fd);
        exit(-1);
    } else if (bytesWritten != FRAME_SIZE) {
        fprintf(stderr, "Error: Only %d bytes written out of 5\n", bytesWritten);
        close(fd);
        exit(-1);
    }




/*

    // Create string to send
    unsigned char buf[BUF_SIZE] = {0};

    for (int i = 0; i < BUF_SIZE; i++)
    {
        buf[i] = 'a' + i % 26;
    }

    // In non-canonical mode, '\n' does not end the writing.
    // Test this condition by placing a '\n' in the middle of the buffer.
    // The whole buffer must be sent even with the '\n'.
    buf[5] = '\n';

    int bytes = write(fd, buf, BUF_SIZE);
    printf("%d bytes written\n", bytes);

*/

    // Wait until all bytes have been written to the serial port
    sleep(1);


    
    
    // Reading and validating the UA frame response
    unsigned char response[FRAME_SIZE];
    int bytesRead = 0, totalBytesRead = 0;

    while (totalBytesRead < FRAME_SIZE) {
        bytesRead = read(fd, &response[totalBytesRead], FRAME_SIZE - totalBytesRead);
        
        if (bytesRead < 0) {
            perror("Error reading from the serial port");
            close(fd);
            exit(-1);
        }

        totalBytesRead += bytesRead;
    }

    // validate the frame
    if (response[0] == FLAG && response[1] == A_RECEIVER &&
        response[2] == UA && response[3] == BCC1_receiver && response[4] == FLAG) {
        printf("Received correct UA frame\n");
    } else {
        printf("Incorrect frame received\n");
    }




    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 0;
}
