#ifndef INVENTORYTRACKER_GLOBAL_H
#define INVENTORYTRACKER_GLOBAL_H

#include <Qt/qglobal.h>

#ifdef INVENTORYTRACKER_LIB
# define INVENTORYTRACKER_EXPORT Q_DECL_EXPORT
#else
# define INVENTORYTRACKER_EXPORT Q_DECL_IMPORT
#endif

#endif // INVENTORYTRACKER_GLOBAL_H
