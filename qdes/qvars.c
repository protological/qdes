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
#include "defs.h"

#include <string.h>

#include "qutil.h"
#include "qvars.h"
#include "qdes.h"

void _log(char* fmt, ...);

typedef struct{
    char * name;
    char * value;
}entry_t;
entry_t **m_vars=NULL;
int m_vars_count=0;

//
// Itterate over the variable list and see if we have an
// entry for 'var', return in outvar
//
bool var_find(char * var,char * outvar)
{
    bool ret = false;
    int x;
    
    // Check all dynamic variables
    //_log("\tSearch for '%s'\n",var);
    for(x=0;x<m_vars_count;x++)
    {
        if(m_vars[x])
        {
            if(m_vars[x]->name)
            {
                if(strcmp(m_vars[x]->name,var)==0)
                {
                    //_log("Match, %s == %s\n",m_vars[x]->name,var);
                    sprintf(outvar,"%s",m_vars[x]->value);
                    return true;
                }
            }
        }
    }
    
    // Check constants
    if(cmp(var,"version")){
        sprintf(outvar,"%s",SCRIPT_VERSION);
        ret = true;
    }else if(cmp(var,"hostname")){
        u_net_get_hostname(outvar,50);
        ret = true;
    }else if(cmp(var,"timestamp")){
        sprintf(outvar,"%d",u_get_unixtime());
        ret = true;
    }else{
        *outvar=0;
    }
    
    return ret;
}

//
// Scan the 'in' string and replace all '$xxxxx' values with the
// variable stored in the var list.  If we don't have it just leave
// the original
//
int var_replace(char * in,char * out,int outmax)
{
    int count=0;
    char var[50], outvar[50];
    char * c = in;
    char * start = in;
    char * last = in;
    
    *out=0;

    c = strchr(in,'$');
    while(c){
        
        // Find start and end of the variable
        //_log("----\nFound at '%s'\n",c);
        strncat(out,last,(c-last));
        c++;
        start = c;
        while( (*c>='a'&& *c<='z') || (*c>='A'&& *c<='Z') || (*c>='0'&& *c<='9') || *c=='_') c++;
        //_log("Ends at '%s'\n",c);
        
        // Pull the variable text out
        strncpy(var,start,(c-start));
        var[(c-start)]=0;
        
        // Search for it
        if(!var_find(var,outvar))
            sprintf(outvar,"$%s",var);
            
        // Place the result in the output
        strcat(out,outvar);
        
        //_log("str is now '%s'\n",out);

        // Increment the count
        count++;
        
        // See if we have more variables
        last = c;
        c = strchr(c,'$');
    }
    if(*last!=0)
        strcat(out,last);
    return count;
}

//
// Add a variable to the vars list
//
void var_add(char * name, char * value)
{
    entry_t ** list;
    entry_t * item;
    char * ptr;
    int items;
    int index;
    int len;
    _log("VARADD: '%s' = '%s'\n",name,value);
    
    // We are talking to the Xth entry
    index = m_vars_count;
    
    // Make the list bigger
    items = m_vars_count+1;
    list = (entry_t**)realloc_t(m_vars,(sizeof(entry_t*)*items));
    if(!list){
        _log("Can't allocate the list of size %d\n",items);
        return;
    }
    m_vars = list;
    m_vars_count++;
    //_log("list: x%08X\n",(int)list);
    //_log("Now have space for %d vars\n",m_vars_count);
    
    // Now, create an entry for this var
    item = (entry_t*)malloc_t(sizeof(entry_t));
    if(!item){
        _log("Error allocating entry_t\n");
        m_vars[index] = NULL;
        return;
    }
    //_log("Entry: x%08X\n",(int)item);
    m_vars[index] = item;

    // Now, create the buffer for name
    len = strlen(name);
    ptr = (char*)malloc_t(len+1);
    if(!ptr){
        _log("Can't malloc the name string\n");
        m_vars[index]->name = NULL;
        return;
    }
    //_log("Name: x%08X\n",(int)name);
    m_vars[index]->name = ptr;
    sprintf( (m_vars[index]->name),"%s",name);
    
    // Now, create the buffer for value
    len = strlen(value);
    ptr = (char*)malloc_t(len+1);
    if(!ptr){
        _log("Can't malloc the value string\n");
        m_vars[index]->value = NULL;
        return;
    }
    //_log("Value: x%08X\n",(int)value);
    m_vars[index]->value = ptr;
    sprintf( (m_vars[index]->value),"%s",value);

    //_var_printall();
    return;
}

//
// Itterate over the list and free all variables
//
void var_clearall()
{
    int x;
    for(x=0;x<m_vars_count;x++)
    {
        if(m_vars[x])
        {
            if(m_vars[x]->name) free_t(m_vars[x]->name);
            if(m_vars[x]->name) free_t(m_vars[x]->value);
            free_t(m_vars[x]);
        }
    }
    free_t(m_vars);
    m_vars=NULL;
    m_vars_count=0;
}

//
// Print the variable list
//
void var_printall()
{
    int x;
    _log("List at x%08X\n",(int)(m_vars));
    for(x=0;x<m_vars_count;x++)
    {
        _log("Entry %d: x%08X\n",x,(int)(m_vars[x]));
        if(m_vars[x])
        {
            _log("  Name x%08X: '%s'\n",(int)(m_vars[x]->name),m_vars[x]->name);
            _log("  Value x%08X:'%s'\n",(int)(m_vars[x]->value),m_vars[x]->value);
        }
    }
    return;
}

//
// Check to see if a variable matches the passed value
//
bool var_check(char * name, char * value)
{
    char getvalue[50];
    
    _log("VARCHECK: '%s' ?= '%s'\n",name,value);
    
    if(!var_find(name,getvalue)){
        _log("\tCheck failed, variable '%s' not found\n",name);
        return false;
    }else{
        //found the variable!
        if(strncmp(value,getvalue,sizeof(getvalue))==0)
        {
            // MAtch!
            _log("\tCheck passed, variable '%s', '%s'=='%s'\n",name,value,getvalue);
            return true;
        }
        _log("\tCheck failed, variable %s, '%s'!='%s'\n",name,value,getvalue);
    }
    return false;
}

// eof
