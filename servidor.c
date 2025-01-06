/*
 * Archivo para las funciones del servidor en el proyecto de shell remoto
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

#define SERVER_PORT 12348
#define MAX_RESPONSE_LENGTH BUFSIZ

#define ANSI_COLOR_RED "\e[0;31m"
#define ANSI_COLOR_GREEN "\e[0;32m"
#define ANSI_COLOR_BLUE "\e[0;34m"
#define ANSI_COLOR_CYAN "\e[0;36m"
#define ANSI_COLOR_RESET "\x1b[0m"

int main()
{
  int serverSocket = TCP_Server_Open(SERVER_PORT);
  int clientSocket = TCP_Accept(serverSocket);

  while (1)
  {
    char command[BUFSIZ];
    TCP_Read_String(clientSocket, command, BUFSIZ);

    if (strcmp("salida", command) == 0) //Si se envía ell comando para cerrar conexión
    {
      TCP_Write_String(clientSocket, ANSI_COLOR_GREEN "Cerrando conexion..." ANSI_COLOR_RESET);
      break;
    }
    if (strncmp(command, "create", 6) == 0) {
      char *filename = strchr(command,' ');
    if (filename != NULL) {
        filename++; // Avanzar al siguiente carácter después del espacio
        printf("Archivo a crear: %s\n", filename);
        
        if (access(filename, F_OK) != -1) {
            // El archivo existe
            TCP_Write_String(clientSocket, "El archivo ya existe.");
        } else {
            // Intentamos crear el archivo
            FILE* archivo = fopen(filename, "w");
            if (archivo != NULL) {
                printf("Se ha creado el archivo '%s'.\n", filename);
                TCP_Write_String(clientSocket, "Se ha creado el archivo");
                fclose(archivo);
            } else {
                printf("No se pudo crear el archivo '%s'.\n", filename);
                TCP_Write_String(clientSocket, "El archivo no se pudo crear en el servidor.");
            }
        }
    }
    continue;
  }   
     if (strncmp(command, "delete", 4) == 0) {
            char *filename = strchr(command, ' ');
            if (filename != NULL) {
                filename++; // Avanzar al siguiente carácter después del espacio
                printf("Archivo a borrar: %s\n", filename);
                if (access(filename, F_OK) != -1) {
                    // El archivo existe, se borra
                    if (remove(filename) == 0) {
                      printf("El archivo '%s' ha sido borrado.\n", filename);
                      TCP_Write_String(clientSocket, "El archivo ha sido borrado.");
                    } else {
                     printf("No se pudo borrar el archivo '%s'.\n", filename);
                     TCP_Write_String(clientSocket, "No se pudo borrar el archivo."); 
                    }
               }
            }
      continue;
    }
    pid_t pid = fork();

    if (pid == 0)
    {
      // Redirigir la salida estándar al socket
      dup2(clientSocket, STDOUT_FILENO);
      dup2(clientSocket, STDERR_FILENO);

      // Cerrar el socket de lectura en el proceso hijo
      close(clientSocket);

      // Ejecutar el comando y salir
      execlp("/bin/sh", "/bin/sh", "-c", command, NULL);
      perror("Error al ejecutar el comando en el SERVIDOR");
      exit(1);
    }else if (pid > 0) {
    // Padre: esperar que el proceso hijo termine antes de continuar
    int status;
    waitpid(pid, &status, 0);
    // Verificar si el proceso hijo finalizó correctamente
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        // Comando ejecutado sin errores, enviar marca de fin de respuesta
        TCP_Write_String(clientSocket, "$");
    } else {
        // Comando finalizado con errores, enviar mensaje de error al cliente
        TCP_Write_String(clientSocket, ANSI_COLOR_RED "Error al ejecutar el comando." ANSI_COLOR_RESET);
       }
    }  else
    {
      // Ocurrió un error al intentar crear el proceso hijo
      TCP_Write_String(clientSocket, ANSI_COLOR_RED "Error al ejecutar el comando." ANSI_COLOR_RESET);
    }
    TCP_Write_String(clientSocket, "$");
    bzero(command, BUFSIZ);
  }

  TCP_Close(clientSocket);
  TCP_Close(serverSocket);
  return 0;
}
