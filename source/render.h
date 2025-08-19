// TODO: add "render commands" that the render functions use to push on commands, then render those commands later.

typedef enum
{
  render_command_ClearBackground,
  render_command_DrawRectangleRec,
  render_command_DrawText,
  render_command_DrawRectangleLinesEx,
  render_command_DrawRectangle,
  render_command_DrawLine,
  render_command_DrawLineBezierCubic,
  render_command_DrawPoly,
  render_command_DrawPolyLinesEx,
} render_command_kind;


typedef struct render_command
{
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
} render_command;


global_variable arena *GlobalTempArena;


function void render_Initialize(arena *TempArena) {
  GlobalTempArena = TempArena;
}


function void render_ClearBackground(arena *Arena, Color C)
{
  render_command *Command = ryn_memory_PushZeroStruct(Arena, render_command);
  if (Command)
  {
    Command->Color = C;
  }
}

function void render_DrawRectangleRec(arena *Arena, Rectangle R, Color C)
{
  render_command *Command = ryn_memory_PushZeroStruct(Arena, render_command);

  if (Command)
  {
    Command->Kind = render_command_DrawRectangleRec;
    Command->Rectangle = R;
    Command->Color = C;
  }
}

function char *render_PushTempString(const char *CString)
{
  Assert(GlobalTempArena);
  char *Result = (char *)GetArenaWriteLocation(GlobalTempArena);
  const char *C = CString;

  for (;; ++C)
  {
    if (PushChar(GlobalTempArena, *C) == 0) {
      *Result = 0; // Null out the result in case somebody prints the result.
      Assert(0);
      break;
    }

    if ((*C) == 0) {
      break;
    }
  }

  return Result;
}

function void render_DrawText(arena *Arena, const char *Text, F32 X, F32 Y, S32 FontSize, Color C, B32 copy_string)
{
  render_command *Command = ryn_memory_PushZeroStruct(Arena, render_command);
  const char *RenderString;
  if (copy_string) {
    RenderString = render_PushTempString(Text);
  } else {
    RenderString = Text;
  }

  if (Command)
  {
    Command->Kind = render_command_DrawText;
    Command->Text = RenderString;
    Command->X = X;
    Command->Y = Y;
    Command->FontSize = FontSize;
    Command->Color = C;
  }
}

function void render_DrawRectangleLinesEx(arena *Arena, Rectangle R, F32 Thickness, Color C)
{
  render_command *Command = ryn_memory_PushZeroStruct(Arena, render_command);

  if (Command)
  {
    Command->Kind = render_command_DrawRectangleLinesEx;
    Command->Rectangle = R;
    Command->Thickness = Thickness;
    Command->Color = C;
  }
}

function void render_DrawRectangle(arena *Arena, F32 X, F32 Y, F32 W, F32 H, Color C)
{
  render_command *Command = ryn_memory_PushZeroStruct(Arena, render_command);

  if (Command)
  {
    Command->Kind = render_command_DrawRectangle;
    Command->X = X;
    Command->Y = Y;
    Command->Width = W;
    Command->Height = H;
    Command->Color = C;
  }
}

function void render_DrawLine(arena *Arena, int startPosX, int startPosY, int endPosX, int endPosY, F32 thickness, Color color)
{
  render_command *Command = ryn_memory_PushZeroStruct(Arena, render_command);

  if (Command)
  {
    Command->Kind = render_command_DrawLine;
    Command->X = startPosX;
    Command->Y = startPosY;
    Command->X2 = endPosX;
    Command->Y2 = endPosY;
    Command->Thickness = thickness;
    Command->Color = color;
  }
}


function void render_DrawLineBezierCubic(arena *Arena, Vector2 startPos, Vector2 endPos, Vector2 startControlPos, Vector2 endControlPos, float thick, Color color)
{
  render_command *Command = ryn_memory_PushZeroStruct(Arena, render_command);

  if (Command)
  {
    Command->Kind = render_command_DrawLineBezierCubic;
    Command->X = startPos.x;
    Command->Y = startPos.y;
    Command->X2 = endPos.x;
    Command->Y2 = endPos.y;
    Command->ControlX = startControlPos.x;
    Command->ControlY = startControlPos.y;
    Command->ControlX2 = endControlPos.x;
    Command->ControlY2 = endControlPos.y;
    Command->Thickness = thick;
    Command->Color = color;
  }
}


function void render_DrawPoly(arena *Arena, Vector2 center, int sides, float radius, float rotation, Color color) {
  render_command *Command = ryn_memory_PushZeroStruct(Arena, render_command);

  if (Command)
  {
    Command->Kind = render_command_DrawPoly;
    Command->X = center.x;
    Command->Y = center.y;
    Command->Sides = sides;
    Command->Radius = radius;
    Command->Rotation = rotation;
    Command->Color = color;
  }
}

function void render_DrawPolyLinesEx(arena *Arena, Vector2 center, int sides, float radius, float rotation, float lineThick, Color color) {
  render_command *Command = ryn_memory_PushZeroStruct(Arena, render_command);

  if (Command)
  {
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

function void render_Commands(arena *Arena)
{
  // NOTE: Assume that the render commands get cleared every frame, so start from the start.
  U32 CommandCount = Arena->Offset / sizeof(render_command);
  render_command *Commands = (render_command *)Arena->Data;

  for (U32 i = 0; i < CommandCount; ++i)
  {
    render_command *C = Commands + i;

    switch(C->Kind)
    {
    case render_command_ClearBackground: { ClearBackground(C->Color); } break;
    case render_command_DrawRectangleRec: { DrawRectangleRec(C->Rectangle, C->Color); } break;
    case render_command_DrawText: { DrawText(C->Text, C->X, C->Y, C->FontSize, C->Color); } break;
    case render_command_DrawRectangleLinesEx: { DrawRectangleLinesEx(C->Rectangle, C->Thickness, C->Color); } break;
    case render_command_DrawRectangle: { DrawRectangle(C->X, C->Y, C->Width, C->Height, C->Color); } break;
    case render_command_DrawLine: { DrawLineEx((Vector2){C->X, C->Y}, (Vector2){C->X2, C->Y2}, C->Thickness, C->Color); } break;
    case render_command_DrawLineBezierCubic: { DrawLineBezierCubic((Vector2){C->X, C->Y}, (Vector2){C->X2, C->Y2}, (Vector2){C->ControlX, C->ControlY}, (Vector2){C->ControlX2, C->ControlY2}, C->Thickness, C->Color); } break;
    case render_command_DrawPoly: { DrawPoly((Vector2){C->X, C->Y}, C->Sides, C->Radius, C->Rotation, C->Color); } break;
    case render_command_DrawPolyLinesEx: { DrawPolyLinesEx((Vector2){C->X, C->Y}, C->Sides, C->Radius, C->Rotation, C->Thickness, C->Color); } break;

    default: Assert(0); break;
    }
  }
}
