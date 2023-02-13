#include "platform_specific.h"

#if defined(__amigaos4__)
#undef __USE_INLINE__
#include <proto/exec.h>
#include <proto/dos.h>

#include <stdarg.h>

int VARARGS68K adebug(UBYTE *fmt, ...);

int VARARGS68K araddebug(UBYTE *fmt, ...);

int adebug(UBYTE *fmt, ...)
{
    VA_LIST ap;
    LONG *argv;
    LONG result;

    struct ExecIFace *myExec = (struct ExecIFace*)(*(struct ExecBase **)4)->MainInterface;

    struct DOSBase *myDOSBase = (struct DOSBase *)myExec->OpenLibrary("dos.library",0);
    struct DOSIFace *myIDOS = (struct DOSIFace *)myExec->GetInterface((struct Library *)myDOSBase,"main",1,0);
    if(myDOSBase && myIDOS)
    {

    BPTR debug = myIDOS->Open("ram:debug",MODE_OLDFILE);
    VA_STARTLIN(ap,fmt);
    argv = (LONG *)VA_GETLIN(ap,LONG *);

    if(debug)
    {
        myIDOS->Seek(debug,0,OFFSET_END);
        result = myIDOS->VFPrintf(debug,fmt,argv);
        myIDOS->Close(debug);
    }
    else
    {
    debug = myIDOS->Open("ram:debug",MODE_NEWFILE);
    if(debug)
    {
        myIDOS->Seek(debug,0,OFFSET_END);
        result = myIDOS->VFPrintf(debug,fmt,argv);
        myIDOS->Close(debug);
    }

    }

    VA_END(argv);

    }
    if(myIDOS)      myExec->DropInterface((struct Interface *)myIDOS);
    if(myDOSBase) myExec->CloseLibrary((struct Library *)myDOSBase);

}

#if defined(__amigaos4__)
#define RADDEBUG "sys:debug"
#else
#define RADDEBUG "rad:debug"
#endif

int araddebug(UBYTE *fmt, ...)
{
    VA_LIST ap;
    LONG *argv;
    LONG result;

    struct ExecIFace *myExec = (struct ExecIFace*)(*(struct ExecBase **)4)->MainInterface;

    struct DOSBase *myDOSBase = (struct DOSBase *)myExec->OpenLibrary("dos.library",0);
    struct DOSIFace *myIDOS = (struct DOSIFace *)myExec->GetInterface((struct Library *)myDOSBase,"main",1,0);
    if(myDOSBase && myIDOS)
    {

    BPTR debug = myIDOS->Open(RADDEBUG,MODE_OLDFILE);
    VA_STARTLIN(ap,fmt);
    argv = (LONG *)VA_GETLIN(ap,LONG *);

    if(debug)
    {
        myIDOS->Seek(debug,0,OFFSET_END);
        result = myIDOS->VFPrintf(debug,fmt,argv);
        myIDOS->Close(debug);
    }
    else
    {
    debug = myIDOS->Open(RADDEBUG,MODE_NEWFILE);
    if(debug)
    {
        myIDOS->Seek(debug,0,OFFSET_END);
        result = myIDOS->VFPrintf(debug,fmt,argv);
        myIDOS->Close(debug);
    }

    }

    VA_END(argv);

    }
    if(myIDOS)      myExec->DropInterface((struct Interface *)myIDOS);
    if(myDOSBase) myExec->CloseLibrary((struct Library *)myDOSBase);

}
#endif
