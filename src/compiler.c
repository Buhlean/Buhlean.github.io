
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

struct Parser{
  Token current;
  Token previous;
  bool hadError;
};

struct Compiler{
  int scopeDepth;
  int subgraph_running_ID;
  int *subgraph_ID_stack;
};

struct Parser parser;
struct Compiler *current_compiler = NULL;

//-----------------------------------------------------------------------------

static void record_error_into_buffer(const char *error_format_string, ...){
  int buf_cap = arrcap(r->compiler_errors);
  int buf_len = arrlen(r->compiler_errors);
  int index = (buf_len > 1) ? (buf_len) : (0);
  
  if(buf_cap - buf_len < 100){
    arrsetcap(r->compiler_errors, buf_cap*2);
    buf_cap = arrcap(r->compiler_errors);
  }
  int space_remaining = buf_cap - buf_len;
  
  char *buf_pos = &(r->compiler_errors[index]);
  
  va_list args;
  va_start(args, error_format_string);
  int written = vsnprintf(buf_pos, space_remaining, error_format_string, args);
  va_end(args);
  
  arrsetlen(r->compiler_errors, buf_len + written);
}

static void error_at(Token *token, const char *message){
  record_error_into_buffer("[line %d] Error", token->line);
  if (token->type == TOKEN_EOF){
    record_error_into_buffer(" at end");
  }
  else if (token->type == TOKEN_ERROR){
    // Nothing
  }
  else {
    record_error_into_buffer(" at '%.*s'", token->length, token->start);
  }
  record_error_into_buffer(": %s\n", message);
  parser.hadError = true;
}
static void error(const char *message){
  error_at(&parser.previous, message);
}
static void error_at_current(const char *message){
  error_at(&parser.current, message);
}

static void advance(){
  parser.previous = parser.current;
  debug_print_token(parser.current.type);

  for (;;){
    parser.current = scanToken();
    if (parser.current.type != TOKEN_ERROR) break;

    error_at_current(parser.current.start);
  }
}
static void consume(enum TOKEN_TYPE type, const char *message){
  if (parser.current.type == type){
    advance();
    return;
  }
  error_at_current(message);
}
static bool check(enum TOKEN_TYPE type){
  return parser.current.type == type;
}
static bool match(enum TOKEN_TYPE type){
  if (!check(type)) return false;
  advance();
  return true;
}

static void init_compiler(struct Compiler *compiler){
  compiler->subgraph_ID_stack = NULL;
  arrsetcap(compiler->subgraph_ID_stack, 16);
  compiler->scopeDepth = 0;
  compiler->subgraph_running_ID = 0;
  current_compiler = compiler;
}

static void begin_scope(){
  current_compiler->scopeDepth++;
  current_compiler->subgraph_running_ID++;
  arrput(current_compiler->subgraph_ID_stack, current_compiler->subgraph_running_ID);
}
static void end_scope(){
  current_compiler->scopeDepth--;
  if(arrpop(current_compiler->subgraph_ID_stack) < 0)
    ;
}

static void declaration();

static char *handle_node_text(){
  char *new_name_ptr = NULL;
  if(check(TOKEN_RIGHT_BRACE) || check(TOKEN_RIGHT_BRACKET) || check(TOKEN_RIGHT_PAREN) || check(TOKEN_EOF)/*  || check(TOKEN_DOT) */) return NULL;
  
  // do it once, but differently
  if(check(TOKEN_BACKSLASH)){
    advance(); // ignore the char, but bypass the "closing brackets check" of the loop (escaping)
    if(check(TOKEN_EOF)) return NULL;
  }
  else if(!check(TOKEN_EOF)){
    advance();
    new_name_ptr = arraddnptr(r->keys, parser.previous.length);
    for(int i = 0; i < parser.previous.length; i++){
      new_name_ptr[i] = parser.previous.start[i];
    }
    arrput(r->keys, '\0');
  }
  else return NULL;
  
  while(!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_RIGHT_BRACKET) && !check(TOKEN_RIGHT_PAREN) && !check(TOKEN_EOF)/*  && !check(TOKEN_DOT) */){
    if(check(TOKEN_BACKSLASH)){
      advance(); // ignore the char, but bypass the "closing brackets check" of the loop (escaping)
      if(check(TOKEN_EOF)) return NULL;
    }
    advance();
    
    int len_keys = arrlen(r->keys);
    if(len_keys > 0) {
      r->keys[len_keys - 1] = ' ';
    }
    char *name_ptr = arraddnptr(r->keys, parser.previous.length);
    for(int i = 0; i < parser.previous.length; i++){
      name_ptr[i] = parser.previous.start[i];
    }
    arrput(r->keys, '\0');
  }
  return new_name_ptr;
}

static void direction(int *insert_direction_here){
  if(parser.current.length == 2){
    if(parser.current.start[0] == 'T'){
      if(parser.current.start[1] == 'B' || parser.current.start[1] == 'D'){
        *insert_direction_here = DIR_TB;
        advance();
        return;
      }
    }
    if(parser.current.start[0] == 'B'){
      if(parser.current.start[1] == 'T'){
        *insert_direction_here = DIR_BT;
        advance();
        return;
      }
    }
    if(parser.current.start[0] == 'R'){
      if(parser.current.start[1] == 'L'){
        *insert_direction_here = DIR_RL;
        advance();
        return;
      }
    }
    if(parser.current.start[0] == 'L'){
      if(parser.current.start[1] == 'R'){
        *insert_direction_here = DIR_LR;
        advance();
        return;
      }
    }
  }
  error_at_current("Couldn't find valid direction. Choose one of: TB/TD, BT, RL or LR.");
}
static void subgraph(){
  { // name of subgraph
    consume(TOKEN_IDENTIFIER, "Please name the subgraph like this: 'subgraph name'.");
    char *new_key_ptr = arraddnptr(r->keys, parser.previous.length);
    {
      for(int i = 0; i < parser.previous.length; i++){
        new_key_ptr[i] = parser.previous.start[i];
      }
    }
    arrput(r->keys, '\0');

    int index = shgeti(r->bubble_hm, new_key_ptr);
    if(index != -1){
      if(r->bubbles[index].is_subgraph_plus_direction != 0){
        error("Cannot name two subgraphs the same.");
      }
      else{
        error("The name of a subgraph cannot be shared with a node.");
      }
      return;
    }
    else{
      // put it in the list, then remember the index and associate it with the ID/key/name
      struct Bubble b = { 0 };
      b.key = new_key_ptr;
      b.depth = current_compiler->scopeDepth;
      b.subgraph_entry_index = current_compiler->subgraph_ID_stack[arrlen(current_compiler->subgraph_ID_stack)-1];
      b.is_subgraph_plus_direction = 1;
      
      int where = arrlen(r->bubbles);
      arrput(r->bubbles, b);
      shput(r->bubble_hm, new_key_ptr, where);
    }
  }

  consume(TOKEN_DIRECTION, "Every subgraph needs a 'direction' first. Choose one of: TB/TD, BT, RL or LR.");
  direction( (&(r->bubbles[arrlen(r->bubbles)-1].is_subgraph_plus_direction)) );

  while (!check(TOKEN_END) && !check(TOKEN_EOF)){
    declaration();
  }
  consume(TOKEN_END, "Every 'subgraph' needs an 'end'.");
}
static void style(){
  advance();
}
static void node_plus_maybe_connections(){
  mymemzero(r->conn_list_A, arrlen(r->conn_list_A)*sizeof(*(r->conn_list_A)));
  mymemzero(r->conn_list_B, arrlen(r->conn_list_B)*sizeof(*(r->conn_list_B)));
  arrsetlen(r->conn_list_A, 0);
  arrsetlen(r->conn_list_B, 0);
  // These will be switched at every arrow encountered (there can be many arrows per line)
  // after all permutations got a connection added, then b set to zero.
  // We start with the first group being the targets with 0 sources
  int *list_B = r->conn_list_A;
  int *list_A = r->conn_list_B;
  
  int conn_type_current = 0;

  do{
    { // name of node

      // Find the bubbles id and make it into a CString (copy it out of the markup stream)
      consume(TOKEN_IDENTIFIER, "Expected a node name here.");
      char *new_key_ptr = arraddnptr(r->keys, parser.previous.length);
      for(int i = 0; i < parser.previous.length; i++){
        new_key_ptr[i] = parser.previous.start[i];
      }
      arrput(r->keys, '\0');
      
      char *new_text_ptr = NULL;
      int bubble_type = BUBBLE_STANDARD;
      { // bubble shape + text
        if(match(TOKEN_LEFT_PAREN)){
          
          if(match(TOKEN_LEFT_BRACKET)){
            bubble_type = BUBBLE_STADIUM;
            while (!check(TOKEN_RIGHT_BRACKET)){
              new_text_ptr= handle_node_text();
            }
            consume(TOKEN_RIGHT_BRACKET, "Expected matching closing '])'.");
            consume(TOKEN_RIGHT_PAREN,   "Expected matching closing '])'.");
          }
          else
          if(match(TOKEN_LEFT_PAREN)){
            bubble_type = BUBBLE_CIRCLE;
            while (!check(TOKEN_RIGHT_PAREN)){
              new_text_ptr= handle_node_text();
            }
            consume(TOKEN_RIGHT_PAREN,   "Expected matching closing '))'.");
            consume(TOKEN_RIGHT_PAREN,   "Expected matching closing '))'.");
          }
          else{
            bubble_type = BUBBLE_STANDARD;
            while (!check(TOKEN_RIGHT_PAREN)){
              new_text_ptr= handle_node_text();
            }
            consume(TOKEN_RIGHT_PAREN,   "Expected matching closing ')'.");
          }
          
        }
        else
        if(match(TOKEN_LEFT_BRACKET)){
          
          if(match(TOKEN_LEFT_BRACKET)){
            bubble_type = BUBBLE_SUBROUTINE;
            while (!check(TOKEN_RIGHT_BRACKET)){
              new_text_ptr= handle_node_text();
            }
            consume(TOKEN_RIGHT_BRACKET, "Expected matching closing ']]'.");
            consume(TOKEN_RIGHT_BRACKET, "Expected matching closing ']]'.");
          }
          else
          if(match(TOKEN_LEFT_PAREN)){
            bubble_type = BUBBLE_CYLINDER;
            while (!check(TOKEN_RIGHT_PAREN)){
              new_text_ptr= handle_node_text();
            }
            consume(TOKEN_RIGHT_PAREN,   "Expected matching closing ')]'.");
            consume(TOKEN_RIGHT_BRACKET, "Expected matching closing ')]'.");
          }
          else{
            bubble_type = BUBBLE_RECTANGLE;
            while (!check(TOKEN_RIGHT_BRACKET)){
              new_text_ptr= handle_node_text();
            }
            consume(TOKEN_RIGHT_BRACKET, "Expected matching closing ']'.");
          }
          
        }
        else
        if(match(TOKEN_LEFT_BRACE)){
          
          if(match(TOKEN_LEFT_BRACE)){
            bubble_type = BUBBLE_HEXAGON;
            while (!check(TOKEN_RIGHT_BRACE)){
              new_text_ptr= handle_node_text();
            }
            consume(TOKEN_RIGHT_BRACE,   "Expected matching closing '}}'.");
            consume(TOKEN_RIGHT_BRACE,   "Expected matching closing '}}'.");
          }
          else{
            bubble_type = BUBBLE_RHOMBUS;
            while (!check(TOKEN_RIGHT_BRACE)){
              new_text_ptr= handle_node_text();
            }
            consume(TOKEN_RIGHT_BRACE,   "Expected matching closing '}'.");
          }
          
        }
        // no default, there is no shape
      }
      // Store the finished Bubble in the list
      int index = shgeti(r->bubble_hm, new_key_ptr);
      int where = 0;
      if(index != -1){
        if(new_text_ptr != NULL){
          if(r->bubbles[index].text == NULL){
            r->bubbles[index].text = new_text_ptr;
          }
          else{
            error("Can't assign text to the same node twice; pick one, please.");
          }
        }
        if(bubble_type != BUBBLE_STANDARD){
          if(r->bubbles[index].bubble_type == BUBBLE_STANDARD){
            r->bubbles[index].bubble_type = bubble_type;
          }
          else{
            error("Can't assign a shape to the same node twice; pick one, please.");
          }
        }
        
        where = shget(r->bubble_hm, new_key_ptr);
      }
      else{
        // put it in the list, then remember the index and associate it with the ID/key/name
        struct Bubble b = {0};
        b.key = new_key_ptr;
        b.text = new_text_ptr;
        b.depth = current_compiler->scopeDepth;
        b.subgraph_entry_index = current_compiler->subgraph_ID_stack[arrlen(current_compiler->subgraph_ID_stack)-1];
        b.bubble_type = bubble_type;
        
        arrput(r->bubbles, b);
        where = arrlen(r->bubbles)-1;
        shput(r->bubble_hm, new_key_ptr, where);
      }
      arrput(list_B, where);
      
    } // name of node - scope end
    
    if(match(TOKEN_AMP)){
      // do nothing here, let it loop and add it to the list
    }
    else {

      if(arrlen(list_A) != 0 && arrlen(list_B) != 0){
        for(int a = 0; a < arrlen(list_A); a++){
          for(int b = 0; b < arrlen(list_B); b++){
            int index_a = list_A[a];
            int index_b = list_B[b];
            struct Connection c = {index_a, index_b, conn_type_current, NULL, 0};
            arrput(r->connections, c);
            r->bubbles[index_a].outgoing_count++;
            r->bubbles[index_b].incoming_count++;
          }
        }
      }
      {
        int *help = list_A;
        list_A = list_B;
        list_B = help;
        mymemzero(list_B, arrlen(list_B)*sizeof(*list_B));
        arrsetlen(list_B, 0);
      }

      { // connection types
        conn_type_current = CT_NULL;
        if(match(TOKEN_MINUS)){
          if(match(TOKEN_MINUS)){
            if(match(TOKEN_MINUS)){
              conn_type_current = CT_LINE;
            }
            else
            if(match(TOKEN_GREATER)){
              conn_type_current = CT_ARROW;
            }
            else error("Connection type not recognized.");
          }
          else
          if(match(TOKEN_DOT)){
            if(match(TOKEN_MINUS)){
              if(match(TOKEN_GREATER)){
                conn_type_current = CT_ARROW_DOTTED;
              }
              else{
                conn_type_current = CT_LINE_DOTTED;
              }
            }
            else error("Connection type not recognized.");
          }
          else error("Connection type not recognized.");
        }
        else
        if(match(TOKEN_LESS)){
          if(match(TOKEN_MINUS)){
            if(match(TOKEN_GREATER)){
              conn_type_current = CT_BIDIR;
            }
            else error("Connection type not recognized.");
          }
          else
          if(match(TOKEN_EQUAL)){
            if(match(TOKEN_GREATER)){
              conn_type_current = CT_BIDIR_THICK;
            }
            else error("Connection type not recognized.");
          }
          else error("Connection type not recognized.");
        }
        else
        if(match(TOKEN_EQUAL)){
          if(match(TOKEN_EQUAL)){
            if(match(TOKEN_EQUAL)){
              conn_type_current = CT_LINE_THICK;
            }
            else
            if(match(TOKEN_GREATER)){
              conn_type_current = CT_ARROW_THICK;
            }
            else error("Connection type not recognized.");
          }
          else error("Connection type not recognized.");
        }
        else
        if(match(TOKEN_COLON)){
          if(match(TOKEN_COLON)){
            if(match(TOKEN_COLON)){
              advance(); // TODO: classes
            }
            else error("Expected ':::CLASS_NAME'?");
          }
          else error("Expected ':::CLASS_NAME'?");
        }
        else break;
      }
    }
  }while(true);
}

static void declaration(){
  if (check(TOKEN_IDENTIFIER)){
    node_plus_maybe_connections();
  }
  else if (match(TOKEN_STYLE)){
    style();
  }
  else if (match(TOKEN_SUBGRAPH)){
    begin_scope();
    subgraph();
    end_scope();
  }
  else{
    advance();
    error("Expected a node, a subgraph or a style here.");
  }
}

static bool core_compile(const char *source){
  init_scanner(source);
  struct Compiler compiler;
  init_compiler(&compiler);
  parser.hadError = false;

  advance(); // current == first token
  
  consume(TOKEN_FLOWCHART, "Flowchart description must start with 'flowchart'.");
  if(!parser.hadError){
    {
      struct Bubble b = (struct Bubble){ 0 };
      arrput(r->bubbles, b);
      Assert(arrlen(r->bubbles) == 1);
    }
    direction( (&(r->bubbles[0].is_subgraph_plus_direction)) );
    if(!parser.hadError){
      
      while (!match(TOKEN_EOF)){
        declaration();
        if(parser.hadError) break;
      }
      
    }
  }
  arrfree(compiler.subgraph_ID_stack);
  
  return parser.hadError;
}
static bool compile(const char *source){
  state->compile_cd = 0.f;
  
  arrsetlen(r->keys, 0);
  mymemzero(r->keys, 1024*4*sizeof(*(r->keys)));
  arrsetlen(r->bubbles, 0);
  mymemzero(r->bubbles, 256*sizeof(*(r->bubbles)));
  arrsetlen(r->connections, 0);
  mymemzero(r->connections, 512*sizeof(*(r->connections)));
  arrsetlen(r->compiler_errors, 0);
  shfree(r->bubble_hm);
  
  core_compile(source);
  
  r->subgraph_count = current_compiler->subgraph_running_ID;

  SaveFileText("stored_text.txt", textbox.text);

  if(parser.hadError) arrsetlen(r->bubbles, 0); // used as proxy for existing compiler errors
  
  int bubbles_length = arrlen(r->bubbles);
  for(int i = 0; i < bubbles_length; i++){
    
  }

  return parser.hadError;
}



