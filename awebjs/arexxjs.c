
/* arexxjs.c - AWeb js arexx suport */
/* experimental support for sending commands to arexx ports */

//#include "platform_specific.h"

#undef __USE_INLINE__

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <stdio.h>
#include <string.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/locale.h>
#include <proto/rexxsyslib.h>
#include <rexx/errors.h>
 
//#include "jslib.h"
#include <proto/awebjs.h>
#include "keyfile.h"

#include "rexxmsgext.h" 
#include <string.h>


struct RexxMsgExt _rmex;
struct RexxMsgExt *rmex = &_rmex;
BOOL arexxisgood;

struct MsgPort *replyport = NULL;

static WORD SendARexxMsgHost( STRPTR RString, STRPTR Host, STRPTR Extension, BOOL results);

struct Library *RexxSysBase;
struct RexxSysIFace *IRexxSys;


ULONG SetJSVarCallBack(struct Hook *hook, APTR unused, struct RexxMsg *rm)
{
    struct JContext *jc = ((struct RexxMsgExt *)rm->rm_avail)->rme_UserData;
    STRPTR varname = ((struct SetVarHookData *)hook->h_Data)->name;
    STRPTR value = ((struct SetVarHookData *)hook->h_Data)->value;
    

    /* Need to check for dots and split the variable by them then create a object 'tree' from it */
    /* If the last character is a dot we give up straight away. */
    
    ULONG length = strlen(varname);
    STRPTR nextdot;
    STRPTR p,varnamecopy;
    struct Jobject *jo,*jthis;
    struct Jvar *jv;
    
    jthis = IAWebJS->Jthis(jc);
    
    if(varname[length -1] == '.')
    {
        return ERR10_018; // Invalid argument to function 
    }
    if((jv = IAWebJS->Jproperty(jc,jthis,"varname")))
    {
        IAWebJS->Jasgstring(jc,jv,varname);
    }
    /* Make a copy of the varname string so we can safely modify it as we parse it */
    if((varnamecopy = IExec->AllocVecTags(length + 1,AVT_Type,MEMF_PRIVATE,TAG_DONE)))
    {
        strcpy(varnamecopy,varname);
        p = varnamecopy;
        
        while((nextdot = strchr(p,'.')))
        {
            nextdot[0] = '\0';
            if((jv = IAWebJS->Jproperty(jc,jthis,p)))
            {
                if(!(jo = IAWebJS->Jtoobject(jc,jv)))
                {
                    if((jo = IAWebJS->Newjobject(jc)))
                    {
                        IAWebJS->Jasgobject(jc,jv,jo);
                       
                    }
                }
                if(jo)
                {
                     jthis = jo;
                }
            }
            p = nextdot + 1;
        }
        if((jv = IAWebJS->Jproperty(jc,jthis,p)))
        {
            IAWebJS->Jasgstring(jc,jv,value);
        }
        IExec->FreeVec(varnamecopy);
    }
    return 0; 
    
}

ULONG GetJSVarCallBack(struct Hook *hook, APTR unused, struct RexxMsg *rm)
{
    struct Jcontext *jc = ((struct RexxMsgExt *)rm->rm_avail)->rme_UserData;
    STRPTR varname = ((struct GetVarHookData *)hook->h_Data)->name;
    STRPTR buffer = ((struct GetVarHookData *)hook->h_Data)->buffer;  // sizelimit 255 bytes!
    

    /* Need to check for dots and split the variable by them then walk the object 'tree' from it */
    /* If the last character is a dot we give up straight away. */
    
    ULONG length = strlen(varname);
    STRPTR nextdot;
    STRPTR p,varnamecopy;
    struct Jobject *jthis;
    struct Jvar *jv;
    
    jthis = IAWebJS->Jthis(jc);
    
    if(varname[length -1] == '.')
    {
        return ERR10_018; // Invalid argument to function 
    }
    
    /* make a copy of the varname string so we can safley modify it as we parse it */

    if((varnamecopy = IExec->AllocVecTags(length + 1,AVT_Type,MEMF_PRIVATE,TAG_DONE)))
    {
        strcpy(varnamecopy,varname);
        p = varnamecopy;
    
        while((nextdot = strchr(p,'.')))
        {

            nextdot[0] = '\0';

            if((jv = IAWebJS->Jproperty(jc,jthis,p)))
            {
                if(!(jthis = IAWebJS->Jtoobject(jc,jv)))
                {
                    IExec->FreeVec(varnamecopy);
                    return ERR10_003; // No memory available.
                }
             
            }
            else
            { 
                IExec->FreeVec(varnamecopy);
                return ERR10_040; // Invalid variable name;
            }
            p = nextdot + 1;
        }
        if((jv = IAWebJS->Jproperty(jc,jthis,p)))
        {
            STRPTR value = IAWebJS->Jtostring(jc,jv);
            if(value)
            {
                strncpy(buffer,value,255);
                buffer[255] = '\0';
            }
        }
        else
        {
             IExec->FreeVec(varnamecopy);
             return ERR10_040; // Invalid variable name;       
        }
        IExec->FreeVec(varnamecopy);
    }
    return 0; 
}

BOOL initarexx(struct Jcontext *jc)
{
    /* Open RexxSysLib */
    
    if((RexxSysBase = IExec->OpenLibrary("rexxsyslib.library",0)))
    {
        if((IRexxSys = (struct RexxSysIFace *)IExec->GetInterface (RexxSysBase,"main",1,0)))
        {
            /* Create the hooks */
            if((rmex->rme_SetVarHook = IExec->AllocSysObjectTags(ASOT_HOOK,ASOHOOK_Entry,SetJSVarCallBack,ASOHOOK_Subentry,SetJSVarCallBack,TAG_DONE)))
            {
                if((rmex->rme_GetVarHook = IExec->AllocSysObjectTags(ASOT_HOOK,ASOHOOK_Entry,GetJSVarCallBack,ASOHOOK_Subentry,GetJSVarCallBack,TAG_DONE)))
                {
                    /* Create a msg port for replies */
                    /* Needn't be public */
                    if((replyport = IExec->AllocSysObjectTags(ASOT_PORT,TAG_DONE)))
                    {
                        rmex->rme_UserData = jc;  // in this command line version of javascript there can only be one context. 
                        arexxisgood = TRUE;
                        return TRUE;
                    }

                }
            }
        }
    }
    arexxisgood = FALSE;
    return FALSE;
}

VOID freearexx(struct Jcontext *jc)
{
    /* Clear the hooks */
    if(rmex->rme_SetVarHook)
    {
        IExec->FreeSysObject(ASOT_HOOK,rmex->rme_SetVarHook);
        rmex->rme_SetVarHook = NULL;
    }
    if(rmex->rme_GetVarHook)
    {
        IExec->FreeSysObject(ASOT_HOOK,rmex->rme_GetVarHook);
        rmex->rme_GetVarHook = NULL;
    }
    
    /* Delete The Port */
    if(replyport)
    {
        IExec->FreeSysObject(ASOT_PORT,replyport);
        replyport = NULL;
    }
    if(IRexxSys)
    {
        IExec->DropInterface((struct Interface *)IRexxSys);
        IRexxSys = NULL;
    }
    if(RexxSysBase)
    {
        IExec->CloseLibrary(RexxSysBase);
        RexxSysBase = NULL;
    }
}

void SendCommand(struct Jcontext *jc)
{
    /* pop our args of the stack */
    
    struct JVar *jhost, *jcommand,*jresults;
    STRPTR host,command;
    BOOL results = TRUE;
    
    struct RexxMsg *rm;
    if(!arexxisgood)
    {
        printf("WARNING: ARexx not correctly initialised\n");
        return;
    }
    
    /* To start simple fail if insufficient args (really should throw an exeption!) */
    
    if(!(jhost = IAWebJS->Jfargument(jc,0)))
    {
        IAWebJS->Jasgboolean(jc,NULL,FALSE);
        return;
    }
    if(!(jcommand = IAWebJS->Jfargument(jc,1)))
    {
        IAWebJS->Jasgboolean(jc,NULL,FALSE);
        return;
    }
    
    /* this one optional defaults to true */
    
    jresults = IAWebJS->Jfargument(jc,2);
    if(jresults)
    {
    	results = IAWebJS->Jtoboolean(jc,jresults);
    }
    
    host = IAWebJS->Jtostring(jc,jhost);
    command = IAWebJS->Jtostring(jc,jcommand);
    if(SendARexxMsgHost(command,host,"",results))
    {
    
	    /* Now wait for the result ! */
	    
	    IExec->Wait(1<<replyport->mp_SigBit);
	    
	    rm = IExec->GetMsg(replyport);
	    if (rm->rm_Node.mn_Node.ln_Type==NT_REPLYMSG)
	    {
	        /* Extract the result */
	        /* Need to set result rc and rc2 in our parent object */
	        ULONG RC = 0;
	        ULONG RC2 = 0;
	        STRPTR Result = NULL;
	        
	        struct Jobject *jthis;
	        struct Jvar *jv;
	        
	        if((RC = rm->rm_Result1))
	        {
	            RC2 = rm->rm_Result2;
	        }
	        else
	        {
	            Result = rm->rm_Result2;

	        }
	        
	        jthis = IAWebJS->Jthis(jc);
	        
	        if((jv = IAWebJS->Jproperty(jc,jthis,"RC")))
	        {
	            IAWebJS->Jasgnumber(jc,jv,RC);
	        }
	        if((jv = IAWebJS->Jproperty(jc,jthis,"RESULT")))
	        {
	            if(Result)
	            {
	                IAWebJS->Jasgstring(jc,jv,Result);
	            }
	            else
	            {
	                IAWebJS->Jasgstring(jc,jv,""); // would prefer to set this to undefined but not sure if I can with modifying the javascript lib.
	            }
	        }
	        
	        if(Result)
	        {
	            /* I thik this my responsibilty to 'free' guess things will go bang if not! */
	            
	            IRexxSys->DeleteArgstring((STRPTR)rm->rm_Result2);
	        }
	        /*
	         * Free the arguments and the message...
	         */
	        IRexxSys->DeleteArgstring(rm->rm_Args[0]);
	        IRexxSys->DeleteRexxMsg(rm);
          IAWebJS->Jasgboolean(jc,NULL,TRUE);
          return;
	    }
	    else
	    {
	        printf("WARNING! Unexpected packet! %s %ld\n",__FUNCTION__,__LINE__);
	    }
	  }
    IAWebJS->Jasgboolean(jc,NULL,FALSE);
}


static WORD SendARexxMsgHost( STRPTR RString, STRPTR Host, STRPTR Extension, BOOL results)
{
    struct   MsgPort  *RexxPort;
    struct   RexxMsg  *rmsg;
    BOOL flag = FALSE;
    BOOL found = FALSE;
    if(replyport) 
    {
        if (RString)
        {
            if (rmsg=IRexxSys->CreateRexxMsg(replyport, Extension, Host))
            {
                rmsg->rm_Action=RXCOMM | (results?RXFF_RESULT:0);
                rmsg->rm_avail = rmex;
                if((rmsg->rm_Args[0] = IRexxSys->CreateArgstring(RString, (LONG)strlen(RString))))
                {
                    /*
                     * We need to find the RexxPort and this needs
                     * to be done in a Forbid()
                     */
       	            IExec->Forbid();
                    if((RexxPort = IExec->FindPort(Host)))
                    {
                        /*
                         * We found the port, so put the
                         * message to it...
                         */
                         found = TRUE;
                         IExec->PutMsg(RexxPort,(struct Message *)rmsg);
                  //       outstanding+=1;
                         flag=TRUE;
                     }
                     else
                     {
                        /*
                         * No port, so clean up...
                         */
                         IRexxSys->DeleteArgstring(rmsg->rm_Args[0]);
                         IRexxSys->DeleteRexxMsg(rmsg);
                     }
                     IExec->Permit();

                 }
                 else 
                 {
                    IRexxSys->DeleteRexxMsg(rmsg);
                 }
            }
       }
   }
   return(flag);
}

