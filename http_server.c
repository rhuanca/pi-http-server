/*
 * File Name : http_server.c 
 * Author : Gaurav Tungatkar
 * Creation Date : 17-01-2011
 * Description :
 *
 */

#include <fileparser.h>
#include <httpconf.h>
#include <log.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tokenize.h>
#include <unistd.h>

#include "pi_server.h"

#define BUFFSIZE (4*1024)
#define MAX_FILENAME 512
#define HTTP_HDR_SIZE 512
#define HTTP_URI_SIZE 1024
#define HTTP_STATUS_SIZE 1024
#define HTTP_GET 11
#define SP 0x20 
#define CRLF "\r\n"


void hello_handler(char *buffer, int *reponse_len) {
	char *resp = "{\"Response\":\"Hello\"}";
	strcpy(buffer, resp);
	*reponse_len = strlen(resp);
}

int main(int argc, char *argv[]) {
	struct pi_server_config cfg;
	pi_init_config(8080, &cfg);
	pi_map_handler(&cfg, "/hello", &hello_handler);
	if (pi_start_server(&cfg) == -1) {
		printf("Unable to start REST server.\n");
	}
	return 0;
}

