#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <netinet/tcp.h> 
#include <stdlib.h>
#include <string.h> 
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h> 
#include "tcp.h"

#define ANSI_COLOR_RED "\e[0;31m"
#define ANSI_COLOR_GREEN "\e[0;32m"
#define ANSI_COLOR_BLUE "\e[0;34m"
#define ANSI_COLOR_CYAN "\e[0;36m"
#define ANSI_COLOR_RESET "\x1b[0m"

int TCP_Open(char *hostname, t_port port) {
  return TCP_Open_By_IP(Get_IP(hostname),port);
}

//Esta es la funcion que usa un cliente para conectarse a un servidor
int TCP_Open_By_IP(char *hostip, t_port port) {
	int sockfd; 
	struct sockaddr_in servaddr; 

	//Creación y verificación del socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("%sCreación del socket fallida.%s\n", ANSI_COLOR_RED, ANSI_COLOR_RESET); 
		exit(0); 
	} 
	else
		printf("%sSocket creado con éxito.%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
	bzero(&servaddr, sizeof(servaddr)); 

	//Asignar IP y port
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr(hostip); 
	servaddr.sin_port = htons(port); 

	//Conectar el socket del cliente con el del servidor
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("%sConexión con el servidor fallida.%s\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
		exit(0); 
	} 
	else
		printf("%sConectado exitosamente con el servidor.%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);

	return sockfd;
}

//Esta funcion es usada para crear un servidor de socket
//El usuario pasa el número de puerto donde desea que este servidor sea accedido
int TCP_Server_Open(t_port port) {
	int sockfd; 
	struct sockaddr_in servaddr; 

	//Creación y verificación del socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("%sCreación del socket fallida.%s\n", ANSI_COLOR_RED, ANSI_COLOR_RESET); 
		exit(0); 
	} 
	else
		printf("%sSocket creado con éxito.%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
	bzero(&servaddr, sizeof(servaddr)); 

	//Asignar IP y port
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(port); 

	//Vincular el socket recién creado a una IP y verificación determinadas
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("%sVinculación del socket fallida.%s\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
		exit(0); 
	} 
	else
		printf("%sSocket vinculado exitosamente.%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);

	//Servidor listo para recibir información
	if ((listen(sockfd, 5)) != 0) { 
		printf("%sFallo al intentar establecer comunicación%s\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
		exit(0); 
	} 
	else
		printf("%sComunicación establecida exitosamente. %sEsperando instrucciones...%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_CYAN,ANSI_COLOR_RESET);

        return sockfd;
}

//Esta funcion espera por conexiones de red desde un cliente
//Devuelve un socket que es el canal de comunicacion entre el servidor y el cliente que solicita la conexion
int TCP_Accept(t_socket socket) {
	struct sockaddr_in servaddr, cli; 
	int len = sizeof(cli); 
	int connfd;

	printf("Servidor esperando conexiones...\n");
	//Aceptar el paquete del cliente y verificarlo
	connfd = accept(socket, (SA*)&cli, &len); 
	if (connfd < 0) { 
		printf("%sFallo al aceptar al cliente.%s\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
		exit(0); 
	} 
	else
		printf("%sCliente aceptado.%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);

        return connfd;
}

int TCP_Write_String(t_socket socket, char* string) {
	int len = strlen(string)*sizeof(char);
	int written;
	int flag = 1;
	written = write(socket,string, len);
	if (len != written) {
		printf("No se enviaron todos los datos %d/%d\n",written,len);
	}
	return written;
}

int TCP_Read_String(t_socket socket, char* string,int maxstring) {
	int _read;
	_read = read(socket,string,maxstring);

	return _read;
}

//Código tomado de https://idiotdeveloper.com/file-transfer-using-tcp-socket-in-c/
int TCP_Send_File(t_socket socket, char* filename) {
	int n;
	char buffer[BUFSIZ] = {0};
	FILE *fp;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "Fallo en abrir archivo (TCP_Send_File) %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	while (fgets(buffer, BUFSIZ, fp) != NULL) {
		int n;
		if ((n = send(socket, buffer, sizeof(buffer), 0)) == -1) {
			fprintf(stderr, "Fallo en enviar datos archivo (TCP_Send_File) %s\n",strerror(errno));
			exit(EXIT_FAILURE);
		}
		bzero(buffer,BUFSIZ);
	}
	strcpy(buffer,TCP_FIN_FILE);
	send(socket,buffer,sizeof(buffer),0);
	fclose(fp);
	
	return 0;
}

//Código tomado de https://idiotdeveloper.com/file-transfer-using-tcp-socket-in-c/
int TCP_Recv_File(t_socket socket, char* filename) {
	FILE *fp;
	int n;
	char buffer[BUFSIZ];

	fp = fopen(filename, "w");
	if (fp == NULL) {
		fprintf(stderr, "Fallo en abrir archivo (TCP_Recv_File) %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	while (1) {
		n = recv(socket, buffer, BUFSIZ, 0);
		if (n <= 0) break;
		if (!strcmp(TCP_FIN_FILE,buffer)) break;
		fprintf(fp,"%s",buffer);
	}
	fflush(fp); fclose(fp);

	return 0;
}

int TCP_Close(t_socket sck) {
	close(sck);
}

char *Get_IP(char *hostname) {
	struct hostent *host_entry;

	host_entry = gethostbyname(hostname);
	if (host_entry == NULL) {
		printf("host_entry failed..");
		return NULL;	
	}
	return inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
}

int Send_ACK(t_socket socket) {
  TCP_Write_String(socket,TCP_ACK);
}

int Recv_ACK(t_socket socket) {
  char ack[MAX_TCP_ACK] = {0};
  TCP_Read_String(socket,ack,MAX_TCP_ACK);
  return !strcmp(ack,TCP_ACK);
}
