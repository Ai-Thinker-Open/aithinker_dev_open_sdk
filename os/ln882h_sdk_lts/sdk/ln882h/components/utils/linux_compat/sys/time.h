/**
 * Linux compat
*/
#ifndef __SYS_TIME_H__
#define __SYS_TIME_H__

#include <stdlib.h> /* std libc */
#include <stdint.h> /* std libc */
/**
 * ARMCC
 *      ARM\ARMCC\include\time.h
 *      ARM\ARMCLANG\include\time.h
 *      cpu time type
 *          typedef unsigned int clock_t;
 *      date/time in unix secs past 1-Jan-70
 *          typedef unsigned int time_t;
*/
#include <time.h> /* std libc */

#ifndef suseconds_t
typedef long             suseconds_t;   /* microseconds. */
#endif

#ifndef useconds_t
typedef unsigned long    useconds_t;    /* microseconds (unsigned) */
#endif

struct timezone
{
    int tz_minuteswest;   /* minutes west of Greenwich */
    int tz_dsttime;       /* type of dst correction */
};

#ifndef _TIMEVAL_DEFINED
#define _TIMEVAL_DEFINED
struct timeval
{
    time_t      tv_sec;     /* seconds */
    suseconds_t tv_usec;    /* and microseconds */
};
#endif /* _TIMEVAL_DEFINED */

int stime(const time_t *t);
time_t timegm(struct tm * const t);
int gettimeofday(struct timeval *tv, struct timezone *tz);
int settimeofday(const struct timeval *tv, const struct timezone *tz);
#if defined(__ARMCC_VERSION) || defined (__ICCARM__)
struct tm *gmtime_r(const time_t *timep, struct tm *r); /* ARMCC is not defined, gmtime is defined */
struct tm* localtime_r(const time_t* t, struct tm* r);  /* ARMCC is defined if not defined __STRICT_ANSI__ */
char* asctime_r(const struct tm *t, char *buf);         /* ARMCC is defined if not defined __STRICT_ANSI__ */
char *ctime_r(const time_t * tim_p, char * result);     /* ARMCC is not defined, ctime is defined */
#endif

/* timezone */
void tz_set(int8_t tz);
int8_t tz_get(void);
int8_t tz_is_dst(void);

#endif /* __SYS_TIME_H__ */
