#ifndef  __LN_UTILS_H__
#define  __LN_UTILS_H__


/* Bit map macros */
#define LN_BIT(n)                (1U << (n))
#define LN_SET_BIT(x,n)          ((x) |=  (1U << (n)))
#define LN_CLR_BIT(x,n)          ((x) &= ~(1U << (n)))
#define LN_REVERSE_BIT(x,n)      ((x) ^=  (1U << (n)))
#define LN_GET_BIT(x,n)          (((x) >> (n)) & 1U)
#define LN_IS_BIT_SET(x,n)       (((x) >> (n)) & 1U)
#define LN_IS_BIT_CLR(x,n)       ((((x) >> (n)) & 1U) == 0)


/* Macros for counting and rounding. */
#define LN_NELEMENTS(array)      (sizeof(array) / sizeof(array[0]))
#define LN_NITEMS(array)         (sizeof(array) / sizeof(array[0]))
#define	LN_HOWMANY(x, y)	     (((x)+((y)-1)) / (y))
#define	LN_ROUNDDOWN(x, n)       (((x)/(n)) * (n))
#define	LN_ROUNDDOWN2(x, n)      ((x) & (~((n)-1)))           /* if n is power of two */
#define	LN_ROUNDUP(x, n)         ((((x)+((n)-1))/(n)) * (n))  /* to any n */
#define	LN_ROUNDUP2(x, n)        (((x)+((n)-1)) & (~((n)-1))) /* if n is powers of two */
#define LN_POWEROF2(x)           ((((x)-1)&(x)) == 0)
#define LN_DIV_ROUNDUP(x, n)     (((x) + (n) - 1) / (n))


/* MAC addr hex-printf */
#ifndef MAC2STR
#define MAC2STR(a)               (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR                   "%02X:%02X:%02X:%02X:%02X:%02X"
#endif


/* Macros for min/max. */
#ifndef MIN
#define	MIN(a,b)                 (((a)<(b)) ? (a) : (b))
#endif
#ifndef MAX
#define	MAX(a,b)                 (((a)>(b)) ? (a) : (b))
#endif

/** Eliminates compiler warning about unused arguments (GCC -Wextra -Wunused). */
#ifndef LN_UNUSED
#define LN_UNUSED(x)            ((void)(x))
#endif


#endif /* __LN_UTILS_H__ */

