#include <stdio.h>
#include <string.h>
#include "link_layer.h"

// Function to test link layer operations
void testLinkLayer(LinkLayerRole role) {
    LinkLayer connectionParams;

    // Set the serial port based on the role
    if (role == LlTx) {
        strcpy(connectionParams.serialPort, "/dev/ttyS10"); // Transmitter
    } else {
        strcpy(connectionParams.serialPort, "/dev/ttyS11"); // Receiver
    }

    connectionParams.role = role;
    connectionParams.baudRate = 9600;  // Set the baud rate
    connectionParams.nRetransmissions = 3;  // Example value
    connectionParams.timeout = 30;  // Example value

    // Test llopen
    if (llopen(connectionParams) == 1) {
        printf("[%s] Connection opened successfully.\n", role == LlTx ? "Transmitter" : "Receiver");
    } else {
        printf("[%s] Failed to open connection.\n", role == LlTx ? "Transmitter" : "Receiver");
        return; // Exit if opening fails
    }

    // Test llwrite (only for transmitter)
    if (role == LlTx) {
        int result = llwrite(NULL, 0); // Focus on sending the SET frame
        if (result == 0) {
            printf("[%s] SET frame sent successfully.\n", "Transmitter");
        } else {
            printf("[%s] Failed to send SET frame.\n", "Transmitter");
        }
    }

    // Test llread (only for receiver)
    if (role == LlRx) {
        unsigned char receivedPacket[MAX_PAYLOAD_SIZE];
        int bytesRead = llread(receivedPacket);
        if (bytesRead >= 0) {
            printf("[%s] UA frame received successfully.\n", "Receiver");
        } else {
            printf("[%s] Failed to read data.\n", "Receiver");
        }
    }

    // Test llclose
    if (llclose(TRUE) == 1) {
        printf("[%s] Connection closed successfully.\n", role == LlTx ? "Transmitter" : "Receiver");
    } else {
        printf("[%s] Failed to close connection.\n", role == LlTx ? "Transmitter" : "Receiver");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <role>\n", argv[0]);
        fprintf(stderr, "role: LlTx or LlRx\n");
        return -1;
    }

    LinkLayerRole role;
    if (strcmp(argv[1], "LlTx") == 0) {
        role = LlTx;
    } else if (strcmp(argv[1], "LlRx") == 0) {
        role = LlRx;
    } else {
        fprintf(stderr, "Invalid role specified. Use 'LlTx' or 'LlRx'.\n");
        return -1;
    }

    testLinkLayer(role);
    return 0;
}
