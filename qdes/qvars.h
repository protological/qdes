/********************************************************************************************
*
* Script Library - Simple C scripting library for updating embedded systems
*
* Variable processing source file. Store name,value strings for 'variables'
*
* Protological
* January 2017
*
********************************************************************************************/
#ifndef __QDES_VARS_H__
#define __QDES_VARS_H__


#ifdef __cplusplus
extern "C" {
#endif

//
// Itterate over the variable list and see if we have an
// entry for 'var', return in outvar
//
bool var_find(char * var,char * outvar);

//
// Scan the 'in' string and replace all '$xxxxx' values with the
// variable stored in the var list.  If we don't have it just leave
// the original
//
int var_replace(char * in,char * out,int outmax);

//
// Add a variable to the vars list
//
void var_add(char * name, char * value);

//
// Itterate over the list and free all variables
//
void var_clearall();

//
// Print the variable list
//
void var_printall();

//
// Check to see if a variable matches the passed value
//
bool var_check(char * name, char * value);


#ifdef __cplusplus
}
#endif

#endif

// eof
