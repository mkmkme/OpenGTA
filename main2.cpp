#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include "m_exceptions.h"
#include "log.h"

extern void parse_args(int argc, char* argv[]);
extern void on_exit();
extern void run_init(const char*);
extern void run_main();

int global_Done = 0;
int global_Restart = 0;

int main(int argc, char* argv[]) {
  if (argc > 1)
    parse_args(argc, argv);

  atexit(on_exit);

  run_init(argv[0]);
  run_main();
  if (global_Restart) {
    on_exit();
    execvp(argv[0], argv);
  }

  return 0;
}
