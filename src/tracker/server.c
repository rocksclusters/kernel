#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "tracker.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

hash_table_t	*hash_table = NULL;

int
init_hash_table(int size)
{
	int		len;

	len = sizeof(hash_table_t) + (size * sizeof(hash_info_t));

	if ((hash_table = malloc(len)) == NULL) {
		perror("init_hash_table:malloc failed:");
		return(-1);
	}
	
	bzero(hash_table, len);
	hash_table->size = size;
	hash_table->head = 0;
	hash_table->tail = size - 1;

	return(0);
}

void
print_peers(hash_info_t *hashinfo)
{
	struct in_addr	in;
	int		i;

	for (i = 0 ; i < hashinfo->numpeers ; ++i) {
		in.s_addr = hashinfo->peers[i];
		fprintf(stderr, "\t%s\n", inet_ntoa(in));
	}
}

void
print_hash_table()
{
	int	i;

fprintf(stderr, "head: (%d)\n", hash_table->head);
fprintf(stderr, "tail: (%d)\n", hash_table->tail);

	if (hash_table->head >= hash_table->tail) {
		for (i = hash_table->head ;
				(i >= hash_table->tail) && (i >= 0) ; --i) {

			fprintf(stderr, "entry[%d] : hash (0x%lx)\n", i,
				hash_table->entry[i].hash);

			print_peers(&hash_table->entry[i]);
		}
	} else {
		for (i = hash_table->head - 1; i >= 0 ; --i) {
			fprintf(stderr, "entry[%d] : hash (0x%lx)\n", i,
				hash_table->entry[i].hash);

			print_peers(&hash_table->entry[i]);
		}

		for (i = (hash_table->size - 1) ;
				(i > hash_table->tail) && (i >= 0) ; --i) {

			fprintf(stderr, "entry[%d] : hash (0x%lx)\n", i,
				hash_table->entry[i].hash);

			print_peers(&hash_table->entry[i]);
		}
	}

	return;
}

/*
 * remove all free entries in between the head and the tail
 */
void
compact_hash_table()
{
	int	free_index = hash_table->head - 1;
	int	allocated_index;
	char	found, done;

#ifdef	DEBUG
fprintf(stderr, "compact_hash_table:before\n\n");
print_hash_table();
#endif


	while (1) {
		/*
		 * find a free slot
		 */
		found = 0;
		done = 0;
		while (!done) {
			if (free_index == hash_table->tail) {
				done = 1;
				continue;
			}

			if (free_index < 0) {
				free_index = hash_table->size - 1;
				continue;
			}

			if (hash_table->entry[free_index].hash == 0) {
				done = 1;
				found = 1;
				continue;
			}

			--free_index;
		}

		if (found == 0) {
			/*
			 * there are no free slots between head and tail
			 */
			return;
		}

		/*
		 * find the next allocated slot
		 */
		allocated_index = free_index - 1;
		found = 0;
		done = 0;

		while (!done) {
			if (allocated_index == hash_table->tail) {
				done = 1;
				continue;
			}

			if (allocated_index < 0) {
				allocated_index = hash_table->size - 1;
				continue;
			}

			if (hash_table->entry[allocated_index].hash != 0) {
				done = 1;
				found = 1;
				continue;
			}

			--allocated_index;
		}

		if (found == 0) {
			/*
			 * there are no allocated slots between free_index and
			 * tail
			 */
			return;
		}

		/*
		 * move the allocated slot to the free slot, then free the
		 * previous allocated slot
		 */
		hash_table->entry[free_index].hash =
			hash_table->entry[allocated_index].hash;
		hash_table->entry[free_index].numpeers =
			hash_table->entry[allocated_index].numpeers;
		hash_table->entry[free_index].peers =
			hash_table->entry[allocated_index].peers;

		hash_table->entry[allocated_index].hash = 0;
		hash_table->entry[allocated_index].numpeers = 0;
		hash_table->entry[allocated_index].peers = NULL;

		--free_index;
	}

#ifdef	DEBUG
fprintf(stderr, "compact_hash_table:after\n\n");
print_hash_table();
#endif

}

void
reclaim_free_entries()
{
	int	i;
	int	current_tail, current_head;

#ifdef	DEBUG
fprintf(stderr, "reclaim_free_entries:head (%d), tail (%d), size (%d)\n",
	hash_table->head, hash_table->tail, hash_table->size);
#endif

	/*
	 * first, try to move the tail
	 */
	if (hash_table->tail > hash_table->head) {
		current_tail = hash_table->tail;

		for (i = current_tail + 1 ; i < hash_table->size &&
				hash_table->entry[i].hash == 0 ; ++i) {
			++hash_table->tail;
		}

		/*
	 	 * special case when the tail hits the end of the list
		 */
		if (hash_table->tail == hash_table->size - 1) {
			if ((hash_table->entry[0].hash == 0) && 
					(hash_table->head != 0)) {
				hash_table->tail = 0;
			}
		}
	}

	if (hash_table->tail < hash_table->head) {
		current_tail = hash_table->tail;

		for (i = current_tail + 1 ; i < hash_table->head &&
				hash_table->entry[i].hash == 0 ; ++i) {
			++hash_table->tail;
		}
	}

	/*
	 * now try to move the head
	 */
	if (hash_table->head < hash_table->tail) {
		current_head = hash_table->head;

		for (i = current_head - 1 ; i >= 0 &&
				hash_table->entry[i].hash == 0 ; --i) {
			--hash_table->head;
		}

		/*
		 * special case for when head hits the top of the list
		 */
		if ((hash_table->head == 0) &&
			(hash_table->entry[hash_table->size - 1].hash == 0)) {

			hash_table->head = hash_table->size - 1;
		}
	}

	if (hash_table->head > hash_table->tail) {
		current_head = hash_table->head;

		for (i = current_head - 1 ; i < hash_table->tail &&
				hash_table->entry[i].hash == 0 ; --i) {
			--hash_table->head;
		}
	}

#ifdef	DEBUG
fprintf(stderr, "reclaim_free_entries:head (%d), tail (%d)\n",
	hash_table->head, hash_table->tail);
#endif
}

/*
 * 'size' is the number of new entries to be added to the table
 */
int
grow_hash_table(int size)
{
	uint32_t	oldsize = hash_table->size;
	uint32_t	newsize = size + hash_table->size;
	int		len;

	len = sizeof(hash_table_t) + (newsize * sizeof(hash_info_t));

#ifdef	DEBUG
fprintf(stderr, "grow_hash_table:enter:size (%d)\n", hash_table->size);
fprintf(stderr, "grow_hash_table:enter:head (%d)\n", hash_table->head);
fprintf(stderr, "grow_hash_table:enter:tail (%d)\n", hash_table->tail);

fprintf(stderr, "grow_hash_table:before\n\n");
print_hash_table();
#endif

	if ((hash_table = realloc(hash_table, len)) == NULL) {
		perror("grow_hash_table:realloc failed:");
		return(-1);
	}

	/*
	 * create an initialized space in the new table for the new entries.
	 * these new entries are being 'spliced' into the middle of the table
	 * starting at 'head'.
	 * 
	 * need to move the entries from the table head down to the end of
	 * the old table into the last part of the new table.
	 */
	memcpy(&hash_table->entry[oldsize],
		&hash_table->entry[hash_table->head],
			size * sizeof(hash_info_t));
	
	/*
	 * initialize the new entries
	 */
	bzero(&hash_table->entry[hash_table->head], size * sizeof(hash_info_t));

	hash_table->size = newsize;
	hash_table->tail = hash_table->tail + size;

#ifdef	DEBUG
fprintf(stderr, "grow_hash_table:exit:size (%d)\n", hash_table->size);
fprintf(stderr, "grow_hash_table:exit:head (%d)\n", hash_table->head);
fprintf(stderr, "grow_hash_table:exit:tail (%d)\n", hash_table->tail);

fprintf(stderr, "grow_hash_table:after\n\n");
print_hash_table();
#endif

	return(0);
}

hash_info_t *
newentry()
{
	int	index;
	
	index = hash_table->head;

	++hash_table->head;

	if (hash_table->head == hash_table->tail) {
		if (grow_hash_table(HASH_TABLE_ENTRIES) != 0) {
			fprintf(stderr, "newentry:grow_hash_table:failed\n");
			return(NULL);
		}
	}

	if (hash_table->head == hash_table->size) {
		hash_table->head = 0;
	}

	return(&hash_table->entry[index]);
}

int
addpeer(hash_info_t *hashinfo, in_addr_t *peer)
{

#ifdef	DEBUG
{
	struct in_addr	in;

	in.s_addr = *peer;
	fprintf(stderr, "addpeer:adding peer (%s) for hash (0x%016lx)\n",
		inet_ntoa(in), hashinfo->hash);
}
#endif

	if (hashinfo == NULL) {
		fprintf(stderr, "addpeer:hashinfo NULL\n");
		return(-1);
	}

	if (hashinfo->peers) {
		if ((hashinfo->peers = realloc(hashinfo->peers,
			(hashinfo->numpeers + 1) * sizeof(*peer))) == NULL) {

			fprintf(stderr, "addpeer:realloc failed\n");
			return(-1);
		}
	} else {
		if ((hashinfo->peers = malloc(sizeof(*peer))) == NULL) {
			fprintf(stderr, "addpeer:malloc failed\n");
			return(-1);
		}
	}

	memcpy(&hashinfo->peers[hashinfo->numpeers], peer, sizeof(*peer));
	++hashinfo->numpeers;
		
	return(0);
}

hash_info_t *
getpeers(uint64_t hash, int *index)
{
	int	i;
	int	h = hash_table->head;
	int	found = -1;

	if (h < hash_table->tail) {
		/*
		 * work down to entry 0, then reset the head (h) to the
		 * last entry of the table
		 */
		for (i = h - 1; i >= 0 ; --i) {
			if (hash_table->entry[i].hash == hash) {
				found = i;
				break;
			}
		}

		h = hash_table->size - 1;
	}

	if (found == -1) {
		/*
		 * we know head (h) is greater than tail
		 */
		for (i = h ; i >= hash_table->tail ; --i) {
			if (hash_table->entry[i].hash == hash) {
				found = i;
				break;
			}
		}
	}

	if (found != -1) {
#ifdef	DEBUG
		fprintf(stderr, "getpeers:hash (0x%016lx) found\n", hash);
#endif
		if (index != NULL) {
			*index = i;
		}

		return(&hash_table->entry[i]);
	}

	/*
	 * if we made it here, then the hash is not in the table
	 */
#ifdef	DEBUG
	fprintf(stderr, "getpeers:hash (0x%016lx) not found\n", hash);
#endif
	return(NULL);
}

hash_info_t *
getnextpeers(uint64_t hash, int *index)
{
	int	i;
	int	newindex = *index;

	++newindex;

	if (newindex >= hash_table->size) {
		newindex = 0;
	}

	for (i = newindex ; i < hash_table->size ; ++i) {
		if (hash_table->entry[i].hash != 0) {
			*index = i;
			return(&hash_table->entry[i]);
		}
	}

	/*
	 * edge case where *index points to the last entry in the table, which
	 * means that the above loop already scanned all the entries
	 */
	if (*index == (hash_table->size - 1)) {
		return(NULL);
	}

	for (i = 0 ; i < *index ; ++i) {
		if (hash_table->entry[i].hash != 0) {
			*index = i;
			return(&hash_table->entry[i]);
		}
	}

	return(NULL);
}

void
dolookup(int sockfd, uint64_t hash, struct sockaddr_in *from_addr)
{
	tracker_lookup_resp_t	*resp;
	tracker_info_t		*respinfo;
	hash_info_t		*hashinfo;
	size_t			len;
	int			i, j;
	int			flags;
	int			index, next_index;
	char			buf[64*1024];

#ifdef	DEBUG
	fprintf(stderr, "dolookup:enter:hash (0x%lx) from (%s)\n", hash,
		inet_ntoa(from_addr->sin_addr));
#endif

	resp = (tracker_lookup_resp_t *)buf;
	resp->header.op = LOOKUP;

	/*
	 * keep a running count for the length of the data
	 */
	len = sizeof(tracker_lookup_resp_t);

#ifdef	LATER
fprintf(stderr, "len (%d)\n", (int)len);
#endif

	/*
	 * look up info for this hash
	 */
	respinfo = (tracker_info_t *)resp->info;
	respinfo->hash = hash;

	len += sizeof(tracker_info_t);

#ifdef	LATER
fprintf(stderr, "len (%d)\n", (int)len);
#endif

	/*
	 * always send back at least one hash, even if it is empty (i.e.,
	 * it has no peers.
	 */
	resp->numhashes = 1;

	if ((hashinfo = getpeers(hash, &index)) != NULL) {
		/*
		 * copy the hash info into the response buffer
		 */
		respinfo->hash = hashinfo->hash;
		respinfo->numpeers = 0;

		for (i = 0 ; i < hashinfo->numpeers ; ++i) {
			/*
			 * don't copy in address of requestor
			 */
			if (hashinfo->peers[i] == from_addr->sin_addr.s_addr) {
				continue;
			}

			respinfo->peers[i] = hashinfo->peers[i];
			++respinfo->numpeers;
		}

#ifdef	DEBUG
fprintf(stderr, "resp info numpeers (%d)\n", respinfo->numpeers);
#endif

		len += (sizeof(respinfo->peers[0]) * respinfo->numpeers);

		respinfo = (tracker_info_t *)
			(&(respinfo->peers[respinfo->numpeers]));

		/*
		 * now get hash info for the "predicted" next file downloads
		 */
		next_index = index;
		for (j = 0 ; j < PREDICTIONS ; ++j) {
			if ((hashinfo = getnextpeers(hash, &next_index))
					!= NULL) {
				/*
				 * copy the hash info into the response buffer
				 */
				if (index == next_index) {
					/*
					 * there are less than 'PREDICTIONS'
					 * number of valid hash entries and
					 * we've examined them all. break out
					 * of this loop.
					 */
					break;
				}

#ifdef	DEBUG
fprintf(stderr, "prediction:adding hash (0%016lx)\n", hashinfo->hash);
#endif

				respinfo->hash = hashinfo->hash;
				respinfo->numpeers = 0;

				for (i = 0 ; i < hashinfo->numpeers ; ++i) {
					/*
					 * don't copy in address of requestor
					 */
					if (hashinfo->peers[i] ==
						from_addr->sin_addr.s_addr) {

						continue;
					}

					respinfo->peers[i] = hashinfo->peers[i];
					++respinfo->numpeers;
				}
			} else {
				/*
				 * no more valid hashes
				 */
				break;
			}

			len += sizeof(tracker_info_t) +
				(sizeof(respinfo->peers[0]) *
					respinfo->numpeers);

			respinfo = (tracker_info_t *)
				(&(respinfo->peers[respinfo->numpeers]));

			++resp->numhashes;
		}

#ifdef	DEBUG
fprintf(stderr, "len (%d)\n", (int)len);
#endif

	} else {
		respinfo->numpeers = 0;
	}

	resp->header.length = len;

#ifdef	DEBUG
	fprintf(stderr, "send buf: ");
	dumpbuf((char *)resp, len);
#endif

	flags = 0;
	sendto(sockfd, buf, len, flags, (struct sockaddr *)from_addr,
		sizeof(*from_addr));

#ifdef	DEBUG
fprintf(stderr, "dolookup:exit:hash (0x%lx)\n", hash);
#endif

	return;
}

void
register_hash(char *buf, struct sockaddr_in *from_addr)
{
	tracker_register_t	*req = (tracker_register_t *)buf;
	hash_info_t		*hashinfo;
	tracker_info_t		*reqinfo;
	uint16_t		numpeers;
	in_addr_t		dynamic_peers[1];
	in_addr_t		*peers;
	int			i, j, k;

	for (i = 0; i < req->numhashes; ++i) {
		reqinfo = &req->info[i];
#ifdef	DEBUG
fprintf(stderr, "register_hash:enter:hash (0x%lx)\n", reqinfo->hash);
fprintf(stderr, "register_hash:hash_table:before\n\n");
print_hash_table();
#endif

		if (reqinfo->numpeers == 0) {
			/*
			 * no peer specified. dynamically determine
			 * the peer IP address from the host who
			 * sent us the message
			 */
			numpeers = 1;
			dynamic_peers[0] = from_addr->sin_addr.s_addr;
			peers = dynamic_peers;
		} else {
			numpeers = reqinfo->numpeers;
			peers = reqinfo->peers;
		}

#ifdef	DEBUG
fprintf(stderr, "register_hash:numpeers:1 (0x%d)\n", numpeers);
#endif

		/*
		 * scan the list for this hash.
		 */
		if ((hashinfo = getpeers(reqinfo->hash, NULL)) != NULL) {
			/*
			 * this hash is already in the table, see if this peer
			 * is already in the list
			 */
			for (j = 0 ; j < numpeers ; ++j) {
				int	found = 0;
		
				for (k = 0 ; k < hashinfo->numpeers ; ++k) {
					if (peers[j] == hashinfo->peers[k]) {
						found = 1;
						break;
					}
				}

				if (!found) {
					addpeer(hashinfo, &peers[j]);
				}
			}
		} else {
			/*
			 * if not, then add this hash to the end of the list
			 */
			if ((hashinfo = newentry()) == NULL) {
				fprintf(stderr, "addentry:failed\n");
				abort();
			}

			hashinfo->hash = reqinfo->hash;
			hashinfo->numpeers = 0;

			for (j = 0 ; j < numpeers ; ++j) {
				addpeer(hashinfo, &peers[j]);
			}

		}
#ifdef	DEBUG
fprintf(stderr, "register_hash:hash_table:after\n\n");
print_hash_table();
fprintf(stderr, "register_hash:exit:hash (0x%lx)\n", reqinfo->hash);
#endif
	}
}

void
removepeer(int index, in_addr_t *peer)
{
	hash_info_t	*hashinfo = &hash_table->entry[index];
	int		i, j;

#ifdef	DEBUG
{
	struct in_addr	in;

	in.s_addr = *peer;
	fprintf(stderr, "removepeer:removing peer (%s) for hash (0x%016lx)\n",
		inet_ntoa(in), hashinfo->hash);
}
#endif

	for (i = 0; i < hashinfo->numpeers; ++i) {
		if (hashinfo->peers[i] == *peer) {
			if (hashinfo->numpeers == 1) {
				/*
				 * do the easy case first
				 */
				free(hashinfo->peers);

				hashinfo->peers = NULL;
				hashinfo->hash = 0;
				hashinfo->numpeers = 0;
				
				compact_hash_table();
				reclaim_free_entries();
			} else {
				/*
				 * remove the peer by compacting all subsequent
				 * peers
				 */
				for (j = i; j < hashinfo->numpeers - 1; ++j) {
					memcpy(&hashinfo->peers[j],
						&hashinfo->peers[j+1],
						sizeof(*peer));
				}

				/*
				 * zero out the last entry
				 */
				bzero(&hashinfo->peers[hashinfo->numpeers-1],
					sizeof(*peer));

				--hashinfo->numpeers;
			}

			break;
		}
	}

	return;
}

void
unregister_hash(char *buf, struct sockaddr_in *from_addr)
{
	tracker_unregister_t	*req = (tracker_unregister_t *)buf;
	in_addr_t		peer;
	int			index;
	int			i;

#ifdef	DEBUG
fprintf(stderr, "unregister_hash:enter\n");
fprintf(stderr, "unregister_hash:hash_table:before\n\n");
print_hash_table();
#endif

	peer = from_addr->sin_addr.s_addr;

	for (i = 0 ; i < req->numhashes ; ++i) {
		if (getpeers(req->info[i].hash, &index) != NULL) {
			removepeer(index, &peer);
		}
	}

#ifdef	DEBUG
fprintf(stderr, "unregister_hash:hash_table:after\n\n");
print_hash_table();
#endif

}

void
unregister_all(char *buf, struct sockaddr_in *from_addr)
{
	in_addr_t	peer;
	int		i;

#ifdef	DEBUG
fprintf(stderr, "unregister_all:enter\n");
fprintf(stderr, "unregister_all:hash_table:before\n\n");
print_hash_table();
#endif

	peer = from_addr->sin_addr.s_addr;

	for (i = 0 ; i < hash_table->size; ++i) {
		if (hash_table->entry[i].hash != 0) {
			removepeer(i, &peer);
		}
	}

#ifdef	DEBUG
fprintf(stderr, "unregister_all:hash_table:after\n\n");
print_hash_table();
#endif

}

int
main()
{
	struct sockaddr_in	from_addr;
	socklen_t		from_addr_len;
	ssize_t			recvbytes;
	int			sockfd;
	char			buf[64*1024];
	char			done;

	if ((sockfd = init_tracker_comm(TRACKER_PORT)) < 0) {
		fprintf(stderr, "main:init_tracker_comm:failed\n");
		abort();
	}

	if (init_hash_table(HASH_TABLE_ENTRIES) != 0) {
		fprintf(stderr, "main:init_hash_table:failed\n");
		abort();
	}

#ifdef	DEBUG
fprintf(stderr, "main:starting\n");
#endif

	done = 0;
	while (!done) {
		from_addr_len = sizeof(from_addr);
		recvbytes = tracker_recv(sockfd, buf, sizeof(buf),
			(struct sockaddr *)&from_addr, &from_addr_len, NULL);

		if (recvbytes > 0) {
			tracker_header_t	*p;

			p = (tracker_header_t *)buf;

			switch(p->op) {
			case LOOKUP:
				{
					tracker_lookup_req_t	*req;

					req = (tracker_lookup_req_t *)buf;
					dolookup(sockfd, req->hash, &from_addr);
				}
				break;

			case REGISTER:
				register_hash(buf, &from_addr);
				break;

			case UNREGISTER:
				unregister_hash(buf, &from_addr);
				break;

			case END:
				unregister_all(buf, &from_addr);
				break;

			default:
				fprintf(stderr, "Unknown op (%d)\n", p->op);
				abort();
				break;
			}
		}
	}

	return(0);
}

