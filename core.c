// Standard infrastructure.
#include <stdio.h>

// Compile-time defults.
#include "config.h"
// The settings.
#include "settings.h"
// Globals.
#include "hellod.h"


// Called while still in foreground.
void
begin()
  {
  syserr ( LOG_NOTICE, "%s : begin()", settings.message ) ;
  }


// Called once detached.
void
setup()
  {
  syserr ( LOG_NOTICE, "%s : setup()", settings.message ) ;
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
  syserr ( LOG_NOTICE, "%s : finish()", settings.message ) ;
  }


// VIM formatting info.
// vim:ts=2:sw=2:tw=120:fo=tcnq2b:foldmethod=indent
