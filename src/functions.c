
static void mymemzero(void *p, int size){
  if(!p) return;
  for(int i = 0; i < size; i++){
    ((unsigned char *)p)[i] = 0;
  }
}
/* static void TextCopyN(char *dst, const char *src, int length){
  if (dst != NULL){
    for(int i = 0; i < length; i++){
      *dst = *src;
      dst++;
      src++;
    }
    *dst = '\0';
  }
} */
static Vector2 Vector2Clamp(Vector2 value, Vector2 min, Vector2 max)
{
    Vector2 result = (Vector2){(value.x < min.x)? min.x : value.x, (value.y < min.y)? min.y : value.y};

    if (result.x > max.x) result.x = max.x;
    if (result.y > max.y) result.y = max.y;

    return result;
}

static void UpdateDraw();

static inline Rectangle minus_padding(Rectangle r, int padding){
  return (Rectangle){r.x+padding, r.y+padding, r.width-padding*2, r.height-padding*2};
}
static inline void BeginScissorModeR(Rectangle r){
  BeginScissorMode(r.x, r.y, r.width, r.height);
}

#define NODE(name,num)  Assert(TextLength(name)<MENU_NAME_LENGTH);\
                        AssertS(count<total, "Set total == count!");\
                        check=(num);\
                        entries_in_this=(num); \
                        reserved_up_to_here+=entries_in_prev; \
                        m[count]=(struct menu){(name),entries_in_this,(reserved_up_to_here),0}; \
                        count++; \
                        entries_in_prev = entries_in_this
#define LEAF(name,fun)  Assert(TextLength(name)<MENU_NAME_LENGTH);\
                        m[count]=(struct menu){(name),0,0,fun}; \
                        AssertS(count<total, "Set total == count!");\
                        count++
static void menu_top_init(){
  
  // adjust total to mirror the amount of entries. Don't worry, an assertion will remind you.
  const int total = 5;
  state->top_menu = RL_MALLOC(sizeof(*state->top_menu) * total);
  struct menu *m = state->top_menu;
  
  int count = 0;
  int reserved_up_to_here = 0;
  int entries_in_this = 0;
  int entries_in_prev = 1;
  int check = 0;
  
  { // level 0
    NODE("", 2);
  }
  { // level 1
    NODE("File", 1);
    NODE("Help", 1);
  }
  { // level 2
    //File
    LEAF("Exit",199);

    //Help
    LEAF("Good Luck!",1501);
  }
  { // layer 3 

  }
  
  //if(debug_switch) TraceLog(LOG_INFO, "t: %d, c: %d, r: %d, c: %d, s: %d", total, count, reserved_up_to_here, check, reserved_up_to_here + check);
  AssertSI(count == total, "Set total == %d", count);
  Assert(total == reserved_up_to_here + check);
  
  state->menu_array_count = total;
  
  // encode line_count per submenu into unused struct member
  for(int i = 1; i <= state->top_menu[0].entry_count; i++){
    int j_res = 0;
    if(state->top_menu[i].entry_count != 0){
      for(int j = state->top_menu[i].sub_menu_index ; j < state->top_menu[i].sub_menu_index + state->top_menu[i].entry_count; j++){
        int k_res = 0;
        if(state->top_menu[j].entry_count == 0){
          if(state->top_menu[j].functionality_index == -1) j_res++;
        }
        else{
          for(int k = state->top_menu[j].sub_menu_index; k < state->top_menu[j].sub_menu_index + state->top_menu[j].entry_count; k++){
            if(state->top_menu[k].entry_count == 0 && state->top_menu[k].functionality_index == -1) k_res++;
          }
          state->top_menu[j].functionality_index = k_res;
        }
        
      }
      state->top_menu[i].functionality_index = j_res;
    }
  }
}
static void menu_rightclick_init(){
  
  // adjust total to mirror the amount of entries. Don't worry, an assertion will remind you.
  const int total = 13;
  state->rightclick_menu = RL_MALLOC(sizeof(*state->top_menu) * total*2);
  struct menu *m = state->rightclick_menu;
  
  int count = 0;
  int reserved_up_to_here = 0;
  int entries_in_this = 0;
  int entries_in_prev = 1;
  int check = 0;
  
  { // level 0
    NODE("", 1);
  }
  { // level 1
    NODE("TAB", 11);

  }
  { // level 2
    //TAB
    LEAF("Close",100001);
    LEAF("Close All BUT This",100002);
    LEAF("Close All To The Left",100003);
    LEAF("Close All To The Right",100004);
    LEAF("Close All Unchanged",100005);
    LEAF("Save",102);
    LEAF("Save as...",103);
    LEAF("Rename",100008);
    LEAF("Move To Recycle Bin",100009);
    LEAF("Reload",100010);
    LEAF("Print",100011);

  }
  { // layer 3 
  }
  
  //if(debug_switch) TraceLog(LOG_INFO, "t: %d, c: %d, r: %d, c: %d, s: %d", total, count, reserved_up_to_here, check, reserved_up_to_here + check);
  AssertSI(count == total, "Set total == %d", count);
  Assert(total == reserved_up_to_here + check);
  
  state->menu_array_count = total;
  
  // encode line_count per submenu into unused struct member
  for(int i = 1; i <= state->rightclick_menu[0].entry_count; i++){
    int j_res = 0;
    if(state->rightclick_menu[i].entry_count != 0){
      for(int j = state->rightclick_menu[i].sub_menu_index ; j < state->rightclick_menu[i].sub_menu_index + state->rightclick_menu[i].entry_count; j++){
        int k_res = 0;
        if(state->rightclick_menu[j].entry_count == 0){
          if(state->rightclick_menu[j].functionality_index == -1) j_res++;
        }
        else{
          for(int k = state->rightclick_menu[j].sub_menu_index; k < state->rightclick_menu[j].sub_menu_index + state->rightclick_menu[j].entry_count; k++){
            if(state->rightclick_menu[k].entry_count == 0 && state->rightclick_menu[k].functionality_index == -1) k_res++;
          }
          state->rightclick_menu[j].functionality_index = k_res;
        }
        
      }
      state->rightclick_menu[i].functionality_index = j_res;
    }
  }
}
#undef NODE
#undef LEAF
static Vector2 menu_measure_labels_up_to(int which){
  Vector2 result = {0,0};
  Assert(which <= state->top_menu[0].entry_count);
  for(int i = 1; i < which; i++){
    result = Vector2Add(result, MeasureTextEx(font, state->top_menu[i].name, g->font_size_menu_entry, 0));
  }
  return result;
}
static Vector2 menu_measure_entry_label(){
  Vector2 result = {0,0};
  Assert(state->menu_array_count > 2);
  Assert(state->top_menu[1].entry_count >= 1);
  int first_menu_index = state->top_menu[1].sub_menu_index;
  struct menu first_entry = state->top_menu[first_menu_index];
  //if(debug_switch) {TraceLog(LOG_DEBUG, "%d, %s", first_menu_index, first_entry.name);}
  result = MeasureTextEx(font, first_entry.name, g->font_size_menu_entry, 0);
  return result;
}
static void menu_click_do_something(int functionality_index){
  switch(functionality_index){
    case 101:{
      TraceLog(LOG_INFO, "Open File");
    };break;
    case 102:{
      TraceLog(LOG_INFO, "Save");
    };break;
    case 103:{
      TraceLog(LOG_INFO, "Save as...");
    };break;
    case 199:{
      TraceLog(LOG_INFO, "Exit via menu");
      exit(0);
    };break;

    default: TraceLog(LOG_DEBUG, "FORGOT FUNCTIONALITY: %d", functionality_index);break;
  }
  
  state->menu_tab_current    =  0;
  state->menu_layer3_current =  0;
  state->hover               = -1;
  state->clicked             = -1;
  state->dragged             = -1;
  
}

/* static Rectangle rectangle_scale_size(Rectangle rec, float scale){
  Rectangle result = {rec.x, rec.y, rec.width * scale, rec.height * scale};
  return result;
} */
/* static Rectangle rectangle_scale_position(Rectangle rec, float scale){
  Rectangle result = {rec.x * scale, rec.y * scale, rec.width, rec.height};
  return result;
} */
/* static Rectangle rectangle_scale_all(Rectangle rec, float scale){
  Rectangle result = {rec.x * scale, rec.y * scale, rec.width * scale, rec.height * scale};
  return result;
} */

/* static Rectangle icon_atlas_get_icon(int icon_index, struct TextureAtlas ia){
  Rectangle result = {
    (icon_index % ia.atlas_width) * ia.pixel_width,
    (icon_index / ia.atlas_width) * ia.pixel_height,
    ia.pixel_width,
    ia.pixel_height,
  };
  return result;
} */
/* static struct window *window_make_empty(struct tab *tab, int type){
  struct window w = {type, 0};
  arrput(tab->windows, w);
  return &(tab->windows[arrlen(tab->windows)-1]);
} */
/* static struct window *window_get_by_type(struct tab *tab, int type){
  struct window *result = NULL;
  for(int i = 0; i < arrlen(tab->windows); i++){
    if(tab->windows[i].type == type){
      result = &(tab->windows[i]);
      break;
    }
  }
  if(!result){
    result = window_make_empty(tab, type);
  }
  return result;
} */
/* static void draw_spritesheet_dialog(int x, int y, int w, int h){
  
  int type = WIN_SPRITESHEET;
  struct window *window = window_get_by_type(&(tabs[state->tab_current]), type);
  
  Vector2 mouse_pos = GetMousePosition();
  Rectangle parent = {x, y, w, h};
  Rectangle child = parent;
  
  // heading
  {
    char *text = "Import Mask from Spritesheet";
    float size = g->font_size_dialog_heading;
    Vector2 text_measure = MeasureTextEx(font, text, size, 0);
    DrawRectangle(x, y, w, 24, BLACK);
    DrawTextEx(font, text, (Vector2){child.x + 3, child.y + 5}, size, 0, palette[TEXT_ACTIVE]);
    float to_remove = text_measure.y + 10;
    child.y      += to_remove;
    child.height -= to_remove;
  }
  
  // bg 
  DrawRectangleRec(child, palette[BORDER]);
  child = minus_padding(child, 10);
  parent = child;
  
  {
    char *text = "Comprised of several spritesheets";
    float size = g->font_size_dialog;
    Vector2 text_measure = MeasureTextEx(font, text, size, 0);
    DrawTextEx(font, text, (Vector2){child.x + g->padding + text_measure.y, child.y + 1}, size, 0, palette[TEXT_ACTIVE]);
    
    Rectangle collider = {child.x, child.y, text_measure.y, text_measure.y};
    DrawRectangleLinesEx(collider, 1, palette[TEXT_ACTIVE]);
    
    int element = 0;
    if(CheckCollisionPointRec(mouse_pos, collider)){
      state->hover = WIDGETS_WINDOWS + type*100 + element;
      if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        state->clicked = WIDGETS_WINDOWS + type*100 + element;
      }
      if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
        if(state->clicked == state->hover && state->hover != -1){
          window->state[element] = !window->state[element];
        }
        state->clicked = -1;
      }
    }
    
    const int inset = 3;
    if(window->state[element]) DrawRectangle(child.x + inset, child.y + inset, text_measure.y - inset*2, text_measure.y - inset*2, palette[HIGHLIGHT]);
    
    float to_remove = text_measure.y + g->padding;
    child.y      += to_remove;
    child.height -= to_remove;
  }
  {
    
  }
} */
 

// Draw text using font inside rectangle limits with support for text selection
// Figures out where the caret should be placed while doing that and returns it as an index
static int DrawTextBoxed(Font font, const char *text, Rectangle rec, float font_size, 
                        float spacing, bool word_wrap, Color tint, 
                        int caret_index, bool caret_draw, Vector2 caret_new_pos, 
                        int select_start, int select_length, Color select_tint, Color select_back_tint){
  
  int length = TextLength(text);  // Total length in bytes of the text, scanned by codepoints in loop

  float text_offset_y = 0;          // Offset between lines (on line break '\n')
  float text_offset_x = 0.0f;       // Offset X to next character to draw
  
  bool new_caret_found = 0;
  int  new_caret_index = caret_index;

  float scale_factor = font_size/(float)font.baseSize;     // Character rectangle scaling factor

  // Word/character wrapping mechanism variables
  enum { MEASURE_STATE = 0, DRAW_STATE = 1 };
  int state = word_wrap? MEASURE_STATE : DRAW_STATE;

  int line_start = -1;         // Index where to begin drawing (where a line begins)
  int line_end = -1;           // Index where to stop drawing (where a line ends)
  int last_k = -1;             // Holds last value of the character position

  for (int i = 0, k = 0; i < length; i++, k++){
    // Get next codepoint from byte string and glyph index in font
    int codepoint_byte_count = 0;
    int codepoint = GetCodepoint(&text[i], &codepoint_byte_count);
    int glyph_index = GetGlyphIndex(font, codepoint);

    // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
    // but we need to draw all of the bad bytes using the '?' symbol moving one byte
    if (codepoint == 0x3f) codepoint_byte_count = 1;
    i += (codepoint_byte_count - 1);

    float glyph_width = 0;
    if (codepoint != '\n'){
      glyph_width = (font.glyphs[glyph_index].advanceX == 0) ? font.recs[glyph_index].width*scale_factor : font.glyphs[glyph_index].advanceX*scale_factor;

      if (i + 1 < length) glyph_width = glyph_width + spacing;
    }

    // NOTE: When word_wrap is ON we first measure how much of the text we can draw before going outside of the rec container
    // We store this info in line_start and line_end, then we change states, draw the text between those two variables
    // and change states again and again recursively until the end of the text (or until we get outside of the container).
    // When word_wrap is OFF we don't need the measure state so we go to the drawing state immediately
    // and begin drawing on the next line before we can get outside the container.
    if (state == MEASURE_STATE){
      // TODO: There are multiple types of spaces in UNICODE, maybe it's a good idea to add support for more
      // Ref: http://jkorpela.fi/chars/spaces.html
      if ((codepoint == ' ') || (codepoint == '\t') || (codepoint == '\n')) line_end = i;

      if ((text_offset_x + glyph_width) > rec.width){
        line_end = (line_end < 1)? i : line_end;
        if (i == line_end) line_end -= codepoint_byte_count;
        if ((line_start + codepoint_byte_count) == line_end) line_end = (i - codepoint_byte_count);

        state = !state;
      }
      else if ((i + 1) == length){
        line_end = i;
        state = !state;
      }
      else if (codepoint == '\n') state = !state;

      if (state == DRAW_STATE){
        text_offset_x = 0;
        i = line_start;
        glyph_width = 0;

        // Save character position when we switch states
        int tmp = last_k;
        last_k = k - 1;
        k = tmp;
      }
    }
    else{
      
      // caret
      if(!new_caret_found){
        // mouse inside of this line
        if((caret_new_pos.y > text_offset_y -2 && caret_new_pos.y < text_offset_y -1 + (font.baseSize + font.baseSize/4)*scale_factor)
        // mouse above the first line, nice to have
        || (text_offset_y < .1f && caret_new_pos.y > -20.f && caret_new_pos.y < text_offset_y)){
          // mouse off to the left
          if((text_offset_x < .1f && caret_new_pos.x > -20.f && caret_new_pos.x < text_offset_x) 
          // mouse between two characters
          || (caret_new_pos.x >= text_offset_x - glyph_width * .81f && caret_new_pos.x < text_offset_x + glyph_width * .2f) 
          // mouse off to the right
          || (codepoint == '\n' && caret_new_pos.x > text_offset_x - glyph_width * .81f)){
            
            new_caret_index = i;
            new_caret_found = 1;
          }
        }
      }
      {
        // the caret has to be drawn on top but we change the offsets here if newline, therefore store them
        float caretx = text_offset_x, carety = text_offset_y;
        
        if (codepoint == '\n'){
          if (!word_wrap){
            text_offset_y += (font.baseSize + font.baseSize/4)*scale_factor;
            text_offset_x = 0;
          }
        }
        else{
          if (!word_wrap && ((text_offset_x + glyph_width) > rec.width)){
            text_offset_y += (font.baseSize + font.baseSize/4)*scale_factor;
            text_offset_x = 0;
          }

          // When text overflows rectangle height limit, just stop drawing
          if ((text_offset_y + font.baseSize*scale_factor) > rec.height) break;

          // Draw selection background
          bool isGlyphSelected = false;
          if ((select_start >= 0) && (k >= select_start) && (k < (select_start + select_length))){
            DrawRectangleRec((Rectangle){ rec.x + text_offset_x - 1, rec.y + text_offset_y - 2, glyph_width, (float)(font.baseSize + font.baseSize/4)*scale_factor }, select_back_tint);
            isGlyphSelected = true;
          }

          // Draw current character glyph
          if ((codepoint != ' ') && (codepoint != '\t')){
            DrawTextCodepoint(font, codepoint, (Vector2){ rec.x + text_offset_x, rec.y + text_offset_y }, font_size, isGlyphSelected? select_tint : tint);
          }
        }
        if(caret_index == i && caret_draw){
          DrawLine(rec.x + caretx + 1, rec.y + carety + 0, rec.x + caretx, rec.y + carety + (font.baseSize + font.baseSize/4)*scale_factor - 6 , WHITE);
          caret_draw = 0;
        }
      }

      if (word_wrap && (i == line_end)){
        text_offset_y += (font.baseSize + font.baseSize/4)*scale_factor;
        text_offset_x = 0;
        line_start = line_end;
        line_end = -1;
        glyph_width = 0;
        select_start += last_k - k;
        k = last_k;

        state = !state;
      }
    }

    text_offset_x += glyph_width;
  }
  
  if(!new_caret_found){
    new_caret_index = last_k;
  }
  
  return new_caret_index;
}
static int DrawTextBoxedMono(Font font, const char *text, Rectangle rec, float font_size, 
                        float spacing, bool word_wrap, Color tint, 
                        int caret_index, bool caret_draw, Vector2 caret_new_pos, 
                        int select_start, int select_length, Color select_tint, Color select_back_tint){
  
  int length = TextLength(text);  // Total length in bytes of the text, scanned by codepoints in loop

  float text_offset_y = 0;          // Offset between lines (on line break '\n')
  float text_offset_x = 0.0f;       // Offset X to next character to draw
  
  bool new_caret_found = 0;
  int  new_caret_index = caret_index;

  float scale_factor = font_size/(float)font.baseSize;     // Character rectangle scaling factor

  // Word/character wrapping mechanism variables
  enum { MEASURE_STATE = 0, DRAW_STATE = 1 };
  int state = word_wrap? MEASURE_STATE : DRAW_STATE;

  int line_start = -1;         // Index where to begin drawing (where a line begins)
  int line_end = -1;           // Index where to stop drawing (where a line ends)
  int last_k = -1;             // Holds last value of the character position
  
  int codepoint_byte_count = 0;
  int glyph_index = GetGlyphIndex(font, GetCodepoint(&text[0], &codepoint_byte_count));
  float glyph_width = spacing + ((font.glyphs[glyph_index].advanceX == 0) ? font.recs[glyph_index].width*scale_factor : font.glyphs[glyph_index].advanceX*scale_factor);

  for (int i = 0, k = 0; i < length; i++, k++){
    // Get next codepoint from byte string and glyph index in font
    codepoint_byte_count = 0;
    int codepoint = GetCodepoint(&text[i], &codepoint_byte_count);
    
    // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
    // but we need to draw all of the bad bytes using the '?' symbol moving one byte
    if (codepoint == 0x3f) codepoint_byte_count = 1;
    i += (codepoint_byte_count - 1);

    // NOTE: When word_wrap is ON we first measure how much of the text we can draw before going outside of the rec container
    // We store this info in line_start and line_end, then we change states, draw the text between those two variables
    // and change states again and again recursively until the end of the text (or until we get outside of the container).
    // When word_wrap is OFF we don't need the measure state so we go to the drawing state immediately
    // and begin drawing on the next line before we can get outside the container.
    if (state == MEASURE_STATE){
      // TODO: There are multiple types of spaces in UNICODE, maybe it's a good idea to add support for more
      // Ref: http://jkorpela.fi/chars/spaces.html
      if ((codepoint == ' ') || (codepoint == '\t') || (codepoint == '\n')) line_end = i;

      if ((text_offset_x + glyph_width) > rec.width){
        line_end = (line_end < 1)? i : line_end;
        if (i == line_end) line_end -= codepoint_byte_count;
        if ((line_start + codepoint_byte_count) == line_end) line_end = (i - codepoint_byte_count);

        state = !state;
      }
      else if ((i + 1) == length){
        line_end = i;
        state = !state;
      }
      else if (codepoint == '\n') state = !state;

      if (state == DRAW_STATE){
        text_offset_x = 0;
        i = line_start;
        glyph_width = 0;

        // Save character position when we switch states
        int tmp = last_k;
        last_k = k - 1;
        k = tmp;
      }
    }
    else{
      
      // caret
      if(!new_caret_found){
        // mouse inside of this line
        if((caret_new_pos.y > text_offset_y -2 && caret_new_pos.y < text_offset_y -1 + (font.baseSize + font.baseSize/4)*scale_factor)
        // mouse above the first line, nice to have
        || (text_offset_y < .1f && caret_new_pos.y > -20.f && caret_new_pos.y < text_offset_y)){
          // mouse off to the left
          if((text_offset_x < .1f && caret_new_pos.x > -20.f && caret_new_pos.x < text_offset_x) 
          // mouse between two characters
          || (caret_new_pos.x >= text_offset_x - glyph_width * .81f && caret_new_pos.x < text_offset_x + glyph_width * .2f) 
          // mouse off to the right
          || (codepoint == '\n' && caret_new_pos.x > text_offset_x - glyph_width * .81f)){
            
            new_caret_index = i;
            new_caret_found = 1;
          }
        }
      }
      {
        // the caret has to be drawn on top but we change the offsets here if newline, therefore store them
        float caretx = text_offset_x, carety = text_offset_y;
        
        if (codepoint == '\n'){
          if (!word_wrap){
            text_offset_y += (font.baseSize + font.baseSize/4)*scale_factor;
            text_offset_x = 0;
          }
        }
        else{
          if (!word_wrap && ((text_offset_x + glyph_width) > rec.width)){
            text_offset_y += (font.baseSize + font.baseSize/4)*scale_factor;
            text_offset_x = 0;
          }

          // When text overflows rectangle height limit, just stop drawing
          if ((text_offset_y + font.baseSize*scale_factor) > rec.height) break;

          // Draw selection background
          bool isGlyphSelected = false;
          if ((select_start >= 0) && (k >= select_start) && (k < (select_start + select_length))){
            DrawRectangleRec((Rectangle){ rec.x + text_offset_x - 1, rec.y + text_offset_y - 2, glyph_width, (float)(font.baseSize + font.baseSize/4)*scale_factor }, select_back_tint);
            isGlyphSelected = true;
          }

          // Draw current character glyph
          if ((codepoint != ' ') && (codepoint != '\t')){
            DrawTextCodepoint(font, codepoint, (Vector2){ rec.x + text_offset_x, rec.y + text_offset_y }, font_size, isGlyphSelected? select_tint : tint);
          }
        }
        if(caret_index == i && caret_draw){
          DrawLine(rec.x + caretx + 1, rec.y + carety + 0, rec.x + caretx, rec.y + carety + (font.baseSize + font.baseSize/4)*scale_factor - 6 , WHITE);
          caret_draw = 0;
        }
      }

      if (word_wrap && (i == line_end)){
        text_offset_y += (font.baseSize + font.baseSize/4)*scale_factor;
        text_offset_x = 0;
        line_start = line_end;
        line_end = -1;
        glyph_width = 0;
        select_start += last_k - k;
        k = last_k;

        state = !state;
      }
    }

    text_offset_x += glyph_width;
  }
  
  if(!new_caret_found){
    new_caret_index = last_k;
  }
  
  return new_caret_index;
}

static void textbox_delete_selection(){
  int start = (textbox.caret_anchor < textbox.caret_current) ? (textbox.caret_anchor) : (textbox.caret_current);
  int length = textbox.caret_current - textbox.caret_anchor;
  if(length < 0){
    length *= -1;
  }
  arrdeln(textbox.text, start, length);
  textbox.caret_anchor  = start;
  textbox.caret_current = start;
}


