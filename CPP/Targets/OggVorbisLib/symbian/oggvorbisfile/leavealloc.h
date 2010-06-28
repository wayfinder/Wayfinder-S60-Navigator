// leavealloc.h
//
// Copyright © Symbian Software Ltd 2004.  All rights reserved.
//


#ifndef __LEAVEALLOC_H__
#define __LEAVEALLOC_H__

/*  #ifdef __cplusplus */
/*  extern "C" { */
/*  #endif */

#include <stdlib.h>

void* mallocL(size_t _size);

void* callocL(size_t _nmemb, size_t _size);

void* reallocL(void * _r, size_t _size);

void freeL(void *);


/*  #ifdef __cplusplus */
/*  } */
/*  #endif */


#endif
