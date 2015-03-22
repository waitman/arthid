/*
 * hid_sppd.c
 */

/*-
 * Waitman Gobble <ns@waitman.net> (based on code by -->
 * Copyright (c) 2003 Maksim Yevmenkin <m_evmenkin@yahoo.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <bluetooth.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <paths.h>
#include <sdp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <termios.h>
#include <unistd.h>
#include <libutil.h>
#include <sys/queue.h>

#define FIFO_NAME 		"/tmp/ain"
#define HID_SPPD_PID		"/var/run/hid_sppd.pid"
#define	REPORTID_KEYBD		1

int controlsockfd,intrsockfd,is_connected,key_delay;
struct pidfh *pfh;
char fifo_name[_POSIX_PATH_MAX];

void client_kbd(int controlsock, int intrsock);
void handle_signal(int signal);
static void usage(void);

struct hidrep_keyb_t
{
  unsigned char	btcode; 
  unsigned char	rep_id; 
  unsigned char	modify; 
  unsigned char pad;
  unsigned char	key[6]; 
} __attribute((packed));

char
retkey(char press)
{
  char u = 1;
//  syslog(LOG_ERR,"Press %d",press);
  switch (press)
  {
 
	case	0x2F:	++u; 	/*/*/
	case	0x2E:	++u; 	
	case	',':	++u; 	
	case	0x60:	++u; 	/*`*/
	case	0x27:	++u; 	/*'*/
	case 	':':		/*need shift*/
	case	';':	++u; 	
			++u; 	/*bonus #102 key not implemented*/
	case	0x5C:	++u; 	/*\*/
	case	']':		/*need shift*/
	case	'}':	++u; 	
	case 	'[':		/*need shift*/
	case	'{':	++u; 	
	case 	'+':		/*need shift*/
	case	'=':	++u; 	
	case 	'_':		/*need shift*/
	case	'-':	++u; 	/*-*/
	case	' ':	++u; 	/*space*/
	case	0x09:	++u; 	/*tab*/
	case	0x08:	++u; 	/*backspace*/
	case	0x1B:	++u; 	/*escape*/
	case	0x0A:	++u; 	/*newline = enter*/
	case	'0':	++u;
	case	'9':	++u;
	case	'8':	++u;
	case	'7':	++u;
	case	'6':	++u;
	case	'5':	++u;
	case	'4':	++u;
	case	'3':	++u;
	case	'2':	++u;
	case	'1':	++u;
	case	'z':
	case	'Z':	++u;
	case	'y':
	case	'Y':	++u;
	case	'x':
	case	'X':	++u;
	case	'w':
	case	'W':	++u;
	case	'v':
	case	'V':	++u;
	case	'u':
	case	'U':	++u;
	case	't':
	case	'T':	++u;
	case	's':
	case	'S':	++u;
	case	'r':
	case	'R':	++u;
	case	'q':
	case	'Q':	++u;
	case	'p':
	case	'P':	++u;
	case	'o':
	case	'O':	++u;
	case	'n':
	case	'N':	++u;
	case	'm':
	case	'M':	++u;
	case	'l':
	case	'L':	++u;
	case	'k':
	case	'K':	++u;
	case	'j':
	case	'J':	++u;
	case	'i':
	case	'I':	++u;
	case	'h':
	case	'H':	++u;
	case	'g':
	case	'G':	++u;
	case	'f':
	case	'F':	++u;
	case	'e':
	case	'E':	++u;
	case	'd':
	case	'D':	++u;
	case	'c':
	case	'C':	++u;
	case	'b':
	case	'B':	++u;
	case	'a':
	case	'A':	u +=3;	// A =>  4
  }
  return (u);
}

char
shiftkey(char press)
{
  switch (press)
  {
	case 	':':
	case	']':
	case 	'[':
	case 	'+':
	case 	'_':
	case	'Z':	
	case	'Y':	
	case	'X':	
	case	'W':	
	case	'V':	
	case	'U':	
	case	'T':	
	case	'S':	
	case	'R':	
	case	'Q':	
	case	'P':	
	case	'O':	
	case	'N':	
	case	'M':	
	case	'L':	
	case	'K':	
	case	'J':	
	case	'I':	
	case	'H':	
	case	'G':	
	case	'F':	
	case	'E':	
	case	'D':	
	case	'C':	
	case	'B':	
	case	'A':	
	      return (0x2);
	      break;
	  default:
	      return(0x00);
	      break;
  }
}

/* Main */
int
main(int argc, char *argv[]) 
{
	
	int			 channel;
	int			 controlsock,intrsock;
	bdaddr_t		 bt_addr_any;
	sdp_sp_profile_t	 sp;
	void			*ss;
	uint32_t		 sdp_handle;
	struct sigaction	 sa;
	int			custom_pid=0;
	char			*pidfile;
	is_connected=0;
	key_delay=10000; /* 10 ms default */

	
	int32_t			 opt;

	strncpy ( fifo_name, FIFO_NAME, sizeof(fifo_name) );
	
	while ((opt = getopt(argc, argv, "P:d:F:h")) != -1) {
		switch (opt) {
		case 'P': /* set pid file */
			pidfile = optarg;
			syslog(LOG_ERR,"Using PID File %s", pidfile);
			custom_pid = 1;
			break;

		case 'd': /* delay between keystrokes in milliseconds */
			key_delay = atoi(optarg) * 1000;
			syslog(LOG_ERR,"Setting Keystroke Delay %d microseconds",key_delay); 
			break;

		case 'F': /* FIFO buffer */
			strncpy ( fifo_name, optarg, sizeof(fifo_name) );
			syslog(LOG_ERR,"Using FIFO Buffer %s",fifo_name);
			break;
			
		case 'h': /* help me */
			usage();
			break;
		}
	}

	
	pid_t mypid;
	if (custom_pid)
	{
		pfh = pidfile_open(pidfile, 0600, &mypid);
	} else {
		pfh = pidfile_open(HID_SPPD_PID, 0600, &mypid);
	}
	if	(pfh ==	NULL) {
		if	(errno == EEXIST)
			errx(EXIT_FAILURE,	"Daemon	already	running, pid: %d.", mypid);
		warn("Cannot open or create pidfile");
	}


	/* Set signal handlers */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handle_signal;

	if (sigaction(SIGTERM, &sa, NULL) < 0)
		err(1, "Could not sigaction(SIGTERM)");
 
	if (sigaction(SIGHUP, &sa, NULL) < 0)		/* SIGHUP will disconnect client and wait for new connections */
		err(1, "Could not sigaction(SIGHUP)");
 
	if (sigaction(SIGINT, &sa, NULL) < 0)
		err(1, "Could not sigaction(SIGINT)");

	sa.sa_handler = SIG_IGN;
	sa.sa_flags = SA_NOCLDWAIT;

	if (sigaction(SIGCHLD, &sa, NULL) < 0)
		err(1, "Could not sigaction(SIGCHLD)");
	
	memcpy(&bt_addr_any, NG_HCI_BDADDR_ANY, sizeof(bt_addr_any));

	channel = 17;

	ss = sdp_open_local(NULL);
	if (ss == NULL)
			errx(1, "Unable to create local SDP session");
	if (sdp_error(ss) != 0)
			errx(1, "Unable to open local SDP session. %s (%d)",
			    strerror(sdp_error(ss)), sdp_error(ss));
	memset(&sp, 0, sizeof(sp));
	sp.server_channel = channel;

	if (sdp_register_service(ss, SDP_SERVICE_CLASS_HUMAN_INTERFACE_DEVICE,
				&bt_addr_any, (void *)&sp, sizeof(sp),
				&sdp_handle) != 0) {
			errx(1, "Unable to register LAN service with "
			    "local SDP daemon. %s (%d)",
			    strerror(sdp_error(ss)), sdp_error(ss));
		}


	/* Create interrupt socket */
	struct sockaddr_l2cap	l2addr;

	controlsock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BLUETOOTH_PROTO_L2CAP);
	
	l2addr.l2cap_len = sizeof(l2addr);
	l2addr.l2cap_family = AF_BLUETOOTH;
	memcpy(&l2addr.l2cap_bdaddr, &bt_addr_any, sizeof(l2addr.l2cap_bdaddr));
	l2addr.l2cap_psm = htole16(0x11);

	if (bind(controlsock, (struct sockaddr *) &l2addr, sizeof(l2addr)) < 0) {
		syslog(LOG_ERR, "Could not bind control L2CAP socket. " \
			"%s (%d)", strerror(errno), errno);
		return (-1);
	}


	if (listen(controlsock, 10) < 0) {
		syslog(LOG_ERR, "Could not listen on control L2CAP socket. " \
			"%s (%d)", strerror(errno), errno);
		return (-1);
	}

	/* Create interrupt socket */
	intrsock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BLUETOOTH_PROTO_L2CAP);

	l2addr.l2cap_psm = htole16(0x13);

	if (bind(intrsock, (struct sockaddr *) &l2addr, sizeof(l2addr)) < 0) {
		syslog(LOG_ERR, "Could not bind interrupt L2CAP socket. " \
			"%s (%d)", strerror(errno), errno);
		return (-1);
	}

	if (listen(intrsock, 10) < 0) {
		syslog(LOG_ERR, "Could not listen on interrupt L2CAP socket. "\
			"%s (%d)", strerror(errno), errno);
		return (-1);
	}

	mknod(fifo_name, S_IFIFO | 0666, 0);
	
	if (daemon(0, 0) == -1)
	{
	     warn("Cannot daemonize");
	     pidfile_remove(pfh);
	     exit(EXIT_FAILURE);
	}
	
	pidfile_write(pfh);
	syslog(LOG_ERR,"Starting");
	
	while (1==1)				/* forever accept clients (one at a time) */
	{
		client_kbd(controlsock,intrsock);
	}

	return (0);
}


void 
client_kbd(int controlsock, int intrsock)
{

	int 				clilen,i,numfifo,fdfifo;	
	struct sockaddr_l2cap		cli_addr;
	char sfifo[300];
	
	clilen = sizeof(cli_addr);
	controlsockfd = accept(controlsock, (struct sockaddr *) &cli_addr, &clilen);
	intrsockfd = accept(intrsock, (struct sockaddr *) &cli_addr, &clilen);	

	is_connected=1;
	char	hidrep[32];
	struct hidrep_keyb_t  * vkeyb  = (void *)hidrep;

	syslog(LOG_ERR,"Client Connected %s",bt_ntoa(&cli_addr.l2cap_bdaddr, NULL));
	
	while (1==1) {
		fdfifo = open(fifo_name, O_RDONLY);
		do {
			if (!is_connected) {
				goto SHUTDOWN;
			}
			if ((numfifo = read(fdfifo, sfifo, 300)) == -1)
			{
				goto SHUTDOWN;
			} else {
				sfifo[numfifo] = '\0';
				for (i=0;i<numfifo-1;i++)
				{
					vkeyb->btcode = 0xA1;
					vkeyb->rep_id = REPORTID_KEYBD;
					vkeyb->modify = shiftkey(sfifo[i]);
					vkeyb->pad = 0x00;
					vkeyb->key[0] = retkey(sfifo[i]); /* key press */
					vkeyb->key[1] = 0x00;
					vkeyb->key[2] = 0x00;
					vkeyb->key[3] = 0x00;
					vkeyb->key[4] = 0x00;
					vkeyb->key[5] = 0x00;
					if (send ( intrsockfd, vkeyb,
						sizeof(struct hidrep_keyb_t), MSG_NOSIGNAL ) < 0)
					{
						goto SHUTDOWN;
					}
					usleep(key_delay); 
					
					vkeyb->btcode = 0xA1;		/*release*/
					vkeyb->rep_id = REPORTID_KEYBD;
					vkeyb->modify = 0x00;
					vkeyb->pad = 0x00;
					vkeyb->key[0] = 0x00; 		/* key press */
					vkeyb->key[1] = 0x00;
					vkeyb->key[2] = 0x00;
					vkeyb->key[3] = 0x00;
					vkeyb->key[4] = 0x00;
					vkeyb->key[5] = 0x00;
					if (send ( intrsockfd, vkeyb,
						sizeof(struct hidrep_keyb_t), MSG_NOSIGNAL ) < 0)
					{
						goto SHUTDOWN;
					}
					usleep(key_delay); 
					
				}
			}
		} while (numfifo>0);
		if (!is_connected) {
			goto SHUTDOWN;
		}
		close(fdfifo);
	} /* forever */
SHUTDOWN:
	close(fdfifo);
	close(controlsockfd);
	close(intrsockfd);
}

void 
handle_signal(int signal) {
    switch (signal) {
        case SIGHUP:
		is_connected=0;
		syslog(LOG_ERR,"HUP - Closing Client Connections");
		break;
	case SIGTERM:
        case SIGINT:
		syslog(LOG_ERR,"Shutting Down");
		remove(fifo_name);
		pidfile_remove(pfh);
		exit(EXIT_SUCCESS);
    }
}

/* Display usage and exit */
static void 
usage(void)
{
	fprintf(stdout,
"Usage: hid_sppd options\n" \
"Where options are:\n" \
"\t-P        PID File (default /var/run/hid_sppd.pid)\n" \
"\t-F        FIFO buffer (default /tmp/ain)\n" \
"\t-d        Keystroke delay in milliseconds (default 10ms)\n" \
"\t-h         Display this message\n");
	exit(EXIT_SUCCESS);
}