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

static int	sppd_read	(int fd, char *buffer, int size);
static int	sppd_write	(int fd, char *buffer, int size);


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


	struct sockaddr_l2cap	l2addr;

	controlsock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BLUETOOTH_PROTO_L2CAP);
	
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
	intrsock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BLUETOOTH_PROTO_L2CAP);

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
	
	printf("waiting.. press Control-C to exit.\n");
	
	
	while (1==1) { } 
	return (0);
}


/* Read data */
static int
sppd_read(int fd, char *buffer, int size)
{
	int	n;

again:
	n = read(fd, buffer, size);
	if (n < 0) {
		if (errno == EINTR)
			goto again;

		return (-1);
	}

	return (n);
} /* sppd_read */

/* Write data */
static int
sppd_write(int fd, char *buffer, int size)
{
	int	n, wrote;

	for (wrote = 0; size > 0; ) {
		n = write(fd, buffer, size);
		switch (n) {
		case -1:
			if (errno != EINTR)
				return (-1);
			break;

		case 0: 
			/* XXX can happen? */
			break;

		default:
			wrote += n;
			buffer += n;
			size -= n;
			break;
		}
	}

	return (wrote);
} /* sppd_write */


