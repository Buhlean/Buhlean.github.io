
static void mymemzero(void *p, int size){
  if(!p) return;
  for(int i = 0; i < size; i++){
    ((unsigned char *)p)[i] = 0;
  }
}
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
// modified to make dealing with various glyph sizes easier (due to monospaced font)
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
  
  // pulling out the glyph measuring out of the loop
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


