#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#include "tracker.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void
logmsg(const char *fmt, ...)
{
	FILE	*file;
	va_list argptr;

	if ((file = fopen("/tmp/tracker-client.debug", "a+")) != NULL) {
		va_start(argptr, fmt);
		vfprintf(file, fmt, argptr);
		va_end(argptr);

		fclose(file);
	}

	return;
}

uint64_t
hashit(char *ptr)
{
	uint64_t	hash = 0;
	int		c;

	/*
	 * SDBM hash function
	 */

	while ((c = *ptr++) != '\0') {
		hash = c + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}

void
dumpbuf(char *buf, int len)
{
	int	i;

	for (i = 0; i < len; ++i) {
		fprintf(stderr, "%02x ", (unsigned char)buf[i]);
	}
	fprintf(stderr, "\n");
}

int
tracker_send(int sockfd, void *buf, size_t len, struct sockaddr *to,
	socklen_t tolen)
{
	int	flags = 0;

#ifdef	DEBUG
	fprintf(stderr, "send buf: ");
	dumpbuf(buf, len);
#endif

	if (sendto(sockfd, buf, len, flags, (struct sockaddr *)to, tolen) < 0){
		logmsg("tracker_send:sendto failed:errno (%d)\n", errno);
	}

	return(0);
}

ssize_t
tracker_recv(int sockfd, void *buf, size_t len, struct sockaddr *from,
	socklen_t *fromlen, struct timeval *timeout)
{
	ssize_t	size = 0;
	int	flags = 0;
	int	readit = 0;

	if (timeout) {
		fd_set			sockfds;
#ifdef	TIMEIT
		struct timeval		start_time, end_time;
		unsigned long long	s, e;

		gettimeofday(&start_time, NULL);
#endif

		FD_ZERO(&sockfds);
		FD_SET(sockfd, &sockfds);

		if ((select(sockfd+1, &sockfds, NULL, NULL, timeout) > 0) &&
				(FD_ISSET(sockfd, &sockfds))) {

#ifdef	TIMEIT
			gettimeofday(&end_time, NULL);
			s = (start_time.tv_sec * 1000000) +
				start_time.tv_usec;
			e = (end_time.tv_sec * 1000000) + end_time.tv_usec;
			logmsg("tracker_recv:svc time: %lld usec\n", (e - s));
#endif

			readit = 1;
		} else {
			logmsg("tracker_recv:timeout:error\n");
		}
	} else {
		readit = 1;
	}

	if (readit) {
		size = recvfrom(sockfd, buf, len, flags, from, fromlen);
	}
	
#ifdef	DEBUG
	if (size > 0) {
		fprintf(stderr, "recv buf: ");
		dumpbuf(buf, size);
	}
#endif

	return(size);
}

int
init_tracker_comm(int port)
{
	struct sockaddr_in	client_addr;
	int			sockfd;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("init_tracker_comm:socket failed:");
		return(-1);
	}

	/*
	 * bind the socket so we can send from it
	 */
	bzero(&client_addr, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	client_addr.sin_port = htons(port);

	if (bind(sockfd, (struct sockaddr *)&client_addr,
			sizeof(client_addr)) < 0) {
                perror("init_tracker_comm:bind failed:");
		close(sockfd);
                return(-1);
	}

	return(sockfd);
}

static void
random_shuffle(peer_t *peers, uint16_t numpeers)
{
	peer_t	temp;
	int	i, j;

	if (numpeers < 2) {
		return;
	}

	for (i = 0 ; i < numpeers - 1 ; ++i) {
		j = i + rand() / (RAND_MAX / (numpeers - i) + 1);

		memcpy(&temp, &peers[j], sizeof(temp));
		memcpy(&peers[j], &peers[i], sizeof(peers[j]));
		memcpy(&peers[i], &temp, sizeof(peers[i]));
	}
}

unsigned long long
stampit()
{
	struct timeval		now;
	unsigned long long	n;

	gettimeofday(&now, NULL);
	n = (now.tv_sec * 1000000) + now.tv_usec;
	return(n);
}

dt_table_t	*dt_table = NULL;

int
grow_dt_table(int size)
{
	uint32_t	oldsize;
	uint32_t	newsize;
	int		len;

	if (dt_table == NULL) {
		oldsize = 0;
		newsize = size;
	} else {
		oldsize = dt_table->size;
		newsize = size + dt_table->size;
	}

	len = sizeof(dt_table_t) + sizeof(download_timestamp_t) * newsize;

	if ((dt_table = realloc(dt_table, len)) == NULL) {
		perror("grow_dt_table:malloc failed:");
		return(-1);
	}

	bzero(&dt_table[oldsize], (size * sizeof(download_timestamp_t)));
	dt_table->size = newsize;

	return(0);
}

unsigned long long
add_to_dt_table(in_addr_t host)
{
	int	i;

#ifdef	DEBUG
{
	struct in_addr	in;

	in.s_addr = host;
	fprintf(stderr, "add_to_dt_table:host (%s)\n", inet_ntoa(in));
}
#endif

	/*
	 * first check if the table is not created yet or if it is full
	 */
	if ((dt_table == NULL) ||
			(dt_table->entry[dt_table->size - 1].host != 0)) {
		grow_dt_table(DT_TABLE_ENTRIES);
	}

	for (i = 0 ; i < dt_table->size ; ++i) {
		if (dt_table->entry[i].host == 0) {
			/*
			 * this entry is free
			 */
			dt_table->entry[i].host = host;
			dt_table->entry[i].timestamp = 0;
			break;
		}
	}

	return(dt_table->entry[i].timestamp);
}

static unsigned long long
lookup_timestamp(in_addr_t host)
{
	unsigned long long	timestamp = 0;
	int			i;

	if (dt_table == NULL) {
		if (grow_dt_table(DT_TABLE_ENTRIES) < 0) {
			return(0);
		}
	}

	for (i = 0 ; i < dt_table->size ; ++i) {
		if (dt_table->entry[i].host == 0) {
			/*
			 * at the last entry. this host was not found.
			 */
			break;
		}

		if (dt_table->entry[i].host == host) {
			timestamp = dt_table->entry[i].timestamp;
			break;
		}
	}

	if (timestamp == 0) {
		/*
		 * didn't find the host in the table. let's add it.
		 */
		timestamp = add_to_dt_table(host);
	}

	return(timestamp);
}

static void
update_timestamp(in_addr_t host)
{
	int	i;

	if (dt_table == NULL) {
		return;
	}

	for (i = 0 ; i < dt_table->size ; ++i) {
		if (dt_table->entry[i].host == 0) {
			/*
			 * at the last entry. this host was not found.
			 */
			break;
		}

		if (dt_table->entry[i].host == host) {
			dt_table->entry[i].timestamp = stampit();
			break;
		}
	}

	return;
}


static int
timestamp_compare(const void *a, const void *b)
{
	peer_timestamp_t	*key1 = (peer_timestamp_t *)a;
	peer_timestamp_t	*key2 = (peer_timestamp_t *)b;

	if (key1->peer.state == key2->peer.state) {
		if (key1->timestamp < key2->timestamp) {
			return(-1);
		} else if (key1->timestamp > key2->timestamp) {
			return(1);
		}
	} else {
		if (key1->peer.state == DOWNLOADING) {
			return(1);
		} else if (key2->peer.state == DOWNLOADING) {
			return(-1);
		}
	}

	/*
	 * the keys must be equal
	 */
	return(0);
}

void
shuffle(peer_t *peers, uint16_t numpeers)
{
	peer_timestamp_t	*list;
	int			i;

	if ((list = (peer_timestamp_t *)malloc(
			numpeers * sizeof(peer_timestamp_t))) == NULL) {
		/*
		 * if this fails, just do a random shuffle
		 */
		random_shuffle(peers, numpeers);

		/*
		 * need to touch all the timestamps in the download timestamps
		 * table
		 */

		return;
	}

	for (i = 0 ; i < numpeers ; ++i) {
		memcpy(&list[i].peer, &peers[i], sizeof(list[i].peer));
		list[i].timestamp = lookup_timestamp(list[i].peer.ip);
	}

	if (numpeers > 1) {

#ifdef	DEBUG
fprintf(stderr, "shuffle:before sort\n");

for (i = 0 ; i < numpeers ; ++i) {
	struct in_addr	in;

	in.s_addr = list[i].peer.ip;
	
	fprintf(stderr, "\thost %s : state %c : timestamp %lld\n",
		inet_ntoa(in),
		(list[i].peer.state == DOWNLOADING ? 'd' : 'r'),
		list[i].timestamp);
}
#endif

		/*
		 * sort the list by timestamps
		 */
		qsort(list, numpeers, sizeof(peer_timestamp_t),
			timestamp_compare);

#ifdef	DEBUG
fprintf(stderr, "shuffle:after sort\n");

for (i = 0 ; i < numpeers ; ++i) {
	struct in_addr	in;

	in.s_addr = list[i].peer.ip;
	
	fprintf(stderr, "\thost %s : state %c : timestamp %lld\n",
		inet_ntoa(in),
		(list[i].peer.state == DOWNLOADING ? 'd' : 'r'),
		list[i].timestamp);
}
#endif

		/*
		 * now copy the sorted list back into peers
		 */
		for (i = 0 ; i < numpeers ; ++i) {
			memcpy(&peers[i], &list[i].peer, sizeof(peers[i]));
		}

		/*
		 * update the timestamp on only the first entry in the new list
		 */ 
		update_timestamp(peers[0].ip);
	}

	free(list);
	return;
}

