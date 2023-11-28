#include "sys/time.h" /* search from local sys dirctory */
#include "osal/osal.h"
#include "ln_compiler.h"

static uint32_t init_tick;
static time_t init_time;

/* seconds per day */
#define SPD 24*60*60

/* days per month -- nonleap! */
static const short __spm[13] =
{
    0,
    (31),
    (31 + 28),
    (31 + 28 + 31),
    (31 + 28 + 31 + 30),
    (31 + 28 + 31 + 30 + 31),
    (31 + 28 + 31 + 30 + 31 + 30),
    (31 + 28 + 31 + 30 + 31 + 30 + 31),
    (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31),
    (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30),
    (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),
    (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30),
    (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31),
};

__ALIGNED__(4) static const char *days = "Sun Mon Tue Wed Thu Fri Sat ";
__ALIGNED__(4) static const char *months = "Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec ";

static int __isleap(int year)
{
    /* every fourth year is a leap year except for century years that are
     * not divisible by 400. */
    /*  return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)); */
    return (!(year % 4) && ((year % 100) || !(year % 400)));
}

static void num2str(char *c, int i)
{
    c[0] = i / 10 + '0';
    c[1] = i % 10 + '0';
}

struct tm *gmtime_r(const time_t *timep, struct tm *r)
{
    time_t i;
    time_t work = *timep % (SPD);
    r->tm_sec = work % 60;
    work /= 60;
    r->tm_min = work % 60;
    r->tm_hour = work / 60;
    work = *timep / (SPD);
    r->tm_wday = (4 + work) % 7;
    for (i = 1970;; ++i)
    {
        time_t k = __isleap(i) ? 366 : 365;
        if (work >= k)
            work -= k;
        else
            break;
    }
    r->tm_year = i - 1900;
    r->tm_yday = work;

    r->tm_mday = 1;
    if (__isleap(i) && (work > 58))
    {
        if (work == 59)
            r->tm_mday = 2; /* 29.2. */
        work -= 1;
    }

    for (i = 11; i && (__spm[i] > work); --i)
        ;
    r->tm_mon = i;
    r->tm_mday += work - __spm[i];

    r->tm_isdst = tz_is_dst();
    return r;
}

struct tm* gmtime(const time_t* t)
{
    static struct tm tmp;
    return gmtime_r(t, &tmp);
}

struct tm* localtime_r(const time_t* t, struct tm* r)
{
    time_t local_tz;

    local_tz = *t + (time_t)tz_get() * 3600;
    return gmtime_r(&local_tz, r);
}

struct tm* localtime(const time_t* t)
{
    static struct tm tmp;
    return localtime_r(t, &tmp);
}

time_t mktime(struct tm * const t)
{
    time_t timestamp;

    timestamp = timegm(t);
    timestamp = timestamp - 3600 * (time_t)tz_get();
    return timestamp;
}

char* asctime_r(const struct tm *t, char *buf)
{
    /* Checking input validity */
    if ((int)strlen(days) <= (t->tm_wday << 2) || (int)strlen(months) <= (t->tm_mon << 2))
    {
        //LOG(LOG_LVL_INFO, "asctime_r: the input parameters exceeded the limit, please check it.");
        *(int*) buf = *(int*) days;
        *(int*) (buf + 4) = *(int*) months;
        num2str(buf + 8, t->tm_mday);
        if (buf[8] == '0')
            buf[8] = ' ';
        buf[10] = ' ';
        num2str(buf + 11, t->tm_hour);
        buf[13] = ':';
        num2str(buf + 14, t->tm_min);
        buf[16] = ':';
        num2str(buf + 17, t->tm_sec);
        buf[19] = ' ';
        num2str(buf + 20, 2000 / 100);
        num2str(buf + 22, 2000 % 100);
        buf[24] = '\n';
        buf[25] = '\0';
        return buf;
    }

    /* "Wed Jun 30 21:49:08 1993\n" */
    *(int*) buf = *(int*) (days + (t->tm_wday << 2));
    *(int*) (buf + 4) = *(int*) (months + (t->tm_mon << 2));
    num2str(buf + 8, t->tm_mday);
    if (buf[8] == '0')
        buf[8] = ' ';
    buf[10] = ' ';
    num2str(buf + 11, t->tm_hour);
    buf[13] = ':';
    num2str(buf + 14, t->tm_min);
    buf[16] = ':';
    num2str(buf + 17, t->tm_sec);
    buf[19] = ' ';
    num2str(buf + 20, (t->tm_year + 1900) / 100);
    num2str(buf + 22, (t->tm_year + 1900) % 100);
    buf[24] = '\n';
    buf[25] = '\0';
    return buf;
}

char* asctime(const struct tm *timeptr)
{
    static char buf[26];
    return asctime_r(timeptr, buf);
}

char *ctime_r(const time_t * tim_p, char * result)
{
    struct tm tm;
    return asctime_r(localtime_r(tim_p, &tm), result);
}

char* ctime(const time_t *tim_p)
{
    return asctime(localtime(tim_p));
}

double difftime(time_t time1, time_t time2)
{
    return (double)(time1 - time2);
}

/**
 * Returns the current time.
 *
 * @param time_t * t the timestamp pointer, if not used, keep NULL.
 *
 * @return The value ((time_t)-1) is returned if the calendar time is not available.
 *         If timer is not a NULL pointer, the return value is also stored in timer.
 *
 */
__WEAK__ time_t time(time_t *t)
{
    struct timeval now;

    now.tv_sec = init_time + (OS_GetTicks() - init_tick)/OS_HZ;

    if (t)
    {
        *t = now.tv_sec;
    }
    return now.tv_sec;
}

__WEAK__ clock_t clock(void)
{
    return OS_GetTicks();
}

int stime(const time_t *t)
{
    struct timeval tv;

    if (!t)
    {
        return -1;
    }

    tv.tv_sec = *t;
    init_time = tv.tv_sec - (OS_GetTicks() - init_tick)/OS_HZ;

    return 0;
}

time_t timegm(struct tm * const t)
{
    time_t day;
    time_t i;
    time_t years = (time_t)t->tm_year - 70;

    if (t->tm_sec > 60)         /* seconds after the minute - [0, 60] including leap second */
    {
        t->tm_min += t->tm_sec / 60;
        t->tm_sec %= 60;
    }
    if (t->tm_min >= 60)        /* minutes after the hour - [0, 59] */
    {
        t->tm_hour += t->tm_min / 60;
        t->tm_min %= 60;
    }
    if (t->tm_hour >= 24)       /* hours since midnight - [0, 23] */
    {
        t->tm_mday += t->tm_hour / 24;
        t->tm_hour %= 24;
    }
    if (t->tm_mon >= 12)        /* months since January - [0, 11] */
    {
        t->tm_year += t->tm_mon / 12;
        t->tm_mon %= 12;
    }
    while (t->tm_mday > __spm[1 + t->tm_mon])
    {
        if (t->tm_mon == 1 && __isleap(t->tm_year + 1900))
        {
            --t->tm_mday;
        }
        t->tm_mday -= __spm[t->tm_mon];
        ++t->tm_mon;
        if (t->tm_mon > 11)
        {
            t->tm_mon = 0;
            ++t->tm_year;
        }
    }

    if (t->tm_year < 70)
        return (time_t) - 1;

    /* Days since 1970 is 365 * number of years + number of leap years since 1970 */
    day = years * 365 + (years + 1) / 4;

    /* After 2100 we have to substract 3 leap years for every 400 years
     This is not intuitive. Most mktime implementations do not support
     dates after 2059, anyway, so we might leave this out for it's
     bloat. */
    if (years >= 131)
    {
        years -= 131;
        years /= 100;
        day -= (years >> 2) * 3 + 1;
        if ((years &= 3) == 3)
            years--;
        day -= years;
    }

    day += t->tm_yday = __spm[t->tm_mon] + t->tm_mday - 1 +
                        (__isleap(t->tm_year + 1900) & (t->tm_mon > 1));

    /* day is now the number of days since 'Jan 1 1970' */
    i = 7;
    t->tm_wday = (day + 4) % i; /* Sunday=0, Monday=1, ..., Saturday=6 */

    i = 24;
    day *= i;
    i = 60;
    return ((day + t->tm_hour) * i + t->tm_min) * i + t->tm_sec;
}

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    /* The use of the timezone structure is obsolete;
     * the tz argument should normally be specified as NULL.
     * The tz_dsttime field has never been used under Linux.
     * Thus, the following is purely of historic interest.
     */
    if(tz != NULL)
    {
        tz->tz_dsttime = 0;
        tz->tz_minuteswest = -(tz_get() * 60);
    }

    if (tv != NULL)
    {
        tv->tv_usec = 0; /* ignore it */
        tv->tv_sec = init_time + (OS_GetTicks() - init_tick)/OS_HZ;
        return 0;
    }
    else
    {
        return -1;
    }
}

int settimeofday(const struct timeval *tv, const struct timezone *tz)
{
    /* The use of the timezone structure is obsolete;
     * the tz argument should normally be specified as NULL.
     * The tz_dsttime field has never been used under Linux.
     * Thus, the following is purely of historic interest.
     */
    if (tv != NULL
        && tv->tv_usec >= 0)
    {
        init_time = tv->tv_sec - (OS_GetTicks() - init_tick)/OS_HZ;
        return 0;
    }
    else
    {
        return -1;
    }
}


/* timezone */
#ifndef LINUX_COMPAT_DEFAULT_TIMEZONE
#define LINUX_COMPAT_DEFAULT_TIMEZONE    0
#endif

static volatile int8_t _current_timezone = LINUX_COMPAT_DEFAULT_TIMEZONE;

void tz_set(int8_t tz)
{
    // disable global interrupt
    _current_timezone = tz;
    // enable global interrupt
}

int8_t tz_get(void)
{
    return _current_timezone;
}

int8_t tz_is_dst(void)
{
    return 0;
}

/* 2022-02-02 02:02:02 = LOCAL_TIME_INIT(2022, 2, 2, 2, 2, 2)  */
#define LOCAL_TIME_INIT(year, month, day, hour, minute, second)                 \
    {                                                                           \
        .tm_year = year - 1900,                                                 \
        .tm_mon = month - 1,                                                    \
        .tm_mday = day,                                                         \
        .tm_hour = hour,                                                        \
        .tm_min = minute,                                                       \
        .tm_sec = second                                                        \
    }

#define LOCAL_TIME_DEFAULT                    LOCAL_TIME_INIT(2022, 2, 2, 2, 2, 2)
void linux_copat_time_init(void)
{
    struct tm time_new = LOCAL_TIME_DEFAULT;
    init_tick = OS_GetTicks();
    init_time = timegm(&time_new);
}
