/*
 * Copyright (c) 1999, 2001 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */

#include "config.h"

#include <sys/types.h>
#include <sys/param.h>
#ifdef HAVE_FLOCK
# include <sys/file.h>
#endif /* HAVE_FLOCK */
#include <stdio.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <fcntl.h>
#include <time.h>
#ifdef HAVE_UTIME
# ifdef HAVE_UTIME_H
#  include <utime.h>
# endif /* HAVE_UTIME_H */
#else
# include "emul/utime.h"
#endif /* HAVE_UTIME */

#include "sudo.h"

#ifndef lint
static const char rcsid[] = "$Sudo$";
#endif /* lint */

/*
 * Update the access and modify times on a file.
 */
int
touch(path, when)
    char *path;
    time_t when;
{
#ifdef HAVE_UTIME_POSIX
    struct utimbuf ut, *utp;

    ut.actime = ut.modtime = when;
    utp = &ut;
#else
    /* BSD <= 4.3 has no struct utimbuf */
    time_t utp[2];

    utp[0] = utp[1] = when;
#endif /* HAVE_UTIME_POSIX */

    return(utime(path, utp));
}

/*
 * Lock/unlock a file.
 */
#ifdef HAVE_LOCKF
int
lock_file(fd, lockit)
    int fd;
    int lockit;
{
    int op = 0;

    switch (lockit) {
	case SUDO_LOCK:
	    op = F_LOCK;
	    break;
	case SUDO_TLOCK:
	    op = F_TLOCK;
	    break;
	case SUDO_UNLOCK:
	    op = F_ULOCK;
	    break;
    }
    return(lockf(fd, op, 0) == 0);
}
#elif HAVE_FLOCK
int
lock_file(fd, lockit)
    int fd;
    int lockit;
{
    int op = 0;

    switch (lockit) {
	case SUDO_LOCK:
	    op = LOCK_EX;
	    break;
	case SUDO_TLOCK:
	    op = LOCK_EX | LOCK_NB;
	    break;
	case SUDO_UNLOCK:
	    op = LOCK_UN;
	    break;
    }
    return(flock(fd, op) == 0);
}
#else
int
lock_file(fd, lockit)
    int fd;
    int lockit;
{
#ifdef F_SETLK
    int func;
    struct flock lock;

    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();
    lock.l_type = (lockit == SUDO_UNLOCK) ? F_UNLCK : F_WRLCK;
    lock.l_whence = SEEK_SET;
    func = (lockit == SUDO_TLOCK) ? F_SETLK : F_SETLKW;

    return(fcntl(fd, func, &lock) == 0);
#else
    return(TRUE);
#endif
}
#endif
