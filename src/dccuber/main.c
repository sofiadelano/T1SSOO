#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdbool.h>

#include "../file_manager/manager.h"

int semaforos[3] = {1, 1, 1};
int semaforos_pid[3];
pid_t *repartidores_pid;
int cant_repartidores;
pid_t fabrica_pid;
pid_t principal_pid;

void handle_sigint(int sig)
{
  printf("sigint a último\n");
  kill(fabrica_pid, SIGABRT);
}

void handle_sigalrm(int sig)
{
  printf("alarma\n");
  char *filename = "input.txt";
  InputFile *data_in = read_file(filename);
  int cantidad_restante = strtol(data_in->lines[1][1], NULL, 10);
  int tiempo_generacion = strtol(data_in->lines[1][0], NULL, 10);
  int estado_semaforos[3];
  int ubicacion_semaforos[3];
  int ubicacion_bodega;
  int ultimo;
  int pid_parent = getpid();

  for (int i = 1; i < cantidad_restante; i++)
  {
    repartidores_pid[i] = fork();
    if (!repartidores_pid[i])
    {
      char *myargs[11];
      sprintf(&estado_semaforos[0], "%d", semaforos[0]);
      sprintf(&estado_semaforos[1], "%d", semaforos[1]);
      sprintf(&estado_semaforos[2], "%d", semaforos[2]);
      sprintf(&ubicacion_semaforos[0], "%d", strtol(data_in->lines[0][0], NULL, 10));
      sprintf(&ubicacion_semaforos[1], "%d", strtol(data_in->lines[0][1], NULL, 10));
      sprintf(&ubicacion_semaforos[2], "%d", strtol(data_in->lines[0][2], NULL, 10));
      sprintf(&ubicacion_bodega, "%d", strtol(data_in->lines[0][3], NULL, 10));
      sprintf(&pid_parent, "%d", pid_parent);
      if (i == cantidad_restante-1)
      {
        printf("SE MANDO ULTIMOOOOO\n");
        sprintf(&ultimo, "%d", 1);
      } else {
        sprintf(&ultimo, "%d", 0);
      }
      myargs[0] = strdup("./repartidor");
      myargs[1] = &estado_semaforos[0];
      myargs[2] = &estado_semaforos[1];
      myargs[3] = &estado_semaforos[2];
      myargs[4] = &ubicacion_semaforos[0];
      myargs[5] = &ubicacion_semaforos[1];
      myargs[6] = &ubicacion_semaforos[2];
      myargs[7] = &ubicacion_bodega;
      myargs[8] = &ultimo;
      myargs[9] = &pid_parent;
      myargs[10] = NULL;
      execvp(myargs[0], myargs);
    }
    sleep(tiempo_generacion);
  }
}

void handle_sigusr1(int sig, siginfo_t *siginfo, void *context)
{
  int number_received = siginfo->si_value.sival_int;
  semaforos[number_received] = !semaforos[number_received];
  //printf("Padre: Recibi semaforo id %i en estado %i\n", number_received, semaforos[number_received]);
  for (int i = 0; i < cant_repartidores; i++)
  {
    send_signal_with_int(repartidores_pid[i], number_received);
  }
}

void handle_sigusr2(int sig)
{
  printf("Llegó señal del último\n");
  kill(principal_pid, SIGINT);
}

void handle_sigabrt(int sig)
{
  int status;
  printf("ABRT A FABRICA\n");
  for (int i = 0; i < cant_repartidores; i++)
  {
    kill(repartidores_pid[0], SIGABRT);
  }
}

int main(int argc, char const *argv[])
{
  printf("I'm the DCCUBER process and my PID is: %i\n", getpid());

  char *filename = "input.txt";
  InputFile *data_in = read_file(filename);

  printf("Leyendo el archivo %s...\n", filename);
  printf("- Lineas en archivo: %i\n", data_in->len);
  printf("- Contenido del archivo:\n");

  printf("\t- ");
  for (int i = 0; i < 4; i++)
  {
    printf("%s, ", data_in->lines[0][i]);
  }
  printf("\n");

  printf("\t- ");
  for (int i = 0; i < 5; i++)
  {
    printf("%s, ", data_in->lines[1][i]);
  }
  printf("\n");

  principal_pid = getpid();
  // inicializar variables
  cant_repartidores = strtol(data_in->lines[1][1], NULL, 10);
  repartidores_pid = calloc(cant_repartidores, sizeof(pid_t));
  char *pid_parent = malloc(sizeof(char));
  int status_fabrica;
  int status;
  // Crear fábrica
  fabrica_pid = fork();

  if (!fabrica_pid) // Solo el fabrica cumple el if
  {
    printf("Hola soy la fabrica mi pid es %i\n", getpid());
    int pid_parent2 = getpid();
    // // Crear RePARTIDORES
    repartidores_pid[0] = fork();
    if (!repartidores_pid[0])
    {
      int estado_semaforos[3];
      int ubicacion_semaforos[3];
      int ubicacion_bodega;
      int ultimo;
      char *myargs[11];
      sprintf(&estado_semaforos[0], "%d", semaforos[0]);
      sprintf(&estado_semaforos[1], "%d", semaforos[1]);
      sprintf(&estado_semaforos[2], "%d", semaforos[2]);
      sprintf(&ubicacion_semaforos[0], "%d", strtol(data_in->lines[0][0], NULL, 10));
      sprintf(&ubicacion_semaforos[1], "%d", strtol(data_in->lines[0][1], NULL, 10));
      sprintf(&ubicacion_semaforos[2], "%d", strtol(data_in->lines[0][2], NULL, 10));
      sprintf(&ubicacion_bodega, "%d", strtol(data_in->lines[0][3], NULL, 10));
      sprintf(&ultimo, "%d", 0);
      sprintf(&pid_parent2, "%d", pid_parent2);
      myargs[0] = strdup("./repartidor");
      myargs[1] = &estado_semaforos[0];
      myargs[2] = &estado_semaforos[1];
      myargs[3] = &estado_semaforos[2];
      myargs[4] = &ubicacion_semaforos[0];
      myargs[5] = &ubicacion_semaforos[1];
      myargs[6] = &ubicacion_semaforos[2];
      myargs[7] = &ubicacion_bodega;
      myargs[8] = &ultimo;
      myargs[9] = &pid_parent2;
      myargs[10] = NULL;
      execvp(myargs[0], myargs);
    }
    else
    {
      signal(SIGALRM, handle_sigalrm);
      signal(SIGUSR2, handle_sigusr2);
      signal(SIGABRT, handle_sigabrt);
      alarm(strtol(data_in->lines[1][0], NULL, 10));
      connect_sigaction(SIGUSR1, handle_sigusr1);
      printf("pid rep %i\n", repartidores_pid[0]);
      waitpid(repartidores_pid[cant_repartidores-1], &status_fabrica, 0);
    }
  }
  else
  {
    sprintf(pid_parent, "%d", fabrica_pid);
    for (int i = 0; i < 3; i++)
    {
      semaforos_pid[i] = fork();
      if (!semaforos_pid[i])
      {
        char *myargs[5];
        char id_semaforo = i + '0';
        myargs[0] = strdup("./semaforo");
        myargs[1] = &id_semaforo;
        myargs[2] = data_in->lines[1][2 + i];
        myargs[3] = pid_parent;
        myargs[4] = NULL;
        execvp(myargs[0], myargs);
      }
    }
    int status_main;
    signal(SIGINT, handle_sigint);
    waitpid(fabrica_pid, &status_main, 0);
    kill(semaforos_pid[0], SIGABRT);
    kill(semaforos_pid[1], SIGABRT);
    kill(semaforos_pid[2], SIGABRT);
    waitpid(semaforos_pid[0], &status, 0);
    waitpid(semaforos_pid[1], &status, 0);
    waitpid(semaforos_pid[2], &status, 0);
    printf("Liberando memoria...\n");
    input_file_destroy(data_in);
    //free(repartidores_pid);
    return 0;
  }
}
