/********************************************************************************************
*
* Script Library - Simple C scripting library for updating embedded systems
*
* Protological
* January 2017
*
* Rules
*     Sections end with ':'. Example "main:" nothing after ':'
*     Entries are wrapped in { and } characters, nothing before or after
*     Entries format: {command, arg1, arg2}
*     Everything else is a comment
*     variables names can contain a-z,A-Z,0-9 and _ (underscore)
*
* Sections
*     MUST have init section first
*
* Commands
*      var,name,value     Set variable 'name'='value'
*      check,name,value   Check if variable 'name'=='value'
*      cmd,command        Run system(command)
*      copy,src,dest      Copy 'src' file to 'dest'
*      touch,file         Run command 'touch file'
*      exists,file        Make sure 'file' (or path) exists
*      echo,string        Print 'string' in output
*      writeable,path     Checks to see if 'path' is writeable
*
* Internal variables
*     $version - The version string for the library
*     $package_dir - The tmp location of where the pkz was decompressed
*     $timestamp - The current unix timestamp
*     $user - The current user running the updater
*     $log - The full path to the log file
*     $hostname - The hostname of the system we are running on
*
* Maximum lengths
*         args - 50 chars
*         variable names - 50 chars
*         variable values - 50 chars
*
********************************************************************************************/
#ifndef __QDES_H__
#define __QDES_H__

//#define DEFAULT_SCRIPT_FILE     "install.dat"

#define SCRIPT_VERSION          "1.2"

typedef enum{
    COMMAND_RUN=0,
    COMMAND_TEST,
}command_e;

// Return true for pass, false for fail
typedef bool (*cmd_ptr_t)(char * cmd, char * arg1,char * arg2, command_e command);

#ifdef __cplusplus
extern "C" {
#endif
//
// Register a function to be the callback when we have a message
// that is not included in qdes
//
void script_cmdhandler(cmd_ptr_t cb);

//
// Itterate over the script file and make sure
// we understand all of the commands
//
bool script_checkfile(char * file);

//
// Process a section of the script
//
bool script_run(char * path, char * file, char * section, command_e command);

//
// Reset the script processor, free all vars, close log, etc
//
void script_clear();



#ifdef __cplusplus
}
#endif

#endif

// eof
