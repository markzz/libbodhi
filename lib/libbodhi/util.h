#ifndef BODHI_UTIL_H
#define BODHI_UTIL_H

#define MALLOC(p, s, action) do { p = malloc(s); if (p == NULL) { action; } } while(0)
#define CALLOC(p, l, s, action) do { p = calloc(l, s); if (p == NULL) { action; } } while(0)
#define FREE(p) do { if (p != NULL) { free(p); p = NULL; } } while(0)

#define ASSERT(cond, action) do { if (!(cond)) { action; } } while(0)

#endif
