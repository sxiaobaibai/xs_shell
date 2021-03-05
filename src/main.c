#include "listen_input.h"

int main(int argc, char const* argv[])
{
  (void)(argc);
  (void)(argv);
  while (1)
    listen_input("$ ");
  //shell();
  return 0;
}
