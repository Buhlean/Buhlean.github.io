
#if defined(PLATFORM_WEB)
  #include "./raylib_web/raylib.h"
  #include "./raylib_web/raymath.h"
  #include "./raylib_web/rlgl.h"
#else
  #include "./raylib/raylib.h"
  #include "./raylib/raymath.h"
  #include "./raylib/rlgl.h"
#endif

#define STB_DS_IMPLEMENTATION
#include "../../libs/stb_ds.h"

#include <stdlib.h> // exit
#include <string.h> // memcpy

// taken from rcore.h
#ifndef MAX_FILEPATH_LENGTH
    #if defined(__linux__)
        #define MAX_FILEPATH_LENGTH     4096        // Maximum length for filepaths (Linux PATH_MAX default value)
    #else
        #define MAX_FILEPATH_LENGTH      512        // Maximum length supported for filepaths
    #endif
#endif

#if defined(PLATFORM_WEB)
  #define Assert(c) 
  #define AssertS(c,s) 
  #define AssertSI(c,s,i) 

#else
  #define Assert(c) if(!(c)){TraceLog(LOG_WARNING,"%s,%d",__FILE__,__LINE__);exit(24);}
  #define AssertS(c,s) if(!(c)){TraceLog(LOG_WARNING,"%s,%d: %s",__FILE__,__LINE__,s);exit(25);}
  #define AssertSI(c,s,i) if(!(c)){TraceLog(LOG_WARNING,"%s,%d",__FILE__,__LINE__);TraceLog(LOG_WARNING,s,i);exit(26);}
#endif

#include "header.c"
#include "functions.c"

#include "scanner.c"
#include "compiler.c"
#include "renderer.c"

//#define PLATFORM_WEB
#if defined(PLATFORM_WEB)
  #include <emscripten/emscripten.h>
#endif

/* TODO: (20220617)
  
  COPMPILER:
    store tab-width in Token and compare with 'end' for better errors
  RENDER:                                 
    screenshot:                           
      only graph                          CHECK 
    mode:                                 
      light                               CHECK
      dark                                CHECK
    directions:                           
      TB/BT                               CHECK
      LR/RL                               CHECK 
      CE/EC                               -
    shapes:                               
      BUBBLE_STANDARD,                    
      BUBBLE_RECTANGLE,   // [___]        
      BUBBLE_ROUNDED,                     
      BUBBLE_TRAPEZOID,   // /___\ .      
      BUBBLE_TRAPEZOID_R, // \___/ .      
      BUBBLE_FSLANTED,    // /___/ .      
      BUBBLE_BSLANTED,    // \___\ .      
      BUBBLE_CIRCLE,                      CHECK
      BUBBLE_CYLINDER,    //database      
      BUBBLE_SUBROUTINE,                  
      BUBBLE_STADIUM,     // (___)        
      BUBBLE_HEXAGON,                     
      BUBBLE_RHOMBUS,                     
    connections:                          
      CT_LINE_THICK,                      
      CT_LINE_DOTTED,                     
      CT_ARROW,                           
      CT_ARROW_THICK,                     
      CT_ARROW_DOTTED,                    
      CT_BIDIR,                           
      CT_BIDIR_THICK,                     
  
*/


//-----------------------------------------------------------------------------
//--MAIN-----------------------------------------------------------------------
//-----------------------------------------------------------------------------

int main(){
  
  g     = malloc(1 * sizeof(*g));
  if(!g)     exit(1);
  mymemzero(g,       sizeof(*g));
    
  g->width  = (int)(STARTING_SCREENWIDTH);
  g->height = (int)(STARTING_SCREENHEIGHT);
  
  //---------------------------------------------------------------------------

#if defined(PLATFORM_WEB)
  SetTraceLogLevel(LOG_NONE);
#else
  SetTraceLogLevel(LOG_ALL);
#endif


  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(g->width, g->height, "Flowgraph");
  
  
#if defined(PLATFORM_WEB)
  
#else
	SetTargetFPS(FPS_GOAL);
#endif
  
  
	font_mono = LoadFontEx("./resources/CourierNewBold.ttf", 16, NULL, 0);
  font = LoadFontEx("./resources/CourierNewBold.ttf", 16, NULL, 0);
  
  BeginDrawing();
    ClearBackground(GRAY);
    char* text = "Loading...";
    float font_size=16, spacing=2;
    float size = MeasureTextEx(font_mono, text, font_size, spacing).x;
    DrawTextEx(font, text, (Vector2){g->width/2 - (size/2), g->height/2}, font_size, spacing, WHITE);
  EndDrawing();
  
  SetWindowState(FLAG_WINDOW_MAXIMIZED);
  
  //---------------------------------------------------------------------------
  
  state = malloc(1 * sizeof(*state));
  if(!state) exit(1);
  mymemzero(state,   sizeof(*state));
  
  r     = malloc(1 * sizeof(*r));
  if(!r)     exit(1);
  mymemzero(r,       sizeof(*r));
  
  stbds_rand_seed(24); // TODO?
  
  g->border      = 4;
#if defined(PLATFORM_WEB)
  g->menu_height = 0;
  g->tab_height  = 0;
#else
  g->menu_height = 20;
  g->tab_height  = 24;
#endif
  g->padding     = 4;
  
  g->width  = GetScreenWidth();
  g->height = GetScreenHeight();
  
  g->font_size_menu = 14;
  g->font_size_menu_entry = 14;
  g->font_size_tab_label = 14;
  g->font_size_layer_label = 14;
  g->font_size_dialog_heading = 14;
  g->font_size_dialog = 14;
  
  state->hover = -1;
  state->clicked = -1;
  state->dark_mode = 1;
  palette = dark_mode;
  
  {
    int effective_height = g->height - g->menu_height - g->tab_height;
    textbox.collider = (Rectangle){6, 6 + g->menu_height + g->tab_height, g->width/3, effective_height - 80};
    minus_padding(textbox.collider, 6);
  }
#if 0
  {
    Image icons_im = LoadImage("./resources/Icons.png");
    g->icons = LoadTextureFromImage(icons_im);
    UnloadImage(icons_im);
  }
  g->icon_atlas = (struct TextureAtlas){24, 24, 6, 6};
#endif
  
  menu_top_init();
  menu_rightclick_init();
  
  g->filenames = NULL;
  arrsetcap(g->filenames, 32);
  g->drop_counter = 0;
  g->last_dropped_files = 0;
  g->last_dropped_count = 0;
  
  {
    r->keys = NULL;
    r->bubbles = NULL;
    r->bubble_hm = NULL;
    r->connections = NULL;
    r->conn_list_A = NULL; 
    r->conn_list_B = NULL;
    r->render_output = LoadRenderTexture(RENDERTEXTURE_WIDTH, RENDERTEXTURE_HEIGHT);
    SetTextureWrap(r->render_output.texture, TEXTURE_WRAP_CLAMP);
    r->compiler_errors = NULL;
    r->remember_graph_info.positions = NULL;
    
    arrsetcap(r->keys, 1024*4);
    mymemzero(r->keys, 1024*4*sizeof(*(r->keys)));
    arrsetcap(r->bubbles, 256);
    mymemzero(r->bubbles, 256*sizeof(*(r->bubbles)));
    arrsetcap(r->connections, 512);
    mymemzero(r->connections, 512*sizeof(*(r->connections)));
    arrsetcap(r->conn_list_A, 16);
    mymemzero(r->conn_list_A, 16*sizeof(*(r->conn_list_A)));
    arrsetcap(r->conn_list_B, 16);
    mymemzero(r->conn_list_B, 16*sizeof(*(r->conn_list_B)));
    arrsetcap(r->compiler_errors, 1000*4);
    mymemzero(r->compiler_errors, 1000*4*sizeof(*(r->compiler_errors)));
    arrsetcap(r->remember_graph_info.positions, 256);
    mymemzero(r->remember_graph_info.positions, 256*sizeof(*(r->remember_graph_info.positions)));
    
    //GenTextureMipmaps(&(r->render_output.texture));
    
    textbox.text = NULL;
    arrsetcap(textbox.text, 1024*16);
    mymemzero(textbox.text, 1024*16);
    
    // debug
    
    char *text = NULL;
    if(FileExists("stored_text.txt")){
      text = LoadFileText("stored_text.txt");
    }
    else{
      text = " \
%% Made by Alexander Buhl.\n\
%%\n\
%% Written entirely in C.\n\
%% This web version exists purely for demoing the program \n\
%% on a desktop browser.\n\
%% Textbox, compiler and graph layout are entirely handmade.\n\
%% Platform and graphics abstraction: raylib (raylib.com)\n\
%%\n\
%% Syntax follows Mermaid's Flowchart syntax closely.\n\
%% Graph responds to changes in the text automatically.\n\
%% Compiler errors guide to a valid graph.\n\
\n\
flowchart LR\n\
  TOP(Root) --- Tasks & program(The Program) & Manage\n\
  Tasks   --- 1 & 2\n\
  program --- Cluster \n\
  Cluster --- C(()) & D(()) & E(())\n\
  Cluster --- F(()) & G(()) & H(())\n\
  Manage  <-> Dep_1(Testing)\n\
  Constraint --- program\n\
  Isolate\n\
  DyadA(Dyad A) --- DyadB(Dyad B)\n\
  \n\
  ";
    }
  /*
  %% classDef RED fill:#f00 \n\
  %% A:::RED --- B:::RED \n\
  %% OR \n\
  %% RED: A, B \n\
  %% A --- B \n\
  %% OR \n\
  %% RED: A --- B \n\
  %%  \n\
  %%  \n\
  %%  \n\
  */

    int textlength = TextLength(text);
    for(int i = 0; i < textlength; i++){
      arrput(textbox.text, text[i]);
    }
    
    arrput(textbox.text, '\n');
    arrput(textbox.text, '\0');
    
    textbox.text_hash = stbds_hash_string(textbox.text, 24);
    
    // TODO: fix weird layout bug at cursor == 0
    textbox.caret_anchor = 1;
    textbox.caret_current = 1;
    
    // int len = arrlen(textbox.text);
    // TraceLog(LOG_DEBUG, "%d, %d, %d", textbox.text[len-2], textbox.text[len-1], textbox.text[len]);
  }
  
  if(!compile(textbox.text)){
    r_produce_satisfactory_output();
  }
  else{
    r_draw_errors();
  }
  
  g->width = GetScreenWidth();
  g->height = GetScreenHeight();
  
  //---------------------------------------------------------------------------

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDraw, 0, 1);
#else
  SetTargetFPS(FPS_GOAL);
  
  while (!WindowShouldClose()){
    UpdateDraw();
  }
#endif
  
  CloseWindow();
  return 0;
}

//-----------------------------------------------------------------------------

static void UpdateDraw(){

  if(IsWindowResized()) {
    int new_w = GetScreenWidth();
    int new_h = GetScreenHeight();
    g->width  = new_w;
    g->height = new_h;
    {
      int effective_height = new_h - g->menu_height - g->tab_height;
      textbox.collider = (Rectangle){6, g->menu_height + g->tab_height, new_w/3, effective_height - 80};
      minus_padding(textbox.collider, 6);
    }
    state->compile_cd = .4f;
  }
  
  /* if (IsFileDropped()){
    
    Assert(g->drop_counter < 128);
    
    g->last_dropped_files = GetDroppedFiles(&(g->last_dropped_count));
    memcpy(&(g->filenames[g->drop_counter * MAX_FILEPATH_LENGTH]), (g->last_dropped_files[0]), MAX_FILEPATH_LENGTH); // only copy over the first filename dropped
    
    if(CheckCollisionPointRec(GetMousePosition(), textbox.collider)){
      g->drop_counter++;
    }
    
    ClearDroppedFiles();
    g->last_dropped_files = 0;
  } */
  
  /* if(IsKeyDown(KEY_LEFT_ALT)){
    if(IsKeyDown(KEY_Q)){
      debug_switch = 1;
    }
  } */

  if(IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)){
    if(IsKeyPressed(KEY_S)){
      state->dark_mode = !(state->dark_mode);
      palette = ((state->dark_mode) ? (dark_mode) : (light_mode));
      r_draw_texture();
    }
  }
  
#define INIT_KEY_HOLD_STATE(key)      state->last_pressed_key_for_hold_action = key; \
                                      state->last_key_pressed_hp_time = GetTime(); \
                                      state->last_key_hold_action_acc = 0
#define TICK_AND_ASK_HOLD_STATE(key)  bool do_stuff = 0; \
                                      if((state->last_pressed_key_for_hold_action == key) \
                                      && (GetTime() - state->last_key_pressed_hp_time > .5)){ \
                                        state->last_key_hold_action_acc += GetFrameTime(); \
                                        if(state->last_key_hold_action_acc > .050){ \
                                          state->last_key_hold_action_acc -= .050; \
                                          do_stuff = 1; }}
  
  if(IsKeyDown(KEY_LEFT)){
    if(IsKeyPressed(KEY_LEFT)){
      INIT_KEY_HOLD_STATE(KEY_LEFT);
      if(textbox.caret_current > 1){ // TODO: fix weird layout bug at cursor == 0
        if(!(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))){
          if(textbox.caret_anchor != textbox.caret_current){
            textbox.caret_anchor = textbox.caret_current;
          }
          textbox.caret_anchor  -= 1;
        }
        textbox.caret_current -= 1;
      }
    }
    else{
      TICK_AND_ASK_HOLD_STATE(KEY_LEFT)
      if(do_stuff){
        if(textbox.caret_current > 1){ // TODO: fix weird layout bug at cursor == 0
          if(!(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))){
            if(textbox.caret_anchor != textbox.caret_current){
              textbox.caret_anchor = textbox.caret_current;
            }
            textbox.caret_anchor  -= 1;
          }
          textbox.caret_current -= 1;
        }
      }
    }
  }
  if(IsKeyDown(KEY_RIGHT)){
    if(IsKeyPressed(KEY_RIGHT)){
      INIT_KEY_HOLD_STATE(KEY_RIGHT);
      if(textbox.caret_current < arrlen(textbox.text) - 2){
        if(!(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))){
          if(textbox.caret_anchor != textbox.caret_current){
            textbox.caret_anchor = textbox.caret_current;
          }
          textbox.caret_anchor  += 1;
        }
        textbox.caret_current += 1;
      }
    }
    else{
      TICK_AND_ASK_HOLD_STATE(KEY_RIGHT)
      if(do_stuff){
        if(textbox.caret_current < arrlen(textbox.text) - 2){
          if(!(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))){
            if(textbox.caret_anchor != textbox.caret_current){
              textbox.caret_anchor = textbox.caret_current;
            }
            textbox.caret_anchor  += 1;
          }
          textbox.caret_current += 1;
        }
      }
    }
  }
  if(IsKeyDown(KEY_UP)){
    INIT_KEY_HOLD_STATE(KEY_UP);
  }
  if(IsKeyDown(KEY_DOWN)){
    INIT_KEY_HOLD_STATE(KEY_DOWN);
  }
  
  if(IsKeyDown(KEY_DELETE)){
    if(IsKeyPressed(KEY_DELETE)){
      INIT_KEY_HOLD_STATE(KEY_DELETE);
      if(textbox.caret_anchor != textbox.caret_current){
        textbox_delete_selection();
      }
      else{
        if(textbox.caret_current < arrlen(textbox.text) - 2){
          arrdel(textbox.text, textbox.caret_current);
        }
      }
    }
    else{
      TICK_AND_ASK_HOLD_STATE(KEY_DELETE)
      if(do_stuff){
        if(textbox.caret_current < arrlen(textbox.text) - 2){
          arrdel(textbox.text, textbox.caret_current);
        }
      }
    }
  }
  if(IsKeyDown(KEY_BACKSPACE)){
    if(IsKeyPressed(KEY_BACKSPACE)){
      INIT_KEY_HOLD_STATE(KEY_BACKSPACE);
      if(textbox.caret_anchor != textbox.caret_current){
        textbox_delete_selection();
      }
      else{
        if(textbox.caret_current > 1){ // TODO: fix weird layout bug at cursor == 0
          arrdel(textbox.text, textbox.caret_current-1);
          textbox.caret_anchor  -= 1;
          textbox.caret_current -= 1;
        }
      }
    }
    else{
      TICK_AND_ASK_HOLD_STATE(KEY_BACKSPACE)
      if(do_stuff){
        if(textbox.caret_current > 1){ // TODO: fix weird layout bug at cursor == 0
          arrdel(textbox.text, textbox.caret_current-1);
          textbox.caret_anchor  -= 1;
          textbox.caret_current -= 1;
        }
      }
    }
  }
  if(IsKeyDown(KEY_ENTER)){
    if(IsKeyPressed(KEY_ENTER)){
      INIT_KEY_HOLD_STATE(KEY_ENTER);
      if(textbox.caret_anchor != textbox.caret_current){
        textbox_delete_selection();
      }
      arrins(textbox.text, textbox.caret_current, '\n');
      textbox.caret_anchor  += 1;
      textbox.caret_current += 1;
    }
    else{
      TICK_AND_ASK_HOLD_STATE(KEY_ENTER)
      if(do_stuff){
        arrins(textbox.text, textbox.caret_current, '\n');
        textbox.caret_anchor  += 1;
        textbox.caret_current += 1;
      }
    }
  }
  if(IsKeyDown(KEY_TAB)){
    if(IsKeyPressed(KEY_TAB)){
      INIT_KEY_HOLD_STATE(KEY_TAB);
      if(textbox.caret_anchor != textbox.caret_current){
        textbox_delete_selection();
      }
      arrins(textbox.text, textbox.caret_current, ' ');
      arrins(textbox.text, textbox.caret_current, ' ');
      textbox.caret_anchor  += 2;
      textbox.caret_current += 2;
    }
    else{
      TICK_AND_ASK_HOLD_STATE(KEY_TAB)
      if(do_stuff){
        arrins(textbox.text, textbox.caret_current, ' ');
        arrins(textbox.text, textbox.caret_current, ' ');
        textbox.caret_anchor  += 2;
        textbox.caret_current += 2;
      }
    }
  }
  
  char key = GetCharPressed();
  while (key > 0){
    // NOTE: Only allow keys in range [32..125]
    if ((key >= 32) && (key <= 125)){
      if(textbox.caret_anchor != textbox.caret_current){
        textbox_delete_selection();
      }
      arrins(textbox.text, textbox.caret_current, key);
      textbox.caret_anchor  += 1;
      textbox.caret_current += 1;
    }
    key = GetCharPressed();  // Check next character in the queue
  }
  if(key != 0){
    INIT_KEY_HOLD_STATE(key);
  }
  if(IsKeyDown(key)){
    TICK_AND_ASK_HOLD_STATE(key)
    if(do_stuff){
      arrins(textbox.text, textbox.caret_current, key);
      textbox.caret_anchor  += 1;
      textbox.caret_current += 1;
    }
  }
  
#undef INIT_KEY_HOLD_STATE
#undef TICK_AND_ASK_HOLD_STATE
  
  { // compile manually and automatically
    
    if(textbox.text_hash != stbds_hash_string(textbox.text, 24)){
      state->compile_cd = .8f;
    }
    if(state->compile_cd < 0){
      
      int bubbles_length = arrlen(r->bubbles);
      // read: if (no compiler errors)
      if(bubbles_length > 0){
        
        Assert(0 == arraddnindex(r->remember_graph_info.positions, bubbles_length));
        
        for(int i = 0; i < bubbles_length; i++){
          r->remember_graph_info.positions[i] = r->bubbles[i].pos;
        }
        r->remember_graph_info.bubble_count = bubbles_length;
        r->remember_graph_info.conn_count = arrlen(r->connections);
        
        if(compile(textbox.text)){
          r_draw_errors();
        }
        else{
          if(arrlen(r->bubbles) == r->remember_graph_info.bubble_count && arrlen(r->connections) == r->remember_graph_info.conn_count){
            for(int i = 0; i < bubbles_length; i++){
              r->bubbles[i].pos = r->remember_graph_info.positions[i];
            }
            r_draw_texture();
          }
          else{
            r_produce_satisfactory_output();
          }
        }
        arrsetlen(r->remember_graph_info.positions, 0);
      }
      else{
        if(!compile(textbox.text)){
          r_produce_satisfactory_output();
        }
        else{
          r_draw_errors();
        }
      }
    }
    else
    if(state->compile_cd == 0.f){
      
    }
    if(state->compile_cd > 0){
      state->compile_cd -= GetFrameTime();
    }
    
    if(IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)){
      if(IsKeyPressed(KEY_C)){
        if(!compile(textbox.text)){
          r_produce_satisfactory_output();
        }
        else{
          r_draw_errors();
        }
      }
    }
    textbox.text_hash = stbds_hash_string(textbox.text, 24);
  }
  
  //-------------------------------------------------------------------------
  
  #include "draw.c"
  
}












