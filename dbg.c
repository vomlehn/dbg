/*
 * Debugging message support
 */

#include <sys/types.h>

#include <stdarg.h>
#include <time.h>
#include <unistd.h>

#if 1
#define	EOL	"\r\n"
#else
#define EOL	"\n"
#endif

#ifdef __linux__
#define HAS_GETTID
#endif	/* __linux */

/* From https://dustri.org/b/min-and-max-macro-considered-harmful.html */
#define MIN(a,b) \
	({					\
	 	__typeof__ (a) _a = (a);	\
		__typeof__ (b) _b = (b);	\
		_a < _b ? _a : _b;		\
	 })

/* Number of items per line when printing buffers in hex */
#define N_ROW_ITEMS	16u

#define DBG	1
#include <include/dbg.h>

/*
 * dbg_add_hdr - add a string for the debug header
 * @hdr:	Pointer to a pointer to the header format
 * @p:		Pointer to location where the string should be stored
 * @remaining:	Remaining space at @p
 * @file:	Pointer to name of the file
 * @func:	Pointer to name of the function
 * @lineno:	Line number
 *
 * Returns the size of the generated header on success. This may be greater
 * than @remaining, in which case only @remaining characters are actually
 * stored. This value is negative if an error occurred.
 */
static int dbg_add_hdr(const char **hdr, char *p, int remaining,
	const char *file, const char *func, unsigned lineno)
{
	struct timespec now;
	int rc;
	const char *basename;
	char c;

	c = *(++*hdr);

	switch (c) {
	case '%':
		*p = '%';
		rc = 1;
		break;
	
	case 'B':
		basename = strrchr(file, '/');
		if (basename == NULL)
			basename = file;
		else
			basename++;
		rc = snprintf(p, remaining, "%s", basename);
		break;

	case 'f':
		rc = snprintf(p, remaining, "%s", func);
		break;

	case 'F':
		rc = snprintf(p, remaining, "%s", file);
		break;

	case 'l':
		rc = snprintf(p, remaining, "%u", lineno);
		break;

	case 'p':
		rc = snprintf(p, remaining, "%d", getpid());
		break;

#ifdef HAS_GETTID
	case 't':
		rc = snprintf(p, remaining, "%d", gettid());
		break;
#endif
	
	case 'T':
		rc = clock_gettime(CLOCK_MONOTONIC, &now);
		rc = snprintf(p, remaining, "%ld.%06ld", now.tv_sec,
			now.tv_nsec / 1000);
		break;

	default:
		/* Behavior is undefined here. We choose to return a
		 * non-negative value to ensure that some sort of a debug
		 * string is at least printed */
		rc = 0;
		break;
	}

	return rc;
}

int __vdbg(const char *hdr, const char *file, const char *func, unsigned lineno,
	const char *fmt, va_list ap)
{
	char buf[256];
	char *p;
	int remaining, rc, rc2;
	char c;

	rc = 0;
	p = buf;
	remaining = sizeof(buf);

	do {
		c = *hdr;

		switch (c) {
		case '%':
			rc2 = dbg_add_hdr(&hdr, p, remaining, file, func,
				lineno);
			break;

		case '\0':
			/* Now at end of the header format string. Add the
			 * user's message */
			rc2 = vsnprintf(p, remaining, fmt, ap);
			break;

		default:
			/* If we have at least enough space for the character
			 * and the terminating NUL, store the character. */
			if (remaining > 2)
				*p = c;
			rc2 = 1;
			break;
		}


		if (rc2 < 0) {
			rc = rc2;
		} else {
			unsigned added;

			rc += rc2;
			added = MIN(rc2, remaining);
			remaining -= added;
			p += added;
		}

		hdr++;
	} while (rc >= 0 && c != '\0');

	if (rc >= 0) {
		rc2 = printf("%s", buf);
		if (rc2 < 0)
			rc = rc2;
	}

	return rc;
}

/*
 * __vdbg_hex - print a hex buffer
 * @hdr:	Debg header
 * @file:	Name of the file
 * @func:	Name of the function
 * @lineno:	Line number
 * @in_buf:	Pointer to the buffer to print
 * @size:	Number of bytes to print
 * @fmt:	Pointer to printf-like format string
 * @ap:		Arguments to format
 */
int __vdbg_hex(const char *hdr, const char *file, const char *func,
	unsigned lineno, const void *in_buf, size_t size, const char *fmt,
	va_list ap)
{
	unsigned i;

	__vdbg(hdr, file, func, lineno, fmt, ap);

	for (i = 0; i < (size + N_ROW_ITEMS - 1) / N_ROW_ITEMS; i++) {
		char out_buf[256];
		const char *sep;
		char *p;
		size_t remaining;
		int j;
		int n_cols;
		int rc;

		p = out_buf;
		remaining = sizeof(out_buf);

		rc = snprintf(p, remaining, "%02x: ", i);
		if (rc < 0)
			return rc;

		p += rc;
		remaining -= rc;
		n_cols = MIN(size - i * N_ROW_ITEMS, N_ROW_ITEMS);

		sep = "";
		for (j = 0; j < n_cols; j++) {
			rc = snprintf(p, remaining, "%s%02x", sep,
				((unsigned char *)in_buf)[i * N_ROW_ITEMS + j]);
			if (rc < 0)
				return rc;
			p += rc;
			remaining -= rc;
			sep = " ";
		}
		rc = printf("%s%s", out_buf, EOL);
		if (rc < 0)
			return rc;
	}

	return 0;
}

/*
 * __vdbg_hex - print a hex buffer
 * @hdr:	Debug header
 * @file:	Name of the file
 * @func:	Name of the function
 * @lineno:	Line number
 * @data:	Pointer to the buffer to print
 * @size:	Number of bytes to print
 * @fmt:	Pointer to printf-like format string
 * @va:		Arguments to format
 */
int __vdbg_hex_iov(const char *hdr, const char *func, const char *file,
	unsigned lineno, const struct iovec *vec, size_t iovcnt,
	const char *fmt, va_list ap)
{
	char out_buf[256];
	char *p;
	size_t remaining;
	unsigned c;
	unsigned i;
	unsigned j;
	unsigned k;
	const char *sep;
	int rc;

	__vdbg(hdr, file, func, lineno, fmt, ap);

	rc = 0;
	p = out_buf;
	remaining = sizeof(out_buf);
	i = 0;
	sep = " ";

	for (j = 0; j < iovcnt; j++) {
		for (k = 0; k < vec[j].iov_len; k++) {
			c = ((unsigned char *)vec[j].iov_base)[k];

			if ((i % N_ROW_ITEMS) == 0) {
				rc = snprintf(p, remaining, "%02x:", i);
				if (rc < 0)
					return rc;
				p += rc;
				remaining -= rc;
			}

			rc = snprintf(p, remaining, "%s%02x", sep, c);
			if (rc < 0)
				return rc;
			p += rc;
			remaining -= rc;

			i++;
			if ((i % N_ROW_ITEMS) == 0) {
				printf("%s%s", out_buf, EOL);
				p = out_buf;
				remaining = sizeof(out_buf);
			}

			sep = " ";
		}

		sep = "*";
	}

	return rc;
}
