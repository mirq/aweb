#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#undef isblank
#define isblank(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r')

FILE *xfopen(char *file, char *mode)
{
    FILE *ret = fopen(file, mode);
    if (!ret)
    {
        fprintf(stderr, "Error while opening %s: %s\n", file, strerror(errno));
        exit(20);
    }

    return ret;
}

char *ReadFile(FILE *file, long *sizep)
{
    char *buf; long size;

    if (fseek(file, 0, SEEK_END) == -1)
    {
        perror(NULL);
        exit(20);
    }

    size = ftell(file);
    rewind(file);

    buf = malloc(size + 1);
    if (!buf)
    {
        perror(NULL);
        exit(20);
    }

    if (fread(buf, size, 1, file) != 1)
    {
        perror(NULL);
        exit(20);
    }

    buf[size] = '\0';

    if (sizep) *sizep = size;

    return buf;


}

char *GetPrevName(char *in, char **end)
{
    while (isblank(*in)) in--;

    *end = in;

    while (isalnum(*in)) in--;
    in++;

    return in;
}

FILE *xtmpfile()
{
    FILE *file = tmpfile();

    if (!file)
    {
        perror(NULL);
        exit(20);
    }

    return file;
}

char *ProcessArg(char *in, char *max, FILE *outfile)
{
    char regnam, regnum;
    int numchar;

    char *name, *name_end, *end;

    end = in;

    while (end < max && *end != ',') end++;

    printf("max = %s\n", max);
    printf("end = %s\n", end);
    printf("in = %s\n", in);

    if (sscanf(in, " register __%c%c%n", &regnam, &regnum, &numchar) != 2)
        return 0;

    in += numchar;

    name = GetPrevName(end - 1, &name_end);

    printf("name = %s\n", name);

    fputs(",\n", outfile);
    fwrite(in, name - in, 1, outfile);
    fputs(", ", outfile);
    fwrite(name, name_end - name + 1, 1, outfile);
    fprintf(outfile, ", %c%c",
            toupper(regnam), regnum);

    return end;
}

char *FindMatchingBraket(char *in)
{
    int counter = 1;

    char open  = *in;
    char close;

    switch (open)
    {
        case '(': close = ')'; break;
        case '{': close = '}'; break;
        case '[': close = ']'; break;

        default: return 0;
    }

    for (in++; *in && counter; in++)
    {
        if (*in == open) counter++;
        else
        if (*in == close) counter--;
    }

    return !counter ? in-1 : 0;
}

char *ProcessFunc(char *in, FILE *outfile)
{
    char *lparen, *rparen, *funcname, *funcname_end;
    FILE *tmp;
    int numargs = 0;

    in += 5; //skip "__asm"


    lparen = strchr(in, '(');
    if (!lparen)
        return 0;

    rparen = FindMatchingBraket(lparen);
    if (!rparen)
        return 0;

    tmp = xtmpfile();

    funcname = GetPrevName(lparen - 1, &funcname_end);
    fputs("USRFUNC_Hxx\n"
            "(\n", tmp);
    fwrite(in, funcname - in, 1, tmp);
    fputs(", ", tmp);
    fwrite(funcname, funcname_end - funcname + 1, 1, tmp);

    in = lparen + 1;

    /* Is this a 'void' function? */
    while (isblank(*in)) in++;
    if (strncmp(in, "void", 4) == 0)
    {
        in += 4;
        while (isblank(*in)) in++;
    }

    if (in != rparen)
    {
        in--;

        while (in != rparen && (in = ProcessArg(in+1, rparen, tmp)))
            numargs++;
    }


    if (in)
    {
        in++;

        fputs("\n)", tmp);

        while (isblank(*in)) in++;
        if (*in == '{')
        {
            char *next_in;

            fputs("\n{\n     USRFUNC_INIT\n", tmp);
            next_in = FindMatchingBraket(in);

            if (!next_in)
            {
                 fputs("Missing '}'\n", stderr);
                 exit(20);
            }

            fwrite(in+1, next_in - in - 2, 1, tmp);

            fputs("\n     USRFUNC_EXIT\n", tmp);
            in = next_in-1;
        }

        fseek(tmp, 8, SEEK_SET);
        fprintf(tmp, "%c%-2d", *in==';'?'P':'H', numargs);

        fputs(ReadFile(tmp, NULL), outfile);
    }

    fclose(tmp);

    return in;
}

int ProcessFile(FILE *infile, FILE *outfile)
{
    char *in, *loc, *oldin;
    long size;

    if (!(infile && outfile))
        return 20;

    in = ReadFile(infile, &size);


    while ((loc = strstr(in, "__asm")))
    {
        if (loc != in && fwrite(in, (long)(loc - in), 1, outfile) != 1)
        {
            perror(NULL);
            exit(20);
        }

        oldin = in;

        in = ProcessFunc(loc, outfile);
        if (!in)
        {
            fwrite("__asm", 5, 1, outfile);
            in = loc + 5;
        }

        size -= (long)(in - oldin);
    }

    fwrite(in, size, 1, outfile);

    return 0;
}

int main(int argc, char *argv[])
{
    int i, res = 0;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s infile outfile\n", argv[0]);
        return 20;
    }

    res = ProcessFile(xfopen(argv[1], "r"), xfopen(argv[2], "w+"));

    return res;
}
