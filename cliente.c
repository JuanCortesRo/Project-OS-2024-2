/*
 * Archivo para las funciones del cliente en el proyecto de shell remoto
 * Codigo tomado y adaptado de: 
 * https://github.com/michaelRS2002/Proyecto-Shell-Remoto
 * https://github.com/cristiandpt/remote_terminal_in_rust
 * https://idiotdeveloper.com/file-transfer-using-tcp-socket-in-c/
 *
 * Modificado por: 
 * Miguel Casanova - miguel.felipe.casanova@correounivalle.edu.co
 * Johan Ceballos - johan.tabarez@correounivalle.edu.co
 * Juan Cortés - juan.jose.cortes@correounivalle.edu.co
 * Fecha: 2025-01-06
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "tcp.h"
#include "leercadena.h"

#define MAX_COMMAND_LENGTH 100
#define MAX_RESPONSE_LENGTH BUFSIZ

#define ANSI_COLOR_RED "\e[0;31m"
#define ANSI_COLOR_GREEN "\e[0;32m"
#define ANSI_COLOR_BLUE "\e[0;34m"
#define ANSI_COLOR_CYAN "\e[0;36m"
#define ANSI_COLOR_RESET "\x1b[0m"

int main(int argc, char *argv[])
{
  if (argc != 3) //Se espera que al ejecutar se ofrezcan exactamente tres argumentos. Si no es así, se especifica
  {
    fprintf(stderr, "%sPara conectarse, ofrecer los siguientes argumentos: %s <ip_servidor> <puerto>%s\n", ANSI_COLOR_BLUE, argv[0], ANSI_COLOR_RESET);
    return 1;
  }

  char *serverIP = argv[1]; //Se guarda el segundo argumento como la IP del servidor a conectar
  int serverPort = atoi(argv[2]); //El tercer argumento como el puerto

  int clientSocket = TCP_Open(serverIP, serverPort);

  if (clientSocket == -1) //Si no se conecta correctamente al socket
  {
    fprintf(stderr, "%sError al intentar conectar.%s\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
    return 1;
  }

  while (1)
  {
    char comando[MAX_COMMAND_LENGTH];
    bzero(comando, MAX_COMMAND_LENGTH);

    printf("%sDigite su comando. %sEscriba '%ssalida%s' para salir):> %s", ANSI_COLOR_BLUE,ANSI_COLOR_CYAN, ANSI_COLOR_RED, ANSI_COLOR_CYAN, ANSI_COLOR_RESET);
    leer_de_teclado(MAX_COMMAND_LENGTH, comando);

    TCP_Write_String(clientSocket, comando);

    if (strcmp(comando, "salida") == 0) //Comando para salir del servidor
    {
      printf("Cerrando conexion con servidor...\n");
      break;
    }

    if (strcmp(comando, "help") == 0) //Comando para salir del servidor
    {
      printf("%s%sls: Ver la lista de archivos.\n%screate %s<nombre_archivo>: Crea un nuevo archivo.\n%sdelete %s<nombre_archivo>:Borra un archivo.\n%ssalida: Cierra la conexión con el servidor (se acaba todo pues).%s\n",ANSI_COLOR_CYAN, ANSI_COLOR_RED, ANSI_COLOR_RED, ANSI_COLOR_RED, ANSI_COLOR_RED, ANSI_COLOR_RED, ANSI_COLOR_RED, ANSI_COLOR_RESET);
      continue;
    }
       if (strncmp(comando, "create", 6) == 0) { // Si el comando comienza con "crear"
    char *filename = strchr(comando, ' ');
      if (filename != NULL) {
        filename++; // Avanzar al nombre del archivo después del espacio

        // Enviar el nombre del archivo al servidor
        printf("Crear archivo: %s\n", filename);

        char response[MAX_RESPONSE_LENGTH];
        TCP_Read_String(clientSocket, response, MAX_RESPONSE_LENGTH);

        // Si el servidor envía la señal de que se ha creado el archivo
        if (strcmp(response, "Se ha creado el archivo") == 0) {
            // Archivo creado correctamente
            printf("%sEl archivo %s\n fue creado correctamente%s\n", ANSI_COLOR_GREEN, filename, ANSI_COLOR_RESET);
            continue;
        }
        if (strcmp(response, "El archivo ya existe.") == 0) {
            // Archivo ya existe
            printf("%sEl archivo %s\n ya existe.%s\n", ANSI_COLOR_RED, filename, ANSI_COLOR_RESET);
            continue;
        }
        if (strcmp(response, "El archivo no se pudo crear en el servidor.") == 0) {
            // Archivo no se pudo crear
            printf("%sError. No se pudo crear el archivo.%s\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
            continue;
        }
      }
    }
    if (strncmp(comando, "delete", 6) == 0) { // Si el comando comienza con "borrar"
    char *filename = strchr(comando, ' ');
      if (filename != NULL) {
        filename++; // Avanzar al nombre del archivo después del espacio

        // Enviar el nombre del archivo al servidor
        TCP_Write_String(clientSocket, filename);
        printf("Borrar archivo: %s\n", filename);

        char response[MAX_RESPONSE_LENGTH];
        TCP_Read_String(clientSocket, response, MAX_RESPONSE_LENGTH);

        // Si el servidor envía la señal de que borró el archivo
        if (strcmp(response, "El archivo ha sido borrado.") == 0) {
            printf("%sEl archivo %s\n ha sido borrado exitosamente.%s\n", ANSI_COLOR_RED,filename,ANSI_COLOR_RESET);
            continue;
        }
        if(strcmp(response,"No se pudo borrar el archivo.") == 0){
          printf("%sEl archivo %s\n no ha podido ser borrado.%s\n", ANSI_COLOR_RED,filename,ANSI_COLOR_RESET);
          continue;
        }
      }
    }
    

    char response[MAX_RESPONSE_LENGTH];
    bzero(response, MAX_RESPONSE_LENGTH);

    // Leer hasta encontrar la marca de fin de respuesta
    while (1)
    {
      TCP_Read_String(clientSocket, response, MAX_RESPONSE_LENGTH);
      if (strcmp(response, "$") == 0)
      {
        break;
      } else if (strcmp(response, "Error al ejecutar el comando.") == 0){
        break;
      }
      printf("%s -> \n%s%s\n", ANSI_COLOR_GREEN, response, ANSI_COLOR_RESET);
      bzero(response, MAX_RESPONSE_LENGTH);
    }
    bzero(comando, MAX_COMMAND_LENGTH);
  }

  TCP_Close(clientSocket);
  return 0;
}
