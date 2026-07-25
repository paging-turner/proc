#ifndef PROC_RENDER_INCLUDE_H
# define PROC_RENDER_INCLUDE_H
/*
    A wrapper for raylib rendering functions. Also, implements a command buffer, just in case we want to process the render-commands before actually drawing anything.
*/

typedef enum {
  render_command__Null,
  render_command_ClearBackground,
  render_command_DrawRectangleRec,
  render_command_DrawText,
  render_command_DrawRectangle,
  render_command_DrawLine,
  render_command_DrawLineBezierCubic,
  render_command_DrawTriangleStrip,
  render_command_DrawTriangleStrip_P,
  render_command_DrawTriangleFan,
  render_command_DrawCircle,
  render_command_DrawCircleLines,
  render_command_DrawCircleSectorLines,
  render_command_BeginScissorMode,
  render_command_EndScissorMode,
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
  Vector2 *Points_Ptr;
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
  /* B32 reverse_commands; */
} Render_Context;

global_variable Arena *render_GlobalTempArena;


function void render_Initialize(Arena *TempArena) {
  render_GlobalTempArena = TempArena;
}

function render_command *create_render_command(Render_Context *rc) {
  render_command *command = push_struct(rc->arena, render_command);

  /* if (rc->reverse_commands) { */
  /*   SLLQueuePushFront(rc->command_list.first, rc->command_list.last, command); */
  /* } else { */
    SLLQueuePush(rc->command_list.first, rc->command_list.last, command);
  /* } */

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




function void render_DrawLineBezierCubic(Render_Context *rc, Vector2 startPos, Vector2 endPos, Vector2 startControlPos, Vector2 endControlPos, float thick, Color color, B32 closed) {
  render_command *Command = create_render_command(rc);

  if (Command) {
    U32 point_count = Points_Per_Wire;
    Vector2 *points = push_array(rc->arena, Vector2, point_count);

    if (points) {
      // fill out points on bezier line
      for (U32 i = 0; i < point_count; ++i) {
        F32 t = (F32)i / (F32)(point_count-1);
        points[i] = get_bezier_point(startPos, endPos, startControlPos, endControlPos, t);
      }

      // get connected-path from bezier-path
      Buffer_V2 connected_path = get_connected_path(rc->arena, points, point_count, thick, closed);

      Command->Kind = render_command_DrawLineBezierCubic;
      Command->PointCount = connected_path.point_count;
      Command->Points_Ptr = connected_path.points;
      Command->Color = color;
    }
  }
}



function void render_DrawTriangleStrip_P(Render_Context *rc, Vector2 *Points, S32 PointCount, Color Color) {
  render_command *Command = create_render_command(rc);

  if (Command) {
    Command->Kind = render_command_DrawTriangleStrip_P;
    Command->Points_Ptr = Points;
    Command->PointCount = PointCount;
    Command->Color = Color;
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


function void render_DrawCircleLines(Render_Context *rc, int centerX, int centerY, float radius, F32 thickness, Color color) {
  render_command *Command = create_render_command(rc);
  U32 point_count = 24; // TODO: figure out a better way to determine the number of points to use
  Vector2 *points = push_array(rc->arena, Vector2, point_count);

  if (points) {
    for (U32 i = 0; i < point_count; ++i) {
      F32 t = (F32)i / (F32)point_count;
      points[i].x = centerX + radius*cos_F32(t);
      points[i].y = centerY + radius*sin_F32(t);
    }

    Buffer_V2 connected_path = get_connected_path(rc->arena, points, point_count, thickness, 1);

    if (Command) {
      Command->Kind = render_command_DrawCircleLines;
      Command->PointCount = connected_path.point_count;
      Command->Points_Ptr = connected_path.points;
      Command->Color = color;
    }
  }
}


function void render_BeginScissorMode(Render_Context *rc, Vector2 position, Vector2 size) {
  render_command *Command = create_render_command(rc);

  if (Command) {
    Command->Kind = render_command_BeginScissorMode;
    Command->X = position.x;
    Command->Y = position.y;
    Command->Width = size.x;
    Command->Height = size.y;
  }
}

function void render_EndScissorMode(Render_Context *rc) {
  render_command *Command = create_render_command(rc);

  if (Command) {
    Command->Kind = render_command_EndScissorMode;
  }
}


function void render_Commands(Render_Context *rc) {
  for (render_command *C = rc->command_list.first; C != 0; C = C->next) {
    switch(C->Kind) {
    case render_command__Null: /* nothing to do here */ break;
    case render_command_ClearBackground: { ClearBackground(C->Color); } break;
    case render_command_DrawRectangleRec: { DrawRectangleRec(C->Rectangle, C->Color); } break;
    case render_command_DrawText: { DrawText(C->Text, C->X, C->Y, C->FontSize, C->Color); } break;
    case render_command_DrawRectangle: { DrawRectangle(C->X, C->Y, C->Width, C->Height, C->Color); } break;
    case render_command_DrawLine: { DrawLineEx((Vector2){C->X, C->Y}, (Vector2){C->X2, C->Y2}, C->Thickness, C->Color); } break;
    case render_command_DrawLineBezierCubic: { DrawTriangleStrip(C->Points_Ptr, C->PointCount, C->Color); } break;
    case render_command_DrawTriangleStrip: { DrawTriangleStrip(C->Points, C->PointCount, C->Color); } break;
    case render_command_DrawTriangleStrip_P: { DrawTriangleStrip(C->Points_Ptr, C->PointCount, C->Color); } break;
    case render_command_DrawTriangleFan: { DrawTriangleFan(C->Points, C->PointCount, C->Color); } break;
    case render_command_DrawCircle: { DrawCircle(C->X, C->Y, C->Radius, C->Color); } break;
    case render_command_DrawCircleLines: { DrawTriangleStrip(C->Points_Ptr, C->PointCount, C->Color); } break;
    case render_command_BeginScissorMode: { BeginScissorMode(C->X, C->Y, C->Width, C->Height); } break;
    case render_command_EndScissorMode: { EndScissorMode(); } break;

    default: Assert(0); break;
    }
  }
}






#endif // PROC_RENDER_INCLUDE_H
