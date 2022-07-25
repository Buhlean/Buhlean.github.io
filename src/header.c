
enum palette_colours{
  BACKGROUND,
  TAB_INACTIVE,
  TAB_ACTIVE,
  TEXT_INACTIVE,
  TEXT_ACTIVE,
  BORDER,
  MENU_BG,
  HIGHLIGHT,
  CARET,
  
  PALETTE_COUNT,
};

static Color dark_mode[PALETTE_COUNT] = {   
  (Color){39,39,39,255},                    // BACKGROUND,
  (Color){24,24,24,255},                    // TAB_INACTIVE,
  (Color){76,76,76,255},                    // TAB_ACTIVE,
  (Color){200,200,200,255},                 // TEXT_INACTIVE,
  (Color){240,240,240,255},                 // TEXT_ACTIVE,
  (Color){200,200,200,255},                 // BORDER,
  (Color){50,50,50,255},                    // MENU_BG,
  (Color){240,133,0,255},                   // HIGHLIGHT,
  WHITE,                                    // CARET,
};                                             
static Color light_mode[PALETTE_COUNT] = {     
  RAYWHITE,                                 // BACKGROUND,
  (Color){24,24,24,255},                    // TAB_INACTIVE,
  (Color){76,76,76,255},                    // TAB_ACTIVE,
  (Color){20,20,20,255},                    // TEXT_INACTIVE,
  BLACK,                                    // TEXT_ACTIVE,
  (Color){32,32,32,255},                    // BORDER,
  RAYWHITE,                                 // MENU_BG,
  (Color){240,133,0,255},                   // HIGHLIGHT,
  BLACK,                                    // CARET,
};
Color *palette = light_mode;

enum {
#if defined(PLATFORM_WEB)
  STARTING_SCREENWIDTH = 1750,
  STARTING_SCREENHEIGHT = 850,
#else
  STARTING_SCREENWIDTH = 1845,
  STARTING_SCREENHEIGHT = 1050,
#endif
  FPS_GOAL = 20,
  MENU_NAME_LENGTH = 36,
  WORD_SPACING = 7,
  BLINK_FRAMES = 5,
  RENDERTEXTURE_WIDTH = 1920,
  RENDERTEXTURE_HEIGHT = 1080,
};

// starting indices for UI
enum{
  WIDGETS_MENU = 0,
  WIDGETS_TEXTBOX = 400,
};


enum ICONS{
  ICON_X,
  ICON_HOME,
  ICON_LOCKED,
  ICON_EDITABLE,
  ICON_VISIBLE,
  ICON_INVISIBLE,
  ICON_BACK,
  ICON_FIRST,
  ICON_REVERSE,
  ICON_PLAY,
  ICON_END,
  ICON_FORWARD,
  ICON_ROTATE,
  ICON_SHEET,
  ICON_LAYERS,
  ICON_SAVE,
  ICON_OPEN,
    ICON_EMPTY,
  ICON_ZOOMOUT,
  ICON_ZOOMIN,
  
  ICON_COUNT
};
struct TextureAtlas{
  int pixel_width, pixel_height;
  int atlas_width, atlas_height;
};

struct globals{
  int width, height;
  
  int font_size_menu, font_size_menu_entry;
  int font_size_tab_label, font_size_layer_label; 
  int font_size_dialog_heading, font_size_dialog;
  float scale;
  
  int border, padding;
  int menu_height, tab_height;
  
  struct TextureAtlas icon_atlas;
  Texture icons;
  
  char **filenames;
  int drop_counter;
  char **last_dropped_files;
  int last_dropped_count;
};
static struct globals *g = { 0 };

struct menu{
  char name[MENU_NAME_LENGTH];
  int entry_count;
  int sub_menu_index;
  int functionality_index;
};

struct UI_state{
  int hover, clicked, dragged;
  
  Vector2 drag_start, drag_offset;
  
  float compile_cd;
  int dark_mode;
  
  int last_pressed_key_for_hold_action;
  float last_key_pressed_hp_time;
  float last_key_hold_action_acc;
  
  float caret_blink_hp_time_acc;
  
  int menu_tab_current;
  int menu_layer3_current;
  
  int menu_array_count;
  struct menu *top_menu;
  
  int menu_rightclick_current;
  
  int menu_righclick_count;
  struct menu *rightclick_menu;
};
static struct UI_state *state = NULL;

Font font;
Font font_mono;

enum CONNECTION_TYPE{
  CT_NULL = 0,
  CT_LINE,
  CT_LINE_THICK,
  CT_LINE_DOTTED,
  CT_ARROW,
  CT_ARROW_THICK,
  CT_ARROW_DOTTED,
  CT_BIDIR,
  CT_BIDIR_THICK,
  
};
enum GRAPH_DIRECTION{
  DIR_TB = 1, //TD
  DIR_BT = 2, 
  DIR_RL = 3,
  DIR_LR = 4,
  DIR_CE = 5,
  DIR_EC = 6,
};
enum BUBBLE_STYLE{
  BUBBLE_STANDARD,
  BUBBLE_RECTANGLE,   // [___]
  BUBBLE_ROUNDED,
  BUBBLE_TRAPEZOID,   // /___\ .
  BUBBLE_TRAPEZOID_R, // \___/ .
  BUBBLE_FSLANTED,    // /___/ .
  BUBBLE_BSLANTED,    // \___\ .
  BUBBLE_CIRCLE,
  BUBBLE_CYLINDER,    //database
  BUBBLE_SUBROUTINE,
  BUBBLE_STADIUM,     // (___)
  BUBBLE_HEXAGON,
  BUBBLE_RHOMBUS,
  BUBBLE_BUBBLE,      // no text, just bubble
  
};

struct Bubble{
  char *key;
  char *text;
  int   text_length;
  int depth;
  int subgraph_entry_index;
  int style;
  enum BUBBLE_STYLE bubble_type;
  int outgoing_count; 
  int incoming_count;
  int is_subgraph_plus_direction;
  Vector2 pos;
};
struct kv{
  char *key;
  int value;
};

struct Connection{
  int A_i;
  int B_i;
  enum CONNECTION_TYPE conn_type;
  char *text;
  int   text_length;
};

struct remember_graph_info{
  int bubble_count;
  int conn_count;
  Vector2 *positions;
};

struct render_info{
  char *keys;
  struct Bubble *bubbles;
  struct Connection *connections;
  struct kv *bubble_hm;
  int *conn_list_A, *conn_list_B;
  int subgraph_count;
  RenderTexture2D render_output;
  char *compiler_errors;
  struct remember_graph_info remember_graph_info;
};
struct render_info *r = NULL;

struct TextBox{
  char *text;
  Rectangle collider;
  size_t text_hash;
  int caret_anchor; 
  int caret_current;
  int blink_acc;
};
struct TextBox textbox = { 0 };

static int debug_switch = 1;



