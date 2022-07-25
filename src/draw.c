

BeginDrawing();
  ClearBackground(palette[BACKGROUND]);
  
  Vector2 mouse_pos = GetMousePosition();
  
  { // update menu 
    if(state->menu_tab_current > 0){
      int which = state->menu_tab_current;
      
      Assert(which >= 0);
      Assert(which < state->menu_array_count);

      int entry_index = state->top_menu[which].sub_menu_index;
      int entry_count = state->top_menu[which].entry_count;
      //int lines_count = state->top_menu[which].functionality_index; // reused in nodes
      int lines_tally = 0;
      
      Vector2 offset = {menu_measure_labels_up_to(which).x + (WORD_SPACING*2 * (which-1)), g->menu_height};
      offset.y += 3;
      Vector2 og_offset = offset;
      float entry_width = 300;
      float entry_height = menu_measure_entry_label().y + WORD_SPACING;
      
      for(int i = 0; i < entry_count; i++){
        
        if(state->top_menu[entry_index + i].functionality_index == -1){
          offset.y += 7 - entry_height;
          if(state->menu_layer3_current > (entry_index + i)) lines_tally++;
          continue;
        }
        
        if(CheckCollisionPointRec(mouse_pos, 
            (Rectangle){
              offset.x + 3, 
              offset.y + i * entry_height, 
              entry_width - 6, 
              entry_height
            })){
          
          state->hover = WIDGETS_MENU + entry_index + i;
          
          if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            state->clicked = WIDGETS_MENU + entry_index + i;
          }
          if(state->top_menu[entry_index + i].entry_count > 0){
            state->menu_layer3_current = entry_index + i;
            break;
          }
          else{
            state->menu_layer3_current = 0;
          }
          if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
            if(state->top_menu[entry_index + i].entry_count <= 0){
              menu_click_do_something(state->top_menu[entry_index + i].functionality_index);
            }
          }
        }
      }
      if(state->menu_layer3_current > 0){
        
        offset.y = og_offset.y + (state->menu_layer3_current - entry_index - lines_tally) * entry_height + (lines_tally * 7);
        offset.x += entry_width - 3;
        
        entry_index = state->top_menu[state->menu_layer3_current].sub_menu_index;
        entry_count = state->top_menu[state->menu_layer3_current].entry_count;
        
        for(int i = 0; i < entry_count; i++){
          
          if(state->top_menu[entry_index + i].functionality_index == -1){
            offset.y += 7 - entry_height;
            continue;
          }
          
          if(CheckCollisionPointRec(mouse_pos, 
              (Rectangle){
                offset.x + 3, 
                offset.y + i * entry_height, 
                entry_width - 6, 
                entry_height
              })){
            
            state->hover = WIDGETS_MENU + entry_index + i;
            
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
              state->clicked = WIDGETS_MENU + entry_index + i;
            }
            if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
              if(state->top_menu[entry_index + i].entry_count <= 0){
                menu_click_do_something(state->top_menu[entry_index + i].functionality_index);
              }
            }
          }
        }
      }
    }
    if(state->menu_rightclick_current > 0){
      
    }
  }
  
  if(state->clicked == -1 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
    state->menu_tab_current = 0;
  }

  { // main
    { // top menu 
      int top_entries = state->top_menu[0].entry_count;
      float entry_size = 0;
      float entry_height = g->menu_height;
      float entry_sum  = 0;
      for(int i = 1; i <= top_entries; i++){
        entry_size = MeasureTextEx(font, state->top_menu[i].name, g->font_size_menu_entry, 0).x + WORD_SPACING*2;
        
        //DrawRectangleRec((Rectangle){entry_sum, 0, entry_size, entry_height}, GREEN);
        int mouse_in_entry = (CheckCollisionPointRec(mouse_pos, (Rectangle){entry_sum, 0, entry_size, entry_height}));
        if (mouse_in_entry){
          
          state->hover = WIDGETS_MENU + i;
          
          if(state->menu_tab_current > 0) state->menu_tab_current = i;
          
          if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            state->clicked = WIDGETS_MENU + i;
            
            // this is how my editor behaves and I'm copying more experienced people here
            state->menu_tab_current = i;
          }
          if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
            // nothing happens
            state->clicked = -1; 
          }
        }
        
        if(mouse_in_entry) DrawRectangle(entry_sum, 0, entry_size, entry_height, palette[TAB_ACTIVE]);
        DrawTextPro(font, state->top_menu[i].name, (Vector2){entry_sum + WORD_SPACING, 5}, Vector2Zero(), 0.f, g->font_size_menu, 0, palette[TEXT_ACTIVE]);
        
        entry_sum += entry_size;
      }
    }
    
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

#if defined(PLATFORM_WEB)

#else
  { // draw menu
    if(state->menu_tab_current > 0){
      int which = state->menu_tab_current;
      
      Assert(which > 0);
      Assert(which < state->menu_array_count);

      int entry_index = state->top_menu[which].sub_menu_index;
      int entry_count = state->top_menu[which].entry_count;
      int lines_count = state->top_menu[which].functionality_index; // reused in nodes
      int lines_tally = 0;
      
      Vector2 offset = {menu_measure_labels_up_to(which).x + (WORD_SPACING*2 * (which-1)), g->menu_height};
      offset.y += 3;
      Vector2 og_offset = offset;
      float entry_width = 300;
      float entry_height = menu_measure_entry_label().y + WORD_SPACING;
      
      DrawRectangle(offset.x, offset.y - 3, Clamp(entry_width, 0, g->width), 6 + entry_height * (entry_count - lines_count) + 7 * lines_count, palette[MENU_BG]);
      DrawRectangleLines(offset.x, offset.y - 3, Clamp(entry_width, 0, g->width), 6 + entry_height * (entry_count - lines_count) + 7 * lines_count, palette[TEXT_INACTIVE]);
      for(int i = 0; i < entry_count; i++){
        
        if(state->top_menu[entry_index + i].functionality_index == -1){
          int line_y = offset.y + i * entry_height + 3;
          DrawLine(offset.x + 3, line_y, offset.x + 3 + entry_width - 6, line_y, palette[TEXT_INACTIVE]);
          offset.y += 7 - entry_height;
          if(state->menu_layer3_current > (entry_index + i)) lines_tally++;
          continue;
        }
        
        if(state->hover == WIDGETS_MENU + entry_index + i){
          DrawRectangle(offset.x + 3, offset.y + i * entry_height, entry_width - 6, entry_height, palette[TAB_ACTIVE]);
        }
        DrawTextPro(font, state->top_menu[entry_index + i].name, (Vector2){offset.x + (WORD_SPACING), offset.y + WORD_SPACING/2 + i * entry_height}, Vector2Zero(), 0.f, g->font_size_menu_entry, 0, palette[TEXT_ACTIVE]);
      }
      if(state->menu_layer3_current > 0){
        offset.y = og_offset.y + (state->menu_layer3_current - entry_index - lines_tally) * entry_height + (lines_tally * 7);
        offset.x += entry_width - 3;
        
        entry_index = state->top_menu[state->menu_layer3_current].sub_menu_index;
        entry_count = state->top_menu[state->menu_layer3_current].entry_count;
        lines_count = state->top_menu[state->menu_layer3_current].functionality_index; // reused in nodes
        
        DrawRectangle(offset.x, offset.y - 3, Clamp(entry_width, 0, g->width), 6 + entry_height * (entry_count), palette[MENU_BG]);
        DrawRectangleLines(offset.x, offset.y - 3, Clamp(entry_width, 0, g->width), 6 + entry_height * (entry_count), palette[TEXT_INACTIVE]);
        for(int i = 0; i < entry_count; i++){
          
          if(state->top_menu[entry_index + i].functionality_index == -1){
            int line_y = offset.y + i * entry_height + 3;
            DrawLine(offset.x + 3, line_y, offset.x + 2 + entry_width - 6, line_y, palette[TEXT_INACTIVE]);
            offset.y += 7 - entry_height;
            continue;
          }
          
          if(state->hover == WIDGETS_MENU + entry_index + i){
            DrawRectangle(offset.x + 3, offset.y + i * entry_height, entry_width - 6, entry_height, palette[TAB_ACTIVE]);
          }
          DrawTextPro(font, state->top_menu[entry_index + i].name, (Vector2){offset.x + (WORD_SPACING), offset.y + WORD_SPACING/2 + i * entry_height}, Vector2Zero(), 0.f, g->font_size_menu_entry, 0, palette[TEXT_ACTIVE]);
        }
      }
    }
    if(state->menu_rightclick_current > 0){
      
    }
  }
#endif
  
  draw();
  
#if defined(PLATFORM_WEB)
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

