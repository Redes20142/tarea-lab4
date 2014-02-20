#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#define USERAGENT "NACHINTOCH-HTTPCLIENT 1.0"

/*
 * Cliente de HTTP/1.0
 *
 * Autor: Manuel Ignacio Castillo López; 3-0801743-8.
 * manuel_castillo_cc@ciencias.unam.mx
 * Autor: Mijail Gutiérrez Valdéz; 3-423250-3.
 * mij000@ciencias.unam.mx
 * Version 1, febrero 2014
 */

//prototipos de funciones
int create_socket();
char *get_ip(char *);
char *build_query(char *host, char *page);

/*
 * Ejecuta el cliente de HTTP haciendo un request a la página dada como
 * argumento en la línea de comandos.
 */
int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		printf("Invoque el programa pasando como argumentos el host deseado ");
		printf("y la p\u00E1gina que desea consultar del mismo.\n");
		printf("Por ejemplo: httpclient web.fciencias.unam.mx /acceder\n");
		printf("Puede omitir la p\u00E1gina.\n");
		exit(EXIT_FAILURE);
	}//comprueba que se le hayan dado argumentos de consola
	struct sockaddr_in *remote;
	int sock, n;
	char *ip, *get;
	char buffer[BUFSIZ +1];
	char *host = argv[1];
	char *page;
	if(argc > 2)
	{
		page = argv[2];
	}
	else
	{
		page = "/";
	}//obtiene la página que se desea visitar
	sock = create_socket();
	ip = get_ip(host);
	remote = (struct sockaddr *) malloc(sizeof(struct sockaddr_in));
	remote -> sin_family = AF_INET;
	n = inet_pton(AF_INET, ip, (void *) (&(remote -> sin_addr.s_addr)));
	if(n < 0)
	{
		printf("Ocurri\u00F3 un problema al obtener el servicio remoto\n");
		exit(EXIT_FAILURE);
	}
	else if(n == 0)
	{
		printf("La IP obtenida no es v\u00E1lida\n");
		exit(EXIT_FAILURE);
	}//resuelve la conexión
	remote -> sin_port = htons(80);
	if(connect(sock, (struct sockaddr *) remote, sizeof(struct sockaddr)) < 0)
	{
		perror("Error al conectar");
		exit(EXIT_FAILURE);
	}//conecta
	get = build_query(host, page);
	int sent = 0;
	while(sent < strlen(get))
	{
		n = send(sock, get +sent, strlen(get) -sent, 0);
		if(n == -1)
		{
			perror("No es posible enviar una petici\u00F3n al server");
			exit(EXIT_FAILURE);
		}//si ocurre un error de salida
		sent += n;
	}//envía la petición al server
	memset(buffer, 0, sizeof(buffer));
	unsigned int data = 0;
	char *content;
	while((n = recv(sock, buffer, BUFSIZ, 0)) > 0)
	{
		if(data == 0)
		{
			content = strstr(buffer, "\r\n\r\n");
			if(content != NULL)
			{
				data = 1;
				content += 4;
			}//indicamos que se ha obtenido el inicio de los datos de entrada
		}
		else
		{
			content = buffer;
		}//si se ha obtenido el resto de la página
		if(data)
		{
			printf(content);
		}//muestra el documento obtenido
		memset(buffer, 0, n);
	}//recupera los datos recibidos por el server
	if(n < 0)
	{
		perror("Error al recuperar los datos");
	}//comprueba que no hayan ocurrido errores
	free(get);
	free(remote);
	free(ip);
	close(sock);
	return 0;
}//main

// crea un socket de TCP
int create_socket()
{
	int sock;
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		perror("Error al crear el socket");
		exit(EXIT_FAILURE);
	}//comprueba que se haya creado el socket
	return sock;
}//create_socket

// obtiene una IP del nombre del servicio dado
char *get_ip(char *host)
{
	struct hostent *hent;
	int iplen = 15;
	char *ip = (char *) malloc(iplen +1);
	memset(ip, 0, iplen +1);
	if((hent = gethostbyname(host)) == NULL)
	{
		herror("No es posible obtener la IP");
		exit(EXIT_FAILURE);
	} if(inet_ntop(AF_INET, (void *) hent -> h_addr_list[0], ip, iplen) == NULL)
	{
		perror("No es posible resolver el nombre de servicio dado");
		exit(EXIT_FAILURE);
	}//obitne la IP del servicio
	return ip;
}//get_ip

//construye una consulta al server
char *build_query(char *host, char *page)
{
	char *query;
	char *getpage = page;
	char *tpl = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
	if(getpage[0] == '/')
	{
		getpage = getpage +1;
	}//prepara el encabezado
	query = (char *) malloc(strlen(host) +strlen(getpage) +strlen(USERAGENT)
		+ strlen(tpl) -5);
	sprintf(query, tpl, getpage, host, USERAGENT);
	return query;
}//build_query

