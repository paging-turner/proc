#ifndef PROC_KEYBIND_INCLUDE_H
# define PROC_KEYBIND_INCLUDE_H
//////////////////////////////////
// Keybind Declarations
//////////////////////////////////

typedef enum {
  Keybind_Behavior_Overwrite,
  Keybind_Behavior_Alternate
} Keybind_Behavior;

typedef enum {
  Ui_Constraint__Null            = 0,
  Ui_Constraint_HoverProcess     = (1 << 1),
  Ui_Constraint_HotProcess       = (1 << 2),
  Ui_Constraint_NoHotProcess     = (1 << 3),
  Ui_Constraint_ExitOnKeyup      = (1 << 4),
  Ui_Constraint_ActionNotOccured = (1 << 5),
  Ui_Constraint_ActiveProcesses  = (1 << 6),
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
  Modifier_Key_Super    = (1 << 4),
} Modifier_Key;

enum Keybind_Result {
  Keybind_Result__Null,
  Keybind_Result_Enter,
  Keybind_Result_Exit,
};

typedef struct Keybind_Environment {
  Context *context;
  Process_Selection selection;

  // temp members
  Keybind_Result desired_kb_res;
  B32 is_active;
  Process *p;
} Keybind_Environment;


struct Keybind {
  Keybind_Behavior behavior;
  U32 bind;
  U32 key_kind; // Uses raylib's KEY_* enum and some special ones for mouse-keys
  U32 modifiers;
  Ui_Constraint constraint;
  String8 name;
  String8 description;
  B32 (*handle)(Keybind_Environment env);
  Keybind *next;
};


function B32 keybind_zoom_handler(Keybind_Environment *env);




///////////////////////////
// Keybind Action
///////////////////////////

#define SYMBOL_SET_DEFINE keybind_action
#define keybind_action_Type      Keybind
#define keybind_action_section   "_prckbac"
#define keybind_action_ID(N)    SymbolID(keybind_action, N)
#define keybind_action_RAW(N)   SymbolRaw(keybind_action, N)
#define keybind_action_DECL(N)  SymbolDeclare(keybind_action, N)
#define keybind_action_REF(N)   SymbolMetadata(keybind_action, N)
#include "../libraries/mr4th/src/mr4th_symbol_set.define.h"




///////////////////////////
// Keybind
///////////////////////////
#define SYMBOL_SET_DEFINE keybind
#define keybind_Type      Keybind
#define keybind_section   "_prckbnd"
#define keybind_ID(N)    SymbolID(keybind, N)
#define keybind_RAW(N)   SymbolRaw(keybind, N)
#define keybind_DECL(N)  SymbolDeclare(keybind, N)
#define keybind_REF(N)   SymbolMetadata(keybind, N)
#include "../libraries/mr4th/src/mr4th_symbol_set.define.h"


// TODO: There should be a better way to determine custom keybinds than just non-zero bind-value....
#define Is_Keybind_Custom(keybind)\
  ((keybind) && (keybind)->bind > 0 && (keybind)->handle)


#define Define_Keybind(action_name, keybind_name, behavior_name, bind_value, k, m, c, d)\
  static keybind_action_DECL(action_name);\
  static keybind_DECL(action_name##keybind_name);\
  function B32 handle_keybind_##action_name##bind_value(Keybind_Environment env);\
  MR4TH_BEFORE_MAIN(proc_keybind_##action_name##_##bind_value){\
    keybind_Type *keybind = keybind_action_REF(action_name);\
    B32 both_zero = (U32)(bind_value) == 0 && keybind->bind == 0;\
    B32 stronger_bind = (U32)(bind_value) >= keybind->bind;\
    if (keybind->handle == 0 ||\
        (behavior_name == Keybind_Behavior_Overwrite && stronger_bind)) {\
      keybind->behavior = (behavior_name);\
      keybind->bind = (bind_value);\
      keybind->key_kind = (k);\
      keybind->modifiers = (m);\
      keybind->constraint = (c);\
      keybind->name = str8_lit(Stringify(action_name##keybind_name));\
      keybind->description = str8_lit(d);\
      keybind->handle = handle_keybind_##action_name##bind_value;\
    }\
    else if (behavior_name == Keybind_Behavior_Alternate &&\
             keybind->behavior != Keybind_Behavior_Overwrite) {\
    }\
  }\
  function B32 handle_keybind_##action_name##bind_value(Keybind_Environment env)


#define Keybind_Has_Mouse_Wheel_Movement(keybind_name)\
  (keybind_action_REF(keybind_name)->key_kind == Key_Kind_MouseWheelUp ||\
   keybind_action_REF(keybind_name)->key_kind == Key_Kind_MouseWheelDown)




function Keybind_Environment create_keybind_environment(
  Context *context,
  Process_Selection selection
  ) {
  Keybind_Environment env = (Keybind_Environment){0};

  env.context = context;
  env.selection = selection;

  return env;
}


#define Check_Keybind(env, _name)\
  (((env) && (env)->context)\
   ? check_keybind((env)->context, keybind_action_REF(_name), (env)->selection)\
   : 0)

#define Test_Keybind(env, _name, _kb_res)\
  (Check_Keybind((env), _name) == Keybind_Result_##_kb_res)

#define Keybind_Modifier_Matches(kb, _mod_name, _ui_flag_name)\
  !(Get_Flag_Bool((kb)->modifiers, Modifier_Key_##_mod_name) ^\
    Get_Flag_Bool(ui_state->flags, Ui_State_Flag_##_ui_flag_name))

#define Keybind_Constraint_Holds(kb, _con_name, con_expr)\
  (Get_Flag((kb)->constraint, Ui_Constraint_##_con_name)\
   ? (con_expr)\
   : 1)

function Keybind_Result check_keybind(Context *context, Keybind *keybind, Process_Selection selection) {
  Keybind_Result result = 0;
  Ui_State *ui_state = &context->ui_state;

  B32 key_is_pressed = 0;
  B32 key_is_down = 0;

  switch(keybind->key_kind) {
  case Key_Kind_Mouse0: {
    key_is_pressed = Get_Flag(ui_state->flags, Ui_State_Flag_mouse0_pressed);
    key_is_down = Get_Flag(ui_state->flags, Ui_State_Flag_mouse0_down);
  } break;
  case Key_Kind_Mouse1: {
    key_is_pressed = Get_Flag(ui_state->flags, Ui_State_Flag_mouse1_pressed);
    key_is_down = Get_Flag(ui_state->flags, Ui_State_Flag_mouse1_down);
  } break;
  case Key_Kind_MouseWheelUp: {
    key_is_pressed = ui_state->mouse_wheel_movement.y > 0.0f;
  } break;
  case Key_Kind_MouseWheelDown: {
    key_is_pressed = ui_state->mouse_wheel_movement.y < 0.0f;
  } break;
  default: {
    key_is_pressed = IsKeyPressed(keybind->key_kind);
    key_is_down = IsKeyDown(keybind->key_kind);
  } break;
  }

  B32 modifier_matches =
    (Keybind_Modifier_Matches(keybind, Control, control_down) &&
     Keybind_Modifier_Matches(keybind,   Shift,   shift_down) &&
     Keybind_Modifier_Matches(keybind,     Alt,     alt_down) &&
     Keybind_Modifier_Matches(keybind,   Super,   super_down));

  // constraints
  B32 constraints_met = 0;
  {
    B32 con_hover_process = Keybind_Constraint_Holds(
      keybind,
      HoverProcess,
      (selection.type != 0));

    B32 con_hot = Keybind_Constraint_Holds(
      keybind,
      HotProcess,
      (context->hot_process != 0));

    B32 con_no_hot = Keybind_Constraint_Holds(
      keybind,
      NoHotProcess,
      (context->hot_process == 0));

    U32 kb_action_id = SymbolIDFromMetadata(keybind_action, keybind);
    B32 action_occured_bool = Get_Flag_Bool(ui_state->flags, Ui_State_Flag_action_occured);
    B32 con_action_not_occured = Keybind_Constraint_Holds(
      keybind,
      ActionNotOccured,
      ((kb_action_id == ui_state->kb_action)
       ? 1
       : (action_occured_bool == 0)));

    B32 con_active_processes = Keybind_Constraint_Holds(
      keybind,
      ActiveProcesses,
      (context->active_processes.first != 0));


    constraints_met = (con_hover_process &&
                       con_hot &&
                       con_no_hot &&
                       con_action_not_occured &&
                       con_active_processes);
  }

  if (key_is_pressed && modifier_matches && constraints_met) {
    result = Keybind_Result_Enter;
  }

  if (Get_Flag(keybind->constraint, Ui_Constraint_ExitOnKeyup)) {
    if (!key_is_down) {
      result = Keybind_Result_Exit;
    }
  }

  if (result == Keybind_Result_Enter) {
    Set_Flag(ui_state->flags, Ui_State_Flag_action_occured);
    ui_state->kb_action = SymbolIDFromMetadata(keybind_action, keybind);
  }

  return result;
}

// TODO: @Cleanup We should NOT need to have these DECL's here...
static keybind_action_DECL(ZoomIn);
static keybind_action_DECL(ZoomOut);

function B32 keybind_zoom_handler(Keybind_Environment *env) {
  B32 handled = 0;
  Context *context = env->context;

  B32 zoom_in = Test_Keybind(env, ZoomIn, Enter);
  B32 zoom_out = Test_Keybind(env, ZoomOut, Enter);

  Vector2 mouse_world_position = GetScreenToWorld2D(context->ui_state.mouse_position,
                                                    context->camera);

  context->camera.offset = context->ui_state.mouse_position;
  context->camera.target = mouse_world_position;

  if (zoom_in) {
    handled = 1;
    if (Keybind_Has_Mouse_Wheel_Movement(ZoomIn)) {
      context->camera.zoom += -0.1f*context->ui_state.mouse_wheel_movement.y;
    }
    else {
      context->camera.zoom *= 1.4f;
    }
  }
  else if (zoom_out) {
    handled = 1;
    if (Keybind_Has_Mouse_Wheel_Movement(ZoomOut)) {
      context->camera.zoom += -0.1f*context->ui_state.mouse_wheel_movement.y;
    }
    else {
      context->camera.zoom *= (1.0f/1.4f);
    }
  }

  context->camera.zoom = Max(0.1f, context->camera.zoom);

  return handled;
}







#endif // PROC_KEYBIND_INCLUDE_H
