#ifndef FUNC_H
#define FUNC_H

#include <stddef.h>

static size_t MinSizeT(const size_t a, const size_t b) { return a < b ? a : b; }
static size_t MaxSizeT(const size_t a, const size_t b) { return a > b ? a : b; }

#endif  // FUNC_H
