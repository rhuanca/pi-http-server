/*
 * Server.c
 *
 *  Created on: Dec 31, 2013
 *      Author: renan
 */

#include "pi_server.h"

#include <asm-generic/socket.h>
#include <httpconf.h>
#include <log.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <tokenize.h>
#include <unistd.h>

#define BUFFSIZE (4*1024)
#define HTTP_STATUS_SIZE 1024
#define HTTP_HDR_SIZE 512
#define HTTP_URI_SIZE 1024
#define SP 0x20

void pi_init_config(int port, struct pi_server_config *cfg) {
	cfg->listen_port = port;
	cfg->hanlders_cnt = 0;
	printf("Config initialized.\n");
}

int pi_start_server(struct pi_server_config *cfg) {
	int server_fd;
	int set = 1;
	struct sockaddr_in listener_addr;
	int client_fd;
	struct sockaddr_in client_addr;
	socklen_t addr_len = 0;
	pid_t pid;

	printf("Starting REST Server.\n");
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		LOG(stdout, "socket() failure\n");
		return -1;
	}

	printf(">>>> listen_fd = %i\n", server_fd);

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(int))
			== -1) {
		LOG(stdout, "setsockopt() failed");
		return -1;
	}

	bzero(&listener_addr, sizeof(listener_addr));
	listener_addr.sin_family = AF_INET;
	listener_addr.sin_port = htons(cfg->listen_port);
	listener_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(listener_addr.sin_zero, '\0', sizeof(listener_addr.sin_zero));

	printf(">>>> cfg->listen_port = %i\n", cfg->listen_port);

	if (bind(server_fd, (struct sockaddr*) &listener_addr, sizeof listener_addr)
			== -1) {
		printf("bind() failed\n");
		return -1;
	}

	if (listen(server_fd, PENDING_CONN) == -1) {
		LOG(stdout, "listen() failed\n");
		return -1;
	}

	printf("REST server listening on port:%d\n", cfg->listen_port);

	while (1) {
		client_fd = accept(server_fd, (struct sockaddr*) &client_addr,
				&addr_len);

		if (client_fd == -1) {
			//log
			printf("accept() failed\n");
		}
		printf("new connection accepted\n");
		//fork a new process to handle this request
		if ((pid = fork()) == -1) {
			//LOG Error
			printf("Error in fork\n");
		} else if (pid == 0) {

			/*This is the child process. This will service the
			 *request while parent goes back to listening.*/
			printf("Servicing request started\n");
			_pi_handle_request(cfg, client_fd);
			printf("Servicing request finished\n");
		} else {
			close(client_fd);
		}
	}
	return 0;
}

int _valid_method_string(char **request, char *request_method) {
	/*only GET method supported */
	if ((tokenize(request, request_method) != 3)
			|| (strcmp(request_method, "GET") != 0)) {
		LOG(stdout, "Invalid method\n");
		return -1;
	} else {
		return HTTP_GET;
	}
}

int _valid_uri(char **request, char *uri) {

	if (*(*(request) + 1) == SP) {
		printf("Invalid URI\n");
		return -1;
	}

	if ((tokenize(request, uri) <= 0)) {
		printf("Invalid URI\n");
		return -1;
	} else {
		//cannot refer to the parent directory
		if (uri[0] == '.' && uri[1] == '.') {
			printf("Invalid URI\n");
			return -1;
		}
		// handle '/' or other request when need.
		//if just '/' , append the default index file name
	}
	return 0;
}

int _valid_version(char **request, char *version) {
	/* HTTP versions 1.0 and 1.1 messages are accepted
	 */
	if ((tokenize(request, version) <= 0)
			|| ((strcmp(version, "HTTP/1.1") != 0)
					&& (strcmp(version, "HTTP/1.0") != 0))) {
		printf("Version not supported\n");
		return -1;
	} else {
		return 0;
	}
}

void _default_handler(char *buffer, int *reponse_len) {
	char *resp = "{\"response\":\"default_handler\"}";
	strcpy(buffer, resp);
	*reponse_len = strlen(resp);
}

void pi_map_handler(struct pi_server_config *cfg, char *uri,
		void (*handler)(char *buffer, int *len)) {
	printf("mapped resource %s\n", uri);
	struct pi_request_handler request_handler;
	// memset(&request_handler, 0, sizeof(request_handler));
	strcpy(request_handler.uri, uri);
	request_handler.handler = handler;
	cfg->hanlders[cfg->hanlders_cnt] = request_handler;
	cfg->hanlders_cnt++;
}

void _pi_handle_request(struct pi_server_config *cfg, int client_fd) {
	char request[BUFFSIZE + 1];

	// uri
	char uri[HTTP_URI_SIZE];
	char status[HTTP_STATUS_SIZE];
	char header[HTTP_HDR_SIZE];

	// version
	char version[10];
	// method
	char *requestptr = request;
	int method;
	char request_method[5];
	char buffer[BUFFSIZE];
	void (*handler)(char *buffer, int *len) = NULL;
	ssize_t numbytes;
	int errflag = 0;
	int reponse_len;
	int i;

	printf("Handing request\n");

	if ((numbytes = read(client_fd, (void *) request, BUFFSIZE)) <= 0) {
		printf("read from socket failed\n");
		return;
	}

	if ((method = _valid_method_string(&requestptr, request_method)) == -1) {
		printf("HTTP/1.0 400 Bad Request: Invalid Method: %s\n",
				request_method);
		errflag = 1;
	}

	if (!errflag && (method == HTTP_GET)) {
		if (_valid_uri(&requestptr, uri) == -1) {
			printf("HTTP/1.0 400 Bad Request: Invalid URI: %s\r\n", uri);
			errflag = 1;
		}
	}

	if (!errflag) {
		if (_valid_version(&requestptr, version) == -1) {
			printf("HTTP/1.0 400 Bad Request: Invalid HTTP-Version: %s\r\n",
					version);
			errflag = 1;
		}
	}

	if (!errflag) {
		handler = &_default_handler;
		for (i = 0; i < cfg->hanlders_cnt; i++) {
			if (strcmp(cfg->hanlders[i].uri, uri) == 0) {
				handler = cfg->hanlders[i].handler;
				break;
			}
		}

		handler(buffer, &reponse_len);

		snprintf(status, HTTP_STATUS_SIZE, "HTTP/1.0 200 Document Follows\r\n");
		snprintf(header, HTTP_HDR_SIZE,
				"Content-Type: %s\r\nAccess-Control-Allow-Origin:*\r\nContent-Length: %d\r\n\n", "application/json",
				reponse_len);

		write(client_fd, status, strlen(status));
		write(client_fd, header, strlen(header));
		write(client_fd, buffer, reponse_len);
	}
}

