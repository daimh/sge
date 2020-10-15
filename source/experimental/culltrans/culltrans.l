%{
#if defined(LINUX) || defined(ALPHA4) || defined(ALPHA5) || defined(IRIX6) || defined(HP11)
#  define YY_DECL int yylex(void)
#else
#  define YY_DECL extern "C" int yylex(void)
#endif
#include "culltrans_y.tab.h"
#include <string.h>
%}

liststart          LISTDEF
listend            LISTEND
sge                SGE_
booldef            BOOL
floatdef           FLOAT
doubledef          DOUBLE
ulongdef           ULONG
longdef            LONG
chardef            CHAR
intdef             INT
stringdef          STRING
listdef            LIST
objdef             OBJECT
idlstart           \/\*IDL
idlend             XIDL\*\/
uppercase          [A-Z]
spaces             [ \t]*

%start list idl attr
%p 4000
%n 1550
%e 2000

%%
{spaces}I{liststart}                  {BEGIN list; printf("%s\n", yytext); return IDLGEN_ILISTSTART; }
{spaces}S{liststart}                  {BEGIN list; printf("%s\n", yytext); return IDLGEN_SLISTSTART; }
<list>{spaces}{listend}               {BEGIN 0; printf("%s\n", yytext); return IDLGEN_LISTEND; }
<list>{spaces}{sge}K{ulongdef}                  {printf("%s\n", yytext); return IDLGEN_KULONG;}
<list>{spaces}{sge}K{stringdef}                 {printf("%s\n", yytext); return IDLGEN_KSTRING;}
<list>{spaces}{sge}I{booldef}                   {printf("%s\n", yytext); return IDLGEN_IBOOL;}
<list>{spaces}{sge}I{floatdef}                  {printf("%s\n", yytext); return IDLGEN_IFLOAT;}
<list>{spaces}{sge}I{doubledef}                 {printf("%s\n", yytext); return IDLGEN_IDOUBLE;}
<list>{spaces}{sge}I{ulongdef}                  {printf("%s\n", yytext); return IDLGEN_IULONG;}
<list>{spaces}{sge}I{longdef}                   {printf("%s\n", yytext); return IDLGEN_ILONG;}
<list>{spaces}{sge}I{chardef}                   {printf("%s\n", yytext); return IDLGEN_ICHAR;}
<list>{spaces}{sge}I{intdef}                    {printf("%s\n", yytext); return IDLGEN_IINT;}
<list>{spaces}{sge}I{stringdef}                 {printf("%s\n", yytext); return IDLGEN_ISTRING;}
<list>{spaces}{sge}IR{booldef}                  {printf("%s\n", yytext); return IDLGEN_IRBOOL;}
<list>{spaces}{sge}IR{floatdef}                 {printf("%s\n", yytext); return IDLGEN_IRFLOAT;}
<list>{spaces}{sge}IR{doubledef}                {printf("%s\n", yytext); return IDLGEN_IRDOUBLE;}
<list>{spaces}{sge}IR{ulongdef}                 {printf("%s\n", yytext); return IDLGEN_IRULONG;}
<list>{spaces}{sge}IR{longdef}                  {printf("%s\n", yytext); return IDLGEN_IRLONG;}
<list>{spaces}{sge}IR{chardef}                  {printf("%s\n", yytext); return IDLGEN_IRCHAR;}
<list>{spaces}{sge}IR{intdef}                   {printf("%s\n", yytext); return IDLGEN_IRINT;}
<list>{spaces}{sge}IR{stringdef}                {printf("%s\n", yytext); return IDLGEN_IRSTRING;}
<list>{spaces}{sge}I{listdef}                   {printf("%s\n", yytext); return IDLGEN_ILIST;}
<list>{spaces}{sge}I{objdef}                    {printf("%s\n", yytext); return IDLGEN_IOBJECT;}
<list>{spaces}{sge}IR{listdef}                  {printf("%s\n", yytext); return IDLGEN_IRLIST;}
<list>{spaces}{sge}IR{objdef}                   {printf("%s\n", yytext); return IDLGEN_IROBJECT;}
<list>{spaces}{sge}{booldef}                    {printf("%s\n", yytext); return IDLGEN_BOOL;}
<list>{spaces}{sge}{floatdef}                   {printf("%s\n", yytext); return IDLGEN_FLOAT;}
<list>{spaces}{sge}{doubledef}                  {printf("%s\n", yytext); return IDLGEN_DOUBLE;}
<list>{spaces}{sge}{ulongdef}                   {printf("%s\n", yytext); return IDLGEN_ULONG;}
<list>{spaces}{sge}{longdef}                    {printf("%s\n", yytext); return IDLGEN_LONG;}
<list>{spaces}{sge}{chardef}                    {printf("%s\n", yytext); return IDLGEN_CHAR;}
<list>{spaces}{sge}{intdef}                     {printf("%s\n", yytext); return IDLGEN_INT;}
<list>{spaces}{sge}{stringdef}                  {printf("%s\n", yytext); return IDLGEN_STRING;}
<list>{spaces}{sge}R{booldef}                   {printf("%s\n", yytext); return IDLGEN_RBOOL;}
<list>{spaces}{sge}R{floatdef}                  {printf("%s\n", yytext); return IDLGEN_RFLOAT;}
<list>{spaces}{sge}R{doubledef}                 {printf("%s\n", yytext); return IDLGEN_RDOUBLE;}
<list>{spaces}{sge}R{ulongdef}                  {printf("%s\n", yytext); return IDLGEN_RULONG;}
<list>{spaces}{sge}R{longdef}                   {printf("%s\n", yytext); return IDLGEN_RLONG;}
<list>{spaces}{sge}R{chardef}                   {printf("%s\n", yytext); return IDLGEN_RCHAR;}
<list>{spaces}{sge}R{intdef}                    {printf("%s\n", yytext); return IDLGEN_RINT;}
<list>{spaces}{sge}R{stringdef}                 {printf("%s\n", yytext); return IDLGEN_RSTRING;}
<list>{spaces}{sge}T{listdef}                   {printf("%s\n", yytext); return IDLGEN_LIST;}
<list>{spaces}{sge}R{listdef}                   {printf("%s\n", yytext); return IDLGEN_RLIST;}
<list>{spaces}{sge}R{objdef}                    {printf("%s\n", yytext); return IDLGEN_ROBJECT;}
<list>{spaces}{sge}X?{listdef}                  {printf("%s\n", yytext); return IDLGEN_XLIST;}
<list>{spaces}{sge}X{objdef}                    {printf("%s\n", yytext); return IDLGEN_XOBJECT;}
<list>{spaces}{sge}X{uppercase}*                {strcpy(yylval.name, yytext); printf("%s\n", yytext); return IDLGEN_XELEM;}
<list>\,{spaces}CULL_[_a-zA-Z0-9]*         {printf("%s\n", yytext); BEGIN attr;}
<list>[_a-zA-Z][_a-zA-Z0-9]*               {strcpy(yylval.name, yytext); printf("%s\n", yytext); return IDLGEN_IDENTIFIER;}
<list>{spaces}\({spaces}                   {printf("%s\n", yytext); return IDLGEN_OPENBRACE;}
<list>{spaces}\,{spaces}                   {printf("%s\n", yytext); return IDLGEN_COMMA;}
<list>{spaces}\)                           {printf("%s\n", yytext); return IDLGEN_CLOSEBRACE;}
<list>{spaces}{idlstart}                   {printf("%s\n", yytext); BEGIN idl;}
<list>\n                                   {yylval.other = '\n';printf("%s\n", yytext);  return IDLGEN_OTHER;}
<list>.                                    {yylval.other = yytext[0];printf("%s\n", yytext);  return IDLGEN_OTHER;}
<idl>{spaces}{idlend}\n                    {printf("%s\n", yytext); BEGIN list;}
<idl>.*\n                                  {strcpy(yylval.name, yytext); printf("%s\n", yytext); return IDLGEN_IDL;}
<attr>{spaces}\|{spaces}CULL_[_a-zA-Z0-9]* {printf("%s\n", yytext);}
<attr>{spaces}\)                           {BEGIN list; printf("%s\n", yytext); return IDLGEN_CLOSEBRACE;}
\n                                         {yylval.other = '\n'; /* printf("%s\n", yytext);*/ return IDLGEN_OTHER;}
.                                          {yylval.other = yytext[0];/* printf("%s\n", yytext);*/ return IDLGEN_OTHER;}
