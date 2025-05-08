#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdbool.h>

// OPTION + SHIFT + F to format the code

// TODO: add support for multi-line comments

enum
{
    ID,
    BREAK,
    CHAR,
    CT_INT,
    CT_REAL,
    CT_STRING,
    CT_CHAR,
    EQUAL,
    ASSIGN,
    ELSE,
    FOR,
    IF,
    RETURN,
    STRUCT,
    VOID,
    WHILE,
    COMMA,
    SEMICOLON,
    LPAR,
    RPAR,
    LBRACKET,
    RBRACKET,
    LACC,
    RACC,
    ADD,
    SUB,
    MUL,
    DIV,
    DOT,
    AND,
    OR,
    NOT,
    NOTEQ,
    LESS,
    LESSEQ,
    GREATER,
    GREATEREQ,
    SCOMMENT,
    MCOMMENT,
    END
}; // tokens codes

char *tokenNames[] = {
    "ID",
    "BREAK",
    "CHAR",
    "CT_INT",
    "CT_REAL",
    "CT_STRING",
    "CT_CHAR",
    "EQUAL",
    "ASSIGN",
    "ELSE",
    "FOR",
    "IF",
    "RETURN",
    "STRUCT",
    "VOID",
    "WHILE",
    "COMMA",
    "SEMICOLON",
    "LPAR",
    "RPAR",
    "LBRACKET",
    "RBRACKET",
    "LACC",
    "RACC",
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "DOT",
    "AND",
    "OR",
    "NOT",
    "NOTEQ",
    "LESS",
    "LESSEQ",
    "GREATER",
    "GREATEREQ",
    "SCOMMENT",
    "MCOMMENT",
    "END"
};

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

int line = 1;     // current line
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

int isDelimiter(char ch)
{
    char delimiters[30] = ",;()[]{}";
    for (int i = 0; i < strlen(delimiters); i++)
    {
        if (ch == delimiters[i])
            return 1;
    }
    return 0;
}

int isOperator(char ch)
{
    char operators[30] = ".+-*/&|!<>=";
    for (int i = 0; i < strlen(operators); i++)
    {
        if (ch == operators[i])
            return 1;
    }
    return 0;
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
            else if(ch == '\''){
                pStartCh = pCrtCh;
                pCrtCh++;
                state = 16;
            }
            else if(ch == '\"'){
                pStartCh = pCrtCh;
                pCrtCh++;
                state = 17;
            }
            else if(isDelimiter(ch))
            {
                pStartCh = pCrtCh;
                pCrtCh++;
                state = 2;
            }
            else if(isOperator(ch))
            {
                pStartCh = pCrtCh;
                pCrtCh++;
                state = 15;
            }
            else if (ch == '0')
            {
                pStartCh = pCrtCh;
                pCrtCh++;
                state = 5;
            }
            else if (isdigit(ch))
            {
                pStartCh = pCrtCh;
                // pCrtCh++;
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

        case 2: // final state ID

            // keywords
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
            else if (nCh == 4 && !memcmp(pStartCh, "else", 4))
                tk = addTk(ELSE);
            else if (nCh == 3 && !memcmp(pStartCh, "for", 3))
                tk = addTk(FOR);
            else if (nCh == 2 && !memcmp(pStartCh, "if", 2))
                tk = addTk(IF);
            else if (nCh == 6 && !memcmp(pStartCh, "return", 6))
                tk = addTk(RETURN);
            else if (nCh == 6 && !memcmp(pStartCh, "struct", 6))
                tk = addTk(STRUCT);
            else if (nCh == 4 && !memcmp(pStartCh, "void", 4))
                tk = addTk(VOID);
            else if (nCh == 5 && !memcmp(pStartCh, "while", 5))
                tk = addTk(WHILE);
            else if (nCh == 1 && !memcmp(pStartCh, ",", 1))
                tk = addTk(COMMA);
            else if (nCh == 1 && !memcmp(pStartCh, ";", 1))
                tk = addTk(SEMICOLON);
            else if (nCh == 1 && !memcmp(pStartCh, "(", 1))
                tk = addTk(LPAR);
            else if (nCh == 1 && !memcmp(pStartCh, ")", 1))
                tk = addTk(RPAR);
            else if (nCh == 1 && !memcmp(pStartCh, "[", 1))
                tk = addTk(LBRACKET);
            else if (nCh == 1 && !memcmp(pStartCh, "]", 1))
                tk = addTk(RBRACKET);
            else if (nCh == 1 && !memcmp(pStartCh, "{", 1))
                tk = addTk(LACC);
            else if (nCh == 1 && !memcmp(pStartCh, "}", 1))
                tk = addTk(RACC);
            else if (nCh == 1 && !memcmp(pStartCh, "+", 1))
                tk = addTk(ADD);
            else if (nCh == 1 && !memcmp(pStartCh, "-", 1))
                tk = addTk(SUB);
            else if (nCh == 1 && !memcmp(pStartCh, "*", 1))
                tk = addTk(MUL);
            else if (nCh == 1 && !memcmp(pStartCh, "/", 1))
                tk = addTk(DIV);
            else if (nCh == 1 && !memcmp(pStartCh, ".", 1))
                tk = addTk(DOT);
            else if (nCh == 2 && !memcmp(pStartCh, "&&", 2))
                tk = addTk(AND);
            else if (nCh == 2 && !memcmp(pStartCh, "||", 2))
                tk = addTk(OR);
            else if (nCh == 1 && !memcmp(pStartCh, "!", 1))
                tk = addTk(NOT);
            else if (nCh == 2 && !memcmp(pStartCh, "==", 2))
                tk = addTk(EQUAL);
            else if (nCh == 2 && !memcmp(pStartCh, "!=", 2))
                tk = addTk(NOTEQ);
            else if (nCh == 1 && !memcmp(pStartCh, "<", 1))
                tk = addTk(LESS);
            else if (nCh == 2 && !memcmp(pStartCh, "<=", 2))
                tk = addTk(LESSEQ);
            else if (nCh == 1 && !memcmp(pStartCh, ">", 1))
                tk = addTk(GREATER);
            else if (nCh == 2 && !memcmp(pStartCh, ">=", 2))
                tk = addTk(GREATEREQ);
            else if (nCh == 1 && !memcmp(pStartCh, "=", 1))
                tk = addTk(ASSIGN);
            // … all keywords …
            else
            { // if no keyword, then it is an ID
                tk = addTk(ID);
                tk->text = createString(pStartCh, pCrtCh);
            }
            return tk->code;

        case 3:
            if (isdigit(ch))
                pCrtCh++;
            else if (ch == '.')
            {
                pCrtCh++;
                state = 8;
            }
            else if (ch == 'e' || ch == 'E')
            {
                pCrtCh++;
                state = 10;
            }
            else
                state = 4;
            break;
        case 4: // final state for CT_INT
        {
            nCh = pCrtCh - pStartCh;
            if (nCh > 10)
                tkerr(addTk(END), "integer too large");
            tk = addTk(CT_INT);
            tk->i = strtol(pStartCh, NULL, 0);
            return tk->code;
        }
        case 5:
        {
            if (ch == 'x')
            {
                pCrtCh++;
                state = 6;
            }
            else if (ch == 'e' || ch == 'E')
            {
                pCrtCh++;
                state = 10;
            }
            else if (ch == '.')
            {
                pCrtCh++;
                state = 8;
            }
            else if (ch >= '0' && ch <= '7')
                pCrtCh++;
            else if (ch == '8' || ch == '9')
            {
                pCrtCh++;
                state = 3;
            }
            else
                state = 4;
            break;
        }
        case 6:
        {
            if (isxdigit(ch))
            {
                pCrtCh++;
                state = 7;
            }
            break;
        }
        case 7:
        {
            if (isxdigit(ch))
                pCrtCh++;
            else
                state = 4;
            break;
        }
        case 8:
        {
            if (isdigit(ch))
            {
                pCrtCh++;
                state = 9;
            }
            else
                tkerr(addTk(END), "invalid real number");
            break;
        }
        case 9:
        {
            if (isdigit(ch))
                pCrtCh++;
            else if (ch == 'e' || ch == 'E')
            {
                pCrtCh++;
                state = 10;
            }
            else
                // tkerr(addTk(END), "invalid real number");
                state = 13;
            break;
        }
        case 10:
        {
            if (ch == '+' || ch == '-')
            {
                pCrtCh++;
                state = 11;
            }
            else if (isdigit(ch))
            {
                state = 12;
            }
            else
                tkerr(addTk(END), "invalid real number");
            break;
        }
        case 11:
        {
            if (isdigit(ch))
            {
                pCrtCh++;
                state = 12;
            }
            else
                tkerr(addTk(END), "invalid real number");
            break;
        }
        case 12:
        {
            if (isdigit(ch))
                pCrtCh++;
            else
                state = 13;
            break;
        }
        case 13:
        {
            nCh = pCrtCh - pStartCh;
            tk = addTk(CT_REAL);
            tk->r = strtof(pStartCh, NULL);
            return tk->code;
        }
        case 14: // consume single-line comment
        {
            if(!strchr("\n\r\0",ch)){
                pCrtCh++; 
            }   
            else{
                // tk = addTk(SCOMMENT);
                // tk->text = createString(pStartCh, pCrtCh);
                return -1;
            }
        }
        case 15: // handle operators
        {
            if (isOperator(ch))
            {
                pCrtCh++;
            }
            if(!memcmp(pStartCh, "//", 2)){
                state = 14;
                break;
            }
            else if(!memcmp(pStartCh, "/*", 2)){
                state = 18;
                break;
            }
            state = 2;
            break;
        }
        case 16: //handle char
        {
            if(ch == '\'')
            {
                pCrtCh++;
                tk = addTk(CT_CHAR);
                tk->i = *(pStartCh+1);
                return tk->code;
            }
            else if(ch == '\\'){
                pCrtCh+=2;
            }
            else 
            {
                pCrtCh++;
            }
            break;
        }
        case 17: //handle strings
        {
            if(ch == '\"')
            {
                pCrtCh++;
                tk = addTk(CT_STRING);
                tk->text = createString(pStartCh+1, pCrtCh-1);
                return tk->code;
            }
            else if(ch == '\\'){
                pCrtCh+=2;
            }
            else
            {
                pCrtCh++;
            }
            break;
        }
        case 18:
        {
            if(ch == '*')
            {
                pCrtCh++;
                state = 19;
                break;
            }
            else if(ch == '\0')
            {
                tkerr(addTk(END), "unclosed comment");
            }
            else
            {
                pCrtCh++;
                break;
            }
        }
        case 19:
        {
            if(ch == '/')
            {
                pCrtCh++;
                // tk = addTk(MCOMMENT);
                // tk->text = createString(pStartCh, pCrtCh);
                return -1;
            }
            else if(ch == '\0')
            {
                tkerr(addTk(END), "unclosed comment");
            }
            else
            {
                pCrtCh++;
                state = 18;
                break;
            }
            break;
        }
    }
}
}

Token* lexical_analyzer(char *filename)
{
    FILE *f = fopen(filename, "r");
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
    return tk;
}

int main(int argc, char **argv)
{
    if (argc < 2)
        err("Usage: %s <filename>", argv[0]);

    Token *tk = lexical_analyzer(argv[1]);
    
    while (tk)
    {
        printf("Token: %s, Line: %d", tokenNames[tk->code], tk->line);
        if (tk->i && (tk->code == CT_INT || tk->code == CT_CHAR))
            printf(", Value: %ld\n", tk->i);
        else if (tk->r && tk->code == CT_REAL)
            printf(", Value: %lf\n", tk->r);
        else if (tk->text && (tk->code == ID || tk->code == CT_STRING || tk->code == SCOMMENT || tk->code == MCOMMENT))
            printf(", Value: %s\n", tk->text);
        else
            printf("\n");

        tk = tk->next;
    }

    // free(pCrtCh);
    return 0;
}