/*
 * Definition of gettid()
 */
#ifndef _GETTID_H_
#define _GETTID_H_

#ifdef __linux__
#include <sys/types.h>

extern pid_t gettid(void);
#endif	/* __linux__ */

#endif /* _GETTID_H_ */
