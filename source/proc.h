#ifndef PROC_INCLUDE_H
# define PROC_INCLUDE_H
//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
#define Process_Connection_Xlist\
  X(In, 0) X(Out, 1)

enum Process_Connection {
#define X(conn, ...)\
  Process_Connection_##conn,
  Process_Connection_Xlist
#undef X
  Process_Connection__Count,
};

enum Process_Connection_Flag {
#define X(conn, i)\
  Process_Connection_Flag_##conn = (1 << i),
  Process_Connection_Xlist
#undef X
};

typedef struct Process Process;
typedef struct Context Context;

typedef enum Ref_Kind {
  Ref_Kind__Null,
  Ref_Kind_ProcTrie,
  Ref_Kind_ProcTrieNode,
  Ref_Kind_ProcTrieRoot,
} Ref_Kind;

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

  Ref_Kind ref_kind;
  void *ref;
};

// Process Trie
#define Proc_Trie_Key_Bits               64
#define Proc_Trie_Slot_Bits              2
#define Proc_Trie_Use_Key_Value          1
#define Steady_Trie(ident)               Proc_Trie_##ident
#define steady_trie(ident)               proc_trie_##ident
#define Steady_Trie_Key_Bits             Proc_Trie_Key_Bits
#define Steady_Trie_Slot_Bits            Proc_Trie_Slot_Bits
#define Steady_Trie_Root_Is_Least_Significant_Byte 1
#define Steady_Trie_Use_Key_Value_Pair   Proc_Trie_Use_Key_Value
#define Steady_Trie_Value_Type           Process
#define Steady_Trie_Default_Value        (Process){0}
#define Steady_Trie_Use_Debug_Log        0
#include "../libraries/steady_trie.h"
#define Proc_Trie_Iterate(iter_name, arena, trie)\
  for (Proc_Trie_Iterator *iter_name = proc_trie_iter_init(arena, trie->current_root->node);\
       proc_trie_iter_test(iter_name);\
       proc_trie_iter_next(iter_name))

typedef struct View View;

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
  View *view;
};

typedef struct Process_Shape Process_Shape;
typedef struct Process_Selection Process_Selection;
typedef struct Process_List Process_List;
typedef struct Process_Ref Process_Ref;

typedef struct Editable_Process {
  B32 is_being_edited;
  Process process;
} Editable_Process;

typedef struct Process_Edit {
  Proc_Trie_Edit_Kind edit_kind;
  Process *process;
  Process new_process;
  Process *new_process_ptr;
  struct Process_Edit *next;
} Process_Edit;

typedef struct Process_Edit_List {
  Process_Edit *first;
  Process_Edit *last;
} Process_Edit_List;

enum Keybind_Result {
  Keybind_Result__Null,
  Keybind_Result_Enter,
  Keybind_Result_Exit,
};

typedef struct Keybind_Environment Keybind_Environment;

typedef enum Process_Connection Process_Connection;
typedef enum Process_Connection_Flag Process_Connection_Flag;
typedef struct Connection_Result Connection_Result;
typedef enum Keybind_Result Keybind_Result;
typedef struct Keybind Keybind;
function              void clear_process_list(Context *context, Process_List *list);
function              void clear_active_processes(Context *context);
function              void clear_ds_view_process_list(Context *context);
function          Process *create_detached_process(Context *context);
function          Process *create_process(Context *context);
function     String_Chunk *create_string_chunk(Context *context);
function String_Chunk_List string_chunk_list_from_string8(Context *context, String8 string8);
function               S32 collect_save_files(Context *context);
function               B32 rectangle_contains_point(Rectangle r, Vector2 p);
function           Vector2 get_process_wire_position(Context *context, View *view, Process *p, Process_Shape shape, Process_Connection conn, U32 wire_index);
function          Process *get_process_wire_by_selection(Context *context, Process_Selection selection);
function     Process_Shape get_process_shape(Context *context, View *view, Process *p);
function           Vector2 get_process_size(Context *context, Process *p, Process_Shape shape);
function         Rectangle get_selection_rectangle(Context *context);
function           Vector2 get_process_position(Context *context, View *view, Process *process);
function Process_Selection get_process_selection(Context *context, View *view, Process *p);
function               B32 is_active_process(Context *context, Process *p);
function              void remove_process_from_active_processes(Context *context, Process *p);
/* function    Keybind_Result check_keybind(Context *context, Keybind *keybind, Process_Selection selection); */
/* function    Keybind_Result check_keybind(Context *context, Keybind_Environment *keybind_env, Process_Selection selection); */
function    Keybind_Result check_keybind(Keybind_Environment *keybind_env);
function              void exit_add_wire_mode(Context *context);
function              void delete_process(Context *context, Process *p, Process_Connection_Flag which_conn_flags);
function              void copy_active_processes(Context *context);
function              void paste_processes(Context *context);
function              void gather_processes_from_trie(Context *context);

function              void remove_process_from_process_list(Context *context, Process_List *list, Process *p);
function          Process *connect_detached_processes(Context *context, Process *out, Process *in);
function Connection_Result connect_processes_no_gather(Context *context, Process *out, Process *in);
function Connection_Result connect_processes(Context *context, Process *out, Process *in);
function              void delete_wire(Context *context, Process *wire, Process_Connection_Flag conn_flags);
function              void add_wire_connection(Context *context, Process *wire, Process *process, Process_Connection conn, U32 which_conn);
function              void handle_label_editing(Context *context, Process_List ps);
function          Process *find_process_connection(Context *context, Process *p, Process_Connection conn, U32 which_conn);



//////////////////////////////////////
// Process
//////////////////////////////////////

#define Process_Flag_Xlist(X)\
  /* Name               Shift */\
  X( Wire            ,  0      )\
  X( Empty           ,  1      )\
  X( Cup             ,  2      )\
  X( Cap             ,  3      )\
  X( Identity        ,  4      )\
  X( Drag_In         ,  5      )\
  X( Drag_Out        ,  6      )\
  X( Invisible       ,  7      )\
  X( AsBox           ,  8      )\
  X( RefIsActive     ,  9      )\
  X( TextEdit        , 10      )\
  X( CanBeActive     , 11      )\
  X( Clickable       , 12      )\
  X( FitToText       , 13      )\
  X( UseLabelCString , 14      )\
  X( IsDetached      , 15      )\
  X( Line            , 16      )


typedef enum {
#define X(name, shift, ...)\
  Process_Flag_##name        = 1 << (shift),
  Process_Flag_Xlist(X)
#undef X
} Process_Flag;

typedef enum {
#define X(name, shift, ...)\
  Process_Flag_Kind_##name        = (shift),
  Process_Flag_Xlist(X)
#undef X
} Process_Flag_Kind;

struct Connection_Result {
  Process *out;
  Process *in;
  Process *new_wire;
};




struct Process_Ref {
  Process *process;
  struct Process_Ref *next;
};





struct Process_List {
  Process *first;
  Process *last;
};

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


// NOTE: Flags like Context_Flag_AutoAlignChains are defined in Keybinds, so this Context_Flag enum probably also needs to be determined at link-time.
typedef enum {
  Context_Flag_Dragging           = 1 << 0,
  Context_Flag_Bounding           = 1 << 1,
  Context_Flag_NewWire            = 1 << 2,
  Context_Flag_RoundedShapes      = 1 << 3,
  Context_Flag_DataStructureView  = 1 << 4,
  Context_Flag_AutoAlignChains    = 1 << 5,
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
  Ui_State_Flag_super_down      = 1 << 8,
  Ui_State_Flag_action_occured  = 1 << 9,
} Ui_State_Flag;

typedef struct {
  U32 flags;
  U32 kb_action;

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

#define View_Count  5

typedef enum View_Kind {
  View_Kind_Procs,
  View_Kind_Trie,
  View_Kind__Count,
} View_Kind;

typedef enum View_Flag {
  View_Flag_Active   = 1 << 0,
  View_Flag_Panning  = 1 << 1,
  View_Flag_Editable = 1 << 2,
} View_Flag;

struct View {
  U32 flags;
  Rectangle screen_region;
  Camera2D camera;
  Process_List processes;
  Process_List active_processes;
};

typedef struct Process_Loc {
  View *view;
  Process *process;
} Process_Loc;

struct Context {
  Arena *render_arena;
  Arena *permanent_arena;
  Arena *ui_arena;
  Arena *temp_arena;
  Arena *per_frame_arena;

  U32 flags;

  Proc_Trie_Trie *proc_trie;
  Process_List free_processes;
  Process_List free_ui_elements;
  String_Chunk_List free_strings;

  Process_Loc hot_process;
  // TODO: do active/copy_processes need to be per-view? What about hot_process?
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

  View views[View_Count];

  Process_Edit_List process_edit_list;
};


////////////////////////
// Shared Globals
////////////////////////
global_variable Process global_null_process;
#define The_Null_Process() (global_null_process=(Process){0}, &global_null_process)



////////////////////////
// Integer as c-string
////////////////////////
#define Integer_C_String_Lookup_Xlist(X)\
  X( 0) X( 1)\
  X( 2) X( 3)\
  X( 4) X( 5) X( 6) X( 7)\
  X( 8) X( 9) X(10) X(11) X(12) X(13) X(14) X(15)\
  X(16) X(17) X(18) X(19) X(20) X(21) X(22) X(23) X(24) X(25) X(26) X(27) X(28) X(29) X(30) X(31)\
  X(32) X(33) X(34) X(35) X(36) X(37) X(38) X(39) X(40) X(41) X(42) X(43) X(44) X(45) X(46) X(47) X(48) X(49) X(50) X(51) X(52) X(53) X(54) X(55) X(56) X(57) X(58) X(59) X(60) X(61) X(62) X(63)

#define Integer_C_String_Lookup_Size_Log2   6
StaticAssert(Integer_C_String_Lookup_Size_Log2 <= 6, integer_c_string_lookup_cant_be_too_big);
#define Integer_C_String_Lookup_Size        (1 << Integer_C_String_Lookup_Size_Log2)
#define Integer_C_String_Lookup_Mask        (Integer_C_String_Lookup_Size - 1)

global_variable char *global_integer_c_string_lookup[Integer_C_String_Lookup_Size] = {
#define X(i)\
  [i] = #i,
  Integer_C_String_Lookup_Xlist(X)
#undef X
};

global_variable char *global_trie_root_c_string_lookup[Integer_C_String_Lookup_Size] = {
#define X(i)\
  [i] = "root " #i,
  Integer_C_String_Lookup_Xlist(X)
#undef X
};

global_variable char *global_trie_node_c_string_lookup[Integer_C_String_Lookup_Size] = {
#define X(i)\
  [i] = "node " #i,
  Integer_C_String_Lookup_Xlist(X)
#undef X
};

#define Get_C_String_From_Integer(i)\
  (global_integer_c_string_lookup[(i) & Integer_C_String_Lookup_Mask])

#define Get_Trie_Root_C_String_From_Integer(i)\
  (global_trie_root_c_string_lookup[(i) & Integer_C_String_Lookup_Mask])

#define Get_Trie_Node_C_String_From_Integer(i)\
  (global_trie_node_c_string_lookup[(i) & Integer_C_String_Lookup_Mask])


#endif // PROC_INCLUDE_H
