/*
 * $Id: unregister-file.c,v 1.2 2010/03/07 23:20:18 bruno Exp $
 *
 * @COPYRIGHT@
 * @COPYRIGHT@
 *
 * $Log: unregister-file.c,v $
 * Revision 1.2  2010/03/07 23:20:18  bruno
 * progress. can now run this as a non-root user -- should be able to run tests
 * on triton.
 *
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

extern int init(uint16_t *, char *, in_addr_t *, uint16_t *, char *, uint16_t *,
	in_addr_t *);
extern int unregister_hash(int, in_addr_t *, uint32_t, tracker_info_t *);
extern void logmsg(const char *, ...);

int	status = HTTP_OK;

int
getargs(char *forminfo, char *filename, char *trackers_url,
	char *pkg_servers_url)
{
	char	*ptr;

	/*
	 * help out sscanf by putting in a blank for '&'
	 */
	while (1) {
		if ((ptr = strchr(forminfo, '&')) == NULL) {
			break;
		}

		*ptr = ' ';
	}

	if (sscanf(forminfo, "filename=%4095s trackers=%256s pkgservers=%256s",
			filename, trackers_url, pkg_servers_url) != 3) {
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
	in_addr_t	trackers[MAX_TRACKERS];
	uint16_t	maxpeers;
	uint16_t	num_pkg_servers;
	in_addr_t	pkg_servers[MAX_PKG_SERVERS];
	tracker_info_t	info[1];
	int		sockfd;
	int		i;
	char		trackers_url[256];
	char		pkg_servers_url[256];
	FILE		*file;

	hash = hashit(filename);

	if ((file = fopen("/tmp/rocks.conf", "r")) == NULL) {
		logmsg("unregister_file:fopen failed\n");
		return(-1);
	}

	fscanf(file, "var.trackers = \"%255s\"", trackers_url);
	fseek(file, 0, SEEK_SET);
	fscanf(file, "var.pkgservers = \"%255s\"", pkg_servers_url);

	fclose(file);
	
	if (init(&num_trackers, trackers_url, trackers, &maxpeers,
			pkg_servers_url, &num_pkg_servers, pkg_servers) != 0) {
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

	return(0);
}

int
main()
{
	char	*forminfo;
	char	filename[PATH_MAX];
	char	trackers_url[256];
	char	pkg_servers_url[256];

	bzero(filename, sizeof(filename));
	bzero(trackers_url, sizeof(trackers_url));
	bzero(pkg_servers_url, sizeof(pkg_servers_url));

	if ((forminfo = getenv("QUERY_STRING")) == NULL) {
		fprintf(stderr, "No QUERY_STRING\n");
		return(0);
	}

	if (getargs(forminfo, filename, trackers_url, pkg_servers_url) != 0) {
		fprintf(stderr, "getargs():failed\n");
		return(0);
	}

	unregister_file(filename);

	return(0);
}

