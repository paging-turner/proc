/*
   Proc is intended to be an app to help edit diagrams for doing exercises in "Picturing Quantum Processes" found at https://www.cs.ox.ac.uk/people/aleks.kissinger/PQP.pdf

   The initial focus is to offer basic diagram editing features in order to communicate concepts from the book. At some point it would be nice to implement compilation features and simulation. Baby steps...

   TODO:
   [x] Fix naming collisions with Raylib and "Windows.h" !!!

   [x] Allow deleting of processes
     [x] BUG: Connect two processes with two wires. Delete the last wire. The processes do not update their in/out counts and look too wide.
   [x] BUG: Wires seems to have invisible, interactive parts. Since wires aren't given a position, you can click the top-left of the screen and select wires. This should not be allowed.
   [ ] File save and load
   [ ] Processes should expand to contain their label
   === Wires ===
     [x] Allow reordering of connected wires
     [x] BUG: While making a new wire-connection, if you click on another wire-box, the new-wire jumps to a place off the screen. Clicking on a process while in this busted state makes a new wire that leads to some invisible process.
     [ ] Preview wire movement while dragging wire.
   === Multi-select ===
     [x] Allow multi-selection of processes
     [x] Allow dragging all selected processes
     [x] Allow de-selecting of processes
     [x] Click-and-drag selection rectangle
     [ ] Ctrl-click-and-drag to include more processes (this collides with process creation right now...)
   === Zooming and Panning ===
     [x] BUG: When zoomed way out, the wire positioning gets messed up.
   === Coordinates ===
     [ ] We use the word "position" a lot, but sometimes it's screen-pos and sometimes world-pos. Should probably distinguish between the two to avoid confusion.
   === Graphics ===
     [ ] Replace line-drawing calls with a call that draws triangle-strips/fans. This should help deal with how to cleanly connect the ends of lines together.
   === Keybind Config ===
     [ ] Test some features of the new keybind-config file
       [ ] Comments
       [ ] Line without semi-color
       [ ] Line without equals
       [ ] Multiple definitions for same feature (should overwrite)
       [ ] Multiple key-kind definitions for single feature (should error)
     [ ] Add in error messages for tokenizing/parsing errors.
     [ ] Rewrite example-config to have good-instructions to help users write their own config.
     [ ] Add ability to define bindings for mouse buttons and mouse-wheels
   === Testing ===
     [ ] Enumerate some test cases, to at least be able to manually check that things are working.
     [ ] Automated test??
   === Copy/Paste ===
     [x] Copy-paste of selected processes
     [x] BUG: Connect two processes by a single wire. Select the wire *first* and then one or both of the other processes. Copy and paste. There will be extra processes pasted.
     [ ] Allow cutting processes
   [ ] Use a font other than the raylib default
   [ ] Expand base-layer and let it consume core.h and ryn_memory.h
   [ ] Make some sliders/fields for global settings like process-size and font-size.
   === Text Editing ===
     [ ] Show cursor when editing the text of a process.
     [ ] Allow moving the cursor to the middle of the text.
     [ ] Allow selecting regions of text.
     [ ] Cut/copy/paste
   [x] Allow toggling on/off "mr4th style" process drawing, which is a variation on the visual style of diragrams in the book.
     [x] Move towards defining shapes using triangle strips/fans. We used some raylib funcs for circles and stuff just because it was easy, but now we need more control.
     [x] Implement collision detection for triangle strip/fan so we can just define a shape with triangles and be able to interact and draw with the same shape.
   [x] BUG: If you toggle a process to be a special display (cup/cap/invisible), and then connect a new wire to it, the special visual still applies and you cannot toggle away. When connecting wires, we need to check if the special display flag should be unset.
   === Undo/redo ===
     [ ]
   [x] BUG: Connect a two processes. Make one process invisible. Delete the *other* process. The invisible process is still there but, well, you can't see it! Either delete the invisible one, or make it visible again. Probably just delete it??
   [x] Automatically show invisible processes. This should only apply to processes with no connections, set to invisible, and no text.
   === UI / Process-Interaction ===
     [x] Merge structs for ui-element and process. It will make memory management easier and give more use-cases of processes with different properties than what's covered in PQP.
   [ ] New Arena Changes
     [x] Change how we loop through processes, so that we can enable growable arenas.
     [x] Right now we have to make sure the Process struct is a size that's a multiple of a pointer. We should fix that :(
     [ ] Enable growable arenas
*/





#include <stdio.h> // printf, fopen

#define MR4TH_NO_INCLUDES 1
#define MR4TH_NO_CLAMP 1
#if !No_Assert
# define MR4TH_ASSERTS 1
#endif
#include "../libraries/mr4th/src/mr4th_base.h"
#define push_struct(a, s) arena_push((a), sizeof(s))



#if OS_WINDOWS
# include "../libraries/raylib-5.5_win32_msvc16/include/raylib.h"
# include "../libraries/raylib-5.5_win32_msvc16/include/raymath.h"
#elif OS_MAC
# include "../libraries/raylib-5.5_macos/include/raylib.h"
# include "../libraries/raylib-5.5_macos/include/raymath.h"
#else
# error We have not included the raylib release for this OS yet.
#endif

#include "../source/core.h"
#include "../source/render.h"


//////////////////////////////////////
// Paths
//////////////////////////////////////
#if OS_WINDOWS
# define _ "\\"
#else
# define _ "/"
#endif

global_variable String8 Keybind_Config_Filepath = str8_comptime_lit(".."_"config"_"keybind.txt");
global_variable String8 Saves_Filepath = str8_comptime_lit(".."_"saves"_);
global_variable String8 Build_Filepath = str8_comptime_lit(".."_"build"_);

#undef _


//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////

typedef struct Context Context;




//////////////////////////////////////
// Process
//////////////////////////////////////

typedef enum {
  Process_Flag_Wire      = 1 << 0,
  Process_Flag_Empty     = 1 << 1,
  Process_Flag_Cup       = 1 << 2,
  Process_Flag_Cap       = 1 << 3,
  Process_Flag_Identity  = 1 << 4,
  Process_Flag_Drag_In   = 1 << 5,
  Process_Flag_Drag_Out  = 1 << 6,

  // UI features
  Process_Flag_Button      = 1 << 7,
  Process_Flag_TextEdit    = 1 << 8,
  Process_Flag_CanBeActive = 1 << 9,
} Process_Flag;

#define Process_Connection_Xlist\
  X(In, 0) X(Out, 1)

typedef enum {
#define X(conn, ...)\
  Process_Connection_##conn,
  Process_Connection_Xlist
#undef X
  Process_Connection__Count,
} Process_Connection;

typedef enum {
#define X(conn, i)\
  Process_Connection_Flag_##conn = (1 << i),
  Process_Connection_Xlist
#undef X
} Process_Connection_Flag;

typedef struct Process Process;
struct Process {
  B32 flags;
  Vector2 position;
  Vector2 size;
  U32 cold_index;

  union {
    struct {
      S32 in_count;
      S32 out_count;
    };
    S32 conn_count[Process_Connection__Count];
  };

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

  Process *next;
  Process *next_active;

  Process *to_copied;

  union {
    String_Chunk_List label;
    Cold_String cold_label;
  };
  U32 label_cursor;
  B32 ignore_label_size;
};

typedef enum {
  Process_Selection__Null,
  Process_Selection_In,
  Process_Selection_Out,
  Process_Selection_NewWire,
  Process_Selection_Process,
} Process_Selection_Type;

typedef struct {
  Process *process;
  Process_Selection_Type type;
  S32 index;
  B32 hot_id_assigned;
} Process_Selection;

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
// UI Structs
//////////////////////////////////////

typedef struct {
  String_Chunk_List name;
  void (*func)(Context*);
} Ui_Dropdown_Item;




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

typedef struct {
  B32 mouse0_pressed;
  B32 mouse1_pressed;
  B32 mouse0_down;
  B32 mouse1_down;
  B32 hot_id_assigned;
  Vector2 mouse_wheel_movement;
  B32 control_down;
  B32 shift_down;
  B32 alt_down;

  B32 action_occured;
} Ui_State;

typedef enum {
  Menu_State__Null,
  Menu_State_File,
  Menu_State_OpenFile,
  Menu_State_SaveFileAs,
} Menu_State;

struct Context {
  Arena *render_arena;
  Arena *permanent_arena;
  Arena *ui_arena;
  Arena *temp_arena;

  U32 flags;

  Process_List processes;
  Process_List free_processes;
  String_Chunk_List free_strings;

  Process *hot_process;
  Process_List active_processes;

  Process_List copy_processes;

  Process_List ui_element_list;

  Render_Context ui_render_context;
  Render_Context process_render_context;

  Ui_State ui_state;
  Vector2 mouse_position; // TODO: move this to ui_state
  Vector2 active_position;
  Vector2 copy_center;
  Menu_State menu_state;

  String_Chunk_List save_file_name;

  Camera2D camera;
};





//////////////////////////////////////
// Includes relying on Context
//////////////////////////////////////

#include "../source/keybind.h"



//////////////////////////////////////
// Process Shape
//////////////////////////////////////

typedef enum {
  Process_Shape_TriangleFan,
  Process_Shape_TriangleStrip,
  Process_Shape_Circle,
  Process_Shape_HalfCircle,
} Process_Shape_Kind;


typedef struct {
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
} Process_Shape;


//////////////////////////////////////
// Saves
//////////////////////////////////////

#include "../source/saves.h"





//////////////////////////////////////
// Globals
//////////////////////////////////////

global_variable Vector2 global_window_size;

global_variable F32 global_process_wire_padding = 8.0f;
global_variable F32 global_process_wire_spacing = 22.0f;

#define Default_Box_Size 10.0f
global_variable F32 global_box_size = Default_Box_Size;
global_variable F32 global_box_half_size = 0.5f*Default_Box_Size;

#define Default_Shape_Size 40.0f
global_variable F32 global_shape_size = Default_Shape_Size;
global_variable F32 global_shape_half_size = 0.5f*Default_Shape_Size;

global_variable F32 global_line_thickness;
global_variable F32 global_active_line_thickness;

global_variable F32 global_process_font_size = 16.0f;
global_variable F32 global_panel_font_size = 14.0f;

global_variable Color global_background_color;

global_variable S32 global_shape_fan_triangle_count = 12;

global_variable Vector2 global_button_padding = (Vector2){12.0f, 5.0f};
global_variable Color global_button_dormant_bg_color = {90, 70, 90, 255};
global_variable Color global_button_hot_bg_color = {100, 80, 100, 255};
global_variable Color global_button_font_color = {220, 220, 160, 255};

global_variable String_Chunk_List global_open_button_label;
global_variable String_Chunk_List global_save_button_label;
global_variable String_Chunk_List global_save_as_button_label;
global_variable String_Chunk_List global_cancel_button_label;
global_variable String_Chunk_List global_file_button_label;

global_variable Process global_null_process;
global_variable String_Chunk global_null_string_chunk;
#define The_Null_Process() (global_null_process=(Process){0}, &global_null_process)
#define The_Null_String_Chunk() (global_null_string_chunk=(String_Chunk){0}, &global_null_string_chunk)

#define Half_Circle_Fudge 1.32f
#define Half_Circle_Radius_Fudge 1.0f




function B32 rectangle_contains_point(Rectangle r, Vector2 p) {
  F32 x2 = r.x + r.width;
  F32 y2 = r.y + r.height;
  B32 contains = (p.x >= r.x) && (p.y >= r.y) && (p.x <= x2) && (p.y <= y2);
  return contains;
}




function String_Chunk *create_string_chunk(Context *context) {
  String_Chunk *c = context->free_strings.first;

  if (c) {
    SLLQueuePop(context->free_strings.first,context->free_strings.last);
  } else {
    c = push_struct(context->permanent_arena, String_Chunk);
  }

  if (c) {
    *c = (String_Chunk){0};
  } else {
    c = The_Null_String_Chunk();
  }

  return c;
}

function void free_string_chunk(Context *context, String_Chunk *chunk) {
  SLLQueuePush(context->free_strings.first, context->free_strings.last, chunk);
  chunk->next = 0;
}


function String_Chunk_List string_chunk_list_from_string8(Context *context, String8 string8) {
  String_Chunk_List list = (String_Chunk_List){0};

  U64 remaining_size = string8.size;
  U64 string8_index = 0;

  for (;;) {
    if (remaining_size == 0) {
      break;
    }

    String_Chunk *chunk = create_string_chunk(context);
    SLLQueuePush(list.first, list.last, chunk);

    U64 amount_to_write = Min(remaining_size, String_Chunk_Size);
    remaining_size -= amount_to_write;

    for (S32 i = 0; i < amount_to_write; ++i) {
      chunk->str_array[i] = string8.str[string8_index];
      string8_index += 1;
    }
  }

  // add null-termination chunk if the last byte is not 0
  if (list.last && list.last->str_array[String_Chunk_Size-1] != 0) {
    String_Chunk *chunk = create_string_chunk(context);
    SLLQueuePush(list.first, list.last, chunk);
  }

  return list;
}



function Process *create_detached_process(Context *context) {
  Process *p = context->free_processes.first;

  if (p) {
    SLLQueuePop(context->free_processes.first, context->free_processes.last);
  } else {
    p = push_struct(context->permanent_arena, Process);
  }

  if (p) {
    *p = (Process){0};
  } else {
    p = The_Null_Process();
  }

  return p;
}

function Process *create_process(Context *context) {
  Process *p = create_detached_process(context);

  if (p) {
    SLLQueuePush(context->processes.first, context->processes.last, p);
  }

  return p;
}

function void clear_active_processes(Context *context) {
  if (context->active_processes.first) {
    for (Process *p = context->active_processes.first; p != 0;) {
      Process *next_active = p->next_active;
      p->next_active = 0;
      p = next_active;
    }
  }

  context->active_processes.first = 0;
  context->active_processes.last = 0;
}

function void remove_process_from_process_list(Context *context, Process_List *list, Process *p) {
  if (list->first == p) {
    SLLQueuePop(list->first, list->last);
  } else {
    for (Process *test_p = list->first; test_p != 0; test_p = test_p->next) {
      if (test_p->next == p) {
        test_p->next = p->next;
        if (p == list->last) {
          list->last = test_p;
        }
        break;
      }
    }
  }

  SLLQueuePush(context->free_processes.first, context->free_processes.last, p);
}

function void clear_processes(Context *context) {
  clear_active_processes(context);

  for (Process *p = context->processes.first; p != 0;) {
    Process *next_process = p->next;
    remove_process_from_process_list(context, &context->processes, p);
    p = next_process;
  }
}



////////////////////////////////////////
// UI Functions
////////////////////////////////////////

function void clear_ui_state(Context *context) {
  context->menu_state = 0;

  context->ui_element_list.first = 0;
  context->ui_element_list.last = 0;

  arena_pop_to(context->ui_arena, 0);
}

function void handle_label_editing(Context *context, Process_List ps) {
  U32 key = 0;
  B32 shift_down = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

  while ((key = GetKeyPressed())) {
    for (Process *a = ps.first; a != 0; a = a->next_active) {
      if (Get_Flag(a->flags, Process_Flag_TextEdit)) {
        B32 is_ascii = key > 0 && key < 256;
        U8 c = ascii_char_lookup[key&0xff][shift_down];
        if (is_ascii && c != 0) {
          // push string-chunk if string list is empty
          if (a->label.last == 0) {
            String_Chunk *sc = create_string_chunk(context);
            SLLQueuePush(a->label.first, a->label.last, sc);
            a->label_cursor = 0;
          }
          // add char to label
          a->label.last->str_array[a->label_cursor] = c;
          a->label_cursor += 1;
          // push string-chunk if at the end of current chunk
          if (a->label_cursor == String_Chunk_Size) {
            String_Chunk *sc = create_string_chunk(context);
            SLLQueuePush(a->label.first, a->label.last, sc);
            a->label_cursor = 0;
          }
        } else if (key == KEY_BACKSPACE) {
          if (a->label_cursor == 0) {
            // only free string-chunk if it is _not_ the only chunk
            if (a->label.first != a->label.last) {
              String_Chunk *free_chunk = a->label.last;
              if (free_chunk) {
                a->label_cursor = String_Chunk_Size - 1;
                // remove last string-chunk
                for (String_Chunk *chunk = a->label.first; chunk != 0; chunk = chunk->next) {
                  if (chunk->next == a->label.last) {
                    chunk->next = 0;
                    a->label.last = chunk;
                    break;
                  }
                }
                // add unused string-chunk to free-list
                SLLQueuePush(context->free_strings.first, context->free_strings.last, free_chunk);
                // zero current character
                a->label.last->str_array[a->label_cursor] = 0;
              }
            }
          } else if (a->label.last) {
            // decrement and zero current character
            a->label_cursor -= 1;
            a->label.last->str_array[a->label_cursor] = 0;
          }
        }
      }
    }
  }
}


function B32 do_button(Context *context, Process *button) {
  Render_Context *rc = &context->ui_render_context;
  // NOTE: Assumes label is null-terminated for now...
  F32 font_size = global_panel_font_size;
  U8 *label_c_string = c_string_from_string_chunk_list(render_GlobalTempArena, &button->label);
  S32 text_width = MeasureText((char *)label_c_string, font_size);
  Vector2 padding = global_button_padding;
  Color dormant_bg_color = global_button_dormant_bg_color;
  Color hot_bg_color = global_button_hot_bg_color;
  Color font_color = global_button_font_color;
  Rectangle bg_rect = (Rectangle){button->position.x, button->position.y, (F32)text_width+2.0f*padding.x, font_size+2.0f*padding.y};
  if (button->ignore_label_size) {
    bg_rect.width = button->size.x;
    bg_rect.height = button->size.y;
  }
  B32 is_hot = 0;
  B32 clicked = 0;

  if (!context->ui_state.action_occured &&
      rectangle_contains_point(bg_rect, context->mouse_position)) {
    context->hot_process = 0;
    is_hot = 1;

    if (IsMouseButtonPressed(0)) {
      clicked = 1;
      context->ui_state.action_occured = 1;
      if (Get_Flag(button->flags, Process_Flag_CanBeActive)) {
        clear_active_processes(context);
        SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, button, next_active, 0);
      }
    }
  }

  Color bg_color = is_hot ? hot_bg_color : dormant_bg_color;

  render_DrawRectangle(rc, bg_rect.x, bg_rect.y, bg_rect.width, bg_rect.height, bg_color);
  render_DrawText(rc, (char *)label_c_string, button->position.x+padding.x+1.0f, button->position.y+padding.y+1.0f, font_size, (Color){0, 0, 0, 255}, 0);
  render_DrawText(rc, (char *)label_c_string, button->position.x+padding.x, button->position.y+padding.y, font_size, font_color, 0);

  return clicked;
}


function Process *create_button(Arena *arena, Vector2 position, String_Chunk_List label) {
  Process *button = push_struct(arena, Process);

  if (button) {
    Set_Flag(button->flags, Process_Flag_Button);
    button->position = position;
    button->label = label;
  }

  return button;
}



function void do_dropdown_items(Context *context, Ui_Dropdown_Item *items, S32 item_count, Vector2 position) {
  Render_Context *rc = &context->ui_render_context;
  F32 button_height = 2.0f*global_button_padding.y + global_panel_font_size;

  // get max text width
  F32 max_text_width = 0.0f;
  for (S32 i = 0; i < item_count; ++i) {
    U8 *text_c_string = c_string_from_string_chunk_list(render_GlobalTempArena, &items[i].name);
    S32 text_width_raw = MeasureText((char *)text_c_string, global_panel_font_size);
    F32 text_width = 2.0f*global_button_padding.x + (F32)text_width_raw;
    if (text_width > max_text_width) {
      max_text_width = text_width;
    }
  }

  render_DrawRectangle(rc, position.x, position.y, (F32)max_text_width, button_height*item_count, global_button_dormant_bg_color);

  for (S32 i = 0; i < item_count; ++i) {
    Ui_Dropdown_Item item = items[i];
    Vector2 action_position = (Vector2){position.x, position.y + button_height*(F32)i};
    Process *file_action = create_button(context->temp_arena, action_position, item.name);
    file_action->size = (Vector2){max_text_width, button_height};
    file_action->ignore_label_size = 1;
    B32 clicked = do_button(context, file_action);
    if (clicked) {
      context->menu_state = 0;
      item.func(context);
    }
  }
}






////////////////////////////////////////
// File actions
////////////////////////////////////////

function void collect_save_files(Context *context) {
  Arena *uia = context->ui_arena;

  // clear old potentially old ui-element-list
  context->ui_element_list.first = 0;
  context->ui_element_list.last = 0;

  // gather the current files in the "saves" folder
  FileProperties file_props = Zero_Struct(FileProperties);
  String8 file_name = Zero_Struct(String8);
  OS_FileIter file_iter = os_file_iter_init(Saves_Filepath);
  while(os_file_iter_next(uia, &file_iter, &file_name, &file_props)) {
    if (!Get_Flag(file_props.flags, FilePropertyFlag_IsFolder)) {
      Process *element = push_struct(uia, Process);
      Set_Flag(element->flags, Process_Flag_Button);
      element->label = string_chunk_list_from_string8(context, file_name);
      U8 *cstring = c_string_from_string_chunk_list(context->temp_arena, &element->label);

      if (element) {
        SLLQueuePush(context->ui_element_list.first, context->ui_element_list.last, element);
      }
    }
  }
}

function void set_menu_state_as_open_file(Context *context) {
  context->menu_state = Menu_State_OpenFile;

  collect_save_files(context);
}

function void set_menu_state_as_save_file_as(Context *context) {
  context->menu_state = Menu_State_SaveFileAs;

  collect_save_files(context);

  F32 input_height = 2.0f*global_button_padding.y + global_panel_font_size;

  String_Chunk_List empty_text = Zero_Struct(String_Chunk_List);
  Process *text_input = create_button(context->ui_arena, (Vector2){0}, empty_text);
  if (text_input) {
    // setup text input element
    Set_Flag(text_input->flags, Process_Flag_TextEdit|Process_Flag_CanBeActive);
    text_input->ignore_label_size = 1;
    text_input->size = (Vector2){300.0f, input_height};
    SLLQueuePushFront(context->ui_element_list.first, context->ui_element_list.last, text_input);
  }
}

function void save_file(Context *context) {
  write_save_file(context, context->temp_arena, context->save_file_name);
}

function void open_file_and_replace_processes(Context *context, String_Chunk_List file_name_list) {
  os_set_current_directory(Saves_Filepath);
  String8 file_name = string8_from_string_chunk_list(context->temp_arena, &file_name_list);
  String8 file_data = os_file_read(context->temp_arena, file_name);

  if (file_data.str && file_data.size > sizeof(Save_File_Header)) {
    Save_File_Header *header = (Save_File_Header *)file_data.str;
    if (header->magic_number == Save_File_Magic_Number) {
      U64 process_array_size = file_data.size - sizeof(Save_File_Header);
      if (process_array_size == sizeof(Process) * header->process_count) {
        Process *cold_processes = (Process *)(header + 1);
        Process *previous_process = 0;
        clear_processes(context);

        // create new processes
        for (S32 i = 0; i < header->process_count; ++i) {
          Process cold_process = cold_processes[i];
          Process *new_process = create_process(context);
          *new_process = cold_process;

          if (previous_process) {
            previous_process->next = new_process;
          }

          previous_process = new_process;
        }

        // @Speed
        // convert cold-indices to pointers
        for (Process *p = context->processes.first; p != 0; p = p->next) {
          for (Process *r = context->processes.first; r != 0; r = r->next) {
            if (IntFromPtr(p->in) == r->cold_index) {
              p->in = r;
            }

            if (IntFromPtr(p->out) == r->cold_index) {
              p->out = r;
            }
          }
        }
      } else {
        printf("[ Error ] Mismatched size of process array in save-file. Header says %d processes for a size of %lu, but the actual size is %llu.\n", header->process_count, sizeof(Process) * header->process_count, process_array_size);
      }
    }
  }

  os_set_current_directory(Build_Filepath);
}

function void handle_open_file(Context *context) {
  Arena *ra = context->render_arena;
  F32 font_size = global_panel_font_size;
  Vector2 size = (Vector2){ 400.0f, 300.0f };
  Vector2 position = Vector2Scale(Vector2Subtract(global_window_size, size), 0.5f);
  Color dormant_bg_color = global_button_dormant_bg_color;
  Color font_color = global_button_font_color;
  F32 button_height = 2.0f*global_button_padding.y + global_panel_font_size;

  // show existing save files
  for (Process *element = context->ui_element_list.first; element; element = element->next) {
    element->position = position;
    if (do_button(context, element)) {
      open_file_and_replace_processes(context, element->label);
    }
    position.y += button_height;
  }

  // TODO: replace this with handling of input, or detect if the user has clicked "away"
  if (IsMouseButtonPressed(0)) {
    clear_ui_state(context);
  }
}

function void handle_save_file_as(Context *context) {
  Arena *ra = context->render_arena;
  F32 font_size = global_panel_font_size;
  Vector2 size = (Vector2){ 400.0f, 300.0f };
  Vector2 position = Vector2Scale(Vector2Subtract(global_window_size, size), 0.5f);
  Color dormant_bg_color = global_button_dormant_bg_color;
  Color font_color = global_button_font_color;
  F32 button_height = 2.0f*global_button_padding.y + global_panel_font_size;

  Process *file_name_element = context->ui_element_list.first;

  // show existing save files
  for (Process *element = context->ui_element_list.first; element; element = element->next) {
    element->position = position;
    do_button(context, element);
    position.y += button_height;
  }

  Process *cancel_button = create_button(context->temp_arena, position, global_cancel_button_label);
  position.y += button_height;
  Process *save_button = create_button(context->temp_arena, position, global_save_button_label);

  if (do_button(context, cancel_button)) {
    clear_ui_state(context);
  } else if (do_button(context, save_button)) {
    if (file_name_element) {
      write_save_file(context, context->temp_arena, file_name_element->label);
    }
    clear_ui_state(context);
  }

  // TODO: replace this with handling of input, or detect if the user has clicked "away"
  if (IsKeyPressed(KEY_ESCAPE)) {
    clear_ui_state(context);
  }
}






function Vector2 get_percentage_between_points(Vector2 p0, Vector2 p1, F32 percentage) {
  Vector2 norm_delta = Vector2Normalize(Vector2Subtract(p1, p0));
  F32 distance_along_delta = percentage * Vector2Distance(p1, p0);
  Vector2 center = Vector2Add(p0, Vector2Scale(norm_delta, distance_along_delta));

  return center;
}




function B32 is_active_process(Context *context, Process *p) {
  B32 is_active = 0;

  for (Process *test_p = context->active_processes.first; test_p != 0; test_p = test_p->next_active) {
    if (test_p == p) {
      is_active = 1;
      break;
    }
  }

  return is_active;
}





function Vector2 get_process_position(Context *context, Process *process) {
  Vector2 position = process->position;
  B32 is_active = is_active_process(context, process);
  B32 is_dragging = Get_Flag(context->flags, Context_Flag_Dragging);


  if (is_active && is_dragging) {
    // @Copypasta draw_processes    draw wire
    Vector2 delta = Vector2Subtract(context->mouse_position, context->active_position);
    position = Vector2Add(position, Vector2Scale(delta, 1.0f/context->camera.zoom));
  }

  return position;
}








function Vector2
get_process_wire_position(Context *context, Process *p, Process_Shape shape, Process_Connection conn, U32 wire_index) {
  F32 padding = context->camera.zoom * global_process_wire_padding;
  Vector2 p0;
  Vector2 p1;

  switch(conn) {
  case Process_Connection_In: {
    if (shape.kind == Process_Shape_HalfCircle) {
      p0 = shape.points[0];
      p1 = shape.points[shape.point_count-1];
    } else if (shape.point_count == 4) {
      p0 = shape.points[2];
      p1 = shape.points[3];
    } else {
      p0 = shape.points[2];
      p1 = shape.points[1];
    }
  } break;
  case Process_Connection_Out: {
    if (shape.kind == Process_Shape_HalfCircle) {
      p0 = shape.points[shape.point_count-1];
      p1 = shape.points[0];
    } else {
      p0 = shape.points[0];
      p1 = shape.points[1];
    }
  } break;
  }

  Vector2 delta = Vector2Subtract(p0, p1);
  Vector2 delta_norm = Vector2Normalize(delta);
  F32 inner_distance = fmax(0.0f, Vector2Distance(p0, p1) - 2.0f*padding);
  F32 chunk_size = inner_distance / (F32)(p->conn_count[conn]+1);
  F32 distance_from_point = padding + chunk_size*(F32)(wire_index+1);

  Vector2 wire_position = Vector2Add(p1, Vector2Scale(delta_norm, distance_from_point));

  return wire_position;
}



function Rectangle get_wire_box(Context *context, Vector2 position) {
  F32 size = context->camera.zoom * global_box_size;
  F32 half_size = context->camera.zoom * global_box_half_size;
  Rectangle box = (Rectangle){position.x-half_size, position.y-half_size, size, size};
  return box;
}



function Rectangle get_new_wire_box(Context *context, Process *p, Process_Shape shape) {
  Vector2 position = shape.new_wire_position;
  F32 size = context->camera.zoom * global_box_size;
  F32 half_size = context->camera.zoom * global_box_half_size;

  Rectangle new_wire_box = (Rectangle){position.x - half_size,
                                       position.y - half_size,
                                       size, size};

  return new_wire_box;
}


function Rectangle get_selection_rectangle(Context *context) {
  F32 x = fmin(context->active_position.x, context->mouse_position.x);
  F32 y = fmin(context->active_position.y, context->mouse_position.y);
  F32 x1 = fmax(context->active_position.x, context->mouse_position.x);
  F32 y1 = fmax(context->active_position.y, context->mouse_position.y);
  Rectangle selection_rect = (Rectangle){x, y, x1-x, y1-y};

  return selection_rect;
}




function Process *get_process_wire_by_selection(Context *context, Process_Selection selection) {
  Process *wire = 0;

  for (Process *p = context->processes.first; p != 0; p = p->next) {
    if (Get_Flag(p->flags, Process_Flag_Wire)) {
      if (selection.type == Process_Selection_In &&
          p->in == selection.process &&
          p->which_in == selection.index) {
        // matching in-wire
        wire = p;
        break;
      } else if (selection.type == Process_Selection_Out &&
                 p->out == selection.process &&
                 p->which_out == selection.index) {
        // matching out-wire
        wire = p;
        break;
      }
    }
  }

  if (wire == 0) {
    wire = The_Null_Process();
  }

  return wire;
}





function void remove_copy_process_list(Context *context, Process_List *list) {
  // TODO: @Speed can probably do some fancy stuff with just the ends of the list?
  for (Process *p = list->first; p != 0;) {
    Process *next_process = p->next;
    remove_process_from_process_list(context, &context->copy_processes, p);
    p = next_process;
  }
}

function void remove_process_from_active_processes(Context *context, Process *p) {
  if (context->active_processes.first == p) {
    SLLQueuePop_NZ(context->active_processes.first, context->active_processes.last, next_active, 0);
  } else {
    for (Process *test_p = context->active_processes.first; test_p != 0; test_p = test_p->next_active) {
      if (test_p->next_active == p) {
        test_p->next_active = p->next_active;
        if (p == context->active_processes.last) {
          context->active_processes.last = test_p;
        }
        break;
      }
    }
  }
}


function void
add_wire_connection(Context *context, Process *wire, Process *process, Process_Connection conn, U32 which_conn) {
  // Add wire at the given connection index, moving any wires that come after that index over to the right.
  wire->conn[conn] = process;
  wire->which_conn[conn] = which_conn;

  for (Process *test_wire = context->processes.first; test_wire != 0; test_wire = test_wire->next) {
    if (wire != test_wire &&
        test_wire->conn[conn] == process &&
        test_wire->which_conn[conn] >= which_conn) {
      // increment the wire's which_conn if it comes at or after the added wire's which_conn
      test_wire->which_conn[conn] += 1;
    }
  }

  process->conn_count[conn] += 1;
}

function void
remove_wire_connection(Context *context, Process *wire, Process_Connection_Flag conn_flags) {
  // Remove a wire and move wires to the right of it to the left one.
  B32 in_matched = 0;
  B32 out_matched = 0;

  B32 remove_in = Get_Flag(conn_flags, Process_Connection_Flag_In);
  B32 remove_out = Get_Flag(conn_flags, Process_Connection_Flag_Out);

  for (Process *test_wire = context->processes.first; test_wire != 0; test_wire = test_wire->next) {
    // adjust in-connections that come after deleted wire
    if (remove_in && test_wire->in == wire->in) {
      if (test_wire->which_in > wire->which_in) {
        test_wire->which_in -= 1;
      }
      in_matched = 1;
    }

    // adjust out-connections that come after deleted wire
    if (remove_out && test_wire->out == wire->out) {
      if (test_wire->which_out > wire->which_out) {
        test_wire->which_out -= 1;
      }
      out_matched = 1;
    }
  }

  B32 only_in_conn = wire->in != 0 && wire->which_in == 0;
  B32 only_out_conn = wire->out != 0 && wire->which_out == 0;

  // decrement process' in-count
  if (remove_in && (in_matched || only_in_conn)) {
    if (wire->in) {
      wire->in->in_count -= 1;
    }
  }

  // decrement process' out-count
  if (remove_out && (out_matched || only_out_conn)) {
    if (wire->out) {
      wire->out->out_count -= 1;
    }
  }
}



function void delete_process(Context *context, Process *p) {
  // TODO: Do we really want to check if it is a live process? Maybe sometimes the program wants to delete a process that is *not* in the processes list and still want it to end up in the free-list.
  B32 is_live_process = 0;
  for (Process *test_p = context->processes.first; test_p != 0; test_p = test_p->next) {
    if (test_p == p) {
      is_live_process = 1;
      break;
    }
  }

  if (is_live_process) {
    // if deleting a wire, adjust connected processes
    if (Get_Flag(p->flags, Process_Flag_Wire)) {
      U32 both_conns = Process_Connection_Flag_In | Process_Connection_Flag_Out;
      remove_wire_connection(context, p, both_conns);
    }

    // remove p from processes
    remove_process_from_process_list(context, &context->processes, p);

    // check for wires connected to the deleted process, and delete those also
    for (Process *wire = context->processes.first; wire != 0;) {
      B32 in_match = wire->in == p;
      B32 out_match = wire->out == p;
      B32 should_delete = 0;

      if (in_match || out_match) {
        if (!in_match) {
          // adjust in-connections to deleted wire
          for (Process *test_wire = context->processes.first; test_wire != 0; test_wire = test_wire->next) {
            if (test_wire->in == wire->in &&
                test_wire->which_in > wire->which_in) {
              test_wire->which_in -= 1;
            }
          }

          if (wire->in) {
            wire->in->in_count -= 1;
          }
        }

        if (!out_match) {
          // adjust out-connections to deleted wire
          for (Process *test_wire = context->processes.first; test_wire != 0; test_wire = test_wire->next) {
            if (test_wire->out == wire->out &&
                test_wire->which_out > wire->which_out) {
              test_wire->which_out -= 1;
            }
          }

          if (wire->out) {
            wire->out->out_count -= 1;
          }
        }

        should_delete = 1;
      }

      if (should_delete) {
        Process *next_process = wire->next;
        // remove wire from processes
        remove_process_from_process_list(context, &context->processes, wire);
        wire = next_process;
      } else {
        wire = wire->next;
      }
    }
  }
}


function void copy_active_processes(Context *context) {
  B32 error = 0;
  Vector2 copy_center = (Vector2){0};
  F32 copy_count = 0.0f;

  // remove whatever processes were already in the copy-list
  remove_copy_process_list(context, &context->copy_processes);

  // copy processes from active-list to copy-list
  for (Process *a = context->active_processes.first; a != 0; a = a->next_active) {
    if (Get_Flag(a->flags, Process_Flag_Wire)) {
      // add connected processes if they have not been added yet
      for (S32 conn = 0; conn < Process_Connection__Count; ++conn) {
        if (a->conn[conn] && a->conn[conn]->to_copied == 0) {
          B32 found_conn = 0;
          for (Process *test_p = context->active_processes.first; test_p != 0; test_p = test_p->next_active) {
            if (test_p == a->conn[conn]) {
              found_conn = 1;
              // add connected process to copied list
              // @Copypasta    v------------------v
              Process *copied_p = create_detached_process(context);
              *copied_p = *a->conn[conn];
              a->conn[conn]->to_copied = copied_p;
              copy_center = Vector2Add(copy_center, a->conn[conn]->position);
              copy_count += 1.0f;
              SLLQueuePush(context->copy_processes.first, context->copy_processes.last, copied_p);
              break;
            }
          }
          if (!found_conn) {
            // add invisible process to copied list
            // @Copypasta       ^----------v
            Process *copied_p = create_detached_process(context);
            *copied_p = *a->conn[conn];
            Set_Flag(copied_p->flags, Process_Flag_Empty);
            a->conn[conn]->to_copied = copied_p;
            copy_center = Vector2Add(copy_center, a->conn[conn]->position);
            copy_count += 1.0f;
            SLLQueuePush(context->copy_processes.first, context->copy_processes.last, copied_p);
          }
        }
      }
      // add wire to copied-list
      Process *copied_wire = create_detached_process(context);
      *copied_wire = *a;
      // connect copied wire to copied processes
      for (S32 conn = 0; conn < Process_Connection__Count; ++conn) {
        if (copied_wire->conn[conn]) {
          copied_wire->conn[conn] = copied_wire->conn[conn]->to_copied;
        }
      }
      SLLQueuePush(context->copy_processes.first, context->copy_processes.last, copied_wire);
    } else if (!a->to_copied) {
      // only add process if it hasn't already been added by a connected wire
      // @Copypasta       ^----------^
      Process *copied_p = create_detached_process(context);
      *copied_p = *a;
      a->to_copied = copied_p;
      copy_center = Vector2Add(copy_center, a->position);
      copy_count += 1.0f;
      SLLQueuePush(context->copy_processes.first, context->copy_processes.last, copied_p);
    }
  }

  // remove all to_copied fields
  for (Process *p = context->processes.first; p != 0; p = p->next) {
    p->to_copied = 0;
  }

  // TODO: @Speed
  // fix-up copied wire positions (in the cases that only some of the wires between two processes are copied)
  for (Process *c = context->copy_processes.first; c != 0; c = c->next) {
    if (!Get_Flag(c->flags, Process_Flag_Wire)) {
      for (S32 conn = 0; conn < Process_Connection__Count; ++conn) {
        // get connected wire count
        S32 conn_count = 0;
        for (Process *w = context->copy_processes.first; w != 0; w = w->next) {
          if (Get_Flag(w->flags, Process_Flag_Wire) && w->conn[conn] == c) {
            conn_count += 1;
          }
        }
        // adjust process' conn count
        c->conn_count[conn] = conn_count;
        // we have to loop wire-count times to re-assign each wire
        for (S32 min_conn = 0; min_conn < conn_count; ++min_conn) {
          Process *min_wire = 0;
          // find the next connected wire
          for (Process *w = context->copy_processes.first; w != 0; w = w->next) {
            if (Get_Flag(w->flags, Process_Flag_Wire) && w->conn[conn] == c) {
              if (w->which_conn[conn] >= min_conn) {
                if (min_wire == 0 || w->which_conn[conn] < min_wire->which_conn[conn]) {
                  min_wire = w;
                }
              }
            }
          }
          // adjust the wire's connection
          if (min_wire) {
            min_wire->which_conn[conn] = min_conn;
          }
        }
      }
    }
  }

  // make any process with more than one connection (in or out) visible
  for (Process *c = context->copy_processes.first; c != 0; c = c->next) {
    B32 more_than_one_connection = 0;
    for (S32 conn = 0; conn < Process_Connection__Count; ++conn) {
      if (c->conn_count[conn] > 1) {
        more_than_one_connection = 1;
        break;
      }
    }
    if (more_than_one_connection) {
      Unset_Flag(c->flags, Process_Flag_Empty);
    }
  }

  if (error) {
    remove_copy_process_list(context, &context->copy_processes);
  } else {
    if (copy_count > 0.0f) {
      copy_center = Vector2Scale(copy_center, 1.0f/copy_count);
    } else {
      // TODO: If copy_count is 0 then something went wrong and maybe we should just bail?
      copy_center = (Vector2){0};
    }
    context->copy_center = copy_center;
  }
}

function void paste_processes(Context *context) {
  Vector2 mouse_world_pos = GetScreenToWorld2D(context->mouse_position, context->camera);
  Vector2 center_delta = Vector2Subtract(mouse_world_pos, context->copy_center);

  for (Process *p = context->copy_processes.first; p != 0;) {
    Process *next_p = p->next;
    if (!Get_Flag(p->flags, Process_Flag_Wire)) {
      p->position = Vector2Add(p->position, center_delta);
    }

    SLLQueuePush(context->processes.first, context->processes.last, p);
    p = next_p;
  }

  context->copy_processes.first = 0;
  context->copy_processes.last = 0;
}



function void connect_processes(Context *context, Process *out, Process *in) {
  Process *new_wire = create_process(context);

  if (new_wire && out && in) {
    Set_Flag(new_wire->flags, Process_Flag_Wire);
    new_wire->out = out;
    new_wire->in = in;

    new_wire->which_out = out->out_count;
    new_wire->which_in = in->in_count;

    out->out_count += 1;
    in->in_count += 1;
  }
}


function Half_Circle_Points
get_half_circle_points(Context *context, Process_Shape shape, Process *p, Vector2 position, S32 text_width, B32 downward) {
  Half_Circle_Points half_circle_points;
  F32 padding = context->camera.zoom * global_process_wire_padding;
  F32 spacing = context->camera.zoom * global_process_wire_spacing;

  F32 height = context->camera.zoom * global_shape_size;
  F32 half_height = context->camera.zoom * global_shape_half_size;

  F32 conn_count = (F32)(downward ? p->out_count : p->in_count);
  F32 width = (2.0f*padding + conn_count*spacing);
  F32 half_width = 0.5f*(width);
  // fit shape to text, if the text is wider than the shape
  if ((F32)text_width > half_width) {
    half_width = 0.8f*(2.0f*padding + text_width);
  }

  F32 multiplier = downward ? -1.0f : 1.0f;
  F32 x_offset = multiplier * half_width;
  F32 y_offset = multiplier * half_height;

  Vector2 first_point = (Vector2){position.x-x_offset, position.y+y_offset};
  Vector2 second_point = (Vector2){position.x+x_offset, position.y+y_offset};

  half_circle_points.first_point = first_point;
  half_circle_points.second_point = second_point;
  half_circle_points.first_control = (Vector2){first_point.x, first_point.y-2.0f*y_offset};
  half_circle_points.second_control = (Vector2){second_point.x, second_point.y-2.0f*y_offset};

  half_circle_points.middle_of_curve = get_bezier_point(
    first_point,
    second_point,
    half_circle_points.first_control,
    half_circle_points.second_control,
    0.5f);

  half_circle_points.middle_of_line = get_percentage_between_points(first_point, second_point, 0.5f);

  return half_circle_points;
}


function void
fill_out_half_circle_shape(Context *context, Process_Shape *shape, Process *p, Vector2 position, S32 text_width, B32 downward) {
  Half_Circle_Points half_circle_points = get_half_circle_points(context, *shape, p, position, text_width, downward);

  shape->kind = Process_Shape_HalfCircle;
  shape->triangle_count = global_shape_fan_triangle_count;
  shape->downward = downward;

  shape->first_control = half_circle_points.first_control;
  shape->second_control = half_circle_points.second_control;

  Vector2 first_point = half_circle_points.first_point;
  Vector2 second_point = half_circle_points.second_point;

  Vector2 middle_of_curve = half_circle_points.middle_of_curve;
  Vector2 middle_of_line = half_circle_points.middle_of_line;

  shape->center = get_percentage_between_points(middle_of_curve, middle_of_line, 0.5);

  if (downward) {
    shape->new_wire_position = half_circle_points.first_point;
  } else {
    shape->new_wire_position = middle_of_curve;
  }

  shape->point_count = create_bezier_triangle_fan(
    first_point, second_point,
    shape->first_control, shape->second_control,
    shape->points, Process_Shape_Max_Points, shape->triangle_count);
}



function Process_Shape
get_process_shape(Context *context, Process *p) {
  Process_Shape shape = {0};
  U64 arena_pop_pos = arena_current_pos(context->temp_arena);

  F32 font_size = context->camera.zoom * global_process_font_size;
  U8 *label_c_string = c_string_from_string_chunk_list(context->temp_arena, &p->label);
  S32 text_width = MeasureText((char *)label_c_string, font_size);

  Vector2 position = get_process_position(context, p);
  position = GetWorldToScreen2D(position, context->camera);

  F32 half_size = context->camera.zoom * global_shape_half_size;
  F32 quarter_size = context->camera.zoom * global_shape_size / 4.0f;
  F32 padding = context->camera.zoom * global_process_wire_padding;
  F32 spacing = context->camera.zoom * global_process_wire_spacing;

  S32 has_in = p->in_count > 0;
  S32 has_out = p->out_count > 0;

  B32 rounded = Get_Flag(context->flags, Context_Flag_RoundedShapes);

  if (has_in && has_out) {
    // rectangular
    F32 max_conn = (F32)Max(p->in_count, p->out_count);
    F32 half_width = 0.5f*(2.0f*padding + max_conn*spacing);
    // fit shape to text if text is wide enough
    if ((F32)text_width > half_width) {
      half_width = 0.5f*(2.0f*padding + (F32)text_width);
    }
    shape.kind = Process_Shape_TriangleStrip;
    shape.point_count = 4;
    shape.triangle_count = 2;
    shape.points[0].x = position.x + half_width;
    shape.points[0].y = position.y - half_size;
    shape.points[1].x = position.x - half_width;
    shape.points[1].y = position.y - half_size;
    shape.points[2].x = position.x + half_width;
    shape.points[2].y = position.y + half_size;
    shape.points[3].x = position.x - half_width;
    shape.points[3].y = position.y + half_size;
    shape.center = get_percentage_between_points(shape.points[0], shape.points[3], 0.5f);
    shape.new_wire_position = shape.points[0];
  } else if (has_in) {
    F32 width = (2.0f*padding + p->in_count*spacing);
    F32 half_width = 0.5f*(width);
    // fit shape to text if text is wide enough
    if ((F32)text_width > 0.5f*half_width) {
      half_width = (2.0f*padding + (F32)text_width);
    }
    if (rounded) {
      // upward half-circle
      fill_out_half_circle_shape(context, &shape, p, position, text_width, 0);
    } else {
      // upward triangle
      shape.kind = Process_Shape_TriangleFan;
      shape.point_count = 3;
      shape.triangle_count = 1;
      shape.points[0].x = position.x;
      shape.points[0].y = position.y - quarter_size;
      shape.points[1].x = position.x - half_width;
      shape.points[1].y = position.y + half_size;
      shape.points[2].x = position.x + half_width;
      shape.points[2].y = position.y + half_size;
      Vector2 outer_mid = get_percentage_between_points(shape.points[1], shape.points[2], 0.5f);
      shape.center = get_percentage_between_points(shape.points[0], outer_mid, 0.66f);
      shape.new_wire_position = shape.points[0];
    }
  } else if (has_out) {
    F32 width = (2.0f*padding + p->out_count*spacing);
    F32 half_width = 0.5f*(width);
    // fit shape to text if text is wide enough
    if ((F32)text_width > 0.5f*half_width) {
      half_width = (2.0f*padding + (F32)text_width);
    }
    if (rounded) {
      // downward half-circle
      fill_out_half_circle_shape(context, &shape, p, position, text_width, 1);
    } else {
      // downward triangle
      shape.kind = Process_Shape_TriangleFan;
      shape.point_count = 3;
      shape.triangle_count = 1;
      shape.points[0].x = position.x + half_width;
      shape.points[0].y = position.y - half_size;
      shape.points[1].x = position.x - half_width;
      shape.points[1].y = position.y - half_size;
      shape.points[2].x = position.x;
      shape.points[2].y = position.y + quarter_size;
      Vector2 outer_mid = get_percentage_between_points(shape.points[0], shape.points[1], 0.5f);
      shape.center = get_percentage_between_points(shape.points[2], outer_mid, 0.66f);
      shape.new_wire_position = shape.points[0];
    }
  } else {
    if (rounded) {
      // circle
      // TODO: Setup the circle in a different way so that it can fit text. Like an ellipse with beziers...
      shape.kind = Process_Shape_Circle;
      shape.center = position;
      shape.radius = half_size*0.7f;
      shape.new_wire_position = (Vector2){shape.center.x, shape.center.y - shape.radius};
    } else {
      // diamond
      // fit shape to text if text is wide enough
      F32 half_size_x = half_size;
      if ((F32)text_width > half_size) {
        half_size_x = (F32)text_width;
      }
      shape.kind = Process_Shape_TriangleFan;
      shape.point_count = 4;
      shape.triangle_count = 2;
      shape.points[0].x = position.x;
      shape.points[0].y = position.y - half_size;
      shape.points[1].x = position.x - half_size_x;
      shape.points[1].y = position.y;
      shape.points[2].x = position.x;
      shape.points[2].y = position.y + half_size;
      shape.points[3].x = position.x + half_size_x;
      shape.points[3].y = position.y;
      shape.center = position;
      shape.new_wire_position = shape.points[0];
    }
  }

  arena_pop_to(context->temp_arena, arena_pop_pos);

  return shape;
}



function B32
triangle_fan_contains_point(Vector2 *points, S32 triangle_count, Vector2 point) {
  B32 contains = 0;

  for (S32 i = 1; i <= triangle_count; ++i) {
    F32 side1 = which_side_of_line(points[0], points[i], point);
    F32 side2 = which_side_of_line(points[i], points[i+1], point);
    F32 side3 = which_side_of_line(points[i+1], points[0], point);

    if (side1 < 0.0f && side2 < 0.0f && side3 < 0.0f) {
      contains = 1;
      break;
    }
  }

  return contains;
}



function B32
triangle_strip_contains_point(Vector2 *points, S32 triangle_count, Vector2 point) {
  B32 contains = 0;

  for (S32 i = 0; i < triangle_count; ++i) {
    F32 side1 = which_side_of_line(points[i], points[i+1], point);
    F32 side2 = which_side_of_line(points[i+1], points[i+2], point);
    F32 side3 = which_side_of_line(points[i+2], points[i], point);

    if (i % 2 == 0) {
      if (side1 < 0.0f && side2 < 0.0f && side3 < 0.0f) {
        contains = 1;
        break;
      }
    } else {
      if (side1 > 0.0f && side2 > 0.0f && side3 > 0.0f) {
        contains = 1;
        break;
      }
    }
  }

  return contains;
}



function B32
process_shape_contains_point(Context *context, Process_Shape shape, Vector2 point) {
  B32 contains = 0;

  switch(shape.kind) {
  case Process_Shape_Circle: {
    F32 distance = Vector2Distance(shape.center, point);
    contains = distance <= shape.radius;
  } break;
  case Process_Shape_HalfCircle: {
    contains = triangle_fan_contains_point(shape.points, shape.triangle_count, point);
  } break;
  case Process_Shape_TriangleFan: {
    contains = triangle_fan_contains_point(shape.points, shape.triangle_count, point);
  } break;
  case Process_Shape_TriangleStrip: {
    contains = triangle_strip_contains_point(shape.points, shape.triangle_count, point);
  } break;
  default: Assert(0);
  }

  return contains;
}






function Process_Selection
get_process_selection(Context *context, Process *p) {
  Process_Selection selection = {0};
  selection.index = -1;
  selection.process = p;

  Process_Shape shape = get_process_shape(context, p);
  Rectangle new_wire_box = get_new_wire_box(context, p, shape);

  if (rectangle_contains_point(new_wire_box, context->mouse_position)) {
    // check new-wire-box
    selection.type = Process_Selection_NewWire;
    context->hot_process = p;
    selection.hot_id_assigned = 1;
  } else {
    // check in wire-boxes
    for (U32 i = 0; i < p->in_count; ++i) {
      Vector2 in_position = get_process_wire_position(context, p, shape, Process_Connection_In, i);
      Rectangle r = get_wire_box(context, in_position);
      if (rectangle_contains_point(r, context->mouse_position)) {
        selection.type = Process_Selection_In;
        selection.index = i;
        Process *wire = get_process_wire_by_selection(context, selection);
        context->hot_process = wire;
        selection.hot_id_assigned = 1;
        break;
      }
    }

    if (selection.type == 0) {
      // check out wire-boxes
      for (U32 i = 0; i < p->out_count; ++i) {
        Vector2 out_position = get_process_wire_position(context, p, shape, Process_Connection_Out, i);
        Rectangle r = get_wire_box(context, out_position);
        if (rectangle_contains_point(r, context->mouse_position)) {
          selection.type = Process_Selection_Out;
          selection.index = i;
          Process *wire = get_process_wire_by_selection(context, selection);
          context->hot_process = wire;
          selection.hot_id_assigned = 1;
          break;
        }
      }
    }

    if (selection.type == 0 && !Get_Flag(p->flags, Process_Flag_Wire)) {
      if (process_shape_contains_point(context, shape, context->mouse_position)) {
        // process selection
        selection.type = Process_Selection_Process;
        context->hot_process = p;
        selection.hot_id_assigned = 1;
      }
    }
  }

  return selection;
}




function Keybind_Result check_keybind(Context *context, Ui_Feature feature, Process_Selection selection) {
  Keybind_Result result = 0;

  Keybind keybind = global_keybind_lookup[feature];
  Ui_State *ui_state = &context->ui_state;

  B32 key_is_pressed = 0;
  B32 key_is_down = 0;

  switch(keybind.key_kind) {
  case Key_Kind_Mouse0: {
    key_is_pressed = ui_state->mouse0_pressed;
    key_is_down = ui_state->mouse0_down;
  } break;
  case Key_Kind_Mouse1: {
    key_is_pressed = ui_state->mouse1_pressed;
    key_is_down = ui_state->mouse1_down;
  } break;
  case Key_Kind_MouseWheelUp: {
    key_is_pressed = ui_state->mouse_wheel_movement.y > 0.0f;
  } break;
  case Key_Kind_MouseWheelDown: {
    key_is_pressed = ui_state->mouse_wheel_movement.y < 0.0f;
  } break;
  default: {
    key_is_pressed = IsKeyPressed(keybind.key_kind);
    key_is_down = IsKeyDown(keybind.key_kind);
  } break;
  }

  B32 modifier_control = Get_Flag(keybind.modifiers, Modifier_Key_Control) ? 1 : 0;
  B32 modifier_shift = Get_Flag(keybind.modifiers, Modifier_Key_Shift) ? 1 : 0;
  B32 modifier_alt = Get_Flag(keybind.modifiers, Modifier_Key_Alt) ? 1 : 0;

  B32 modifier_matches = ((!(modifier_control ^ ui_state->control_down)) &&
                          (!(modifier_shift ^ ui_state->shift_down)) &&
                          (!(modifier_alt ^ ui_state->alt_down)));

  B32 constraint_hover_process =
    (Get_Flag(keybind.constraint, Ui_Constraint_HoverProcess)
     ? selection.type != 0
     : 1);
  B32 constraint_no_hover =
    (Get_Flag(keybind.constraint, Ui_Constraint_NoHotProcess)
     ? (context->hot_process == 0)
     : 1);

  B32 constraints_met = (constraint_hover_process &&
                         constraint_no_hover);

  if (key_is_pressed && modifier_matches && constraints_met) {
    result = Keybind_Result_Enter;
  }

  if (Get_Flag(keybind.constraint, Ui_Constraint_ExitOnKeyup)) {
    if (!key_is_down) {
      result = Keybind_Result_Exit;
    }
  }

  if (result == Keybind_Result_Enter) {
    context->ui_state.action_occured = 1;
  }

  return result;
}



function Ui_State get_ui_state(Context *context) {
  Ui_State ui_state;
  context->mouse_position = GetMousePosition(); // TODO: mouse_position should go in ui_state
  ui_state.mouse0_pressed = IsMouseButtonPressed(0);
  ui_state.mouse1_pressed = IsMouseButtonPressed(1);
  ui_state.mouse0_down = IsMouseButtonDown(0);
  ui_state.mouse1_down = IsMouseButtonDown(1);
  ui_state.hot_id_assigned = 0;
  ui_state.mouse_wheel_movement = GetMouseWheelMoveV();
  ui_state.control_down = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
  ui_state.shift_down = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
  ui_state.alt_down = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
  ui_state.action_occured = 0;
  return ui_state;
}



function void handle_ui(Context *context) {
  Process *file_button = create_button(context->temp_arena, (Vector2){0.0f, 0.0f}, global_file_button_label);

  if (do_button(context, file_button)) {
    // show file menu
    context->menu_state = context->menu_state == Menu_State_File ? 0 : Menu_State_File;
  }

  switch(context->menu_state) {
  case Menu_State_File: {
    Ui_Dropdown_Item file_dropdown_items[] = {
      {global_open_button_label, set_menu_state_as_open_file},
      {global_save_button_label, save_file},
      {global_save_as_button_label, set_menu_state_as_save_file_as},
    };
    F32 button_height = 2.0f*global_button_padding.y + global_panel_font_size;
    Vector2 position = (Vector2){file_button->position.x, button_height};

    do_dropdown_items(context, file_dropdown_items, ArrayCount(file_dropdown_items), position);
  } break;
  case Menu_State_OpenFile: {
    handle_open_file(context);
  } break;
  case Menu_State_SaveFileAs: {
    handle_save_file_as(context);
  } break;
  }
}



function void handle_process_interaction(Context *context) {
  Ui_State *ui_state = &context->ui_state;
  Process_Selection selection = (Process_Selection){0};

  B32 should_stop_dragging = check_keybind(context, Ui_Feature_SelectSingleProcess, selection) == Keybind_Result_Exit;
  Process *moved_wire = 0;
  Process_Connection moved_wire_conn = 0;

  // initial bounding handling
  if (Get_Flag(context->flags, Context_Flag_Bounding)) {
    if (check_keybind(context, Ui_Feature_Bound, selection) == Keybind_Result_Exit) {
      Unset_Flag(context->flags, Context_Flag_Bounding);
    } else {
      clear_active_processes(context);
    }
  }

  // panning
  {
    if (check_keybind(context, Ui_Feature_Pan, selection) == Keybind_Result_Enter) {
      Set_Flag(context->flags, Context_Flag_Panning);
      context->active_position = context->mouse_position;
    }

    if (Get_Flag(context->flags, Context_Flag_Panning)) {
      if (check_keybind(context, Ui_Feature_Pan, selection) == Keybind_Result_Exit) {
        Unset_Flag(context->flags, Context_Flag_Panning);
      } else {
        // Update camera position
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -1.0f/context->camera.zoom);
        context->camera.target = Vector2Add(context->camera.target, delta);
      }
    }
  }

  // zooming
  {
    B32 zoom_in = check_keybind(context, Ui_Feature_ZoomIn, selection) == Keybind_Result_Enter;
    B32 zoom_out = check_keybind(context, Ui_Feature_ZoomOut, selection) == Keybind_Result_Enter;

    if (zoom_in || zoom_out) {
      Keybind keybind_in = global_keybind_lookup[Ui_Feature_ZoomIn];
      Keybind keybind_out = global_keybind_lookup[Ui_Feature_ZoomOut];
      B32 in_wheel = (keybind_in.key_kind == Key_Kind_MouseWheelUp ||
                      keybind_in.key_kind == Key_Kind_MouseWheelDown);
      B32 out_wheel = (keybind_out.key_kind == Key_Kind_MouseWheelUp ||
                       keybind_out.key_kind == Key_Kind_MouseWheelDown);
      Vector2 mouse_world_position = GetScreenToWorld2D(context->mouse_position, context->camera);
      context->camera.offset = context->mouse_position;
      context->camera.target = mouse_world_position;
      F32 zoom_delta;

      // HACK: is it hacky that when check the mouse wheel and handle it differently like this? maybe not.
      if ((zoom_in && in_wheel) || (zoom_out && out_wheel)) {
        zoom_delta = -0.1f * ui_state->mouse_wheel_movement.y;
      } else if (zoom_in) {
        zoom_delta = 0.2f;
      } else {
        zoom_delta = -0.2f;
      }

      context->camera.zoom += zoom_delta;
      context->camera.zoom = Max(0.1f, context->camera.zoom);
    }
  }

  // process interaction
  for (Process *p = context->processes.first; p != 0; p = p->next) {
    selection = get_process_selection(context, p);
    ui_state->hot_id_assigned = selection.hot_id_assigned || ui_state->hot_id_assigned;
    B32 is_active = is_active_process(context, p);

    // check if we need to stop dragging wire
    if (should_stop_dragging) {
      // unset drag flag
      B32 wire_drag_flag = Process_Flag_Drag_In | Process_Flag_Drag_Out;
      if (Get_Flag(p->flags, wire_drag_flag)) {
        B32 is_in = Get_Flag(p->flags, Process_Flag_Drag_In);
        Unset_Flag(p->flags, wire_drag_flag);
        moved_wire = p;
        moved_wire_conn = is_in ? Process_Connection_In : Process_Connection_Out;
      }
    }

    if (check_keybind(context, Ui_Feature_SelectSingleProcess, selection) == Keybind_Result_Enter) {
      B32 in_selection = selection.type == Process_Selection_In;
      B32 out_selection = selection.type == Process_Selection_Out;
      if (in_selection || out_selection) {
        // select wire
        Process *wire = get_process_wire_by_selection(context, selection);
        B32 is_active_wire = is_active_process(context, wire);

        if (wire) {
          U32 drag_flag = in_selection ? Process_Flag_Drag_In : Process_Flag_Drag_Out;
          Unset_Flag(context->flags, Context_Flag_NewWire);
          Set_Flag(wire->flags, drag_flag);
          context->active_position = context->mouse_position;
          if (!is_active_wire) {
            clear_active_processes(context);
            SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, wire, next_active, 0);
          }
        }
      } else if ((is_active || context->hot_process == p) &&
                 selection.type == Process_Selection_NewWire) {
        // begin new-wire
        Set_Flag(context->flags, Context_Flag_NewWire);
        if (!is_active) {
          clear_active_processes(context);
          SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, p, next_active, 0);
        }
      } else if (selection.type == Process_Selection_Process) {
        if (Get_Flag(context->flags, Context_Flag_NewWire)) {
          // connect processes
          connect_processes(context, context->active_processes.first, p);
        } else {
          // select process
          context->hot_process = p;
          if (!is_active) {
            clear_active_processes(context);
            SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, p, next_active, 0);
          }
          Unset_Flag(context->flags, Context_Flag_NewWire);
          Set_Flag(context->flags, Context_Flag_Dragging);
          context->active_position = context->mouse_position;
        }
      }
    } else if (check_keybind(context, Ui_Feature_SelectAnotherProcess, selection) == Keybind_Result_Enter) {
      // select another process
      if (selection.type == Process_Selection_In || selection.type == Process_Selection_Out) {
        Process *wire = get_process_wire_by_selection(context, selection);
        if (wire) {
          if (is_active_process(context, wire)) {
            remove_process_from_active_processes(context, wire);
          } else {
            SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, wire, next_active, 0);
          }
        }
      } else if (selection.type == Process_Selection_Process) {
        if (is_active_process(context, selection.process)) {
          remove_process_from_active_processes(context, selection.process);
        } else {
          SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, selection.process, next_active, 0);
        }
      }
    } else if (selection.type == Process_Selection_Process) {
      // process hover
      context->hot_process = p;
    }

    // bounding
    if (Get_Flag(context->flags, Context_Flag_Bounding)) {
      Rectangle selection_rectangle = get_selection_rectangle(context);

      if (Get_Flag(p->flags, Process_Flag_Wire)) {
        Process_Shape out_shape = get_process_shape(context, p->out);
        Process_Shape in_shape = get_process_shape(context, p->in);
        Vector2 out_position = get_process_wire_position(context, p->out, out_shape, Process_Connection_Out, p->which_out);
        Vector2 in_position = get_process_wire_position(context, p->in, in_shape, Process_Connection_In, p->which_in);

        if (rectangle_contains_point(selection_rectangle, out_position) ||
            rectangle_contains_point(selection_rectangle, in_position)) {
          SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, p, next_active, 0);
        }
      } else {
        Process_Shape shape = get_process_shape(context, p);

        if (rectangle_contains_point(selection_rectangle, shape.center)) {
          SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, p, next_active, 0);
        }
      }
    }
  }

  // zero out selection
  selection = (Process_Selection){0};
  // zero the old hot-id
  if (!ui_state->hot_id_assigned) {
    context->hot_process = 0;
  }

  // more rectangle selection handling
  if (Get_Flag(context->flags, Context_Flag_Bounding)) {
    // add hot process to active processes
    if (context->hot_process) {
      B32 hot_is_active = is_active_process(context, context->hot_process);
      if (!hot_is_active) {
        SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, context->hot_process, next_active, 0);
      }
    }
  }

  // create process
  if (check_keybind(context, Ui_Feature_CreateProcess, selection)) {
    Process *new_p = create_process(context);
    if (new_p) {
      Set_Flag(new_p->flags, Process_Flag_TextEdit);
      new_p->position = GetScreenToWorld2D(context->mouse_position, context->camera);
      clear_active_processes(context);
      SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, new_p, next_active, 0);
    }
  }

  // cancel selection
  if (check_keybind(context, Ui_Feature_CancelSelection, selection)) {
    clear_active_processes(context);
    Unset_Flag(context->flags, Context_Flag_NewWire);
  }

  // enter bounding
  if (check_keybind(context, Ui_Feature_Bound, selection) == Keybind_Result_Enter) {
    Set_Flag(context->flags, Context_Flag_Bounding);
    context->active_position = context->mouse_position;
  }

  // toggle between rounded and triangular shapes
  if (check_keybind(context, Ui_Feature_ToggleDisplayMode, selection) == Keybind_Result_Enter) {
    Toggle_Flag(context->flags, Context_Flag_RoundedShapes);
  }

  // copy processes
  if (check_keybind(context, Ui_Feature_CopyProcess, selection) == Keybind_Result_Enter) {
    copy_active_processes(context);
  }
  // paste processes
  if (check_keybind(context, Ui_Feature_PasteProcess, selection) == Keybind_Result_Enter) {
    paste_processes(context);
  }

  // handle moved wire
  if (moved_wire && context->hot_process) {
    if (Get_Flag(context->hot_process->flags, Process_Flag_Wire)) {
      Process *connected_process = context->hot_process->conn[moved_wire_conn];
      if (connected_process) {
        // move wire to hovered wire
        U32 which_conn = context->hot_process->which_conn[moved_wire_conn];
        if (moved_wire != context->hot_process) {
          remove_wire_connection(context, moved_wire, (1<<moved_wire_conn));
          add_wire_connection(context, moved_wire, connected_process, moved_wire_conn, which_conn);
        }
      }
    } else {
      Process *connected_process = context->hot_process;
      // move wire to last wire of process
      U32 which_conn;
      if (moved_wire->conn[moved_wire_conn] == connected_process) {
        which_conn = connected_process->conn_count[moved_wire_conn] - 1;
      } else {
        which_conn = connected_process->conn_count[moved_wire_conn];
      }
      remove_wire_connection(context, moved_wire, (1<<moved_wire_conn));
      add_wire_connection(context, moved_wire, connected_process, moved_wire_conn, which_conn);
    }
  }

  // handle active-process
  if (context->active_processes.first) {
    B32 is_dragging = Get_Flag(context->flags, Context_Flag_Dragging);
    if (is_dragging && should_stop_dragging) {
      // update positions of active processes
      for (Process *a = context->active_processes.first; a != 0; a = a->next_active) {
        Vector2 new_position = get_process_position(context, a);
        a->position = new_position;
      }
      // stop dragging
      Unset_Flag(context->flags, Context_Flag_Dragging);
    } else if (check_keybind(context, Ui_Feature_CycleProcessDisplay, selection)) {
      // cycle through special process types (cups/caps/empty)
      for (Process *a = context->active_processes.first; a != 0; a = a->next_active) {
        if (!Get_Flag(a->flags, Process_Flag_Wire)) {
          U32 toggle_flags = (Process_Flag_Empty | Process_Flag_Cup | Process_Flag_Cap | Process_Flag_Identity);
          if (Get_Flag(a->flags, toggle_flags)) {
            // toggle off process-display flags first, before trying to toggle them on
            Unset_Flag(a->flags, toggle_flags);
          } else if ((a->in_count == 0 && a->out_count == 0) ||
                     (a->in_count == 1 && a->out_count == 0) ||
                     (a->in_count == 0 && a->out_count == 1)) {
            // toggle single in/out or unconnected process
            Toggle_Flag(a->flags, Process_Flag_Empty);
          } else if (a->in_count == 0 && a->out_count == 2) {
            Toggle_Flag(a->flags, Process_Flag_Cup);
          } else if (a->in_count == 2 && a->out_count == 0) {
            Toggle_Flag(a->flags, Process_Flag_Cap);
          } else if (a->in_count == 1 && a->out_count == 1) {
            Toggle_Flag(a->flags, Process_Flag_Identity);
          }
        }
      }
    } else if (check_keybind(context, Ui_Feature_DeleteProcess, selection)) {
      // delete processes
      for (Process *a = context->active_processes.first; a != 0;) {
        Process *next_active = a->next_active;
        delete_process(context, a);
        a = next_active;
      }
      clear_active_processes(context);
    } else if (!ui_state->action_occured) {
      // process label editing
      handle_label_editing(context, context->active_processes);
    }
  }
}



function void handle_user_input(Context *context) {
  context->ui_state = get_ui_state(context);
  handle_ui(context);
  if (!context->ui_state.action_occured) {
    handle_process_interaction(context);
  }
}


function void draw_circular_process(Context *context, Vector2 center, F32 radius, F32 thickness, Color bg_color, Color stroke_color) {
  Render_Context *rc = &context->process_render_context;

  render_DrawCircle(rc, center, radius, bg_color);
  F32 fudge = Half_Circle_Fudge*radius;
  Vector2 first_point = (Vector2){center.x-radius, center.y};
  Vector2 second_point = (Vector2){center.x+radius, center.y};
  Vector2 control0 = (Vector2){first_point.x, first_point.y-fudge};
  Vector2 control1 = (Vector2){second_point.x, second_point.y-fudge};
  render_DrawLineBezierCubic(rc, first_point, second_point, control0, control1, thickness, stroke_color);
  Vector2 control2 = (Vector2){first_point.x, first_point.y+fudge};
  Vector2 control3 = (Vector2){second_point.x, second_point.y+fudge};
  render_DrawLineBezierCubic(rc, first_point, second_point, control2, control3, thickness, stroke_color);
}




function void draw_process_with_triangle_fan(Context *context, Process_Shape shape, F32 thickness, Color bg_color, Color stroke_color) {
  Assert(shape.triangle_count == (shape.point_count - 2));
  Render_Context *rc = &context->process_render_context;

  // draw background
  render_DrawTriangleFan(rc, shape.points, shape.point_count, bg_color);

  // draw lines
  for (S32 i = 0; i < shape.point_count-1 && i < Process_Shape_Max_Points; ++i) {
    Vector2 p0 = shape.points[i];
    Vector2 p1 = shape.points[i+1];
    render_DrawLine(rc, p0.x, p0.y, p1.x, p1.y, thickness, stroke_color);
  }

  // draw line from last point to first point
  Vector2 p0 = shape.points[0];
  Vector2 p1 = shape.points[shape.point_count-1];
  render_DrawLine(rc, p0.x, p0.y, p1.x, p1.y, thickness, stroke_color);
}



function void draw_process_with_triangle_strip(Context *context, Process_Shape shape, F32 thickness, Color bg_color, Color stroke_color) {
  Render_Context *rc = &context->process_render_context;
  // draw process background
  render_DrawTriangleStrip(rc, shape.points, shape.point_count, bg_color);

  if (shape.triangle_count) {
    // draw first two lines
    Vector2 p0 = shape.points[0];
    Vector2 p1 = shape.points[1];
    Vector2 p2 = shape.points[2];
    render_DrawLine(rc, p0.x, p0.y, p1.x, p1.y, thickness, stroke_color);
    render_DrawLine(rc, p0.x, p0.y, p2.x, p2.y, thickness, stroke_color);

    // draw in-between lines
    for (S32 i = 1; i < shape.triangle_count; ++i) {
      Vector2 p0 = shape.points[i];
      Vector2 p1 = shape.points[i+2];
      render_DrawLine(rc, p0.x, p0.y, p1.x, p1.y, thickness, stroke_color);
    }

    // draw line connecting last two points
    p0 = shape.points[shape.point_count-1];
    p1 = shape.points[shape.point_count-2];
    render_DrawLine(rc, p0.x, p0.y, p1.x, p1.y, thickness, stroke_color);
  }
}


function void draw_processes(Context *context) {
  Render_Context *rc = &context->process_render_context;

  Color bg_color = (Color){255, 255, 255, 255};
  Color invisible_bg_color = (Color){0, 0, 0, 0};
  Color stroke_color = (Color){0, 0, 0, 255};
  Color invisible_stroke_color = (Color){0, 0, 0, 100};
  Color text_color = (Color){0, 0, 0, 255};
  Color box_color = (Color){10, 190, 40, 255};
  Color box_hover_color = (Color){5, 250, 20, 255};

  F32 font_size = context->camera.zoom * global_process_font_size;

  F32 padding = global_process_wire_padding;
  F32 spacing = global_process_wire_spacing;
  B32 rounded = Get_Flag(context->flags, Context_Flag_RoundedShapes);

  // draw processes
  for (Process *p = context->processes.first; p != 0; p = p->next) {
    B32 is_wire = Get_Flag(p->flags, Process_Flag_Wire);
    U8 *label_c_string = c_string_from_string_chunk_list(context->temp_arena, &p->label);
    S32 text_width = MeasureText((char *)label_c_string, font_size);

    if (!is_wire) {
      Process_Shape shape = get_process_shape(context, p);

      B32 is_hot = context->hot_process == p;
      B32 is_active = is_active_process(context, p);
      F32 thickness = (is_hot||is_active) ? global_active_line_thickness : global_line_thickness;
      thickness *= context->camera.zoom;
      F32 cup_cap_control_offset = 10.0f;

      if (Get_Flag(p->flags, Process_Flag_Empty)) {
        // draw line through empty shape
        B32 upward = p->in_count == 1 && p->out_count == 0;
        B32 downward = p->in_count == 0 && p->out_count == 1;
        // only if it's valid
        if (upward || downward) {
          Vector2 p0 = (Vector2){0};
          Vector2 p1 = (Vector2){0};
          if (rounded) {
            // rounded half-circle
            Vector2 position = get_process_position(context, p);
            position = GetWorldToScreen2D(position, context->camera);
            Half_Circle_Points points = get_half_circle_points(context, shape, p, position, text_width, downward);
            p0 = points.middle_of_line;
            p1 = points.middle_of_curve;
          } else {
            if (upward) {
              // upward triangle
              p0 = get_percentage_between_points(shape.points[1], shape.points[2], 0.5f);
              p1 = shape.points[0];
            } else if (downward) {
              // downward triangle
              p0 = get_percentage_between_points(shape.points[0], shape.points[1], 0.5f);
              p1 = shape.points[2];
            }
          }
          render_DrawLineBezierCubic(rc, p0, p1, p1, p0, thickness, stroke_color);
        } else if (!label_c_string[0]) {
          if (rounded) {
            draw_circular_process(context, shape.center, shape.radius, thickness, invisible_bg_color, invisible_stroke_color);
          } else {
            draw_process_with_triangle_strip(context, shape, thickness, invisible_bg_color, invisible_stroke_color);
          }
        }
      } else if (Get_Flag(p->flags, Process_Flag_Cup)) {
        // draw cup
        Vector2 pos0 = get_process_wire_position(context, p, shape, Process_Connection_Out, 0);
        Vector2 pos1 = get_process_wire_position(context, p, shape, Process_Connection_Out, 1);
        Vector2 ctrl0 = (Vector2){pos0.x, pos0.y+cup_cap_control_offset};
        Vector2 ctrl1 = (Vector2){pos1.x, pos1.y+cup_cap_control_offset};
        render_DrawLineBezierCubic(rc, pos0, pos1, ctrl0, ctrl1, thickness, stroke_color);
      } else if (Get_Flag(p->flags, Process_Flag_Cap)) {
        // draw cap
        Vector2 pos0 = get_process_wire_position(context, p, shape, Process_Connection_In, 0);
        Vector2 pos1 = get_process_wire_position(context, p, shape, Process_Connection_In, 1);
        Vector2 ctrl0 = (Vector2){pos0.x, pos0.y-cup_cap_control_offset};
        Vector2 ctrl1 = (Vector2){pos1.x, pos1.y-cup_cap_control_offset};
        render_DrawLineBezierCubic(rc, pos0, pos1, ctrl0, ctrl1, thickness, stroke_color);
      } else if (Get_Flag(p->flags, Process_Flag_Identity)) {
        // draw "identity" process (just a wire)
        Vector2 pos0 = get_process_wire_position(context, p, shape, Process_Connection_In, 0);
        Vector2 pos1 = get_process_wire_position(context, p, shape, Process_Connection_Out, 0);
        render_DrawLineBezierCubic(rc, pos0, pos1, pos1, pos0, thickness, stroke_color);
      } else {
        switch(shape.kind) {
        case Process_Shape_TriangleStrip: {
          draw_process_with_triangle_strip(context, shape, thickness, bg_color, stroke_color);
        } break;
        case Process_Shape_TriangleFan: {
          draw_process_with_triangle_fan(context, shape, thickness, bg_color, stroke_color);
        } break;
        case Process_Shape_Circle: {
          draw_circular_process(context, shape.center, shape.radius, thickness, bg_color, stroke_color);
        } break;
        case Process_Shape_HalfCircle: {
          // draw half-circle background
          render_DrawTriangleFan(rc, shape.points, shape.point_count, bg_color);
          // draw half-circle lines
          for (S32 i = 0; i < shape.point_count-1; ++i) {
            Vector2 p0 = shape.points[i];
            Vector2 p1 = shape.points[i+1];
            render_DrawLine(rc, p0.x, p0.y, p1.x, p1.y, thickness, stroke_color);
          }
          // connect the line endpoints
          render_DrawLine(rc,
                          shape.points[0].x,
                          shape.points[0].y,
                          shape.points[shape.point_count-1].x,
                          shape.points[shape.point_count-1].y,
                          thickness, stroke_color);
        } break;
        default: Assert(0);
        }
      }

      // draw label
      if (label_c_string[0]) {
        F32 text_x = shape.center.x-0.5f*text_width;
        F32 text_y = shape.center.y-0.5f*font_size;
        if (shape.kind == Process_Shape_HalfCircle) {
          F32 flip = shape.downward ? -1.0f : 1.0f;
          F32 fudge = 0.9f;
          F32 offset = fudge * flip * (0.5f * shape.radius);
          text_y -= offset;
        }
        render_DrawText(rc, (char *)label_c_string, text_x, text_y, font_size, text_color, 0);
      }

      // draw new-wire-box
      if (is_active || is_hot) {
        Rectangle new_wire_box = get_new_wire_box(context, p, shape);
        B32 new_wire_box_is_active = (
          (is_active && Get_Flag(context->flags, Context_Flag_NewWire)) ||
          rectangle_contains_point(new_wire_box, context->mouse_position));
        Color color = new_wire_box_is_active ? box_hover_color : box_color;
        render_DrawRectangleRec(rc, new_wire_box, color);
      }
    }
  }

  // draw wires
  for (Process *p = context->processes.first; p != 0; p = p->next) {
    B32 is_wire = Get_Flag(p->flags, Process_Flag_Wire);

    if (is_wire) {
      Process_Shape out_shape = get_process_shape(context, p->out);
      Process_Shape in_shape = get_process_shape(context, p->in);

      Vector2 out_position = get_process_wire_position(context, p->out, out_shape, Process_Connection_Out, p->which_out);
      Vector2 in_position = get_process_wire_position(context, p->in, in_shape, Process_Connection_In, p->which_in);
      if (Get_Flag(p->flags, Process_Flag_Drag_In)) {
        // @Copypasta get_process_position
        Vector2 delta = Vector2Subtract(context->mouse_position, context->active_position);
        in_position = Vector2Add(in_position, delta);
      } else if (Get_Flag(p->flags, Process_Flag_Drag_Out)) {
        Vector2 delta = Vector2Subtract(context->mouse_position, context->active_position);
        out_position = Vector2Add(out_position, delta);
      }

      Vector2 out_control = out_position;
      out_control.y -= context->camera.zoom * 30.0f;
      Vector2 in_control = in_position;
      in_control.y += context->camera.zoom * 30.0f;

      B32 is_active = is_active_process(context, p) || context->hot_process == p;
      B32 connected_in_active = (is_active_process(context, p->in) ||
                                 context->hot_process == p->in);
      B32 connected_out_active = (is_active_process(context, p->out) ||
                                  context->hot_process == p->out);
      F32 thickness = is_active ? global_active_line_thickness : global_line_thickness;
      thickness *= context->camera.zoom;

      // draw wire
      render_DrawLineBezierCubic(rc, out_position, in_position, out_control, in_control, thickness, stroke_color);

      // draw out wire-box
      if (connected_out_active || is_active) {
        Rectangle box = get_wire_box(context, out_position);
        Color c = is_active ? box_hover_color : box_color;
        render_DrawRectangleRec(rc, box, c);
      }

      // draw in wire-box
      if (connected_in_active || is_active) {
        Rectangle box = get_wire_box(context, in_position);
        Color c = is_active ? box_hover_color : box_color;
        render_DrawRectangleRec(rc, box, c);
      }
    }
  }

  // draw new wire
  if (Get_Flag(context->flags, Context_Flag_NewWire) && context->active_processes.first) {
    Process_Shape shape = get_process_shape(context, context->active_processes.first);
    Vector2 position = shape.new_wire_position;

    Vector2 from_control = position;
    from_control.y -= context->camera.zoom * 30.f;
    Vector2 to_control = context->mouse_position;
    to_control.y += context->camera.zoom * 30.0f;

    F32 thickness = context->camera.zoom * global_line_thickness;

    render_DrawLineBezierCubic(rc, position, context->mouse_position, from_control, to_control, thickness, stroke_color);
  }

  // draw selection rectangle
  if (Get_Flag(context->flags, Context_Flag_Bounding)) {
    Rectangle selection_rect = get_selection_rectangle(context);
    Color selection_color = (Color){10, 30, 200, 50};

    render_DrawRectangleRec(rc, selection_rect, selection_color);
  }
}




function S32 debug_process_list_count(Process_List list) {
  S32 count = 0;
  for (Process *p = list.first; p != 0; p = p->next) {
    count += 1;
  }
  return count;
}
function S32 debug_process_active_list_count(Process_List list) {
  S32 count = 0;
  for (Process *p = list.first; p != 0; p = p->next_active) {
    count += 1;
  }
  return count;
}

function void draw_info_panel(Context *context) {
  Render_Context *rc = &context->ui_render_context;
  Color text_color = (Color){0, 0, 0, 255};
  F32 x = 5.0f;
  F32 y = 5.0f;
  F32 padding = 2.0f;

#if 0
  if (context->active_processes.first) {
    for (Process *a = context->active_processes.first; a != 0; a = a->next_active) {
      char *format = a == context->active_processes.first ? "active-id = %p" : "            %p";
      const char *text = TextFormat(format, a);
      render_DrawText(rc, text, 5.0f, y, global_panel_font_size, text_color, 1);
      y += global_panel_font_size + padding;
    }
  }
#elif 0
  render_DrawText(rc, TextFormat("process count %d\n", debug_process_list_count(context->processes)),
                  x, y, global_panel_font_size, text_color, 1);
  y += global_panel_font_size + padding;
  render_DrawText(rc, TextFormat("active count %d\n", debug_process_active_list_count(context->active_processes)),
                  x, y, global_panel_font_size, text_color, 1);
  y += global_panel_font_size + padding;
  render_DrawText(rc, TextFormat("free count %d\n", debug_process_list_count(context->free_processes)),
                  x, y, global_panel_font_size, text_color, 1);
  y += global_panel_font_size + padding;
  render_DrawText(rc, TextFormat("copy count %d\n", debug_process_list_count(context->copy_processes)),
                  x, y, global_panel_font_size, text_color, 1);
  y += global_panel_font_size + padding;
#elif 1
  S32 arena_font_size = 12;
  y = global_window_size.y - arena_font_size - padding;
  render_DrawText(rc, TextFormat("ui arena %llu/%llu\n", context->ui_arena->chunk_pos, context->ui_arena->chunk_cap), x, y, arena_font_size, text_color, 1);
  y -= arena_font_size + padding;
  render_DrawText(rc, TextFormat("temp arena %llu/%llu\n", context->temp_arena->chunk_pos, context->temp_arena->chunk_cap), x, y, arena_font_size, text_color, 1);
  y -= arena_font_size + padding;
  render_DrawText(rc, TextFormat("render arena %llu/%llu\n", context->render_arena->chunk_pos, context->render_arena->chunk_cap), x, y, arena_font_size, text_color, 1);
  y -= arena_font_size + padding;
  render_DrawText(rc, TextFormat("process arena %llu/%llu\n", context->permanent_arena->chunk_pos, context->permanent_arena->chunk_cap), x, y, arena_font_size, text_color, 1);
  y -= arena_font_size + padding;
#endif
}





function Context initialize_context(void) {
  Context context = (Context){0};

  context.render_arena = arena_alloc_reserve(Megabytes(1), 0);
  context.permanent_arena = arena_alloc_reserve(Megabytes(1), 0);
  context.temp_arena = arena_alloc_reserve(Megabytes(1), 0);
  context.ui_arena = arena_alloc_reserve(Megabytes(1), 0);

  context.ui_render_context.arena = context.render_arena;
  context.process_render_context.arena = context.render_arena;

  context.camera.zoom = 1.0f;

  return context;
}






typedef struct {
  char *c_string;
  String_Chunk_List *string_chunk_list;
} Init_String_Chunk;

function void initialize_global_string_chunk_lists(Context *context) {
  Init_String_Chunk inits[] = {
    {"open", &global_open_button_label},
    {"cancel", &global_cancel_button_label},
    {"save", &global_save_button_label},
    {"save as", &global_save_as_button_label},
    {"File", &global_file_button_label},
  };

  for (S32 i = 0; i < ArrayCount(inits); ++i) {
    Init_String_Chunk init = inits[i];
    String_Chunk *sc = push_struct(context->permanent_arena, String_Chunk);
    SLLQueuePush(init.string_chunk_list->first, init.string_chunk_list->last, sc);
    S32 char_index = 0;
    for (;;) {
      sc->str_array[char_index] = init.c_string[char_index];
      if (init.c_string[char_index] == 0) {
        break;
      }
      char_index += 1;
    }
  }
}

function void initialize_globals(Context *context) {
  S32 monitor_id = GetCurrentMonitor();
  S32 screen_width = GetMonitorWidth(monitor_id);
  S32 screen_height = GetMonitorHeight(monitor_id);

  global_background_color = (Color){220, 220, 200, 255};

  global_window_size.x = 0.7f*(F32)screen_width;
  global_window_size.y = 0.7f*(F32)screen_height;

  global_shape_size = global_window_size.x / 20.0f;
  global_shape_half_size = 0.5f*global_shape_size;

  global_box_size = global_shape_size*0.22f;
  global_box_half_size = 0.5f*global_box_size;

  global_process_wire_padding = 0.2f*global_shape_size;
  global_process_wire_spacing = 0.55f*global_shape_size;

  global_process_font_size = 0.4f*global_shape_size;
  global_panel_font_size = 0.35f*global_shape_size;

  global_line_thickness = 0.05f*global_shape_size;
  global_active_line_thickness = 0.1f*global_shape_size;

  initialize_global_string_chunk_lists(context);

  load_keybinds(context);
}





int main(void) {
  InitWindow(800, 500, "proc");
  SetExitKey(0);
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);

  Context context = initialize_context();
  initialize_globals(&context);
  SetWindowSize(global_window_size.x, global_window_size.y);

  Render_Context *prc = &context.process_render_context;
  render_Initialize(context.temp_arena);

  while (!WindowShouldClose()) {
    handle_user_input(&context);

    render_ClearBackground(prc, global_background_color);
    draw_processes(&context);
#if 1
    draw_info_panel(&context);
#endif

    BeginDrawing();
    render_Commands(&context.process_render_context);
    render_Commands(&context.ui_render_context);

    // clear out per-frame stuff
    arena_pop_to(context.render_arena, 0);
    arena_pop_to(context.temp_arena, 0);
    context.ui_render_context.command_list.first = 0;
    context.ui_render_context.command_list.last = 0;
    context.process_render_context.command_list.first = 0;
    context.process_render_context.command_list.last = 0;

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
