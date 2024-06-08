#pragma once
#ifndef MEMUTILS_H
#define MEMUTILS_H

#include <stdlib.h>
#include <stdio.h>

void* xmalloc(size_t size);
void* xrealloc(void* ptr, size_t size);
void* xcalloc(size_t count, size_t size);

#endif
