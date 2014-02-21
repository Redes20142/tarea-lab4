#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

#define WEBROOT "/var/www"
#define EOL "\r\n"
#define EOL_SIZE 2

/*
 * Servidor de HTTP/1.0
 *
 * Autor: Manuel Ignacio Castillo López; 3-0801743-8.
 * manuel_castillo_cc@ciencias.unam.mx
 * Autor: Mijail Gutiérrez Valdéz; 3-423250-3.
 * mij000@ciencias.unam.mx
 * Version 1, febrero 2014
 */

// prototipos de funciones
int get_file_size(int);
void send_new(int, char *);
int recv_new(int, char *);
int connection(int fd);

// instancia el servidor y lo mantiene en ejecución
int main(void)
{
	//TODO wirte me
	return 0;
}//main

//devuelve el tamaño del fichero apuntado por el apuntador dado
int get_file_size(int fd)
{
	struct stat st;
	if(fstat(fd, &st) == -1)
	{
		return 1;
	}//comprueba que se pueda obtener el estado del archivo
	return (int) st.st_size;
}//get_file_size

// envia un mensaje
void send_new(int fd, char *msg)
{
	int len = strlen(msg);
	if(send(fd, msg, len, 0) == -1)
	{
		printf("Error al enviar\n");
	}//comrpueba que el envío haya sido exitoso
}//send_new

// recibe un nuevo mensaje.
int recv_new(int fd, char *buffer)
{
	char *p = buffer;
	unsigned short int eol_match = 0;
	while(recv(fd, p, 1, 0) != 0)
	{
		if(*p == EOL[eol_matched])
		{
			eol_match++;
			if(eol_match == EOL_SIZE)
			{
				*(p +1 -EOL_SIZE) = '\0';
				return (strlen(buffer));
			}//si se ha recibido el fin de línea
		}
		else
		{
			eol_match = 0;
		}//trata de cazar el fin de línea
		p++;
	}//lee el mensaje
	return 0;
}//recv_new

// inicia una conexión con el cliente
int connection(int fd)
{
	char request[512], resource[512], *ptr;
	int fd1, length, req_type = 0;
	if(recv_new(fd, request) == 0)
	{
		printf("Error al recibir una petici\u00F3n\n");
	}//recibe una petición
	ptr = strstr(request, "HTTP/");
	if(ptr == NULL)
	{
		printf("Error. No se reconoce el protocolo de una petici\u00F3n");
	}
	else
	{
		*ptr = 0;
		ptr = NULL;
		if(strncmp(request, "GET ", 4) == 0)
		{
			ptr = request +4;
			req_type = 1;
		}
		if(strncmp(request, "HEAD ", 5) == 0)
		{
			ptr = request +5;
			req_type = 2;
		}
		if(strncmp(request, "POST ", 5) == 0)
		{
			ptr = request +5;
			req_type = 3;
		}//responde al método
		if(ptr == NULL)
		{
			printf("M\u00E9todo no soportado\n");
		}
		else
		{
			if(ptr[strlen(ptr) -1] == '/')
			{
				strcat(ptr, "index.html");
			}//si se solicitó el archivo raíz
			strcpy(resource, WEBROOT);
			strcat(resoruce, ptr);
			fd1 = open(resoruce, O_RDONLY, 0);
			if(fd1 == -1)
			{
				send_new(fd, "HTTP/1.0 404 Not found\r\n");
				send_new(fd, "Server: NACHINTOCH-HTTPSERVER\r\n\r\n");
				send_new(fd, "404 - File not found\r\n\r\n");
			}
			else
			{
				send_new(fd, "HTTP/1.0 200 OK\r\n");
				send_new(fd, "server: NACHINTOCH-HTTPSERVER\r\n\r\n");
				switch(req_type) {
				case 1 :
					if((length = get_file_size(fd1)) == -1)
					{
						printf("ELOL al obtener el tamanio\n");
					}
					if((ptr = (char *) malloc(length)) == NULL)
					{
						printf("ELOL al dar espaccio\n");
					}//recupera el archivo y asigna espacio para la transmisión
					read(fd1, ptr, length);
					if(send(fd, ptr, length, 0) == -1)
					{
						printf("ELOL en transmision\n");
					}//comrpueba que la transmisión del arhivo haya sido exitosa
					free(ptr);
					break;
				case 2 :
					//TODO send head
					break;
				case 3 :
					//TODO receive POST
					break;
				default :
					//TODO send ELOL
				}//actua dependiendo el méotodo
			}//recupera el archivo solicitado
			close(fd);
		}//identifica el méotodo
	}//recibe una petición
	shutdown(fd, SHUT_RDWR);
}//conection

