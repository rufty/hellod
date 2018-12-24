// For PATH_MAX
#if __linux
#define _POSIX_SOURCE
#endif //__linux

#include <stdio.h>
#include <limits.h>

#include "config.h"
#include "hellod.h"


// Called while still in foreground.
void
begin()
  {
  }


// Called once detached.
void
setup()
  {
  }


// Called every <interval> seconds.
#include <time.h>
#define SIZE 20
void
loop()
  {
  char buf[SIZE] ;
  time_t curtime ;
  struct tm* loctime ;
  // Get the current time.
  curtime = time ( NULL ) ;
  // Convert it to local representation.
  loctime =  localtime ( &curtime ) ;
  // Get the time in a nice format.
  strftime ( buf, SIZE, "%Y-%m-%d %H:%M:%S", loctime ) ;
  // Dump it, and the message, out.
  syserr ( LOG_NOTICE, "%s :- %s", settings.message, buf ) ;
  }


// Called when quitting.
void
finish()
  {
  }


// VIM formatting info.
// vim:ts=2:sw=2:tw=114:fo=tcnq2b:foldmethod=indent
