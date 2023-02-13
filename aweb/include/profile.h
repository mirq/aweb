/**********************************************************************
 *
 * This file is part of the AWeb-II distribution
 *
 * Copyright (C) 2002 Yvon Rozijn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the AWeb Public License as included in this
 * distribution.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * AWeb Public License for more details.
 *
 **********************************************************************/

#ifdef PROFILE

extern void setupprof(void);
extern void prolog(char *p);
extern void epilog(char *p);
extern void report(void);

#define profile(n,c) prolog(n);c;epilog(n)

#else

#define setupprof()
#define prolog(n)
#define epilog(n)
#define report()

#define profile(n,c) c

#endif
