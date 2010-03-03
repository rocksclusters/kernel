/*
 * $Id: unregister-file.c,v 1.1 2010/03/03 19:21:03 bruno Exp $
 *
 * @COPYRIGHT@
 * @COPYRIGHT@
 *
 * $Log: unregister-file.c,v $
 * Revision 1.1  2010/03/03 19:21:03  bruno
 * add code to 'unregister' a file and clean up the hash table when there are
 * no more peers for a hash.
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <curl/curl.h>
#include <httpd/httpd.h>
#include <netinet/in.h>
#include "tracker.h"
#include <sys/socket.h>
#include <arpa/inet.h>

extern int init(uint16_t *, in_addr_t **, uint16_t *, uint16_t *, in_addr_t **);
extern int unregister_hash(int, in_addr_t *, uint32_t, tracker_info_t *);
extern void logmsg(const char *, ...);

int	status = HTTP_OK;

int
getargs(char *forminfo, char *filename, char *serverip)
{
	char	*ptr;

	/*
	 * help out sscanf by putting in a blank for '&'
	 */
	if ((ptr = strchr(forminfo, '&')) == NULL) {
		/*
		 * XXX - log an error
		 */
		return(-1);
	}

	*ptr = ' ';

	if (sscanf(forminfo, "filename=%4095s serverip=%15s", filename,
			serverip) != 2) {
		/*
		 * XXX - log an error
		 */
		return(-1);
	}

	return(0);
}

int
unregister_file(char *filename)
{
	uint64_t	hash;
	uint16_t	num_trackers;
	in_addr_t	*trackers;
	uint16_t	maxpeers;
	uint16_t	num_pkg_servers;
	in_addr_t	*pkg_servers;
	tracker_info_t	info[1];
	int		sockfd;
	int		i;

	hash = hashit(filename);

	if (init(&num_trackers, &trackers, &maxpeers, &num_pkg_servers,
			&pkg_servers) != 0) {
		logmsg("trackfile:init failed\n");
		return(-1);
	}

	if ((sockfd = init_tracker_comm(0)) < 0) {
		logmsg("trackfile:init_tracker_comm failed\n");
		return(-1);
	}

	bzero(info, sizeof(info));
	info[0].hash = hash;
	info[0].numpeers = 0;

	for (i = 0 ; i < num_trackers; ++i) {
		unregister_hash(sockfd, &trackers[i], 1, info);
	}

	/*
	 * init() mallocs trackers
	 */
	free(trackers);

	return(0);
}

int
main()
{
	char	*forminfo;
	char	filename[PATH_MAX];
	char	serverip[16];

	bzero(filename, sizeof(filename));
	bzero(serverip, sizeof(serverip));

	if ((forminfo = getenv("QUERY_STRING")) == NULL) {
		fprintf(stderr, "No QUERY_STRING\n");
		return(0);
	}

	if (getargs(forminfo, filename, serverip) != 0) {
		fprintf(stderr, "getargs():failed\n");
		return(0);
	}

	unregister_file(filename);

	return(0);
}

