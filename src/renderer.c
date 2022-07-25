
#include<limits.h> // INT_MAX

static void r_draw_texture();
static void r_relax_step();

#define W_H_LOCALS  float ox = 6 + g->width/3 + 6 + 6;\
                    float width = g->width - 6 - ox;\
                    float height = g->height - g->menu_height - g->tab_height - 6 - 6\

static void r_init(){
  int bubbles_length = arrlen(r->bubbles);
  
  W_H_LOCALS;
  
  for(int i = 1; i < bubbles_length; i++){
    r->bubbles[i].pos = (Vector2){GetRandomValue(10, width), GetRandomValue(10, height)};
  }
  r_draw_texture();
}

static void r_relax_step(){
  
  int bubbles_length = arrlen(r->bubbles);
  int conns_length = arrlen(r->connections);
  if(bubbles_length == 0 || conns_length == 0) return;
  
  W_H_LOCALS;
  
  const float should = 180;
  const float correction = .2f;
  Vector2 min = {10,10}, max = {width-20 -200, height-20 -100};
  
  // pulling springs
  for(int c = 0; c < conns_length; c++){
    struct Connection cur = r->connections[c];
    Vector2 a_to_b = Vector2Subtract(r->bubbles[cur.B_i].pos, r->bubbles[cur.A_i].pos);
    float dist = Vector2Length(a_to_b);
    
    if(dist > should){
      r->bubbles[cur.A_i].pos = Vector2Clamp(Vector2Add(r->bubbles[cur.A_i].pos, Vector2Scale(a_to_b,  correction)), min, max);
      r->bubbles[cur.B_i].pos = Vector2Clamp(Vector2Add(r->bubbles[cur.B_i].pos, Vector2Scale(a_to_b, -correction)), min, max);
    }
  }
  // repellant magnetism
  for(int a = 0; a < bubbles_length; a++){
    
    // lead weights
    if(r->bubbles[a].outgoing_count == 0 && r->bubbles[a].incoming_count != 0){
      int direction = r->bubbles[r->bubbles[a].subgraph_entry_index].is_subgraph_plus_direction;
      Assert(direction != 0);
      if(direction == DIR_LR || direction == DIR_RL){
        int sign = (direction == DIR_RL) ? (-1) : (1);
        r->bubbles[a].pos.x = Clamp(r->bubbles[a].pos.x + (sign * 15), min.x, max.x);
      }
      else
      if(direction == DIR_TB || direction == DIR_BT){
        int sign = (direction == DIR_BT) ? (-1) : (1);
        r->bubbles[a].pos.y = Clamp(r->bubbles[a].pos.y + (sign * 15), min.y, max.y);
      }
    }
    
    for(int b = a+1; b < bubbles_length; b++){
      
      // floaties
      if( !( r->bubbles[a].is_subgraph_plus_direction || r->bubbles[b].is_subgraph_plus_direction)){
        
        if(r->bubbles[a].incoming_count == 0){
          int direction = r->bubbles[r->bubbles[a].subgraph_entry_index].is_subgraph_plus_direction;
          Assert(direction != 0);
          if(direction == DIR_LR || direction == DIR_RL){
            int sign = (direction == DIR_LR) ? (-1) : (1);
            r->bubbles[a].pos.x = Clamp(r->bubbles[a].pos.x + (sign * 30), min.x, max.x);
            r->bubbles[a].pos.y = Clamp(r->bubbles[a].pos.y - 1, min.y, max.y);
          }
          else
          if(direction == DIR_TB || direction == DIR_BT){
            int sign = (direction == DIR_TB) ? (-1) : (1);
            r->bubbles[a].pos.y = Clamp(r->bubbles[a].pos.y + (sign * 30), min.y, max.y);
            r->bubbles[a].pos.x = Clamp(r->bubbles[a].pos.x - 1, min.x, max.x);
          }
          
        }

        Vector2 a_to_b = Vector2Subtract(r->bubbles[b].pos, r->bubbles[a].pos);
        float dist = Vector2Length(a_to_b);
        
        if(dist < should){
          float force = (should/dist - 1) *.5f;
          r->bubbles[a].pos = Vector2Clamp(Vector2Add(r->bubbles[a].pos, Vector2Scale(a_to_b, -force)), min, max);
          r->bubbles[b].pos = Vector2Clamp(Vector2Add(r->bubbles[b].pos, Vector2Scale(a_to_b,  force)), min, max);
        }
      }
    }
  }
}

static void r_draw_texture(){
  int bubbles_length = arrlen(r->bubbles);
  int conns_length = arrlen(r->connections);
  
  W_H_LOCALS;
  if(width - height)
    ;
  
  BeginDrawing();
    BeginTextureMode(r->render_output);
      ClearBackground(state->dark_mode ? dark_mode[BACKGROUND] : light_mode[BACKGROUND]);
      
      for(int i = 0; i < conns_length; i++){
        if(!r->bubbles[r->connections[i].A_i].is_subgraph_plus_direction && !r->bubbles[r->connections[i].B_i].is_subgraph_plus_direction){
          DrawLineEx(
            Vector2Add(r->bubbles[r->connections[i].A_i].pos, (Vector2){50,25}), 
            Vector2Add(r->bubbles[r->connections[i].B_i].pos, (Vector2){50,25}),
            3, (state->dark_mode ? dark_mode[TEXT_ACTIVE] : light_mode[TEXT_ACTIVE])
          );
        }
      }
      for(int i = 1; i < bubbles_length; i++){
        if(! r->bubbles[i].is_subgraph_plus_direction){
          
          if(r->bubbles[i].bubble_type != BUBBLE_CIRCLE){
            DrawRectangle(r->bubbles[i].pos.x, r->bubbles[i].pos.y, 110, 50, RAYWHITE);
            if(state->dark_mode){
              
            }
            else{
              DrawRectangleLines(r->bubbles[i].pos.x, r->bubbles[i].pos.y, 110, 50, BLACK);
            }
          }
          else{
            Vector2 xy = Vector2Add(r->bubbles[i].pos, (Vector2){50,25});
            float x = xy.x, y = xy.y;
            DrawCircle(x, y, 20, RAYWHITE);
            if(!state->dark_mode) DrawCircleLines(x, y, 20, BLACK);
          }
          
          if(r->bubbles[i].bubble_type != BUBBLE_CIRCLE){
            const char *text = NULL;
            if(r->bubbles[i].text == NULL){
              text = r->bubbles[i].key;
            }
            else{
              text = r->bubbles[i].text;
            }
            
            Vector2 pos_mod = Vector2Add(r->bubbles[i].pos, (Vector2){10,10});
            DrawTextBoxed(font_mono, text, (Rectangle){pos_mod.x, pos_mod.y, 100, 50}, 16, 
                          0, 0, BLACK,
                          0, 0, Vector2Zero(),
                          0, 0, WHITE, BLACK);
          }
        }
      }
    EndTextureMode();
  EndDrawing();
}
static void r_draw_errors(){
  BeginDrawing();
    BeginTextureMode(r->render_output);
      ClearBackground(state->dark_mode ? dark_mode[BACKGROUND] : light_mode[BACKGROUND]);
      
      DrawTextBoxed(font_mono, r->compiler_errors, (Rectangle){10, 10, r->render_output.texture.width, r->render_output.texture.height}, 
                    16, 0, 0, (state->dark_mode ? dark_mode[TEXT_ACTIVE] : light_mode[TEXT_ACTIVE]), 
                    0, 0, Vector2Zero(),
                    0, 0, WHITE, BLACK);
      
    EndTextureMode();
  EndDrawing();
}

static void r_produce_satisfactory_output(){
  int bubbles_length = arrlen(r->bubbles);
  int conns_length = arrlen(r->connections);
  
  int least_crossovers_so_far = INT_MAX;
  
  Vector2 *best_positions_found = NULL;
  arrsetcap(best_positions_found, bubbles_length);
  Assert(0 == arraddnindex(best_positions_found, bubbles_length));
  
  for(int repeat = 0; repeat < 20; repeat++){
    r_init();
    for(int i = 0; i < 100; i++){
      r_relax_step(); 
    }
    
    int crossover_count = 0;
    
    for(int c = 0; c < conns_length; c++){
      
      int c_a = r->connections[c].A_i;
      int c_b = r->connections[c].B_i;
      Vector2 c_a_p = r->bubbles[c_a].pos, c_b_p = r->bubbles[c_b].pos;

      for(int d = c+1; d < conns_length; d++){
        int d_a = r->connections[d].A_i;
        int d_b = r->connections[d].B_i;
        
        // much quicker to compare the indices for equality (to eliminate false positives)
        if(c_a != d_a && c_b != d_b && c_a != d_b && c_b != d_a){
          Vector2 collide_here = { 0 };
          Vector2 d_a_p = r->bubbles[d_a].pos, d_b_p = r->bubbles[d_b].pos;
          
          if(CheckCollisionLines(
            c_a_p, c_b_p,
            d_a_p, d_b_p,
            &collide_here
          )){
            crossover_count++;
          }
        }
      }
    }
    
    if(crossover_count < least_crossovers_so_far){
      least_crossovers_so_far = crossover_count;
      for(int i = 0; i < bubbles_length; i++){
        best_positions_found[i] = r->bubbles[i].pos;
      }
    }
    if(crossover_count == 0) break;
    
  }
  
  for(int i = 0; i < bubbles_length; i++){
    r->bubbles[i].pos = best_positions_found[i];
  }
  
  arrfree(best_positions_found);
  
  r_draw_texture();
}

static void draw(){
  float oy = 6 + g->menu_height + g->tab_height;
  W_H_LOCALS;
  
  Vector2 offset   = (Vector2){ox + 10, oy + 10};
  Rectangle source = { 0.0f, (RENDERTEXTURE_HEIGHT - height), width, -height };
  Rectangle dest   = { ox, oy, width, height };

  BeginScissorModeR(minus_padding(dest, 10));
    DrawTextureRec(r->render_output.texture, source, offset, WHITE);
  EndScissorMode();
  DrawRectangleLinesEx((Rectangle){ox, oy, width, height}, 3, palette[BORDER]);
}


