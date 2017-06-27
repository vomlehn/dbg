/*
 * Before #including this file, #define DBG to be zero to disable debugging output,
 * non-zero to enable it
 */

#ifndef _HORD_DBG_H_
#define _HORD_DBG_H_
#include <sys/uio.h>

#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int __vdbg_hex(const char *hdr, const char *file, const char *func,
	unsigned lineno, const void *in_buf, size_t size, const char *fmt,
	va_list ap) __attribute__((format (printf, 7, 0)));
int __vdbg_hex_iov(const char *hdr, const char *file, const char *func,
	unsigned lineno, const struct iovec *vec, size_t iovcnt,
	const char *fmt, va_list va) __attribute__((format(printf, 7, 0)));
int __vdbg(const char *hdr, const char *file, const char *func, unsigned line,
	const char *fmt, va_list ap) __attribute__((format(printf, 5, 0)));

static inline int __dbg(bool cond, const char *FILE, const char *func,
	unsigned line, const char *fmt, ...)
	__attribute__((format(printf,5,6)));
static inline int __dbg_hex(bool cond, const char *file, const char *func,
	unsigned lineno, const void *in_buf, size_t size, const char *fmt, ...)
	__attribute__((format(printf, 7, 8)));
static inline int __dbg_hex_iov(bool cond, const char *file, const char *func,
	unsigned lineno, const struct iovec *vec, size_t iovcnt,
	const char *fmt, ...)
	__attribute__((format(printf, 7, 8)));

#ifndef DBG
#warning DBG should be #defined as a numeric value
#endif

#if DBG
#include <stdarg.h>

/*
 * DBG_HEADER - control string to print before general debugging message
 * This consists of a series of characters with embedded formatting control
 * substrings. The substrings start with '%' and are followed by a singal
 * character. Possible printed values are:
 * %%:	The percent character
 * %B:	Final component of the pathname for the file in which the debug
 *	message is printed
 * %f:	function in which the debug message is printed, as given by the
 *	C preprocessor __func__
 * %F:	file in which the debug message is printed, as given by the
 *	C preprocessor __FILE__
 * %l:	line number in which the debug message is printed, as given by the
 *	C preprocessor __LINE__
 * %p:	Process ID
 * %t:	Thread ID
 * %T:	Current time
 *
 * If DBG_HEADER was not defined by the user before #including this file,
 * a default value is supplied.
 */
#ifndef DBG_HEADER
#define DBG_HEADER	"%f:%l: "
#endif

static inline int __dbg(bool cond, const char *file, const char *func,
	unsigned line, const char *fmt, ...)
{
	va_list ap;
	int rc;

	if (!cond)
		return 0;

	va_start(ap, fmt);
	rc = __vdbg(DBG_HEADER, file, func, line, fmt, ap);
	va_end(ap);

	return rc;
}

static inline int __dbg_hex(bool cond, const char *file, const char *func,
	unsigned lineno, const void *in_buf, size_t size, const char *fmt, ...)
{
	va_list ap;
	int rc;

	if (!cond)
		return 0;

	va_start(ap, fmt);
	rc = __vdbg_hex(DBG_HEADER, file, func, lineno, in_buf, size, fmt, ap);
	va_end(ap);

	return rc;
}

static inline int __dbg_hex_iov(bool cond, const char *file, const char *func,
	unsigned lineno, const struct iovec *vec, size_t iovcnt,
	const char *fmt, ...)
{
	va_list ap;
	int rc;

	if (!cond)
		return 0;

	va_start(ap, fmt);
	rc = __vdbg_hex_iov(DBG_HEADER, file, func, lineno, vec, iovcnt, fmt,
		ap);
	va_end(ap);

	return rc;
}
#else
static inline int __dbg(bool cond, const char *file, const char *func,
	unsigned line, const char *fmt, ...)
{
	return 0;
}

static inline int __dbg_hex(bool cond, const char *file, const char *func,
	unsigned lineno, const void *in_buf, size_t size, const char *fmt, ...)
{
	return 0;
}

static inline int __dbg_hex_iov(bool cond, const char *file, const char *func,
	unsigned lineno, const struct iovec *vec, size_t iovcnt,
	const char *fmt, ...)
{
	return 0;
}
#endif

/*
 * dbg_cond - print a debugging message if a condition is true
 * @cond:	Boolean expression
 * @fmt:	Printf format string
 * @...:	Arguments to the format string
 */
#define dbg_cond(cond, fmt, ...) __dbg((cond), __FILE__, __func__, __LINE__, \
	(fmt), ##__VA_ARGS__)

/*
 * dbg_t - always print a debugging message
 * @fmt:	Printf format string
 * @...:	Arguments to the format string
 */
#define dbg_t(fmt, ...) dbg_cond(true, fmt, ##__VA_ARGS__)

/*
 * dbg_f - never print a debugging message
 * @fmt:	Printf format string
 * @...:	Arguments to the format string
 */
#define dbg_f(fmt, ...) dbg_cond(false, fmt, ##__VA_ARGS__)


/*
 * dbg - always print a debugging message--synonym for dbg_t
 * @fmt:	Printf format string
 * @...:	Arguments to the format string
 */
#define dbg(fmt, ...) dbg_t(fmt, ##__VA_ARGS__)

/*
 * dbg_hex - dump a hex buffer
 * @in_buf:	Pointer to data to dump
 * @size:	Number of bytes to dump
 * @fmt:	Pointer to printf-like format string
 * @...:	Arguments to format string
 */
#define dbg_hex(in_buf, size, fmt, ...) __dbg_hex(true, __FILE__, __func__, \
	__LINE__, in_buf, size, (fmt), ##__VA_ARGS__)

/*
 * dbg_hex_cond - dump a hex buffer
 * @cond:	True to print the buffer, false otherwise
 * @in_buf:	Pointer to data to dump
 * @size:	Number of bytes to dump
 * @fmt:	Pointer to printf-like format string
 * @...:	Arguments to format string
 */
#define dbg_hex_cond(cond, in_buf, size, fmt, ...) __dbg_hex((cond), __FILE__, \
	__func__, __LINE__, in_buf, size, (fmt), ##__VA_ARGS__)

/*
 * dbg_hex - dump a hex buffer
 * @vec:	Pointer to an array of struct iovec items
 * @iovcnt:	Number of items in the @vec array
 * @fmt:	Pointer to printf-like format string
 * @...:	Arguments to format string
 */
#define dbg_hex_iov(vec, iovcnt, fmt, ...) __dbg_hex_iov(true, __FILE__, \
	__func__, __LINE__, vec, iovcnt, (fmt), ##__VA_ARGS__)

/*
 * dbg_hex_iov_cond - dump a hex buffer
 * @cond:	True to print the buffer, false otherwise
 * @vec:	Pointer to an array of struct iovec items
 * @iovcnt:	Number of items in the @vec array
 * @fmt:	Pointer to printf-like format string
 * @...:	Arguments to format string
 */
#define dbg_hex_iov_cond(cond, vec, iovcnt, fmt, ...) __dbg_hex_iov((cond), \
	__FILE__, __func__, __LINE__, vec, iovcnt, (fmt), ##__VA_ARGS__)

#endif	/* _HORD_DBG_H_ */
