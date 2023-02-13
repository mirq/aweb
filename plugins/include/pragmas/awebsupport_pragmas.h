/**/
/*--- object dispatcher*/
/**/
#pragma libcall AwebSupportBase AmethodA 1e 9802
#pragma libcall AwebSupportBase AmethodasA 24 98003
/* */
/*--- object methods*/
/* */
#pragma libcall AwebSupportBase AnewobjectA 2a 8002
#pragma libcall AwebSupportBase Adisposeobject 30 801
#pragma libcall AwebSupportBase AsetattrsA 36 9802
#pragma libcall AwebSupportBase AgetattrsA 3c 9802
#pragma libcall AwebSupportBase Agetattr 42 0802
#pragma libcall AwebSupportBase AupdateattrsA 48 A9803
#pragma libcall AwebSupportBase Arender 4e A432109808
#pragma libcall AwebSupportBase Aaddchild 54 09803
#pragma libcall AwebSupportBase Aremchild 5a 09803
#pragma libcall AwebSupportBase Anotify 60 9802
/* */
/*--- object support*/
/* */
#pragma libcall AwebSupportBase Allocobject 66 81003
#pragma libcall AwebSupportBase AnotifysetA 6c 9802
/* */
/*--- memory*/
/* */
#pragma libcall AwebSupportBase Allocmem 72 1002
#pragma libcall AwebSupportBase Dupstr 78 0802
#pragma libcall AwebSupportBase Freemem 7e 801
/* */
/*--- render*/
/* */
#pragma libcall AwebSupportBase Clipcoords 84 9802
#pragma libcall AwebSupportBase Unclipcoords 8a 801
#pragma libcall AwebSupportBase Erasebg 90 32109806
/* */
/*--- application*/
/* */
#pragma libcall AwebSupportBase Aweb 96 0
/* */
/*--- task*/
/* */
#pragma libcall AwebSupportBase AsetattrsasyncA 9c 9802
#pragma libcall AwebSupportBase Waittask a2 001
#pragma libcall AwebSupportBase Gettaskmsg a8 0
#pragma libcall AwebSupportBase Replytaskmsg ae 801
#pragma libcall AwebSupportBase Checktaskbreak b4 0
#pragma libcall AwebSupportBase Updatetask ba 801
#pragma libcall AwebSupportBase UpdatetaskattrsA c0 801
#pragma libcall AwebSupportBase Obtaintasksemaphore c6 801
/**/
/*--- debug*/
/**/
#pragma libcall AwebSupportBase Avprintf cc 9802
/**/
/*pragma libcall AwebSupportBase Awebprivate1 d2 0*/
/*pragma libcall AwebSupportBase Awebprivate2 d8 0*/
/*pragma libcall AwebSupportBase Awebprivate3 de 0*/
/*pragma libcall AwebSupportBase Awebprivate4 e4 0*/
/*pragma libcall AwebSupportBase Awebprivate5 ea 0*/
/*pragma libcall AwebSupportBase Awebprivate6 f0 0*/
/*pragma libcall AwebSupportBase Awebprivate7 f6 0*/
/*pragma libcall AwebSupportBase Awebprivate8 fc 0*/
/*pragma libcall AwebSupportBase Awebprivate9 102 0*/
/*pragma libcall AwebSupportBase Awebprivate10 108 0*/
/*pragma libcall AwebSupportBase Awebprivate11 10e 0*/
/*pragma libcall AwebSupportBase Awebprivate12 114 0*/
/**/
/*--- render*/
/**/
#pragma libcall AwebSupportBase Obtainbgrp 11a 32109806
#pragma libcall AwebSupportBase Releasebgrp 120 801
/**/
/*pragma libcall AwebSupportBase AWebprivate13 126 0*/
/*pragma libcall AwebSupportBase AWebprivate14 12c 0*/
/*pragma libcall AwebSupportBase AWebprivate15 132 0*/
/*pragma libcall AwebSupportBase AWebprivate16 138 0*/
/*pragma libcall AwebSupportBase AWebprivate17 13e 0*/
/*pragma libcall AwebSupportBase AWebprivate18 144 0*/
/*pragma libcall AwebSupportBase AWebprivate19 14a 0*/
/*pragma libcall AwebSupportBase AWebprivate20 150 0*/
/*pragma libcall AwebSupportBase AWebprivate21 156 0*/
/*pragma libcall AwebSupportBase AWebprivate22 15c 0*/
/*pragma libcall AwebSupportBase AWebprivate23 162 0*/
/*pragma libcall AwebSupportBase AWebprivate24 168 0*/
/*pragma libcall AwebSupportBase AWebprivate25 16e 0*/
/*pragma libcall AwebSupportBase AWebprivate26 174 0*/
/*pragma libcall AwebSupportBase AWebprivate27 17a 0*/
/*pragma libcall AwebSupportBase AWebprivate28 180 0*/
/*pragma libcall AwebSupportBase AWebprivate29 186 0*/
/*pragma libcall AwebSupportBase AWebprivate30 18c 0*/
/*pragma libcall AwebSupportBase AWebprivate31 192 0*/
/*pragma libcall AwebSupportBase AWebprivate32 198 0*/
/*pragma libcall AwebSupportBase AWebprivate33 19e 0*/
/*pragma libcall AwebSupportBase AWebprivate34 1a4 0*/
/*pragma libcall AwebSupportBase AWebprivate35 1aa 0*/
/**/
/* Library version 2.0*/
/**/
/*--- filter type plugins*/
/**/
#pragma libcall AwebSupportBase Setfiltertype 1b0 9802
#pragma libcall AwebSupportBase Writefilter 1b6 09803
/**/
/*--- commands*/
/**/
#pragma libcall AwebSupportBase Awebcommand 1bc 198004
/**/

#ifdef __SASC
#pragma tagcall AwebSupportBase Amethod 1e 9802
#pragma tagcall AwebSupportBase Amethodas 24 98003
#pragma tagcall AwebSupportBase Anewobject 2a 8002
#pragma tagcall AwebSupportBase Asetattrs 36 9802
#pragma tagcall AwebSupportBase Agetattrs 3c 9802
#pragma tagcall AwebSupportBase Aupdateattrs 48 A9803
#pragma tagcall AwebSupportBase Anotifyset 6c 9802
#pragma tagcall AwebSupportBase Asetattrsasync 9c 9802
#pragma tagcall AwebSupportBase Updatetaskattrs c0 801
#pragma tagcall AwebSupportBase Aprintf cc 9802
#endif
