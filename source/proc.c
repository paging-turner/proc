#include "../libraries/mr4th/src/mr4th_base.h"
#define function static
#define global_variable static
#define Kilobytes(n) (1024 * (n))
#define Megabytes(n) (1024 * Kilobytes(n))
#define Gigabytes(n) (1024 * Megabytes(n))

#define   Set_Flag(flags, flag) ((flags) |=  (flag))
#define Unset_Flag(flags, flag) ((flags) &= ~(flag))
#define   Get_Flag(flags, flag) ((flags) &   (flag))

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
} Process;

global_variable F32 global_process_wire_padding = 6.0f;
global_variable F32 global_process_wire_spacing = 12.0f;
#define Default_Box_Size 10.0f
global_variable F32 global_box_size = Default_Box_Size;
global_variable F32 global_box_half_size = 0.5f*Default_Box_Size;

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
  Context_Flag_New_Wire  = 1 << 1,
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

  // process click selection
  for (S32 i = 1; i <= pc; ++i) {
    Process *p = Get_Process_By_Id(pa, i);
    Vector2 size = get_process_size(context, p);
    Rectangle r = (Rectangle){p->position.x, p->position.y, size.x, size.y};
    B32 contains = rectangle_contains_point(r, context->mouse_position);
    Rectangle new_wire_box = get_new_wire_box(context, p);
    B32 contains_box = rectangle_contains_point(new_wire_box, context->mouse_position);
    if (mouse_pressed) {
      if (context->active_id == i && contains_box) {
        // handle new-wire interaction
        Set_Flag(context->flags, Context_Flag_New_Wire);
        printf("new wire %f\n", GetTime());
      } else if (contains) {
        context->hot_id = i;
        context->active_id = i;
        Set_Flag(context->flags, Context_Flag_Dragging);
        context->active_position = context->mouse_position;
        process_clicked = 1;
      }
    }
  }

  // handle active-id
  if (context->active_id) {
    Process *p = Get_Process_By_Id(pa, context->active_id);
    B32 is_dragging = Get_Flag(context->flags, Context_Flag_Dragging);
    if (is_dragging && !mouse_down) {
      // update dragged position
      Vector2 new_position = get_process_position(context, p);
      p->position = new_position;
      // we are no longer dragging
      Unset_Flag(context->flags, Context_Flag_Dragging);
    }
  }

  // unselect active-id
  if (mouse_pressed && !process_clicked) {
    context->active_id = 0;
  }
}




function void draw_processes(Context *context) {
  arena *pa = &context->process_arena;
  arena *ra = &context->render_arena;
  S32 pc = Get_Process_Count(pa);

  Color bg_color = (Color){255, 255, 255, 255};
  Color stroke_color = (Color){0, 0, 0, 255};
  Color box_color = (Color){10, 190, 40, 255};
  Color new_wire_color = (Color){100, 190, 40, 255};

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

      if (is_active) {
        Rectangle new_wire_box = get_new_wire_box(context, p);
        Color color = Get_Flag(context->flags, Context_Flag_New_Wire) ? new_wire_color : box_color;
        render_DrawRectangleRec(ra, new_wire_box, color);
      }
    }
  }

  // draw wires
  for (S32 i = 1; i <= pc; ++i) {
    Process *p = Get_Process_By_Id(pa, i);
    B32 is_wire = Get_Flag(p->flags, Process_Flag_Wire);

    F32 padding = global_process_wire_padding;
    F32 spacing = global_process_wire_spacing;

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
        } else if (p->to_id == context->active_id) {
          box = get_wire_box(context, to_position);
        }
        if (box.width) {
          render_DrawRectangleRec(ra, box, box_color);
        }
      }
    }
  }
}



function void draw_active_process(Context *context) {
  arena *pa = &context->process_arena;

  if (context->active_id) {
    Process *p = Get_Process_By_Id(pa, context->active_id);
  }
}




function void debug_setup_processes(Context *context) {
  arena *pa = &context->process_arena;
  Process *p1 = create_process(context);
  Process *p2 = create_process(context);
  Process *p3 = create_process(context);
  Process *p4 = create_process(context);

  if (p1 && p2) {
    p1->position.x = 50;
    p1->position.y = 70;

    p2->position.x = 250;
    p2->position.y = 170;

    p3->position.x = 350;
    p3->position.y = 170;

    p4->position.x = 150;
    p4->position.y = 70;

    connect_processes(context, p1, p2);
    connect_processes(context, p3, p2);

    connect_processes(context, p2, p4);
  }
}



int main(void) {
  Context context = (Context){};

  context.render_arena = CreateArena(Megabytes(1));
  context.process_arena = CreateArena(Megabytes(1));
  arena *ra = &context.render_arena;
  arena *pa = &context.process_arena;
  create_process(&context); // NOTE: unused first process

  InitWindow(600, 400, "proc");
  SetTargetFPS(60);

  debug_setup_processes(&context);

  while (!WindowShouldClose()) {
    Color background_color = (Color){220, 220, 200, 255};
    Color stroke_color = (Color){10, 10, 16, 255};

    handle_user_input(&context);

    render_ClearBackground(ra, background_color);
    draw_processes(&context);
    draw_active_process(&context);

    BeginDrawing();
    render_Commands(ra);
    context.render_arena.Offset = 0;
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
