#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define LOG_FILE "/var/log/tcpdaemon.log" // ruta del archivo .log
#define TRUE 1

FILE *log_file; //archivo que apunta a tcpdaemon.log

// Funcion que abre y escribe en el .log

void escribir_log (const char *mensaje){

	time_t ahora = time(NULL); //funcion time que toma el tiempo actual.
	fprintf(log_file, "%s - %s\n", ctime(&ahora), mensaje);
	fflush(log_file);
}

// Manejador de signals

void manejador (int signal){

	if (signal == SIGTERM || signal == SIGINT){
		escribir_log("Signal recibida, cerrando daemon...");
		fclose(log_file);
		exit(0);
	}
}

// Funcion para convertir app en daemon

void convertir_en_daemon(){

	pid_t pid = fork(); //llama al syscall de fork

	if (pid < 0){
		exit(EXIT_FAILURE);
	}


	if (pid > 0) {
        	exit(EXIT_SUCCESS);  // Padre termina
    	}

   	umask(0);

    	if (setsid() < 0) {
        	exit(EXIT_FAILURE);
    	}

    	if (chdir("/") < 0) {
        	exit(EXIT_FAILURE);
    	}

    	close(STDIN_FILENO);
    	close(STDOUT_FILENO);
    	close(STDERR_FILENO);
}

int main(){

	log_file = fopen(LOG_FILE, "a");

	if (!log_file){
		perror("No se pudo abrir el log");
		exit(EXIT_FAILURE);
	}

	escribir_log ("Daemon iniciado correctamente.");

	signal(SIGTERM, manejador);
	signal(SIGINT, manejador);

	// Bucle
	while (TRUE){

		sleep(5);
		escribir_log("Daemon activo...");
	}

	return 0;
}


