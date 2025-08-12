#include "../libraries/mr4th/src/mr4th_base.h"
#define function static
#define global_variable static
#define Kilobytes(n) (1024 * (n))
#define Megabytes(n) (1024 * Kilobytes(n))
#define Gigabytes(n) (1024 * Megabytes(n))

#define ryn_memory_(identifier) identifier
#include "../libraries/ryn_memory.h"

#include "../libraries/raylib.h"

#include "../source/render.h"



#define Process_Id U32

typedef struct {
  Vector2 position;
  Vector2 size;
  Process_Id next_id;
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



typedef struct {
  arena render_arena;
  arena process_arena;

  Process_Id first_free_process_id;

  Process_Id hot_id;
  Process_Id active_id;
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


function void handle_user_input(Context *context) {
  arena *pa = &context->process_arena;
  Vector2 mouse_position = GetMousePosition();
  S32 pc = Get_Process_Count(pa);

  context->hot_id = 0;

  if (!context->hot_id) {
    for (S32 i = 1; i <= pc; ++i) {
      Process *p = Get_Process_By_Id(pa, i);
      Rectangle r = (Rectangle){p->position.x, p->position.y,
                                p->size.x, p->size.y};
      B32 contains = rectangle_contains_point(r, mouse_position);
      if (contains) {
        context->hot_id = i;
      }
    }
  }
}


function void draw_processes(Context *context) {
  arena *pa = &context->process_arena;
  arena *ra = &context->render_arena;
  S32 pc = Get_Process_Count(pa);

  for (S32 i = 1; i <= pc; ++i) {
    Process *p = Get_Process_By_Id(pa, i);
    B32 is_hot = context->hot_id == i;
    F32 thickness = is_hot ? 3.0f : 2.0f;
    Color bg_color = (Color){255, 255, 255, 255};
    Color stroke_color = (Color){0, 0, 0, 255};
    Rectangle rect = (Rectangle){p->position.x, p->position.y, p->size.x, p->size.y};

    render_DrawRectangle(ra, p->position.x, p->position.y, p->size.x, p->size.y, bg_color);
    render_DrawRectangleLinesEx(ra, rect, thickness, stroke_color);
  }
}


function void debug_setup_processes(Context *context) {
  Process *p1 = create_process(context);
  Process *p2 = create_process(context);

  if (p1 && p2) {
    p1->position.x = 50;
    p1->position.y = 70;
    p1->size.x = 20;
    p1->size.y = 50;

    p2->position.x = 250;
    p2->position.y = 170;
    p2->size.x = 80;
    p2->size.y = 80;
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

    BeginDrawing();
    render_Commands(ra);
    context.render_arena.Offset = 0;
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
