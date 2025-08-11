#include "../libraries/mr4th/src/mr4th_base.h"
#define function static
#define Kilobytes(n) (1024 * (n))
#define Megabytes(n) (1024 * Kilobytes(n))
#define Gigabytes(n) (1024 * Megabytes(n))

#define ryn_memory_(identifier) identifier
#include "../libraries/ryn_memory.h"

#include "../libraries/raylib.h"

#include "../source/render.h"







int main(void) {
  arena render_arena = CreateArena(Megabytes(1));
  arena *ra = &render_arena;

  InitWindow(600, 400, "proc");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    Color background_color = (Color){220, 220, 200, 255};
    Color stroke_color = (Color){10, 10, 16, 255};

    render_ClearBackground(ra, background_color);
    render_DrawRectangleLinesEx(ra, (Rectangle){200, 100, 40, 40}, 2.0f, stroke_color);

    render_Commands(ra);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
