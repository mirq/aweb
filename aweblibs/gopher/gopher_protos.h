/* Prototypes for functions defined in
gopher.c
 */

extern struct Library * AwebTcpBase;

extern void * AwebSupportBase;

extern struct Library * GopherBase;

extern struct ExecBase * SysBase;

LONG __asm __saveds Libstart(void);

extern int AWEBLIBVSTRING;

extern struct Resident romtag;

struct Library * __asm __saveds Initlib(register __a6 struct ExecBase * , register __a0 struct SegList * , register __d0 struct Library * );

struct Library * __asm __saveds Openlib(register __a6 struct Library * );

struct SegList * __asm __saveds Closelib(register __a6 struct Library * );

struct SegList * __asm __saveds Expungelib(register __a6 struct Library * );

ULONG __asm __saveds Extfunclib(void);
