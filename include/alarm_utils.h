// alarm_utils.h
#ifndef _ALARM_UTILS_H_
#define _ALARM_UTILS_H_

// Function prototypes
void alarmHandler(int signal);
void setupAlarm(int maxRetries);
void activateAlarm(int seconds);
void deactivateAlarm();
int maxRetriesReached();

// Declare external variables (without initialization)
extern int alarmEnabled;
extern int alarmCount;
extern int maxRetransmissions;

#endif // _ALARM_UTILS_H_
