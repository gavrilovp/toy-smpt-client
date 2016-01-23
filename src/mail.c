#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "mail.h"
#include "toylog.h"

#define BASE_SIZE 50
#define INC_SIZE 50

size_t getdelims(char** buf, size_t* bufsize, const int* delims,
                 int delimc, int* delimiter_hit, FILE* stream) {
  if (*buf == NULL) {
    *bufsize = BASE_SIZE;
    *buf = malloc(sizeof(char) * BASE_SIZE);
  }

  size_t strl = 0;
  
  while (1) {
    if (strl + 1 >= *bufsize) {
      size_t newsize = *bufsize + INC_SIZE;
      char* newbuf = malloc(sizeof(char) * newsize);

      memcpy(newbuf, *buf, strl * sizeof(char));
      free(*buf);
      *buf = newbuf;
      *bufsize = newsize;
    }

    char c = fgetc(stream);
    if (c == EOF) {
      *delimiter_hit = -1;
      return strl;
    }

    for (int i=0; i<delimc; i++) {
      if (delims[i] == c) {
        *delimiter_hit = c;
        (*buf)[strl] = '\0';
        return strl;
      }
    }

    (*buf)[strl++] = c;
  }
}

int parse_mailto(FILE* f, char* str, int* delhit, const int* delimiters,
                 char** name, size_t* name_max) {
  do {
    getdelims(name, name_max, delimiters, 3, delhit, f);
    
    if (*delhit != '@') {
      toylog("Error parsing mailto user name");
      free(name);
      return -1;
    }

    char *srv = NULL;
    size_t srv_max;

    getdelims(&srv, &srv_max, delimiters, 3, delhit, f);

    if (*delhit != ' ' && *delhit != '\n') {
      toylog("Error parsing mailto server name, delhit: %c", delhit);
      free(name);
      return -1;
    }

    if (*delhit == '\n') break;
    
  } while (1);

  return 0;
}

int parse_mailfrom(FILE* f, char* str, int* delhit, const int* delimiters, int* empty_sender,
                   char** snam, size_t* snam_max, size_t* snam_l,
                   char** ssrv, size_t* ssrv_max, size_t* ssrv_l) {
  *snam_l = getdelims(snam, snam_max, delimiters, 3, delhit, f);
  if (*delhit != ' ') {
    toylog("Mailfrom field not found");
    free(snam);
    return -1;
  }

  *snam_l = getdelims(snam, snam_max, delimiters, 3, delhit, f);
  if (*delhit != '@') {
    if (*delhit == '\n') {
      *empty_sender = 1;
    } else {
      toylog("Error parsing mailfrom user name");
      free(snam);
      return -1;
    }
  }

  if (!*empty_sender) {
    *ssrv_l = getdelims(ssrv, ssrv_max, delimiters, 3, delhit, f);
    if (*delhit != '\n') {
      toylog("Error parsing mailfrom user name");
      free(snam);
      free(ssrv);
      return -1;
    }
  }
  return 0;
}

Mail* mail_read(const char* filename) {
  FILE* f = fopen(filename, "r");
  if (f == NULL) {
    toylog("Unable to open file: %s", strerror(errno));
    return NULL;
  }

  char* str = NULL;
  size_t str_sz;

  // Don't need to check what getdelim returned because of next strcmp
  getdelim(&str, &str_sz, ' ', f);
  if (strcmp("mailto: ", str)) {
    toylog("Mailto field not found");
    free(str);
    return NULL;
  }

  const int delimiters[] = { ' ', '\n', '@' };
  int delhit;
  int empty_sender = 0;

  char* name = NULL;
  size_t name_max;

  toylog("Trying to parse mailto");
  if (parse_mailto(f, str, &delhit, delimiters, &name, &name_max) == -1) {
    toylog("Parsing mailto failed");
    free(str);
    return NULL;
  }

  toylog("Trying to parse mailfrom");
  char* snam = NULL; size_t snam_max, snam_l;
  char* ssrv = NULL; size_t ssrv_max, ssrv_l;
  if (parse_mailfrom(f, str, &delhit, delimiters, &empty_sender,
                     &snam, &snam_max, &snam_l,
                     &ssrv, &ssrv_max, &ssrv_l) == -1) {
    toylog("Parsing mailfrom failed");
    free(str);
    return NULL;
  }

  toylog("Tring to parse mailbody");
  char* body = NULL; size_t body_max, body_l;
  body_l = getdelim(&body, &body_max, EOF, f);
  if (body_l <= 0) {
    toylog("Parsing mailbody failed");
    free(str);
    return NULL;
  }
 
  Mail* m = malloc(sizeof(Mail));
  if (m == NULL) {
    free(str);
    return NULL;
  }

  m->to = name;
  m->tol = name_max;
  if (!empty_sender) {
    m->froml = snam_l + ssrv_l + 2;
    m->from = malloc(sizeof(char)*(m->froml));
    snprintf(m->from, m->froml, "%s@%s", snam, ssrv);
  } else {
    m->from = "\0";
    m->froml = 0;
  }
  m->body = body;
  m->bodyl = body_l;
  m->filename = malloc(sizeof(char)*(strlen(filename) + 1));
  strcpy(m->filename, filename);
  
  free(str);

  return m;
}
