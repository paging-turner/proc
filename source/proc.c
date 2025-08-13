#include "../libraries/mr4th/src/mr4th_base.h"
#define function static
#define global_variable static
#define Kilobytes(n) (1024 * (n))
#define Megabytes(n) (1024 * Kilobytes(n))
#define Gigabytes(n) (1024 * Megabytes(n))

#define   Set_Flag(flags, flag) ((flags) |=  (flag))
#define Unset_Flag(flags, flag) ((flags) &= ~(flag))
#define   Get_Flag(flags, flag) ((flags) &   (flag))

#include "../source/core.h"

#define ryn_memory_(identifier) identifier
#include "../libraries/ryn_memory.h"

#include "../libraries/raylib.h"
#include "../libraries/raymath.h"

#include "../source/render.h"



#define Process_Id U32

typedef enum {
  Process_Flag_Wire   = 1 << 0,
} Process_Flag;

typedef struct {
  Vector2 position;

  B32 flags;

  S32 from_count;
  S32 to_count;

  Process_Id next_id;

  Process_Id from_id;
  Process_Id to_id;

  U32 which_from;
  U32 which_to;

  // TODO: Use a growable structure for strings, like string-list!
#define Process_Label_Size 64
  U8 label[Process_Label_Size];
  U32 label_cursor;
} Process;

global_variable F32 global_process_wire_padding = 6.0f;
global_variable F32 global_process_wire_spacing = 12.0f;
#define Default_Box_Size 10.0f
global_variable F32 global_box_size = Default_Box_Size;
global_variable F32 global_box_half_size = 0.5f*Default_Box_Size;
global_variable F32 global_font_size = 14.0f;


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


// TODO: maybe this should be a mode and not flags??
typedef enum {
  Context_Flag_Dragging  = 1 << 0,
  Context_Flag_NewWire   = 1 << 1,
  Context_Flag_EditText  = 1 << 2,
} Context_Flag;

typedef struct {
  arena render_arena;
  arena process_arena;
  U32 flags;

  Process_Id first_free_process_id;
  Process_Id hot_id;
  Process_Id active_id;

  Vector2 mouse_position;
  Vector2 active_position;
} Context;



function B32 rectangle_contains_point(Rectangle r, Vector2 p) {
  F32 x2 = r.x + r.width;
  F32 y2 = r.y + r.height;
  B32 contains = (p.x >= r.x) && (p.y >= r.y) && (p.x <= x2) && (p.y <= y2);
  return contains;
}



function Process *create_process(Context *context) {
  arena *pa = &context->process_arena;
  Process *p = ryn_memory_PushZeroStruct(pa, Process);
  return p;
}



function void connect_processes(Context *context, Process *from, Process *to) {
  arena *pa = &context->process_arena;
  Process *new_wire = create_process(context);

  if (new_wire) {
    U32 from_id = Get_Process_Id(pa, from);
    U32 to_id = Get_Process_Id(pa, to);

    Set_Flag(new_wire->flags, Process_Flag_Wire);
    new_wire->from_id = from_id;
    new_wire->to_id = to_id;

    new_wire->which_from = from->to_count;
    new_wire->which_to = to->from_count;

    from->to_count += 1;
    to->from_count += 1;
  }
}



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
  S32 max_connection_count = Max(process->from_count, process->to_count);
  Vector2 size = (Vector2){(F32)max_connection_count*spacing + padding*2.0f,
                           50.0f};
  if (process->label[0]) {
    const char *text = (char *)process->label;
    F32 text_width = (F32)MeasureText(text, global_font_size);
    if (text_width > (size.x-2.0f*padding)) {
      size.x = text_width+2.0f*padding;
    }
  }

  return size;
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




function void handle_user_input(Context *context) {
  arena *pa = &context->process_arena;
  S32 pc = Get_Process_Count(pa);

  context->mouse_position = GetMousePosition();
  B32 mouse_pressed = IsMouseButtonPressed(0);
  B32 mouse_down = IsMouseButtonDown(0);
  B32 process_clicked = 0;

  context->hot_id = 0;

  // process interaction
  for (S32 i = 1; i <= pc; ++i) {
    Process *p = Get_Process_By_Id(pa, i);
    Vector2 size = get_process_size(context, p);
    Rectangle r = (Rectangle){p->position.x, p->position.y, size.x, size.y};
    B32 contains = rectangle_contains_point(r, context->mouse_position);
    Rectangle new_wire_box = get_new_wire_box(context, p);
    B32 contains_box = rectangle_contains_point(new_wire_box, context->mouse_position);
    if (mouse_pressed) {
      if (context->active_id == i && contains_box) {
        // begin new-wire
        Set_Flag(context->flags, Context_Flag_NewWire);
        process_clicked = 1;
      } else if (contains) {
        if (Get_Flag(context->flags, Context_Flag_NewWire)) {
          // connect processes
          Process *active_p = Get_Process_By_Id(pa, context->active_id);
          connect_processes(context, active_p, p);
        } else {
          // select process
          context->hot_id = i;
          context->active_id = i;
          Set_Flag(context->flags, Context_Flag_Dragging);
          context->active_position = context->mouse_position;
          process_clicked = 1;
        }
      }
    }
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
  Color new_wire_color = (Color){100, 190, 40, 255};

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

      Vector2 position = get_process_position(context, p);
      Vector2 size = get_process_size(context, p);
      Rectangle rect = (Rectangle){position.x, position.y, size.x, size.y};
      F32 line_offset = 1.0f;
      Rectangle line_rect = (Rectangle){position.x-line_offset, position.y-line_offset,
                                        size.x+2.0f*line_offset, size.y+2.0f*line_offset};

      render_DrawRectangle(ra, position.x, position.y, size.x, size.y, bg_color);
      render_DrawRectangleLinesEx(ra, line_rect, thickness, stroke_color);

      if (p->label[0]) {
        const char *text = (char *)p->label;
        F32 text_width = (F32)MeasureText(text, global_font_size);
        F32 text_x = position.x+0.5f*(size.x-text_width);
        F32 text_y = position.y+0.5f*(size.y-global_font_size);
        render_DrawText(ra, text, text_x, text_y, global_font_size, text_color, 0);
      }

      if (is_active) {
        Rectangle new_wire_box = get_new_wire_box(context, p);
        Color color = Get_Flag(context->flags, Context_Flag_NewWire) ? new_wire_color : box_color;
        render_DrawRectangleRec(ra, new_wire_box, color);
      }
    }
  }

  // draw wires
  for (S32 i = 1; i <= pc; ++i) {
    Process *p = Get_Process_By_Id(pa, i);
    B32 is_wire = Get_Flag(p->flags, Process_Flag_Wire);

    if (is_wire) {
      Process *from = Get_Process_By_Id(pa, p->from_id);
      Process *to = Get_Process_By_Id(pa, p->to_id);

      Vector2 from_size = get_process_size(context, from);
      Vector2 to_size = get_process_size(context, to);

      Vector2 from_position = get_process_position(context, from);
      from_position.x += padding + 0.5f*spacing + spacing*p->which_from;

      Vector2 to_position = get_process_position(context, to);
      to_position.x += padding + 0.5f*spacing + spacing*p->which_to;
      to_position.y += to_size.y;

      Vector2 from_control = from_position;
      from_control.y -= 30.0f;
      Vector2 to_control = to_position;
      to_control.y += 30.0f;

      render_DrawLineBezierCubic(ra, from_position, to_position, from_control, to_control, 2.0f, stroke_color);

      if (context->active_id) {
        Rectangle box = {0};
        if (p->from_id == context->active_id) {
          box = get_wire_box(context, from_position);
          render_DrawRectangleRec(ra, box, box_color);
        }
        if (p->to_id == context->active_id) {
          box = get_wire_box(context, to_position);
          render_DrawRectangleRec(ra, box, box_color);
        }
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









int main(void) {
  Context context = (Context){};
  context.render_arena = CreateArena(Megabytes(1));
  context.process_arena = CreateArena(Megabytes(1));
  create_process(&context); // NOTE: unused first process

  arena *ra = &context.render_arena;

  InitWindow(600, 400, "proc");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    Color background_color = (Color){220, 220, 200, 255};
    Color stroke_color = (Color){10, 10, 16, 255};

    handle_user_input(&context);

    render_ClearBackground(ra, background_color);
    draw_processes(&context);

    BeginDrawing();
    render_Commands(ra);
    context.render_arena.Offset = 0;
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
