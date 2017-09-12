#ifndef AE_ZMALLOC_H
#define AE_ZMALLOC_H

#include <stdlib.h>

/* just use the glibc */
#define zmalloc malloc
#define zrealloc realloc
#define zcalloc calloc
#define zfree free

#endif /* AE_ZMALLOC_H */