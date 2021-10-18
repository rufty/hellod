#ifndef HELLOD_H
#define HELLOD_H


// Standard infractracture.
#include <stdint.h>
#include <stdbool.h>
// Unix logging.
#include <syslog.h>
// For NAME_MAX, PATH_MAX
#ifdef __linux__
#include <linux/limits.h>
#else
#include <limits.h>
#endif //__linux__


// Configuration variables. ADDARG
struct Settings
  {
  bool detach ;
  int verbose ;
  int interval ;
  char* message ;
  } ;
extern struct Settings settings ;


// What we're called.
extern char dname[NAME_MAX] ;
// Where to put the PID file.
extern char pidfile[PATH_MAX] ;


// Utility for redirecting messages.
void syserr ( int priority,  char* msg, ... ) ;

// Called while still in foreground.
void begin ( void ) ;
// Called once in the background.
void setup ( void ) ;
// Called every <runinterval> seconds.
void loop ( void ) ;
// Called when quitting.
void finish ( void ) ;


#endif //HELLOD_H

// VIM formatting info.
// vim:ts=2:sw=2:tw=120:fo=tcnq2b:foldmethod=indent
