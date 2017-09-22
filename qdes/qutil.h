/********************************************************************
 *
 * Utility module
 *
 * Provides functions and macros to access parts of a Linux system
 * from C.  Functions such as file access and network status.  Also
 * includes a number of macros useful in development
 *
 * Protological
 * January 2017
 *
 *******************************************************************/
#ifndef __QDES_UTIL_H__
#define __QDES_UTIL_H__

/********************************************************************
 *
 * Constants
 *
 *******************************************************************/

/********************************************************************
 *
 * Macros for useful functions
 *
 *******************************************************************/

// Helper for static strings
#define cmp(B,S)    (strncmp((char*)(B),(S),sizeof(S)-1)==0)


#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
 *
 * General functions for information and system flow
 *
 *******************************************************************/

void u_rand_seed();

void u_rand_string(char * str, int len);

// Returns unix timestamp, seconds from epoch
uint32_t u_get_unixtime();

int u_run_cmd_withresult(char * cmd, char * resultbuf, int bufmax);

int u_run_cmd_retcode(char * cmd);

// Get the current user
void u_get_user(char * user,int max);

bool u_touch(char * path);

/********************************************************************
 *
 * General file access functions
 *
 *******************************************************************/

bool u_file_exists(char * file);

bool u_file_copy(char * file, char * filenew);

// Checks to see if the path is writeable
bool u_path_writeable(char * path);

bool u_path_exists(char * path);

/********************************************************************
 *
 * Network related functions
 *
 *******************************************************************/

int u_net_get_hostname(char * outbuf, int max);


#ifdef __cplusplus
}
#endif

#endif

//eof

