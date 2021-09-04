#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>

void handle_sigabrt(int sig)
{
  printf("Semaforo %i ESCRIBIR ARCHIVO Y SALIR\n", getpid());
  exit(0);
}

int main(int argc, char const *argv[])
{
  connect_sigaction(SIGABRT, handle_sigabrt);
  int parent_pid = strtol(argv[3], NULL, 10);
  int id = strtol(argv[1], NULL, 10);
  int delay = strtol(argv[2], NULL, 10);
  bool prendido = true;
  int cambios = 0;
  printf("I'm the SEMAFORO process and my PID is: %i\n", getpid());
  while (true)
  {
    sleep(delay);
    prendido = !prendido;
    cambios++;
    //printf("SEMAFORO %i prendido: %d\n", id, prendido);
    send_signal_with_int(parent_pid, id);
  };
}
