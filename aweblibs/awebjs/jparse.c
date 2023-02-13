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

/* jparse.c - AWeb js parser */

#include "awebjs.h"
#include "jprotos.h"
#include <math.h>

#include <stdio.h>

struct Parser
{  void *pool;
   UBYTE *next;
   UBYTE *line;
   struct Jcontext *jc;
   UWORD flags;
   long linenr;
   BOOL newexpr;             /* New expression */
   BOOL skipnewline;              /* Don't skip new lines as white space */
};

struct Parsestate
{
    struct Parser ps_pa;
    struct Token  ps_token;
};

#define PAF_ERROR    0x0001   /* Error occurred, recover at ; */

struct Keyworddef
{  UBYTE *kwd;
   UWORD id;
};

static struct Keyworddef keywordtab[]=
{
#ifdef JSADDRESS
   "address",     JT_ADDRESS,
#endif
   "break",       JT_BREAK,
   "case",        JT_CASE,
   "catch",       JT_CATCH,
   "continue",    JT_CONTINUE,
#ifdef JSDEBUG
   "debug",       JT_DEBUG,
#endif
   "default",     JT_DEFAULT,
   "delete",      JT_DELETE,
   "do",          JT_DO,
   "else",        JT_ELSE,
   "finally",     JT_FINALLY,
   "for",         JT_FOR,
   "function",    JT_FUNCTION,
   "if",          JT_IF,
   "in",          JT_IN,
   "instanceof",  JT_INSTANCEOF,
   "new",         JT_NEW,
   "return",      JT_RETURN,
   "switch",      JT_SWITCH,
   "this",        JT_THIS,
   "throw",       JT_THROW,
   "try",         JT_TRY,
   "typeof",      JT_TYPEOF,
   "var",         JT_VAR,
   "void",        JT_VOID,
   "while",       JT_WHILE,
   "with",        JT_WITH
};
#define NRKEYWORDS   sizeof(keywordtab)/sizeof(struct Keyworddef)

/*-----------------------------------------------------------------------*/

/* Return TRUE if there is nothing but whitespace from here */
static BOOL Isallws(UBYTE *p)
{  while(*p && isspace(*p)) p++;
   return (BOOL)!*p;
}

/* Return the value for this hex digit */
static int Hexvalue(UBYTE c)
{  c=toupper(c);
   if(c>='0' && c<='9') return c-'0';
   if(c>='A' && c<='F') return c-'A'+10;
   return 0;
}

/* Return delimited string, advance parser */
static UBYTE *Getstring(struct Parser *pa,UBYTE sep)
{  UBYTE *p,*q;
   UBYTE *svalue;
   short o;
   for(p=pa->next+1;*p && *p!=sep;p++)
   {  if(*p=='\\') p++;
   }
   if((svalue = ALLOCTYPE(UBYTE, p-pa->next, 0, pa->pool)))
   {  for(p=pa->next+1,q=svalue;*p && *p!=sep;p++)
      {  switch(*p)
         {  case '\\':
               p++;
               switch(*p)
               {  case 'b':   *q++='\b';break;
                  case 't':   *q++='\t';break;
                  case 'n':   *q++='\n';break;
                  case 'f':   *q++='\f';break;
                  case 'r':   *q++='\r';break;
                  case '"':   *q++='"';break;
                  case '\'':  *q++='\'';break;
                  case '\\':  *q++='\\';break;
                  case 'x':
                  case 'X':
                     *q=0;
                     p++;
                     if(isxdigit(*p))
                     {  *q=(*q)*16+Hexvalue(*p);
                        p++;
                        if(isxdigit(*p))
                        {  *q=(*q)*16+Hexvalue(*p);
                           p++;
                        }
                        p--;
                        q++;
                     }
                     else
                     {  p--;
                        *q++=*p;
                     }
                     break;
                  default:
                     *q=0;
                     if(isdigit(*p) && (o=*p-'0')<8)
                     {  *q=(*q)*8+o;
                        p++;
                        if(isdigit(*p) && (o=*p-'0')<8)
                        {  *q=(*q)*8+o;
                           p++;
                           if(isdigit(*p) && (o=*p-'0')<8)
                           {  *q=(*q)*8+o;
                              p++;
                           }
                        }
                        p--;
                        q++;
                     }
                     else
                     {  *q++=*p;
                     }
                     break;
               }
               break;
            case '\n':
            case '\r':
            case '\0':
               Errormsg(pa,"String not terminated");
               break;
            default:
               *q++=*p;
               break;
         }
      }
      if(*p) p++;
   }
   pa->next=p;
   return svalue;
}

/* Return regexpression */

static UBYTE *Getregexp(struct Parser *pa,UBYTE sep)
{
    UBYTE *p,*q;
    UBYTE *svalue;
    BOOL inclass = FALSE;

    for(p=pa->next+1;*p && (*p!=sep || inclass);p++)
    {
        switch(*p)
        {
            case '\\':
               p++;
               break;
            case '[':
               inclass = TRUE;
               break;
            case ']':
               inclass = FALSE;
               break;
        }
    }
    inclass = FALSE;

    if((svalue = ALLOCTYPE(UBYTE, p-pa->next, 0, pa->pool)))
    {  for(p=pa->next+1,q=svalue;*p && (*p!=sep || inclass);p++)
       {  switch(*p)
          {
             case '\\':
                *q++ = *p++;
                *q++ = *p;
                break;
             case '\n':
             case '\r':
             case '\0':
                Errormsg(pa,"Regular expression unterminated");
                break;
             case '[':
                inclass = TRUE;
                *q++ = *p;
                break;
             case ']':
                inclass = FALSE;
                *q++ = *p;
                break;
             default:
                *q++ = *p;
                break;
          }
       }
       if(*p) p++;
    }

    pa->next=p;
    return svalue;
}

static UBYTE *Getregexpflag(struct Parser *pa){

    UBYTE *p,*q;
    UBYTE *svalue;

    for(p=pa->next;*p && isalpha(*p);p++)
    {
    }
    if(p == pa->next)
    {
        return NULL;
    }
    if((svalue = ALLOCTYPE(UBYTE, p-pa->next + 1, 0, pa->pool)))
    {  for(p=pa->next,q=svalue;*p && isalpha(*p);p++)
       {
          *q++=*p;
       }
       //if(*p) p++;
    }

    pa->next=p;
    return svalue;

}

/* Return integer value, advance parser */
static double Getint(struct Parser *pa)
{  double n=0.0;
   while(isdigit(*pa->next))
   {  n=n*10.0+(double)(*pa->next-'0');
      pa->next++;
   }
   return n;
}

/* Return octal value, advance parser */
static double Getoctint(struct Parser *pa)
{  long n=0.0;
   short o;
   while(isdigit(*pa->next) && (o=*pa->next-'0')<8)
   {  n=n*8+(double)o;
      pa->next++;
   }
   return n;
}

/* Return hex value, advance parser */
static double Gethexint(struct Parser *pa)
{  double n=0.0;
   while(isxdigit(*pa->next))
   {  n=n*16.0 +(double)Hexvalue(*pa->next);
      pa->next++;
   }
   return n;
}

/* Return float value, advance parser */
static double Getfloat(struct Parser *pa, double ipart)
{  long exp=0;
   double ffrac,f;
   short exps=1;
   f=(double)ipart;
   while(isdigit(*pa->next))
   {  f=f*10.0+(*pa->next-'0');
      pa->next++;
   }
   if(*pa->next=='.')
   {  pa->next++;
      ffrac=1.0;
      while(isdigit(*pa->next))
      {  ffrac/=10.0;
         f+=ffrac*(*pa->next-'0');
         pa->next++;
      }
   }
   if(toupper(*pa->next)=='E')
   {  pa->next++;
      switch(*pa->next)
      {  case '-':   exps=-1;pa->next++;break;
         case '+':   pa->next++;break;
      }
      while(isdigit(*pa->next))
      {  exp=10*exp+(*pa->next-'0');
         pa->next++;
      }
   }
   f*=pow(10.0,(double)(exps*exp));
   return f;
}

/* Return identifier or keyword string, advance parser */
static UBYTE *Getname(struct Parser *pa)
{  UBYTE *p;
   UBYTE *name;
   for(p=pa->next;*p && (isalnum(*p) || *p=='_' || *p=='$');p++);
   if((name=ALLOCTYPE(UBYTE,p-pa->next+1,MEMF_CLEAR,pa->pool)))
   {  strncpy(name,pa->next,p-pa->next);
   }
   pa->next=p;
   return name;
}

/* Advance parser to end of line */
static void Skipline(struct Parser *pa)
{  while(*pa->next && !((pa->next[0]=='\r' && pa->next[1]!='\n') || pa->next[0]=='\n'))
   {  pa->next++;
   }
   if(*pa->next) pa->next++;
   pa->line=pa->next;
   pa->linenr++;
   /* is it always a new expression? investigate*/
   /* I supsect not a cooment to end of line can occur in an if statement ie if() // */
   //pa->newexpr = TRUE;
}

/* Advance parser to end of comment */
static void Skipcomment(struct Parser *pa)
{  while(*pa->next && !(pa->next[0]=='*' && pa->next[1]=='/'))
   {  if((pa->next[0]=='\r' && pa->next[1]!='\n') || pa->next[0]=='\n')
      {  pa->line=pa->next+1;
         pa->linenr++;
      }
      pa->next++;
   }
   if(*pa->next) pa->next+=2;
}

/* If name is a keyword or special literal, use its id */
static void Tokenize(struct Token *tok)
{  short a=0,b=NRKEYWORDS-1,m;
   long c;
   while(a<=b)
   {  m=(a+b)/2;
      c=strcmp(keywordtab[m].kwd,tok->svalue);
      if(c==0)
      {  tok->id=keywordtab[m].id;
         return;
      }
      if(c<0) a=m+1;
      else b=m-1;
   }
   if(STREQUAL(tok->svalue,"null"))
   {  tok->id=JT_NULLLIT;
   }
   else if(STREQUAL(tok->svalue,"true"))
   {  tok->id=JT_BOOLEANLIT;
      tok->ivalue=1;
   }
   else if(STREQUAL(tok->svalue,"false"))
   {  tok->id=JT_BOOLEANLIT;
      tok->ivalue=0;
   }
}

/*-----------------------------------------------------------------------*/

/* Build the next token, advance parser */
struct Token *Nexttoken(struct Parser *pa)
{
   double ivalue;

   BOOL newexpression = FALSE;
   struct Token *token = &(pa->jc->token);



   if(token->svalue)
   {
      FREE(token->svalue);
      token->svalue=NULL;
   }
   if(token->svalue2)
   {
      FREE(token->svalue);
      token->svalue2=NULL;
   }

   while(isspace(*pa->next))
   {  if((pa->next[0]=='\r' && pa->next[1]!='\n') || pa->next[0]=='\n')
      {  pa->line=pa->next+1;
         pa->linenr++;
         if(!(pa->skipnewline))
         {
             pa->next++;
             token->id = JT_NEWLINE;
             return token;
         }
      }
      pa->next++;
   }

   switch(*pa->next)
   {  case '\0':
         token->id=0;
         /* end of line? arb*/
         newexpression = TRUE;
         break;
      case '=':
         pa->next++;
         switch(*pa->next)
         {  case '=':
                pa->next++;
                switch(*pa->next)
                {
                    case '=':
                        token->id=JT_EXEQ;pa->next++;break;
                    default:
                        token->id=JT_EQ;break;
                }
                break;
            default:    token->id=JT_ASSIGN;break;
         }
         newexpression = TRUE;
         break;
      case '<':
         pa->next++;
         switch(*pa->next)
         {  case '=':   token->id=JT_LE;pa->next++;break;
            case '<':
               pa->next++;
               switch(*pa->next)
               {  case '=':   token->id=JT_ASHLEFT;pa->next++;break;
                  default:    token->id=JT_SHLEFT;break;
               }
               break;
            case '!':
               if(pa->next[1]=='-' && pa->next[2]=='-')
               {  Skipline(pa);
                  return Nexttoken(pa);
               }
               token->id=JT_LT;
               break;
            default:    token->id=JT_LT;break;
         }
         newexpression = TRUE;
         break;
      case '>':
         pa->next++;
         switch(*pa->next)
         {  case '=':   token->id=JT_GE;pa->next++;break;
            case '>':
               pa->next++;
               switch(*pa->next)
               {  case '=':   token->id=JT_ASHRIGHT;pa->next++;break;
                  case '>':
                     pa->next++;
                     switch(*pa->next)
                     {  case '=':   token->id=JT_AUSHRIGHT;pa->next++;break;
                        default:    token->id=JT_USHRIGHT;break;
                     }
                     break;
                  default:    token->id=JT_SHRIGHT;break;
               }
               break;
            default:    token->id=JT_GT;break;
         }
         newexpression = TRUE;
         break;
      case '!':
         pa->next++;
         switch(*pa->next)
         {
            case '=':
                pa->next++;
                switch(*pa->next)
                {
                    case '=':
                        token->id=JT_NEXEQ;pa->next++;break;
                    default:
                        token->id=JT_NE;break;
                }
                break;
            default:    token->id=JT_NOT;break;
         }
         newexpression = TRUE;
         break;
      case '+':
         pa->next++;
         switch(*pa->next)
         {  case '=':   token->id=JT_APLUS;pa->next++;break;
            case '+':   token->id=JT_INC;pa->next++;break;
            default:    token->id=JT_PLUS;break;
         }
         newexpression = TRUE;
         break;
      case '-':
         pa->next++;
         switch(*pa->next)
         {  case '=':   token->id=JT_AMINUS;pa->next++;break;
            case '-':
               pa->next++;
               switch(*pa->next)
               {  case '>':   /* --> */
                     if(Isallws(pa->next+1))
                     {  token->id=JT_SEMICOLON;
                        pa->next++;
                     }
                     else
                     {  token->id=JT_DEC;
                     }
                     break;
                  default:
                     token->id=JT_DEC;
                     break;
               }
               break;
            default:    token->id=JT_MINUS;break;
         }
         newexpression = TRUE;
         break;
      case '*':
         pa->next++;
         switch(*pa->next)
         {  case '=':   token->id=JT_AMULT;pa->next++;break;
            default:    token->id=JT_MULT;break;
         }
         newexpression = TRUE;
         break;
      case '/':
         pa->next++;
         switch(*pa->next)
         {  case '=':   token->id=JT_ADIV;pa->next++;break;
            case '/':
               Skipline(pa);
               return Nexttoken(pa);
            case '*':
               pa->next++;
               Skipcomment(pa);
               return Nexttoken(pa);
            default:

               if(pa->newexpr)
               {
                   /*step back the one char we advanced */
                   pa->next--;
                   token->id = JT_REGEXPLIT;
                   token->svalue = Getregexp(pa,'/');
                   token->svalue2 = Getregexpflag(pa);
               }
               else  token->id=JT_DIV;
               break;
         }
         newexpression = TRUE;
         break;
      case '%':
         pa->next++;
         switch(*pa->next)
         {  case '=':   token->id=JT_AREM;pa->next++;break;
            default:    token->id=JT_REM;break;
         }
         newexpression = TRUE;
         break;
      case '&':
         pa->next++;
         switch(*pa->next)
         {  case '=':   token->id=JT_ABITAND;pa->next++;break;
            case '&':   token->id=JT_AND;pa->next++;break;
            default:    token->id=JT_BITAND;break;
         }
         newexpression = TRUE;
         break;
      case '|':
         pa->next++;
         switch(*pa->next)
         {  case '=':   token->id=JT_ABITOR;pa->next++;break;
            case '|':   token->id=JT_OR;pa->next++;break;
            default:    token->id=JT_BITOR;break;
         }
         newexpression = TRUE;
         break;
      case '^':
         pa->next++;
         switch(*pa->next)
         {  case '=':   token->id=JT_ABITXOR;pa->next++;break;
            default:    token->id=JT_BITXOR;break;
         }
         newexpression = TRUE;
         break;
      case '?':
         token->id=JT_CONDIF;
         pa->next++;
         newexpression = TRUE;
         break;
      case ':':
         token->id=JT_CONDELSE;
         pa->next++;
         newexpression = TRUE;
         break;
      case '.':
         if(isdigit(pa->next[1]))
         {  token->id=JT_FLOATLIT;
            token->fvalue=Getfloat(pa,0);
         }
         else
         {  pa->next++;
            token->id=JT_DOT;
         }
         break;
      case '~':
         token->id=JT_BITNEG;
         pa->next++;
         newexpression = TRUE;
         break;
      case '(':
         token->id=JT_LEFTPAR;
         pa->next++;
         newexpression = TRUE;
         break;
      case ')':         token->id=JT_RIGHTPAR;pa->next++;break;
      case '{':
         token->id=JT_LEFTBRACE;
         pa->next++;
         newexpression = TRUE;
         break;
      case '}':         token->id=JT_RIGHTBRACE;pa->next++;break;
      case '[':
         token->id=JT_LEFTBRACKET;
         pa->next++;
         newexpression = TRUE;
         break;
      case ']':         token->id=JT_RIGHTBRACKET;pa->next++;break;

      case ',':
         token->id=JT_COMMA;
         pa->next++;
         newexpression = TRUE;
         break;
      case ';':
         token->id=JT_SEMICOLON;
         pa->next++;
         pa->flags&=~PAF_ERROR;
         newexpression = TRUE;
         break;
      case '"':
         token->id=JT_STRINGLIT;
         token->svalue=Getstring(pa,'"');
         break;
      case '\'':
         token->id=JT_STRINGLIT;
         token->svalue=Getstring(pa,'\'');
         break;
      case '0':
         pa->next++;
         switch(*pa->next)
         {  case '.':
            case 'e':
            case 'E':
               token->id=JT_FLOATLIT;
               token->fvalue=Getfloat(pa,0);
               break;
            case 'x':
            case 'X':
               pa->next++;
               token->id=JT_INTEGERLIT;
               token->fvalue=Gethexint(pa);
               break;
            default:
               token->id=JT_INTEGERLIT;
               if(isdigit(*pa->next))
               {  token->fvalue=Getoctint(pa);
               }
               else
               {  token->fvalue=0.0;
               }
               break;
         }
         break;
      default:
         if(isdigit(*pa->next))
         {  ivalue=Getint(pa);
            switch(*pa->next)
            {  case '.':
               case 'e':
               case 'E':
                  token->id=JT_FLOATLIT;
                  token->fvalue=Getfloat(pa,ivalue);
                  break;
               default:
                  token->id=JT_INTEGERLIT;
                  token->fvalue=ivalue;
                  break;
            }
         }
         else if(isalpha(*pa->next) || *pa->next=='_' || *pa->next=='$')
         {  token->svalue=Getname(pa);
            token->id=JT_IDENTIFIER;
            Tokenize(token);
         }
         else
         {  pa->flags&=~PAF_ERROR;
            Errormsg(pa,"Invalid token");
            pa->flags&=~PAF_ERROR;
            token->id=JT_SEMICOLON;
            pa->next++;
            newexpression = TRUE;
         }
         break;
   }

   pa->newexpr = FALSE;
   if(newexpression) pa->newexpr = TRUE;

   return token;
}

void *Newparser(struct Jcontext *jc,UBYTE *source)
{  struct Parser *pa=ALLOCSTRUCT(Parser,1,0,jc->pool);
   if(pa)
   {  pa->pool=jc->pool;
      pa->next=source;
      pa->line=source;
      pa->jc=jc;
      pa->linenr=1;
      pa->newexpr = TRUE;
      pa->skipnewline=TRUE;
   }
   return pa;
}

/* make a copy of the current parse state (not just position */

void *Saveparser(struct Jcontext *jc, struct Parser *pa)
{
    struct Parsestate *npa = ALLOCSTRUCT(Parsestate,1,0,jc->pool);
    if(npa)
    {
        *(&npa->ps_pa) = *pa;
        npa->ps_token.id = jc->token.id;
        if(jc->token.svalue)
        {
            npa->ps_token.svalue = Jdupstr(jc->token.svalue,-1,jc->pool);
        }
        if(jc->token.svalue2)
        {
            npa->ps_token.svalue2 = Jdupstr(jc->token.svalue2,-1,jc->pool);
        }
        npa->ps_token.ivalue = jc->token.ivalue;
        npa->ps_token.fvalue = jc->token.fvalue;

    }
    return npa;
}

void Restoreparser(struct Jcontext *jc, struct Parser *pa,  void *saved)
{
    struct Parsestate *ps = saved;
    if(pa && ps)
    {
        *pa = *(&ps->ps_pa);
        /* before restoreing free any strings */
        if(jc->token.svalue)
        {
            FREE(jc->token.svalue);
            jc->token.svalue = NULL;
        }
        if(jc->token.svalue2)
        {
            FREE(jc->token.svalue2);
            jc->token.svalue = NULL;
        }
        *(&jc->token) = *(&ps->ps_token);
    }
    if(ps)
    {
        FREE(ps);
    }

}

void Freesavedparser(struct Jcontext *jc, void *saved)
{
    struct Parsestate *ps = (struct Parsestate *)saved;
    if(ps)
    {
        if (ps->ps_token.svalue)
        {
            FREE(ps->ps_token.svalue);
        }
        if (ps->ps_token.svalue2)
        {
            FREE(ps->ps_token.svalue2);
        }
        FREE(ps);
    }

}
void Freeparser(struct Parser *pa)
{  if(pa)
   {  FREE(pa);
   }
}

void Errormsg(struct Parser *pa,UBYTE *msg,...)
{
   VA_LIST va;
   VA_START(va,msg);
   if(!(pa->jc->flags&JCF_ERRORS))
   {  pa->jc->flags|=JCF_IGNORE;
   }
   if(!(pa->flags&PAF_ERROR) && !(pa->jc->flags&JCF_IGNORE))
   {  if(Errorrequester(pa->jc,pa->jc->linenr+pa->linenr,pa->line,pa->next-pa->line,msg,va))
      {  pa->jc->flags|=JCF_IGNORE;
      }
   }
   pa->jc->flags|=JCF_ERROR;
   pa->flags|=PAF_ERROR;

   VA_END(va);
}

UBYTE *Tokenname(UWORD id)
{  UBYTE *p;
   switch(id)
   {  case JT_LEFTPAR:     p="'('";break;
      case JT_RIGHTPAR:    p="')'";break;
      case JT_COMMA:       p="','";break;
      case JT_SEMICOLON:   p="';'";break;
      case JT_RIGHTBRACKET:p="']'";break;
      case JT_RIGHTBRACE:  p="'}'";break;
      case JT_WHILE:       p="while";break;
      case JT_CONDIF:      p="'?'";break;
      case JT_CONDELSE:    p="':'";break;
      default:             p="(?)";break;
   }
   return p;
}

ULONG Parserstate(struct Parser *pa)
{  return (ULONG)pa->next;
}

long Plinenr(struct Parser *pa)
{  return pa->linenr;
}

void Pskipnewline(struct Parser *pa, BOOL skip)
{
    pa->skipnewline = skip;
}
