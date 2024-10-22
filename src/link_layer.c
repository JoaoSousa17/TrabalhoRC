// Link layer protocol implementation

#include "link_layer.h"
#include "serial_port.h"
#include "bcc1_utils.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source
#define FLAG 0x7E
#define ADDRESS_SENT_BY_SENDER 0x03
#define ADDRESS_SENT_BY_RECEIVER 0x01
#define CONTROL_SET 0x03
#define CONTROL_UA 0x07

// Variáveis globais para rastrear as estatísticas
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
int llwrite(const unsigned char *buf, int bufSize){
    unsigned char frame[5]; 
    frame[0] = FLAG;           
    frame[1] = ADDRESS_SENT_BY_SENDER;           
    frame[2] = CONTROL_SET;           
    frame[3] = calculate_bcc1(frame[1], frame[2]); 
    frame[4] = FLAG;           

    int bytes_written = writeBytesSerialPort(frame, 5);
    if (bytes_written != 5){
        printf("Erro ao enviar a trama SET!");
        return -1;
    }

    // Atualizar as estatísticas de bytes enviados
    totalBytesSent += bytes_written;

    printf("Trama SET enviada, com sucesso!\n");
    return bytes_written;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet){
    unsigned char received_frame[5];
    int bytes_read = 0;
    unsigned char byte;

    // Ler 5 bytes (a trama SET), byte por byte usando readByteSerialPort
    while (bytes_read < 5) {
        int result = readByteSerialPort(&byte);
        if (result < 0) {
            printf("Erro na leitura do byte da Porta Série!\n");
            return -1;
        } else if (result == 0) {
            // Timeout sem receber byte
            continue;
        } else {
            // Sucesso na leitura de um byte
            received_frame[bytes_read] = byte;
            bytes_read++;
        }
    }

    // Atualizar as estatísticas de bytes recebidos
    totalBytesReceived += bytes_read;

    // Verificar FLAGs, Address, Control e BCC1
    if (received_frame[0] == FLAG && received_frame[4] == FLAG &&
        received_frame[1] == ADDRESS_SENT_BY_SENDER && received_frame[2] == CONTROL_SET &&
        received_frame[3] == calculate_bcc1(received_frame[1], received_frame[2])){
        
        printf("Trama SET recebida e validada!\n");

        // Responder com UA
        unsigned char ua_frame[5];
        ua_frame[0] = FLAG;           
        ua_frame[1] = ADDRESS_SENT_BY_RECEIVER;           
        ua_frame[2] = CONTROL_UA;           
        ua_frame[3] = calculate_bcc1(ua_frame[1], ua_frame[2]); // BCC1
        ua_frame[4] = FLAG;           

        int ua_written = writeBytesSerialPort(ua_frame, 5);
        if (ua_written != 5) {
            printf("Erro ao enviar a trama UA!\n");
            return -1;
        }

        // Atualizar as estatísticas de bytes enviados
        totalBytesSent += ua_written;

        printf("Trama UA enviada, com sucesso!\n");
        return 0;
    } else {
        printf("Erro: Trama SET inválida!\n");
        return -1;
    }
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics) {
    if (showStatistics) {
        // Exibir as estatísticas de bytes enviados e recebidos
        printf("Estatísticas:\n");
        printf("Total de bytes enviados: %d\n", totalBytesSent);
        printf("Total de bytes recebidos: %d\n", totalBytesReceived);
    }

    int clstat = closeSerialPort();
    return clstat;
}
