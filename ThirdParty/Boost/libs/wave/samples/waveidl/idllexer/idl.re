/*=============================================================================
    Boost.Wave: A Standard compliant C++ preprocessor library

    Sample: IDL lexer 

    http://www.boost.org/

    Copyright (c) 2001-2005 Hartmut Kaiser. Distributed under the Boost
    Software License, Version 1.0. (See accompanying file
    LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <boost/config.hpp>

#if defined(BOOST_HAS_UNISTD_H)
#include <unistd.h>
#else
#include <io.h>
#endif 

#include <boost/assert.hpp>
#include <boost/detail/workaround.hpp>

// reuse the token ids and re2c helper functions from the default C++ lexer
#include <boost/wave/token_ids.hpp>
#include <boost/wave/cpplexer/re2clex/aq.hpp>
#include <boost/wave/cpplexer/re2clex/scanner.hpp>

#include "idl_re.hpp"

#if defined(_MSC_VER) && !defined(__COMO__)
#pragma warning (disable: 4101)     // 'foo' : unreferenced local variable
#pragma warning (disable: 4102)     // 'foo' : unreferenced label
#endif

#define BSIZE     196608

#define YYCTYPE   uchar
#define YYCURSOR  cursor
#define YYLIMIT   s->lim
#define YYMARKER  s->ptr
#define YYFILL(n) {cursor = fill(s, cursor);}

//#define RET(i)    {s->cur = cursor; return (i);}
#define RET(i)    \
    { \
        s->line += count_backslash_newlines(s, cursor); \
        s->cur = cursor; \
        return (i); \
    } \
    /**/

///////////////////////////////////////////////////////////////////////////////
namespace boost {
namespace wave {
namespace idllexer {
namespace re2clex {

#define RE2C_ASSERT BOOST_ASSERT

int 
get_one_char(boost::wave::cpplexer::re2clex::Scanner *s)
{
    using namespace boost::wave::cpplexer::re2clex;
    if (s->fd != -1) {
    uchar val;
    
        if (read(s->fd, &val, sizeof(val)))
            return val;
    }
    else if (0 != s->act) {
        RE2C_ASSERT(s->first != 0 && s->last != 0);
        RE2C_ASSERT(s->first <= s->act && s->act <= s->last);
        if (s->act < s->last) 
            return *(s->act)++;
    }
    return -1;
}

std::ptrdiff_t 
rewind_stream (boost::wave::cpplexer::re2clex::Scanner *s, int cnt)
{
    if (s->fd != -1) {
        return lseek(s->fd, cnt, SEEK_CUR);
    }
    else if (0 != s->act) {
        RE2C_ASSERT(s->first != 0 && s->last != 0);
        s->act += cnt;
        RE2C_ASSERT(s->first <= s->act && s->act <= s->last);
        return s->act - s->first;
    }
    return 0;
}

std::size_t 
get_first_eol_offset(boost::wave::cpplexer::re2clex::Scanner* s)
{
    if (!AQ_EMPTY(s->eol_offsets))
    {
        return s->eol_offsets->queue[s->eol_offsets->head];
    }
    else
    {
        return (unsigned int)-1;
    }
}

void 
adjust_eol_offsets(boost::wave::cpplexer::re2clex::Scanner* s, 
    std::size_t adjustment)
{
    boost::wave::cpplexer::re2clex::aq_queue q;
    std::size_t i;
    
    if (!s->eol_offsets)
        s->eol_offsets = boost::wave::cpplexer::re2clex::aq_create();

    q = s->eol_offsets;

    if (AQ_EMPTY(q))
        return;

    i = q->head;
    while (i != q->tail)
    {
        if (adjustment > q->queue[i])
            q->queue[i] = 0;
        else
            q->queue[i] -= adjustment;
        ++i;
        if (i == q->max_size)
            i = 0;
    }
    if (adjustment > q->queue[i])
        q->queue[i] = 0;
    else
        q->queue[i] -= adjustment;
}

int 
count_backslash_newlines(boost::wave::cpplexer::re2clex::Scanner *s, 
    boost::wave::cpplexer::re2clex::uchar *cursor)
{
    using namespace boost::wave::cpplexer::re2clex;

    std::size_t diff, offset;
    int skipped = 0;
    
    /* figure out how many backslash-newlines skipped over unknowingly. */
    diff = cursor - s->bot;
    offset = get_first_eol_offset(s);
    while (offset <= diff && offset != (unsigned int)-1)
    {
        skipped++;
        boost::wave::cpplexer::re2clex::aq_pop(s->eol_offsets);
        offset = get_first_eol_offset(s);
    }
    return skipped;
}

bool is_backslash(
  boost::wave::cpplexer::re2clex::uchar *p, 
  boost::wave::cpplexer::re2clex::uchar *end, int &len)
{
    if (*p == '\\') {
        len = 1;
        return true;
    }
    else if (*p == '?' && *(p+1) == '?' && (p+2 < end && *(p+2) == '/')) {
        len = 3;
        return true;
    }
    return false;
}

boost::wave::cpplexer::re2clex::uchar *
fill(boost::wave::cpplexer::re2clex::Scanner *s, 
    boost::wave::cpplexer::re2clex::uchar *cursor)
{
    using namespace std;    // some systems have memcpy etc. in namespace std
    using namespace boost::wave::cpplexer::re2clex;

    if(!s->eof)
    {
        uchar* p;
        std::ptrdiff_t cnt = s->tok - s->bot;
        if(cnt)
        {
            memcpy(s->bot, s->tok, s->lim - s->tok);
            s->tok = s->bot;
            s->ptr -= cnt;
            cursor -= cnt;
            s->lim -= cnt;
            adjust_eol_offsets(s, cnt);
        }

        if((s->top - s->lim) < BSIZE)
        {
            uchar *buf = (uchar*) malloc(((s->lim - s->bot) + BSIZE)*sizeof(uchar));
            if (buf == 0)
            {
                using namespace std;      // some systems have printf in std
                if (0 != s->error_proc)
                    (*s->error_proc)(s, "Out of memory!");
                else 
                    printf("Out of memory!\n");
                    
                /* get the scanner to stop */
                *cursor = 0;
                return cursor;
            }

            memcpy(buf, s->tok, s->lim - s->tok);
            s->tok = buf;
            s->ptr = &buf[s->ptr - s->bot];
            cursor = &buf[cursor - s->bot];
            s->lim = &buf[s->lim - s->bot];
            s->top = &s->lim[BSIZE];
            free(s->bot);
            s->bot = buf;
        }

        if (s->fd != -1) {
            if((cnt = read(s->fd, (char*) s->lim, BSIZE)) != BSIZE)
            {
                s->eof = &s->lim[cnt]; *(s->eof)++ = '\0';
            }
        }
        else if (s->act != 0) {
            cnt = s->last - s->act;
            if (cnt > BSIZE)
                cnt = BSIZE;
            memcpy(s->lim, s->act, cnt);
            s->act += cnt;
            if (cnt != BSIZE) 
            {
                s->eof = &s->lim[cnt]; *(s->eof)++ = '\0';
            }
        }
        
        /* backslash-newline erasing time */

        /* first scan for backslash-newline and erase them */
        for (p = s->lim; p < s->lim + cnt - 2; ++p)
        {
            int len = 0;
            if (is_backslash(p, s->lim + cnt, len))
            {
                if (*(p+len) == '\n')
                {
                    int offset = len + 1;
                    memmove(p, p + offset, s->lim + cnt - p - offset);
                    cnt -= offset;
                    --p;
                    aq_enqueue(s->eol_offsets, p - s->bot + 1);    
                }
                else if (*(p+len) == '\r')
                {
                    if (*(p+len+1) == '\n')
                    {
                        int offset = len + 2;
                        memmove(p, p + offset, s->lim + cnt - p - offset);
                        cnt -= offset;
                        --p;
                    }
                    else
                    {
                        int offset = len + 1;
                        memmove(p, p + offset, s->lim + cnt - p - offset);
                        cnt -= offset;
                        --p;
                    }
                    aq_enqueue(s->eol_offsets, p - s->bot + 1);    
                }
            }
        }

        /* FIXME: the following code should be fixed to recognize correctly the 
                  trigraph backslash token */

        /* check to see if what we just read ends in a backslash */
        if (cnt >= 2)
        {
            uchar last = s->lim[cnt-1];
            uchar last2 = s->lim[cnt-2];
            /* check \ EOB */
            if (last == '\\')
            {
                int next = get_one_char(s);
                /* check for \ \n or \ \r or \ \r \n straddling the border */
                if (next == '\n')
                {
                    --cnt; /* chop the final \, we've already read the \n. */
                    boost::wave::cpplexer::re2clex::aq_enqueue(s->eol_offsets, 
                        cnt + (s->lim - s->bot));    
                }
                else if (next == '\r')
                {
                    int next2 = get_one_char(s);
                    if (next2 == '\n')
                    {
                        --cnt; /* skip the backslash */
                    }
                    else
                    {
                        /* rewind one, and skip one char */
                        rewind_stream(s, -1);
                        --cnt;
                    }
                    boost::wave::cpplexer::re2clex::aq_enqueue(s->eol_offsets, 
                        cnt + (s->lim - s->bot));    
                }
                else if (next != -1) /* -1 means end of file */
                {
                    /* next was something else, so rewind the stream */
                    lseek(s->fd, -1, SEEK_CUR);
                }
            }
            /* check \ \r EOB */
            else if (last == '\r' && last2 == '\\')
            {
                int next = get_one_char(s);
                if (next == '\n')
                {
                    cnt -= 2; /* skip the \ \r */
                }
                else
                {
                    /* rewind one, and skip two chars */
                    rewind_stream(s, -1);
                    cnt -= 2;
                }
                boost::wave::cpplexer::re2clex::aq_enqueue(s->eol_offsets, 
                    cnt + (s->lim - s->bot));    
            }
            /* check \ \n EOB */
            else if (last == '\n' && last2 == '\\')
            {
                cnt -= 2;
                boost::wave::cpplexer::re2clex::aq_enqueue(s->eol_offsets, 
                    cnt + (s->lim - s->bot));    
            }
        }
        
        s->lim += cnt;
        if (s->eof) /* eof needs adjusting if we erased backslash-newlines */
        {
            s->eof = s->lim;
            *(s->eof)++ = '\0';
        }
    }
    return cursor;
}

boost::wave::token_id  
scan(boost::wave::cpplexer::re2clex::Scanner *s)
{
    using namespace boost::wave::cpplexer::re2clex;

    uchar *cursor = s->tok = s->cur;

/*!re2c
any                = [\t\v\f\r\n\040-\377];
OctalDigit         = [0-7];
Digit              = [0-9];
HexDigit           = [a-fA-F0-9];
ExponentPart       = [Ee] [+-]? Digit+;
FractionalConstant = (Digit* "." Digit+) | (Digit+ ".");
FloatingSuffix     = [fF][lL]?|[lL][fF]?;
IntegerSuffix      = [uU][lL]?|[lL][uU]?;
FixedPointSuffix   = [dD];
Backslash          = [\\]|"??/";
EscapeSequence     = Backslash ([abfnrtv?'"] | Backslash | "x" HexDigit+ | OctalDigit OctalDigit? OctalDigit?);
HexQuad            = HexDigit HexDigit HexDigit HexDigit;
UniversalChar      = Backslash ("u" HexQuad | "U" HexQuad HexQuad);
Newline            = "\r\n" | "\n" | "\r";
PPSpace            = ([ \t]|("/*"(any\[*]|Newline|("*"+(any\[*/]|Newline)))*"*"+"/"))*;
Pound              = "#" | "??=" | "%:";
*/

/*!re2c
    "/*"            { goto ccomment; }
    "//"            { goto cppcomment; }
    
    "TRUE"          { RET(T_TRUE); }
    "FALSE"         { RET(T_FALSE); }
    
    "{"             { RET(T_LEFTBRACE); }
    "}"             { RET(T_RIGHTBRACE); }
    "["             { RET(T_LEFTBRACKET); }
    "]"             { RET(T_RIGHTBRACKET); }
    "#"             { RET(T_POUND); }
    "##"            { RET(T_POUND_POUND); }
    "("             { RET(T_LEFTPAREN); }
    ")"             { RET(T_RIGHTPAREN); }
    ";"             { RET(T_SEMICOLON); }
    ":"             { RET(T_COLON); }
    "?"             { RET(T_QUESTION_MARK); }
    "."             { RET(T_DOT); }
    "+"             { RET(T_PLUS); }
    "-"             { RET(T_MINUS); }
    "*"             { RET(T_STAR); }
    "/"             { RET(T_DIVIDE); }
    "%"             { RET(T_PERCENT); }
    "^"             { RET(T_XOR); }
    "&"             { RET(T_AND); }
    "|"             { RET(T_OR); }
    "~"             { RET(T_COMPL); }
    "!"             { RET(T_NOT); }
    "="             { RET(T_ASSIGN); }
    "<"             { RET(T_LESS); }
    ">"             { RET(T_GREATER); }
    "<<"            { RET(T_SHIFTLEFT); }
    ">>"            { RET(T_SHIFTRIGHT); }
    "=="            { RET(T_EQUAL); }
    "!="            { RET(T_NOTEQUAL); }
    "<="            { RET(T_LESSEQUAL); }
    ">="            { RET(T_GREATEREQUAL); }
    "&&"            { RET(T_ANDAND); }
    "||"            { RET(T_OROR); }
    "++"            { RET(T_PLUSPLUS); }
    "--"            { RET(T_MINUSMINUS); }
    ","             { RET(T_COMMA); }

    ([a-zA-Z_] | UniversalChar) ([a-zA-Z_0-9] | UniversalChar)*        
        { RET(T_IDENTIFIER); }
    
    (("0" [xX] HexDigit+) | ("0" OctalDigit*) | ([1-9] Digit*)) IntegerSuffix?
        { RET(T_INTLIT); }

    ((FractionalConstant ExponentPart?) | (Digit+ ExponentPart)) FloatingSuffix?
        { RET(T_FLOATLIT); }

    (FractionalConstant | Digit+) FixedPointSuffix
        { RET(T_FIXEDPOINTLIT); }
        
    "L"? (['] (EscapeSequence|any\[\n\r\\']|UniversalChar)+ ['])
        { RET(T_CHARLIT); }
    
    "L"? (["] (EscapeSequence|any\[\n\r\\"]|UniversalChar)* ["])
        { RET(T_STRINGLIT); }
    

    Pound PPSpace "include" PPSpace "<" (any\[\n\r>])+ ">" 
        { RET(T_PP_HHEADER); }

    Pound PPSpace "include" PPSpace "\"" (any\[\n\r"])+ "\"" 
        { RET(T_PP_QHEADER); } 

    Pound PPSpace "include" PPSpace
        { RET(T_PP_INCLUDE); } 

    Pound PPSpace "if"        { RET(T_PP_IF); }
    Pound PPSpace "ifdef"     { RET(T_PP_IFDEF); }
    Pound PPSpace "ifndef"    { RET(T_PP_IFNDEF); }
    Pound PPSpace "else"      { RET(T_PP_ELSE); }
    Pound PPSpace "elif"      { RET(T_PP_ELIF); }
    Pound PPSpace "endif"     { RET(T_PP_ENDIF); }
    Pound PPSpace "define"    { RET(T_PP_DEFINE); }
    Pound PPSpace "undef"     { RET(T_PP_UNDEF); }
    Pound PPSpace "line"      { RET(T_PP_LINE); }
    Pound PPSpace "error"     { RET(T_PP_ERROR); }
    Pound PPSpace "pragma"    { RET(T_PP_PRAGMA); }

    Pound PPSpace "warning"   { RET(T_PP_WARNING); }
    
    [ \t\v\f]+
        { RET(T_SPACE); }

    Newline
    {
        s->line++;
        RET(T_NEWLINE);
    }

    "\000"
    {
        if(cursor != s->eof) 
        {
            using namespace std;      // some systems have printf in std
            if (0 != s->error_proc)
                (*s->error_proc)(s, "'\\000' in input stream");
            else
                printf("Error: 0 in file\n");
        }
        RET(T_EOF);
    }

    any
    {
        /* if (0 != s->error_proc)
            (*s->error_proc)(s, "Unexpected character: '%c'", *s->tok);
        else
            printf("unexpected character: '%c'\n", *s->tok);
        */
        RET(TOKEN_FROM_ID(*s->tok, UnknownTokenType));
    }
*/

ccomment:
/*!re2c
    "*/"            { RET(T_CCOMMENT); }
    Newline
    {
        /*if(cursor == s->eof) RET(T_EOF);*/
        /*s->tok = cursor; */
        s->line += count_backslash_newlines(s, cursor) +1;
        goto ccomment;
    }

    any            { goto ccomment; }

    "\000"
    {
        using namespace std;      // some systems have printf in std
        if(cursor == s->eof) 
        {
            if (s->error_proc)
                (*s->error_proc)(s, "Unterminated comment");
            else
                printf("Error: Unterminated comment\n");
        }
        else
        {
            if (s->error_proc)
                (*s->error_proc)(s, "'\\000' in input stream");
            else
                printf("Error: 0 in file");
        }
        /* adjust cursor such next call returns T_EOF */
        --YYCURSOR;
        /* the comment is unterminated, but nevertheless its a comment */
        RET(T_CCOMMENT);
    }

*/

cppcomment:
/*!re2c
    Newline
    {
        /*if(cursor == s->eof) RET(T_EOF); */
        /*s->tok = cursor; */
        s->line++;
        RET(T_CPPCOMMENT);
    }

    any            { goto cppcomment; }

    "\000"
    {
        using namespace std;      // some systems have printf in std
        if(cursor != s->eof) 
        {
            if (s->error_proc)
                (*s->error_proc)(s, "'\\000' in input stream");
            else
                printf("Error: 0 in file");
        }
        /* adjust cursor such next call returns T_EOF */
        --YYCURSOR;
        /* the comment is unterminated, but nevertheless its a comment */
        RET(T_CPPCOMMENT);
    }
*/

} /* end of scan */

#undef RE2C_ASSERT

///////////////////////////////////////////////////////////////////////////////
}   // namespace re2clex
}   // namespace idllexer
}   // namespace wave
}   // namespace boost
