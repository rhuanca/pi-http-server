/*
 * File Name : http_config.c 
 * Author : Gaurav Tungatkar
 * Creation Date : 14-01-2011
 * Description :
 *
 */
/*Read http server configuration from the config file and store it in memory
 */

#include <assert.h>
//#include <fcntl.h>
#include <httpconf.h>
#include <log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/stat.h>
//#include <sys/types.h>
#include <tokenize.h>
#include <unistd.h>
//#include "fileparser.h"

int cfg_reader(void *c, char *line) {
	assert(c);
	assert(line);
	char token[MAX_TOKEN_SIZE];
	int next_token = DEFAULT;
	struct http_server_config *cfg = (struct http_server_config *) c;
	int indx = 0;
	int config_cnt = 0;
	/* From the given config file format, we assume we have 4 distinct
	 * config objects - port, DocumentRoot, DirectoryIndex, supported files
	 * and all are mandatory. If one of them is missing, we should give an
	 * error and return.
	 * This function gets one line of the config file at a time, which is
	 * further parses.
	 */

	while (tokenize(&line, token) > 0) {
		if (token[0] == '#') {
			//This line must be a Comment. Ignore and return
			return 0;
		} else if (strcmp(token, "Listen") == 0) {
			next_token = LISTEN_PORT;
			config_cnt++;

		} else //list of valid files
		{
			int len;
			char *p;
			len = strlen(token);
			switch (next_token) {
			case LISTEN_PORT:
				cfg->listen_port = strtol(token, &p, 10);
				if (cfg->listen_port > 65535) {
					LOG(stdout, "Port value invalid\n");
					return -1;
				}
				config_cnt++;
				next_token = DEFAULT;
				break;
			default:
				LOG(stdout, "Error in config file.Exiting...\n")
				;
				return -1;

			}

		}

	}
	/* config_cnt counts how many config types or values we have seen.
	 * if it is 1, it means we just got the type and not the value.*/
	if (config_cnt == 1) {
		LOG(stdout, "Error in config file\n");
		return -1;

	}
	return 0;
}
