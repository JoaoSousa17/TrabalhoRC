// Link layer protocol implementation

#include "link_layer.h"
#include "serial_port.h"
#include "bcc1_utils.h"
#include "alarm_utils.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source
#define FLAG 0x7E
#define ADDRESS_SENT_BY_SENDER 0x03
#define ADDRESS_SENT_BY_RECEIVER 0x01
#define CONTROL_SET 0x03
#define CONTROL_UA 0x07

// Variáveis para as estatísticas
int totalBytesSent = 0;
int totalBytesReceived = 0;

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters){
    if (openSerialPort(connectionParameters.serialPort, connectionParameters.baudRate) == -1){
        return -1;
    }
    return 1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize) {
    unsigned char frame[5]; 
    frame[0] = FLAG;           
    frame[1] = ADDRESS_SENT_BY_SENDER;           
    frame[2] = CONTROL_SET;           
    frame[3] = calculate_bcc1(frame[1], frame[2]); 
    frame[4] = FLAG;           

    setupAlarm(30);

    while (!maxRetriesReached()) {
        if (!alarmEnabled) {
            int bytes_written = writeBytesSerialPort(frame, 5);
            if (bytes_written != 5) {
                printf("Erro ao enviar o frame SET!");
                return -1;
            }

            totalBytesSent += bytes_written;
            printf("Frame SET enviada com sucesso!");
            activateAlarm(3);
        }

        unsigned char packet[5];
        int result = llread(packet);
        if (result >= 0) {
            deactivateAlarm(); 
            printf("Resposta UA recebida! Conexão estabelecida!");
            return 0;
        }
    }

    printf("Erro: Número máximo de retransmissões atingido!");
    return -1;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet) {
    unsigned char byte;
    enum { START, FLAG_RCV, A_RCV, C_RCV, BCC1_RCV, STOP } state = START;

    unsigned char address, control, bcc1;
    int bytes_read = 0;

    setupAlarm(30);
    activateAlarm(30);

    while (state != STOP) {
        int result = readByteSerialPort(&byte);
        if (result < 0) {
            printf("Erro na leitura do byte da Porta Série!\n");
            deactivateAlarm();
            return -1;
        } else if (result == 0) {  // Timeout sem receber byte
            if (maxRetriesReached()) {
                printf("Timeout: Número máximo de retransmissões atingido!\n");
                deactivateAlarm();
                return -1;
            }
            continue;
        } else {  // Sucesso na leitura de um byte
            switch (state) {
                case START:
                    if (byte == FLAG) {
                        state = FLAG_RCV;
                    }
                    break;

                case FLAG_RCV:
                    if (byte == ADDRESS_SENT_BY_SENDER) {
                        address = byte;
                        state = A_RCV;
                    } else if (byte != FLAG) {
                        state = START;
                    }
                    break;

                case A_RCV:
                    if (byte == CONTROL_SET) {
                        control = byte;
                        state = C_RCV;
                    } else if (byte == FLAG) {
                        state = FLAG;
                    } else {
                        state = START;
                    }
                    break;

                case C_RCV:
                    bcc1 = calculate_bcc1(address, control);
                    if (byte == bcc1) {
                        state = BCC1_RCV;
                    } else if (byte == FLAG) {
                        state = FLAG;
                    } else {
                        printf("Erro: BCC1 inválido!\n");
                        state = START;
                    }
                    break;

                case BCC1_RCV:
                    if (byte == FLAG) {
                        state = STOP;  
                    } else {
                        state = START;
                    }
                    break;

                default:
                    break;
            }
        }
    }
    deactivateAlarm();

    unsigned char ua_frame[5];
    ua_frame[0] = FLAG;           
    ua_frame[1] = ADDRESS_SENT_BY_RECEIVER;           
    ua_frame[2] = CONTROL_UA;           
    ua_frame[3] = calculate_bcc1(ua_frame[1], ua_frame[2]); 
    ua_frame[4] = FLAG;           

    int ua_written = writeBytesSerialPort(ua_frame, 5);
    if (ua_written != 5) {
        printf("Erro ao enviar o frame UA!\n");
        return -1;
    }

    totalBytesSent += ua_written;

    printf("Frame UA enviada com sucesso!\n");
    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics) {
    if (showStatistics) {
        printf("\tEstatísticas:\n");
        printf("Total de bytes enviados: %d;\n", totalBytesSent);
        printf("Total de bytes recebidos: %d;\n", totalBytesReceived);
    }

    int clstat = closeSerialPort();
    return clstat;
}
