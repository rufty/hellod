// For NAME_MAX, PATH_MAX
#ifdef __linux__
#include <linux/limits.h>
#else
#include <limits.h>
#endif //__linux__

// Globals.
#include "hellod.h"
// The settings.
#include "settings.h"


// What we're called.
char dname[NAME_MAX] ;
// Where to put the PID file.
char pidfile[PATH_MAX] ;
// The setting variables.
struct Settings settings ;


// VIM formatting info.
// vim:ts=2:sw=2:tw=120:fo=tcnq2b:foldmethod=indent
