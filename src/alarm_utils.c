#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "alarm_utils.h"

#define TRUE 1
#define FALSE 0


int alarmEnabled = FALSE;
int alarmCount = 0;
int maxRetransmissions = 3;


// Alarm handler function
void alarmHandler(int signal) {
    alarmEnabled = FALSE;
    alarmCount++;
    printf("Alarm #%d: Timeout! Retransmitindo a trama SET.\n", alarmCount);
}

// Configura o alarme e o handler
void setupAlarm(int maxRetries) {
    maxRetransmissions = maxRetries;

    struct sigaction act = {0};
    act.sa_handler = &alarmHandler;
    if (sigaction(SIGALRM, &act, NULL) == -1) {
        perror("Erro ao configurar sigaction");
        exit(EXIT_FAILURE);
    }

    alarmCount = 0;  // Reset alarm count
    alarmEnabled = FALSE;  // Reset alarm flag
}

// Ativa o alarme com um timeout de 'seconds'
void activateAlarm(int seconds) {
    if (alarmEnabled == FALSE) {
        alarm(seconds);
        alarmEnabled = TRUE;
    }
}

// Desativa o alarme
void deactivateAlarm() {
    alarm(0);  // Disable any pending alarms
    alarmEnabled = FALSE;
}

// Verifica se o número máximo de retransmissões foi atingido
int maxRetriesReached() {
    return (alarmCount >= maxRetransmissions);
}
