#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>

int other_id;
int current_index = 0;
int len_numbers = 9;
int numbers[9] = {1, 2, 3, 4, 5, 6, 7, 125, 9};

/** Connects a signal to a more powerful handler
 *
 * @param sig: Received signal (ex: SIGUSR1)
 * @param handler: Function to be called when the signal is received
*/
void connect_sigaction(int sig, void (*handler)(int, siginfo_t *, void *))
{
  // Define the sigaction struct required to setup the handler
  struct sigaction action;

  // Setup the handler function
  action.sa_sigaction = handler;

  // Set the mask as empty
  sigemptyset(&action.sa_mask);

  // Set SA_SIGINFO as the primary flag
  // This tells sigaction that the handler receives three parameters instead
  // of one
  action.sa_flags = SA_SIGINFO;

  // Associate the signal to the handler
  sigaction(sig, &action, NULL);
}

/** Sends a SIGUSR1 signal to a process with an int payload
 *
 * @param pid: Process that will receive the signal
 * @param payload: Payload to send along with the signal
*/
void send_signal_with_int(int pid, int payload)
{
  union sigval sig = {};
  sig.sival_int = payload;
  sigqueue(pid, SIGUSR1, sig);
}

// Este es el handler SIGINT del hijo
void handle_sigint(int sig)
{
  printf("Gracefully finishing\n");

  // Abrimos un archivo en modo de lectura
  FILE *output = fopen("output.txt", "w");
  for (int i = 0; i < current_index; i++)
  {
    fprintf(output, "%i", numbers[i]);
    // No agregamos el separador al último número
    if (i + 1 != current_index)
      fprintf(output, ";");
  }

  // Se cierra el archivo (si no hay leak)
  fclose(output);

  // Terminamos el programa con exit code 0
  exit(0);
}

// Este es el handler SIGUSR1 del hijo
void handle_sigusr1(int sig, siginfo_t *siginfo, void *context)
{
  int number_received = siginfo->si_value.sival_int;
  printf("Hijo: Recibi %i\n", number_received);
  numbers[current_index++] = number_received;
}

// Este es el handler SIGALRM del padre
void handle_sigalrm(int sig)
{
  // Vamos a enviarle todos los números al hijo
  for (int i = 0; i < len_numbers; i++)
  {
    printf("Padre: Envio %i\n", numbers[i]);

    // Le enviamos numbers[i] al hijo
    send_signal_with_int(other_id, numbers[i]);

    // Esperamos un segundo antes de enviar otro (esto es solo para que los prints se vean ordenados)
    sleep(1);
  }

  // Le mandamos SIGINT al hijo para que termine
  kill(other_id, SIGINT);
}

int main(int argc, const char **argv)
{
  // Creamos un proceso hijo
  other_id = fork();
  if (!other_id) // Solo el hijo cumple el if
  {
    // Conectamos SIGINT a handle_sgint y SIGUSR1 a handle_sigusr1
    signal(SIGINT, handle_sigint);
    connect_sigaction(SIGUSR1, handle_sigusr1);

    // Se espera infinitamente
    while (true)
      ;
  }

  // Estas lineas solo las alcanza el proceso padre

  // Conectamos SIGALRM a handle_sigalrm
  signal(SIGALRM, handle_sigalrm);

  // Puedes intentar de cambiar la siguiente linea por alarm(1);
  kill(getpid(), SIGALRM);

  // Esperamos al único hijo
  wait(NULL);
  printf("Padre ha terminado.\n");
  return 0;
}