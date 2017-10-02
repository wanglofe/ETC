#ifndef _DEBUG_H
#define _DEBUG_H

//printf FORMAT OPTIONS

#ifdef _DEBUG
#define debug(format, arg...) printf(format, ##arg)
#define debug_verbose(format, arg...) printf("[%s, %s: %d]: "format, __FILE__, __func__, __LINE__,  ##arg)
#define print(format, arg...) printf(format"\n", ##arg)
#else
#define debug(format, arg...)
#define debug_verbose(format, arg...)
#define print(format, arg...)
#endif

#endif //debug.h


