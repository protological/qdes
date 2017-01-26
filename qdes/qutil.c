/********************************************************************************************
*
* Utility function source file
*
* Protological
* January 2017
*
********************************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#include "qutil.h"

/********************************************************************
 *
 * General functions for information and system flow
 *
 *******************************************************************/

void u_rand_seed()
{
	//printf("RAND_MAX: %d\n",RAND_MAX);
	srand(time(NULL));
	return;
}

// From:
//http://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
void u_rand_string(char * str, int len)
{
	int i;
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    for (i = 0; i < len; ++i) {
        str[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    str[len] = 0;
    return;
}

uint32_t u_get_unixtime()
{
    return (unsigned)time(NULL);
}

int u_run_cmd_withresult(char * cmd, char * resultbuf, int bufmax)
{
    FILE *fp;
    int len;
    int total=0;
    char c;
    /* Open the command for reading. */
    fp = popen(cmd, "r");
    if (fp == NULL) {
        printf("Failed to run cmd '%s'\n",cmd);
        resultbuf = NULL;
        return 0;
    }
    // Handle NULL resultbuf
    if(resultbuf){
        while (fgets(resultbuf, bufmax-total, fp) != NULL)
        {
                total+=strlen(resultbuf);
        }
        // Stomp on newline
        if(total)
            resultbuf[total-1]=0;
        else
            resultbuf[0]=0;
    }else{
        while(fgetc(fp)!=EOF) total++;
    }
    pclose(fp);
    return total;
}

int u_run_cmd_retcode(char * cmd)
{
    int a;
    //printf("Running command: %s to get retcode\n",cmd);
    a = system(cmd);
    if(a == -1){
        //printf("Retcode value: %d\n",a);
        return -1;
    }
    //printf("Retcode value: %d\n",WEXITSTATUS(a));
    return WEXITSTATUS(a);
}

void u_get_user(char * user,int max)
{
    u_run_cmd_withresult("echo $USER", user, max);
    return;
}

bool u_touch(char * path)
{
    char command[100];
    int ret;
    sprintf(command,"touch %s",path);
    return (u_run_cmd_retcode(command)==0);
}

/********************************************************************
 *
 * General file access functions
 *
 *******************************************************************/

bool u_file_exists(char * file)
{
    return ( access( file, F_OK ) != -1 );
}

bool u_file_copy(char * file, char * filenew)
{
    char command[100];
    int ret =0;
    sprintf(command,"cp %s %s",file,filenew);
    ret = u_run_cmd_retcode(command);
    //printf("Running copy: '%s', ret %d\n",command,ret);
    return (ret==0);
}

bool u_path_writeable(char * path)
{
#if 0
    struct stat sb;
    return ((stat(path, &sb)==0) && (sb.st_mode&S_IWUSR));
#else
    int result = access(path, W_OK);
    //printf("is %s writeable, ret: %d\n",path, result);
    return (result == 0);
#endif
}
bool u_path_exists(char * path)
{
    struct stat sb;
    return (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode));
}

/********************************************************************
 *
 * Network related functions
 *
 *******************************************************************/

int u_net_get_hostname(char * outbuf, int max)
{
    return u_run_cmd_withresult("uname -n",outbuf,max);
}


// eof

