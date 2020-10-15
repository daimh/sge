/* $VER: $Id: coda.yy,v 1.1 2001-07-18 11:05:53 root Exp $ */

%{
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef yywrap
#undef yywrap
#endif
%}

%%

(\n|\f)[^\n\t\f ]+"/"[^\n\t\f ]+[\t ]+[^\n\t\f ]+"/"[^\n\t\f ]+\n  {
                           register char *s, *t;
                           yyless(yyleng-1);
                           if( (s= strdup(&yytext[1])) ) {
                             for(t=s; *t && *t!=' ' && *t!='\t'; t++) ;
                             *t='\0';
                             printf("\n/****** %s ******",s);
                             free(s);
                           }
                         }

\f[\n]*[^\n\t\f ]+"/"[^\n\t\f ]+([\t ]*\n){2}   { /* The MUI Problem */
                           register char *s, *t, *u;
                           yyless(yyleng-2);
                           for(u=&yytext[1]; *u=='\n' || *u=='\f'; u++) ;
                           if( (s= strdup(u)) ) {
                             for(t=s; *t && *t!=' ' && *t!='\t' && *t!='\n'; t++) ;
                             *t='\0';
                             printf("******/\n/****** %s ******\n*",s);
                             free(s);
                           }
                         }

\n                       { printf("%s*",yytext); }
\f                       { printf("*****/\n"); }
\n[^ \f\t\n]             { printf(" %s", &yytext[1]); }       /* word wrap? */
\n"TABLE OF CONTENTS"    { printf("\n******/\n%s",yytext); }  /* next file */
\n\n"TABLE OF CONTENTS"  { printf("\n******/\n%s",yytext); }  /* next file */
.                        { ECHO; }

<<EOF>>                  { printf("******/\n"); yyterminate(); }

%%

#if 0
\n\f                     { printf("\n******/\n"); }
#endif

#ifdef yywrap
#undef yywrap
#endif

int yywrap() { return 1; }
int main()   { return yylex(); }
