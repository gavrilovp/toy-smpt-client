#ifndef LOG_H
#define LOG_H

void openlog(void);
void toylog(const char* format, ...);
void closelog(void);

#endif
