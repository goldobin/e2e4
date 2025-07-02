#ifndef U_FUNC_H
#define U_FUNC_H

size_t MinSizeT(size_t a, size_t b);
size_t MaxSizeT(size_t a, size_t b);

inline size_t MinSizeT(const size_t a, const size_t b) { return a < b ? a : b; }
inline size_t MaxSizeT(const size_t a, const size_t b) { return a > b ? a : b; }

#endif  // U_FUNC_H
