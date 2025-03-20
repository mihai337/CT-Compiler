#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

// OPTION + SHIFT + F to format the code

enum
{
    ID,
    BREAK,
    CHAR,
    CT_INT,
    CT_REAL,
    EQUAL,
    ASSIGN,
    END
}; // tokens codes

typedef struct _Token
{
    int code; // code (name)
    union
    {
        char *text; // used for ID, CT_STRING (dynamically allocated)
        long int i; // used for CT_INT, CT_CHAR
        double r;   // used for CT_REAL
    };
    int line;            // the input file line
    struct _Token *next; // link to the next token
} Token;

int line = 0;     // current line
Token *lastToken; // current character in the source code
Token *tokens;    // the list of tokens
char *pCrtCh;     // the current character in the source code

void err(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(EXIT_FAILURE);
}

void tkerr(const Token *tk, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error in line %d: ", tk->line);
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(EXIT_FAILURE);
}

#define SAFEALLOC(var, Type)                          \
    if ((var = (Type *)malloc(sizeof(Type))) == NULL) \
        err("not enough memory");

Token *addTk(int code)
{
    Token *tk;
    SAFEALLOC(tk, Token);
    tk->code = code;
    tk->line = line;
    tk->next = NULL;

    if (lastToken)
    {
        lastToken->next = tk;
    }
    else
    {
        tokens = tk;
    }

    lastToken = tk;
    return tk;
}

char *createString(const char *pStartCh, const char *pEndCh)
{
    int len = pEndCh - pStartCh;
    char *pStr;
    SAFEALLOC(pStr, char);
    memcpy(pStr, pStartCh, len);
    pStr[len] = 0;
    return pStr;
}

int getNextToken(void)
{
    int state = 0, nCh;
    char ch;
    const char *pStartCh;
    Token *tk;

    while (1)
    { // infinite loop
        ch = *pCrtCh;
        switch (state)
        {
        case 0: // transitions test for state 0
            if (isalpha(ch) || ch == '_')
            {
                pStartCh = pCrtCh; // memorizes the beginning of the ID
                pCrtCh++;          // consume the character
                state = 1;         // set the new state
            }
            else if (ch == '=')
            {
                pCrtCh++;
                state = 3;
            }
            else if (ch == ' ' || ch == '\r' || ch == '\t')
            {
                pCrtCh++; // consume the character and remains in state 0
            }
            else if (ch == '\n')
            { // handled separately in order to update the current line
                line++;
                pCrtCh++;
            }
            else if (ch == 0)
            { // the end of the input string
                addTk(END);
                return END;
            }
            else
                tkerr(addTk(END), "invalid character");
            break;

        case 1:
            if (isalnum(ch) || ch == '_')
                pCrtCh++;
            else
                state = 2;
            // Laboratory Compilation Techniques, Politehnica University Timisoara. © Aciu Razvan Mihai
            break;

        case 2:
            nCh = pCrtCh - pStartCh; // the id length
            // keywords tests
            if (nCh == 5 && !memcmp(pStartCh, "break", 5))
                tk = addTk(BREAK);
            else if (nCh == 4 && !memcmp(pStartCh, "char", 4))
                tk = addTk(CHAR);
            else if (nCh == 3 && !memcmp(pStartCh, "int", 3))
                tk = addTk(CT_INT);
            else if (nCh == 5 && !memcmp(pStartCh, "float", 5))
                tk = addTk(CT_REAL);
            // … all keywords …
            else
            { // if no keyword, then it is an ID
                tk = addTk(ID);
                tk->text = createString(pStartCh, pCrtCh);
            }
            return tk->code;

        case 3:
            if (ch == '=')
            {
                pCrtCh++;
                state = 4;
            }
            else
                state = 5;
            break;

        case 4:
            addTk(EQUAL);
            return EQUAL;
        case 5:
            addTk(ASSIGN);
            pStartCh = pCrtCh;
            state = 6;
            break;
            // return ASSIGN;

        case 6: // Number detection
            if (isdigit(ch))
            {
                pCrtCh++;
            }
            else if (ch == '.')
            {
                pCrtCh++;
                state = 7;
            }
            else
                state = 8;
            break;

        case 7: // Real number detection
            if (isdigit(ch))
                pCrtCh++;
            else
                state = 9;
            break;

        case 8:
            tk = addTk(CT_INT);
            tk->i = strtol(pStartCh, NULL, 10);
            return CT_INT;

        case 9:
            tk = addTk(CT_REAL);
            tk->r = strtod(pStartCh, NULL);
            return CT_REAL;
        }
    }
}

int main(void)
{
    FILE *f = fopen("test.c", "r");
    if (!f)
        err("Cannot open file");

    fseek(f, 0, SEEK_END);
    long fSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    pCrtCh = (char *)malloc(fSize + 1);
    if (!pCrtCh)
        err("Not enough memory");

    fread(pCrtCh, 1, fSize, f);
    pCrtCh[fSize] = '\0';
    fclose(f);

    while (getNextToken() != END)
        ;

    Token *tk = tokens;
    while (tk)
    {
        printf("Token: %d, Line: %d", tk->code, tk->line);
        if(tk->i && tk->code == CT_INT)
            printf(", Value: %ld\n", tk->i);
        else if(tk->r && tk->code == CT_REAL)
            printf(", Value: %lf\n", tk->r);
        else if(tk->text && tk->code == ID)
            printf(", Value: %s\n", tk->text);
        else
            printf("\n");

        tk = tk->next;
    }

    // free(pCrtCh);
    return 0;
}