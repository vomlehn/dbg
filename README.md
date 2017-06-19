dbg library
==========
Flexible debugging message printing library with configurable header data
and atomic printing.

Every program eventually gets tired of writing out error mssages with
printing and devising methods for turning debugging messages off and off.
They often wind up printing function names for each message, too, which
is an additional nuisance. This library tries to simplify that procedure.

To use it, first build it. This is only been built with GCC, modifications
to work with other build chains are welcome. Just type "make" and it will
build a libdbg.so in the current directory. It has other, non-default
targets:

Target	Directory			Use
-------	-------------------------------	-------------------
staging	$(STAGING_DIR)/usr/include	Puts dbg.h here

	$(STAGING_DIR)/usr/lib		Puts libdbg.so here

install	$(TARGET_DIR)/usr/lib		Puts libdbg.so here

Both STAGING_DIR and TARGET_DIR are defined in the Makefile but can be
overridden.

Typical usage
=============
#define DBG	1			/* Enable output messages */
#define DBG_HEADER	"%T:%f:%l: "	/* Print time, function, line number */
#include <dbg.h>			/* Get debugging definitions */

Dbg will first assemble an entire line of output before printing it. This
helps keep each line of debugging information together, though there is not
guarantee that this will be the case,

Definitions
===========
DBG			This is the key one. If dbg.h is used, this must be
			defined.  It it is 0, all debugging messages are
			disabled, otherwise the specified messages will be
			enabled.

DBG_HEADER		This is a printf-like string that is interpreted and
			printed before the rest of the debugging messages. The
			interpolated format items are:

			%%	The percent character

			%B	Final component of the pathname for the file
				in which the debug message is printed

			%f	function in which the debug message is printed,
				as given by the C preprocessor __func__ symbol

			%F	file in which the debug message is printed, as
				given by the C preprocessor __FILE__ symbol

			%l	line number in which the debug message is
				printed, as given by the C preprocessor
				__LINE__ symbol

			%p	Process ID

			%t	Thread ID

			%T	Current time

			The default value is: "%f:%l: "

dbg_cond		Print a debugging message if a condition is met

			cond:	Boolean expression
			fmt:	Printf format string
			...:	Arguments to the format string

dbg_t			Always print a debugging message

			fmt:	Printf format string
			...:	Arguments to the format string

			It is often the case that you want to flick debugging
			messages on and off on an individual line basis. By
			replacing the "t" character with "f", and vice versa,
			it is simple to control output.

dbg_f			Never print the debugging message

			fmt:	Printf format string
			...:	Arguments to the format string

dbg			Always print a debugging message--synonym for dbg_t

			fmt:	Printf format string
			...:	Arguments to the format string

			This is shorter than dbg and can be used when you know
			that you always want to see the messages when debugging
			message output is enabled

dbg_hex			Dump a hex buffer

			in_buf:	Pointer to data to dump
			size:	Number of bytes to dump
			fmt:	Pointer to printf-like format string
			...:	Arguments to format string

dbg_hex_cond		Dump a hex buffer if a condition is met

			cond:	True to print the buffer, false otherwise
			in_buf:	Pointer to data to dump
			size:	Number of bytes to dump
			fmt:	Pointer to printf-like format string
			...:	Arguments to format string

dbg_hex_iov		Dump a series of hex buffers using struct iovec
			descriptors. The data is printed and number
			contiguously with an asterisk inserted between
			the sections specified by a struct iovec

			vec:	Pointer to an array of struct iovec items
			iovcnt:	Number of items in the @vec array
			fmt:	Pointer to printf-like format string
			...:	Arguments to format string

dbg_hex_iov_cond	Dump a hex buffer specified by a series of struct iovec
			elements, when a condition is met

			cond:	True to print the buffer, false otherwise
			vec:	Pointer to an array of struct iovec items
			iovcnt:	Number of items in the @vec array
			fmt:	Pointer to printf-like format string
			...:	Arguments to format string
#endif	/* _HORD_DBG_H_ */
