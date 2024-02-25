#include "include/lexer.h"

#include <Pigeon/string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

static u32 err_line = 0;
static u32 err_col = 0;

#define LexerErr(msg, lexer) do {\
    err_line = lexer->line_number;\
    err_col = lexer->column;\
    lexer_free(lexer);\
    fprintf(stderr, "Error: %s. Line: %d Column: %d\n", msg, err_line, err_col);\
    exit(1);\
} while (0)

static Token token_make_string(Lexer* lexer);
static Token token_make_number(Lexer* lexer);
static Token token_make_sym(Lexer* lexer);

static void lexer_free(Lexer* lexer);
static void lexer_advance(Lexer* lexer);
static char lexer_peek_offset(Lexer* lexer, usize offset);
static char lexer_peek(Lexer* lexer);
static char lexer_consume(Lexer* lexer);
static bool lexer_bound(Lexer* lexer);
static bool lexer_match(Lexer* lexer, char expected);

Token token_new(TokenType type, String lexeme, u32 line, u32 col) {
    Token t;
    t.type = type;
    t.lexeme = lexeme;
    t.line = line;
    t.col = col;
    return t;
}

String token_type_str(TokenType type) {
    switch (type) {
        case Token_None:        return string("Token_None");
        case Token_Symbol:      return string("Token_Symbol");
        case Token_Number:      return string("Token_Number");
        case Token_String:      return string("Token_String");
        case Token_Plus:        return string("Token_Plus");
        case Token_Dash:        return string("Token_Dash");
        case Token_Star:        return string("Token_Star");
        case Token_Slash:       return string("Token_Slash");
        case Token_Tick:        return string("Token_Tick");
        case Token_Greater:     return string("Token_Greater");
        case Token_Less:        return string("Token_Less");
        case Token_Eq:          return string("Token_Eq");
        case Token_LParen:      return string("Token_LParen");
        case Token_RParen:      return string("Token_RParen");
        case Token_LessEq:      return string("Token_LessEq");
        case Token_GreaterEq:   return string("Token_GreaterEq");
        case Token_EOF:         return string("Token_EOF");
        case Token_Unexpected:  return string("Token_Unexpected");
        case TokenTypeCount:    return string("TokenTypeCount");
        default:                return string("Unreachable");
    }
}

void token_print(Token t) {
    printf("Token [Type: %s, Lexeme: %s]\n", 
           token_type_str(t.type).data, 
           t.lexeme.data);
}

void token_loc_print(Token t) {
    printf("Token [Type: %s, Lexeme: %s] %d:%d\n", 
           token_type_str(t.type).data, 
           t.lexeme.data,
           t.line,
           t.col);
}

static Token token_make_string(Lexer* lexer) {
    usize buf_len = 0;
    usize src_ptr = lexer->cursor;
    char c;
    while ((c = lexer->src.data[src_ptr++]) && c != '"') {
        if (c == '\\') {
            c = lexer->src.data[src_ptr];

            switch (c) {
                case 'n': buf_len++; src_ptr++; break;
                case 'r': buf_len++; src_ptr++; break;
                case 't': buf_len++; src_ptr++; break;
                case 'b': buf_len++; src_ptr++; break;
                case '\'': buf_len++; src_ptr++; break;
                case '"': buf_len++; src_ptr++; break;
                case '\\': buf_len++; src_ptr++; break;
                default: break;
            }
        }
        buf_len++;
    }

    String buf = string_malloc(buf_len);
    u64 buf_ptr = 0;
    while (!lexer_bound(lexer) && lexer_peek(lexer) != '"') {
        char c = lexer_peek(lexer);
        if (c == '\\') {
            lexer_advance(lexer);

            switch (lexer_consume(lexer)) {
                case 'n': buf.data[buf_ptr++] = '\\'; buf.data[buf_ptr++] = 'n'; break;
                case 't': buf.data[buf_ptr++] = '\\'; buf.data[buf_ptr++] = 't'; break;
                case 'r': buf.data[buf_ptr++] = '\\'; buf.data[buf_ptr++] = 'r'; break;
                case 'b': buf.data[buf_ptr++] = '\\'; buf.data[buf_ptr++] = 'b'; break;
                case '\'': buf.data[buf_ptr++] = '\\'; buf.data[buf_ptr++] = '\''; break;
                case '"': buf.data[buf_ptr++] = '\\'; buf.data[buf_ptr++] = '"'; break;
                case '\\': buf.data[buf_ptr++] = '\\'; break;
                default: 
                    LexerErr("Unknown Escape Char", lexer);

            }
        } else {
            buf.data[buf_ptr++] = c;
            lexer_advance(lexer);
        }
    }

    if (lexer_bound(lexer)) {
        LexerErr("Unterminated String", lexer);
    } 

    lexer_advance(lexer);

    return token_new(Token_String, buf, lexer->line_number, lexer->column);
}

static Token token_make_number(Lexer* lexer) {
    usize buf_len = 0;
    usize src_ptr = lexer->cursor-1;
    char c;
    while ((c = lexer->src.data[src_ptr++]) && 
        (isdigit(c) || c == '.')) {
        buf_len++;
    }

    String buf = string_malloc(buf_len);
    
    u64 buf_ptr = 0;
    bool dec_point = false;

    buf.data[buf_ptr++] = lexer_peek_offset(lexer, -1);

    while (!lexer_bound(lexer) &&
    (isdigit(lexer_peek(lexer)) || lexer_peek(lexer) == '.')) {
        char c = lexer_peek(lexer);
        if (c == '.') {
            if (lexer_peek_offset(lexer, 1) == '.') {
                return token_new(Token_Number, buf, lexer->line_number, lexer->column);
            }

            if (dec_point) {
                LexerErr("Multiple Decimals in Float", lexer);
            } 

            dec_point = true; 
        } 

        buf.data[buf_ptr++] = c;
        lexer_advance(lexer);
    }

    return token_new(Token_Number, buf, lexer->line_number, lexer->column);
}

static Token token_make_sym(Lexer* lexer) {
    usize buf_len = 0;
    usize src_ptr = lexer->cursor-1;
    char c;
    while ((c = lexer->src.data[src_ptr++]) && 
        (isalnum(c) || c == '_')) {
        buf_len++;
    }

    String buf = string_malloc(buf_len);
    u64 buf_ptr = 0;

    buf.data[buf_ptr++] = lexer_peek_offset(lexer, -1);

    while (!lexer_bound(lexer) &&
            (isalnum(lexer_peek(lexer)) || lexer_peek(lexer) == '_')) {
          buf.data[buf_ptr++] = lexer_consume(lexer);
    }

    buf.len = buf_ptr;

    return token_new(Token_Symbol, buf, lexer->line_number, lexer->column);
}

/*
*  Lexer Code Begins Here
*/

Lexer* lexer_new(Arena* a, String src) {
    Lexer* l = (Lexer*)arena_alloc(a, sizeof(Lexer));

    l->src = src;
    l->cursor = 0;
    l->column = 0;
    l->line_number = 1;

    l->arena = a;
    
    return l;
}

static void lexer_free(Lexer* lexer) {
    arena_free(lexer->arena);
}

static bool lexer_bound(Lexer* lexer) {
    return lexer->src.data[lexer->cursor] == '\0';
}

static char lexer_peek_offset(Lexer* lexer, usize offset) {
    return lexer->src.data[lexer->cursor+offset];
}

static char lexer_peek(Lexer* lexer) {
    return lexer_peek_offset(lexer, 0);
}

static bool lexer_match(Lexer* lexer, char expected) {
    if (lexer_bound(lexer)) return false;
    if (lexer_peek(lexer) != expected) return false;

    lexer_advance(lexer);
    return true;
}

static char lexer_consume(Lexer* lexer) {
    char c = lexer_peek(lexer);
    lexer_advance(lexer);
    return c;
}

static void lexer_advance(Lexer* lexer) {
    lexer->cursor++;
    lexer->column++;
}

Token lexer_next_token(Lexer* lexer) {
    if (lexer_bound(lexer)) {
        return token(Token_EOF, lexer->line_number, lexer->column);
    }

    char c = lexer_consume(lexer);

    if (c == '\n') {
        lexer->line_number++;
        lexer->column = 0;
    }

    if (isspace(c)) {
        return lexer_next_token(lexer);
    }
    switch (c) {
        case ';': {
            while (!lexer_bound(lexer) && lexer_peek(lexer) != '\n') {
                lexer_consume(lexer);
            }
            return lexer_next_token(lexer);
        }
        case '>': {
            if (lexer_match(lexer, '=')) {
                return token_new(Token_GreaterEq, string(">="), lexer->line_number, lexer->column);
            } 

            return token_new(Token_Greater, string(">"), lexer->line_number, lexer->column);
        }
        case '<': {
            if (lexer_match(lexer, '=')) {
                return token_new(Token_LessEq, string("<"), lexer->line_number, lexer->column);
            } 

            return token_new(Token_Less, string("<"), lexer->line_number, lexer->column);
        }
        case '/': {
            if (lexer_match(lexer, '=')) {
                return token_new(Token_Neq, string("/="), lexer->line_number, lexer->column);
            } 

            return token_new(Token_Slash, string("/"), lexer->line_number, lexer->column);
        }
        case '=': return token_new(Token_Eq, string("="), lexer->line_number, lexer->column);
        case '+': return token_new(Token_Plus, string("+"), lexer->line_number, lexer->column);
        case '-': return token_new(Token_Dash, string("-"), lexer->line_number, lexer->column);
        case '*': return token_new(Token_Star, string("*"), lexer->line_number, lexer->column);
        case '"': return token_make_string(lexer);
        case '(': return token_new(Token_LParen, string("("), lexer->line_number, lexer->column);
        case ')': return token_new(Token_RParen, string(")"), lexer->line_number, lexer->column);
        case '\'': return token_new(Token_Tick,  string("\'"), lexer->line_number, lexer->column);
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': {
            return token_make_number(lexer);
        }
        default: {
            if (isalpha(c)) {
                return token_make_sym(lexer);
            }

            printf("%c\n", c);
            LexerErr("Unexpected Token", lexer);
        }
    }

    return token(Token_Unexpected, lexer->line_number, lexer->column);
}


