#include <string.h>
#include <stdio.h>
#include "sqlite3.h"


/* This routine executes a simple sql statement */
void sql_stmt(sqlite3 *db, const char* stmt) {
  char *errmsg;
  int   ret;

  ret = sqlite3_exec(db, stmt, 0, 0, &errmsg);

  if(ret != SQLITE_OK) {
    printf("Error in statement: %s [%s].\n", stmt, errmsg);
  }
}

/* Housekeeping/Wrapping Functions */
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
int hostid = 0;
	sprintf(sqlStmt, "SELECT hostid FROM hosts WHERE IP=%d", ip);
	return getIntValue(db,sqlStmt);
}

/* --- add a host  --- */
int addHost(sqlite3 *db, int ip) {
char sqlStmt[256];
int hostid = 0;
	if (hostid = hostExists(db, ip))
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

/* --- check if a hash exists in the hashes table --- */
int hashExists(sqlite3 *db, char *hash) {
char sqlStmt[256];
int hostid = 0;
	sprintf(sqlStmt, "SELECT hashid FROM hashes WHERE hash='%s'", hash);
	return getIntValue(db,sqlStmt);
}

/* --- add a hash --- */
int addHash(sqlite3 *db, char *hash) {
char sqlStmt[256];
int hashid = 0;
	if (hashid = hashExists(db, hash)) 
		return hashid;
	sprintf(sqlStmt, "INSERT INTO hashes(hashid,hash) VALUES(NULL,'%s')", hash);
	sql_stmt(db,sqlStmt);
	return hashExists(db,hash);
}

/* ---delete Hash  --- */
int deleteHash(sqlite3 *db, char *hash) {
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


/* --- registerPeer --- */
int registerPeer(sqlite3 *db, char *hash, int ip) {
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
/* --- get peers --- */
int getPeers(sqlite3 *db, char *hash) {
char sqlStmt[256];
sqlite3_stmt *preppedStmt; 
int id, ip, rcode = SQLITE_OK;
int sqlCode;
	sprintf(sqlStmt, "select IP,hashid as sequence from peers inner join hashes using(hashid) inner join hosts using(hostid) where peers.state='r' and hash='%s'", hash);
	if (prep_stmt(db, sqlStmt, &preppedStmt) == SQLITE_OK)
	{
		while ( (sqlCode = sqlite3_step(preppedStmt))  == SQLITE_ROW)
		{
			ip = sqlite3_column_int(preppedStmt,0);
			id = sqlite3_column_int(preppedStmt,1);
			printf("0x%08x,%d \n", ip, id);
		}
		if (sqlCode != SQLITE_DONE)
		{
			rcode=sqlCode;
			printf ("error in getPeers (%d)\n", rcode);
		}
		sqlite3_finalize(preppedStmt);
	}
	return rcode; 
}

/* --- Delete Hosts and Hashes that are no longer referenced in Peers Table */
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
int dumpTables(sqlite3 *db) {
char sqlStmt[256];
sqlite3_stmt *preppedStmt; 
int hashid, groupid, id, ip, hostid, rcode = SQLITE_OK;
const char *hash, *state;
int sqlCode;
	sprintf(sqlStmt, "select hostid,ip,groupid from hosts");
	if (prep_stmt(db, sqlStmt, &preppedStmt) == SQLITE_OK)
	{
		printf("\nID\t|IP\t|GROUPID\n");
		while ( (sqlCode = sqlite3_step(preppedStmt))  == SQLITE_ROW)
		{
			hostid = sqlite3_column_int(preppedStmt,0);
			ip = sqlite3_column_int(preppedStmt,1);
			groupid = sqlite3_column_int(preppedStmt,2);
			printf("%d\t|0x%08x\t|%d \n",hostid, ip, groupid);
		}
		sqlite3_finalize(preppedStmt);
	}
	sprintf(sqlStmt, "SELECT hashid,hash FROM hashes");
	if (prep_stmt(db, sqlStmt, &preppedStmt) == SQLITE_OK)
	{
		printf("\nID\t|HASH\n");
		while ( (sqlCode = sqlite3_step(preppedStmt))  == SQLITE_ROW)
		{
			hashid = sqlite3_column_int(preppedStmt,0);
			hash = sqlite3_column_text(preppedStmt,1);
			printf("%d\t|%s\n",hashid, hash);
		}
		sqlite3_finalize(preppedStmt);
	}

	sprintf(sqlStmt, "SELECT hashes.hash,hosts.ip,peers.state FROM peers INNER JOIN hashes USING(hashid) INNER JOIN hosts USING(hostid) ORDER by hashes.hash" );
	if (prep_stmt(db, sqlStmt, &preppedStmt) == SQLITE_OK)
	{
		printf("\nHASH\t|IP\t|STATE\n");
		while ( (sqlCode = sqlite3_step(preppedStmt))  == SQLITE_ROW)
		{
			hash = sqlite3_column_text(preppedStmt,0);
			ip = sqlite3_column_int(preppedStmt,1);
			state = sqlite3_column_text(preppedStmt,2);
			printf("%s\t|0x%08x\t|%s\n",hash, ip, state);
		}
		sqlite3_finalize(preppedStmt);
	}
}
/* --- populate --- */
/* testing data */
int populate(sqlite3 *db)
{
	
	sql_stmt(db,"insert into hosts(hostid,IP,GROUPID) values(NULL,167903230,0)");
	sql_stmt(db,"insert into hosts(hostid,IP,GROUPID) values(NULL,167903229,0)");
	sql_stmt(db,"insert into hosts(hostid,IP,GROUPID) values(NULL,167903228,0)");
	sql_stmt(db,"insert into hosts(hostid,IP,GROUPID) values(NULL,167903227,0)");
	sql_stmt(db,"insert into hosts(hostid,IP,GROUPID) values(NULL,167903226,0)");
	sql_stmt(db,"insert into hosts(hostid,IP,GROUPID) values(NULL,167903225,0)");
	sql_stmt(db,"insert into hosts(hostid,IP,GROUPID) values(NULL,167903224,0)");
	sql_stmt(db,"insert into hosts(hostid,IP,GROUPID) values(NULL,167903223,0)");

	sql_stmt(db,"insert into hashes(HASHID,HASH) values(NULL,'file1')");
	sql_stmt(db,"insert into hashes(HASHID,HASH) values(NULL,'file2')");
	sql_stmt(db,"insert into hashes(HASHID,HASH) values(NULL,'file3')");
	sql_stmt(db,"insert into hashes(HASHID,HASH) values(NULL,'file4')");
	sql_stmt(db,"insert into hashes(HASHID,HASH) values(NULL,'file55')");
	sql_stmt(db,"insert into hashes(HASHID,HASH) values(NULL,'file6')");
	sql_stmt(db,"insert into hashes(HASHID,HASH) values(NULL,'file7')");
	sql_stmt(db,"insert into hashes(HASHID,HASH) values(NULL,'file8')");
	sql_stmt(db,"insert into hashes(HASHID,HASH) values(NULL,'file9')");
	sql_stmt(db,"insert into hashes(HASHID,HASH) values(NULL,'file10')");
	sql_stmt(db,"insert into peers(HASHID,HOSTID,STATE) values(1,1,'r')");
	sql_stmt(db,"insert into peers(HASHID,HOSTID,STATE) values(1,2,'r')");
	sql_stmt(db,"insert into peers(HASHID,HOSTID,STATE) values(1,5,'r')");
	sql_stmt(db,"insert into peers(HASHID,HOSTID,STATE) values(1,7,'r')");
	sql_stmt(db,"insert into peers(HASHID,HOSTID,STATE) values(2,1,'r')");
	sql_stmt(db,"insert into peers(HASHID,HOSTID,STATE) values(2,4,'r')");
	sql_stmt(db,"insert into peers(HASHID,HOSTID,STATE) values(2,3,'r')");
	sql_stmt(db,"insert into peers(HASHID,HOSTID,STATE) values(3,4,'r')");
	sql_stmt(db,"insert into peers(HASHID,HOSTID,STATE) values(3,3,'r')");
	sql_stmt(db,"insert into peers(HASHID,HOSTID,STATE) values(10,3,'r')");
	sql_stmt(db,"insert into peers(HASHID,HOSTID,STATE) values(10,4,'r')");
	return 0;
}
int main()
{
sqlite3* db;
int hostip;
char hash[256];
int id;
int cmd;
int yesno;

	sqlite3_open(":memory:", &db);
	if(db == 0) {
		printf("Could not open database.");
		return 1;
	}

	/* create the tracker schema */
	sql_stmt(db, "create table hosts (hostid INTEGER PRIMARY KEY, ip INTEGER, groupid INT)");
	sql_stmt(db, "create table hashes (hashid INTEGER PRIMARY KEY, hash TEXT)");
	sql_stmt(db, "create table peers (hashid INTEGER, hostid INTEGER, state TEXT)");

	populate(db);

	do {
		cmd = -1;
		do 
		{
			printf(" 1) ID of IP address\n");
			printf(" 2) ID of Hash\n");
			printf(" 3) Peers for Hash\n");
			printf(" 4) Delete Host\n");
			printf(" 5) Delete Hash\n");
			printf(" 6) Add Host\n");
			printf(" 7) Add Hash\n");
			printf(" 8) Register Peer\n");
			printf(" 9) Dump DB\n");
			printf(" 0) Quit\n");
			printf("    selection (1-4): ");
			scanf("%d",&cmd);
			if ( !(cmd >= 0 && cmd <= 9))
				cmd = -1;
		}
		while (cmd < 0);
		
		switch (cmd) 
		{

			case 1:
				printf("Enter IP (Hex): ");
				scanf("%x",&hostip);
				if (id = hostExists(db,hostip))
					printf ("IP (0x%08x) found with index %d\n",hostip,id);
				else
					printf ("(0x%08x) NOT found\n",hostip);
				break;
			case 2:
				printf("Enter hash: ");
				scanf("%s",hash);
				if (id = hashExists(db,hash))
					printf ("Hash (%s) found with index %d\n",hash,id);
				else
					printf ("(%s) NOT found\n",hash);
				break;

			case 3:
				printf("Enter hash: ");
				scanf("%s",hash);
				if (id = hashExists(db,hash))
					getPeers(db,hash);
				else
					printf ("(%s) NOT found\n",hash);
				break;

			case 4:
				printf("Enter IP (Hex): ");
				scanf("%x",&hostip);
				deleteHost(db,hostip);
				break;
			case 5:
				printf("Enter hash: ");
				scanf("%s",hash);
				deleteHash(db,hash);
				break;

			case 6:
				printf("Enter IP (Hex): ");
				scanf("%x",&hostip);
				addHost(db,hostip);
				break;
			case 7:
				printf("Enter hash: ");
				scanf("%s",hash);
				addHash(db,hash);
				break;
			case 8:
				printf("Enter hash: ");
				scanf("%s",hash);
				printf("Enter IP (Hex): ");
				scanf("%x",&hostip);
				registerPeer(db,hash,hostip);
				break;
			case 9:
				printf("Garbage Collect? [0/1]");
				scanf("%d", &yesno);
				if (yesno)
					garbageCollect(db);
				dumpTables(db);
				break;
		}
	}
	while ( cmd != 0 );

  sqlite3_close(db);
  return 0;
}
