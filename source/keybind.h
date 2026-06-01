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
  Keybind_Timing__Null             = 0,
  Keybind_Timing_AtTheStart        = 1<<0,
  Keybind_Timing_ForAllProcesses   = 1<<1,
  Keybind_Timing_AtTheEnd          = 1<<2,
} Keybind_Timing;

typedef enum {
  Ui_Constraint__Null            = 0,
  Ui_Constraint_HoverProcess     = (1 << 0),
  Ui_Constraint_HotProcess       = (1 << 1),
  Ui_Constraint_NoHotProcess     = (1 << 2),
  Ui_Constraint_ExitOnKeyup      = (1 << 3),
  Ui_Constraint_ActionNotOccured = (1 << 4),
  Ui_Constraint_ActiveProcesses  = (1 << 5),
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
  Modifier_Key_Shift    = (1 << 1),
  Modifier_Key_Alt      = (1 << 2),
  Modifier_Key_Super    = (1 << 3),
} Modifier_Key;

#define Modifier_Key_4(a1, a2, a3, a4, ...)   Modifier_Key_##a1|Modifier_Key_##a2|Modifier_Key_##a3|Modifier_Key_##a4
#define Modifier_Key_3(a1, a2, a3, ...)       Modifier_Key_##a1|Modifier_Key_##a2|Modifier_Key_##a3
#define Modifier_Key_2(a1, a2, ...)           Modifier_Key_##a1|Modifier_Key_##a2
#define Modifier_Key_1(a1, ...)               Modifier_Key_##a1
#define SEMI_LIST(a1, a2, a3, a4, a5, ...)   Modifier_Key ## a5 (a1, a2, a3, a4)
#define Modifier_Keys(...)   SEMI_LIST(__VA_ARGS__, _4, _3, _2, _1)

struct Keybind_Environment {
  Context *context;
  View *view;
  Process_Selection selection;
  Keybind *keybind;

  // temp members
  /* Keybind_Result desired_kb_res; */
  /* Keybind_Result old_kb_res; */
  B32 is_active;
  Process *p;
  B32 should_stop_dragging;
  Process *moved_wire;
  Process_Connection moved_wire_conn;
};


struct Keybind {
  Keybind_Behavior behavior;
  Keybind_Timing timing;
  U32 bind;
  U32 key_kind; // Uses raylib's KEY_* enum and some special ones for mouse-keys
  U32 modifiers;
  Ui_Constraint constraint;
  String8 name;
  B32 (*handle)(Keybind_Environment *env);
};


function B32 keybind_zoom_handler(Keybind_Environment *env);






///////////////////////////
// Keybind Action
///////////////////////////
#define SYMBOL_SET_DEFINE Keybind_Action_Sym
#define Keybind_Action_Sym_Type      Keybind
#define Keybind_Action_Sym_section   "_prckbac"
#define Keybind_Action_Sym_ID(  N)  SymbolID(      Keybind_Action_Sym, N)
#define Keybind_Action_Sym_RAW( N)  SymbolRaw(     Keybind_Action_Sym, N)
#define Keybind_Action_Sym_DECL(N)  SymbolDeclare( Keybind_Action_Sym, N)
#define Keybind_Action_Sym_REF( N)  SymbolMetadata(Keybind_Action_Sym, N)
#include "../libraries/mr4th/src/mr4th_symbol_set.define.h"




///////////////////////////
// Keybind
///////////////////////////
#define SYMBOL_SET_DEFINE Keybind_Sym
#define Keybind_Sym_Type      Keybind
#define Keybind_Sym_section   "_prckbnd"
#define Keybind_Sym_ID(N)    SymbolID(Keybind_Sym, N)
#define Keybind_Sym_RAW(N)   SymbolRaw(Keybind_Sym, N)
#define Keybind_Sym_DECL(N)  SymbolDeclare(Keybind_Sym, N)
#define Keybind_Sym_REF(N)   SymbolMetadata(Keybind_Sym, N)
#include "../libraries/mr4th/src/mr4th_symbol_set.define.h"




#define Define_Keybind_Action(action_name, desc)\
  static Keybind_Action_Sym_DECL(action_name);\
  function B32 handle_keybind_##action_name(Keybind_Environment *env);\
  MR4TH_BEFORE_MAIN(proc_keybind_action##action_name){\
    Keybind_Sym_Type *keybind = Keybind_Action_Sym_REF(action_name);\
    keybind->name = str8_lit(Stringify(action_name));\
    keybind->handle = handle_keybind_##action_name;\
  }\
  function B32 handle_keybind_##action_name(Keybind_Environment *env)



#define Define_Keybind(\
  action_name, keybind_name,\
  behavior_name, bind_value, timing_name,\
  k, m, c)\
  static Keybind_Action_Sym_DECL(action_name);\
  static Keybind_Sym_DECL(action_name##_##keybind_name);\
  function B32 handle_keybind_##action_name(Keybind_Environment *env);\
  MR4TH_BEFORE_MAIN(proc_keybind_##action_name##_##keybind_name){\
    Keybind_Sym_Type *keybind = Keybind_Sym_REF(action_name##_##keybind_name);\
    B32 both_zero = (U32)(bind_value) == 0 && keybind->bind == 0;\
    B32 stronger_bind = (U32)(bind_value) > keybind->bind;\
    if (both_zero || stronger_bind) {\
      keybind->behavior = (behavior_name);\
      keybind->bind = (bind_value);\
      Set_Flag(keybind->timing, Keybind_Timing_##timing_name);\
      keybind->key_kind = (k);\
      keybind->modifiers = (m);\
      keybind->constraint = (c);\
      keybind->name = str8_lit(Stringify(action_name##_##keybind_name));\
      keybind->handle = handle_keybind_##action_name;\
    }\
  }



#define Define_Keybind_And_Action(\
  action_name, keybind_name,\
  behavior_name, bind_value, timing_name,\
  k, m, c, desc)\
  Define_Keybind(action_name, keybind_name, behavior_name, bind_value, timing_name, k, m, c);\
  Define_Keybind_Action(action_name, desc)



#define Handle_Keybind_Action(env, n)\
  handle_keybind_##n(env)


#define Keybind_Has_Mouse_Wheel_Movement(keybind)\
  ((keybind)->key_kind == Key_Kind_MouseWheelUp ||\
   (keybind)->key_kind == Key_Kind_MouseWheelDown)




function Keybind_Environment create_keybind_environment(
  Context *context,
  Process_Selection selection
  ) {
  Keybind_Environment env = (Keybind_Environment){0};

  env.context = context;
  env.selection = selection;

  return env;
}


#define Check_Keybind(env)\
  (((env) && (env)->context)\
   ? check_keybind((env))\
   : 0)

#define Test_Keybind(env, _kb_res)\
  (Check_Keybind((env)) == Keybind_Result_##_kb_res)

#define Keybind_Modifier_Matches(kb, _mod_name, _ui_flag_name)\
  !(Get_Flag_Bool((kb)->modifiers, Modifier_Key_##_mod_name) ^\
    Get_Flag_Bool(ui_state->flags, Ui_State_Flag_##_ui_flag_name))

#define Keybind_Constraint_Holds(kb, _con_name, con_expr)\
  (Get_Flag((kb)->constraint, Ui_Constraint_##_con_name)\
   ? (con_expr)\
   : 1)

function Keybind_Result check_keybind(Keybind_Environment *keybind_env) {
  Keybind_Result result = 0;
  Context *context = keybind_env->context;
  Keybind *keybind = keybind_env->keybind;
  Process_Selection selection = keybind_env->selection;

  if (keybind) {
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
        (context->hot_process.process != 0));

      B32 con_no_hot = Keybind_Constraint_Holds(
        keybind,
        NoHotProcess,
        (context->hot_process.process == 0));

      U32 kb_action_id = SymbolIDFromMetadata(Keybind_Action_Sym, keybind);
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
      ui_state->kb_action = SymbolIDFromMetadata(Keybind_Action_Sym, keybind);
    }
  }

  return result;
}



function B32 keybind_zoom_handler(Keybind_Environment *env) {
  B32 handled = 0;
  Context *context = env->context;

  B32 zoom_in = Test_Keybind(env, Enter);
  B32 zoom_out = Test_Keybind(env, Enter);

  for (S32 v = 0; v < View_Count; ++v) {
    View *view = context->views + v;
    Camera2D *camera = &view->camera;
    Vector2 mouse_world_position = GetScreenToWorld2D(context->ui_state.mouse_position,
                                                      *camera);

    camera->offset = context->ui_state.mouse_position;
    camera->target = mouse_world_position;

    if (zoom_in) {
      handled = 1;
      if (Keybind_Has_Mouse_Wheel_Movement(env->keybind)) {
        camera->zoom += -0.1f*context->ui_state.mouse_wheel_movement.y;
      }
      else {
        camera->zoom *= 1.4f;
      }
    }
    else if (zoom_out) {
      handled = 1;
      if (Keybind_Has_Mouse_Wheel_Movement(env->keybind)) {
        camera->zoom += -0.1f*context->ui_state.mouse_wheel_movement.y;
      }
      else {
        camera->zoom *= (1.0f/1.4f);
      }
    }

    camera->zoom = Max(0.1f, camera->zoom);
  }

  return handled;
}







#endif // PROC_KEYBIND_INCLUDE_H
