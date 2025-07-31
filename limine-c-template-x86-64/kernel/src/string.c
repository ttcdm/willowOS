//using musl's

#include <string.h>
#include <stdint.h>
#include <limits.h>

#define SS (sizeof(size_t))
#define ALIGN (sizeof(size_t)-1)
#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#define HASZERO(x) ((x)-ONES & ~(x) & HIGHS)

void *memchr(const void *src, int c, size_t n)
{
	const unsigned char *s = src;
	c = (unsigned char)c;
#ifdef __GNUC__
	for (; ((uintptr_t)s & ALIGN) && n && *s != c; s++, n--);
	if (n && *s != c) {
		typedef size_t __attribute__((__may_alias__)) word;
		const word *w;
		size_t k = ONES * c;
		for (w = (const void *)s; n>=SS && !HASZERO(*w^k); w++, n-=SS);
		s = (const void *)w;
	}
#endif
	for (; n && *s != c; s++, n--);
	return n ? (void *)s : 0;
}





//HERE the most basic versions are used and they're in main.c. i'm not using these four for now because i'm scared that they'll break something somehow

// #include <string.h>

// int memcmp(const void *vl, const void *vr, size_t n)
// {
// 	const unsigned char *l=vl, *r=vr;
// 	for (; n && *l == *r; n--, l++, r++);
// 	return n ? *l-*r : 0;
// }

// #include <string.h>
// #include <stdint.h>
// #include <endian.h>

// void *memcpy(void *restrict dest, const void *restrict src, size_t n)
// {
// 	unsigned char *d = dest;
// 	const unsigned char *s = src;

// #ifdef __GNUC__

// #if __BYTE_ORDER == __LITTLE_ENDIAN
// #define LS >>
// #define RS <<
// #else
// #define LS <<
// #define RS >>
// #endif

// 	typedef uint32_t __attribute__((__may_alias__)) u32;
// 	uint32_t w, x;

// 	for (; (uintptr_t)s % 4 && n; n--) *d++ = *s++;

// 	if ((uintptr_t)d % 4 == 0) {
// 		for (; n>=16; s+=16, d+=16, n-=16) {
// 			*(u32 *)(d+0) = *(u32 *)(s+0);
// 			*(u32 *)(d+4) = *(u32 *)(s+4);
// 			*(u32 *)(d+8) = *(u32 *)(s+8);
// 			*(u32 *)(d+12) = *(u32 *)(s+12);
// 		}
// 		if (n&8) {
// 			*(u32 *)(d+0) = *(u32 *)(s+0);
// 			*(u32 *)(d+4) = *(u32 *)(s+4);
// 			d += 8; s += 8;
// 		}
// 		if (n&4) {
// 			*(u32 *)(d+0) = *(u32 *)(s+0);
// 			d += 4; s += 4;
// 		}
// 		if (n&2) {
// 			*d++ = *s++; *d++ = *s++;
// 		}
// 		if (n&1) {
// 			*d = *s;
// 		}
// 		return dest;
// 	}

// 	if (n >= 32) switch ((uintptr_t)d % 4) {
// 	case 1:
// 		w = *(u32 *)s;
// 		*d++ = *s++;
// 		*d++ = *s++;
// 		*d++ = *s++;
// 		n -= 3;
// 		for (; n>=17; s+=16, d+=16, n-=16) {
// 			x = *(u32 *)(s+1);
// 			*(u32 *)(d+0) = (w LS 24) | (x RS 8);
// 			w = *(u32 *)(s+5);
// 			*(u32 *)(d+4) = (x LS 24) | (w RS 8);
// 			x = *(u32 *)(s+9);
// 			*(u32 *)(d+8) = (w LS 24) | (x RS 8);
// 			w = *(u32 *)(s+13);
// 			*(u32 *)(d+12) = (x LS 24) | (w RS 8);
// 		}
// 		break;
// 	case 2:
// 		w = *(u32 *)s;
// 		*d++ = *s++;
// 		*d++ = *s++;
// 		n -= 2;
// 		for (; n>=18; s+=16, d+=16, n-=16) {
// 			x = *(u32 *)(s+2);
// 			*(u32 *)(d+0) = (w LS 16) | (x RS 16);
// 			w = *(u32 *)(s+6);
// 			*(u32 *)(d+4) = (x LS 16) | (w RS 16);
// 			x = *(u32 *)(s+10);
// 			*(u32 *)(d+8) = (w LS 16) | (x RS 16);
// 			w = *(u32 *)(s+14);
// 			*(u32 *)(d+12) = (x LS 16) | (w RS 16);
// 		}
// 		break;
// 	case 3:
// 		w = *(u32 *)s;
// 		*d++ = *s++;
// 		n -= 1;
// 		for (; n>=19; s+=16, d+=16, n-=16) {
// 			x = *(u32 *)(s+3);
// 			*(u32 *)(d+0) = (w LS 8) | (x RS 24);
// 			w = *(u32 *)(s+7);
// 			*(u32 *)(d+4) = (x LS 8) | (w RS 24);
// 			x = *(u32 *)(s+11);
// 			*(u32 *)(d+8) = (w LS 8) | (x RS 24);
// 			w = *(u32 *)(s+15);
// 			*(u32 *)(d+12) = (x LS 8) | (w RS 24);
// 		}
// 		break;
// 	}
// 	if (n&16) {
// 		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
// 		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
// 		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
// 		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
// 	}
// 	if (n&8) {
// 		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
// 		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
// 	}
// 	if (n&4) {
// 		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
// 	}
// 	if (n&2) {
// 		*d++ = *s++; *d++ = *s++;
// 	}
// 	if (n&1) {
// 		*d = *s;
// 	}
// 	return dest;
// #endif

// 	for (; n; n--) *d++ = *s++;
// 	return dest;
// }

// #include <string.h>
// #include <stdint.h>

// #ifdef __GNUC__
// typedef __attribute__((__may_alias__)) size_t WT;
// #define WS (sizeof(WT))
// #endif

// void *memmove(void *dest, const void *src, size_t n)
// {
// 	char *d = dest;
// 	const char *s = src;

// 	if (d==s) return d;
// 	if ((uintptr_t)s-(uintptr_t)d-n <= -2*n) return memcpy(d, s, n);

// 	if (d<s) {
// #ifdef __GNUC__
// 		if ((uintptr_t)s % WS == (uintptr_t)d % WS) {
// 			while ((uintptr_t)d % WS) {
// 				if (!n--) return dest;
// 				*d++ = *s++;
// 			}
// 			for (; n>=WS; n-=WS, d+=WS, s+=WS) *(WT *)d = *(WT *)s;
// 		}
// #endif
// 		for (; n; n--) *d++ = *s++;
// 	} else {
// #ifdef __GNUC__
// 		if ((uintptr_t)s % WS == (uintptr_t)d % WS) {
// 			while ((uintptr_t)(d+n) % WS) {
// 				if (!n--) return dest;
// 				d[n] = s[n];
// 			}
// 			while (n>=WS) n-=WS, *(WT *)(d+n) = *(WT *)(s+n);
// 		}
// #endif
// 		while (n) n--, d[n] = s[n];
// 	}

// 	return dest;
// }

// #include <string.h>
// #include <stdint.h>

// void *memset(void *dest, int c, size_t n)
// {
// 	unsigned char *s = dest;
// 	size_t k;

// 	/* Fill head and tail with minimal branching. Each
// 	 * conditional ensures that all the subsequently used
// 	 * offsets are well-defined and in the dest region. */

// 	if (!n) return dest;
// 	s[0] = c;
// 	s[n-1] = c;
// 	if (n <= 2) return dest;
// 	s[1] = c;
// 	s[2] = c;
// 	s[n-2] = c;
// 	s[n-3] = c;
// 	if (n <= 6) return dest;
// 	s[3] = c;
// 	s[n-4] = c;
// 	if (n <= 8) return dest;

// 	/* Advance pointer to align it at a 4-byte boundary,
// 	 * and truncate n to a multiple of 4. The previous code
// 	 * already took care of any head/tail that get cut off
// 	 * by the alignment. */

// 	k = -(uintptr_t)s & 3;
// 	s += k;
// 	n -= k;
// 	n &= -4;

// #ifdef __GNUC__
// 	typedef uint32_t __attribute__((__may_alias__)) u32;
// 	typedef uint64_t __attribute__((__may_alias__)) u64;

// 	u32 c32 = ((u32)-1)/255 * (unsigned char)c;

// 	/* In preparation to copy 32 bytes at a time, aligned on
// 	 * an 8-byte bounary, fill head/tail up to 28 bytes each.
// 	 * As in the initial byte-based head/tail fill, each
// 	 * conditional below ensures that the subsequent offsets
// 	 * are valid (e.g. !(n<=24) implies n>=28). */

// 	*(u32 *)(s+0) = c32;
// 	*(u32 *)(s+n-4) = c32;
// 	if (n <= 8) return dest;
// 	*(u32 *)(s+4) = c32;
// 	*(u32 *)(s+8) = c32;
// 	*(u32 *)(s+n-12) = c32;
// 	*(u32 *)(s+n-8) = c32;
// 	if (n <= 24) return dest;
// 	*(u32 *)(s+12) = c32;
// 	*(u32 *)(s+16) = c32;
// 	*(u32 *)(s+20) = c32;
// 	*(u32 *)(s+24) = c32;
// 	*(u32 *)(s+n-28) = c32;
// 	*(u32 *)(s+n-24) = c32;
// 	*(u32 *)(s+n-20) = c32;
// 	*(u32 *)(s+n-16) = c32;

// 	/* Align to a multiple of 8 so we can fill 64 bits at a time,
// 	 * and avoid writing the same bytes twice as much as is
// 	 * practical without introducing additional branching. */

// 	k = 24 + ((uintptr_t)s & 4);
// 	s += k;
// 	n -= k;

// 	/* If this loop is reached, 28 tail bytes have already been
// 	 * filled, so any remainder when n drops below 32 can be
// 	 * safely ignored. */

// 	u64 c64 = c32 | ((u64)c32 << 32);
// 	for (; n >= 32; n-=32, s+=32) {
// 		*(u64 *)(s+0) = c64;
// 		*(u64 *)(s+8) = c64;
// 		*(u64 *)(s+16) = c64;
// 		*(u64 *)(s+24) = c64;
// 	}
// #else
// 	/* Pure C fallback with no aliasing violations. */
// 	for (; n; n--, s++) *s = c;
// #endif

// 	return dest;
// }










#include <string.h>

char *strcat(char *restrict dest, const char *restrict src)
{
	strcpy(dest + strlen(dest), src);
	return dest;
}

#include <string.h>

char *__strchrnul(const char *s, int c)
{
	c = (unsigned char)c;
	if (!c) return (char *)s + strlen(s);

#ifdef __GNUC__
	typedef size_t __attribute__((__may_alias__)) word;
	const word *w;
	for (; (uintptr_t)s % ALIGN; s++)
		if (!*s || *(unsigned char *)s == c) return (char *)s;
	size_t k = ONES * c;
	for (w = (void *)s; !HASZERO(*w) && !HASZERO(*w^k); w++);
	s = (void *)w;
#endif
	for (; *s && *(unsigned char *)s != c; s++);
	return (char *)s;
}

char *strchr(const char *s, int c)
{
	char *r = __strchrnul(s, c);
	return *(unsigned char *)r == (unsigned char)c ? r : 0;
}

#include <string.h>

int strcmp(const char *l, const char *r)
{
	for (; *l==*r && *l; l++, r++);
	return *(unsigned char *)l - *(unsigned char *)r;
}

//trivial impl. taken from newlib
int strcoll(const char* s1, const char* s2) {
    return strcmp(s1, s2);
}

#include <string.h>

#define BITOP(a,b,op) \
 ((a)[(size_t)(b)/(8*sizeof *(a))] op (size_t)1<<((size_t)(b)%(8*sizeof *(a))))

size_t strcspn(const char *s, const char *c)
{
	const char *a = s;
	size_t byteset[32/sizeof(size_t)];

	if (!c[0] || !c[1]) return __strchrnul(s, *c)-a;

	memset(byteset, 0, sizeof byteset);
	for (; *c && BITOP(byteset, *(unsigned char *)c, |=); c++);
	for (; *s && !BITOP(byteset, *(unsigned char *)s, &); s++);
	return s-a;
}

/***
**** CAUTION!!! KEEP DOC CONSISTENT---if you change text of a message
****            here, change two places:
****            1) the leading doc section (alphabetized by macro)
****            2) the real text inside switch(errnum)
***/

/*
FUNCTION
	<<strerror>>, <<strerror_l>>---convert error number to string

INDEX
	strerror

INDEX
	strerror_l

SYNOPSIS
	#include <string.h>
	char *strerror(int <[errnum]>);
	char *strerror_l(int <[errnum]>, locale_t <[locale]>);
	char *_strerror_r(struct _reent <[ptr]>, int <[errnum]>,
			  int <[internal]>, int *<[error]>);

DESCRIPTION
<<strerror>> converts the error number <[errnum]> into a
string.  The value of <[errnum]> is usually a copy of <<errno>>.
If <<errnum>> is not a known error number, the result points to an
empty string.

<<strerror_l>> is like <<strerror>> but creates a string in a format
as expected in locale <[locale]>.  If <[locale]> is LC_GLOBAL_LOCALE or
not a valid locale object, the behaviour is undefined.

This implementation of <<strerror>> prints out the following strings
for each of the values defined in `<<errno.h>>':

o+
o 0
Success

o E2BIG
Arg list too long

o EACCES
Permission denied

o EADDRINUSE
Address already in use

o EADDRNOTAVAIL
Address not available

o EADV
Advertise error

o EAFNOSUPPORT
Address family not supported by protocol family

o EAGAIN
No more processes

o EALREADY
Socket already connected

o EBADF
Bad file number

o EBADMSG
Bad message

o EBUSY
Device or resource busy

o ECANCELED
Operation canceled

o ECHILD
No children

o ECOMM
Communication error

o ECONNABORTED
Software caused connection abort

o ECONNREFUSED
Connection refused

o ECONNRESET
Connection reset by peer

o EDEADLK
Deadlock

o EDESTADDRREQ
Destination address required

o EEXIST
File exists

o EDOM
Mathematics argument out of domain of function

o EFAULT
Bad address

o EFBIG
File too large

o EHOSTDOWN
Host is down

o EHOSTUNREACH
Host is unreachable

o EIDRM
Identifier removed

o EILSEQ
Illegal byte sequence

o EINPROGRESS
Connection already in progress

o EINTR
Interrupted system call

o EINVAL
Invalid argument

o EIO
I/O error

o EISCONN
Socket is already connected

o EISDIR
Is a directory

o ELIBACC
Cannot access a needed shared library

o ELIBBAD
Accessing a corrupted shared library

o ELIBEXEC
Cannot exec a shared library directly

o ELIBMAX
Attempting to link in more shared libraries than system limit

o ELIBSCN
<<.lib>> section in a.out corrupted

o EMFILE
File descriptor value too large

o EMLINK
Too many links

o EMSGSIZE
Message too long

o EMULTIHOP
Multihop attempted

o ENAMETOOLONG
File or path name too long

o ENETDOWN
Network interface is not configured

o ENETRESET
Connection aborted by network

o ENETUNREACH
Network is unreachable

o ENFILE
Too many open files in system

o ENOBUFS
No buffer space available

o ENODATA
No data

o ENODEV
No such device

o ENOENT
No such file or directory

o ENOEXEC
Exec format error

o ENOLCK
No lock

o ENOLINK
Virtual circuit is gone

o ENOMEM
Not enough space

o ENOMSG
No message of desired type

o ENONET
Machine is not on the network

o ENOPKG
No package

o ENOPROTOOPT
Protocol not available

o ENOSPC
No space left on device

o ENOSR
No stream resources

o ENOSTR
Not a stream

o ENOSYS
Function not implemented

o ENOTBLK
Block device required

o ENOTCONN
Socket is not connected

o ENOTDIR
Not a directory

o ENOTEMPTY
Directory not empty

o ENOTRECOVERABLE
State not recoverable

o ENOTSOCK
Socket operation on non-socket

o ENOTSUP
Not supported

o ENOTTY
Not a character device

o ENXIO
No such device or address

o EOPNOTSUPP
Operation not supported on socket

o EOVERFLOW
Value too large for defined data type

o EOWNERDEAD
Previous owner died

o EPERM
Not owner

o EPIPE
Broken pipe

o EPROTO
Protocol error

o EPROTOTYPE
Protocol wrong type for socket

o EPROTONOSUPPORT
Unknown protocol

o ERANGE
Result too large

o EREMOTE
Resource is remote

o EROFS
Read-only file system

o ESHUTDOWN
Can't send after socket shutdown

o ESOCKTNOSUPPORT
Socket type not supported

o ESPIPE
Illegal seek

o ESRCH
No such process

o ESRMNT
Srmount error

o ESTRPIPE
Strings pipe error

o ETIME
Stream ioctl timeout

o ETIMEDOUT
Connection timed out

o ETXTBSY
Text file busy

o EWOULDBLOCK
Operation would block (usually same as EAGAIN)

o EXDEV
Cross-device link

o-

<<_strerror_r>> is a reentrant version of the above.

RETURNS
This function returns a pointer to a string.  Your application must
not modify that string.

PORTABILITY
ANSI C requires <<strerror>>, but does not specify the strings used
for each error number.

<<strerror_l>> is POSIX-1.2008.

Although this implementation of <<strerror>> is reentrant (depending
on <<_user_strerror>>), ANSI C declares that subsequent calls to
<<strerror>> may overwrite the result string; therefore portable
code cannot depend on the reentrancy of this subroutine.

Although this implementation of <<strerror>> guarantees a non-null
result with a NUL-terminator, some implementations return <<NULL>>
on failure.  Although POSIX allows <<strerror>> to set <<errno>>
to EINVAL on failure, this implementation does not do so (unless
you provide <<_user_strerror>>).

POSIX recommends that unknown <[errnum]> result in a message
including that value, however it is not a requirement and this
implementation does not provide that information (unless you
provide <<_user_strerror>>).

This implementation of <<strerror>> provides for user-defined
extensibility.  <<errno.h>> defines <[__ELASTERROR]>, which can be
used as a base for user-defined error values.  If the user supplies a
routine named <<_user_strerror>>, and <[errnum]> passed to
<<strerror>> does not match any of the supported values,
<<_user_strerror>> is called with three arguments.  The first is of
type <[int]>, and is the <[errnum]> value unknown to <<strerror>>.
The second is of type <[int]>, and matches the <[internal]> argument
of <<_strerror_r>>; this should be zero if called from <<strerror>>
and non-zero if called from any other function; <<_user_strerror>> can
use this information to satisfy the POSIX rule that no other
standardized function can overwrite a static buffer reused by
<<strerror>>.  The third is of type <[int *]>, and matches the
<[error]> argument of <<_strerror_r>>; if a non-zero value is stored
into that location (usually <[EINVAL]>), then <<strerror>> will set
<<errno>> to that value, and the XPG variant of <<strerror_r>> will
return that value instead of zero or <[ERANGE]>.  <<_user_strerror>>
returns a <[char *]> value; returning <[NULL]> implies that the user
function did not choose to handle <[errnum]>.  The default
<<_user_strerror>> returns <[NULL]> for all input values.  Note that
<<_user_sterror>> must be thread-safe, and only denote errors via the
third argument rather than modifying <<errno>>, if <<strerror>> and
<<strerror_r>> are are to comply with POSIX.

<<strerror>> requires no supporting OS subroutines.

QUICKREF
	strerror ansi pure
*/

#include <errno.h>
#include <string.h>

// char *
// _strerror_r (struct _reent *ptr,
// 	int errnum,
// 	int internal,
// 	int *errptr)
// {
char* strerror (int errnum) {
  char *error;
  extern char *_user_strerror (int, int, int *);

  switch (errnum)
    {
    case 0:
      error = "Success";
      break;
/* go32 defines EPERM as EACCES */
#if defined (EPERM) && (!defined (EACCES) || (EPERM != EACCES))
    case EPERM:
      error = "Not owner";
      break;
#endif
#ifdef ENOENT
    case ENOENT:
      error = "No such file or directory";
      break;
#endif
#ifdef ESRCH
    case ESRCH:
      error = "No such process";
      break;
#endif
#ifdef EINTR
    case EINTR:
      error = "Interrupted system call";
      break;
#endif
#ifdef EIO
    case EIO:
      error = "I/O error";
      break;
#endif
/* go32 defines ENXIO as ENODEV */
#if defined (ENXIO) && (!defined (ENODEV) || (ENXIO != ENODEV))
    case ENXIO:
      error = "No such device or address";
      break;
#endif
#ifdef E2BIG
    case E2BIG:
      error = "Arg list too long";
      break;
#endif
#ifdef ENOEXEC
    case ENOEXEC:
      error = "Exec format error";
      break;
#endif
#ifdef EALREADY
    case EALREADY:
      error = "Socket already connected";
      break;
#endif
#ifdef EBADF
    case EBADF:
      error = "Bad file number";
      break;
#endif
#ifdef ECHILD
    case ECHILD:
      error = "No children";
      break;
#endif
#ifdef EDESTADDRREQ
    case EDESTADDRREQ:
      error = "Destination address required";
      break;
#endif
#ifdef EAGAIN
    case EAGAIN:
      error = "No more processes";
      break;
#endif
#ifdef ENOMEM
    case ENOMEM:
      error = "Not enough space";
      break;
#endif
#ifdef EACCES
    case EACCES:
      error = "Permission denied";
      break;
#endif
#ifdef EFAULT
    case EFAULT:
      error = "Bad address";
      break;
#endif
#ifdef ENOTBLK
    case ENOTBLK:
      error = "Block device required";
      break;
#endif
#ifdef EBUSY
    case EBUSY:
      error = "Device or resource busy";
      break;
#endif
#ifdef EEXIST
    case EEXIST:
      error = "File exists";
      break;
#endif
#ifdef EXDEV
    case EXDEV:
      error = "Cross-device link";
      break;
#endif
#ifdef ENODEV
    case ENODEV:
      error = "No such device";
      break;
#endif
#ifdef ENOTDIR
    case ENOTDIR:
      error = "Not a directory";
      break;
#endif
#ifdef EHOSTDOWN
    case EHOSTDOWN:
      error = "Host is down";
      break;
#endif
#ifdef EINPROGRESS
    case EINPROGRESS:
      error = "Connection already in progress";
      break;
#endif
#ifdef EISDIR
    case EISDIR:
      error = "Is a directory";
      break;
#endif
#ifdef EINVAL
    case EINVAL:
      error = "Invalid argument";
      break;
#endif
#ifdef ENETDOWN
    case ENETDOWN:
      error = "Network interface is not configured";
      break;
#endif
#ifdef ENETRESET
    case ENETRESET:
      error = "Connection aborted by network";
      break;
#endif
#ifdef ENFILE
    case ENFILE:
      error = "Too many open files in system";
      break;
#endif
#ifdef EMFILE
    case EMFILE:
      error = "File descriptor value too large";
      break;
#endif
#ifdef ENOTTY
    case ENOTTY:
      error = "Not a character device";
      break;
#endif
#ifdef ETXTBSY
    case ETXTBSY:
      error = "Text file busy";
      break;
#endif
#ifdef EFBIG
    case EFBIG:
      error = "File too large";
      break;
#endif
#ifdef EHOSTUNREACH
    case EHOSTUNREACH:
      error = "Host is unreachable";
      break;
#endif
#ifdef ENOSPC
    case ENOSPC:
      error = "No space left on device";
      break;
#endif
#ifdef ENOTSUP
    case ENOTSUP:
      error = "Not supported";
      break;
#endif
#ifdef ESPIPE
    case ESPIPE:
      error = "Illegal seek";
      break;
#endif
#ifdef EROFS
    case EROFS:
      error = "Read-only file system";
      break;
#endif
#ifdef EMLINK
    case EMLINK:
      error = "Too many links";
      break;
#endif
#ifdef EPIPE
    case EPIPE:
      error = "Broken pipe";
      break;
#endif
#ifdef EDOM
    case EDOM:
      error = "Mathematics argument out of domain of function";
      break;
#endif
#ifdef ERANGE
    case ERANGE:
      error = "Result too large";
      break;
#endif
#ifdef ENOMSG
    case ENOMSG:
      error = "No message of desired type";
      break;
#endif
#ifdef EIDRM
    case EIDRM:
      error = "Identifier removed";
      break;
#endif
#ifdef EILSEQ
    case EILSEQ:
      error = "Illegal byte sequence";
      break;
#endif
#ifdef EDEADLK
    case EDEADLK:
      error = "Deadlock";
      break;
#endif
#ifdef ENETUNREACH
    case  ENETUNREACH:
      error = "Network is unreachable";
      break;
#endif
#ifdef ENOLCK
    case ENOLCK:
      error = "No lock";
      break;
#endif
#ifdef ENOSTR
    case ENOSTR:
      error = "Not a stream";
      break;
#endif
#ifdef ETIME
    case ETIME:
      error = "Stream ioctl timeout";
      break;
#endif
#ifdef ENOSR
    case ENOSR:
      error = "No stream resources";
      break;
#endif
#ifdef ENONET
    case ENONET:
      error = "Machine is not on the network";
      break;
#endif
#ifdef ENOPKG
    case ENOPKG:
      error = "No package";
      break;
#endif
#ifdef EREMOTE
    case EREMOTE:
      error = "Resource is remote";
      break;
#endif
#ifdef ENOLINK
    case ENOLINK:
      error = "Virtual circuit is gone";
      break;
#endif
#ifdef EADV
    case EADV:
      error = "Advertise error";
      break;
#endif
#ifdef ESRMNT
    case ESRMNT:
      error = "Srmount error";
      break;
#endif
#ifdef ECOMM
    case ECOMM:
      error = "Communication error";
      break;
#endif
#ifdef EPROTO
    case EPROTO:
      error = "Protocol error";
      break;
#endif
#ifdef EPROTONOSUPPORT
    case EPROTONOSUPPORT:
      error = "Unknown protocol";
      break;
#endif
#ifdef EMULTIHOP
    case EMULTIHOP:
      error = "Multihop attempted";
      break;
#endif
#ifdef EBADMSG
    case EBADMSG:
      error = "Bad message";
      break;
#endif
#ifdef ELIBACC
    case ELIBACC:
      error = "Cannot access a needed shared library";
      break;
#endif
#ifdef ELIBBAD
    case ELIBBAD:
      error = "Accessing a corrupted shared library";
      break;
#endif
#ifdef ELIBSCN
    case ELIBSCN:
      error = ".lib section in a.out corrupted";
      break;
#endif
#ifdef ELIBMAX
    case ELIBMAX:
      error = "Attempting to link in more shared libraries than system limit";
      break;
#endif
#ifdef ELIBEXEC
    case ELIBEXEC:
      error = "Cannot exec a shared library directly";
      break;
#endif
#ifdef ENOSYS
    case ENOSYS:
      error = "Function not implemented";
      break;
#endif
#ifdef ENMFILE
    case ENMFILE:
      error = "No more files";
      break;
#endif
#ifdef ENOTEMPTY
    case ENOTEMPTY:
      error = "Directory not empty";
      break;
#endif
#ifdef ENAMETOOLONG
    case ENAMETOOLONG:
      error = "File or path name too long";
      break;
#endif
#ifdef ELOOP
    case ELOOP:
      error = "Too many symbolic links";
      break;
#endif
#ifdef ENOBUFS
    case ENOBUFS:
      error = "No buffer space available";
      break;
#endif
#ifdef ENODATA
    case ENODATA:
      error = "No data";
      break;
#endif
#ifdef EAFNOSUPPORT
    case EAFNOSUPPORT:
      error = "Address family not supported by protocol family";
      break;
#endif
#ifdef EPROTOTYPE
    case EPROTOTYPE:
      error = "Protocol wrong type for socket";
      break;
#endif
#ifdef ENOTSOCK
    case ENOTSOCK:
      error = "Socket operation on non-socket";
      break;
#endif
#ifdef ENOPROTOOPT
    case ENOPROTOOPT:
      error = "Protocol not available";
      break;
#endif
#ifdef ESHUTDOWN
    case ESHUTDOWN:
      error = "Can't send after socket shutdown";
      break;
#endif
#ifdef ECONNREFUSED
    case ECONNREFUSED:
      error = "Connection refused";
      break;
#endif
#ifdef ECONNRESET
    case ECONNRESET:
      error = "Connection reset by peer";
      break;
#endif
#ifdef EADDRINUSE
    case EADDRINUSE:
      error = "Address already in use";
      break;
#endif
#ifdef EADDRNOTAVAIL
    case EADDRNOTAVAIL:
      error = "Address not available";
      break;
#endif
#ifdef ECONNABORTED
    case ECONNABORTED:
      error = "Software caused connection abort";
      break;
#endif
#if (defined(EWOULDBLOCK) && (!defined (EAGAIN) || (EWOULDBLOCK != EAGAIN)))
    case EWOULDBLOCK:
        error = "Operation would block";
        break;
#endif
#ifdef ENOTCONN
    case ENOTCONN:
        error = "Socket is not connected";
        break;
#endif
#ifdef ESOCKTNOSUPPORT
    case ESOCKTNOSUPPORT:
        error = "Socket type not supported";
        break;
#endif
#ifdef EISCONN
    case EISCONN:
        error = "Socket is already connected";
        break;
#endif
#ifdef ECANCELED
    case ECANCELED:
        error = "Operation canceled";
        break;
#endif
#ifdef ENOTRECOVERABLE
    case ENOTRECOVERABLE:
        error = "State not recoverable";
        break;
#endif
#ifdef EOWNERDEAD
    case EOWNERDEAD:
        error = "Previous owner died";
        break;
#endif
#ifdef ESTRPIPE
    case ESTRPIPE:
	error = "Streams pipe error";
	break;
#endif
#if defined(EOPNOTSUPP) && (!defined(ENOTSUP) || (ENOTSUP != EOPNOTSUPP))
    case EOPNOTSUPP:
        error = "Operation not supported on socket";
        break;
#endif
#ifdef EOVERFLOW
    case EOVERFLOW:
      error = "Value too large for defined data type";
      break;
#endif
#ifdef EMSGSIZE
    case EMSGSIZE:
        error = "Message too long";
        break;
#endif
#ifdef ETIMEDOUT
    case ETIMEDOUT:
        error = "Connection timed out";
        break;
#endif
    default:
    //   if (!errptr)
    //     errptr = &_REENT_ERRNO(ptr);
    //   if ((error = _user_strerror (errnum, internal, errptr)) == 0)
    //     error = "";
        error = "unknown error";
      break;
    }

  return error;
}

// char *
// strerror (int errnum)
// {
//   return _strerror_r (_REENT, errnum, 0, NULL);
// }

// char *
// strerror_l (int errnum, locale_t locale)
// {
//   /* We don't support per-locale error messages. */
//   return _strerror_r (_REENT, errnum, 0, NULL);
// }

/*
FUNCTION
	<<strlen>>---character string length

INDEX
	strlen

SYNOPSIS
	#include <string.h>
	size_t strlen(const char *<[str]>);

DESCRIPTION
	The <<strlen>> function works out the length of the string
	starting at <<*<[str]>>> by counting chararacters until it
	reaches a <<NULL>> character.

RETURNS
	<<strlen>> returns the character count.

PORTABILITY
<<strlen>> is ANSI C.

<<strlen>> requires no supporting OS subroutines.

QUICKREF
	strlen ansi pure
*/

#include <_ansi.h>
#include <string.h>
#include <limits.h>

#define LBLOCKSIZE   (sizeof (long))
#define UNALIGNED(X) ((long)X & (LBLOCKSIZE - 1))

#if LONG_MAX == 2147483647L
#define DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080)
#else
#if LONG_MAX == 9223372036854775807L
/* Nonzero if X (a long int) contains a NULL byte. */
#define DETECTNULL(X) (((X) - 0x0101010101010101) & ~(X) & 0x8080808080808080)
#else
#error long int is not a 32bit or 64bit type.
#endif
#endif

#ifndef DETECTNULL
#error long int is not a 32bit or 64bit byte
#endif

size_t
strlen (const char *str)
{
  const char *start = str;

#if !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)
  unsigned long *aligned_addr;

  /* Align the pointer, so we can search a word at a time.  */
  while (UNALIGNED (str))
    {
      if (!*str)
	return str - start;
      str++;
    }

  /* If the string is word-aligned, we can check for the presence of
     a null in each word-sized block.  */
  aligned_addr = (unsigned long *)str;
  while (!DETECTNULL (*aligned_addr))
    aligned_addr++;

  /* Once a null is detected, we check each byte in that block for a
     precise position of the null.  */
  str = (char *) aligned_addr;

#endif /* not PREFER_SIZE_OVER_SPEED */

  while (*str)
    str++;
  return str - start;
}

/*
FUNCTION
	<<strncat>>---concatenate strings

INDEX
	strncat

SYNOPSIS
	#include <string.h>
	char *strncat(char *restrict <[dst]>, const char *restrict <[src]>,
                      size_t <[length]>);

DESCRIPTION
	<<strncat>> appends not more than <[length]> characters from
	the string pointed to by <[src]> (including the	terminating
	null character) to the end of the string pointed to by
	<[dst]>.  The initial character of <[src]> overwrites the null
	character at the end of <[dst]>.  A terminating null character
	is always appended to the result

WARNINGS
	Note that a null is always appended, so that if the copy is
	limited by the <[length]> argument, the number of characters
	appended to <[dst]> is <<n + 1>>.

RETURNS
	This function returns the initial value of <[dst]>

PORTABILITY
<<strncat>> is ANSI C.

<<strncat>> requires no supporting OS subroutines.

QUICKREF
	strncat ansi pure
*/

#include <string.h>
#include <limits.h>

/* Nonzero if X is aligned on a "long" boundary.  */
#define ALIGNED(X) \
  (((long)X & (sizeof (long) - 1)) == 0)

#if LONG_MAX == 2147483647L
#define DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080)
#else
#if LONG_MAX == 9223372036854775807L
/* Nonzero if X (a long int) contains a NULL byte. */
#define DETECTNULL(X) (((X) - 0x0101010101010101) & ~(X) & 0x8080808080808080)
#else
#error long int is not a 32bit or 64bit type.
#endif
#endif

#ifndef DETECTNULL
#error long int is not a 32bit or 64bit byte
#endif

char *
strncat (char *__restrict s1,
	const char *__restrict s2,
	size_t n)
{
#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
  char *s = s1;

  while (*s1)
    s1++;
  while (n-- != 0 && (*s1++ = *s2++))
    {
      if (n == 0)
	*s1 = '\0';
    }

  return s;
#else
  char *s = s1;

  /* Skip over the data in s1 as quickly as possible.  */
  if (ALIGNED (s1))
    {
      unsigned long *aligned_s1 = (unsigned long *)s1;
      while (!DETECTNULL (*aligned_s1))
	aligned_s1++;

      s1 = (char *)aligned_s1;
    }

  while (*s1)
    s1++;

  /* s1 now points to the its trailing null character, now copy
     up to N bytes from S2 into S1 stopping if a NULL is encountered
     in S2.

     It is not safe to use strncpy here since it copies EXACTLY N
     characters, NULL padding if necessary.  */
  while (n-- != 0 && (*s1++ = *s2++))
    {
      if (n == 0)
	*s1 = '\0';
    }
	
  return s;
#endif /* not PREFER_SIZE_OVER_SPEED */
}

/*
FUNCTION
	<<strncmp>>---character string compare
	
INDEX
	strncmp

SYNOPSIS
	#include <string.h>
	int strncmp(const char *<[a]>, const char * <[b]>, size_t <[length]>);

DESCRIPTION
	<<strncmp>> compares up to <[length]> characters
	from the string at <[a]> to the string at <[b]>.

RETURNS
	If <<*<[a]>>> sorts lexicographically after <<*<[b]>>>,
	<<strncmp>> returns a number greater than zero.  If the two
	strings are equivalent, <<strncmp>> returns zero.  If <<*<[a]>>>
	sorts lexicographically before <<*<[b]>>>, <<strncmp>> returns a
	number less than zero.

PORTABILITY
<<strncmp>> is ANSI C.

<<strncmp>> requires no supporting OS subroutines.

QUICKREF
	strncmp ansi pure
*/

#include <string.h>
#include <limits.h>

/* Nonzero if either X or Y is not aligned on a "long" boundary.  */
#define UNALIGNED(X, Y) \
  (((long)X & (sizeof (long) - 1)) | ((long)Y & (sizeof (long) - 1)))

/* DETECTNULL returns nonzero if (long)X contains a NULL byte. */
#if LONG_MAX == 2147483647L
#define DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080)
#else
#if LONG_MAX == 9223372036854775807L
#define DETECTNULL(X) (((X) - 0x0101010101010101) & ~(X) & 0x8080808080808080)
#else
#error long int is not a 32bit or 64bit type.
#endif
#endif

#ifndef DETECTNULL
#error long int is not a 32bit or 64bit byte
#endif

int 
strncmp (const char *s1,
	const char *s2,
	size_t n)
{
#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
  if (n == 0)
    return 0;

  while (n-- != 0 && *s1 == *s2)
    {
      if (n == 0 || *s1 == '\0')
	break;
      s1++;
      s2++;
    }

  return (*(unsigned char *) s1) - (*(unsigned char *) s2);
#else
  unsigned long *a1;
  unsigned long *a2;

  if (n == 0)
    return 0;

  /* If s1 or s2 are unaligned, then compare bytes. */
  if (!UNALIGNED (s1, s2))
    {
      /* If s1 and s2 are word-aligned, compare them a word at a time. */
      a1 = (unsigned long*)s1;
      a2 = (unsigned long*)s2;
      while (n >= sizeof (long) && *a1 == *a2)
        {
          n -= sizeof (long);

          /* If we've run out of bytes or hit a null, return zero
	     since we already know *a1 == *a2.  */
          if (n == 0 || DETECTNULL (*a1))
	    return 0;

          a1++;
          a2++;
        }

      /* A difference was detected in last few bytes of s1, so search bytewise */
      s1 = (char*)a1;
      s2 = (char*)a2;
    }

  while (n-- > 0 && *s1 == *s2)
    {
      /* If we've run out of bytes or hit a null, return zero
	 since we already know *s1 == *s2.  */
      if (n == 0 || *s1 == '\0')
	return 0;
      s1++;
      s2++;
    }
  return (*(unsigned char *) s1) - (*(unsigned char *) s2);
#endif /* not PREFER_SIZE_OVER_SPEED */
}

/*
FUNCTION
	<<strncpy>>---counted copy string

INDEX
	strncpy

SYNOPSIS
	#include <string.h>
	char *strncpy(char *restrict <[dst]>, const char *restrict <[src]>,
                      size_t <[length]>);

DESCRIPTION
	<<strncpy>> copies not more than <[length]> characters from the
	the string pointed to by <[src]> (including the terminating
	null character) to the array pointed to by <[dst]>.  If the
	string pointed to by <[src]> is shorter than <[length]>
	characters, null characters are appended to the destination
	array until a total of <[length]> characters have been
	written.

RETURNS
	This function returns the initial value of <[dst]>.

PORTABILITY
<<strncpy>> is ANSI C.

<<strncpy>> requires no supporting OS subroutines.

QUICKREF
	strncpy ansi pure
*/

#include <string.h>
#include <limits.h>

/*SUPPRESS 560*/
/*SUPPRESS 530*/

/* Nonzero if either X or Y is not aligned on a "long" boundary.  */
#define UNALIGNED(X, Y) \
  (((long)X & (sizeof (long) - 1)) | ((long)Y & (sizeof (long) - 1)))

#if LONG_MAX == 2147483647L
#define DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080)
#else
#if LONG_MAX == 9223372036854775807L
/* Nonzero if X (a long int) contains a NULL byte. */
#define DETECTNULL(X) (((X) - 0x0101010101010101) & ~(X) & 0x8080808080808080)
#else
#error long int is not a 32bit or 64bit type.
#endif
#endif

#ifndef DETECTNULL
#error long int is not a 32bit or 64bit byte
#endif

#define TOO_SMALL(LEN) ((LEN) < sizeof (long))

char *
strncpy (char *__restrict dst0,
	const char *__restrict src0,
	size_t count)
{
#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
  char *dscan;
  const char *sscan;

  dscan = dst0;
  sscan = src0;
  while (count > 0)
    {
      --count;
      if ((*dscan++ = *sscan++) == '\0')
	break;
    }
  while (count-- > 0)
    *dscan++ = '\0';

  return dst0;
#else
  char *dst = dst0;
  const char *src = src0;
  long *aligned_dst;
  const long *aligned_src;

  /* If SRC and DEST is aligned and count large enough, then copy words.  */
  if (!UNALIGNED (src, dst) && !TOO_SMALL (count))
    {
      aligned_dst = (long*)dst;
      aligned_src = (long*)src;

      /* SRC and DEST are both "long int" aligned, try to do "long int"
	 sized copies.  */
      while (count >= sizeof (long int) && !DETECTNULL(*aligned_src))
	{
	  count -= sizeof (long int);
	  *aligned_dst++ = *aligned_src++;
	}

      dst = (char*)aligned_dst;
      src = (char*)aligned_src;
    }

  while (count > 0)
    {
      --count;
      if ((*dst++ = *src++) == '\0')
	break;
    }

  while (count-- > 0)
    *dst++ = '\0';

  return dst0;
#endif /* not PREFER_SIZE_OVER_SPEED */
}


/* 
FUNCTION
	<<strnlen>>---character string length
	
INDEX
	strnlen

SYNOPSIS
	#include <string.h>
	size_t strnlen(const char *<[str]>, size_t <[n]>);

DESCRIPTION
	The <<strnlen>> function works out the length of the string
	starting at <<*<[str]>>> by counting chararacters until it
	reaches a NUL character or the maximum: <[n]> number of
        characters have been inspected.

RETURNS
	<<strnlen>> returns the character count or <[n]>.

PORTABILITY
<<strnlen>> is a GNU extension.

<<strnlen>> requires no supporting OS subroutines.

*/

#undef __STRICT_ANSI__
#include <_ansi.h>
#include <string.h>

size_t
strnlen (const char *str,
	size_t n)
{
  const char *start = str;

  while (n-- > 0 && *str)
    str++;

  return str - start;
}

/*
FUNCTION
	<<strpbrk>>---find characters in string

INDEX
	strpbrk

SYNOPSIS
	#include <string.h>
	char *strpbrk(const char *<[s1]>, const char *<[s2]>);

DESCRIPTION
	This function locates the first occurence in the string
	pointed to by <[s1]> of any character in string pointed to by
	<[s2]> (excluding the terminating null character).

RETURNS
	<<strpbrk>> returns a pointer to the character found in <[s1]>, or a
	null pointer if no character from <[s2]> occurs in <[s1]>.

PORTABILITY
<<strpbrk>> requires no supporting OS subroutines.
*/

#include <string.h>

char *
strpbrk (const char *s1,
	const char *s2)
{
  const char *c = s2;

  while (*s1)
    {
      for (c = s2; *c; c++)
	{
	  if (*s1 == *c)
	    return (char *) s1;
	}
      s1++;
    }

  return (char *) NULL;
}

/*
FUNCTION
	<<strrchr>>---reverse search for character in string

INDEX
	strrchr

SYNOPSIS
	#include <string.h>
	char * strrchr(const char *<[string]>, int <[c]>);

DESCRIPTION
	This function finds the last occurence of <[c]> (converted to
	a char) in the string pointed to by <[string]> (including the
	terminating null character).

RETURNS
	Returns a pointer to the located character, or a null pointer
	if <[c]> does not occur in <[string]>.

PORTABILITY
<<strrchr>> is ANSI C.

<<strrchr>> requires no supporting OS subroutines.

QUICKREF
	strrchr ansi pure
*/

#include <string.h>

char *
strrchr (const char *s,
	int i)
{
  const char *last = NULL;
  char c = i;

  if (c)
    {
      while ((s=strchr(s, c)))
	{
	  last = s;
	  s++;
	}
    }
  else
    {
      last = strchr(s, c);
    }

  return (char *) last;
}

/*
FUNCTION
	<<strspn>>---find initial match

INDEX
	strspn

SYNOPSIS
	#include <string.h>
	size_t strspn(const char *<[s1]>, const char *<[s2]>);

DESCRIPTION
	This function computes the length of the initial segment of
	the string pointed to by <[s1]> which consists entirely of
	characters from the string pointed to by <[s2]> (excluding the
	terminating null character).

RETURNS
	<<strspn>> returns the length of the segment found.

PORTABILITY
<<strspn>> is ANSI C.

<<strspn>> requires no supporting OS subroutines.

QUICKREF
	strspn ansi pure
*/

#include <string.h>

size_t
strspn (const char *s1,
	const char *s2)
{
  const char *s = s1;
  const char *c;

  while (*s1)
    {
      for (c = s2; *c; c++)
	{
	  if (*s1 == *c)
	    goto found;
	}
      if (*c == '\0')
	break;
found:
      s1++;
    }

  return s1 - s;
}

/* Optimized strstr function.
   Copyright (c) 2018 Arm Ltd.  All rights reserved.

   SPDX-License-Identifier: BSD-3-Clause

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. The name of the company may not be used to endorse or promote
      products derived from this software without specific prior written
      permission.

   THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

/*
FUNCTION
	<<strstr>>---find string segment

INDEX
	strstr

SYNOPSIS
	#include <string.h>
	char *strstr(const char *<[s1]>, const char *<[s2]>);

DESCRIPTION
	Locates the first occurrence in the string pointed to by <[s1]> of
	the sequence of characters in the string pointed to by <[s2]>
	(excluding the terminating null character).

RETURNS
	Returns a pointer to the located string segment, or a null
	pointer if the string <[s2]> is not found. If <[s2]> points to
	a string with zero length, <[s1]> is returned.

PORTABILITY
<<strstr>> is ANSI C.

<<strstr>> requires no supporting OS subroutines.

QUICKREF
	strstr ansi pure
*/

#include <string.h>
#include <limits.h>

#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__) \
    || CHAR_BIT > 8

/* Small and efficient strstr implementation.  */
char *
strstr (const char *hs, const char *ne)
{
  size_t i;
  int c = ne[0];

  if (c == 0)
    return (char*)hs;

  for ( ; hs[0] != '\0'; hs++)
    {
      if (hs[0] != c)
	continue;
      for (i = 1; ne[i] != 0; i++)
	if (hs[i] != ne[i])
	  break;
      if (ne[i] == '\0')
	return (char*)hs;
    }

  return NULL;
}

#else /* compilation for speed */

# define RETURN_TYPE char *
# define AVAILABLE(h, h_l, j, n_l) (((j) <= (h_l) - (n_l)) \
   || ((h_l) += strnlen ((const char *) (h) + (h_l), (n_l) | 2048), ((j) <= (h_l) - (n_l))))

# include "str-two-way.h"

/* Number of bits used to index shift table.  */
#define SHIFT_TABLE_BITS 6

static inline char *
strstr2 (const unsigned char *hs, const unsigned char *ne)
{
  uint32_t h1 = (ne[0] << 16) | ne[1];
  uint32_t h2 = 0;
  int c;
  for (c = hs[0]; h1 != h2 && c != 0; c = *++hs)
      h2 = (h2 << 16) | c;
  return h1 == h2 ? (char *)hs - 2 : NULL;
}

static inline char *
strstr3 (const unsigned char *hs, const unsigned char *ne)
{
  uint32_t h1 = (ne[0] << 24) | (ne[1] << 16) | (ne[2] << 8);
  uint32_t h2 = 0;
  int c;
  for (c = hs[0]; h1 != h2 && c != 0; c = *++hs)
      h2 = (h2 | c) << 8;
  return h1 == h2 ? (char *)hs - 3 : NULL;
}

static inline char *
strstr4 (const unsigned char *hs, const unsigned char *ne)
{
  uint32_t h1 = (ne[0] << 24) | (ne[1] << 16) | (ne[2] << 8) | ne[3];
  uint32_t h2 = 0;
  int c;
  for (c = hs[0]; c != 0 && h1 != h2; c = *++hs)
    h2 = (h2 << 8) | c;
  return h1 == h2 ? (char *)hs - 4 : NULL;
}

/* Extremely fast strstr algorithm with guaranteed linear-time performance.
   Small needles up to size 4 use a dedicated linear search.  Longer needles
   up to size 254 use Sunday's Quick-Search algorithm.  Due to its simplicity
   it has the best average performance of string matching algorithms on almost
   all inputs.  It uses a bad-character shift table to skip past mismatches.
   By limiting the needle length to 254, the shift table can be reduced to 8
   bits per entry, lowering preprocessing overhead and minimizing cache effects.
   The limit also implies the worst-case performance is linear.
   Even larger needles are processed by the linear-time Two-Way algorithm.
*/
char *
strstr (const char *haystack, const char *needle)
{
  const unsigned char *hs = (const unsigned char *) haystack;
  const unsigned char *ne = (const unsigned char *) needle;
  int i;

  /* Handle short needle special cases first.  */
  if (ne[0] == '\0')
    return (char *) hs;
  if (ne[1] == '\0')
    return (char*)strchr ((const char *) hs, ne[0]);
  if (ne[2] == '\0')
    return strstr2 (hs, ne);
  if (ne[3] == '\0')
    return strstr3 (hs, ne);
  if (ne[4] == '\0')
    return strstr4 (hs, ne);

  size_t ne_len = strlen ((const char *) ne);
  size_t hs_len = strnlen ((const char *) hs, ne_len | 512);

  /* Ensure haystack length is >= needle length.  */
  if (hs_len < ne_len)
    return NULL;

  /* Use the Quick-Search algorithm for needle lengths less than 255.  */
  if (__builtin_expect (ne_len < 255, 1))
    {
      uint8_t shift[1 << SHIFT_TABLE_BITS];
      const unsigned char *end = hs + hs_len - ne_len;

      /* Initialize bad character shift hash table.  */
      memset (shift, ne_len + 1, sizeof (shift));
      for (i = 0; i < ne_len; i++)
	shift[ne[i] % sizeof (shift)] = ne_len - i;

      do
	{
	  hs--;

	  /* Search by skipping past bad characters.  */
	  size_t tmp = shift[hs[ne_len] % sizeof (shift)];
	  for (hs += tmp; hs <= end; hs += tmp)
	    {
	      tmp = shift[hs[ne_len] % sizeof (shift)];
	      if (memcmp (hs, ne, ne_len) == 0)
		return (char*) hs;
	    }
	  if (end[ne_len] == 0)
	    return NULL;
	  end += strnlen ((const char *) (end + ne_len), 2048);
	}
      while (hs <= end);

      return NULL;
    }

  /* Use Two-Way algorithm for very long needles.  */
  return two_way_long_needle (hs, hs_len, ne, ne_len);
}
#endif /* compilation for speed */

/*
FUNCTION
	<<strtok>>, <<strtok_r>>, <<strsep>>---get next token from a string

INDEX
	strtok

INDEX
	strtok_r

INDEX
	strsep

SYNOPSIS
	#include <string.h>
      	char *strtok(char *restrict <[source]>,
                     const char *restrict <[delimiters]>);
      	char *strtok_r(char *restrict <[source]>,
                       const char *restrict <[delimiters]>,
                       char **<[lasts]>);
	char *strsep(char **<[source_ptr]>, const char *<[delimiters]>);

DESCRIPTION
	The <<strtok>> function is used to isolate sequential tokens in a 
	null-terminated string, <<*<[source]>>>. These tokens are delimited 
	in the string by at least one of the characters in <<*<[delimiters]>>>.
	The first time that <<strtok>> is called, <<*<[source]>>> should be
	specified; subsequent calls, wishing to obtain further tokens from
	the same string, should pass a null pointer instead.  The separator
	string, <<*<[delimiters]>>>, must be supplied each time and may 
	change between calls.

	The <<strtok>> function returns a pointer to the beginning of each 
	subsequent token in the string, after replacing the separator 
	character itself with a null character.  When no more tokens remain, 
	a null pointer is returned.

	The <<strtok_r>> function has the same behavior as <<strtok>>, except
	a pointer to placeholder <<*<[lasts]>>> must be supplied by the caller.

	The <<strsep>> function is similar in behavior to <<strtok>>, except
	a pointer to the string pointer must be supplied <<<[source_ptr]>>> and
	the function does not skip leading delimiters.  When the string starts
	with a delimiter, the delimiter is changed to the null character and
	the empty string is returned.  Like <<strtok_r>> and <<strtok>>, the
	<<*<[source_ptr]>>> is updated to the next character following the
	last delimiter found or NULL if the end of string is reached with
	no more delimiters.

RETURNS
	<<strtok>>, <<strtok_r>>, and <<strsep>> all return a pointer to the 
	next token, or <<NULL>> if no more tokens can be found.  For
	<<strsep>>, a token may be the empty string.

NOTES
	<<strtok>> is unsafe for multi-threaded applications.  <<strtok_r>>
	and <<strsep>> are thread-safe and should be used instead.

PORTABILITY
<<strtok>> is ANSI C.
<<strtok_r>> is POSIX.
<<strsep>> is a BSD extension.

<<strtok>>, <<strtok_r>>, and <<strsep>> require no supporting OS subroutines.

QUICKREF
	strtok ansi impure
*/

/* undef STRICT_ANSI so that strtok_r prototype will be defined */
// #undef  __STRICT_ANSI__
// #include <string.h>
// // #include <stdlib.h>
// #include <_ansi.h>
// // #include <reent.h>

// #ifdef _REENT_THREAD_LOCAL
// _Thread_local char *_tls_strtok_last;
// #endif

// #ifndef _REENT_ONLY

// extern char *__strtok_r (char *, const char *, char **, int);

// char *
// strtok (register char *__restrict s,
// 	register const char *__restrict delim)
// {
// 	struct _reent *reent = _REENT;

// 	_REENT_CHECK_MISC(reent);
// 	return __strtok_r (s, delim, &(_REENT_STRTOK_LAST(reent)), 1);
// }
// #endif


//taken from libc
char* strtok(char *s, const char *delim)
{
	static char *last;
	return strtok_r(s, delim, &last);
}
char* strtok_r(char *s, const char *delim, char **last)
{
	char *spanp;
	int c, sc;
	char *tok;
	if (s == NULL && (s = *last) == NULL)
		return (NULL);
	/*
	 * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
	 */
cont:
	c = *s++;
	for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
		if (c == sc)
			goto cont;
	}
	if (c == 0) {		/* no non-delimiter characters */
		*last = NULL;
		return (NULL);
	}
	tok = s - 1;
	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;) {
		c = *s++;
		spanp = (char *)delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*last = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}

/*
FUNCTION
	<<strxfrm>>---transform string

INDEX
	strxfrm

SYNOPSIS
	#include <string.h>
	size_t strxfrm(char *restrict <[s1]>, const char *restrict <[s2]>,
                       size_t <[n]>);

DESCRIPTION
	This function transforms the string pointed to by <[s2]> and
	places the resulting string into the array pointed to by
	<[s1]>. The transformation is such that if the <<strcmp>>
	function is applied to the two transformed strings, it returns
	a value greater than, equal to, or less than zero,
	correspoinding to the result of a <<strcoll>> function applied
	to the same two original strings.

	No more than <[n]> characters are placed into the resulting
	array pointed to by <[s1]>, including the terminating null
	character. If <[n]> is zero, <[s1]> may be a null pointer. If
	copying takes place between objects that overlap, the behavior
	is undefined.

	(NOT Cygwin:) The current implementation of <<strxfrm>> simply copies
	the input and does not support any language-specific transformations.

RETURNS
	The <<strxfrm>> function returns the length of the transformed string
	(not including the terminating null character). If the value returned
	is <[n]> or more, the contents of the array pointed to by
	<[s1]> are indeterminate.

PORTABILITY
<<strxfrm>> is ANSI C.

<<strxfrm>> requires no supporting OS subroutines.

QUICKREF
	strxfrm ansi pure
*/

#include <string.h>

size_t
strxfrm (char *__restrict s1,
	const char *__restrict s2,
	size_t n)
{
  size_t res;
  res = 0;
  while (n-- > 0)
    {
      if ((*s1++ = *s2++) != '\0')
        ++res;
      else
        return res;
    }
  while (*s2)
    {
      ++s2;
      ++res;
    }

  return res;
}