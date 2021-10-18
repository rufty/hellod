#ifndef SETTINGS_H
#define SETTINGS_H


#include <stdint.h>
#include <stdbool.h>


// The settings. ADDARG
typedef struct
  {
  // Pre-defined settings.
  int verbose ;
  // Test settings.
  bool flag ;
  #define FLAGSETTING false
  int intsetting ;
  #define INTSETTING 42
  bool boolsetting ;
  #define BOOLSETTING false
  float realsetting ;
  #define REALSETTING 1.414
  char* stringsetting ;
  #define STRINGSETTING "Hello, World!!!"
  } Settings ;


// Settings helpers.
void initsettings ( int argc, char* argv[] ) ;
void freesettings() ;


#endif //SETTINGS_H
// VIM formatting info.
// vim:ts=2:sw=2:tw=120:fo=tcnq2b:foldmethod=indent
