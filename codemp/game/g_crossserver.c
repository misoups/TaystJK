/*
 * g_crossserver.c — cross-server events for TaystJK
 *
 * Uses POSIX named FIFOs (one per server port) for same-machine IPC.
 * Each server owns /tmp/taystjk_cross_<port>.fifo, opened O_RDWR|O_NONBLOCK
 * so peer servers can write to it without blocking.
 *
 * Wire format (newline-terminated):
 *   src_port|hostname|type|name|detail
 *
 * Types: chat, join, quit, pb, wr
 *
 * Configure with:
 *   g_crossServerPorts "7007 7008"   (space/comma separated list of OTHER ports)
 *
 * Only compiled on Linux; Windows builds compile out the IPC code.
 */

#include "g_local.h"

#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define CS_FIFO_DIR    "/tmp/"
#define CS_FIFO_FMT    "/tmp/taystjk_cross_%d.fifo"
#define CS_MAX_MSG     512   // must be < PIPE_BUF (4096 on Linux) for atomic writes
#define CS_POLL_MS     500   // ms between incoming-message polls in G_RunFrame

static int  cs_fd            = -1;   // our own FIFO, O_RDWR|O_NONBLOCK
static int  cs_our_port      = 0;
static int  cs_last_poll_time = 0;

/* -------------------------------------------------------------------------- */

void G_CrossServerInit( void ) {
	char     path[256];
	vmCvar_t port;

	/* Close/clean up previous state if re-initialised without a shutdown
	 * (e.g. fast_restart / map_restart skips ShutdownGame). */
	if ( cs_fd >= 0 ) {
		close( cs_fd );
		cs_fd = -1;
	}
	if ( cs_our_port > 0 ) {
		Com_sprintf( path, sizeof(path), CS_FIFO_FMT, cs_our_port );
		unlink( path );
		cs_our_port = 0;
	}

	trap->Cvar_Register( &port, "net_port", "7006", CVAR_ROM );
	cs_our_port    = port.integer;
	cs_last_poll_time = 0;

	Com_sprintf( path, sizeof(path), CS_FIFO_FMT, cs_our_port );

	unlink( path ); /* remove stale FIFO from a previous crash */

	if ( mkfifo( path, 0666 ) < 0 ) {
		G_LogPrintf( "CrossServer: mkfifo(%s) failed: %s\n", path, strerror(errno) );
		return;
	}

	cs_fd = open( path, O_RDWR | O_NONBLOCK );
	if ( cs_fd < 0 ) {
		G_LogPrintf( "CrossServer: open(%s) failed: %s\n", path, strerror(errno) );
		unlink( path );
		return;
	}

	G_LogPrintf( "CrossServer: listening on port %d (%s)\n", cs_our_port, path );
}

void G_CrossServerShutdown( void ) {
	char path[256];

	if ( cs_fd >= 0 ) {
		close( cs_fd );
		cs_fd = -1;
	}
	if ( cs_our_port > 0 ) {
		Com_sprintf( path, sizeof(path), CS_FIFO_FMT, cs_our_port );
		unlink( path );
		cs_our_port = 0;
	}
}

/* Write msg to a specific peer port's FIFO.  Silent on failure (peer may be down). */
static void CS_WriteToPeer( int port, const char *msg, int len ) {
	char path[256];
	int  fd;

	if ( port == cs_our_port ) return;

	Com_sprintf( path, sizeof(path), CS_FIFO_FMT, port );
	fd = open( path, O_WRONLY | O_NONBLOCK );
	if ( fd < 0 ) return;

	write( fd, msg, len );
	close( fd );
}

/* Core sender: type is "chat", "join", "quit", "pb", or "wr". */
void G_CrossServerBroadcast( const char *type, const char *name, const char *detail ) {
	char  portsStr[256];
	char  hostname[64];
	char  cleanName[MAX_NETNAME];
	char  cleanDetail[CS_MAX_MSG];
	char  msg[CS_MAX_MSG];
	int   msgLen;
	char *p, *saveptr = NULL;

	trap->Cvar_VariableStringBuffer( "sv_hostname", hostname, sizeof(hostname) );
	Q_strncpyz( cleanName,   name,            sizeof(cleanName) );
	Q_strncpyz( cleanDetail, detail ? detail : "", sizeof(cleanDetail) );

	Q_strstrip( hostname,    "|",  "" );
	Q_strstrip( cleanName,   "|",  "" );
	Q_strstrip( cleanDetail, "|",  "" );
	Q_strstrip( cleanDetail, "\n", "" );
	Q_strstrip( cleanDetail, "\r", "" );

	msgLen = Com_sprintf( msg, sizeof(msg), "%d|%s|%s|%s|%s\n",
	                      cs_our_port, hostname, type, cleanName, cleanDetail );
	if ( msgLen <= 0 || msgLen >= (int)sizeof(msg) )
		return;

	G_CrossServerDisplay( cs_our_port, hostname, type, cleanName, cleanDetail );

	if ( cs_fd < 0 ) return;

	trap->Cvar_VariableStringBuffer( "g_crossServerPorts", portsStr, sizeof(portsStr) );
	if ( !portsStr[0] ) return;

	for ( p = strtok_r(portsStr, " ,", &saveptr); p; p = strtok_r(NULL, " ,", &saveptr) ) {
		int peer = atoi( p );
		if ( peer > 0 )
			CS_WriteToPeer( peer, msg, msgLen );
	}
}

/* Convenience wrapper kept for existing call sites. */
void G_CrossServerSay( gentity_t *ent, const char *text ) {
	char cleanText[MAX_SAY_TEXT];

	if ( !ent || !ent->client ) return;

	Q_strncpyz( cleanText, text, sizeof(cleanText) );
	Q_strstrip( cleanText, "\n", "" );
	Q_strstrip( cleanText, "\r", "" );

	G_CrossServerBroadcast( "chat", ent->client->pers.netname, cleanText );
}

/* Called from G_RunFrame (throttled).  Reads any pending incoming messages. */
void G_CrossServerPoll( int levelTime ) {
	char     buf[4096];
	ssize_t  n;
	char    *line, *end;
	char    *srcPort, *srcHost, *srcType, *srcName, *srcDetail;

	if ( cs_fd < 0 ) return;
	if ( levelTime - cs_last_poll_time < CS_POLL_MS ) return;
	cs_last_poll_time = levelTime;

	n = read( cs_fd, buf, sizeof(buf) - 1 );
	if ( n <= 0 ) return;
	buf[n] = '\0';

	line = buf;
	while ( *line ) {
		end = strchr( line, '\n' );
		if ( !end ) break;
		*end = '\0';

		/* Format: src_port|hostname|type|name|detail */
		srcPort   = line;
		srcHost   = strchr( srcPort, '|' );   if ( !srcHost )   { line = end + 1; continue; }
		*srcHost++ = '\0';
		srcType   = strchr( srcHost, '|' );   if ( !srcType )   { line = end + 1; continue; }
		*srcType++ = '\0';
		srcName   = strchr( srcType, '|' );   if ( !srcName )   { line = end + 1; continue; }
		*srcName++ = '\0';
		srcDetail = strchr( srcName, '|' );   if ( !srcDetail ) { line = end + 1; continue; }
		*srcDetail++ = '\0';

		G_CrossServerDisplay( atoi(srcPort), srcHost, srcType, srcName, srcDetail );

		line = end + 1;
	}
}

/* Format and broadcast a cross-server event to all local players. */
void G_CrossServerDisplay( int srcPort, const char *srcHost, const char *type,
                            const char *name, const char *detail ) {
	char cleanHost[64];
	char cmd[1024];

	(void)srcPort;

	Q_strncpyz( cleanHost, srcHost, sizeof(cleanHost) );
	Q_CleanStr( cleanHost );

	/* Per-server prefix:
	 *   ~ups Reloaded #1 -> ^2[~ups Reloaded #1]
	 *   ~ups Reloaded #2 -> ^5[~ups Reloaded #2]
	 * Any other server falls back to ^2[hostname]. */
	char prefix[80];
	if ( strstr(cleanHost, "Reloaded #1") )
		Q_strncpyz( prefix, "^2[~ups Reloaded #1]", sizeof(prefix) );
	else if ( strstr(cleanHost, "Reloaded #2") )
		Q_strncpyz( prefix, "^5[~ups Reloaded #2]", sizeof(prefix) );
	else
		Com_sprintf( prefix, sizeof(prefix), "^2[%s]", cleanHost );

	if ( !Q_stricmp(type, "connect") ) {
		G_LogPrintf( "cross_connect(%s): %s\n", cleanHost, name );
		Com_sprintf( cmd, sizeof(cmd),
		             "print \"%s ^3%s ^7connected\n\"",
		             prefix, name );
	} else if ( !Q_stricmp(type, "join") ) {
		G_LogPrintf( "cross_join(%s): %s\n", cleanHost, name );
		Com_sprintf( cmd, sizeof(cmd),
		             "print \"%s ^3%s ^7entered the game\n\"",
		             prefix, name );
	} else if ( !Q_stricmp(type, "quit") ) {
		G_LogPrintf( "cross_quit(%s): %s\n", cleanHost, name );
		Com_sprintf( cmd, sizeof(cmd),
		             "print \"%s ^3%s ^7left the game\n\"",
		             prefix, name );
	} else if ( !Q_stricmp(type, "run") ) {
		/* Only display on the receiving server — the originating server already
		 * showed it via PrintRaceTime. */
		if ( srcPort == cs_our_port )
			return;
		G_LogPrintf( "cross_run(%s): %s\n", cleanHost, detail );
		Com_sprintf( cmd, sizeof(cmd),
		             "print \"%s^7 %s\n\"",
		             prefix, detail );
	} else { /* chat */
		G_LogPrintf( "say_cross(%s): %s: %s\n", cleanHost, name, detail );
		if ( cleanHost[0] )
			Com_sprintf( cmd, sizeof(cmd), "chat \"%s ^7%s:^2 %s^7\"",
			             prefix, name, detail );
		else
			Com_sprintf( cmd, sizeof(cmd), "chat \"^2[CROSS] ^7%s:^2 %s^7\"",
			             name, detail );
	}

	trap->SendServerCommand( -1, cmd );
}

#else /* !__linux__ */

void G_CrossServerInit( void )                                                                              {}
void G_CrossServerShutdown( void )                                                                          {}
void G_CrossServerBroadcast( const char *type, const char *name, const char *detail )                      { (void)type; (void)name; (void)detail; }
void G_CrossServerSay( gentity_t *ent, const char *text )                                                   { (void)ent; (void)text; }
void G_CrossServerPoll( int levelTime )                                                                     { (void)levelTime; }
void G_CrossServerDisplay( int p, const char *h, const char *t, const char *n, const char *d )             { (void)p; (void)h; (void)t; (void)n; (void)d; }

#endif /* __linux__ */
