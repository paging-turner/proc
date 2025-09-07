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
     [ ] Allow reordering of connected wires
     [ ] BUG: While making a new wire-connection, if you click on another wire-box, the new-wire jumps to a place off the screen. Clicking on a process while in this busted state makes a new wire that leads to some invisible process.
   === Multi-select ===
     [x] Allow multi-selection of processes
     [x] Allow dragging all selected processes
     [x] Allow de-selectin of processes
     [x] Click-and-drag selection rectangle
     [ ] Ctrl-click-and-drag to include more processes
   === Zooming and Panning ===
     [x] BUG: When zoomed way out, the wire positioning gets messed up.
   === Graphics ===
     [ ] Replace line-drawing calls with a call that draws triangle-strips/fans. This should help deal with how to cleanly connect the ends of lines together.
   === Testing ===
     [ ] Enumerate some test cases, to at least be able to manually check that things are working.
     [ ] Automated test??
   [ ] Copy-paste of selected processes
   [ ] Use a font other than the raylib default
   [ ] Expand base-layer and let it consume core.h and ryn_memory.h
   [ ] Make some sliders/fields for global settings like process-size and font-size.
   [ ] Show cursor when editing the text of a process.
   [x] Allow toggling on/off "mr4th style" process drawing, which is a variation on the visual style of diragrams in the book.
     [x] Move towards defining shapes using triangle strips/fans. We used some raylib funcs for circles and stuff just because it was easy, but now we need more control.
     [x] Implement collision detection for triangle strip/fan so we can just define a shape with triangles and be able to interact and draw with the same shape.
   [ ] BUG: If you toggle a process to be a special display (cup/cap/invisible), and then connect a new wire to it, the special visual still applies and you cannot toggle away. When connecting wires, we need to check if the special display flag should be unset.
   [ ] Undo/redo
   [ ] BUG: Connect a two processes. Make one process invisible. Delete the *other* process. The invisible process is still there but, well, you can't see it! Either delete the invisible one, or make it visible again. Probably just delete it??
   [ ] New Arena Changes
     [x] Change how we loop through processes, so that we can enable growable arenas.
     [x] Right now we have to make sure the Process struct is a size that's a multiple of a pointer. We should fix that :(
     [ ] Enable growable arenas
*/





#include <stdio.h>

#define MR4TH_NO_INCLUDES 1
#define MR4TH_NO_CLAMP 1
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




//////////////////////////////////////
// Process
//////////////////////////////////////

// TODO: Should Process_Flag just be a non-flag enum?
typedef enum {
  Process_Flag_Wire    = 1 << 0,
  Process_Flag_Empty   = 1 << 1,
  Process_Flag_Cup     = 1 << 2,
  Process_Flag_Cap     = 1 << 3,
} Process_Flag;

typedef struct Process Process;
struct Process {
  Vector2 position;

  B32 flags;

  S32 in_count;
  S32 out_count;

  Process *in_id;
  Process *out_id;

  U32 which_in;
  U32 which_out;

  Process *next;
  Process *next_active;

  // TODO: Use a growable structure for strings.
#define Process_Label_Size 64
  U8 label[Process_Label_Size];
  U32 label_cursor;
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




//////////////////////////////////////
// Context
//////////////////////////////////////


// TODO: maybe this should be a mode and not flags?
typedef enum {
  Context_Flag_Dragging       = 1 << 0,
  Context_Flag_Bounding       = 1 << 1,
  Context_Flag_Panning        = 1 << 2,
  Context_Flag_NewWire        = 1 << 3,
  Context_Flag_EditText       = 1 << 4,
  Context_Flag_RoundedShapes  = 1 << 5,
} Context_Flag;

typedef struct {
  Vector2 mouse_position;
  B32 mouse0_pressed;
  B32 mouse1_pressed;
  B32 mouse0_down;
  B32 mouse1_down;
  B32 hot_id_assigned;
  Vector2 mouse_wheel_movement;
  B32 control_down;
  B32 shift_down;
  B32 alt_down;
} Ui_State;

typedef struct {
  Arena *render_arena;
  Arena *process_arena;
  Arena *temp_arena;
  U64 render_zero_pos;
  U64 process_zero_pos;
  U64 temp_zero_pos;

  U32 flags;

  Process_List processes;
  Process_List free_processes;

  Process *hot_process;
  Process_List active_process;

  Vector2 mouse_position; // TODO: move this to ui_state
  Vector2 active_position;

  Camera2D camera;

  Ui_State ui_state;
} Context;



#include "../source/render.h"



//////////////////////////////////////
// Process Shape
//////////////////////////////////////

typedef enum {
  Process_Shape_Triangle,
  Process_Shape_Quadrangle, // TODO: Do we need both Quadrangle and Rectangle?
  Process_Shape_Rectangle,
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
} Process_Shape;




//////////////////////////////////////
// Keybinds
//////////////////////////////////////

#define Ui_Feature_Xlist\
  X(Bound)\
  X(Pan)\
  X(ZoomIn)\
  X(ZoomOut)\
  X(SelectSingleProcess)\
  X(SelectAnotherProcess)\
  X(CancelSelection)\
  X(CreateProcess)\
  X(DeleteProcess)\
  X(BeginEditText)\
  X(CycleProcessDisplay)\
  X(ToggleDisplayMode)

typedef enum {
  Ui_Feature__Null,
#define X(feature)\
  Ui_Feature_##feature,
  Ui_Feature_Xlist
#undef X
  Ui_Feature__Count,
} Ui_Feature;

typedef enum {
  Ui_Constraint__Null         = 0,
  Ui_Constraint_HoverProcess  = (1 << 1),
  Ui_Constraint_NoHotProcess  = (1 << 2),
  Ui_Constraint_ExitOnKeyup   = (1 << 3),
} Ui_Constraint;

// NOTE: These enum values start with values higher than raylib's highest KEY_* value, which is in the 300s
typedef enum {
  Key_Kind_Mouse0 = 1000,
  Key_Kind_Mouse1,
  Key_Kind_MouseWheelUp,
  Key_Kind_MouseWheelDown,
} Key_Kind;

typedef enum {
  Modifier_Key__Null    = 0,
  Modifier_Key_Control  = (1 << 0),
  Modifier_Key_Shift    = (1 << 2),
  Modifier_Key_Alt      = (1 << 3),
} Modifier_Key;

typedef struct {
  Ui_Feature feature;
  U32 key_kind; // Uses raylib's KEY_* enum and some special ones for mouse-keys
  U32 modifiers;
  Ui_Constraint constraint;
  char *description;
} Keybind;

typedef enum {
  Keybind_Result__Null,
  Keybind_Result_Enter,
  Keybind_Result_Exit,
} Keybind_Result;

#define Keybind_Xlist\
  X(Bound, Key_Kind_Mouse0, 0,\
    Ui_Constraint_NoHotProcess|Ui_Constraint_ExitOnKeyup,\
    "Select multiple processes by drawing a rectangle with your mouse.")\
\
  X(Pan, Key_Kind_Mouse1, 0,\
    Ui_Constraint_ExitOnKeyup,\
    "Slide your field of view by moving your mouse.")\
\
  X(ZoomIn, Key_Kind_MouseWheelUp, 0, 0,\
    "Zoom your field of view in to make objects appear closer.")\
\
  X(ZoomOut, Key_Kind_MouseWheelDown, 0, 0,\
    "Zoom your field of view out to make objects appear further.")\
\
  X(SelectSingleProcess, Key_Kind_Mouse0, 0,\
    Ui_Constraint_HoverProcess|Ui_Constraint_ExitOnKeyup,\
    "Select a single process.")\
\
  X(SelectAnotherProcess, Key_Kind_Mouse0, Modifier_Key_Control,\
    Ui_Constraint_HoverProcess,\
    "Add a process to the selected processes.")\
\
  X(CancelSelection, Key_Kind_Mouse0, 0,\
    Ui_Constraint_NoHotProcess,\
    "Clear out the selected processes.")\
\
  X(CreateProcess, Key_Kind_Mouse0, Modifier_Key_Control,\
    Ui_Constraint_NoHotProcess,\
    "Create a new process.")\
\
  X(DeleteProcess, KEY_BACKSPACE, 0, 0,\
    "Delete the selected processes.")\
\
  X(BeginEditText, KEY_I, 0, 0,\
    "Begin text-insertion mode.")\
\
  X(CycleProcessDisplay, KEY_TAB, 0, 0,\
    "Cycle through special displays for selected processes.")\
\
  X(ToggleDisplayMode, KEY_M, 0, 0,\
    "Toggle between 'classic' and 'rounded' display modes.")





//////////////////////////////////////
// Globals
//////////////////////////////////////

global_variable F32 global_window_width;
global_variable F32 global_window_height;

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

global_variable Keybind global_keybind_lookup[Ui_Feature__Count];

#define Half_Circle_Fudge 1.32f
#define Half_Circle_Radius_Fudge 1.0f





function Vector2 get_percentage_between_points(Vector2 p0, Vector2 p1, F32 percentage) {
  Vector2 norm_delta = Vector2Normalize(Vector2Subtract(p1, p0));
  F32 distance_along_delta = percentage * Vector2Distance(p1, p0);
  Vector2 center = Vector2Add(p0, Vector2Scale(norm_delta, distance_along_delta));

  return center;
}




function B32 is_active_process(Context *context, Process *p) {
  B32 is_active = 0;

  for (Process *test_p = context->active_process.first; test_p != 0; test_p = test_p->next_active) {
    if (test_p == p) {
      is_active = 1;
      break;
    }
  }

  return is_active;
}


function void clear_active_process(Context *context) {
  for (Process *p = context->active_process.first; p != 0;) {
    Process *next_active = p->next_active;
    p->next_active = 0;
    p = next_active;
  }

  context->active_process.first = 0;
  context->active_process.last = 0;
}



function Vector2 get_process_position(Context *context, Process *process) {
  Vector2 position = process->position;
  B32 is_active = is_active_process(context, process);
  B32 is_dragging = Get_Flag(context->flags, Context_Flag_Dragging);


  if (is_active && is_dragging) {
    Vector2 delta = Vector2Subtract(context->mouse_position, context->active_position);
    position = Vector2Add(position, Vector2Scale(delta, 1.0f/context->camera.zoom));
  }

  return position;
}





function Vector2
get_process_wire_out_position(Context *context, Process *p, Process_Shape shape, U32 wire_index) {
  F32 padding = context->camera.zoom * global_process_wire_padding;
  Vector2 p0 = shape.points[0];
  Vector2 p1 = shape.points[1];

  if (shape.kind == Process_Shape_HalfCircle) {
    p0 = shape.points[shape.point_count-1];
    p1 = shape.points[0];
  }

  Vector2 delta = Vector2Subtract(p0, p1);
  Vector2 delta_norm = Vector2Normalize(delta);
  F32 inner_distance = fmax(0.0f, Vector2Distance(p0, p1) - 2.0f*padding);
  F32 chunk_size = inner_distance / (F32)(p->out_count+1);
  F32 distance_from_point = padding + chunk_size*(F32)(wire_index+1);

  Vector2 out_position = Vector2Add(p1, Vector2Scale(delta_norm, distance_from_point));

  return out_position;
}


function Vector2
get_process_wire_in_position(Context *context, Process *p, Process_Shape shape, U32 wire_index) {
  F32 padding = context->camera.zoom * global_process_wire_padding;
  Vector2 p0 = shape.points[2];
  Vector2 p1 = shape.points[1];

  if (shape.kind == Process_Shape_HalfCircle) {
    // @Copypasta draw_processes
    p0 = shape.points[0];
    p1 = shape.points[shape.point_count-1];
  } else if (shape.point_count == 4) {
    p0 = shape.points[2];
    p1 = shape.points[3];
  }

  Vector2 delta = Vector2Subtract(p0, p1);
  Vector2 delta_norm = Vector2Normalize(delta);
  F32 inner_distance = fmax(0.0f, Vector2Distance(p0, p1) - 2.0f*padding);
  F32 chunk_size = inner_distance / (F32)(p->in_count+1);
  F32 distance_from_point = padding + chunk_size*(F32)(wire_index+1);

  Vector2 in_position = Vector2Add(p1, Vector2Scale(delta_norm, distance_from_point));

  return in_position;
}



function Rectangle get_wire_box(Context *context, Vector2 position) {
  F32 size = context->camera.zoom * global_box_size;
  F32 half_size = context->camera.zoom * global_box_half_size;
  Rectangle box = (Rectangle){position.x-half_size, position.y-half_size, size, size};
  return box;
}


function Vector2 get_new_wire_position(Context *context, Process *p, Process_Shape shape) {
  Vector2 position = shape.points[0];

  if (shape.kind == Process_Shape_Circle) {
    position.x = shape.center.x;
    position.y = shape.center.y - shape.radius;
  } else if (shape.kind == Process_Shape_HalfCircle) {
    if (shape.downward) {
      position.x = shape.points[shape.point_count-1].x;
      position.y = shape.points[shape.point_count-1].y;
    } else {
      Vector2 point = get_bezier_point(
        shape.points[0], shape.points[shape.point_count-1],
        shape.first_control, shape.second_control,
        0.5f);
      position.x = point.x;
      position.y = point.y;
    }
  }

  return position;
}


function Rectangle get_new_wire_box(Context *context, Process *p, Process_Shape shape) {
  // NOTE: Currently, the first point of any process-shape is always the corner where the new-wire-box wants to be.
  F32 x = shape.points[0].x;
  F32 y = shape.points[0].y;
  Vector2 position = get_new_wire_position(context, p, shape);
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



function B32 rectangle_contains_point(Rectangle r, Vector2 p) {
  F32 x2 = r.x + r.width;
  F32 y2 = r.y + r.height;
  B32 contains = (p.x >= r.x) && (p.y >= r.y) && (p.x <= x2) && (p.y <= y2);
  return contains;
}




function Process *get_process_wire_by_selection(Context *context, Process_Selection selection) {
  Process *wire = 0;
  S32 match_count = 0;

  for (Process *p = context->processes.first; p != 0; p = p->next) {
    if (Get_Flag(p->flags, Process_Flag_Wire)) {
      if (selection.type == Process_Selection_In && p->in_id == selection.process) {
        // matching in-wire
        if (match_count == selection.index) {
          wire = p;
          break;
        } else {
          match_count += 1;
        }
      } else if (selection.type == Process_Selection_Out && p->out_id == selection.process) {
        // matching out-wire
        if (match_count == selection.index) {
          wire = p;
          break;
        } else {
          match_count += 1;
        }
      }
    }
  }

  return wire;
}




function Process *create_process(Context *context) {
  Process *p = context->free_processes.first;

  if (p) {
    SLLQueuePop(context->free_processes.first, context->free_processes.last);
  } else {
    p = push_struct(context->process_arena, Process);
  }

  *p = (Process){0};

  if (p) {
    SLLQueuePush(context->processes.first, context->processes.last, p);
  }

  return p;
}




function void remove_process_from_processes(Context *context, Process *p) {
  if (context->processes.first == p) {
    SLLQueuePop(context->processes.first, context->processes.last);
  } else {
    for (Process *test_p = context->processes.first; test_p != 0; test_p = test_p->next) {
      if (test_p->next == p) {
        test_p->next = p->next;
        if (p == context->processes.last) {
          context->processes.last = test_p;
        }
        break;
      }
    }
  }
}

function void remove_process_from_active_processes(Context *context, Process *p) {
  if (context->active_process.first == p) {
    SLLQueuePop_NZ(context->active_process.first, context->active_process.last, next_active, 0);
  } else {
    for (Process *test_p = context->active_process.first; test_p != 0; test_p = test_p->next_active) {
      if (test_p->next_active == p) {
        test_p->next_active = p->next_active;
        if (p == context->active_process.last) {
          context->active_process.last = test_p;
        }
        break;
      }
    }
  }
}



function void delete_process(Context *context, Process *p) {
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
      B32 in_matched = 0;
      B32 out_matched = 0;

      for (Process *test_wire = context->processes.first; test_wire != 0; test_wire = test_wire->next) {
        // adjust in-connections that come after deleted wire
        if (test_wire->in_id == p->in_id) {
          if (test_wire->which_in > p->which_in) {
            test_wire->which_in -= 1;
          }
          in_matched = 1;
        }

        // adjust out-connections that come after deleted wire
        if (test_wire->out_id == p->out_id) {
          if (test_wire->which_out > p->which_out) {
            test_wire->which_out -= 1;
          }
          out_matched = 1;
        }
      }

      B32 only_in_conn = p->in_id != 0 && p->which_in == 0;
      B32 only_out_conn = p->out_id != 0 && p->which_out == 0;

      // decrement process' in-count
      if (in_matched || only_in_conn) {
        if (p->in_id) {
          p->in_id->in_count -= 1;
        }
      }

      // decrement process' out-count
      if (out_matched || only_out_conn) {
        if (p->out_id) {
          p->out_id->out_count -= 1;
        }
      }
    }

    // remove p from processes
    remove_process_from_processes(context, p);
    // add p to the free-list
    SLLQueuePush(context->free_processes.first, context->free_processes.last, p);

    /* clear_active_process(context); */

    // check for wires connected to the deleted process, and delete those also
    for (Process *wire = context->processes.first; wire != 0;) {
      B32 in_match = wire->in_id == p;
      B32 out_match = wire->out_id == p;
      B32 should_delete = 0;

      if (in_match || out_match) {
        if (!in_match) {
          // adjust in-connections to deleted wire
          for (Process *test_wire = context->processes.first; test_wire != 0; test_wire = test_wire->next) {
            if (test_wire->in_id == wire->in_id &&
                test_wire->which_in > wire->which_in) {
              test_wire->which_in -= 1;
            }
          }

          if (wire->in_id) {
            wire->in_id->in_count -= 1;
          }
        }

        if (!out_match) {
          // adjust out-connections to deleted wire
          for (Process *test_wire = context->processes.first; test_wire != 0; test_wire = test_wire->next) {
            if (test_wire->out_id == wire->out_id &&
                test_wire->which_out > wire->which_out) {
              test_wire->which_out -= 1;
            }
          }

          if (wire->out_id) {
            wire->out_id->out_count -= 1;
          }
        }

        should_delete = 1;
      }

      if (should_delete) {
        Process *next_process = wire->next;
        // remove wire from processes
        remove_process_from_processes(context, wire);
        // add wire to the free-list
        SLLQueuePush(context->free_processes.first,context->free_processes.last, wire);
        wire = next_process;
      } else {
        wire = wire->next;
      }
    }
  }
}



function void connect_processes(Context *context, Process *out, Process *in) {
  Process *new_wire = create_process(context);

  if (new_wire && out && in) {
    Set_Flag(new_wire->flags, Process_Flag_Wire);
    new_wire->out_id = out;
    new_wire->in_id = in;

    new_wire->which_out = out->out_count;
    new_wire->which_in = in->in_count;

    out->out_count += 1;
    in->in_count += 1;
  }
}



function void
fill_out_half_circle_shape(Context *context, Process_Shape *shape, Process *p, Vector2 position, B32 downward) {
  F32 padding = context->camera.zoom * global_process_wire_padding;
  F32 spacing = context->camera.zoom * global_process_wire_spacing;

  F32 conn_count = (F32)(downward ? p->out_count : p->in_count);
  F32 width = (2.0f*padding + conn_count*spacing);
  F32 half_width = 0.5f*(width);

  F32 height = context->camera.zoom * global_shape_size;
  F32 half_height = context->camera.zoom * global_shape_half_size;
  shape->kind = Process_Shape_HalfCircle;
  shape->triangle_count = global_shape_fan_triangle_count;
  shape->downward = downward;

  F32 multiplier = downward ? -1.0f : 1.0f;
  F32 x_offset = multiplier * half_width;
  F32 y_offset = multiplier * half_height;

  Vector2 first_point = (Vector2){position.x-x_offset, position.y+y_offset};
  Vector2 second_point = (Vector2){position.x+x_offset, position.y+y_offset};
  shape->first_control = (Vector2){first_point.x, first_point.y-2.0f*y_offset};
  shape->second_control = (Vector2){second_point.x, second_point.y-2.0f*y_offset};

  Vector2 middle_of_curve = get_bezier_point(
    first_point, second_point,
    shape->first_control, shape->second_control,
    0.5f);
  Vector2 middle_of_line = get_percentage_between_points(first_point, second_point, 0.5f);
  shape->center = get_percentage_between_points(middle_of_curve, middle_of_line, 0.5);

  shape->point_count = create_bezier_triangle_fan(
    first_point, second_point,
    shape->first_control, shape->second_control,
    shape->points, Process_Shape_Max_Points, shape->triangle_count);
}



function Process_Shape
get_process_shape(Context *context, Process *p) {
  Process_Shape shape = {0};

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
    shape.kind = Process_Shape_Rectangle;
    shape.point_count = 4;
    shape.points[0].x = position.x + half_width;
    shape.points[0].y = position.y - half_size;
    shape.points[1].x = position.x - half_width;
    shape.points[1].y = position.y - half_size;
    shape.points[2].x = position.x + half_width;
    shape.points[2].y = position.y + half_size;
    shape.points[3].x = position.x - half_width;
    shape.points[3].y = position.y + half_size;
    shape.center = get_percentage_between_points(shape.points[0], shape.points[3], 0.5f);
  } else if (has_in) {
    F32 width = (2.0f*padding + p->in_count*spacing);
    F32 half_width = 0.5f*(width);
    if (rounded) {
      // upward half-circle
      fill_out_half_circle_shape(context, &shape, p, position, 0);
    } else {
      // upward triangle
      shape.kind = Process_Shape_Triangle;
      shape.point_count = 3;
      shape.points[0].x = position.x;
      shape.points[0].y = position.y - quarter_size;
      shape.points[1].x = position.x - half_width;
      shape.points[1].y = position.y + half_size;
      shape.points[2].x = position.x + half_width;
      shape.points[2].y = position.y + half_size;
      Vector2 outer_mid = get_percentage_between_points(shape.points[1], shape.points[2], 0.5f);
      shape.center = get_percentage_between_points(shape.points[0], outer_mid, 0.66f);
    }
  } else if (has_out) {
    F32 width = (2.0f*padding + p->out_count*spacing);
    F32 half_width = 0.5f*(width);
    if (rounded) {
      // downward half-circle
      fill_out_half_circle_shape(context, &shape, p, position, 1);
    } else {
      // downward triangle
      shape.kind = Process_Shape_Triangle;
      shape.point_count = 3;
      shape.points[0].x = position.x + half_width;
      shape.points[0].y = position.y - half_size;
      shape.points[1].x = position.x - half_width;
      shape.points[1].y = position.y - half_size;
      shape.points[2].x = position.x;
      shape.points[2].y = position.y + quarter_size;
      Vector2 outer_mid = get_percentage_between_points(shape.points[0], shape.points[1], 0.5f);
      shape.center = get_percentage_between_points(shape.points[2], outer_mid, 0.66f);
    }
  } else {
    if (rounded) {
      // circle
      shape.kind = Process_Shape_Circle;
      shape.center = position;
      shape.radius = half_size*0.7f;
    } else {
      // diamond
      shape.kind = Process_Shape_Quadrangle;
      shape.point_count = 4;
      shape.points[0].x = position.x;
      shape.points[0].y = position.y - half_size;
      shape.points[1].x = position.x - half_size;
      shape.points[1].y = position.y;
      shape.points[2].x = position.x + half_size;
      shape.points[2].y = position.y;
      shape.points[3].x = position.x;
      shape.points[3].y = position.y + half_size;
      shape.center = get_percentage_between_points(shape.points[0], shape.points[3], 0.5f);
    }
  }

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
process_shape_contains_point(Context *context, Process_Shape shape, Vector2 point) {
  B32 contains = 0;

  switch(shape.kind) {
  case Process_Shape_Triangle:
  case Process_Shape_Quadrangle:
  case Process_Shape_Rectangle: {
    if (shape.point_count == 3 || shape.point_count == 4) {
      F32 side1 = which_side_of_line(shape.points[0], shape.points[1], point);
      F32 side2 = which_side_of_line(shape.points[1], shape.points[2], point);
      F32 side3 = which_side_of_line(shape.points[2], shape.points[0], point);

      F32 threshold = 0.1f;

      // test first triangle
      if (side1 < threshold && side2 < threshold && side3 < threshold) {
        contains = 1;
      } else if (shape.point_count == 4) {
        F32 side4 = which_side_of_line(shape.points[2], shape.points[3], point);
        F32 side5 = which_side_of_line(shape.points[3], shape.points[1], point);

        // test second triangle
        if (side2 > threshold && side4 > threshold && side5 > threshold) {
          contains = 1;
        }
      }
    }
  } break;
  case Process_Shape_Circle: {
    F32 distance = Vector2Distance(shape.center, point);
    contains = distance <= shape.radius;
  } break;
  case Process_Shape_HalfCircle: {
    contains = triangle_fan_contains_point(shape.points, shape.triangle_count, point);
  } break;
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
      Vector2 in_position = get_process_wire_in_position(context, p, shape, i);
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
        Vector2 out_position = get_process_wire_out_position(context, p, shape, i);
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




function Keybind_Result get_keybind(Context *context, Ui_Feature feature, Process_Selection selection) {
  Keybind keybind = global_keybind_lookup[feature];
  Ui_State *ui_state = &context->ui_state;
  Keybind_Result result = 0;

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

  return result;
}


function void handle_user_input(Context *context) {
  context->mouse_position = GetMousePosition();
  Ui_State *ui_state = &context->ui_state;
  Process_Selection selection = (Process_Selection){0};

  ui_state->mouse0_pressed = IsMouseButtonPressed(0);
  ui_state->mouse1_pressed = IsMouseButtonPressed(1);
  ui_state->mouse0_down = IsMouseButtonDown(0);
  ui_state->mouse1_down = IsMouseButtonDown(1);
  ui_state->hot_id_assigned = 0;
  ui_state->mouse_wheel_movement = GetMouseWheelMoveV();
  ui_state->control_down = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_LEFT_CONTROL);
  ui_state->shift_down = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT);
  ui_state->alt_down = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_LEFT_ALT);

  // initial bounding handling
  if (Get_Flag(context->flags, Context_Flag_Bounding)) {
    if (get_keybind(context, Ui_Feature_Bound, selection) == Keybind_Result_Exit) {
      Unset_Flag(context->flags, Context_Flag_Bounding);
    } else {
      clear_active_process(context);
    }
  }

  // panning
  {
    if (get_keybind(context, Ui_Feature_Pan, selection) == Keybind_Result_Enter) {
      Set_Flag(context->flags, Context_Flag_Panning);
      context->active_position = context->mouse_position;
    }

    if (Get_Flag(context->flags, Context_Flag_Panning)) {
      if (get_keybind(context, Ui_Feature_Pan, selection) == Keybind_Result_Exit) {
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
    B32 zoom_in = get_keybind(context, Ui_Feature_ZoomIn, selection) == Keybind_Result_Enter;
    B32 zoom_out = get_keybind(context, Ui_Feature_ZoomOut, selection) == Keybind_Result_Enter;

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

    if (get_keybind(context, Ui_Feature_SelectSingleProcess, selection) == Keybind_Result_Enter) {
      if (selection.type == Process_Selection_In || selection.type == Process_Selection_Out) {
        // select wire
        Process *wire = get_process_wire_by_selection(context, selection);
        B32 is_active_wire = is_active_process(context, wire);

        if (wire) {
          if (!is_active_wire) {
            clear_active_process(context);
            SLLQueuePush_NZ(context->active_process.first, context->active_process.last, wire, next_active, 0);
          }
        }
      } else if ((is_active || context->hot_process == p) &&
                 selection.type == Process_Selection_NewWire) {
        // begin new-wire
        Set_Flag(context->flags, Context_Flag_NewWire);
        if (!is_active) {
          clear_active_process(context);
          SLLQueuePush_NZ(context->active_process.first, context->active_process.last, p, next_active, 0);
        }
      } else if (selection.type == Process_Selection_Process) {
        if (Get_Flag(context->flags, Context_Flag_NewWire)) {
          // connect processes
          connect_processes(context, context->active_process.first, p);
        } else {
          // select process
          context->hot_process = p;
          if (!is_active) {
            clear_active_process(context);
            SLLQueuePush_NZ(context->active_process.first, context->active_process.last, p, next_active, 0);
          }
          Unset_Flag(context->flags, (Context_Flag_NewWire | Context_Flag_EditText));
          Set_Flag(context->flags, Context_Flag_Dragging);
          context->active_position = context->mouse_position;
        }
      }
    } else if (get_keybind(context, Ui_Feature_SelectAnotherProcess, selection) == Keybind_Result_Enter) {
      // select another process
      if (selection.type == Process_Selection_In || selection.type == Process_Selection_Out) {
        Process *wire = get_process_wire_by_selection(context, selection);
        if (wire) {
          SLLQueuePush_NZ(context->active_process.first, context->active_process.last, wire, next_active, 0);
        }
      } else if (selection.type == Process_Selection_Process) {
        SLLQueuePush_NZ(context->active_process.first, context->active_process.last, selection.process, next_active, 0);
      }
    } else if (selection.type == Process_Selection_Process) {
      // process hover
      context->hot_process = p;
    }

    // bounding
    if (Get_Flag(context->flags, Context_Flag_Bounding)) {
      Rectangle selection_rectangle = get_selection_rectangle(context);

      if (Get_Flag(p->flags, Process_Flag_Wire)) {
        Process_Shape out_shape = get_process_shape(context, p->out_id);
        Process_Shape in_shape = get_process_shape(context, p->in_id);
        Vector2 out_position = get_process_wire_out_position(context, p->out_id, out_shape, p->which_out);
        Vector2 in_position = get_process_wire_in_position(context, p->in_id, in_shape, p->which_in);

        if (rectangle_contains_point(selection_rectangle, out_position) ||
            rectangle_contains_point(selection_rectangle, in_position)) {
          SLLQueuePush_NZ(context->active_process.first, context->active_process.last, p, next_active, 0);
        }
      } else {
        Process_Shape shape = get_process_shape(context, p);

        if (rectangle_contains_point(selection_rectangle, shape.center)) {
          SLLQueuePush_NZ(context->active_process.first, context->active_process.last, p, next_active, 0);
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

  // handle active-process
  if (context->active_process.first) {
    B32 is_dragging = Get_Flag(context->flags, Context_Flag_Dragging);
    B32 should_stop_dragging = get_keybind(context, Ui_Feature_SelectSingleProcess, selection) == Keybind_Result_Exit;
    if (is_dragging && should_stop_dragging) {
      // stop dragging
      for (Process *a = context->active_process.first; a != 0; a = a->next_active) {
        Vector2 new_position = get_process_position(context, a);
        a->position = new_position;
      }
      Unset_Flag(context->flags, Context_Flag_Dragging);
    } else if (Get_Flag(context->flags, Context_Flag_EditText)) {
      // process label editing
      U32 key = 0;
      B32 shift_down = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
      while ((key = GetKeyPressed())) {
        for (Process *a = context->active_process.first; a != 0; a = a->next_active) {
          B32 is_ascii = key > 0 && key < 256;
          U8 c = ascii_char_lookup[key&0xff][shift_down];
          if (is_ascii && c != 0 && a->label_cursor < Process_Label_Size-1) {
            a->label[a->label_cursor] = c;
            a->label_cursor += 1;
          } else if (key == KEY_BACKSPACE && a->label_cursor > 0) {
            a->label_cursor -= 1;
            a->label[a->label_cursor] = 0;
          }
        }
      }
    } else if (get_keybind(context, Ui_Feature_BeginEditText, selection)) {
      // begin process label editing
      Set_Flag(context->flags, Context_Flag_EditText);
    } else if (get_keybind(context, Ui_Feature_CycleProcessDisplay, selection)) {
      // cycle through special process types (cups/caps/empty)
      for (Process *a = context->active_process.first; a != 0; a = a->next_active) {
        if (!Get_Flag(a->flags, Process_Flag_Wire)) {
          if ((a->in_count == 0 && a->out_count == 0) ||
              (a->in_count == 1 && a->out_count == 0) ||
              (a->in_count == 0 && a->out_count == 1)) {
            Toggle_Flag(a->flags, Process_Flag_Empty);
          } else if (a->in_count == 0 && a->out_count == 2) {
            Toggle_Flag(a->flags, Process_Flag_Cup);
          } else if (a->in_count == 2 && a->out_count == 0) {
            Toggle_Flag(a->flags, Process_Flag_Cap);
          }
        }
      }
    } else if (get_keybind(context, Ui_Feature_DeleteProcess, selection)) {
      // delete processes
      for (Process *a = context->active_process.first; a != 0;) {
        Process *next_active = a->next_active;
        delete_process(context, a);
        a = next_active;
      }
      clear_active_process(context);
    }
  }

  // more rectangle selection handling
  if (Get_Flag(context->flags, Context_Flag_Bounding)) {
    // add hot process to active processes
    if (context->hot_process) {
      B32 hot_is_active = is_active_process(context, context->hot_process);
      if (!hot_is_active) {
        SLLQueuePush_NZ(context->active_process.first, context->active_process.last, context->hot_process, next_active, 0);
      }
    }
  }

  // create process
  if (get_keybind(context, Ui_Feature_CreateProcess, selection)) {
    Process *new_p = create_process(context);
    if (new_p) {
      new_p->position = GetScreenToWorld2D(context->mouse_position, context->camera);
      clear_active_process(context);
      SLLQueuePush_NZ(context->active_process.first, context->active_process.last, new_p, next_active, 0);
    }
  }

  // cancel selection
  if (get_keybind(context, Ui_Feature_CancelSelection, selection)) {
    clear_active_process(context);
    Unset_Flag(context->flags, (Context_Flag_NewWire|Context_Flag_EditText));
  }

  // enter bounding
  if (get_keybind(context, Ui_Feature_Bound, selection) == Keybind_Result_Enter) {
    Set_Flag(context->flags, Context_Flag_Bounding);
    context->active_position = context->mouse_position;
  }

  // top-level actions
  // TODO: We don't have to check if we are editing text once we are always editing text for selected processes.
  if (!Get_Flag(context->flags, Context_Flag_EditText)) {
    if (get_keybind(context, Ui_Feature_ToggleDisplayMode, selection) == Keybind_Result_Enter) { 
      // toggle between rounded and triangular shapes
      Toggle_Flag(context->flags, Context_Flag_RoundedShapes);
    }
  }
}






function void draw_processes(Context *context) {
  Arena *ra = context->render_arena;

  Color bg_color = (Color){255, 255, 255, 255};
  Color stroke_color = (Color){0, 0, 0, 255};
  Color text_color = (Color){0, 0, 0, 255};
  Color box_color = (Color){10, 190, 40, 255};
  Color box_hover_color = (Color){5, 250, 20, 255};

  F32 padding = global_process_wire_padding;
  F32 spacing = global_process_wire_spacing;

  // draw processes
  for (Process *p = context->processes.first; p != 0; p = p->next) {
    B32 is_wire = Get_Flag(p->flags, Process_Flag_Wire);

    if (!is_wire) {
      Process_Shape shape = get_process_shape(context, p);

      B32 is_hot = context->hot_process == p;
      B32 is_active = is_active_process(context, p);
      F32 thickness = (is_hot||is_active) ? global_active_line_thickness : global_line_thickness;
      thickness *= context->camera.zoom;
      F32 cup_cap_control_offset = 10.0f;

      if (Get_Flag(p->flags, Process_Flag_Empty)) {
        // don't draw anything, allowing for dangling wire-ends
      } else if (Get_Flag(p->flags, Process_Flag_Cup)) {
        Vector2 pos0 = get_process_wire_out_position(context, p, shape, 0);
        Vector2 pos1 = get_process_wire_out_position(context, p, shape, 1);
        Vector2 ctrl0 = (Vector2){pos0.x, pos0.y+cup_cap_control_offset};
        Vector2 ctrl1 = (Vector2){pos1.x, pos1.y+cup_cap_control_offset};
        render_DrawLineBezierCubic(ra, pos0, pos1, ctrl0, ctrl1, thickness, stroke_color);
      } else if (Get_Flag(p->flags, Process_Flag_Cap)) {
        Vector2 pos0 = get_process_wire_in_position(context, p, shape, 0);
        Vector2 pos1 = get_process_wire_in_position(context, p, shape, 1);
        Vector2 ctrl0 = (Vector2){pos0.x, pos0.y-cup_cap_control_offset};
        Vector2 ctrl1 = (Vector2){pos1.x, pos1.y-cup_cap_control_offset};
        render_DrawLineBezierCubic(ra, pos0, pos1, ctrl0, ctrl1, thickness, stroke_color);
      } else {
        switch(shape.kind) {
        case Process_Shape_Triangle:
        case Process_Shape_Quadrangle:
        case Process_Shape_Rectangle: {
          // draw process background
          render_DrawTriangleStrip(ra, shape.points, shape.point_count, bg_color);

          // draw process lines
          Vector2 p0 = shape.points[0];
          Vector2 p1 = shape.points[1];
          Vector2 p2 = shape.points[2];
          Vector2 p3 = shape.points[3];
          if (shape.point_count == 3) {
            render_DrawLine(ra, p0.x, p0.y, p1.x, p1.y, thickness, stroke_color);
            render_DrawLine(ra, p1.x, p1.y, p2.x, p2.y, thickness, stroke_color);
            render_DrawLine(ra, p2.x, p2.y, p0.x, p0.y, thickness, stroke_color);
          } else if (shape.point_count == 4) {
            render_DrawLine(ra, p0.x, p0.y, p1.x, p1.y, thickness, stroke_color);
            render_DrawLine(ra, p1.x, p1.y, p3.x, p3.y, thickness, stroke_color);
            render_DrawLine(ra, p3.x, p3.y, p2.x, p2.y, thickness, stroke_color);
            render_DrawLine(ra, p2.x, p2.y, p0.x, p0.y, thickness, stroke_color);
          }
        } break;
        case Process_Shape_Circle: {
          render_DrawCircle(ra, shape.center, shape.radius, bg_color);
          F32 fudge = Half_Circle_Fudge*shape.radius;
          Vector2 first_point = (Vector2){shape.center.x-shape.radius, shape.center.y};
          Vector2 second_point = (Vector2){shape.center.x+shape.radius, shape.center.y};
          Vector2 control0 = (Vector2){first_point.x, first_point.y-fudge};
          Vector2 control1 = (Vector2){second_point.x, second_point.y-fudge};
          render_DrawLineBezierCubic(ra, first_point, second_point, control0, control1, thickness, stroke_color);
          Vector2 control2 = (Vector2){first_point.x, first_point.y+fudge};
          Vector2 control3 = (Vector2){second_point.x, second_point.y+fudge};
          render_DrawLineBezierCubic(ra, first_point, second_point, control2, control3, thickness, stroke_color);
        } break;
        case Process_Shape_HalfCircle: {
          // draw half-circle background
          render_DrawTriangleFan(ra, shape.points, shape.point_count, bg_color);
          // draw half-circle lines
          for (S32 i = 0; i < shape.point_count-1; ++i) {
            Vector2 p0 = shape.points[i];
            Vector2 p1 = shape.points[i+1];
            render_DrawLine(ra, p0.x, p0.y, p1.x, p1.y, thickness, stroke_color);
          }
          // connect the line endpoints
          render_DrawLine(ra,
                          shape.points[0].x,
                          shape.points[0].y,
                          shape.points[shape.point_count-1].x,
                          shape.points[shape.point_count-1].y,
                          thickness, stroke_color);
        } break;
        }
      }

      // draw label
      if (p->label[0]) {
        const char *text = (char *)p->label;
        F32 font_size = context->camera.zoom * global_process_font_size;
        F32 text_width = (F32)MeasureText(text, font_size);
        F32 text_x = shape.center.x-0.5f*text_width;
        F32 text_y = shape.center.y-0.5f*font_size;
        if (shape.kind == Process_Shape_HalfCircle) {
          F32 flip = shape.downward ? -1.0f : 1.0f;
          F32 fudge = 0.9f;
          F32 offset = fudge * flip * (0.5f * shape.radius);
          text_y -= offset;
        }
        render_DrawText(ra, text, text_x, text_y, font_size, text_color, 0);
      }

      // draw new-wire-box
      if (is_active || is_hot) {
        Rectangle new_wire_box = get_new_wire_box(context, p, shape);
        B32 new_wire_box_is_active = (
          (is_active && Get_Flag(context->flags, Context_Flag_NewWire)) ||
          rectangle_contains_point(new_wire_box, context->mouse_position));
        Color color = new_wire_box_is_active ? box_hover_color : box_color;
        render_DrawRectangleRec(ra, new_wire_box, color);
      }
    }
  }

  // draw wires
  for (Process *p = context->processes.first; p != 0; p = p->next) {
    B32 is_wire = Get_Flag(p->flags, Process_Flag_Wire);

    if (is_wire) {
      Process_Shape out_shape = get_process_shape(context, p->out_id);
      Process_Shape in_shape = get_process_shape(context, p->in_id);

      Vector2 out_position = get_process_wire_out_position(context, p->out_id, out_shape, p->which_out);
      Vector2 in_position = get_process_wire_in_position(context, p->in_id, in_shape, p->which_in);

      Vector2 out_control = out_position;
      out_control.y -= context->camera.zoom * 30.0f;
      Vector2 in_control = in_position;
      in_control.y += context->camera.zoom * 30.0f;

      B32 is_active = is_active_process(context, p) || context->hot_process == p;
      B32 connected_in_active = (is_active_process(context, p->in_id) ||
                                 context->hot_process == p->in_id);
      B32 connected_out_active = (is_active_process(context, p->out_id) ||
                                  context->hot_process == p->out_id);
      F32 thickness = is_active ? global_active_line_thickness : global_line_thickness;
      thickness *= context->camera.zoom;

      // draw wire
      render_DrawLineBezierCubic(ra, out_position, in_position, out_control, in_control, thickness, stroke_color);

      // draw out wire-box
      if (connected_out_active || is_active) {
        Rectangle box = get_wire_box(context, out_position);
        Color c = is_active ? box_hover_color : box_color;
        render_DrawRectangleRec(ra, box, c);
      }

      // draw in wire-box
      if (connected_in_active || is_active) {
        Rectangle box = get_wire_box(context, in_position);
        Color c = is_active ? box_hover_color : box_color;
        render_DrawRectangleRec(ra, box, c);
      }
    }
  }

  // draw new wire
  if (Get_Flag(context->flags, Context_Flag_NewWire) && context->active_process.first) {
    Process_Shape shape = get_process_shape(context, context->active_process.first);
    Vector2 position = get_new_wire_position(context, context->active_process.first, shape);

    Vector2 from_control = position;
    from_control.y -= context->camera.zoom * 30.f;
    Vector2 to_control = context->mouse_position;
    to_control.y += context->camera.zoom * 30.0f;

    F32 thickness = context->camera.zoom * global_line_thickness;

    render_DrawLineBezierCubic(ra, position, context->mouse_position, from_control, to_control, thickness, stroke_color);
  }

  // draw selection rectangle
  if (Get_Flag(context->flags, Context_Flag_Bounding)) {
    Rectangle selection_rect = get_selection_rectangle(context);
    Color selection_color = (Color){10, 30, 200, 50};

    render_DrawRectangleRec(ra, selection_rect, selection_color);
  }
}





function void draw_info_panel(Context *context) {
  Arena *ra = context->render_arena;
  Color text_color = (Color){0, 0, 0, 255};
  F32 y = 5.0f;
  F32 padding = 2.0f;

  if (context->active_process.first) {
    for (Process *a = context->active_process.first; a != 0; a = a->next_active) {
      char *format = a == context->active_process.first ? "active-id = %p" : "            %p";
      const char *text = TextFormat(format, a);
      render_DrawText(ra, text, 5.0f, y, global_panel_font_size, text_color, 1);
      y += global_panel_font_size + padding;
    }
  }
}





function Context initialize_context(void) {
  Context context = (Context){0};

  context.render_arena = arena_alloc_reserve(Megabytes(1), 0);
  context.process_arena = arena_alloc_reserve(Megabytes(1), 0);
  context.temp_arena = arena_alloc_reserve(Megabytes(1), 0);

  context.process_zero_pos = context.process_arena->chunk_pos;
  context.render_zero_pos = context.render_arena->chunk_pos;
  context.temp_zero_pos = context.temp_arena->chunk_pos;

  context.camera.zoom = 1.0f;

  return context;
}



function void initialize_globals(void) {
  S32 monitor_id = GetCurrentMonitor();
  S32 screen_width = GetMonitorWidth(monitor_id);
  S32 screen_height = GetMonitorHeight(monitor_id);

  global_background_color = (Color){220, 220, 200, 255};

  global_window_width = 0.7f*(F32)screen_width;
  global_window_height = 0.7f*(F32)screen_height;

  global_shape_size = global_window_width / 20.0f;
  global_shape_half_size = 0.5f*global_shape_size;

  global_box_size = global_shape_size*0.22f;
  global_box_half_size = 0.5f*global_box_size;

  global_process_wire_padding = 0.2f*global_shape_size;
  global_process_wire_spacing = 0.55f*global_shape_size;

  global_process_font_size = 0.4f*global_shape_size;
  global_panel_font_size = 0.35f*global_shape_size;

  global_line_thickness = 0.05f*global_shape_size;
  global_active_line_thickness = 0.1f*global_shape_size;

#define X(feature, key_kind, modifiers, constraint, description)\
  global_keybind_lookup[Ui_Feature_##feature] = (Keybind){Ui_Feature_##feature, key_kind, modifiers, constraint, description};
  Keybind_Xlist;
#undef X
}





int main(void) {
  InitWindow(800, 500, "proc");
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);

  initialize_globals();
  SetWindowSize(global_window_width, global_window_height);

  Context context = initialize_context();
  Arena *ra = context.render_arena;
  Arena *ta = context.temp_arena;
  render_Initialize(ta);

  while (!WindowShouldClose()) {
    handle_user_input(&context);

    render_ClearBackground(ra, global_background_color);
    draw_processes(&context);
    draw_info_panel(&context);

    BeginDrawing();
    render_Commands(&context, ra);
    arena_pop_to(ra, context.render_zero_pos);
    arena_pop_to(ta, context.temp_zero_pos);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
