#ifndef MAIL_H
#define MAIL_H

#include <stdlib.h>
#include <sys/types.h>

typedef struct {
  char* to;
  size_t tol;
  char* from;
  size_t froml;
  char* body;
  size_t bodyl;
  char* filename;
} Mail;

Mail* mail_read(const char* filename);

#endif
