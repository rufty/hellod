/****************************************************************************************************************/
/*  Purpose:    Simple "hello world" testbed service that executes a loop() at regular intervals.               */
/*  Author:     Copyright (c) 2018, W.B.Hill <mail@wbh.org> All rights reserved.                                */
/*  License:    The parts of this written by W.B.Hill are available, at your choice, under either:              */
/*                  The GPLv2 license                                                                           */
/*                Or                                                                                            */
/*                  The BSD2clause license                                                                      */
/*              See the included file LICENSE for full text.                                                    */
/****************************************************************************************************************/


// New arguments need to be added where tagged 'ADDARG'.

// For pid_t and vsyslog()
#if __linux
#define _GNU_SOURCE
#endif //__linux

// Standard infrastructure.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
// For kill()
#include <signal.h>
// For stat()
#include <sys/stat.h>

// Compile-time defults.
#include "config.h"
// The settings.
#include "settings.h"
// Globals.
#include "hellod.h"


// Utility for redirecting messages.
#include <stdarg.h>
void
syserr ( int priority,  char* msg, ... )
  {
  va_list args ;
  va_start ( args, msg ) ;
  if ( settings.detach )
    {
    vsyslog ( priority, msg, args ) ;
    }
  else
    {
    vfprintf( stderr, msg, args );
    fprintf ( stderr, "\n" ) ;
    }
  va_end ( args ) ;
  }


// Catch TERM signals and exit gracefully.
static int runloop = true ;
static void
signal_handler ( int sig )
  {
  if ( sig == SIGINT ) runloop = false ;
  if ( sig == SIGTERM ) runloop = false ;
  }


// Parse the config files, then the command line, setup and then finally do stuff.
int
main ( int argc, char* argv[] )
  {
  // Load the config files and parse the command line.
  initsettings ( argc, argv ) ;
  // Initial setup.
  begin() ;
  // Register signal handlers to terminate gracefully.
  signal ( SIGINT, signal_handler ) ;
  signal ( SIGTERM, signal_handler ) ;
  // If in the background, setup for that.
  if ( settings.detach )
    {
    pid_t pid, sid ;
    FILE* pf ;
    struct stat st ;
    // Fork off the background process.
    pid = fork();
    // A-OK?
    if ( pid < 0 )
      {
      fprintf ( stderr, "%s is unable to start background process.\n", dname ) ;
      exit ( EXIT_FAILURE ) ;
      }
    // If we get a PID, parent is done.
    if ( pid > 0 ) exit ( EXIT_SUCCESS ) ;
    // Set where the PID file lives.
    snprintf ( pidfile, PATH_MAX, "%s/%s.pid", PIDPATH, dname ) ;
    if ( settings.verbose > 6 )
      {
      fprintf ( stderr, "PIDFILE:\n\t%s\n", pidfile ) ;
      }
    // Is there a PID file?
    if ( !stat ( pidfile, &st ) )
      {
      // Get the old PID.
      int pi ;
      char ps[256] ;
      pf = fopen ( pidfile, "r" ) ;
      fgets ( ps, 256, pf ) ;
      fclose ( pf ) ;
      pi = atoi ( ps ) ;
      // Is that PID still running?
      if ( !kill ( pi, 0 ) )
        {
        fprintf ( stderr, "%s is already running!\n", dname ) ;
        exit ( EXIT_FAILURE ) ;
        }
      // Zap old PID file.
      unlink ( pidfile ) ;
      }
    // Change the file mode mask to a sane default.
    umask ( 0 ) ;
    // Close standard files.
    close ( STDIN_FILENO ) ; close ( STDOUT_FILENO ) ; close ( STDERR_FILENO ) ;
    // Can be: LOG_EMERG<LOG_ALERT<LOG_CRIT<LOG_ERR<LOG_WARNING<LOG_NOTICE<LOG_INFO<LOG_DEBUG
    setlogmask ( LOG_UPTO ( LOG_DEBUG ) ) ;
    // Log to console if no syslog available, with PID, starting now, local log if defined.
    openlog ( dname, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1 ) ;
    // Create a new SID for the child process
    sid = ( settings.detach ) ? setsid() : 0 ;
    if ( sid < 0 )
      {
      syserr ( LOG_CRIT, "%s unable to setsid().", dname ) ;
      exit ( EXIT_FAILURE ) ;
      }
    // Change the current working directory
    if ( ( chdir ( "/" ) ) < 0 )
      {
      syserr ( LOG_CRIT, "%s unable to chdir(\"/\").", dname ) ;
      exit ( EXIT_FAILURE ) ;
      }
    // Make a note of our PID.
    pf = fopen ( pidfile, "w" ) ;
    if ( pf == NULL )
      {
      syserr ( LOG_CRIT, "Unable to access PID file: %s\n", pidfile ) ;
      exit ( EXIT_FAILURE ) ;
      }
    chmod ( pidfile, 0644 ) ;
    fprintf ( pf, "%i\n", getpid() ) ;
    fclose ( pf ) ;
    }
  // Off we go.
  syserr ( LOG_NOTICE, "%s started.", dname ) ;
  // Background setup.
  setup() ;
  // The meat of the program - loop ad infinitum.
  while ( runloop )
    {
    loop() ;
    sleep ( settings.interval ) ;
    }
  // Backgound cleanup.
  finish() ;
  // Tidy up and quit.
  syserr ( LOG_NOTICE, "%s exiting.", dname ) ;
  if ( settings.detach )
    {
    closelog() ;
    unlink ( pidfile ) ;
    }
  // Done with any config data.
  freesettings() ;
  // That's all, folks!
  return EXIT_SUCCESS ;
  }


// VIM formatting info.
// vim:ts=2:sw=2:tw=120:fo=tcnq2b:foldmethod=indent
