//Simple Test Program
//
// Created by: Patrick McCabe
//
#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  printf(1, "The PID is: %d and the PPID is %d\n", getpid(), getppid());
  exit();
}
