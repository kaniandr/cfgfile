//===--- cfgfile.h ----------- Configuration File -----------------*- C -*-===//
//
// This file implements a simple configuration file parser. A configuration
// file has the follwing structure:
//
// # Comments can be placed after '#' character.
//
// VARIABLE_1     VALUE_1                    # VARIABLE_1='VALUE_1'
// VARIABLE_2 :   VALUE_2                    # VARIABLE_2='   VALUE_2'
// VARIABLE_3 :                              # VARIABLE_3=''
// VARIABLE_4     1
// VARIABLE_5     ${VARIABLE_${VARIABLE_4}}  # VARIABLE_5='VALUE_1'
//
//===----------------------------------------------------------------------===//

#ifndef CFG_FILE_H
#define CFG_FILE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFGFILE {
  char *data;
  int numberOfStrings;
  int sizeOfString;
} CFGFILE;

int cfgopen(const char *filename, CFGFILE *fd);
int cfgclose(CFGFILE *fd);

/// Find an integer value with a specified key,
/// store result in a specified variable.
///
/// Return 0 on success.
int cfgfindi(CFGFILE *fd, const char *id, int *value);

/// Find an floating point value with a specified key,
/// store result in a specified variable.
///
/// Return 0 on success.
int cfgfindf(CFGFILE *fd, const char *id, double *value);

/// Find a raw representation of a value with a specified key,
/// store pointer to the raw representation in a specified variable.
///
/// Do not change a resulting value.
/// Return 0 on success.
int cfgfinds(CFGFILE *fd, const char *id, char **value);

/// Substitute all variables in a value with a specified key.
///
/// A specified buffer *value must be a reallocatable object.
/// This function allocate buffer for the result value, or
/// reallocate it if *value is not NULL.
/// Free *value if it is not used anymore.
///
/// Return 0 on success, do not update *value on error.
int cfgevals(CFGFILE *fd, const char *id, char **value);

#ifdef __cplusplus
}
#endif

#endif//CFG_FILE_H
