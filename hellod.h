#ifndef HELLOD_H
#define HELLOD_H


#include <syslog.h>
#include <stdint.h>
#include <stdbool.h>


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
extern char dname[] ;
// Where to put the PID file.
extern char pidfile[] ;


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
// vim:ts=2:sw=2:tw=114:fo=tcnq2b:foldmethod=indent
