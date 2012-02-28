#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "tracker.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "sqlite3.h"
static char builton[] = { "Built on: " __DATE__ " " __TIME__ };



/* -------------------------------------------- */
/* --          Generic SQL Routines          -- */         
/* -------------------------------------------- */
/* This routine executes a simple sql statement */
void sql_stmt(sqlite3 *db, const char* stmt) {
  char *errmsg;
  int   ret;

  ret = sqlite3_exec(db, stmt, 0, 0, &errmsg);

  if(ret != SQLITE_OK) {
    printf("Error in statement: %s [%s].\n", stmt, errmsg);
  }
}

/* Prepare a SQL statement for execution  */
int prep_stmt(sqlite3 *db, const char* stmt, sqlite3_stmt **preppedStmt) {
const char *tail;
int len;
int rcode;

	if ( stmt != NULL && (len = strlen(stmt)) > 0)
	{
		rcode = sqlite3_prepare_v2(db,stmt,len,preppedStmt,&tail);
		if (rcode != SQLITE_OK)
		{
    			printf("Error in statement: %s.\n", stmt);
			return rcode;
		}
	}
	return SQLITE_OK;
}

/* Interrogation */

int getIntValue(sqlite3 *db, char *sqlStmt) {
sqlite3_stmt *preppedStmt; 
int rvalue = 0;
int sqlCode;
	if (prep_stmt(db, sqlStmt, &preppedStmt) == SQLITE_OK)
	{
		sqlCode = sqlite3_step(preppedStmt); 
		switch (sqlCode) {
			case SQLITE_ROW:
				rvalue = sqlite3_column_int(preppedStmt,0);
				break;
			case SQLITE_DONE:
				break;
			default:
				printf("Error in getRowID Query\n");
	
		}
		sqlite3_finalize(preppedStmt);
	}
	return rvalue;
}

/* --- check if a host exists in the hosts table --- */
int hostExists(sqlite3 *db, int ip) {
char sqlStmt[256];
	sprintf(sqlStmt, "SELECT hostid FROM hosts WHERE IP=%d", ip);
	return getIntValue(db,sqlStmt);
}

/* ----- Initialize an in-memory database ----- */
int
init_db(sqlite3 **db)
{
	sqlite3_open(":memory:", db);
	if(*db == 0) {
		printf("Could not open database.");
		return 1;
	}

	/* create the tracker schema */
	sql_stmt(*db, "CREATE TABLE hosts (hostid INTEGER PRIMARY KEY, ip INTEGER, groupid INTEGER)");
	sql_stmt(*db, "CREATE TABLE hashes (hashid INTEGER PRIMARY KEY, hash INTEGER)");
	sql_stmt(*db, "CREATE TABLE peers (hashid INTEGER, hostid INTEGER, state INTEGER)");

	return(0);
}


/* -------------------------------------------- */
/* --          Host Table Routines           -- */         
/* -------------------------------------------- */
/* --- add a host  --- */
int addHost(sqlite3 *db, int ip) {
char sqlStmt[256];
int hostid = 0;
	/* Don't add if already there */
	if ( (hostid = hostExists(db, ip)))
		return hostid;
	sprintf(sqlStmt, "INSERT INTO hosts(hostid,ip,groupid) values(NULL,%d,0)", ip);
	sql_stmt(db,sqlStmt);
	return hostExists(db,ip);

}

/* ---remove host  --- */
int deleteHost(sqlite3 *db, int ip) {
char sqlStmt[256];
int hostid = 0;
	if ( (hostid = hostExists(db, ip))  <= 0 )
		return 0;
	/* delete host from peers table */
	sprintf(sqlStmt, "DELETE FROM peers WHERE hostid=%d",hostid);
	sql_stmt(db,sqlStmt);
	/* delete host from hosts table */
	sprintf(sqlStmt, "DELETE FROM hosts WHERE hostid=%d",hostid);
	sql_stmt(db,sqlStmt);
	return 0;
}

/* -------------------------------------------- */
/* --          Hash Table Routines           -- */         
/* -------------------------------------------- */
/* --- check if a hash exists in the hashes table --- */
int hashExists(sqlite3 *db, uint64_t hash) {
char sqlStmt[256];
	sprintf(sqlStmt, "SELECT hashid FROM hashes WHERE hash=%ld", hash);
	return getIntValue(db,sqlStmt);
}

/* --- add a hash --- */
int addHash(sqlite3 *db, uint64_t hash) {
char sqlStmt[256];
int hashid = 0;
	/* don't add if already there */
	if ( (hashid = hashExists(db, hash)) ) 
		return hashid;
	sprintf(sqlStmt, "INSERT INTO hashes(hashid,hash) VALUES(NULL,%ld)", hash);
	sql_stmt(db,sqlStmt);
	return hashExists(db,hash);
}

/* ---delete Hash  --- */
int deleteHash(sqlite3 *db, uint64_t hash) {
char sqlStmt[256];
int hashid = 0;
	if ( (hashid = hashExists(db, hash)) <= 0)
		return 0;
	/* delete hash from peers table */
	sprintf(sqlStmt, "DELETE FROM peers WHERE hashid=%d",hashid);
	sql_stmt(db,sqlStmt);
	/* delete from hashes table */
	sprintf(sqlStmt, "DELETE FROM hashes WHERE hashid=%d",hashid);
	sql_stmt(db,sqlStmt);
	return 0;
}

/* -------------------------------------------- */
/* --        Peer Table Routines             -- */         
/* -------------------------------------------- */

/* --- registerPeer --- */
int registerPeer(sqlite3 *db, uint64_t hash, int ip) {
char sqlStmt[256];
int hashid = 0;
int hostid = 0;
	/* These will check for existence of host, hash, add if necessary */
        hostid = addHost(db,ip);
	hashid = addHash(db,hash);

	/* check if registered */
	sprintf(sqlStmt, "SELECT hashid FROM peers WHERE hashid=%d and hostid=%d", hashid, hostid);
	if ( getIntValue(db,sqlStmt) )
		return 0;
	sprintf(sqlStmt, "INSERT INTO PEERS(hashid, hostid, state) VALUES(%d,%d,'r')", hashid, hostid);

	sql_stmt(db,sqlStmt);
	return 0;
}

/* -- Randomly Copy Peers -- */
int
randomCopyPeers(peer_t *dstpeers, peer_t *srcpeers, int npeers, int maxpeers)
{
int start;
int count;
int bottomcopy, topcopy;
	if (npeers <= 0)
		return npeers;

       /* Copy at most maxpeers from src to dest, starting at a random index
          in the srcpeers array.   Take care of wrap */ 

	count = min(npeers,maxpeers);

	/* pick a random starting index [0, npeers -1] */
	start = rand() % npeers;

	if ( (start + count) >= npeers ) /* Wrap */
	{
		bottomcopy=npeers-start;
		topcopy=count-bottomcopy;
	}
	else  /* No Wrap */
	{
		bottomcopy=count;
		topcopy=0;
	}

	/* copy the bottom part of src array */
	memcpy(dstpeers, &srcpeers[start], (bottomcopy)*sizeof(peer_t));

	/* copy top part of src array */
	if (topcopy > 0)
		memcpy(&dstpeers[bottomcopy], srcpeers, 
				topcopy*sizeof(peer_t));
	return count;
}

/* -- dolookup(): lookup peers for hashes -- */
void
dolookup(sqlite3 *db, int sockfd, uint64_t hash, uint32_t seqno,
	struct sockaddr_in *from_addr)
{
tracker_lookup_resp_t	*resp;
tracker_info_t		*respinfo;
size_t			len;
int			flags;
int			npeers;
char			buf[64*1024];
char sqlStmt[256];
peer_t			peers[MAX_SHUFFLE_PEERS];
sqlite3_stmt *preppedStmt; 
int rcode, sqlCode;
int ip, hashid, hostid, state;

	/* -- Response Header -- */
	resp = (tracker_lookup_resp_t *)buf;
	resp->header.op = LOOKUP;
	resp->header.seqno = seqno;

	/*
	 * keep a running count for the length of the data
	 */
	len = sizeof(tracker_lookup_resp_t);

	/*
	 * look up info for this hash
	 */
	respinfo = (tracker_info_t *)resp->info;
	respinfo->hash = hash;
	resp->numhashes = 1;   /* always return this hash, even if no peers */

	len += sizeof(tracker_info_t);

	/*  -- Query Database for peers of this hash -- */
	sprintf(sqlStmt, "select IP,hashid,hostid,state from peers inner join hashes using(hashid) inner join hosts using(hostid) where hash=%ld", hash);
	if (prep_stmt(db, sqlStmt, &preppedStmt) == SQLITE_OK)
	{
		npeers = 0;
		while ( (sqlCode = sqlite3_step(preppedStmt))  == SQLITE_ROW
				&& npeers < MAX_SHUFFLE_PEERS)
		{
			ip = sqlite3_column_int(preppedStmt,0);
			hashid = sqlite3_column_int(preppedStmt,1);
			hostid = sqlite3_column_int(preppedStmt,2);
			state = sqlite3_column_int(preppedStmt,3);
			
			/* check if requestor is already a peer */
			if (ip == (int) from_addr->sin_addr.s_addr) 
				continue;

			/* copy the info just read from DB into unshuffled
                           peers array */ 
			peers[npeers].ip = ip;
			peers[npeers].state = (state == DOWNLOADING ? 'd' :'r');
			npeers++;
		}
		if (sqlCode != SQLITE_DONE && npeers < MAX_SHUFFLE_PEERS)
		{
			rcode=sqlCode;
			printf ("error in getPeers (%d)\n", rcode);
		}
		sqlite3_finalize(preppedStmt);
	}

	respinfo->numpeers = randomCopyPeers(respinfo->peers, peers, npeers, MAX_PEERS);
#ifdef	DEBUG
	fprintf(stderr, "resp info numpeers (%d)\n", respinfo->numpeers);
#endif

	len += (sizeof(respinfo->peers[0]) * respinfo->numpeers);

	/* ---  Predict the Next Hashes this client will want --- */
	respinfo = (tracker_info_t *) (&(respinfo->peers[respinfo->numpeers]));
#ifdef	DEBUG
	fprintf(stderr, "len (%d)\n", (int)len);
	fprintf(stderr, "dolookup:numhashes (%d)\n", resp->numhashes);
#endif

	resp->header.length = len;

#ifdef	DEBUG
	fprintf(stderr, "send buf: ");
	dumpbuf((char *)resp, len);
#endif

	flags = 0;
	sendto(sockfd, buf, len, flags, (struct sockaddr *)from_addr,
		sizeof(*from_addr));

#ifdef	DEBUG
	fprintf(stderr, "dolookup:exit:hash (0x%llx)\n", hash);
#endif

	return;
}

/* -- garbageCollect() -- */
/*    Delete Hosts and Hashes that are no longer referenced in Peers Table */
int garbageCollect(sqlite3 *db) {
char sqlStmt[256];
	/* Remove all hashes not referenced in peers table */
	sprintf(sqlStmt, " DELETE from hashes where hashid in (SELECT hashes.hashid FROM hashes LEFT OUTER JOIN peers ON (hashes.hashid=peers.hashid) WHERE hostid IS NULL)");
	sql_stmt(db,sqlStmt);
	sprintf(sqlStmt, "DELETE FROM HOSTS WHERE hosts.hostid in (SELECT hosts.hostid FROM hosts LEFT OUTER JOIN peers ON (hosts.hostid=peers.hostid) WHERE hashid IS NULL)");
	sql_stmt(db,sqlStmt);
	return 0;
}
/* --- dumpTables --- */
void dumpTables(sqlite3 *db) {
char sqlStmt[256];
sqlite3_stmt *preppedStmt; 
int hashid, groupid, ip, hostid;
uint64_t hash;
int state;
int sqlCode;
	sprintf(sqlStmt, "select hostid,ip,groupid from hosts");
	if (prep_stmt(db, sqlStmt, &preppedStmt) == SQLITE_OK)
	{
		fprintf(stderr, "\nID\t|IP\t|GROUPID\n");
		while ( (sqlCode = sqlite3_step(preppedStmt))  == SQLITE_ROW)
		{
			hostid = sqlite3_column_int(preppedStmt,0);
			ip = sqlite3_column_int(preppedStmt,1);
			groupid = sqlite3_column_int(preppedStmt,2);
			fprintf(stderr, "%d\t|0x%08x\t|%d \n",hostid, ip, groupid);
		}
		sqlite3_finalize(preppedStmt);
	}
	sprintf(sqlStmt, "SELECT hashid,hash FROM hashes");
	if (prep_stmt(db, sqlStmt, &preppedStmt) == SQLITE_OK)
	{
		fprintf(stderr, "\nID\t|HASH\n");
		while ( (sqlCode = sqlite3_step(preppedStmt))  == SQLITE_ROW)
		{
			hashid = sqlite3_column_int(preppedStmt,0);
			hash = sqlite3_column_int64(preppedStmt,1);
			fprintf(stderr,"%d\t|%llx\n",hashid, hash);
		}
		sqlite3_finalize(preppedStmt);
	}

	sprintf(sqlStmt, "SELECT hashes.hash,hosts.ip,peers.state FROM peers INNER JOIN hashes USING(hashid) INNER JOIN hosts USING(hostid) ORDER by hashes.hash" );
	if (prep_stmt(db, sqlStmt, &preppedStmt) == SQLITE_OK)
	{
		fprintf(stderr, "\nHASH\t|IP\t|STATE\n");
		while ( (sqlCode = sqlite3_step(preppedStmt))  == SQLITE_ROW)
		{
			hash = sqlite3_column_int64(preppedStmt,0);
			ip = sqlite3_column_int(preppedStmt,1);
			state = sqlite3_column_int(preppedStmt,2);
			fprintf(stderr, "%llx\t|0x%08x\t|%d\n",hash, ip, state);
		}
		sqlite3_finalize(preppedStmt);
	}
}

void
register_hash(sqlite3 *db, char *buf, struct sockaddr_in *from_addr)
{
	tracker_register_t	*req = (tracker_register_t *)buf;
	tracker_info_t		*reqinfo;
	uint16_t		numpeers;
	peer_t			dynamic_peers[1];
	peer_t			*peers;
	int			i, j;

	for (i = 0; i < req->numhashes; ++i) {
		reqinfo = &req->info[i];
		if (reqinfo->numpeers == 0) {
			/*
			 * no peer specified. dynamically determine
			 * the peer IP address from the host who
			 * sent us the message
			 */
			numpeers = 1;
			dynamic_peers[0].ip = from_addr->sin_addr.s_addr;
			peers = dynamic_peers;
		} else {
			numpeers = reqinfo->numpeers;
			peers = reqinfo->peers;
		}

#ifdef	DEBUG
		fprintf(stderr, "register_hash:numpeers:1 (0x%d)\n", numpeers);
#endif
		for (j = 0 ; j < numpeers ; ++j) 
		{
			registerPeer(db, reqinfo->hash, peers[j].ip); 
		}
#ifdef	DEBUG
	fprintf(stderr, "register_hash:exit:hash (0x%llx)\n", reqinfo->hash);
#endif
	}
}

void
unregister_hash(sqlite3 *db, char *buf, struct sockaddr_in *from_addr)
{
	tracker_unregister_t	*req = (tracker_unregister_t *)buf;
	int			i;

#ifdef	DEBUG
	fprintf(stderr, "unregister_hash:enter\n");
	fprintf(stderr, "unregister_hash:hash_table:before\n\n");
	dumpTables(db);
#endif
	for (i = 0 ; i < req->numhashes ; ++i)
	{
		tracker_info_t	*info = &req->info[i];
		deleteHash(db, info->hash) ;
	}
#ifdef	DEBUG
	fprintf(stderr, "unregister_hash:hash_table:after\n\n");
	dumpTables(db);
#endif
}

void
unregister_all(sqlite3 *db, char *buf, struct sockaddr_in *from_addr)
{
#ifdef	DEBUG
	fprintf(stderr, "unregister_all:enter\n");
	fprintf(stderr, "unregister_all:hash_table:before\n\n");
	dumpTables(db);
#endif

	deleteHost(db,(int) from_addr->sin_addr.s_addr);
	garbageCollect(db);

#ifdef	DEBUG
	fprintf(stderr, "unregister_all:hash_table:after\n\n");
	dumpTables(db);
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
struct timeval		start_time, end_time;
unsigned long long	s, e;
sqlite3  *db;
tracker_lookup_req_t	*req;


	if ((sockfd = init_tracker_comm(TRACKER_PORT)) < 0) {
		fprintf(stderr, "main:init_tracker_comm:failed\n");
		abort();
	}

	if ((init_db(&db)) != 0) {
		fprintf(stderr, "main:init_hash_table:failed\n");
		abort();
	}

	fprintf(stderr, "main:starting\n");
	fprintf(stderr, "main:builton %s\n", builton);

	/*
	 * needed for shuffle()
	 */
	srand(time(NULL));

	done = 0;
	while (!done) {
		from_addr_len = sizeof(from_addr);
		recvbytes = tracker_recv(sockfd, buf, sizeof(buf),
			(struct sockaddr *)&from_addr, &from_addr_len, NULL);

		if (recvbytes > 0) {
			tracker_header_t	*p;

			p = (tracker_header_t *)buf;

			gettimeofday(&start_time, NULL);
#ifdef	TIMEIT
#endif

			fprintf(stderr, "%lld : main:op %d from %s seqno %d\n",
				(long long int)start_time.tv_sec, p->op,
				inet_ntoa(from_addr.sin_addr), p->seqno);
#ifdef	DEBUG
#endif

			switch(p->op) {
			case LOOKUP:
				
				addHost(db, (int) from_addr.sin_addr.s_addr);
				req = (tracker_lookup_req_t *)buf;
				dolookup(db, sockfd, req->hash,
						req->header.seqno, &from_addr);
				break;

			case REGISTER:
				register_hash(db, buf, &from_addr);
				break;

			case UNREGISTER:
				unregister_hash(db, buf, &from_addr);
				break;

			case PEER_DONE:
				unregister_all(db, buf, &from_addr);
				break;

			case STOP_SERVER:
				fprintf(stderr,
					"Received 'STOP_SERVER' from (%s)\n",
					inet_ntoa(from_addr.sin_addr));
				exit(0);

			case DUMP_TABLES:
				fprintf(stderr,
					"Received 'DUMP_TABLES' from (%s)\n",
					inet_ntoa(from_addr.sin_addr));
				dumpTables(db);
				break;

			default:
				fprintf(stderr, "Unknown op (%d)\n", p->op);
				abort();
				break;
			}

			gettimeofday(&end_time, NULL);
			s = (start_time.tv_sec * 1000000) + start_time.tv_usec;
			e = (end_time.tv_sec * 1000000) + end_time.tv_usec;
			fprintf(stderr, "main:svc time: %lld\n", (e - s));
		}
	}

	return(0);
}

