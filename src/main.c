#include "toylog.h"
#include "client.h"

int main(int argc, char *argv[]) {
  openlog();
  toylog("Logger process has been started");

  Client c;

  toylog("Initializing client");
  client_create(&c);
  toylog("Launching client");
  client_run(&c);

  return 0;
}
