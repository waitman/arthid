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

#define SPPD_IDENT		"hid_sppd"
#define SPPD_BUFFER_SIZE	1024
#define max(a, b)		(((a) > (b))? (a) : (b))
#define FIFO_NAME 		"/tmp/ain"
#define	REPORTID_KEYBD		1

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
  switch (press)
  {
	  case	'-':	++u; //Return=> code 40 ENTER
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
	  case	'Z':	++u;
	  case	'Y':	++u;
	  case	'X':	++u;
	  case	'W':	++u;
	  case	'V':	++u;
	  case	'U':	++u;
	  case	'T':	++u;
	  case	'S':	++u;
	  case	'R':	++u;
	  case	'Q':	++u;
	  case	'P':	++u;
	  case	'O':	++u;
	  case	'N':	++u;
	  case	'M':	++u;
	  case	'L':	++u;
	  case	'K':	++u;
	  case	'J':	++u;
	  case	'I':	++u;
	  case	'H':	++u;
	  case	'G':	++u;
	  case	'F':	++u;
	  case	'E':	++u;
	  case	'D':	++u;
	  case	'C':	++u;
	  case	'B':	++u;
	  case	'A':	u +=3;	// A =>  4
  }
  return (u);
}

char
shiftkey(char press)
{
  switch (press)
  {
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
	int			 controlsock,intrsock,clilen,controlsockfd,intrsockfd;
	bdaddr_t		 bt_addr_any;
	sdp_sp_profile_t	 sp;
	void			*ss;
	uint32_t		 sdp_handle;

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
	struct sockaddr_l2cap	l2addr, cli_addr;

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

	printf("control socket created\n");
	
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

	printf("interrupt socket created\n");
	printf("waiting for connect\n");
	
	clilen = sizeof(cli_addr);
	controlsockfd = accept(controlsock, (struct sockaddr *) &cli_addr, &clilen);
	intrsockfd = accept(intrsock, (struct sockaddr *) &cli_addr, &clilen);	

	printf("Connected to %s\n",bt_ntoa(&cli_addr.l2cap_bdaddr, NULL));
	printf("waiting for fifo.. press Control-C to exit.\n");
	fflush(stdout);
	
	char sfifo[300];
	int numfifo, fdfifo, i;
	mknod(FIFO_NAME, S_IFIFO | 0666, 0);
	
	char	hidrep[32];
	struct hidrep_keyb_t  * vkeyb  = (void *)hidrep;
	
	/* empty struct for key release */
	struct hidrep_keyb_t  * releasekey  = (void *)hidrep;
	releasekey->btcode = 0xA1;
	releasekey->rep_id = REPORTID_KEYBD;
	releasekey->modify = 0x00;
	releasekey->pad = 0x00;
	releasekey->key[0] = 0x00;
	releasekey->key[1] = 0x00;
	releasekey->key[2] = 0x00;
	releasekey->key[3] = 0x00;
	releasekey->key[4] = 0x00;
	releasekey->key[5] = 0x00;
	
	while (1==1) {
	  fdfifo = open(FIFO_NAME, O_RDONLY);
      do {
        if ((numfifo = read(fdfifo, sfifo, 300)) == -1)
	{
            /* oh no */
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
	      send ( intrsockfd, vkeyb,sizeof(struct hidrep_keyb_t), MSG_NOSIGNAL );
	      printf("sent data %d %lu\n",vkeyb->key[0],sizeof(struct hidrep_keyb_t));
	      send ( intrsockfd, releasekey,sizeof(struct hidrep_keyb_t), MSG_NOSIGNAL );
	      printf("sent release\n");
	      
	      fflush(stdout);
	    }
        }
      } while (numfifo>0);
	close(fdfifo);
	send ( intrsockfd, releasekey,sizeof(struct hidrep_keyb_t), MSG_NOSIGNAL );
	      printf("sent release\n");
	} /* forever */
      
      remove(FIFO_NAME);
      return (0);
}


