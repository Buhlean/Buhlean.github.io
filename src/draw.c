

BeginDrawing();
  ClearBackground(palette[BACKGROUND]);
  
  Vector2 mouse_pos = GetMousePosition();
  
  { // main
    DrawRectangleLinesEx(textbox.collider, 3, palette[TEXT_INACTIVE]);
    DrawRectangleRec(minus_padding(textbox.collider, 3), palette[BACKGROUND]);
    
    { // textbox
      
      Rectangle draw_rec = minus_padding(textbox.collider, 17);
      Vector2 updated_position = (Vector2){0.0f, 0.0f};
      
      int select_length = textbox.caret_current - textbox.caret_anchor;
      bool change_both = 0;
      
      if(state->hover == -1 && CheckCollisionPointRec(mouse_pos, textbox.collider)){
        
        state->hover = WIDGETS_TEXTBOX;
        SetMouseCursor(MOUSE_CURSOR_IBEAM);
        
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
          state->clicked = WIDGETS_TEXTBOX;
          change_both = 1;
          state->caret_blink_hp_time_acc = 0;
        }
        
        if(state->clicked == WIDGETS_TEXTBOX){
          updated_position.x = mouse_pos.x - draw_rec.x;
          updated_position.y = mouse_pos.y - draw_rec.y;
        }
        
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
          if(state->clicked == WIDGETS_TEXTBOX){
            
          }
          state->clicked = -1;
        }
      }
      else{
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
      }
      
      if(select_length < 0){
        select_length *= -1;
      }
      int select_start = (textbox.caret_anchor < textbox.caret_current) ? (textbox.caret_anchor) : (textbox.caret_current);
      
      const float blink_cycle_time = 1.4f;
      bool caret_blink_state = 0;
      state->caret_blink_hp_time_acc += GetFrameTime();
      if(state->caret_blink_hp_time_acc > 0) caret_blink_state = 1;
      if(state->caret_blink_hp_time_acc > blink_cycle_time/2) state->caret_blink_hp_time_acc -= blink_cycle_time;
      
      int caret_new_index = DrawTextBoxedMono(font_mono, textbox.text, draw_rec, 16, 
                                          0, 0, palette[TEXT_ACTIVE], 
                                          textbox.caret_current, caret_blink_state, updated_position, 
                                          select_start, select_length, palette[HIGHLIGHT], palette[CARET]);
      if(change_both){
        textbox.caret_anchor  = caret_new_index;
      }
      if(state->clicked == WIDGETS_TEXTBOX){
        textbox.caret_current = caret_new_index;
      }
      
      //if(debug_switch) TraceLog(LOG_DEBUG, "a.%d, c.%d, n.%d", textbox.caret_anchor, textbox.caret_current, caret_new_index);
    }
    
  }
  
  // essentially takes the render texture onto which we drew the graph and blits it to the screen
  draw();
  
#if defined(PLATFORM_WEB)
  // TODO: make robust across browsers
  if(IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)){
    if(IsKeyPressed(KEY_P)){
      Texture graph_tex = r->render_output.texture;
      int bubbles_length = arrlen(r->bubbles);
      float x_max = 0, x_min = 0, y_max = 0, y_min = 0;
      if(bubbles_length == 0){
        x_max = graph_tex.width, y_max = 300;
      }
      else{
        x_min = graph_tex.width, y_min = graph_tex.height;
        for(int i = 0; i < bubbles_length; i++){
          float x = r->bubbles[i].pos.x, y = r->bubbles[i].pos.y;
          if(x_min > x) x_min = x;
          if(y_min > y) y_min = y;
          if(x_max < x) x_max = x;
          if(y_max < y) y_max = y;
        }
        x_max += 10 + 110;
        y_max += 10 + 50;
        x_min -= 0;
        y_min -= 0;
      }
      {
        void *pixel_data = rlReadTexturePixels(graph_tex.id, graph_tex.width, graph_tex.height, graph_tex.format);
        Image im = {pixel_data, graph_tex.width, graph_tex.height, 1, graph_tex.format};
        ImageFlipVertical(&im);
        ImageCrop(&im, (Rectangle){x_min, y_min, x_max, y_max});
        ExportImage(im, "screenshot.png");
        UnloadImage(im);
      }
      
      EM_ASM(
        saveFileFromMEMFSToDisk("screenshot.png", "flowgraph.png");
      );
    }
  }
  DrawTextEx(font_mono, "ALT + C -> Recompile and layout", (Vector2){25, g->height - 50}, 16, 0, palette[TEXT_ACTIVE]);
  DrawTextEx(font_mono, "ALT + S -> Palette switch", (Vector2){25, g->height - 30}, 16, 0, palette[TEXT_ACTIVE]);
#else
  if(IsKeyPressed(KEY_F6)){
    
    int screenshot_persistent_counter_storage_position = 0;
    int value = LoadStorageValue(screenshot_persistent_counter_storage_position);
    SaveStorageValue(screenshot_persistent_counter_storage_position, (value+2));
    Texture graph_tex = r->render_output.texture;
    
    int bubbles_length = arrlen(r->bubbles);
    float x_max = 0, x_min = 0, y_max = 0, y_min = 0;
    if(bubbles_length == 0){
      x_max = graph_tex.width, y_max = 300;
    }
    else{
      x_min = graph_tex.width, y_min = graph_tex.height;
      for(int i = 0; i < bubbles_length; i++){
        float x = r->bubbles[i].pos.x, y = r->bubbles[i].pos.y;
        if(x_min > x) x_min = x;
        if(y_min > y) y_min = y;
        if(x_max < x) x_max = x;
        if(y_max < y) y_max = y;
      }
      x_max += 10 + 110;
      y_max += 10 + 50;
      x_min -= 0;
      y_min -= 0;
    }
    {
      void *pixel_data = rlReadTexturePixels(graph_tex.id, graph_tex.width, graph_tex.height, graph_tex.format);
      Image im = {pixel_data, graph_tex.width, graph_tex.height, 1, graph_tex.format};
      ImageFlipVertical(&im);
      const char *format_filename = TextFormat("./screenshot%04d.png", value);
      ImageCrop(&im, (Rectangle){x_min, y_min, x_max, y_max});
      ExportImage(im, format_filename);
      UnloadImage(im);
    }
    state->dark_mode = !state->dark_mode;
    if(bubbles_length == 0){
      r_draw_errors();
    }
    else{
      r_draw_texture();
    }
    {
      void *pixel_data = rlReadTexturePixels(graph_tex.id, graph_tex.width, graph_tex.height, graph_tex.format);
      Image im = {pixel_data, graph_tex.width, graph_tex.height, 1, graph_tex.format};
      ImageFlipVertical(&im);
      const char *format_filename = TextFormat("./screenshot%04d.png", value+1);
      ImageCrop(&im, (Rectangle){x_min, y_min, x_max, y_max});
      ExportImage(im, format_filename);
      UnloadImage(im);
    }
    state->dark_mode = !state->dark_mode;
    if(bubbles_length == 0){
      r_draw_errors();
    }
    else{
      r_draw_texture();
    }
  }
  
  DrawText("F5 -> Try Again", 25, g->height - 50, 20, palette[TEXT_ACTIVE]);
  DrawText("F6 -> Screenshot", 225, g->height - 50, 20, palette[TEXT_ACTIVE]);
  DrawText("F7 -> Palette switch", 25, g->height - 25, 20, palette[TEXT_ACTIVE]);
#endif
  
  
EndDrawing();

// if(debug_switch) TraceLog(LOG_DEBUG, "h: %d", state->hover);
// if(debug_switch) TraceLog(LOG_DEBUG, "c: %d", state->clicked);

state->hover  = -1;

// if(debug_switch) TraceLog(LOG_DEBUG, "");
debug_switch = 0;

