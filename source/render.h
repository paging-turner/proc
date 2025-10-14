/*
    A wrapper for raylib rendering functions. Also, implements a command buffer, just in case we want to process the render-commands before actually drawing anything.
*/

typedef enum {
  render_command__Null,
  render_command_ClearBackground,
  render_command_DrawRectangleRec,
  render_command_DrawText,
  render_command_DrawRectangleLinesEx,
  render_command_DrawRectangle,
  render_command_DrawLine,
  render_command_DrawLineBezierCubic,
  render_command_DrawPoly,
  render_command_DrawPolyLinesEx,
  render_command_DrawTriangleStrip,
  render_command_DrawTriangleFan,
  render_command_DrawCircle,
  render_command_DrawCircleSector,
  render_command_DrawCircleLines,
  render_command_DrawCircleSectorLines,
} render_command_kind;


typedef struct render_command render_command;
struct render_command {
  render_command *next;
  render_command_kind Kind;

  Rectangle Rectangle;
  const char *Text;
  F32 X;
  F32 Y;
  F32 X2;
  F32 Y2;
  F32 ControlX;
  F32 ControlY;
  F32 ControlX2;
  F32 ControlY2;
  S32 FontSize;
  F32 Thickness;
  F32 Rotation;
  F32 Radius;
  S32 Sides;
  Color Color;
  F32 Width;
  F32 Height;
#define render_Max_Points 32
  Vector2 Points[render_Max_Points];
  S32 PointCount;
  F32 StartAngle;
  F32 EndAngle;
};

typedef struct Render_Command_List {
  render_command *first;
  render_command *last;
} Render_Command_List;

typedef struct Render_Context {
  Arena *arena;
  Render_Command_List command_list;
  B32 reverse_commands;
} Render_Context;

global_variable Arena *render_GlobalTempArena;


function void render_Initialize(Arena *TempArena) {
  render_GlobalTempArena = TempArena;
}

function render_command *create_render_command(Render_Context *rc) {
  render_command *command = push_struct(rc->arena, render_command);

  if (rc->reverse_commands) {
    SLLQueuePushFront(rc->command_list.first, rc->command_list.last, command);
  } else {
    SLLQueuePush(rc->command_list.first, rc->command_list.last, command);
  }

  return command;
}


function void render_ClearBackground(Render_Context *rc, Color C) {
  render_command *Command = create_render_command(rc);
  if (Command) {
    Command->Kind = render_command_ClearBackground;
    Command->Color = C;
  }
}

function void render_DrawRectangleRec(Render_Context *rc, Rectangle R, Color C) {
  render_command *Command = create_render_command(rc);

  if (Command) {
    Command->Kind = render_command_DrawRectangleRec;
    Command->Rectangle = R;
    Command->Color = C;
  }
}

function char *render_PushTempString(const char *CString) {
  Assert(render_GlobalTempArena);
  char *Result = 0;

  if (CString) {
    // get CString length
    U32 string_length = 0;
    for (const char *c = CString;; ++c) {
      string_length += 1;
      if (*c == 0) {
        break;
      }
    }

    Result = arena_push_no_zero(render_GlobalTempArena, string_length);

    // copy string
    for (U32 i = 0; i < string_length; ++i) {
      Result[i] = CString[i];
    }
  }

  return Result;
}

function void render_DrawText(Render_Context *rc, const char *Text, F32 X, F32 Y, S32 FontSize, Color C, B32 copy_string) {
  render_command *Command = create_render_command(rc);
  const char *RenderString;
  if (copy_string) {
    RenderString = render_PushTempString(Text);
  } else {
    RenderString = Text;
  }

  if (Command) {
    Command->Kind = render_command_DrawText;
    Command->Text = RenderString;
    Command->X = X;
    Command->Y = Y;
    Command->FontSize = FontSize;
    Command->Color = C;
  }
}

function void render_DrawRectangleLinesEx(Render_Context *rc, Rectangle R, F32 Thickness, Color C) {
  render_command *Command = create_render_command(rc);

  if (Command) {
    Command->Kind = render_command_DrawRectangleLinesEx;
    Command->Rectangle = R;
    Command->Thickness = Thickness;
    Command->Color = C;
  }
}

function void render_DrawRectangle(Render_Context *rc, F32 X, F32 Y, F32 W, F32 H, Color C) {
  render_command *Command = create_render_command(rc);

  if (Command) {
    Command->Kind = render_command_DrawRectangle;
    Command->X = X;
    Command->Y = Y;
    Command->Width = W;
    Command->Height = H;
    Command->Color = C;
  }
}

function void render_DrawLine(Render_Context *rc, int startPosX, int startPosY, int endPosX, int endPosY, F32 thickness, Color color) {
  render_command *Command = create_render_command(rc);

  // HACK: Fudge the line length to avoid gaps between adjoined lines.
  Vector2 start;
  Vector2 end; {
    Vector2 normal_delta = Vector2Normalize((Vector2){endPosX-startPosX, endPosY-startPosY});
    Vector2 offset = Vector2Scale(normal_delta, 0.4f*thickness);
    start.x = startPosX - offset.x;
    start.y = startPosY - offset.y;
    end.x = endPosX + offset.x;
    end.y = endPosY + offset.y;
  }

  if (Command) {
    Command->Kind = render_command_DrawLine;
    Command->X = start.x;
    Command->Y = start.y;
    Command->X2 = end.x;
    Command->Y2 = end.y;
    Command->Thickness = thickness;
    Command->Color = color;
  }
}


function void render_DrawLineBezierCubic(Render_Context *rc, Vector2 startPos, Vector2 endPos, Vector2 startControlPos, Vector2 endControlPos, float thick, Color color) {
  render_command *Command = create_render_command(rc);

  if (Command) {
    Command->Kind = render_command_DrawLineBezierCubic;
    Command->Points[0] = startPos;
    Command->Points[1] = startControlPos;
    Command->Points[2] = endControlPos;
    Command->Points[3] = endPos;
    Command->PointCount = 4;
    Command->Thickness = thick;
    Command->Color = color;
  }
}


function void render_DrawPoly(Render_Context *rc, Vector2 center, int sides, float radius, float rotation, Color color) {
  render_command *Command = create_render_command(rc);

  if (Command) {
    Command->Kind = render_command_DrawPoly;
    Command->X = center.x;
    Command->Y = center.y;
    Command->Sides = sides;
    Command->Radius = radius;
    Command->Rotation = rotation;
    Command->Color = color;
  }
}

function void render_DrawPolyLinesEx(Render_Context *rc, Vector2 center, int sides, float radius, float rotation, float lineThick, Color color) {
  render_command *Command = create_render_command(rc);

  if (Command) {
    Command->Kind = render_command_DrawPolyLinesEx;
    Command->X = center.x;
    Command->Y = center.y;
    Command->Sides = sides;
    Command->Radius = radius;
    Command->Rotation = rotation;
    Command->Thickness = lineThick;
    Command->Color = color;
  }
}

function void render_DrawTriangleStrip(Render_Context *rc, Vector2 *Points, S32 PointCount, Color Color) {
  render_command *Command = create_render_command(rc);

  if (Command && PointCount <= render_Max_Points) {
    Command->Kind = render_command_DrawTriangleStrip;
    for (S32 i = 0; i < PointCount; ++i) {
      Command->Points[i] = Points[i];
    }
    Command->PointCount = PointCount;
    Command->Color = Color;
  }
}

function void render_DrawTriangleFan(Render_Context *rc, Vector2 *Points, int PointCount, Color Color) {
  render_command *Command = create_render_command(rc);

  if (Command && PointCount <= render_Max_Points) {
    Command->Kind = render_command_DrawTriangleFan;
    for (S32 i = 0; i < PointCount; ++i) {
      Command->Points[i] = Points[i];
    }
    Command->PointCount = PointCount;
    Command->Color = Color;
  }
}

function void render_DrawCircle(Render_Context *rc, Vector2 center, F32 radius, Color color) {
  render_command *Command = create_render_command(rc);

  if (Command) {
    Command->Kind = render_command_DrawCircle;
    Command->X = center.x;
    Command->Y = center.y;
    Command->Radius = radius;
    Command->Color = color;
  }
}

function void render_DrawCircleSector(Render_Context *rc, Vector2 center, float radius, float startAngle, float endAngle, Color color) {
  render_command *Command = create_render_command(rc);

  if (Command) {
    Command->Kind = render_command_DrawCircleSector;
    Command->X = center.x;
    Command->Y = center.y;
    Command->Radius = radius;
    Command->StartAngle = startAngle;
    Command->EndAngle = endAngle;
    Command->Color = color;
  }
}

function void render_DrawCircleLines(Render_Context *rc, int centerX, int centerY, float radius, Color color) {
  render_command *Command = create_render_command(rc);

  if (Command) {
    Command->Kind = render_command_DrawCircleLines;
    Command->X = centerX;
    Command->Y = centerY;
    Command->Radius = radius;
    Command->Color = color;
  }
}

function void render_DrawCircleSectorLines(Render_Context *rc, Vector2 center, float radius, float startAngle, float endAngle, Color color) {
  render_command *Command = create_render_command(rc);

  if (Command) {
    Command->Kind = render_command_DrawCircleSectorLines;
    Command->X = center.x;
    Command->Y = center.y;
    Command->Radius = radius;
    Command->StartAngle = startAngle;
    Command->EndAngle = endAngle;
    Command->Color = color;
  }
}



function void render_Commands(Render_Context *rc) {
  // NOTE: Assume that the render commands get cleared every frame, so start from the start.
  /* S32 CommandCount = (context->render_arena->chunk_pos - context->render_zero_pos)/sizeof(render_command); */
  /* render_command *Commands = (render_command *)(arena + 1); */

  for (render_command *C = rc->command_list.first; C != 0; C = C->next) {
    /* render_command *C = Commands + i; */

    switch(C->Kind) {
    case render_command__Null: /* nothing to do here */ break;
    case render_command_ClearBackground: { ClearBackground(C->Color); } break;
    case render_command_DrawRectangleRec: { DrawRectangleRec(C->Rectangle, C->Color); } break;
    case render_command_DrawText: { DrawText(C->Text, C->X, C->Y, C->FontSize, C->Color); } break;
    case render_command_DrawRectangleLinesEx: { DrawRectangleLinesEx(C->Rectangle, C->Thickness, C->Color); } break;
    case render_command_DrawRectangle: { DrawRectangle(C->X, C->Y, C->Width, C->Height, C->Color); } break;
    case render_command_DrawLine: { DrawLineEx((Vector2){C->X, C->Y}, (Vector2){C->X2, C->Y2}, C->Thickness, C->Color); } break;
    case render_command_DrawLineBezierCubic: { DrawSplineBezierCubic(C->Points, C->PointCount, C->Thickness, C->Color); } break;
    case render_command_DrawPoly: { DrawPoly((Vector2){C->X, C->Y}, C->Sides, C->Radius, C->Rotation, C->Color); } break;
    case render_command_DrawPolyLinesEx: { DrawPolyLinesEx((Vector2){C->X, C->Y}, C->Sides, C->Radius, C->Rotation, C->Thickness, C->Color); } break;
    case render_command_DrawTriangleStrip: { DrawTriangleStrip(C->Points, C->PointCount, C->Color); } break;
    case render_command_DrawTriangleFan: { DrawTriangleFan(C->Points, C->PointCount, C->Color); } break;
    case render_command_DrawCircle: { DrawCircle(C->X, C->Y, C->Radius, C->Color); } break;
    case render_command_DrawCircleSector: { DrawCircleSector((Vector2){C->X, C->Y}, C->Radius, C->StartAngle, C->EndAngle, 10, C->Color); } break;
    case render_command_DrawCircleLines: { DrawCircleLines(C->X, C->Y, C->Radius, C->Color); } break;
    case render_command_DrawCircleSectorLines: { DrawCircleSectorLines((Vector2){C->X, C->Y}, C->Radius, C->StartAngle, C->EndAngle, 10, C->Color); } break;

    default: Assert(0); break;
    }
  }
}
