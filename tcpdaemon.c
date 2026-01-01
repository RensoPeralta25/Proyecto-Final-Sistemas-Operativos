#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define LOG_FILE "/var/log/tcpdaemon.log" // ruta del archivo .log
#define TRUE 1
#define PORT 8080

FILE *log_file; //archivo que apunta a tcpdaemon.log
int server_fd;

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

	struct sockaddr_in address; // Estructura que define familia de protocolos IPv4 para comunicacion.
	socklen_t addrlen = sizeof(address); // Longitud de la direccion ip

	convertir_en_daemon();

	log_file = fopen(LOG_FILE, "a");

	if (!log_file){
		perror("No se pudo abrir el log");
		exit(EXIT_FAILURE);
	}

	escribir_log ("Daemon iniciado correctamente.");

	signal(SIGTERM, manejador);
	signal(SIGINT, manejador);

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0){
		escribir_log("Error al crear socket");
		exit(EXIT_FAILURE);
	}

	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {

		escribir_log("Error en bind()");
		exit(EXIT_FAILURE);

	}

	if (listen(server_fd, 10) < 0){

		escribir_log("Error en bind()");
		exit(EXIT_FAILURE);

	}

	char msg[128];

	snprintf(msg, sizeof(msg), "Servidor TCP escuchando en el puerto 8080");

	escribir_log(msg);

	// Bucle
	while (TRUE){

		int client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);

		if (client_fd < 0){
			escribir_log("Error en accept()");
			continue;
		}

		char msg[] = "Conexion aceptada por tcpdaemon\n";
		send(client_fd, msg, strlen(msg), 0);

		close(client_fd);
	}

	return 0;
}


