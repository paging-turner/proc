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
  Vector2 size;

  B32 flags;

  Process_Id next_id;
  Process_Id from_id;
  Process_Id to_id;
} Process;

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


typedef enum {
  Context_Flag_Dragging  = 1 << 0,
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
    Rectangle r = (Rectangle){p->position.x, p->position.y,
                              p->size.x, p->size.y};
    B32 contains = rectangle_contains_point(r, context->mouse_position);
    if (contains) {
      context->hot_id = i;
      if (mouse_pressed) {
        context->active_id = i;
        Set_Flag(context->flags, Context_Flag_Dragging);
        context->active_position = context->mouse_position;
        process_clicked = 1;
      }
    }
  }

  // handle active-id
  if (context->active_id) {
    if (!mouse_down) {
      // update dragged position
      Process *p = Get_Process_By_Id(pa, context->active_id);
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

  for (S32 i = 1; i <= pc; ++i) {
    Process *p = Get_Process_By_Id(pa, i);
    B32 is_wire = Get_Flag(p->flags, Process_Flag_Wire);

    if (is_wire) {
      Process *from = Get_Process_By_Id(pa, p->from_id);
      Process *to = Get_Process_By_Id(pa, p->to_id);

      Vector2 from_position = get_process_position(context, from);
      Vector2 to_position = get_process_position(context, to);

      render_DrawLine(ra, from_position.x, from_position.y, to_position.x, to_position.y, 2.0f, stroke_color);
    } else {
      B32 is_hot = context->hot_id == i;
      B32 is_active = context->active_id == i;
      F32 thickness = (is_hot||is_active) ? 3.0f : 2.0f;

      Vector2 position = get_process_position(context, p);
      Rectangle rect = (Rectangle){position.x, position.y, p->size.x, p->size.y};

      render_DrawRectangle(ra, position.x, position.y, p->size.x, p->size.y, bg_color);
      render_DrawRectangleLinesEx(ra, rect, thickness, stroke_color);
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
  Process *w1 = create_process(context);

  U32 p1_id = Get_Process_Id(pa, p1);
  U32 p2_id = Get_Process_Id(pa, p2);

  if (p1 && p2 && w1) {
    p1->position.x = 50;
    p1->position.y = 70;
    p1->size.x = 20;
    p1->size.y = 50;

    p2->position.x = 250;
    p2->position.y = 170;
    p2->size.x = 80;
    p2->size.y = 80;

    Set_Flag(w1->flags, Process_Flag_Wire);
    w1->from_id = p1_id;
    w1->to_id = p2_id;
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
