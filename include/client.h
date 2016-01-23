#ifndef CLIENT_H
#define CLIENT_H

#define MAILDIR "/home/pavel/toy-smtp-client/mail"

typedef struct {
  int accept_new_messages;
} Client;

int client_create(Client*);
int client_run(Client*);
int client_discover(Client*);

#endif
