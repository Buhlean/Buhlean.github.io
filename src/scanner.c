
#include <stdio.h>
//#include <string.h>

enum TOKEN_TYPE{
  // Single-character tokens.
  TOKEN_LEFT_PAREN,   TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE,   TOKEN_RIGHT_BRACE,
  TOKEN_LEFT_BRACKET, TOKEN_RIGHT_BRACKET,
  TOKEN_PIPE,  TOKEN_MINUS, TOKEN_AMP, TOKEN_DOT,
  TOKEN_SLASH, TOKEN_EQUAL, TOKEN_GREATER, TOKEN_LESS,
  TOKEN_CHAR,  TOKEN_COLON, TOKEN_BACKSLASH, 
  // Keywords.
  TOKEN_DIRECTION, TOKEN_END, TOKEN_FLOWCHART,
  TOKEN_STYLE, TOKEN_SUBGRAPH, TOKEN_IDENTIFIER,
  // common
  TOKEN_ERROR, TOKEN_EOF, 
};

typedef struct{
  enum TOKEN_TYPE type;
  const char *start;
  int length;
  int line;
} Token;

void initScanner(const char *source);
Token scanToken();

struct scanner{
  const char *start;
  const char *current;
  int line;
};

struct scanner scanner;

void init_scanner(const char *source){
  scanner.start = source;
  scanner.current = source;
  scanner.line = 1;
}

static bool isAlpha(char c){
  return (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
          c == '_';
}

static bool isDigit(char c){
  return c >= '0' && c <= '9';
}

static bool isAtEnd(){
  return *scanner.current == '\0';
}

static char scanner_advance(){
  scanner.current++;
  return scanner.current[-1];
}

static char peek(){
  return *scanner.current;
}

static char peekNext(){
  if (isAtEnd()) return '\0';
  return scanner.current[1];
}

/* static bool match(char expected){
  if (isAtEnd()) return false;
  if (*scanner.current == expected){
    scanner.current++;
    return true;
  }
  return false;
} 
*/

static Token makeToken(enum TOKEN_TYPE type){
  Token token;
  token.type = type;
  token.start = scanner.start;
  token.length = (int)(scanner.current - scanner.start);
  token.line = scanner.line;
  return token;
}

static Token errorToken(const char *message){
  Token token;
  token.type = TOKEN_ERROR;
  token.start = message;
  token.length = (int)strlen(message);
  token.line = scanner.line;
  return token;
}

static void skipWhitespace(){
  for (;;){
    char c = peek();
    switch (c){
      case ' ':
      case '\r':
      case '\t':
        scanner_advance();
        break;
      case '\n':
        scanner.line++;
        scanner_advance();
        break;
      case '/':
        if (peekNext() == '/'){
          while (peek() != '\n' && !isAtEnd()) scanner_advance();
        }
        else {
          return;
        }
        break;
      case '%':
        if (peekNext() == '%'){
          while (peek() != '\n' && !isAtEnd()) scanner_advance();
        }
        else {
          return;
        }
        break;
      default:
        return;
    }
  }
}

static enum TOKEN_TYPE checkKeyword(int start, int length, const char *rest, enum TOKEN_TYPE type){
  if (scanner.current - scanner.start == start + length && memcmp(scanner.start + start, rest, length) == 0){
    return type;
  }
  return TOKEN_IDENTIFIER;
}

static enum TOKEN_TYPE identifierType(){
  switch (scanner.start[0]){
    case 'd': return checkKeyword(1, 8, "irection", TOKEN_DIRECTION);
    case 'e': return checkKeyword(1, 2, "nd",       TOKEN_END);
    case 'f': return checkKeyword(1, 8, "lowchart", TOKEN_FLOWCHART);
    case 's': 
      if (scanner.current - scanner.start > 1){
        switch (scanner.start[1]){
          case 't': return checkKeyword(2, 3, "yle",    TOKEN_STYLE);
          case 'u': return checkKeyword(2, 6, "bgraph", TOKEN_SUBGRAPH);
        }
      }
      break;
  }
  return TOKEN_IDENTIFIER;
}

static Token identifier(){
  while (isAlpha(peek()) || isDigit(peek())) scanner_advance();
  return makeToken(identifierType());
}

/* static Token string(){
  while (peek() != '"' && !isAtEnd()){
    if (peek() == '\n') scanner.line++;
    scanner_advance();
  }

  if (isAtEnd()) return errorToken("Unterminated string.");

  // '"'
  scanner_advance();
  return makeToken(TOKEN_STRING);
} */

Token scanToken(){
  skipWhitespace();
  scanner.start = scanner.current;

  if (isAtEnd()) return makeToken(TOKEN_EOF);

  char c = scanner_advance();
  if (isAlpha(c) || isDigit(c)) return identifier();
  switch (c){
    case '(': return makeToken(TOKEN_LEFT_PAREN);
    case ')': return makeToken(TOKEN_RIGHT_PAREN);
    case '{': return makeToken(TOKEN_LEFT_BRACE);
    case '}': return makeToken(TOKEN_RIGHT_BRACE);
    case '[': return makeToken(TOKEN_LEFT_BRACKET);
    case ']': return makeToken(TOKEN_RIGHT_BRACKET);
    case '|': return makeToken(TOKEN_PIPE);
    case '-': return makeToken(TOKEN_MINUS);
    case '/': return makeToken(TOKEN_SLASH);
    case '\\': return makeToken(TOKEN_BACKSLASH);
    case '&': return makeToken(TOKEN_AMP);
    case '.': return makeToken(TOKEN_DOT);
    case '=': return makeToken(TOKEN_EQUAL);
    case '<': return makeToken(TOKEN_LESS);
    case '>': return makeToken(TOKEN_GREATER);
    case ':': return makeToken(TOKEN_COLON);
    default:  return makeToken(TOKEN_CHAR);
  }
  //printf("%d = %c\n", c, c);
  return errorToken("Unexpected character");
}

static void debug_print_token(int type){
#if 0
  switch(type){
    case TOKEN_LEFT_PAREN    : fprintf(stderr, "TOKEN_LEFT_PAREN   ");  break;
    case TOKEN_RIGHT_PAREN   : fprintf(stderr, "TOKEN_RIGHT_PAREN  ");  break;
    case TOKEN_LEFT_BRACE    : fprintf(stderr, "TOKEN_LEFT_BRACE   ");  break;
    case TOKEN_RIGHT_BRACE   : fprintf(stderr, "TOKEN_RIGHT_BRACE  ");  break;
    case TOKEN_LEFT_BRACKET  : fprintf(stderr, "TOKEN_LEFT_BRACKET ");  break;
    case TOKEN_RIGHT_BRACKET : fprintf(stderr, "TOKEN_RIGHT_BRACKET");  break;
    case TOKEN_COMMA         : fprintf(stderr, "TOKEN_COMMA        ");  break;
    case TOKEN_PIPE          : fprintf(stderr, "TOKEN_PIPE         ");  break;
    case TOKEN_DOT           : fprintf(stderr, "TOKEN_DOT          ");  break;
    case TOKEN_MINUS         : fprintf(stderr, "TOKEN_MINUS        ");  break;
    case TOKEN_PLUS          : fprintf(stderr, "TOKEN_PLUS         ");  break;
    case TOKEN_AMP           : fprintf(stderr, "TOKEN_AMP          ");  break;
    case TOKEN_CHAR          : fprintf(stderr, "TOKEN_CHAR         ");  break;
    case TOKEN_SLASH         : fprintf(stderr, "TOKEN_SLASH        ");  break;
    case TOKEN_BACKSLASH     : fprintf(stderr, "TOKEN_BACKSLASH    ");  break;
    case TOKEN_STAR          : fprintf(stderr, "TOKEN_STAR         ");  break;
    case TOKEN_SEMICOLON     : fprintf(stderr, "TOKEN_SEMICOLON    ");  break;
    case TOKEN_EQUAL         : fprintf(stderr, "TOKEN_EQUAL        ");  break;
    case TOKEN_BANG          : fprintf(stderr, "TOKEN_BANG         ");  break;
    case TOKEN_GREATER       : fprintf(stderr, "TOKEN_GREATER      ");  break;
    case TOKEN_LESS          : fprintf(stderr, "TOKEN_LESS         ");  break;
    case TOKEN_COLON         : fprintf(stderr, "TOKEN_COLON        ");  break;
    case TOKEN_IDENTIFIER    : fprintf(stderr, "TOKEN_IDENTIFIER   ");  break;
    case TOKEN_STRING        : fprintf(stderr, "TOKEN_STRING       ");  break;
    case TOKEN_NUMBER        : fprintf(stderr, "TOKEN_NUMBER       ");  break;
    case TOKEN_DIRECTION     : fprintf(stderr, "TOKEN_DIRECTION    ");  break;
    case TOKEN_END           : fprintf(stderr, "TOKEN_END          ");  break;
    case TOKEN_FLOWCHART     : fprintf(stderr, "TOKEN_FLOWCHART    ");  break;
    case TOKEN_STYLE         : fprintf(stderr, "TOKEN_STYLE        ");  break;
    case TOKEN_SUBGRAPH      : fprintf(stderr, "TOKEN_SUBGRAPH     ");  break;
    case TOKEN_ERROR         : fprintf(stderr, "TOKEN_ERROR        ");  break;
    //case TOKEN_EOL           : fprintf(stderr, "TOKEN_EOL          \n");break;
    case TOKEN_EOF           : fprintf(stderr, "TOKEN_EOF          ");  break;
  }
#endif
}


