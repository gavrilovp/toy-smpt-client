#define _GNU_SOURCE // d_type & DT_REG using
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "toylog.h"
#include "mail.h"
#include "client.h"

int client_create(Client *c) {
  // init servers map
  c->accept_new_messages = 1;

  return 0;
}

int file_exists (const char *filename, const char* suffix) {
    char path[PATH_MAX];
    int pathlen = snprintf(path, sizeof(path)-1, "%s%s", filename, suffix);
    path[pathlen] = '\0';

    struct stat buffer;
    int r = stat (path, &buffer);
    return (r == 0);
}

int file_has_extension(const char* filename, const char* suffix) {
    char *dot = strrchr(filename, '.');
    return dot && !strcmp(dot, suffix);
}

int mark_ignorable_mail(const char* filename) {
  int bl = strlen(filename) + strlen(".ignore") + 1;
  char buf[bl];
  snprintf(buf, bl, "%s%s", filename, ".ignore");
  FILE* f = fopen(buf, "a");
  if (f == NULL) {
    toylog("Unable to open file %s: %s", filename, strerror(errno));
    return -1;
  }
  fprintf(f, "Bad message format\n");
  fclose(f);
  return 0;
}

int client_discover(Client *c) {
  DIR* d;
  struct dirent* f;
  /* int discovered = 0; */

  if ((d = opendir(MAILDIR)) == NULL) {
    toylog("Unable to open %s: %s", MAILDIR, strerror(errno));
    return -1;
  }

  while ((f = readdir(d)) != NULL) {
    if (f->d_type != DT_REG) continue;

    char filename[PATH_MAX];
    snprintf(filename, sizeof(filename)-1, "%s/%s", MAILDIR, f->d_name);

    if (file_has_extension(filename, ".ignore") || file_has_extension(filename, ".lock")) continue;
    if (file_exists(filename, ".ignore") || file_exists(filename, ".lock")) continue;

    toylog("Trying to parse mail from: %s", filename);

    Mail* m = mail_read(filename);
    if (m == NULL) {
      toylog("Mark message %s as ignorable due to bad format", filename);
      if (mark_ignorable_mail(filename) == -1) {
        toylog("Cannot mark file %s as ignorable", filename);
        return -1;
      }
      continue;
    } else {
      toylog("Mail has been sucessfully parsed");
      toylog("mailto: %s", m->to);
      toylog("mailfrom: %s", m->from);
      toylog("body: \n%s", m->body);
    }
  }

  return 0;
}

int client_run(Client* c) {
  while (1) {
    int discovered = 0;
    if (c->accept_new_messages) {
      discovered = client_discover(c);
    }
    if (discovered) toylog("%d new mails found", discovered);
    
    break;
  }

  return 0;
}
