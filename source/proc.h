typedef struct Piece_Table_Row Piece_Table_Row;
typedef struct Piece_Table_Chunk Piece_Table_Chunk;
typedef struct Piece_Table_Memory {
  Piece_Table_Row *free_rows;
  Piece_Table_Chunk *free_chunks;
} Piece_Table_Memory;




//////////////////////////////////////
// UI
//////////////////////////////////////

typedef U8 Ui_Align;
enum Ui_Align {
  Ui_Align_Top         = (U8)0,
  Ui_Align_TopLeft     = (U8)1,
  Ui_Align_Left        = (U8)2,
  Ui_Align_BottomLeft  = (U8)3,
  Ui_Align_Bottom      = (U8)4,
  Ui_Align_BottomRight = (U8)5,
  Ui_Align_Right       = (U8)6,
  Ui_Align_TopRight    = (U8)7,
  Ui_Align_Center      = (U8)8,
};

typedef U8 Ui_Layout;
enum Ui_Layout {
  Ui_Layout_None       = (U8)0,
  Ui_Layout_Vertical   = (U8)1,
  Ui_Layout_Horizontal = (U8)2,
};

typedef U8 Ui_Sizing;
enum Ui_Sizing {
  Ui_Sizing_None         = (U8)0,
  Ui_Sizing_FitContents  = (U8)1,
  Ui_Sizing_FitContentsX = (U8)2,
  Ui_Sizing_FitContentsY = (U8)3,
};

typedef enum Ui_Box_Kind {
  Ui_Box_Kind__Null,
  Ui_Box_Kind_OpenFile,
  Ui_Box_Kind_SaveFileAs,
} Ui_Box_Kind;

typedef enum {
  Ui_Box_Flag_Clip           = (1 << 0), // TODO: does this flag conflict with the View's clip flag?
  Ui_Box_Flag_ScrollY        = (1 << 1),
  Ui_Box_Flag_Stretch        = (1 << 2),
  Ui_Box_Flag_OnlyOneActive  = (1 << 3),
} Ui_Box_Flag;

typedef struct Ui_Box {
  Ui_Box_Kind kind;
  Vector2 position;
  Vector2 scroll_offset;
  Vector2 layout_offset;
  Vector2 size;
  Vector2 min_size;
  Vector2 max_size;
  U32 flags;
  Color color;
  Ui_Align align;
  Ui_Layout layout;
  Ui_Sizing sizing;
} Ui_Box;

typedef struct {
  Ui_Box *first;
  Ui_Box *last;
} Ui_Box_List;







typedef struct Process Process;
typedef struct Context Context;
typedef struct View View;

typedef enum Ref_Kind {
  Ref_Kind__Null,
  Ref_Kind_ProcTrie,
  Ref_Kind_ProcTrieNode,
  Ref_Kind_ProcTrieRoot,
} Ref_Kind;


#define Process_Flag_Xlist(X)\
  X( Wire              )\
  X( Empty             )\
  X( Cup               )\
  X( Cap               )\
  X( Identity          )\
  X( Drag_In           )\
  X( Drag_Out          )\
  X( Invisible         )\
  X( IsBox             )\
  X( IsActive          )\
  X( TextEdit          )\
  X( CanBeActive       )\
  X( Clickable         )\
  X( FitToText         )\
  X( IsDetached        )\
  X( Line              )\
  X( UiDescendIfActive )\
  X( UiShowIfActive    )

typedef enum {
#define X(name, ...)\
  Process_Flag_Kind_##name,
  Process_Flag_Xlist(X)
#undef X
} Process_Flag_Kind;

typedef enum {
#define X(name, ...)\
  Process_Flag_##name = 1 << (Process_Flag_Kind_##name),
  Process_Flag_Xlist(X)
#undef X
} Process_Flag;




#include "../source/core.h" // TODO: move this




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
typedef struct Process_Ref Process_Ref;

typedef struct Keybind_Environment Keybind_Environment;

typedef enum Keybind_Result Keybind_Result;
typedef enum Process_Do_Undo_Kind Process_Do_Undo_Kind; // TODO: delete?
typedef enum Process_Do_Undo_Kind_Flag Process_Do_Undo_Kind_Flag; // TODO: delete?
typedef struct Keybind Keybind;


function         V2_Chunk *create_v2_chunk(Context *context);
function               B32 rectangle_contains_point(Rectangle r, Vector2 p);
function           Vector2 get_wire_position_from_wire(Context *context, View *view, Process *wire, Process_Shape shape, Process_Connection conn);
function          Process *get_wire_from_selection(Context *context, View *view, Process_Selection selection);
function     Process_Shape get_process_shape(Context *context, View *view, Process *p);
function         Rectangle get_selection_rectangle(Context *context);
function           Vector2 get_process_position(Context *context, View *view, Process *process);
function Process_Selection get_process_selection(Context *context, View *view, Process *p);
function               B32 is_active_process(Context *context, View *view, Process *p);
function              void remove_process_from_active_processes(Context *context, View *view, Process *p);

function              void copy_active_processes(Context *context, View *view);
function              void paste_processes(Context *context, View *view);

function              void handle_label_editing(Context *context, View *view, Process_List ps);

function              void set_save_file_as_as_active_element(Context *context, View *view, Process *element);
function              void handle_copy(Context *context, View *view, Process *element);
function              void handle_paste(Context *context, View *view, Process *element);











typedef struct Half_Circle_Points {
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
  Vector2 *points_ptr;
  S32 triangle_count;
  F32 radius;
  S32 point_count;
  Vector2 center;
  Vector2 first_control;
  Vector2 second_control;
  B32 downward;
  Vector2 new_wire_position;
};

typedef struct Bezier_Points {
  Buffer_V2 controls;
  Buffer_V2 path;
} Bezier_Points;



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
  Ui_State_Modifier_Flag_control_down    = 1 << 0,
  Ui_State_Modifier_Flag_shift_down      = 1 << 1,
  Ui_State_Modifier_Flag_alt_down        = 1 << 2,
  Ui_State_Modifier_Flag_super_down      = 1 << 3,
} Ui_State_Modifier_Flag;

typedef enum {
  Ui_State_Flag_mouse0_pressed  = 1 << 0,
  Ui_State_Flag_mouse1_pressed  = 1 << 1,
  Ui_State_Flag_mouse0_down     = 1 << 2,
  Ui_State_Flag_mouse1_down     = 1 << 3,
  Ui_State_Flag_hot_id_assigned = 1 << 4,
  Ui_State_Flag_action_occured  = 1 << 5,
} Ui_State_Flag;

typedef struct {
  F32 frame_delta;
  F32 last_frame_time;

  Ui_State_Flag flags;
  Ui_State_Modifier_Flag modifier_flags;
  U32 kb_action;

  Vector2 mouse_position;
  Vector2 active_position;
  B32 mouse_moved;
  Vector2 mouse_wheel_movement;

#define Max_Key_Presses_Per_Frame 256
  U32 key_presses[Max_Key_Presses_Per_Frame];
} Ui_State;






#define View_Kind_Xlist(X)\
  X(Procs)\
  X(Ui)\
  X(Trie)

typedef enum View_Kind {
#define X(n)\
  View_Kind_##n,
  View_Kind_Xlist(X)
#undef X
  View_Kind__Count,
} View_Kind;

typedef enum View_Kind_Flag {
#define X(n)\
  View_Kind_Flag_##n = (1 << (View_Kind_##n)),
  View_Kind_Xlist(X)
#undef X
} View_Kind_Flag;

typedef enum View_Layout {
  View_Layout__Null,
  View_Layout_Horizontal,
  View_Layout_Vertical
} View_Layout;

typedef enum View_Flag {
  View_Flag_Active   = 1 << 0,
  View_Flag_Panning  = 1 << 1,
  View_Flag_Editable = 1 << 2,
  View_Flag_Clip = 1 << 3, // TODO: does this clip conflict with Process' clip flag?
} View_Flag;



struct View {
  View_Kind_Flag kind_flags;
  U32 flags;
  Rectangle screen_region;
  Camera2D camera;
  Proc_History history;
  Process *root_process;
  Color color;

  View_Layout layout;
  View *next;
  View *first;
  View *last;
};



typedef struct Process_Loc {
  View *view;
  Process *process;
} Process_Loc;






struct Context {
  Proc_Core core;
  Arena *render_arena;
  Arena *ui_arena;

  Keybind *keybinds;
  U32 keybind_count;

  U32 flags;

  Process_List free_processes;
  String_Chunk_List free_strings;
  V2_Chunk *free_v2_chunks;

  Process_Loc hot_process;
  // TODO: does copy_processes need to be per-view? What about hot_process?
  Process_List copy_processes;

  Process_List save_file_list;
  Process *selected_element; // Use this for things like picking (button click) a file to open.

  Render_Context ui_render_context;
  Render_Context process_render_context;

  Ui_State ui_state;
  Vector2 copy_center;

  U8 *save_file_name;

  View *root_view;

  Piece_Table_Memory piece_table_memory;

  F32 time_to_wait_for_label_edit; // TODO: move edit-timeout stuff to ui_state?
  F32 edit_timeout;
};




typedef struct View_Stack {
  struct View_Stack *next;
  View *view;
  B32 visited;
} View_Stack;


function View_Stack *view_iter_init(Context *context) {
  View_Stack *stack = push_struct(context->core.per_frame_arena, View_Stack);
  if (stack) {
    stack->view = context->root_view;
  }
  return stack;
}


function void view_iter_next(Context *context, View_Stack **stack) {
  if (stack && *stack && (*stack)->view) {
    if (!(*stack)->visited && (*stack)->view->first && (*stack)->view->last) {
      (*stack)->visited = 1;
      // view has children, so descend
      View_Stack *new_stack = push_struct(context->core.per_frame_arena, View_Stack);
      if (new_stack) {
        new_stack->view = (*stack)->view ? (*stack)->view->first : 0;
        SLLStackPush((*stack), new_stack);
      }
      else {
        *(*stack) = (View_Stack){0};
      }
    }
    else if ((*stack)->view->next) {
      // move to the sibling
      (*stack)->view = (*stack)->view->next;
      (*stack)->visited = 0;
    }
    else {
      // no more siblings, so pop
      SLLStackPop(*stack);
    }
  }
}


#define View_Iterate(view_name, ctx)\
  for (View_Stack *view_name = view_iter_init(ctx);\
       (view_name != 0 && view_name->view != 0);\
       view_iter_next((ctx), &view_name))







