#include <stdio.h>

#include "toylog.h"

int main(int argc, char *argv[]) {
  openlog();

  toylog("hello");

  return 0;
}
