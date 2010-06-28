// leavealloc.cpp
//
// Copyright © Symbian Software Ltd 2004.  All rights reserved.
//

#include <e32std.h>
#include <stdlib.h>
#include "leavealloc.h"



void* mallocL(size_t _size)
    {
    void* ptr = malloc(_size);
    if (!ptr) {
       User::Leave(KErrNoMemory);
    }
    return ptr;
    }

void* callocL(size_t _nmemb, size_t _size)
    {
    void* ptr = calloc(_nmemb, _size);
    if (!ptr) {
       User::Leave(KErrNoMemory);
    }
    return ptr;
    }

void* reallocL(void * _r, size_t _size)
    {
    void* ptr = realloc(_r, _size);
    if (!ptr) {
       User::Leave(KErrNoMemory);
    }
    return ptr;
    }

void freeL(void * _ptr)
    {
    free(_ptr);
    }
