/*
 * server.h
 *
 *  Created on: Dec 31, 2013
 *      Author: renan
 */

#ifndef SERVER_H_
#define SERVER_H_

#define MAX_REQUEST_SIZE_URI 1024

#define HTTP_GET 11
#define PENDING_CONN 64
#define MAX_HANDLERS 20

struct pi_request_handler {
	char uri[MAX_REQUEST_SIZE_URI];
	void (*handler)(char *buffer, int *len);
};

struct pi_server_config {
	int listen_port;
	struct pi_request_handler hanlders[MAX_HANDLERS];
	int hanlders_cnt;
};

void pi_init_config(int port, struct pi_server_config *cfg);
int pi_start_server(struct pi_server_config *cfg);
void pi_map_handler(struct pi_server_config *cfg, char *uri,
		void (*handler)(char *buffer, int *len));

void _pi_handle_request(struct pi_server_config *cfg, int client_fd);
int _valid_method_string(char **request, char *request_method);
int _valid_uri(char **request, char *uri);
int _valid_version(char **request, char *version);
void _default_handler(char *buffer, int *reponse_len);

#endif /* SERVER_H_ */
