//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////

typedef struct Process Process;
typedef struct Process_Shape Process_Shape;
typedef struct Process_Selection Process_Selection;
typedef enum Process_Connection Process_Connection;
typedef enum Keybind_Result Keybind_Result;
typedef struct Keybind Keybind;
typedef struct Context Context;
function void clear_processes(Context *context);
function Process *create_process(Context *context);
function String_Chunk *create_string_chunk(Context *context);
function String_Chunk_List string_chunk_list_from_string8(Context *context, String8 string8);
function S32 collect_save_files(Context *context);
function Vector2 get_process_wire_position(Context *context, Process *p, Process_Shape shape, Process_Connection conn, U32 wire_index);
function Process *get_process_wire_by_selection(Context *context, Process_Selection selection);
function B32 is_active_process(Context *context, Process *p);
function void remove_process_from_active_processes(Context *context, Process *p);
function Keybind_Result check_keybind(Context *context, Keybind *keybind, Process_Selection selection);
function void clear_active_processes(Context *context);
function void exit_add_wire_mode(Context *context);
function void delete_process(Context *context, Process *p);



//////////////////////////////////////
// Process
//////////////////////////////////////

typedef enum {
  Process_Flag_Wire        = 1 << 0,
  Process_Flag_Empty       = 1 << 1,
  Process_Flag_Cup         = 1 << 2,
  Process_Flag_Cap         = 1 << 3,
  Process_Flag_Identity    = 1 << 4,
  Process_Flag_Drag_In     = 1 << 5,
  Process_Flag_Drag_Out    = 1 << 6,
  // UI features
  Process_Flag_TextEdit        = 1 << 7,
  Process_Flag_CanBeActive     = 1 << 8,
  Process_Flag_Clickable       = 1 << 9,
  Process_Flag_FitToText       = 1 << 10,
  Process_Flag_UseLabelCString = 1 << 11,
} Process_Flag;

#define Process_Connection_Xlist\
  X(In, 0) X(Out, 1)

enum Process_Connection {
#define X(conn, ...)\
  Process_Connection_##conn,
  Process_Connection_Xlist
#undef X
  Process_Connection__Count,
};

typedef enum {
#define X(conn, i)\
  Process_Connection_Flag_##conn = (1 << i),
  Process_Connection_Xlist
#undef X
} Process_Connection_Flag;


struct Process {
  //////////////
  // Members that need to be saved when serializing.
  //////////////
  B32 flags;
  Vector2 position;
  String_Chunk_List label;

  union {
    struct {
      Process *in;
      Process *out;
    };
    Process *conn[Process_Connection__Count];
  };

  union {
    struct {
      U32 which_in;
      U32 which_out;
    };
    U32 which_conn[Process_Connection__Count];
  };

  //////////////
  // Members that are "ephemeral", which can be ocnstructed from serialized members.
  //     or it's for UI...
  //////////////
  union {
    struct {
      S32 in_count;
      S32 out_count;
    };
    S32 conn_count[Process_Connection__Count];
  };

  void (*func)(Context*, Process*); // TODO: What do we do about this func? It's only used for UI elements, so maybe we should stop using Processes as UI elements and give up on the idea of process-ui?

  Vector2 size;
  Vector2 margin;
  U32 cold_index;
  Process *to_copied;

  Process *next;
  Process *next_active;

  U8 *label_c_string;
  U32 label_cursor;
};

// Process Trie
#define Proc_Trie_Use_Key_Value          1
#define Steady_Trie(ident)               Proc_Trie_##ident
#define steady_trie(ident)               proc_trie_##ident
#define Steady_Trie_Key_Bits             64
#define Steady_Trie_Slot_Bits            2
#define Steady_Trie_Root_Is_Lowest_Significant_Byte 0
#define Steady_Trie_Use_Key_Value_Pair   Proc_Trie_Use_Key_Value
#define Steady_Trie_Value_Type           Process
#define Steady_Trie_Default_Value        (Process){0}
#include "../libraries/steady_trie.h"
#define Proc_Trie_Iterate(iter_name, arena, trie)\
  for (Proc_Trie_Iterator *iter_name = proc_trie_iter_init(arena, trie->current_root->node);\
       proc_trie_iter_test(iter_name);\
       proc_trie_iter_next(iter_name))



typedef enum {
  Process_Selection__Null,
  Process_Selection_In,
  Process_Selection_Out,
  Process_Selection_NewWire,
  Process_Selection_Process,
} Process_Selection_Type;

struct Process_Selection {
  Process *process;
  Process_Selection_Type type;
  S32 index;
  B32 hot_id_assigned;
};

typedef struct {
  Process *first;
  Process *last;
} Process_List;

typedef struct {
  Vector2 first_point;
  Vector2 second_point;
  Vector2 first_control;
  Vector2 second_control;
  Vector2 middle_of_curve;
  Vector2 middle_of_line;
} Half_Circle_Points;



//////////////////////////////////////
// Process Shape
//////////////////////////////////////

typedef enum {
  Process_Shape_TriangleFan,
  Process_Shape_TriangleStrip,
  Process_Shape_Circle,
  Process_Shape_HalfCircle,
} Process_Shape_Kind;


struct Process_Shape {
  Process_Shape_Kind kind;
#define Process_Shape_Max_Points 16
  Vector2 points[Process_Shape_Max_Points];
  S32 triangle_count;
  F32 radius;
  S32 point_count;
  Vector2 center;
  Vector2 first_control;
  Vector2 second_control;
  B32 downward;
  Vector2 new_wire_position;
};


//////////////////////////////////////
// UI
//////////////////////////////////////

typedef enum {
  Ui_Align_Top,
  Ui_Align_TopLeft,
  Ui_Align_Left,
  Ui_Align_BottomLeft,
  Ui_Align_Bottom,
  Ui_Align_BottomRight,
  Ui_Align_Right,
  Ui_Align_TopRight,
} Ui_Align;

typedef enum {
  Ui_Layout_None,
  Ui_Layout_Vertical,
  Ui_Layout_Horizontal,
} Ui_Layout;

typedef enum {
  Ui_Sizing_None,
  Ui_Sizing_FitContents,
  Ui_Sizing_FitContentsX,
  Ui_Sizing_FitContentsY,
} Ui_Sizing;

typedef enum {
  Ui_Box_Flag_ShouldDraw = (1 << 0),
  Ui_Box_Flag_Clip       = (1 << 1),
  Ui_Box_Flag_ScrollY    = (1 << 2),
  Ui_Box_Flag_Stretch    = (1 << 3),
} Ui_Box_Flag;

typedef struct Ui_Box Ui_Box;
struct Ui_Box {
  Ui_Box *next;
  Vector2 position;
  Vector2 offset;
  Vector2 scroll_offset;
  Vector2 raw_size;
  Vector2 min_size;
  Vector2 max_size;
  U32 flags;
  Color color;
  Ui_Align align;
  Ui_Layout layout;
  Ui_Sizing sizing;
};

typedef struct {
  Ui_Box *first;
  Ui_Box *last;
} Ui_Box_List;


#define Ui_Default_Position (Vector2){0.0f, 0.0f}
#define Ui_Default_Offset   (Vector2){0.0f, 0.0f}
#define Ui_Default_Align    Ui_Align_TopLeft
#define Ui_Default_Layout   Ui_Layout_None
#define Ui_Default_Sizing   Ui_Sizing_None



//////////////////////////////////////
// Context
//////////////////////////////////////


// TODO: maybe this should be a mode and not flags?
typedef enum {
  Context_Flag_Dragging       = 1 << 0,
  Context_Flag_Bounding       = 1 << 1,
  Context_Flag_Panning        = 1 << 2,
  Context_Flag_NewWire        = 1 << 3,
  Context_Flag_RoundedShapes  = 1 << 4,
} Context_Flag;


typedef enum {
  Ui_State_Flag_mouse0_pressed  = 1 << 0,
  Ui_State_Flag_mouse1_pressed  = 1 << 1,
  Ui_State_Flag_mouse0_down     = 1 << 2,
  Ui_State_Flag_mouse1_down     = 1 << 3,
  Ui_State_Flag_hot_id_assigned = 1 << 4,
  Ui_State_Flag_control_down    = 1 << 5,
  Ui_State_Flag_shift_down      = 1 << 6,
  Ui_State_Flag_alt_down        = 1 << 7,
  Ui_State_Flag_action_occured  = 1 << 8,
} Ui_State_Flag;

typedef struct {
  U32 flags;
  Vector2 mouse_position;
  Vector2 mouse_wheel_movement;

#define Max_Key_Presses_Per_Frame 256
  U32 key_presses[Max_Key_Presses_Per_Frame];
} Ui_State;

typedef enum {
  Menu_State__Null,
  // top menu states
  Menu_State_FileMenu,
  Menu_State_EditMenu,
  // other menu states
  Menu_State_OpenFile,
  Menu_State_SaveFileAs,
} Menu_State;

#define Top_Menu_Index(menu_state) ((menu_state) - Menu_State_FileMenu)
#define Menu_State_From_Top_Menu_Index(index) ((index) + Menu_State_FileMenu)
#define Top_Menu_Count  (Top_Menu_Index(Menu_State_EditMenu)+1)

#define Has_Active_Menu_Element(context)\
  ((context)->menu_state >= Menu_State_FileMenu &&\
   (context)->menu_state <= Menu_State_EditMenu)

struct Context {
  Arena *render_arena;
  Arena *permanent_arena;
  Arena *ui_arena;
  Arena *temp_arena;
  Arena *per_frame_arena;

  U32 flags;

  Proc_Trie_Trie *proc_trie;
  Process_List processes;
  Process_List free_processes;
  Process_List free_ui_elements;
  String_Chunk_List free_strings;

  Process *hot_process;
  Process_List active_processes;

  Process_List copy_processes;

  Process_List save_file_list;
  Process *selected_element; // Use this for things like picking (button click) a file to open.
  Ui_Box_List ui_box_stack;

  Render_Context ui_render_context;
  Render_Context process_render_context;

  Ui_State ui_state;
  Menu_State menu_state;
  Vector2 active_position;
  Vector2 copy_center;

  String_Chunk_List save_file_name;

  Camera2D camera;
};
