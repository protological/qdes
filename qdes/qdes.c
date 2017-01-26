/********************************************************************************************
*
* Script Library - Simple C scripting library for updating embedded systems
*
* Protological
* January 2017
*
********************************************************************************************/
#include "defs.h"

#include <string.h>
#include <regex.h>
#include <stdarg.h>

#include "qutil.h"
#include "qdes.h"
#include "qvars.h"

// Definitions and types
// ---------------------------------------------
typedef bool (*cmd_ptr_t)(char * cmd, char * arg1,char * arg2, command_e command);
typedef struct{
    char * command;
    cmd_ptr_t fptr;
}commands_t;

#define SCRIPT_LOG              "script.log"

// Local prototypes
// ---------------------------------------------
char * _command_str(command_e command);
int _cmd_run(char * cmd, command_e command);
bool _file_copy(char * archdir, char *arg1,char * arg2, command_e command);
bool _file_touch(char * file, command_e command);

bool _command_general(char * cmd, char * arg1, char * arg2, command_e command);
bool _command_check(char * cmd, char * arg1, char * arg2, command_e command);

void _log(char* fmt, ...);

// Variables
// ---------------------------------------------
commands_t m_commands[] = {
        {"var",         _command_general},
        {"check",       _command_check},
        {"cmd",         _command_general},
        {"copy",        _command_general},
        {"touch",       _command_general},
        {"exists",      _command_general},
        {"echo",        _command_general},
        {"writeable",   _command_general},
        //{"chmod",       _command_makedir}, (Args: file,mode)
        //{"makedir",     _command_makedir}, (Args: path)
        //{"delete",      _command_delete}, (Args: file)
        //{"write",       _command_write}, (Args: file,string)
        //{"md5check",    _command_md5check}, (Args: file,md5string)
        //{"md5get",      _command_md5get}, (Args: file,variable)
        //{"list",        _command_listfiles}, (Args: path)

        {"test",        _command_general},
        {NULL,NULL},// Must be last
};

char m_path[100];
FILE * m_log_fp=NULL;

// Public functions
// ---------------------------------------------

//
// Itterate over the script file and make sure
// we understand all of the commands
//
bool script_checkfile(char * file)
{
    FILE * fp;
    char line[100];
    char arg1[50],arg2[50];
    int len = 0;
    regex_t entry;
    int reti,cmd;
    char * c;
    bool passed=true;
    char * pch;
        
    if(regcomp(&entry, "^[[:blank:]]*{", 0)) return false;
    
    // Try and open the file
    fp = fopen(file, "r");
    if (fp == NULL){
        _log("Can't open file %s\n",file);
        return false;
    }

    // Itterate over all the lines
    while(fgets(line, sizeof(line), fp) != NULL)
    {
        len = strlen(line);
        //_log("Retrieved line of length %d :\n", len);
        line[len-1]=0;
        // Execute regular expression, check for {....}
        reti = regexec(&entry, line, 0, NULL, 0);

        // If we got a match!
        if (!reti)
        {
            // Get pch to the command
            c = strchr(line,'{');
            pch = strtok(c,"{, \t}");

            // Check this pch against the known commands
            cmd=0;
            passed=false;
            while(m_commands[cmd].command)
            {
                //_log("Check '%s' ?= '%s'\n",m_commands[cmd].command,pch);
                if(strcmp(m_commands[cmd].command,pch)==0)
                {
                    passed=true;
                    break;
                }
                cmd++;
            }

            // If we failed, break out of reading the file
            if(passed==false){
                printf("Error processing command '%s'\n",pch);
                break;
            }

        }else if (reti == REG_NOMATCH) {
            //_log("Not entry: '%s'\n",line);
        }else{
            //regerror(reti, &entry, msgbuf, sizeof(msgbuf));
            if(g_verbose) _log("Regex match failed:\n");
            passed=false;
            break;
        }
    } // end while(line)
        
    fclose(fp);

    // Free memory allocated to the pattern buffer by regcomp()
    regfree(&entry);
    
    // Return if we passed
    return passed;
}

//
// Process a section of the script
//
bool script_run(char * path, char * file, char * section, command_e command)
{
    FILE * fp;
    char line[100];
    char cmd[20];
    char arg1[50],arg2[50];
    int len = 0;
    regex_t entry;
    regex_t sec;
    int reti,cmdid;
    char * c;
    bool passed=true;
    bool foundsection=false;

    // The beginning of the init section
    if(cmp(section,"init"))
    {
        // Save this path (Do first because of log)
        strncpy(m_path,path,sizeof(m_path));

        // Set internal variables
        _log("Setting internal variables\n");
        var_add("package_dir",path);
        memset(line,0,10);
        u_get_user(line,10);
        var_add("user",line);

        // Tell where we are looking
        sprintf(line,"%s/%s",path,file);
        _log("Running script from %s\n",line);

        _log("Command is: %s\n", _command_str(command));
    }

    _log("*** Run section %s ***\n",section);

    // Make the regex to find entries
    if(regcomp(&entry, "^[[:blank:]]*{", 0)){
        _log("Fatal error in regcomp\n");
         return false;
    }
    // Make the regex to find section
    if(regcomp(&sec, ":[[:blank:]]*$", 0)){
        _log("Fatal error in regcomp\n");
         return false;
    }
    
    // Try and open the file
    sprintf(line,"%s/%s",path,file);
    fp = fopen(line, "r");
    if (fp == NULL){
        _log("Can't open file %s\n",line);
        return false;
    }

    // Itterate over each line of the file
    while(fgets(line, sizeof(line), fp) != NULL)
    {
        len = strlen(line);
        line[len-1]=0;
        //_log("Retrieved line: '%s'\n", line);

        // Execute 'section' regular expression
        reti = regexec(&sec, line, 0, NULL, 0);
        if (!reti) {
            if(strncmp(section,line,strlen(section))==0)
            {
                //_log("Found section '%s'\n",line);
                foundsection=true;
            }else{
                foundsection=false;
                passed=true; // this isn't our section, so we can't fail
            }
        }else if (reti == REG_NOMATCH) {
            //_log("Not entry: '%s'\n",line);
        }else {
            //regerror(reti, &entry, msgbuf, sizeof(msgbuf));
            if(g_verbose) _log("Regex match failed:\n");
            passed=false;
            break;
        }

        // Execute 'entry' regular expression
        reti = regexec(&entry, line, 0, NULL, 0);
        if (!reti && foundsection)
        {
            char * pch;

            // If we are here, we got an entry for our section, run it!

            //_log("Retrieved entry: '%s'\n", line);

            // Get the cmd
            c = strchr(line,'{');
            pch = strtok(c,"{, \t}");
            strncpy(cmd,pch,sizeof(cmd));

            // Get arg1
            pch = strtok(NULL,"{,\t}");
            while(pch && *pch==' ') pch++;  //<-- increment the pch until not [SPACE]
            if(!pch){ _log("Errr in entry arg1: '%s'\n",line); continue; }
            strncpy(arg1,pch,sizeof(arg1));

            // Get arg2
            pch = strtok(NULL,"{,\t}");
            while(pch && *pch==' ') pch++;  //<-- increment the pch until not [SPACE]
            if(pch){ strncpy(arg2,pch,sizeof(arg2)); }else{ arg2[0]=0; }

            // Loop over the commands and see if we have a match
            cmdid=0;
            passed=false;
            while(m_commands[cmdid].command)
            {
                if(strcmp(m_commands[cmdid].command,cmd)==0)
                {
                    if(m_commands[cmdid].fptr)
                        passed = (m_commands[cmdid].fptr)(cmd,arg1,arg2,command);
                    break;
                }
                cmdid++;
            } // end loop over cmds

            // if we failed something, break out
            if(passed==false) break;
        }else if (reti == REG_NOMATCH) {
            //_log("Not entry: '%s'\n",line);
        }else if(foundsection==false) {
            // keep going
        }else {
            //regerror(reti, &entry, msgbuf, sizeof(msgbuf));
            if(g_verbose) _log("Regex match failed:\n");
            passed=false;
            break;
        }
    } // end while(line)
        
    fclose(fp);

    // Free memory allocated to the pattern buffer by regcomp()
    regfree(&entry);
    regfree(&sec);

    return passed;
}

//
// Reset the script processor, dump all vars
//
void script_clear()
{
    if(m_log_fp) fclose(m_log_fp);
    memset(m_path,0,sizeof(m_path));
    var_clearall();
    return;
}

// Priviate functions
// ---------------------------------------------

char * _command_str(command_e command)
{
    switch(command){
    case COMMAND_RUN: return "COMMAND_RUN";
    case COMMAND_TEST: return "COMMAND_TEST";
    default: break;
    }
    return "COMMAND_UNKNOWN";
}
int _cmd_run(char * cmd, command_e command)
{
    char cmdout[100]= {0};
    int replaced;
    int ret =0;
    replaced = var_replace(cmd,cmdout,sizeof(cmdout));
    _log("CMDRUN: '%s'\n",cmdout);

    if(command != COMMAND_TEST) ret = u_run_cmd_retcode(cmdout);
    return ret;
}
bool _file_copy(char * archdir, char *arg1,char * arg2, command_e command)
{
    char src[100] = {0};
    char file[100] = {0};
    char dest[100]= {0};
    int replaced;
    bool ret=true;

    replaced = var_replace(arg1,file,sizeof(file));
    replaced = var_replace(arg2,dest,sizeof(dest));
    if(file[0]!='/')
        sprintf(src,"%s/%s",archdir,file);
    else
        sprintf(src,"%s",file);
    _log("FILECPY: %s' -> '%s'\n",src,dest);
    if(command != COMMAND_TEST) ret = u_file_copy(src,dest);
    return ret;
}
bool _file_touch(char * file, command_e command)
{
    char path[100];
    int replaced;
    bool passed=true;
    replaced = var_replace(file,path,sizeof(path));
    _log("TOUCH: '%s'\n",path);
    if(command != COMMAND_TEST) passed=u_touch(path);
    return passed;
}

bool _command_general(char * cmd, char * arg1, char * arg2, command_e command)
{
    char arg1_replaced[100]= {0};
    bool passed = true;

    //_log("_command_gereral func '%s'\n",cmd);

    if(cmp(cmd,"var")){
        // We are adding a variable to the module
        var_add(arg1,arg2);
    }else if(cmp(cmd,"cmd")){
        int ret;
        // Run this command
        ret=_cmd_run(arg1,command);
        if(ret!=0){
            _log("Error running command, ret code %d\n",ret);
            passed=false;
        }
    }else if(cmp(cmd,"copy")){
        // Copy a file to a location
        if(!_file_copy(m_path,arg1,arg2,command))
        {
            _log("Can't copy file!\n");
            passed=false;
        }
    }else if(cmp(cmd,"touch")){
        // Touch a file
        if(!_file_touch(arg1,command)){
            _log("Can't touch file!\n");
            passed=false;
        }
    }else if(cmp(cmd,"exists")){
        char str[100];
        int replaced;
        replaced = var_replace(arg1,str,sizeof(str));
        // Make sure a file/folder exists
        if(!u_file_exists(str) && !u_path_exists(str)){
            _log("Error: exists test failed, '%s'\n",str);
            passed = false;
        }else{
            _log("EXISTS: Passed '%s'\n",str);
        }
    }else if(cmp(cmd,"echo")){
        char str[100];
        int replaced;
        replaced = var_replace(arg1,str,sizeof(str));
        _log("---- %s ----'\n",str);
    }else if(cmp(cmd,"writeable")){
        var_replace(arg1,arg1_replaced,sizeof(arg1_replaced));
        _log("Checking writeable of '%s'\n",arg1_replaced);
        passed = u_path_writeable(arg1_replaced);
        if(!passed)
            _log("'%s' is not writeable, fail\n",arg1_replaced);
    }else if(cmp(cmd,"test")){
        // something?
    }

    return passed;
}
bool _command_check(char * cmd, char * arg1, char * arg2, command_e command)
{
    // Check a variable against this value
    bool ret;
    ret = var_check(arg1,arg2);
    if(!ret){
        _log("ERROR! Check failed, aborting\n");
       return false;
    }
    return true;
}

char m_scratch[200];
void _log(char* fmt, ...)
{
    va_list args;

    if(m_log_fp==NULL)
    {
        char logpath[100];
        sprintf(logpath,"%s/%s",m_path,SCRIPT_LOG);
        //printf("Opening %s\n",logpath);
        m_log_fp = fopen(logpath,"w");
        if(m_log_fp)
            var_add("log",logpath);
    }

    va_start(args,fmt);
    //vsyslog(level, fmt,args);
    vsprintf(m_scratch,fmt,args);
    if(m_log_fp){
        fprintf(m_log_fp,"%s",m_scratch);
    }
    printf("%s",m_scratch);
    va_end(args);
    return;
}


// eof
