#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#define USERAGENT "NACHINTOCH-HTTPCLIENT 1.0"
#define GET "GET"
#define HEAD "HEAD"
#define POST "POST"

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
void create_socket();
void get_ip(char *);
char *build_query(char *, char *, char *, char *);
void die();

// variables globales
int sock;
char *ip, *get;
struct sockaddr_in *remote;

/*
 * Ejecuta el cliente de HTTP haciendo un request a la página dada como
 * argumento en la línea de comandos.
 */
int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		printf("Invoque el programa pasando como argumentos el m\u00E9todo, ");
		printf("host deseado, la p\u00E1gina que desea consultar del ");
		printf("mismo y un posible query para pasar por GET.\nPor ejemplo: ");
		printf("client GET www.google.com / search=nachintoch\nPuede omitir ");
		printf("la p\u00E1gina y el query.\n");
		exit(EXIT_FAILURE);
	}//comprueba que se le hayan dado argumentos de consola
	if(strcmp(argv[1], GET) != 0 && strcmp(argv[1], HEAD) != 0 &&
		strcmp(argv[1], POST) != 0)
	{
		printf("\"%s\" no es un m\u00E9todo de HTTP/1.0 reconocido\n",
			argv[1]);
		exit(EXIT_FAILURE);
	}//comprueba que se le haya dado un método de HTTP/1.0 válido
	char *getq;
	if(argc > 4 && strcmp(argv[1], GET) != 0)
	{
		printf("Un query s\u00F3lo deber\u00EDa usarse con el m\u00E9todo ");
		printf("GET");
		exit(EXIT_FAILURE);
	}
	else if(argc > 4)
	{
		getq = malloc(sizeof(char *) *(strlen(argv[4]) +1));
		memset(getq, 0, strlen(argv[4]) +1);
		strcat(getq, "?");
		strcat(getq, argv[4]);
	}
	else
	{
		getq = "";
	}//si se pasó un méotodo que no es get con un query
	int n;
	char buffer[BUFSIZ +1];
	char *host = argv[2];
	char *page;
	if(argc > 3)
	{
		page = argv[3];
	}
	else
	{
		page = "/";
	}//obtiene la página que se desea visitar
	create_socket();
	get_ip(host);
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
	get = build_query(argv[1], host, page, getq);
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
	char *head;
	unsigned short int i;
	while((n = recv(sock, buffer, BUFSIZ, 0)) > 0)
	{
		content = buffer;
		if(!data)
		{
			head = malloc(sizeof(char *) *strlen(buffer));
			for(i = 0; buffer[i] != '\r' || buffer[i +1] != '\n' ||
				buffer[i +2] != '\r' || buffer[i +3] != '\n'; i++)
			{
				head[i] = buffer[i];
			}//copia el encabezado del flujo
			printf("Encabezado:\n\n%s\n\n", head);
			content = strstr(buffer, "\r\n\r\n");
			if(content)
			{
				content += 4;
			}//si se encontró la subcadena
			printf("Mensaje:\n\n");
			data = 1;
		}//si no se ha recuperado el encabezado
		fprintf(stdout, content);
		memset(buffer, 0, n);
	}//recupera los datos recibidos por el server
	if(head[0] != 'H' || head[1] != 'T' || head[2] != 'T' || head[3] != 'P' ||
		head[4] != '/')
	{
		printf("No se reconoce el protoclo de respuesta\n");
		die();
	} if((head[5] -48 > 1) && (head[6] != '.') && (head[7] -48 != 0
		|| head[7] -48 != 9))
	{
		printf("No se reconoce la versi\u00F3n de HTTP de respuesta\n");
		die();
	}
	char *args[5];
	switch(head[9]) {
	case '1' :
		//informativo, se ignora
		break;
	case '2' :
		//conección correcta, se ignora
		break;
	case '3' :
		//redireccionamiento. Tomamos el Location:
		content = strstr(head, "Location: ");
		if(content)
		{
			content += 10;
			printf("content=%s\n", content);
			args[0] = "./bin/client";
			args[1] = "GET";
			if(tolower(content[0]) != 'h' || tolower(content[1]) != 't' ||
				tolower(content[2]) != 't' || tolower(content[3]) != 'p' ||
				tolower(content[4]) != ':' || tolower(content[5]) != '/' ||
				tolower(content[6]) != '/')
			{
				printf("No se reconoce el protocolo de redireccionamiento.\n");
				die();
			}//comprueba que el potocolo de redireccionamiento
			char *newq = malloc(sizeof(char *) *strlen(content));
			newq = strtok(content, "\n");
			strtok(newq, "//");
			newq = strtok(newq, "/");
			args[2] = malloc(sizeof(char *) *(strlen(newq)));
			memset(args[2], 0, strlen(newq));
			strcat(args[2], newq);
			newq = strtok(newq, "\n");
			args[3] = malloc(sizeof(char *) *(strlen(newq) +1));
			memset(args[3], 0, strlen(newq));
			strcat(args[3], "/");
			strcat(args[3], newq);
			if(argc == 5)
			{
				args[4] = argv[4];
			}
			else
			{
				args[4] = "";
			}//toma el query
			printf("args[0]=%s\n", args[0]);
			printf("args[1]=%s\n", args[1]);
			printf("args[2]=%s\n", args[2]);
			printf("args[3]=%s\n", args[3]);
			printf("args[4]=%s\n", args[4]);
			execvp(args[0], args);
		}
		else
		{
			printf("El servidor indic\u00F3 un redireccionamiento, pero no ");
			printf("especific\u00F3 una direcci\u00F3n a redireccionar\n");
			die();
		}//comprueba que se haya porporcionado un
		break;
	case '4' :
		content = strtok(content, "\n");
		printf("Error 4xx: \"%s\"\n", content);
		die();
		break;
	case '5' :
		content = strtok(content, "\n");
		printf("Error 5xx: \"%s\"\n", content);
		die();
		break;
	default :
		printf("No se reconoce el c\u00F3digo de respuesta.\n");
		die();
	}//dependiendo el código de respuesta
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
void create_socket()
{
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		perror("Error al crear el socket");
		exit(EXIT_FAILURE);
	}//comprueba que se haya creado el socket
}//create_socket

// obtiene una IP del nombre del servicio dado
void get_ip(char *host)
{
	struct hostent *hent;
	int iplen = 15;
	ip = (char *) malloc(iplen +1);
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
}//get_ip

//construye una consulta al server
char *build_query(char *method, char *host, char *page, char *getquery)
{
	char *query;
	char *getpage = page;
	char *tpl = "%s /%s%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
	if(getpage[0] == '/')
	{
		getpage = getpage +1;
	}//prepara el encabezado
	query = (char *) malloc(strlen(method) +strlen(host) +strlen(getpage)
		+strlen(getquery) +strlen(USERAGENT) + strlen(tpl) -6);
	sprintf(query, tpl, method, getpage, getquery, host, USERAGENT);
	return query;
}//build_query

//termina el programa en stado abnormal
void die()
{
	free(get);
	free(remote);
	free(ip);
	close(sock);
	exit(EXIT_FAILURE);
}//die

