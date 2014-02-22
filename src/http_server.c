#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#define WEBROOT "/var/www"
#define EOM "\r\n\r\n"
#define EOM_SIZE 4
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
void connection(int fd);

// instancia el servidor y lo mantiene en ejecución
int main(void)
{
	int sockfd, newfd;
	int err;
	struct addrinfo *res, *p, hints;
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	int yes = 1;
	char ip[INET6_ADDRSTRLEN];
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	char port[16];
	snprintf(port, sizeof(port), "%d", 80);
	if((err = getaddrinfo(NULL, port, &hints, &res)) == -1)
	{
		printf("Error al enlazar el puerto\n");
		exit(EXIT_FAILURE);
	}//obtiene el puerto
	for(p = res; p != NULL; p = p -> ai_next)
	{
		if((sockfd = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol)) == -1)
		{
			printf("Error al preparar el socket\n");
			printf("Compruebe que tiene permisos de super usuario\n");
			continue;
		}
		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		{
			printf("Error al preparar el socket\n");
			printf("Compruebe que tiene permisos de super usuario\n");
			exit(EXIT_FAILURE);
		}
		if(bind(sockfd, p -> ai_addr, p -> ai_addrlen) == -1)
		{
			printf("Error al enlazar el socket\n");
			printf("Compruebe que tiene permisos de super usuario\n");
			close(sockfd);
			continue;
		}//prepara el socket
		break;
	}//explora para preparar el socket
	if(listen(sockfd, 5) == -1)
	{
		printf("Error al escuchar al socket\n");
		exit(EXIT_FAILURE);
	}//escucha el sokcet
	while(1)
	{
		addr_size = sizeof(their_addr);
		if((newfd = accept(sockfd, (struct sockaddr *) &their_addr, &addr_size)) == -1)
		{
			printf("Error al aceptar al cliente\n");
			exit(EXIT_FAILURE);
		}//acepta una conexión entrante
		for(p = res; p != NULL; p = p -> ai_next)
		{
			void *addr;
			if(p -> ai_family == AF_INET)
			{
				struct sockaddr_in *ip;
				ip = (struct sockaddr_in *) p -> ai_addr;
				addr = &(ip -> sin_addr);
			}
			if(p -> ai_family == AF_INET6)
			{
				struct sockaddr_in6 *ip;
				ip = (struct sockaddr_in6 *) p -> ai_addr;
				addr = &(ip -> sin6_addr);
			}//configura la conexión entrante
			inet_ntop(p -> ai_family, addr, ip, sizeof(ip));
			printf("Ateniendo al cliente %s\n", ip);
		}//atiende conexiones entrantes
		connection(newfd);
	}//atiende a los clientes
	freeaddrinfo(res);
	close(newfd);
	close(sockfd);
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

// procesa el archivo recibido
int recv_new(int fd, char *buffer)
{
	char *p = buffer;
	unsigned short int EOM_match = 0;
	unsigned short int matches = 0;
	while(recv(fd, p, 1, 0) != 0)
	{
		if(*p == EOM[EOM_match])
		{
			EOM_match++;
			if(EOM_match == EOM_SIZE)
			{
				matches++;
				if(matches == 2)
				{
					*(p +1 -EOM_SIZE) = '\0';
					return (strlen(buffer));
				}//si se encuentra 2 veces el final de línea
			}//si se ha recibido el fin de línea
		}
		else
		{
			EOM_match = 0;
		}//trata de cazar el fin de línea
		p++;
	}//lee el mensaje
	return 0;
}//recv_new

// inicia una conexión con el cliente
void connection(int fd)
{
	char *request, resource[512], *ptr, buffer[512], *bufferc;
	int fd1, length, req_type = 0;
	if(recv_new(fd, buffer) == 0)
	{
		send_new(fd, "HTTP/1.0 500 Internal error\r\n");
		send_new(fd, "server: NACHINTOCH-HTTPSERVER\r\n\r\n");
		close(fd);
		return;
	}//toma el mensaje
	//printf("buffer=%s\n", buffer);
	request = malloc(sizeof(char *) *strlen(buffer));
	bufferc = malloc(sizeof(char *) *strlen(buffer));
	memset(bufferc, 0, strlen(buffer));
	strcat(bufferc, buffer);
	request = strtok(bufferc, "\r\n");
	ptr = strstr(request, "HTTP/");
	if(ptr == NULL)
	{
		send_new(fd, "HTTP/1.0 400 Invalid protocol\r\n");
		send_new(fd, "server: NACHINTOCH-HTTPSERVER\r\n\r\n");
		close(fd);
		return;
	}
	else
	{
		*ptr = 0;
		ptr = NULL;
		unsigned short int aux = 1;
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
		if(req_type != 2)
		{
			ptr = strtok(ptr, " ");
			if(ptr[strlen(ptr) -1] == '/')
			{
				strcat(ptr, "index.html");
			}//si se solicitó el archivo raíz
			strcpy(resource, WEBROOT);
			strcat(resource, ptr);
			fd1 = open(resource, O_RDONLY, 0);
			if(fd1 == -1)
			{
				send_new(fd, "HTTP/1.0 404 Not found\r\n");
				send_new(fd, "Server: NACHINTOCH-HTTPSERVER\r\n\r\n");
				send_new(fd, "404 - File not found\r\n\r\n");
				aux = 0;
				close(fd);
				return;
			}
			else
			{
				aux = 1;
			}//recupera el archivo e indica posibles errores
		}
		else
		{
			send_new(fd, "HTTP/1.0 200 OK\r\n");
			send_new(fd, "server: NACHINTOCH-HTTPSERVER\r\n\r\n");
		}//reucpera el archivo o crea un header
		if(!aux)
		{
			send_new(fd, "HTTP/1.0 500 Internal error\r\n");
			send_new(fd, "server: NACHINTOCH-HTTPSERVER\r\n\r\n");
			close(fd);
			return;
		}//envía error en su caso
		switch(req_type) {
		case 1 :
			
			if((length = get_file_size(fd1)) == -1)
			{
				send_new(fd, "HTTP/1.0 500 Internal error\r\n");
				send_new(fd, "server: NACHINTOCH-HTTPSERVER\r\n\r\n");
				close(fd);
				return;
			}
			if((ptr = (char *) malloc(length)) == NULL)
			{
				send_new(fd, "HTTP/1.0 500 Internal error\r\n");
				send_new(fd, "server: NACHINTOCH-HTTPSERVER\r\n\r\n");
				close(fd);
				return;
			}//recupera el archivo y asigna espacio para la transmisión
			read(fd1, ptr, length);
			send_new(fd, "HTTP/1.0 200 OK\r\n");
			send_new(fd, "server: NACHINTOCH-HTTPSERVER\r\n\r\n");
			if(send(fd, ptr, length, 0) == -1)
			{
				printf("Error al enviar archivo al cliente\n");
			}//comrpueba que la transmisión del arhivo haya sido exitosa
			free(ptr);
			break;
		case 2 :
			//solicito HEAD, pero el header ya ha sido enviado. No queda más que hacer
			break;
		case 3 :
			bufferc = strstr(buffer, EOM);
			bufferc += 5;
			printf("Se recibieron los siguientes datos en POST:\n%s\n",
				bufferc);
			if((length = get_file_size(fd1)) == -1)
			{
				send_new(fd, "HTTP/1.0 500 Internal error\r\n");
				send_new(fd, "server: NACHINTOCH-HTTPSERVER\r\n\r\n");
				close(fd);
				return;
			}
			if((ptr = (char *) malloc(length)) == NULL)
			{
				send_new(fd, "HTTP/1.0 500 Internal error\r\n");
				send_new(fd, "server: NACHINTOCH-HTTPSERVER\r\n\r\n");
				close(fd);
				return;
			}//recupera el archivo y asigna espacio para la transmisión
			read(fd1, ptr, length);
			send_new(fd, "HTTP/1.0 200 OK\r\n");
			send_new(fd, "server: NACHINTOCH-HTTPSERVER\r\n\r\n");
			if(send(fd, ptr, length, 0) == -1)
			{
				printf("Error al enviar archivo al cliente\n");
			}//comrpueba que la transmisión del arhivo haya sido exitosa
			free(ptr);
			break;
		default :
			send_new(fd, "HTTP/1.0 501 Not implemented\r\n");
			send_new(fd, "server: NACHINTOCH-HTTPSERVER\r\n\r\n");
			close(fd);
			return;
			break;
		}//actua dependiendo el méotodo
		close(fd);
	}//recibe una petición
	shutdown(fd, SHUT_RDWR);
}//conection

