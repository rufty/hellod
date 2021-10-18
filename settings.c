/****************************************************************************************************************/
/*  Purpose:    Process command line and configuration files for settings.                                      */
/*  Author:     Copyright (c) 2018, W.B.Hill <mail@wbh.org> All rights reserved.                                */
/*  License:    Choose either BSD 2 clause or GPLv2 - see included file LICENSE.                                */
/*  Notes:      New arguments need to be added where tagged ADDARG, and in the header, too.                     */
/*              After processing, all extra argv[optind++], if any, should be files. This isn't checked.        */
/*              There are validate_*() for checking option ranges and sanity.                                   */
/*              Core settings are: help, version, quiet and verbose.                                            */
/*              Demo settings are: -D, interval and message.                                                    */
/****************************************************************************************************************/


// For asprintf()
#ifdef __linux__
#define _GNU_SOURCE
#endif //__linux__
// Standard infrastructure.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
// Required by strtol()
#include <limits.h>
// For basename()
#include <libgen.h>
// Options parsing.
#include <getopt.h>
// Config file reading.
#include <confuse.h>

// Compile-time defults.
#include "config.h"
// These settings.
#include "settings.h"
// Globals.
#include "hellod.h"


// Show version info.
static void
version()
  {
  printf ( "%s v%s, W.B.Hill <mail@wbh.org>, December 2018\n", STR(APPNAME), STR(VERSION) ) ;
  }


// Show terse info.
// ADDARG - Show existance and defaults.
static void
usage()
  {
  printf ( "Usage: %s", dname ) ;
  printf ( " -D" ) ;
  printf ( " -q" ) ;
  printf ( " -v%d", settings.verbose ) ;
  printf ( " -i%d", settings.interval ) ;
  printf ( " -m%s", settings.message ) ;
  printf ( "\n" ) ;
  }


// Show some help.
// ADDARG - Show descritions and defaults.
static void
help()
  {
  printf ( "Usage: %s [option] ...\n", dname ) ;
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
static void
easter()
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
static int
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


// Check we get somewhere 0 to 9.
static bool
valid_verbose ( int value )
  {
  return ( value >= 0 && value <= 9 ) ? true : false ;
  }
static int
validate_verbose ( cfg_t* cfg, cfg_opt_t* opt )
  {
  int value = cfg_opt_getnint ( opt, 0 ) ;
  if ( !valid_verbose ( value ) )
    {
    cfg_error ( cfg, "Verbose value out of range." ) ;
    return -1 ;
    }
  return 0 ;
  }


// Check we have something sane.
static bool
valid_text ( char* value )
  {
  return ( strlen ( value ) >=3 ) ? true : false ;
  }
static int
validate_text ( cfg_t* cfg, cfg_opt_t* opt )
  {
  char* value = cfg_opt_getnstr ( opt, 0 ) ;
  if ( !valid_text ( value ) )
    {
    cfg_error ( cfg, "Too short value for 'string'." ) ;
    return -1 ;
    }
  return 0 ;
  }


// Process the config files and the command line.
void
initsettings ( int argc, char* argv[] )
  {
  // What are we called?
  strncpy ( dname, basename ( argv[0] ), NAME_MAX ) ;
  // Defaults.
  // ADDARG flags default to false, anything else from config.h
  // ... that have no config equivalent.
  settings.detach = false ;
  // ... that have config settings.
  static cfg_opt_t opts[] =
    {
    CFG_INT ( "verbose", 1, CFGF_NONE ) ,
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
    {
    char* etcconf ;
    char* usrconf ;
    cfg_t* confuse ;
    // Generate the config file names from the app name.
    asprintf ( &etcconf, "/etc/%s.conf", dname  ) ;
    asprintf ( &usrconf, "%s/.%src", getenv ( "HOME" ), dname ) ;
    // Init the config library.
    confuse = cfg_init ( opts, CFGF_NONE ) ;
    // Check the values. ADDARG
    cfg_set_validate_func ( confuse, "verbose", validate_verbose ) ;
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
    // Debugging.
    if ( settings.verbose > 6 )
      {
      fprintf ( stderr,"CONFIG FILES:\n" ) ;
      fprintf ( stderr, "\tmain:      %s\n", etcconf ) ;
      fprintf ( stderr, "\tuser:      %s\n", usrconf ) ;
      }
    // Done - free stuff.
    cfg_free ( confuse ) ;
    free ( etcconf ) ;
    free ( usrconf ) ;
    }
  // Now parse the command line.
    {
    int opt = 0 ;
    int long_index = 0 ;
    // Process the command line ADDARG
    while ( ( opt = getopt_long ( argc, argv, "hVDqv::i:m:e", long_options, &long_index ) ) != -1 )
      {
      char* end ;
      switch ( opt )
        {
        case 0 :
          easter() ;
          exit ( EXIT_SUCCESS ) ;
        case 'h' :
          help() ;
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
            end = NULL ;
            settings.verbose = (int) strtol ( optarg, &end, 10 ) ;
            if ( optarg == end )
              {
              fprintf ( stderr, "Verbose value unparsable: %s\n", optarg ) ;
              exit ( EXIT_FAILURE ) ;
              }
            if ( !valid_verbose ( settings.verbose ) )
              {
              fprintf ( stderr, "Verbose value out of range: %s\n", optarg ) ;
              exit ( EXIT_FAILURE ) ;
              }
            }
          break ;
        case 'i' :
          {
          end = NULL ;
          settings.interval = (int) strtol ( optarg, &end, 10 ) ;
          if ( optarg == end )
            {
            fprintf ( stderr, "Problem with interval: %s\n", optarg ) ;
            exit ( EXIT_FAILURE ) ;
            }
          }
          if ( *end != '\0' ) fprintf ( stderr, "WARNING trailing chars on int ignored: %s\n", end ) ;
          break ;
        case 'm' :
          if( !valid_text ( optarg ) )
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
        default :
          usage() ;
          exit ( EXIT_FAILURE ) ;
        }
      }
    // Shouldn't be anything left.
    if ( optind != argc )
      {
      fprintf ( stderr, "Unexpected command line argument.\n" ) ;
      usage() ;
      exit ( EXIT_FAILURE ) ;
      }
    }
  // Show the settings.
  // ADDARG - any new flags/settings.
  if ( settings.verbose > 4 )
    {
    fprintf ( stderr,"SETTINGS:\n" ) ;
    fprintf ( stderr, "\tdetach:    %c\n", settings.detach ? 'T' : 'F' ) ;
    fprintf ( stderr, "\tverbose:   %d\n", settings.verbose ) ;
    fprintf ( stderr, "\tinterval:  %d\n", settings.interval ) ;
    fprintf ( stderr, "\tmessage:   %s\n", settings.message ) ;
    }
  // Shouldn't be any files.
  return ;
  }


// Tidy up when done.
// ADDARG - free strdup()'ed settings.
void
freesettings()
  {
  free ( settings.message ) ;
  }


// VIM formatting info.
// vim:ts=2:sw=2:tw=120:fo=tcnq2b:foldmethod=indent
