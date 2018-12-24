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

#if __linux
#define _GNU_SOURCE
#endif //__linux

// Standard infrastructure
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
// Required by strtol()
#include <limits.h>
// For basename()
#include <libgen.h>
// For kill()
#include <signal.h>
// For stat()
#include <sys/stat.h>
// Options parsing.
#include <getopt.h>
// Config file reading.
#include <confuse.h>

// Compile-time defults.
#include "config.h"
// This program.
#include "hellod.h"


// Configuration variables.
struct Settings settings ;

// What we're called.
char dname[NAME_MAX] ;
// Where to put the PID file.
char pidfile[PATH_MAX] ;


// Show terse info. ADDARG
void
usage ( char* appname )
  {
  printf ( "Usage: %s", appname ) ;
  printf ( " -D" ) ;
  printf ( " -q" ) ;
  printf ( " -v%d", settings.verbose ) ;
  printf ( " -i%d", settings.interval ) ;
  printf ( " -m%s", settings.message ) ;
  printf ( "\n" ) ;
  }


// Show version info.
void
version()
  {
  printf ( "%s v%s, W.B.Hill <mail@wbh.org>, December 2018\n", STR(APPNAME), STR(VERSION) ) ;
  }


// Show some help. ADDARG
void
help ( char* appname )
  {
  printf ( "Usage: %s [option] ...\n", appname ) ;
  printf ( "\t-h,--help      This help.\n" ) ;
  printf ( "\t-V,--version   Version information.\n" ) ;
  printf ( "\t-D             Run detached.\n" ) ;
  printf ( "\t-q,--quiet     Set verbose=0\n" ) ;
  printf ( "\t-v,--verbose   Verbosity level. Default, if set, %d\n", VERBOSITY ) ;
  printf ( "\t-i,--interval  Time to wait between loop()'s in seconds. Default %ds\n", INTERVAL ) ;
  printf ( "\t-m,--message   Message embedded in output. Default %s\n", dname ) ;
  printf ( "%s v%s, W.B.Hill <mail@wbh.org>, December 2018\n", STR(APPNAME), STR(VERSION) ) ;
  }


// An easter egg.
#include <time.h>
void
easter ( )
  {
  int X, y, Y, z, Z ;
  // Tempus.
  time_t curtime ;
  struct tm* loctime ;
  curtime = time ( NULL ) ; loctime =  localtime ( &curtime ) ;
  X = 1900 + loctime->tm_year ; Y = 1 + loctime->tm_mon ; Z = loctime->tm_mday ;
  // Computus.
  int a, b, c, d, e, f, g, h, i, j, k, l ;
  a = X % 19 ; b = X / 100 ; c = X % 100 ; d = b / 4 ; e = b % 4 ; f = ( b + 8 ) / 25 ;
  g = ( b - f + 1 ) / 3 ; h = ( 19 * a + b - d - g + 15 ) % 30 ; i = c / 4 ; j = c % 4 ;
  k = ( 32 + 2 * e + 2 * i - h - j ) % 7 ; l = ( a + 11 * h + 22 * k ) / 451 ;
  y = ( h + k - 7 * l + 114 ) / 31 ; z = 1 + ( h + k - 7 * l + 114 ) % 31 ;
  // Probatiat.
  if ( Y < y || ( Y == y && Z <= z ) ) { fprintf ( stderr, "%d%02d%02d\n", X, y, z ) ; exit ( EXIT_SUCCESS ) ; }
  // Iterum.
  a = ++X % 19 ; b = X / 100 ; c = X % 100 ; d = b / 4 ; e = b % 4 ; f = ( b + 8 ) / 25 ;
  g = ( b - f + 1 ) / 3 ; h = ( 19 * a + b - d - g + 15 ) % 30 ; i = c / 4 ; j = c % 4 ;
  k = ( 32 + 2 * e + 2 * i - h - j ) % 7 ; l = ( a + 11 * h + 22 * k ) / 451 ;
  y = ( h + k - 7 * l + 114 ) / 31 ; z = 1 + ( h + k - 7 * l + 114 ) % 31 ;
  fprintf ( stderr, "%d%02d%02d\n", X, y, z ) ; exit ( EXIT_SUCCESS ) ;
  }


// Check we get +ve nonzero.
int
validate_uint ( cfg_t* cfg, cfg_opt_t* opt )
  {
  int value = cfg_opt_getnint ( opt, 0 ) ;
  if ( value < 1 )
    {
    cfg_error ( cfg, "Invalid parameter number." ) ;
    return -1 ;
    }
  return 0 ;
  }


// Check we have something sane.
int
validate_text ( cfg_t* cfg, cfg_opt_t* opt )
  {
  char* value = cfg_opt_getnstr ( opt, 0 ) ;
  if ( strlen ( value ) < 3 )
    {
    cfg_error ( cfg, "Invalid parameter text." ) ;
    return -1 ;
    }
  return 0 ;
  }


// Utility for redirecting messages.
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


// Show the settings.
#ifdef DEBUG
void
dump_settings ( char* msg )
  {
  fprintf ( stderr,"%s:\n", msg ) ;
  fprintf ( stderr, "\tdetach:    %c\n", settings.detach ? 'T' : 'F' ) ;
  fprintf ( stderr, "\tverbose:   %d\n", settings.verbose ) ;
  fprintf ( stderr, "\tinterval:  %d\n", settings.interval ) ;
  fprintf ( stderr, "\tmessage:   %s\n", settings.message ) ;
  }
#endif //DEBUG


// Parse the config files, then the command line, setup and then finally do stuff.
int
main ( int argc, char* argv[] )
  {
  // What are we called?
  strncpy ( dname, basename ( argv[0] ), NAME_MAX ) ;
  // Set where the PID file lives.
  snprintf ( pidfile, PATH_MAX, "%s/%s.pid", PIDPATH, dname ) ;
  // Defaults that have no config equivalent. ADDARG
  settings.detach = false ;
  // Defaults that have config settings. ADDARG
  static cfg_opt_t opts[] =
    {
    CFG_INT ( "verbose", 0, CFGF_NONE ) ,
    CFG_INT ( "interval", INTERVAL, CFGF_NONE ),
    CFG_STR ( "message", STR(APPNAME), CFGF_NONE ),
    CFG_END()
    } ;
  // Command line options. ADDARG
  static struct option long_options[] =
    {
      { "help",      no_argument,       0,  'h' },
      { "version",   no_argument,       0,  'V' },
      { "detach",    no_argument,       0,  'D' },
      { "quiet",     no_argument,       0,  'q' },
      { "verbose",   optional_argument, 0,  'v' },
      { "interval",  required_argument, 0,  'i' },
      { "message",   required_argument, 0,  'm' },
      { "easter",    no_argument,       0,  'e' },
      { 0, 0, 0, 0 }
    } ;
  // Load the config files.
  char* etcconf ;
  char* usrconf ;
  cfg_t* confuse ;
  // Generate the config file names from the app name.
  asprintf ( &etcconf, "/etc/%s.conf", dname  ) ;
  asprintf ( &usrconf, "%s/.%src", getenv ( "HOME" ), dname ) ;
  #ifdef DEBUG // Show config filenames.
  fprintf ( stderr,"CONFIG FILES:\n" ) ;
  fprintf ( stderr, "\tmain:      %s\n", etcconf ) ;
  fprintf ( stderr, "\tuser:      %s\n", usrconf ) ;
  #endif //DEBUG
  // Init the config library.
  confuse = cfg_init ( opts, CFGF_NONE ) ;
  // Check the values. ADDARG
  cfg_set_validate_func ( confuse, "verbose", validate_uint ) ;
  cfg_set_validate_func ( confuse, "interval", validate_uint ) ;
  cfg_set_validate_func ( confuse, "message", validate_text ) ;
  // Read the /etc/app.conf file.
  if ( cfg_parse ( confuse, etcconf ) == CFG_PARSE_ERROR )
    {
    fprintf ( stderr, "Problem with config file '%s'\n", etcconf ) ;
    exit ( EXIT_FAILURE ) ;
    }
  // Read the ~/.apprc file.
  if ( cfg_parse ( confuse, usrconf ) == CFG_PARSE_ERROR )
    {
    fprintf ( stderr, "Problem with config file '%s'\n", usrconf ) ;
    exit ( EXIT_FAILURE ) ;
    }
  // Save the values. ADDARG
  settings.verbose = cfg_getint ( confuse, "verbose" ) ;
  settings.interval = cfg_getint ( confuse, "interval" ) ;
  settings.message = strdup ( cfg_getstr ( confuse, "message" ) ) ;
  // Done - free stuff.
  cfg_free ( confuse ) ;
  free ( etcconf ) ;
  free ( usrconf ) ;
  // Now parse the command line.
  int opt = 0 ;
  int long_index = 0 ;
  // Process the command line ADDARG
  while ( ( opt = getopt_long ( argc, argv, "hVDqv::i:m:e", long_options, &long_index ) ) != -1 )
    {
    switch ( opt )
      {
      case 'h' :
        help ( dname ) ;
        exit ( EXIT_SUCCESS ) ;
      case 'V' :
        version() ;
        exit ( EXIT_SUCCESS ) ;
      case 'D' :
        settings.detach = true ;
        break ;
      case 'q' :
        settings.verbose = 0 ;
        break ;
      case 'v' :
        if ( !optarg )
          {
          settings.verbose = VERBOSITY ;
          }
        else
          {
          settings.verbose = (int) strtol ( optarg, (char **)NULL, 10 ) ;
          if ( settings.verbose < 0 || settings.verbose > 9 )
            {
            fprintf ( stderr, "Problem with verbose: %s\n", optarg ) ;
            exit ( EXIT_FAILURE ) ;
            }
          }
        break ;
      case 'i' :
        settings.interval = (int) strtol ( optarg, (char **)NULL, 10 ) ;
        if ( settings.interval < 1 )
          {
          fprintf ( stderr, "Problem with interval: %s\n", optarg ) ;
          exit ( EXIT_FAILURE ) ;
          }
        break ;
      case 'm' :
        if( strlen ( optarg ) < 3 )
          {
          fprintf ( stderr, "Problem with message: %s\n", optarg ) ;
          exit ( EXIT_FAILURE ) ;
          }
        else
          {
          free ( settings.message ) ;
          settings.message = strdup ( optarg ) ;
          }
        break ;
      case 'e' :
        easter() ;
        break ;
      default :
        usage ( dname ) ;
        exit ( EXIT_FAILURE ) ;
      }
    }
  // Shouldn't be anything left.
  if ( optind != argc )
    {
    fprintf ( stderr, "Unexpected command line argument.\n" ) ;
    usage ( dname ) ;
    exit ( EXIT_FAILURE ) ;
    }
  #ifdef DEBUG
  fprintf ( stderr, "PIDFILE:\n\t%s\n", pidfile ) ;
  dump_settings ( "SETTINGS" ) ;
  #endif //DEBUG
  // Launch the background process loop and quit.
  struct stat st ;
  pid_t pid ;
  pid_t sid ;
  FILE* pf ;
  // Basic setup.
  begin() ;
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
  // Fork off the background process.
  pid = ( settings.detach ) ? fork() : 0 ;
  // A-OK?
  if ( pid < 0 )
    {
    fprintf ( stderr, "%s is unable to start background process.\n", dname ) ;
    exit ( EXIT_FAILURE ) ;
    }
  // If we get a PID, parent is done.
  if ( pid > 0 ) exit ( EXIT_SUCCESS ) ;
  // Now we're in the background. Maybe.
  if ( settings.detach )
    {
    // Change the file mode mask to a sane default.
    umask ( 0 ) ;
    // Close standard files.
    close ( STDIN_FILENO ) ;
    close ( STDOUT_FILENO ) ;
    close ( STDERR_FILENO ) ;
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
  fprintf ( pf, "%i\n", getpid() ) ;
  fclose ( pf ) ;
  // Register to get rid of PID file when done.
  signal ( SIGINT, signal_handler ) ;
  signal ( SIGTERM, signal_handler ) ;
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
  // Tidy up and quit.
  finish() ;
  syserr ( LOG_NOTICE, "%s exiting.", dname ) ;
  closelog() ;
  unlink ( pidfile ) ;
  // Done with any config data. ADDARG
  free ( settings.message ) ;
  // That's all, folks!
  return EXIT_SUCCESS ;
  }


// VIM formatting info.
// vim:ts=2:sw=2:tw=114:fo=tcnq2b:foldmethod=indent
