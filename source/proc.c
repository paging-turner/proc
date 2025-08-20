#if !No_Assert
# define MR4TH_ASSERTS 1
#endif
#include "../libraries/mr4th/src/mr4th_base.h"

#include "../libraries/raylib.h"
#include "../libraries/raymath.h"
#include "../source/core.h"

#define ryn_memory_(identifier) identifier
#include "../libraries/ryn_memory.h"

#include "../source/render.h"



#define Process_Id U32

typedef enum {
  Process_Flag_Wire   = 1 << 0,
} Process_Flag;

typedef enum {
  Process_Selection__Null,
  Process_Selection_In,
  Process_Selection_Out,
  Process_Selection_NewWire,
  Process_Selection_Process,
} Process_Selection_Type;

typedef struct {
  Process_Id process_id;
  Process_Selection_Type type;
  S32 index;
  B32 hot_id_assigned;
} Process_Selection;

typedef struct {
  Vector2 position;

  B32 flags;

  S32 in_count;
  S32 out_count;

  Process_Id next_id;

  Process_Id in_id;
  Process_Id out_id;

  U32 which_in;
  U32 which_out;

  // TODO: Use a growable structure for strings.
#define Process_Label_Size 64
  U8 label[Process_Label_Size];
  U32 label_cursor;
} Process;

typedef struct {
  Vector2 outer_points[4];
  Vector2 inner_points[4];
  S32 point_count;
} Process_Shape;


global_variable F32 global_process_wire_padding = 6.0f;
global_variable F32 global_process_wire_spacing = 12.0f;
#define Default_Box_Size 10.0f
global_variable F32 global_box_size = Default_Box_Size;
global_variable F32 global_box_half_size = 0.5f*Default_Box_Size;
global_variable F32 global_process_font_size = 14.0f;
global_variable F32 global_panel_font_size = 14.0f;


global_variable Process global_zero_process;
#define Zero_Process()\
  ((global_zero_process=(Process){}),\
   &global_zero_process)

#define Get_Process_Count(pa)  (((pa)->Offset/sizeof(Process))-1)

#define Process_Id_Is_Valid(pa, id)\
  ((id) > 0 && (id) <= Get_Process_Count(pa))

#define Get_Process_By_Id(pa, id)\
  (Process_Id_Is_Valid((pa), (id))\
   ? (Process *)((pa)->Data + ((id)*sizeof(Process)))\
   : Zero_Process())

#define Get_Process_Id(pa, p)\
  (((U8 *)(p) > (pa)->Data)\
   ? (U32)(((U8 *)(p)-((pa)->Data))/sizeof(Process))\
   : 0)


// TODO: maybe this should be a mode and not flags?
typedef enum {
  Context_Flag_Dragging  = 1 << 0,
  Context_Flag_NewWire   = 1 << 1,
  Context_Flag_EditText  = 1 << 2,
} Context_Flag;

typedef struct {
  arena render_arena;
  arena process_arena;
  arena temp_arena;
  U32 flags;

  Process_Id first_free_process_id;
  Process_Id hot_id;
  Process_Id active_id;

  Vector2 mouse_position;
  Vector2 active_position;
} Context;



function Vector2 get_process_position(Context *context, Process *process) {
  arena *pa = &context->process_arena;

  U32 id = Get_Process_Id(pa, process);
  B32 is_active = context->active_id == id;
  B32 is_dragging = Get_Flag(context->flags, Context_Flag_Dragging);

  Vector2 position = process->position;
  if (is_active && is_dragging) {
    Vector2 delta = Vector2Subtract(context->mouse_position, context->active_position);
    position = Vector2Add(position, delta);
  }

  return position;
}


function Vector2 get_process_size(Context *context, Process *process) {
  F32 padding = global_process_wire_padding;
  F32 spacing = global_process_wire_spacing;
  S32 max_connection_count = Max(process->in_count, process->out_count);
  Vector2 size = (Vector2){(F32)max_connection_count*spacing + padding*2.0f,
                           50.0f};
  if (process->label[0]) {
    const char *text = (char *)process->label;
    F32 text_width = (F32)MeasureText(text, global_process_font_size);
    if (text_width > (size.x-2.0f*padding)) {
      size.x = text_width+2.0f*padding;
    }
  }

  return size;
}



function Vector2 get_process_wire_out_position(Context *context, Process *p, U32 wire_index) {
  F32 padding = global_process_wire_padding;
  F32 spacing = global_process_wire_spacing;

  Vector2 out_position = get_process_position(context, p);
  out_position.x += padding + 0.5f*spacing + spacing*wire_index;

  return out_position;
}


function Vector2 get_process_wire_in_position(Context *context, Process *p, U32 wire_index) {
  F32 padding = global_process_wire_padding;
  F32 spacing = global_process_wire_spacing;
  Vector2 in_size = get_process_size(context, p);

  Vector2 in_position = get_process_position(context, p);
  in_position.x += padding + 0.5f*spacing + spacing*wire_index;
  in_position.y += in_size.y;

  return in_position;
}



function Rectangle get_wire_box(Context *context, Vector2 position) {
  Rectangle box = (Rectangle){position.x-global_box_half_size,
                              position.y-global_box_half_size,
                              global_box_size, global_box_size};
  return box;
}


function Rectangle get_new_wire_box(Context *context, Process *p) {
  Vector2 position = get_process_position(context, p);
  Vector2 size = get_process_size(context, p);
  Rectangle new_wire_box =
    (Rectangle){position.x+size.x-global_box_half_size,
                position.y-global_box_half_size,
                global_box_size, global_box_size};
  return new_wire_box;
}




function B32 rectangle_contains_point(Rectangle r, Vector2 p) {
  F32 x2 = r.x + r.width;
  F32 y2 = r.y + r.height;
  B32 contains = (p.x >= r.x) && (p.y >= r.y) && (p.x <= x2) && (p.y <= y2);
  return contains;
}




function Process *get_process_wire_by_selection(Context *context, Process_Selection selection) {
  arena *pa = &context->process_arena;
  Process *wire = 0;
  S32 pc = Get_Process_Count(pa);
  S32 match_count = 0;

  for (S32 i = 1; i <= pc; ++i) {
    Process *p = Get_Process_By_Id(pa, i);
    if (Get_Flag(p->flags, Process_Flag_Wire)) {
      if (selection.type == Process_Selection_In && p->in_id == selection.process_id) {
        // matching in-wire
        if (match_count == selection.index) {
          wire = p;
          break;
        } else {
          match_count += 1;
        }
      } else if (selection.type == Process_Selection_Out && p->out_id == selection.process_id) {
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
  arena *pa = &context->process_arena;
  Process *p = ryn_memory_PushZeroStruct(pa, Process);
  return p;
}


function void connect_processes(Context *context, Process *out, Process *in) {
  arena *pa = &context->process_arena;
  Process *new_wire = create_process(context);

  if (new_wire) {
    U32 out_id = Get_Process_Id(pa, out);
    U32 in_id = Get_Process_Id(pa, in);

    Set_Flag(new_wire->flags, Process_Flag_Wire);
    new_wire->out_id = out_id;
    new_wire->in_id = in_id;

    new_wire->which_out = out->out_count;
    new_wire->which_in = in->in_count;

    out->out_count += 1;
    in->in_count += 1;
  }
}



function Process_Shape
get_process_shape(Context *context, Process *p) {
  Process_Shape shape = {0};

  Vector2 position = get_process_position(context, p);


  S32 has_in = p->in_count > 0;
  S32 has_out = p->out_count > 0;

  F32 temp_size = 40.0f;
  F32 temp_half_size = 40.0f;

  if (has_in && has_out) {
    // rectangular
    shape.point_count = 4;
    // outer
    shape.outer_points[0].x = position.x + temp_half_size;
    shape.outer_points[0].y = position.y - temp_half_size;
    shape.outer_points[1].x = position.x - temp_half_size;
    shape.outer_points[1].y = position.y - temp_half_size;
    shape.outer_points[2].x = position.x + temp_half_size;
    shape.outer_points[2].y = position.y + temp_half_size;
    shape.outer_points[3].x = position.x - temp_half_size;
    shape.outer_points[3].y = position.y + temp_half_size;
  } else if (has_in) {
    // upward triangle
    shape.point_count = 3;
    // outer
    shape.outer_points[0].x = position.x - temp_half_size;
    shape.outer_points[0].y = position.y + temp_half_size;
    shape.outer_points[1].x = position.x + temp_half_size;
    shape.outer_points[1].y = position.y + temp_half_size;
    shape.outer_points[2].x = position.x;
    shape.outer_points[2].y = position.y - temp_half_size;
  } else if (has_out) {
    // downward triangle
    shape.point_count = 3;
    // outer
    shape.outer_points[0].x = position.x + temp_half_size;
    shape.outer_points[0].y = position.y - temp_half_size;
    shape.outer_points[1].x = position.x - temp_half_size;
    shape.outer_points[1].y = position.y - temp_half_size;
    shape.outer_points[2].x = position.x;
    shape.outer_points[2].y = position.y + temp_half_size;
  } else {
    // diamond
    shape.point_count = 4;
    // outer
    shape.outer_points[0].x = position.x;
    shape.outer_points[0].y = position.y - temp_half_size;
    shape.outer_points[1].x = position.x - temp_half_size;
    shape.outer_points[1].y = position.y;
    shape.outer_points[2].x = position.x + temp_half_size;
    shape.outer_points[2].y = position.y;
    shape.outer_points[3].x = position.x;
    shape.outer_points[3].y = position.y + temp_half_size;
  }

  return shape;
}



function B32
process_shape_contains_point(Context *context, Process_Shape shape, Vector2 point) {
  B32 contains = 0;

  if (shape.point_count == 3 || shape.point_count == 4) {
    F32 side1 = which_side_of_line(shape.outer_points[0], shape.outer_points[1], point);
    F32 side2 = which_side_of_line(shape.outer_points[1], shape.outer_points[2], point);
    F32 side3 = which_side_of_line(shape.outer_points[2], shape.outer_points[0], point);

    if (side1 < 0.0f && side2 < 0.0f && side3 < 0.0f) {
      contains = 1;
    } else if (shape.point_count == 4) {
      F32 side4 = which_side_of_line(shape.outer_points[2], shape.outer_points[3], point);
      F32 side5 = which_side_of_line(shape.outer_points[3], shape.outer_points[1], point);

      if (side2 > 0.0f && side4 > 0.0f && side5 > 0.0f) {
        contains = 1;
      }
    }
  }

  return contains;
}



function Process_Selection
handle_process_selection(Context *context, Process *p) {
  arena *pa = &context->process_arena;
  Process_Selection selection = {0};
  selection.index = -1;
  selection.process_id = Get_Process_Id(pa, p);

  Vector2 size = get_process_size(context, p);
  Rectangle r = (Rectangle){p->position.x-global_box_half_size,
                            p->position.y-global_box_half_size,
                            size.x+global_box_size, size.y+global_box_size};
  Rectangle new_wire_box = get_new_wire_box(context, p);
  Vector2 mouse_p = context->mouse_position;

  if (rectangle_contains_point(new_wire_box, context->mouse_position)) {
    selection.type = Process_Selection_NewWire;
    context->hot_id = selection.process_id;
    selection.hot_id_assigned = 1;
  } else {
    // check in wire-boxes
    for (U32 i = 0; i < p->in_count; ++i) {
      Vector2 in_position = get_process_wire_in_position(context, p, i);
      Rectangle r = get_wire_box(context, in_position);
      if (rectangle_contains_point(r, mouse_p)) {
        selection.type = Process_Selection_In;
        selection.index = i;
        Process *wire = get_process_wire_by_selection(context, selection);
        Process_Id wire_id = Get_Process_Id(pa, wire);
        context->hot_id = wire_id;
        selection.hot_id_assigned = 1;
        break;
      }
    }

    if (selection.type == 0) {
      // check out wire-boxes
      for (U32 i = 0; i < p->out_count; ++i) {
        Vector2 out_position = get_process_wire_out_position(context, p, i);
        Rectangle r = get_wire_box(context, out_position);
        if (rectangle_contains_point(r, mouse_p)) {
          selection.type = Process_Selection_Out;
          selection.index = i;
          Process *wire = get_process_wire_by_selection(context, selection);
          Process_Id wire_id = Get_Process_Id(pa, wire);
          context->hot_id = wire_id;
          selection.hot_id_assigned = 1;
          break;
        }
      }
    }

    if (selection.type == 0) {
      Process_Shape shape = get_process_shape(context, p);
      B32 contains = process_shape_contains_point(context, shape, context->mouse_position);

      if (contains) {
        // process selection
        selection.type = Process_Selection_Process;
        context->hot_id = selection.process_id;
        selection.hot_id_assigned = 1;
      }
    }
  }

  return selection;
}




function void handle_user_input(Context *context) {
  arena *pa = &context->process_arena;
  S32 pc = Get_Process_Count(pa);

  context->mouse_position = GetMousePosition();
  B32 mouse_pressed = IsMouseButtonPressed(0);
  B32 mouse_down = IsMouseButtonDown(0);
  B32 process_clicked = 0;
  B32 hot_id_assigned = 0;

  // process interaction
  for (S32 i = 1; i <= pc; ++i) {
    Process *p = Get_Process_By_Id(pa, i);
    Process_Selection selection = handle_process_selection(context, p);
    hot_id_assigned = selection.hot_id_assigned || hot_id_assigned;

    if (mouse_pressed) {
      if (selection.type == Process_Selection_In || selection.type == Process_Selection_Out) {
        Process *wire = get_process_wire_by_selection(context, selection);
        if (wire) {
          Process_Id wire_id = Get_Process_Id(pa, wire);
          context->active_id = wire_id;
          if (selection.type == Process_Selection_In) {
            context->hot_id = wire->in_id;
          } else if (selection.type == Process_Selection_Out) {
            context->hot_id = wire->out_id;
          }
          process_clicked = 1;
        }
      } else if ((context->active_id == i || context->hot_id == i) &&
                 selection.type == Process_Selection_NewWire) {
        // begin new-wire
        Set_Flag(context->flags, Context_Flag_NewWire);
        context->active_id = i;
        process_clicked = 1;
      } else if (selection.type == Process_Selection_Process) {
        if (Get_Flag(context->flags, Context_Flag_NewWire)) {
          // connect processes
          Process *active_p = Get_Process_By_Id(pa, context->active_id);
          connect_processes(context, active_p, p);
        } else {
          // select process
          context->hot_id = i;
          context->active_id = i;
          U32 unset_flags = (Context_Flag_NewWire |
                             Context_Flag_EditText);
          Unset_Flag(context->flags, unset_flags);
          Set_Flag(context->flags, Context_Flag_Dragging);
          context->active_position = context->mouse_position;
          process_clicked = 1;
        }
      }
    } else if (selection.type == Process_Selection_Process) {
      // process hover
      context->hot_id = i;
    }

    // break if there has been an interaction
    if (selection.type > -1) {
      break;
    }
  }

  if (!hot_id_assigned) {
    context->hot_id = 0;
  }

  // handle active-id
  if (context->active_id) {
    Process *p = Get_Process_By_Id(pa, context->active_id);
    B32 is_dragging = Get_Flag(context->flags, Context_Flag_Dragging);
    if (is_dragging && !mouse_down) {
      // stop dragging
      Vector2 new_position = get_process_position(context, p);
      p->position = new_position;
      Unset_Flag(context->flags, Context_Flag_Dragging);
    } else if (Get_Flag(context->flags, Context_Flag_EditText)) {
      // process label editing
      U32 c = 0;
      B32 shift_down = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
      while ((c = GetKeyPressed())) {
        if (Is_Editable_Char(c) && p->label_cursor < Process_Label_Size-1) {
          B32 is_alpha = c >= 'A' && c <= 'Z';
          if (is_alpha && !shift_down) {
            c += 32;
          }
          p->label[p->label_cursor] = c&0xff;
          p->label_cursor += 1;
        } else if (c == KEY_BACKSPACE && p->label_cursor > 0) {
          p->label_cursor -= 1;
          p->label[p->label_cursor] = 0;
        }
      }
    } else if (IsKeyDown(KEY_I)) {
      // begin process label editing
      Set_Flag(context->flags, Context_Flag_EditText);
    }
  }

  // non-process clicks
  if (mouse_pressed && !process_clicked) {
    if (context->active_id) {
      // un-select process
      context->active_id = 0;
      U32 flags_to_unset = (Context_Flag_NewWire|
                            Context_Flag_EditText);
      Unset_Flag(context->flags, flags_to_unset);
    } else if (IsKeyDown(KEY_LEFT_CONTROL)) {
      // new process
      Process *new_p = create_process(context);
      if (new_p) {
        new_p->position = context->mouse_position;
      }
    }
  }
}



function void draw_processes(Context *context) {
  arena *pa = &context->process_arena;
  arena *ra = &context->render_arena;
  S32 pc = Get_Process_Count(pa);

  Color bg_color = (Color){255, 255, 255, 255};
  Color stroke_color = (Color){0, 0, 0, 255};
  Color text_color = (Color){0, 0, 0, 255};
  Color box_color = (Color){10, 190, 40, 255};
  Color box_hover_color = (Color){5, 250, 20, 255};

  F32 padding = global_process_wire_padding;
  F32 spacing = global_process_wire_spacing;

  // draw processes
  for (S32 i = 1; i <= pc; ++i) {
    Process *p = Get_Process_By_Id(pa, i);
    B32 is_wire = Get_Flag(p->flags, Process_Flag_Wire);

    if (!is_wire) {
      B32 is_hot = context->hot_id == i;
      B32 is_active = context->active_id == i;
      F32 thickness = (is_hot||is_active) ? 3.0f : 2.0f;

      Process_Shape shape = get_process_shape(context, p);
      render_DrawTriangleStrip(ra, shape.outer_points, shape.point_count, bg_color);

      Vector2 size = (Vector2){40.0f, 40.0f}; // TODO: This is just a temp size, it should be replaced with a shape-aware sizing. 

      if (p->label[0]) {
        const char *text = (char *)p->label;
        F32 text_width = (F32)MeasureText(text, global_process_font_size);
        F32 text_x = p->position.x+0.5f*(size.x-text_width);
        F32 text_y = p->position.y+0.5f*(size.y-global_process_font_size);
        render_DrawText(ra, text, text_x, text_y, global_process_font_size, text_color, 0);
      }

      if (is_active || is_hot) {
        Rectangle new_wire_box = get_new_wire_box(context, p);
        B32 new_wire_box_is_active = (Get_Flag(context->flags, Context_Flag_NewWire) ||
                                      rectangle_contains_point(new_wire_box, context->mouse_position));
        Color color = new_wire_box_is_active ? box_hover_color : box_color;
        render_DrawRectangleRec(ra, new_wire_box, color);
      }
    }
  }

  // draw wires
  for (S32 i = 1; i <= pc; ++i) {
    Process *p = Get_Process_By_Id(pa, i);
    B32 is_wire = Get_Flag(p->flags, Process_Flag_Wire);

    if (is_wire) {
      Process *out = Get_Process_By_Id(pa, p->out_id);
      Process *in = Get_Process_By_Id(pa, p->in_id);

      Vector2 out_position = get_process_wire_out_position(context, out, p->which_out);
      Vector2 in_position = get_process_wire_in_position(context, in, p->which_in);

      Vector2 out_control = out_position;
      out_control.y -= 30.0f;
      Vector2 in_control = in_position;
      in_control.y += 30.0f;

      B32 is_active = context->active_id == i || context->hot_id == i;
      B32 connected_process_active = (context->active_id == p->in_id ||
                                      context->hot_id == p->in_id ||
                                      context->active_id == p->out_id ||
                                      context->hot_id == p->out_id);
      F32 thickness = is_active ? 4.0f : 2.0f;

      render_DrawLineBezierCubic(ra, out_position, in_position, out_control, in_control, thickness, stroke_color);

      if (connected_process_active || is_active) {
        Rectangle box = get_wire_box(context, out_position);
        Color c = is_active ? box_hover_color : box_color;
        render_DrawRectangleRec(ra, box, c);

        box = get_wire_box(context, in_position);
        c = is_active ? box_hover_color : box_color;
        render_DrawRectangleRec(ra, box, c);
      }
    }
  }

  // draw new wire
  if (Get_Flag(context->flags, Context_Flag_NewWire) && context->active_id) {
    Process *p = Get_Process_By_Id(pa, context->active_id);
    Vector2 position = get_process_position(context, p);
    Vector2 size = get_process_size(context, p);
    position.x += size.x;

    Vector2 from_control = position;
    from_control.y -= 30.f;
    Vector2 to_control = context->mouse_position;
    to_control.y += 30.0f;

    render_DrawLineBezierCubic(ra, position, context->mouse_position, from_control, to_control, 2.0f, stroke_color);
  }
}





function void draw_info_panel(Context *context) {
  arena *ra = &context->render_arena;
  Color text_color = (Color){0, 0, 0, 255};

  if (context->active_id) {
    const char *text = TextFormat("active-id = %d", context->active_id);
    render_DrawText(ra, text, 5.0f, 5.0f, global_panel_font_size, text_color, 1);
  }
}






int main(void) {
  Context context = (Context){};
  context.render_arena = CreateArena(Megabytes(1));
  context.process_arena = CreateArena(Megabytes(1));
  context.temp_arena = CreateArena(Megabytes(1));
  create_process(&context); // NOTE: unused first process

  arena *ra = &context.render_arena;
  arena *ta = &context.temp_arena;
  render_Initialize(ta);

  InitWindow(600, 400, "proc");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    Color background_color = (Color){220, 220, 200, 255};
    Color stroke_color = (Color){10, 10, 16, 255};

    handle_user_input(&context);

    render_ClearBackground(ra, background_color);
    draw_processes(&context);
    draw_info_panel(&context);

    BeginDrawing();
    render_Commands(ra);
    context.render_arena.Offset = 0;
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
