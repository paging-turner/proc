//////////////////////////////////
// Keybind Declarations
//////////////////////////////////

enum Keybind_Result {
  Keybind_Result__Null,
  Keybind_Result_Enter,
  Keybind_Result_Exit,
};

typedef enum {
  Keybind_Behavior_Overwrite,
  Keybind_Behavior_Alternate
} Keybind_Behavior;


typedef enum {
  Keybind_Timing_OnlyOnce,
  Keybind_Timing_ForAllProcesses
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


typedef enum {
  Keybind_Order__Null,
  Keybind_Order_Before,
  Keybind_Order_After,
} Keybind_Order_Kind;


typedef struct Keybind_Order {
  Keybind_Order_Kind kind;
  B32 removed;
  Keybind *keybind_a;
  Keybind *keybind_b;
} Keybind_Order;


struct Keybind_Environment {
  Context *context;
  View *view;
  Process_Selection selection;
  Keybind *keybind;

  // temp members
  B32 is_active;
  Process *p;
  Process *moved_wire;
  Process_Connection moved_wire_conn;
  Keybind_Result kb_res;
};


struct Keybind {
  Keybind_Behavior behavior;
  B32 for_all_processes;
  U32 key_kind; // Uses raylib's KEY_* enum and some special ones for mouse-keys
  U32 modifiers;
  Ui_Constraint constraint;
  String8 name;
  B32 (*handle)(Keybind_Environment *env);
  View_Kind_Flag view_kind_flags;

  Keybind *next;
};








///////////////////////////
// Keybind Action
///////////////////////////
#define SYMBOL_SET_DEFINE Keybind_Action_Sym
#define Keybind_Action_Sym_Type     Keybind
#define Keybind_Action_Sym_section  "_prckbac"
#define Keybind_Action_Sym_ID(  N)  SymbolID(      Keybind_Action_Sym, N)
#define Keybind_Action_Sym_RAW( N)  SymbolRaw(     Keybind_Action_Sym, N)
#define Keybind_Action_Sym_DECL(N)  SymbolDeclare( Keybind_Action_Sym, N)
#define Keybind_Action_Sym_REF( N)  SymbolMetadata(Keybind_Action_Sym, N)
#include "../libraries/mr4th/src/mr4th_symbol_set.define.h"




///////////////////////////
// Keybind
///////////////////////////
#define SYMBOL_SET_DEFINE Keybind_Sym
#define Keybind_Sym_Type     Keybind
#define Keybind_Sym_section  "_prckbnd"
#define Keybind_Sym_ID(  N)  SymbolID(      Keybind_Sym, N)
#define Keybind_Sym_RAW( N)  SymbolRaw(     Keybind_Sym, N)
#define Keybind_Sym_DECL(N)  SymbolDeclare( Keybind_Sym, N)
#define Keybind_Sym_REF( N)  SymbolMetadata(Keybind_Sym, N)
#include "../libraries/mr4th/src/mr4th_symbol_set.define.h"



///////////////////////////
// Keybind Order
///////////////////////////
#define SYMBOL_SET_DEFINE Keybind_Order_Sym
#define Keybind_Order_Sym_Type     Keybind_Order
#define Keybind_Order_Sym_section  "_prckbor"
#define Keybind_Order_Sym_ID(  N)  SymbolID(      Keybind_Order_Sym, N)
#define Keybind_Order_Sym_RAW( N)  SymbolRaw(     Keybind_Order_Sym, N)
#define Keybind_Order_Sym_DECL(N)  SymbolDeclare( Keybind_Order_Sym, N)
#define Keybind_Order_Sym_REF( N)  SymbolMetadata(Keybind_Order_Sym, N)
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
  behavior_name, timing_name,\
  k, m, c, v)\
  static Keybind_Action_Sym_DECL(action_name);\
  static Keybind_Sym_DECL(action_name##_##keybind_name);\
  function B32 handle_keybind_##action_name(Keybind_Environment *env);\
  MR4TH_BEFORE_MAIN(proc_keybind_##action_name##_##keybind_name){\
    Keybind_Sym_Type *keybind = Keybind_Sym_REF(action_name##_##keybind_name);\
    keybind->behavior = (behavior_name);\
    keybind->for_all_processes = Keybind_Timing_##timing_name;\
    keybind->key_kind = (k);\
    keybind->modifiers = (m);\
    keybind->constraint = (c);\
    keybind->view_kind_flags = (v);\
    Assert(keybind->view_kind_flags);\
    keybind->name = str8_lit(Stringify(action_name##_##keybind_name));\
    keybind->handle = handle_keybind_##action_name;\
  }


#define Define_Keybind_Order(keybind_name_a, order, keybind_name_b)\
  static Keybind_Sym_DECL(keybind_name_a);\
  static Keybind_Sym_DECL(keybind_name_b);\
  static Keybind_Order_Sym_DECL(keybind_name_a##_##order##_##keybind_name_b);\
  MR4TH_BEFORE_MAIN(proc_keybind_order_##keybind_name_a##_##order##_##keybind_name_b){\
    Keybind_Order_Sym_Type *keybind_order =\
      Keybind_Order_Sym_REF(keybind_name_a##_##order##_##keybind_name_b);\
    keybind_order->kind = Keybind_Order_##order;\
    keybind_order->keybind_a = Keybind_Sym_REF(keybind_name_a);\
    keybind_order->keybind_b = Keybind_Sym_REF(keybind_name_b);\
  }


#define Define_Keybind_And_Action(\
  action_name, keybind_name,\
  behavior_name, timing_name,\
  k, m, c, v, desc)\
  Define_Keybind(action_name, keybind_name, behavior_name, timing_name, k, m, c, v)\
  Define_Keybind_Action(action_name, desc)



#define Handle_Keybind_Action(env, n)\
  handle_keybind_##n(env)


#define Keybind_Has_Mouse_Wheel_Movement(keybind)\
  ((keybind)->key_kind == Key_Kind_MouseWheelUp ||\
   (keybind)->key_kind == Key_Kind_MouseWheelDown)


#define Keybind_Is_Before(k, o)\
  (((o)->kind == Keybind_Order_Before && (o)->keybind_a == (k)) ||\
   ((o)->kind == Keybind_Order_After && (o)->keybind_b == (k)))

#define Keybind_Is_After(k, o)\
  (((o)->kind == Keybind_Order_Before && (o)->keybind_b == (k)) ||\
   ((o)->kind == Keybind_Order_After && (o)->keybind_a == (k)))

#define Keybind_Order_Is_Valid(o)\
  ((o)->keybind_a && (o)->keybind_b &&\
   (o)->keybind_a->handle && (o)->keybind_b->handle &&\
   ((o)->keybind_a != (o)->keybind_b))


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
  (((kb)->modifiers == 0) ||\
   (!(Get_Flag_Bool((kb)->modifiers, Modifier_Key_##_mod_name) ^\
      Get_Flag_Bool(ui_state->modifier_flags, Ui_State_Modifier_Flag_##_ui_flag_name))))

#define Keybind_Constraint_Holds(kb, _con_name, con_expr)\
  (Get_Flag((kb)->constraint, Ui_Constraint_##_con_name)\
   ? (con_expr)\
   : 1)

function Keybind_Result check_keybind(Keybind_Environment *env) {
  Keybind_Result result = 0;
  Context *context = env->context;
  View *view = env->view;
  Keybind *keybind = env->keybind;
  Process_Selection selection = env->selection;
  B32 should_handle = Get_Flag(view->kind_flags, env->keybind->view_kind_flags);

  if (context && view && keybind && should_handle) {
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
        (view->and_whats_this.active_processes.first != 0));

      B32 con_satisfies_keypress = keybind->key_kind == 0 || key_is_pressed;

      constraints_met = (con_hover_process &&
                         con_hot &&
                         con_no_hot &&
                         con_action_not_occured &&
                         con_active_processes &&
                         con_satisfies_keypress);
    }

    if (modifier_matches && constraints_met) {
      result = Keybind_Result_Enter;
    }

    if (Get_Flag(keybind->constraint, Ui_Constraint_ExitOnKeyup)) {
      if (!key_is_down) {
        result = Keybind_Result_Exit;
      }
    }

    if (result == Keybind_Result_Enter) {
      ui_state->kb_action = SymbolIDFromMetadata(Keybind_Action_Sym, keybind);
    }
  }

  return result;
}












function void keybind_handle(Keybind_Environment *env, Keybind *keybind) {
  Context *context = env->context;
  View *view = env->view;
  if (context && view) {
    Keybind_Result kb_res = check_keybind(env);
    if (kb_res) {
      env->kb_res = kb_res;
      keybind->handle(env);
    }
  }
}






function void exit_add_wire_mode(Context *context, View *view) {
  clear_active_processes(&context->what_is_this, &view->and_whats_this.active_processes);
  Unset_Flag(context->flags, Context_Flag_NewWire);
}











Define_Keybind_And_Action(
  HandleActiveProcess, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  0, 0, 0, View_Kind_Flag_Procs,
  "handle active-process"
  ) {
  if (env->context && env->view) {
    if (env->view->and_whats_this.active_processes.first) {
      if (!Get_Flag(env->context->ui_state.flags, Ui_State_Flag_action_occured)) {
        // process label editing
        handle_label_editing(env->context, env->view, env->view->and_whats_this.active_processes);
      }
    }
  }

  return 0;
}

Define_Keybind_Order(HandleActiveProcess_Default, After, ForAllProcessInteractions_Default);





Define_Keybind_And_Action(
  Bound, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  Key_Kind_Mouse0, 0,
  Ui_Constraint_NoHotProcess|Ui_Constraint_ExitOnKeyup, View_Kind_Flag_Procs,
  "Select multiple processes by drawing a rectangle with your mouse."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  View *view = env->view;

  if (context && view) {
    Keybind_Result kb_res = Check_Keybind(env);
    if (kb_res == Keybind_Result_Enter) {
      // enter
      Set_Flag(context->flags, Context_Flag_Bounding);
      context->ui_state.active_position = context->ui_state.mouse_position;
      handled = 1;
    }
    else if (kb_res == Keybind_Result_Exit) {
      if (Get_Flag(context->flags, Context_Flag_Bounding)) {
        // exit
        Unset_Flag(context->flags, Context_Flag_Bounding);
        handled = 1;
      }
    }
  }

  return handled;
}

Define_Keybind_Order(Bound_Default, After, ForAllProcessInteractions_Default);




Define_Keybind_And_Action(
  PerProcessBounding, Default,
  Keybind_Behavior_Alternate, ForAllProcesses,
  0, 0, 0, View_Kind_Flag_Procs,
  "Per-process bounding."
  ) {
  if (env->context && env->p) {
    if (Get_Flag(env->context->flags, Context_Flag_Bounding)) {
      Rectangle selection_rectangle = get_selection_rectangle(env->context);

      if (Get_Flag(env->p->flags, Process_Flag_Wire)) {
        Process_Shape out_shape = get_process_shape(env->context, env->view, env->p->out);
        Process_Shape in_shape = get_process_shape(env->context, env->view, env->p->in);
        Vector2 out_position = get_wire_position_from_wire(env->context, env->view, env->p, out_shape, Process_Connection_Out);
        Vector2 in_position = get_wire_position_from_wire(env->context, env->view, env->p, in_shape, Process_Connection_In);

        if (rectangle_contains_point(selection_rectangle, out_position) ||
            rectangle_contains_point(selection_rectangle, in_position)) {
          SLLQueuePush_NZ(env->view->and_whats_this.active_processes.first, env->view->and_whats_this.active_processes.last, env->p, next_active, 0);
        }
      } else {
        Process_Shape shape = get_process_shape(env->context, env->view, env->p);

        if (rectangle_contains_point(selection_rectangle, shape.center)) {
          SLLQueuePush_NZ(env->view->and_whats_this.active_processes.first, env->view->and_whats_this.active_processes.last, env->p, next_active, 0);
        }
      }
    }
  }

  return 0;
}





Define_Keybind_And_Action(
  ZeroOutSelection, Default,
  Keybind_Behavior_Alternate, OnlyOnce, 0, 0, 0, View_Kind_Flag_Procs,
  ""
  ) {
  if (env->context) {
    // zero out selection
    env->selection = (Process_Selection){0};
    // zero the old hot-id
    if (!Get_Flag(env->context->ui_state.flags, Ui_State_Flag_hot_id_assigned)) {
      env->context->hot_process = (Process_Loc){0};
    }
  }

  return 0;
}

Define_Keybind_Order(ZeroOutSelection_Default, After, ForAllProcessInteractions_Default);






Define_Keybind_And_Action(
  HandleMovedWire, Default,
  Keybind_Behavior_Alternate, OnlyOnce, 0, 0, 0, View_Kind_Flag_Procs,
  "handle moved wire"
  ) {
  Context *context = env->context;
  View *view = env->view;
  if (context && view && env->moved_wire && env->context) {
    Process *hot_process = env->context->hot_process.process;

    if (hot_process) {
      if (Get_Flag(hot_process->flags, Process_Flag_Wire)) {
        Process *connected_process = hot_process->conn[env->moved_wire_conn];
        if (connected_process) {
          // move wire to hovered wire
          U32 which_conn = hot_process->which_conn[env->moved_wire_conn];
          if (env->moved_wire != hot_process) {
            B32 wire_moved_to_new_process = env->moved_wire->conn[env->moved_wire_conn] != connected_process;
            B32 wire_moved_to_same_place = env->moved_wire->which_conn[env->moved_wire_conn] == (which_conn - 1);
            if (wire_moved_to_new_process || !wire_moved_to_same_place) {
              WhatIsThis wit = (WhatIsThis){0}; // TODO: handle passing/returning wits
              add_wire_connection(&wit, env->moved_wire, connected_process, env->moved_wire_conn, which_conn);
              gather_processes_from_trie(&context->what_is_this);
              exit_add_wire_mode(context, view);
            }
          }
        }
      } else {
        // move wire to last wire of process
        Process *connected_process = hot_process;
        U32 which_conn = connected_process->conn_count[env->moved_wire_conn];
        B32 not_moving_to_the_same_place =
          (env->moved_wire->conn[env->moved_wire_conn] != connected_process ||
           env->moved_wire->which_conn[env->moved_wire_conn] != which_conn);
        if (not_moving_to_the_same_place) {
          WhatIsThis wit = (WhatIsThis){0}; // TODO: handle passing/returning wits
          add_wire_connection(&wit, env->moved_wire, connected_process, env->moved_wire_conn, which_conn);
          gather_processes_from_trie(&context->what_is_this);
          exit_add_wire_mode(context, view);
        }
      }
    }
  }

  return 0;
}

Define_Keybind_Order(HandleMovedWire_Default, After, ForAllProcessInteractions_Default);







Define_Keybind_And_Action(
  Pan, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  Key_Kind_Mouse1, 0,
  Ui_Constraint_ExitOnKeyup, View_Kind_Flag_Procs,
  "Slide your field of view by moving your mouse."
  ) {
  B32 handled = 1;
  Context *context = env->context;
  View *view = env->view;

  if (context && view) {
    Keybind_Result kb_res = Check_Keybind(env);

    B32 mouse_is_within_view = 1;

    if (Get_Flag(view->flags, View_Flag_Active) && mouse_is_within_view) {
      if (kb_res == Keybind_Result_Enter) {
        Set_Flag(view->flags, View_Flag_Panning);
        context->ui_state.active_position = context->ui_state.mouse_position;
        handled = 1;
      }

      if (Get_Flag(view->flags, View_Flag_Panning)) {
        if (kb_res == Keybind_Result_Exit) {
          Unset_Flag(view->flags, View_Flag_Panning);
          handled = 1;
        }
        else {
          // Update camera position
          Vector2 delta = GetMouseDelta();
          delta = Vector2Scale(delta, -1.0f/view->camera.zoom);
          view->camera.target = Vector2Add(view->camera.target, delta);
          handled = 1;
        }
      }
    }
  }

  return handled;
}

Define_Keybind_Order(Pan_Default, Before, ForAllProcessInteractions_Default);


Define_Keybind_Action(
  Zoom,
  "Zoom your field of view in to make objects appear closer or further."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  View *view = env->view;

  if (context && view) {
    if (Test_Keybind(env, Enter)) {
      Camera2D *camera = &view->camera;
      Vector2 mouse_world_position = GetScreenToWorld2D(context->ui_state.mouse_position,
                                                        *camera);

      camera->offset = context->ui_state.mouse_position;
      camera->target = mouse_world_position;
      handled = 1;

      if (Keybind_Has_Mouse_Wheel_Movement(env->keybind)) {
        camera->zoom += -0.1f*context->ui_state.mouse_wheel_movement.y;
      }
      else {
        camera->zoom *= 1.4f;
      }

      camera->zoom = Max(0.1f, camera->zoom);
    }
  }

  return handled;
}




Define_Keybind(
  Zoom, DefaultIn,
  Keybind_Behavior_Alternate, OnlyOnce,
  Key_Kind_MouseWheelUp, 0,
  Ui_Constraint_ActionNotOccured, View_Kind_Flag_Procs);


Define_Keybind(
  Zoom, DefaultOut,
  Keybind_Behavior_Alternate, OnlyOnce,
  Key_Kind_MouseWheelDown, 0,
  Ui_Constraint_ActionNotOccured, View_Kind_Flag_Procs);

Define_Keybind_Order(Zoom_DefaultIn, Before, ForAllProcessInteractions_Default);
Define_Keybind_Order(Zoom_DefaultOut, Before, ForAllProcessInteractions_Default);


Define_Keybind_And_Action(
  ForAllProcessInteractions, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  0, 0, 0, View_Kind_Flag_Procs,
  "Loop through all processes and handle per-process interactions."
  ) {
  Context *context = env->context;
  View *view = env->view;

  if (context && view) {
    if (Get_Flag(view->flags, View_Flag_Active)) {
      for (Process *p = view->and_whats_this.processes.first; p != 0; p = p->next) {
        // per-process environment
        env->selection = get_process_selection(context, view, p);
        env->view = view;
        env->is_active = is_active_process(context, view, p);
        env->p = p;

        // hot id assignment
        B32 ui_state_hot_id_assigned = Get_Flag(context->ui_state.flags, Ui_State_Flag_hot_id_assigned);
        B32 hot_id_assigned = env->selection.hot_id_assigned || ui_state_hot_id_assigned;
        Assign_Flag(context->ui_state.flags, Ui_State_Flag_hot_id_assigned, hot_id_assigned);

        // per-process keybinds
        for (U32 i = 0; i < context->keybind_count; ++i) {
          Keybind *keybind = context->keybinds + i;
          env->keybind = keybind;

          if (keybind->for_all_processes) {
            keybind_handle(env, keybind);
          }
        }
      }
    }
  }

  return 0;
}







Define_Keybind_And_Action(
  SelectSingleProcess, Default,
  Keybind_Behavior_Alternate, ForAllProcesses,
  Key_Kind_Mouse0, 0,
  Ui_Constraint_HotProcess|Ui_Constraint_ExitOnKeyup|Ui_Constraint_ActionNotOccured, View_Kind_Flag_Procs,
  "Select a single process."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  View *view = env->view;
  Process_Selection selection = env->selection;
  Keybind_Result kb_res = Check_Keybind(env);

  if (env->p == 0) {
    return 0;
  }

  if (context && view) {
    if (kb_res == Keybind_Result_Enter) {
      if (selection.view == view) {
        handled = 1;
        B32 in_selection = selection.type == Process_Selection_In;
        B32 out_selection = selection.type == Process_Selection_Out;
        if (in_selection || out_selection) {
          // select wire
          Process *wire = get_wire_from_selection(context, view, selection);
          B32 is_active_wire = is_active_process(context, view, wire);

          if (wire) {
            U32 drag_flag = in_selection ? Process_Flag_Drag_In : Process_Flag_Drag_Out;
            Unset_Flag(context->flags, Context_Flag_NewWire);
            Set_Flag(wire->flags, drag_flag);
            context->ui_state.active_position = context->ui_state.mouse_position;
            if (!is_active_wire) {
              clear_active_processes(&context->what_is_this, &view->and_whats_this.active_processes);
              SLLQueuePush_NZ(view->and_whats_this.active_processes.first, view->and_whats_this.active_processes.last, wire, next_active, 0);
            }
          }
        } else if ((env->is_active || context->hot_process.process == env->p) &&
                   selection.type == Process_Selection_NewWire) {
          // begin new-wire
          Set_Flag(context->flags, Context_Flag_NewWire);
          if (!env->is_active) {
            clear_active_processes(&context->what_is_this, &view->and_whats_this.active_processes);
            SLLQueuePush_NZ(view->and_whats_this.active_processes.first, view->and_whats_this.active_processes.last, env->p, next_active, 0);
          }
        } else if (selection.type == Process_Selection_Process) {
          if (Get_Flag(context->flags, Context_Flag_NewWire) &&
              !Get_Flag(env->p->flags, Process_Flag_Wire)) {
            // connect processes
            WhatIsThis wit = (WhatIsThis){0}; // TODO: handle wits
            connect_processes(&wit, view->and_whats_this.active_processes.first, env->p);
            exit_add_wire_mode(context, view);
          } else {
            // select process
            if (!env->is_active) {
              clear_active_processes(&context->what_is_this, &view->and_whats_this.active_processes);
              SLLQueuePush_NZ(view->and_whats_this.active_processes.first, view->and_whats_this.active_processes.last, env->p, next_active, 0);
            }
            Unset_Flag(context->flags, Context_Flag_NewWire);
            Set_Flag(context->flags, Context_Flag_Dragging);
            context->ui_state.active_position = context->ui_state.mouse_position;
          }
        }
      }
    }
    else if (kb_res == Keybind_Result_Exit) {
      // stop dragging proc
      if (Get_Flag(env->context->flags, Context_Flag_Dragging)) {
        if (context->ui_state.mouse_moved) {
          // update positions of active processes
          B32 updated = 0;
          for (Process *a = view->and_whats_this.active_processes.first; a != 0; a = a->next_active) {
            if (Get_Flag(a->flags, Process_Flag_Wire)) {
              Camera2D *camera = &env->view->camera;
              Vector2 mouse_world_position = GetScreenToWorld2D(context->ui_state.mouse_position, *camera);
              Editable_Process new_a = get_editable_process(env->view->and_whats_this.do_undo.edit_list, a);
              {
                // TODO: don't just update the first inner-position
                if (new_a.process.inner_positions == 0) {
                  new_a.process.inner_positions = create_v2_chunk(context);
                }
                if (new_a.process.inner_positions) {
                  new_a.process.inner_positions->e[0] = mouse_world_position;
                  new_a.process.inner_positions->count = 1;
                }
              }
              add_process_to_process_edit_list(&context->what_is_this, a, Proc_Trie_Edit_Update, new_a.process);
              updated = 1;
            }
            else {
              Vector2 new_position = get_process_position(context, view, a);
              Editable_Process new_a = get_editable_process(view->and_whats_this.do_undo.edit_list, a);
              new_a.process.position = new_position;
              add_process_to_process_edit_list(&context->what_is_this, a, Proc_Trie_Edit_Update, new_a.process);
              updated = 1;
            }
          }
          if (updated) {
            gather_processes_from_trie(&context->what_is_this);
          }
        }
        Unset_Flag(env->context->flags, Context_Flag_Dragging);
      }

      // stop dragging wire
      B32 wire_drag_flag = Process_Flag_Drag_In | Process_Flag_Drag_Out;
      if (Get_Flag(env->p->flags, wire_drag_flag)) {
        B32 is_in = Get_Flag(env->p->flags, Process_Flag_Drag_In);
        Unset_Flag(env->p->flags, wire_drag_flag);
        env->moved_wire = env->p;
        env->moved_wire_conn = is_in ? Process_Connection_In : Process_Connection_Out;
      }
    }
  }

  return handled;
}









Define_Keybind_And_Action(
  SelectAnotherProcess, Default,
  Keybind_Behavior_Alternate, ForAllProcesses,
  Key_Kind_Mouse0, Modifier_Key_Control,
  Ui_Constraint_HoverProcess, View_Kind_Flag_Procs,
  "Add a process to the selected processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  View *view = env->view;
  Process_Selection selection = env->selection;

  if (context && view) {
    if (Test_Keybind(env, Enter)) {
      handled = 1;
      if (selection.type == Process_Selection_In || selection.type == Process_Selection_Out) {
        Process *wire = get_wire_from_selection(context, view, selection);
        if (wire) {
          if (is_active_process(context, view, wire)) {
            remove_process_from_active_processes(context, view, wire);
          } else {
            SLLQueuePush_NZ(view->and_whats_this.active_processes.first, view->and_whats_this.active_processes.last, wire, next_active, 0);
          }
        }
      } else if (selection.type == Process_Selection_Process) {
        if (is_active_process(context, view, selection.process)) {
          remove_process_from_active_processes(context, view, selection.process);
        } else {
          SLLQueuePush_NZ(view->and_whats_this.active_processes.first, view->and_whats_this.active_processes.last, selection.process, next_active, 0);
        }
      }
    }
  }

  return handled;
}



Define_Keybind_And_Action(
  CancelSelection, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  Key_Kind_Mouse0, 0,
  Ui_Constraint_NoHotProcess, View_Kind_Flag_Procs,
  "Clear out the selected processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  View *view = env->view;

  if (context && view) {
    if (check_keybind(env)) {
      handled = 1;
      exit_add_wire_mode(context, view);
    }
  }

  return handled;
}

Define_Keybind_Order(CancelSelection_Default, After, ForAllProcessInteractions_Default);


Define_Keybind_And_Action(
  CreateProcess, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  Key_Kind_Mouse0, Modifier_Key_Control,
  Ui_Constraint_NoHotProcess, View_Kind_Flag_Procs,
  "Create a new process."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  View *view = env->view;

  if (context && view) {
    if (check_keybind(env)) {
      handled = 1;

      // TODO: do bounds check to see if we should add process
      if (Get_Flag(view->flags, View_Flag_Active) &&
          Get_Flag(view->flags, View_Flag_Editable)) {
        Process *new_p = create_process(&context->what_is_this);
        if (new_p) {
          Set_Flag(new_p->flags, Process_Flag_TextEdit);
          new_p->position = GetScreenToWorld2D(context->ui_state.mouse_position, view->camera);
          clear_active_processes(&context->what_is_this, &view->and_whats_this.active_processes);
          SLLQueuePush_NZ(view->and_whats_this.active_processes.first, view->and_whats_this.active_processes.last, new_p, next_active, 0);
        }
      }

      gather_processes_from_trie(&context->what_is_this);
    }
  }

  return handled;
}

Define_Keybind_Order(CreateProcess_Default, Before, ForAllProcessInteractions_Default);


Define_Keybind_And_Action(
  DeleteProcess, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  KEY_D, Modifier_Key_Control, 0, View_Kind_Flag_Procs,
  "Delete the selected processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  View *view = env->view;

  if (context && view) {
    if (check_keybind(env)) {
      handled = 1;
      // delete processes
      for (Process *a = view->and_whats_this.active_processes.first; a != 0;) {
        Process *next_active = a->next_active;
        WhatIsThis wit = (WhatIsThis){0}; // TODO: handle passing/returning wits
        delete_process(&wit, a, 0);
        a = next_active;
      }
      gather_processes_from_trie(&context->what_is_this);

      clear_active_process_list(&context->what_is_this, &view->and_whats_this.active_processes);
    }
  }

  return handled;
}

Define_Keybind_Order(DeleteProcess_Default, Before, ForAllProcessInteractions_Default);



Define_Keybind_And_Action(
  CycleProcessDisplay, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  KEY_TAB, 0, 0, View_Kind_Flag_Procs,
  "Cycle through special displays for selected processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  View *view = env->view;

  if (context && view) {
    if (check_keybind(env)) {
      handled = 1;
      // cycle through special process types (cups/caps/empty)
      for (Process *a = view->and_whats_this.active_processes.first; a != 0; a = a->next_active) {
        if (!Get_Flag(a->flags, Process_Flag_Wire)) {
          U32 toggle_flags = (Process_Flag_Empty | Process_Flag_Cup | Process_Flag_Cap | Process_Flag_Identity);
          if (Get_Flag(a->flags, toggle_flags)) {
            // toggle off process-display flags first, before trying to toggle them on
            Unset_Flag(a->flags, toggle_flags);
          } else if ((a->in_count == 0 && a->out_count == 0) ||
                     (a->in_count == 1 && a->out_count == 0) ||
                     (a->in_count == 0 && a->out_count == 1)) {
            // toggle single in/out or unconnected process
            Toggle_Flag(a->flags, Process_Flag_Empty);
          } else if (a->in_count == 0 && a->out_count == 2) {
            Toggle_Flag(a->flags, Process_Flag_Cup);
          } else if (a->in_count == 2 && a->out_count == 0) {
            Toggle_Flag(a->flags, Process_Flag_Cap);
          } else if (a->in_count == 1 && a->out_count == 1) {
            Toggle_Flag(a->flags, Process_Flag_Identity);
          }
        }
      }
    }
  }

  return handled;
}

Define_Keybind_Order(CycleProcessDisplay_Default, Before, ForAllProcessInteractions_Default);


Define_Keybind_And_Action(
  ToggleDisplayMode, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  KEY_M, Modifier_Key_Control, 0, View_Kind_Flag_Procs,
  "Toggle between 'classic' and 'rounded' display modes."
  ) {
  B32 handler = 0;
  Context *context = env->context;

  if (check_keybind(env) == Keybind_Result_Enter) {
    handler = 1;
    Toggle_Flag(context->flags, Context_Flag_RoundedShapes);
  }

  return handler;
}

Define_Keybind_Order(ToggleDisplayMode_Default, Before, ForAllProcessInteractions_Default);



Define_Keybind_And_Action(
  CopyProcess, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  KEY_C, Modifier_Key_Control, 0, View_Kind_Flag_Procs,
  "Copy selected processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  View *view = env->view;

  if (context && view)
  if (check_keybind(env) == Keybind_Result_Enter) {
    handled = 1;
    copy_active_processes(context, view);
  }

  return handled;
}

Define_Keybind_Order(CopyProcess_Default, Before, ForAllProcessInteractions_Default);


Define_Keybind_And_Action(
  PasteProcess, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  KEY_V, Modifier_Key_Control, 0, View_Kind_Flag_Procs,
  "Paste copied processes, centered at the mouse."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  View *view = env->view;

  if (context && view) {
    if (check_keybind(env) == Keybind_Result_Enter) {
      handled = 1;
      paste_processes(context, view);
    }
  }

  return handled;
}

Define_Keybind_Order(PasteProcess_Default, Before, ForAllProcessInteractions_Default);



Define_Keybind_And_Action(
  Undo, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  KEY_Z, Modifier_Key_Control, 0, View_Kind_Flag_Procs,
  "Performs undo on the proc-trie."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  View *view = env->view;

  if (context && view) {
    if (check_keybind(env) == Keybind_Result_Enter) {
      handled = 1;
      proc_trie_undo(view->and_whats_this.do_undo.trie);
      gather_processes_from_trie(&context->what_is_this);
    }
  }

  return handled;
}

Define_Keybind_Order(Undo_Default, Before, ForAllProcessInteractions_Default);


Define_Keybind_And_Action(
  Redo, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  KEY_Z, Modifier_Key_Control|Modifier_Key_Shift, 0, View_Kind_Flag_Procs,
  "Performs redo on the proc-trie."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  View *view = env->view;

  if (context && view) {
    if (check_keybind(env) == Keybind_Result_Enter) {
      handled = 1;
      proc_trie_redo(view->and_whats_this.do_undo.trie);
      gather_processes_from_trie(&context->what_is_this);
    }
  }

  return handled;
}

Define_Keybind_Order(Redo_Default, Before, ForAllProcessInteractions_Default);






function S32 process_compare_pos_x(void *a, void *b, void *udata) {
  S32 result = 0;

  if (a && b) {
    Process *a_proc = *(Process **)a;
    Process *b_proc = *(Process **)b;
    result = a_proc->position.x - b_proc->position.x;
  }

  return result;
}

////////////////////////////////////////
// Process-Connection By Clicking
////////////////////////////////////////
Define_Keybind(
  ProcessConnectionByModClick, Standard,
  Keybind_Behavior_Alternate, OnlyOnce,
  Key_Kind_Mouse0, Modifier_Key_Super,
  (Ui_Constraint_ActionNotOccured |
   Ui_Constraint_HotProcess |
   Ui_Constraint_ActiveProcesses),
  View_Kind_Flag_Procs);

Define_Keybind_Order(ProcessConnectionByModClick_Standard, Before, ForAllProcessInteractions_Default);

Define_Keybind_Action(
  ProcessConnectionByModClick,
  "Connect clicked process to all active processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  View *view = env->view;

  if (context && view) {
    if (Test_Keybind(env, Enter)) {
      handled = 1;

      Assert(view->and_whats_this.active_processes.first);
      Assert(context->hot_process.process);

      // @Speed
      U32 active_count = 0;
      Process **sorted_processes = 0;
      {
        for (Process *a = view->and_whats_this.active_processes.first; a != 0; a = a->next_active) {
          if (!Get_Flag(a->flags, Process_Flag_Wire)) {
            active_count += 1;
          }
        }
        sorted_processes = arena_push(context->what_is_this.per_frame_arena, active_count*sizeof(Process *));
        U32 i = 0;
        for (Process *a = view->and_whats_this.active_processes.first; a != 0; a = a->next_active) {
          if (!Get_Flag(a->flags, Process_Flag_Wire)) {
            sorted_processes[i] = a;
            i += 1;
          }
        }
        sort_merge(sorted_processes, sizeof(Process *), active_count, process_compare_pos_x, 0);
      }

      if (sorted_processes) {
        Connection_Result conn_res = (Connection_Result){0};
        conn_res.in = context->hot_process.process;
        for (U32 i = 0; i < active_count; ++i) {
          B32 out_is_not_wire = !Get_Flag(sorted_processes[i]->flags, Process_Flag_Wire);
          B32 in_is_not_wire = !Get_Flag(conn_res.in->flags, Process_Flag_Wire);
          if (out_is_not_wire && in_is_not_wire) {
            WhatIsThis wit = (WhatIsThis){0}; // TODO: handle passing/returning wits
            conn_res = connect_processes_no_gather(&wit, sorted_processes[i], conn_res.in);
          }
        }
      }

      // TODO: ensure that procs are from main-procs, or allow connected procs from ui or other places???
      gather_processes_from_trie(&context->what_is_this);
    }
  }

  return handled;
}







Define_Keybind_And_Action(
  SelectRootFromUndoTrie, Standard,
  Keybind_Behavior_Alternate, ForAllProcesses,
  Key_Kind_Mouse0, 0,
  Ui_Constraint_HotProcess|Ui_Constraint_ExitOnKeyup|Ui_Constraint_ActionNotOccured, View_Kind_Flag_Procs,
  "Select root from undo trie."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  View *view = env->view;
  Process_Selection selection = env->selection;

  if (context && view) {
    if (Test_Keybind(env, Enter)) {
      if (selection.view == view) {
        if (selection.type == Process_Selection_Process) {
          if (selection.process->ref) {
            handled = 1;

            Process_Do_Undo *do_undo = &view->and_whats_this.do_undo;
            if (do_undo->trie) {
              do_undo->trie->current_root = selection.process->ref;
              // TODO: if we ever display undo trie from other than main procs, we need to switch on that here......
              gather_processes_from_trie(&context->what_is_this);
            }
          }
        }
      }
    }
  }

  return handled;
}
