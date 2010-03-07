/*
 * $Id: peer-done.c,v 1.2 2010/03/07 23:20:18 bruno Exp $
 *
 * @COPYRIGHT@
 * @COPYRIGHT@
 *
 * $Log: peer-done.c,v $
 * Revision 1.2  2010/03/07 23:20:18  bruno
 * progress. can now run this as a non-root user -- should be able to run tests
 * on triton.
 *
 * Revision 1.1  2010/03/02 22:28:06  bruno
 * renamed stop-server to peer-done
 *
 * Revision 1.1  2010/02/24 20:27:32  bruno
 * moved new tracker from base roll to kernel roll
 *
 * Revision 1.1  2009/09/25 21:02:04  bruno
 * got prediction code in
 *
 */

#include <stdio.h>
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
extern int init_tracker_comm(int);
extern int send_done(int, in_addr_t *);

int
main()
{
	uint16_t	num_trackers;
	in_addr_t	trackers[MAX_TRACKERS];
	uint16_t	maxpeers;
	uint16_t	num_pkg_servers;
	in_addr_t	pkg_servers[MAX_PKG_SERVERS];
	int		i;
	int		sockfd;
	char		trackers_url[256];
	char		pkg_servers_url[256];
	FILE		*file;

	if ((file = fopen("/tmp/rocks.conf", "r")) == NULL) {
		fprintf(stderr, "main:fopen\n");
		return(-1);
	}

	fscanf(file, "var.trackers = \"%255s\"", trackers_url);
	fseek(file, 0, SEEK_SET);
	fscanf(file, "var.pkgservers = \"%255s\"", pkg_servers_url);

	fclose(file);

	fprintf(stderr, "main:trackers_url (%s)\n", trackers_url);
	fprintf(stderr, "main:pkg_servers_url (%s)\n", pkg_servers_url);
	
	if (init(&num_trackers, trackers_url, trackers, &maxpeers,
			pkg_servers_url, &num_pkg_servers, pkg_servers) != 0) {
		fprintf(stderr, "main:init failed\n");
		return(-1);
	}

	if ((sockfd = init_tracker_comm(0)) < 0) {
		fprintf(stderr, "main:init_tracker_comm failed\n");
		return(-1);
	}

	for (i = 0 ; i < num_trackers; ++i) {
		send_done(sockfd, &trackers[i]);
	}

	return(0);
}

