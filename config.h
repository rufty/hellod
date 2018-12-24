#ifndef CONFIG_H
#define CONFIG_H


// Convert macros to strings.
#define STR(S) _STR(S)
#define _STR(S) #S

// Set compile-time defaults.
#define INTERVAL 45
#define MESSAGE STR(APPNAME)

// Verbosity.
#ifdef DEBUG
#define VERBOSITY 9
#define LOGMAX LOG_DEBUG
#else
#define VERBOSITY 1
#define LOGMAX LOG_NOTICE
#endif //DEBUG

// Where to put the PID file.
#define PIDPATH "/var/run"


#endif //CONFIG_H

// VIM formatting info.
// vim:ts=2:sw=2:tw=114:fo=tcnq2b:foldmethod=indent
