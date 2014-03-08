#ifndef __HTTPCONF__
#define __HTTPCONF__

#define MAX_BUF 1024
#define PENDING_CONN 64

#include "log.h"

/* various config element types parsed from config file*/
enum cfg_elements_t {
	LISTEN_PORT, DEFAULT
};
struct valid_file_types {
	char extension[MAX_BUF]; /*extension of file*/
	char type[MAX_BUF]; /*type of the file contents*/
};
struct directory_index_file {
	char filename[MAX_BUF]; /*default file name used when only directory is
	 specified*/
};

struct map_item {
	char uri[MAX_BUF];
	void (*f)(int sockfd);
};

struct http_server_config {
	int listen_port;
	void (*defhand)(int sockfd);
	struct map_item items[MAX_BUF]; /*supported file types */
	int items_cnt;
};

/*Not used currently. Convenience data structure in case threads need to be used*/
struct http_server_data {
	struct http_server_config *cfg;
	int sockfd;
};

/*Function declarations */

int cfg_reader(void *c, char *line);
void http_server(struct http_server_config *cfg, int sockfd);

int valid_method_string(char **request, char *request_method);
int valid_version(char **request, struct http_server_config *cfg, char *version);

int valid_uri(char **request, struct http_server_config *cfg, char *uri);
int valid_filetype(char **request, struct http_server_config *cfg, char *uri);
int connection_handler(struct http_server_config *cfg);
#endif
