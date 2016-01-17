#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h>

#include "toylog.h"

static void logwriter(void);

static mqd_t mqd;

void openlog(void) {
  pid_t pid;
  // Should i call mq_open in both parent and child processes with
  // appropriate flags?
  int flags = O_CREAT | O_EXCL | O_RDWR;
  mode_t mode = 0664;

  struct mq_attr attr;
  attr.mq_flags   = 0;
  attr.mq_maxmsg  = 10;
  attr.mq_msgsize = 128;
  attr.mq_curmsgs = 0;

  mqd = mq_open("/toy-smtp-mq", flags, mode, &attr);
  if (mqd == -1) {
    perror("mq_open");
    exit(EXIT_FAILURE);
  }

  // Should i call fork inside of logwriter?
  switch (pid=fork()) {
  case -1:
    perror("fork");
    exit(EXIT_FAILURE);
  case 0:
    logwriter();
  }
}

static void logwriter(void) {
  time_t now;
  char dt_buffer[26];
  char rcvmsg[128];

  FILE* logfile = fopen("smtp-client.log", "a");
  if (logfile == NULL) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  while (1) {
    if (mq_receive(mqd, rcvmsg, 128, NULL) == -1) {
      perror("mq_receive");
      exit(EXIT_FAILURE);
    }

    now = time(NULL);
    if (now == -1) {
      perror("time");
      exit(EXIT_FAILURE);
    }
    strftime(dt_buffer, 26, "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(logfile, "%s %s\n", dt_buffer, rcvmsg);
    fflush(logfile);
  }
}

void closelog(void) {
  
}

void toylog(const char* message) {
  if (mq_send(mqd, message, strlen(message)+1, 1) == -1) {
    perror("mq_send");
    exit(EXIT_FAILURE);
  }
}
