#include "../source/keybind.h"




static keybind_action_DECL(CycleProcessDisplay);
static keybind_action_DECL(DeleteProcess);

Define_Keybind(
  HandleActiveProcess, ,
  Keybind_Behavior_Alternate, 0, AtTheStart,
  0, 0, 0,
  "handle active-process"
  ) {
  if (env->context) {
    if (env->context->active_processes.first) {
      B32 is_dragging = Get_Flag(env->context->flags, Context_Flag_Dragging);
      if (is_dragging && env->should_stop_dragging) {
        // update positions of active processes
        for (Process *a = env->context->active_processes.first; a != 0; a = a->next_active) {
          Vector2 new_position = get_process_position(env->context, &env->context->views[View_Kind_Procs], a);
          a->position = new_position;
        }
        // stop dragging
        Unset_Flag(env->context->flags, Context_Flag_Dragging);
      }
      else if (Keybind_Handle(env, CycleProcessDisplay)) {
        // handled
      }
      else if (Keybind_Handle(env, DeleteProcess)) {
        // handled
      }
      else if (!Get_Flag(env->context->ui_state.flags, Ui_State_Flag_action_occured)) {
        // process label editing
        handle_label_editing(env->context, env->context->active_processes);
      }
    }
  }

  return 0;
}





Define_Keybind(
  Bound, ,
  Keybind_Behavior_Alternate, 0, AtTheStart,
  Key_Kind_Mouse0, 0,
  Ui_Constraint_NoHotProcess|Ui_Constraint_ExitOnKeyup,
  "Select multiple processes by drawing a rectangle with your mouse."
  ) {
  B32 handled = 0;
  Keybind_Result kb_res = Check_Keybind(env, Bound);
  Context *context = env->context;
  Process_Selection selection = env->selection;

  if (env->desired_kb_res == Keybind_Result_Exit) {
    if (Get_Flag(context->flags, Context_Flag_Bounding)) {
      // exit
      if (kb_res == Keybind_Result_Exit) {
        Unset_Flag(context->flags, Context_Flag_Bounding);
        handled = 1;
      }
      else {
        clear_active_processes(context);
      }
    }
  }
  else if (env->desired_kb_res == Keybind_Result_Enter) {
    // enter
    if (kb_res == Keybind_Result_Enter) {
      Set_Flag(context->flags, Context_Flag_Bounding);
      context->active_position = context->ui_state.mouse_position;
      handled = 1;
    }
  }

  return handled;
}





Define_Keybind(
  PerProcessBounding, ,
  Keybind_Behavior_Alternate, 0, ForAllProcesses,
  0, 0, 0,
  "Per-process bounding."
  ) {
  if (env->context && env->p) {
    if (Get_Flag(env->context->flags, Context_Flag_Bounding)) {
      Rectangle selection_rectangle = get_selection_rectangle(env->context);

      if (Get_Flag(env->p->flags, Process_Flag_Wire)) {
        Process_Shape out_shape = get_process_shape(env->context, env->view, env->p->out);
        Process_Shape in_shape = get_process_shape(env->context, env->view, env->p->in);
        Vector2 out_position = get_process_wire_position(env->context, env->view, env->p->out, out_shape, Process_Connection_Out, env->p->which_out);
        Vector2 in_position = get_process_wire_position(env->context, env->view, env->p->in, in_shape, Process_Connection_In, env->p->which_in);

        if (rectangle_contains_point(selection_rectangle, out_position) ||
            rectangle_contains_point(selection_rectangle, in_position)) {
          SLLQueuePush_NZ(env->context->active_processes.first, env->context->active_processes.last, env->p, next_active, 0);
        }
      } else {
        Process_Shape shape = get_process_shape(env->context, env->view, env->p);

        if (rectangle_contains_point(selection_rectangle, shape.center)) {
          SLLQueuePush_NZ(env->context->active_processes.first, env->context->active_processes.last, env->p, next_active, 0);
        }
      }
    }
  }

  return 0;
}



Define_Keybind(
  ZeroOutSelection, ,
  Keybind_Behavior_Alternate, 0, _Null, 0, 0, 0,
  ""
  ) {
  if (env->context) {
    // zero out selection
    env->selection = (Process_Selection){0};
    // zero the old hot-id
    if (!Get_Flag(env->context->ui_state.flags, Ui_State_Flag_hot_id_assigned)) {
      env->context->hot_process = 0;
    }
  }

  return 0;
}



Define_Keybind(
  MoreRectangleSelectionHandling, ,
  Keybind_Behavior_Alternate, 0, _Null, 0, 0, 0,
  "more rectangle selection handling"
  ) {
  if (env->context) {
    if (Get_Flag(env->context->flags, Context_Flag_Bounding)) {
      // add hot process to active processes
      if (env->context->hot_process) {
        B32 hot_is_active = is_active_process(env->context, env->context->hot_process);
        if (!hot_is_active) {
          SLLQueuePush_NZ(env->context->active_processes.first, env->context->active_processes.last, env->context->hot_process, next_active, 0);
        }
      }
    }
  }

  return 0;
}




Define_Keybind(
  HandleMovedWire, ,
  Keybind_Behavior_Alternate, 0, _Null, 0, 0, 0,
  "handle moved wire"
  ) {
  if (env->moved_wire && env->context) {
    printf("handle moved wire\n");
    if (env->context->hot_process) {
      if (Get_Flag(env->context->hot_process->flags, Process_Flag_Wire)) {
        Process *connected_process = env->context->hot_process->conn[env->moved_wire_conn];
        if (connected_process) {
          // move wire to hovered wire
          U32 which_conn = env->context->hot_process->which_conn[env->moved_wire_conn];
          if (env->moved_wire != env->context->hot_process) {
            remove_wire_connection(env->context, env->moved_wire, (1<<env->moved_wire_conn));
            add_wire_connection(env->context, env->moved_wire, connected_process, env->moved_wire_conn, which_conn);
          }
        }
      } else {
        Process *connected_process = env->context->hot_process;
        // move wire to last wire of process
        U32 which_conn;
        if (env->moved_wire->conn[env->moved_wire_conn] == connected_process) {
          which_conn = connected_process->conn_count[env->moved_wire_conn] - 1;
        } else {
          which_conn = connected_process->conn_count[env->moved_wire_conn];
        }
        remove_wire_connection(env->context, env->moved_wire, (1<<env->moved_wire_conn));
        add_wire_connection(env->context, env->moved_wire, connected_process, env->moved_wire_conn, which_conn);
      }
    }
  }

  return 0;
}






Define_Keybind(
  Pan, ,
  Keybind_Behavior_Alternate, 0, _Null,
  Key_Kind_Mouse1, 0,
  Ui_Constraint_ExitOnKeyup,
  "Slide your field of view by moving your mouse."
  ) {
  B32 handled = 1;
  Context *context = env->context;
  Process_Selection selection = env->selection;
  Assert(env->desired_kb_res == 0);

  Keybind_Result kb_res = Check_Keybind(env, Pan);

  for (S32 v = 0; v < View_Count; ++v) {
    View *view = context->views + v;
    B32 mouse_is_within_view = 1;

    if (Get_Flag(view->flags, View_Flag_Active) && mouse_is_within_view) {
      if (kb_res == Keybind_Result_Enter) {
        Set_Flag(view->flags, View_Flag_Panning);
        context->active_position = context->ui_state.mouse_position;
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





Define_Keybind(
  ZoomIn, ,
  Keybind_Behavior_Alternate, 0, _Null,
  Key_Kind_MouseWheelUp, 0,
  Ui_Constraint_ActionNotOccured,
  "Zoom your field of view in to make objects appear closer.") {
  Context *context = env->context;
  Process_Selection selection = env->selection;

  return keybind_zoom_handler(env);
}

Define_Keybind(
  ZoomOut, ,
  Keybind_Behavior_Alternate, 0, _Null,
  Key_Kind_MouseWheelDown, 0,
  Ui_Constraint_ActionNotOccured,
  "Zoom your field of view out to make objects appear further.") {
  Context *context = env->context;
  Process_Selection selection = env->selection;

  return keybind_zoom_handler(env);
}


static keybind_action_DECL(CheckIfWeNeedToStopDraggingTheWire);
static keybind_action_DECL(SelectSingleProcess);
static keybind_action_DECL(MaybeSetHotProcessDesiredKbResStack);

Define_Keybind(
  ForAllProcessInteractions, ,
  Keybind_Behavior_Alternate, 0, _Null, 0, 0, 0,
  "Loop through all processes and handle per-process interactions."
  ) {
  if (env->context) {
    for (S32 v = 0; v < View_Count; ++v) {
      View *view = env->context->views + v;
      if (Get_Flag(view->flags, View_Flag_Active)) {
        for (Process *p = view->processes.first; p != 0; p = p->next) {
          // per-process environment
          env->selection = get_process_selection(env->context, view, p);
          env->view = view;
          env->is_active = is_active_process(env->context, p);
          env->p = p;

          // hot id assignment
          B32 ui_state_hot_id_assigned = Get_Flag(env->context->ui_state.flags, Ui_State_Flag_hot_id_assigned);
          B32 hot_id_assigned = env->selection.hot_id_assigned || ui_state_hot_id_assigned;
          Assign_Flag(env->context->ui_state.flags, Ui_State_Flag_hot_id_assigned, hot_id_assigned);

          // per-process keybinds
          for (U32 i = 0; i < SymbolCount(keybind_action); ++i) {
            Keybind *keybind = SymbolMetadataFromID(keybind_action, i+1);

            if (Get_Flag(keybind->timing, Keybind_Timing_ForAllProcesses)) {
              keybind->handle(env);
            }
          }
        }
      }
    }
  }

  return 0;
}




Define_Keybind(
  CheckIfWeNeedToStopDraggingTheWire, ,
  Keybind_Behavior_Alternate, 0, ForAllProcesses, 0, 0, 0,
  ""
  ) {
  B32 handled = 0;

  if (env->p) {
    if (env->should_stop_dragging) {
      // unset drag flag
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


Define_Keybind(
  SelectSingleProcess, ,
  Keybind_Behavior_Alternate, 0, ForAllProcesses,
  Key_Kind_Mouse0, 0,
  Ui_Constraint_HotProcess|Ui_Constraint_ExitOnKeyup|Ui_Constraint_ActionNotOccured,
  "Select a single process."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  Process_Selection selection = env->selection;

  if (check_keybind(context, keybind_action_REF(SelectSingleProcess), selection) == Keybind_Result_Enter) {
    handled = 1;
    B32 in_selection = selection.type == Process_Selection_In;
    B32 out_selection = selection.type == Process_Selection_Out;
    if (in_selection || out_selection) {
      // select wire
      Process *wire = get_process_wire_by_selection(context, selection);
      B32 is_active_wire = is_active_process(context, wire);

      if (wire) {
        U32 drag_flag = in_selection ? Process_Flag_Drag_In : Process_Flag_Drag_Out;
        Unset_Flag(context->flags, Context_Flag_NewWire);
        Set_Flag(wire->flags, drag_flag);
        context->active_position = context->ui_state.mouse_position;
        if (!is_active_wire) {
          clear_active_processes(context);
          SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, wire, next_active, 0);
        }
      }
    } else if ((env->is_active || context->hot_process == env->p) &&
               selection.type == Process_Selection_NewWire) {
      // begin new-wire
      Set_Flag(context->flags, Context_Flag_NewWire);
      if (!env->is_active) {
        clear_active_processes(context);
        SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, env->p, next_active, 0);
      }
    } else if (selection.type == Process_Selection_Process) {
      if (Get_Flag(context->flags, Context_Flag_NewWire)) {
        // connect processes
        connect_processes(context, context->active_processes.first, env->p);
        exit_add_wire_mode(context);
      } else {
        // select process
        context->hot_process = env->p;
        if (!env->is_active) {
          clear_active_processes(context);
          SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, env->p, next_active, 0);
        }
        Unset_Flag(context->flags, Context_Flag_NewWire);
        Set_Flag(context->flags, Context_Flag_Dragging);
        context->active_position = context->ui_state.mouse_position;
      }
    }
  }

  return handled;
}




static keybind_action_DECL(SelectAnotherProcess);

Define_Keybind(
  MaybeSetHotProcess, ,
  Keybind_Behavior_Alternate, 0, _Null, 0, 0, 0,
  "Maybe select another process, or maybe set the context's hot-process to the keybind-environment's process."
  ) {
  if (Keybind_Handle(env, SelectAnotherProcess)) {
    // handled
  } else if (env->selection.type == Process_Selection_Process) {
    // process hover
    if (env->context) {
      env->context->hot_process = env->p;
    }
  }

  return 0;
}





Define_Keybind(
  SelectAnotherProcess, ,
  Keybind_Behavior_Alternate, 0, _Null,
  Key_Kind_Mouse0, Modifier_Key_Control,
  Ui_Constraint_HoverProcess,
  "Add a process to the selected processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  Process_Selection selection = env->selection;

  if (env->desired_kb_res == check_keybind(context, keybind_action_REF(SelectAnotherProcess), selection)) {
    handled = 1;
    if (selection.type == Process_Selection_In || selection.type == Process_Selection_Out) {
      Process *wire = get_process_wire_by_selection(context, selection);
      if (wire) {
        if (is_active_process(context, wire)) {
          remove_process_from_active_processes(context, wire);
        } else {
          SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, wire, next_active, 0);
        }
      }
    } else if (selection.type == Process_Selection_Process) {
      if (is_active_process(context, selection.process)) {
        remove_process_from_active_processes(context, selection.process);
      } else {
        SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, selection.process, next_active, 0);
      }
    }
  }

  return handled;
}



Define_Keybind(
  CancelSelection, ,
  Keybind_Behavior_Alternate, 0, _Null,
  Key_Kind_Mouse0, 0,
  Ui_Constraint_NoHotProcess,
  "Clear out the selected processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  Process_Selection selection = env->selection;


  if (check_keybind(context, keybind_action_REF(CancelSelection), selection)) {
    handled = 1;
    exit_add_wire_mode(context);
  }

  return handled;
}



Define_Keybind(
  CreateProcess, ,
  Keybind_Behavior_Alternate, 0, _Null,
  Key_Kind_Mouse0, Modifier_Key_Control,
  Ui_Constraint_NoHotProcess,
  "Create a new process."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  Process_Selection selection = env->selection;

  if (check_keybind(context, keybind_action_REF(CreateProcess), selection)) {
    handled = 1;
    View *view = context->views + View_Kind_Procs;

    // TODO: do bounds check to see if we should add process
    if (Get_Flag(view->flags, View_Flag_Active) &&
        Get_Flag(view->flags, View_Flag_Editable)) {
      Process *new_p = create_process(context);
      if (new_p) {
        Set_Flag(new_p->flags, Process_Flag_TextEdit);
        new_p->position = GetScreenToWorld2D(context->ui_state.mouse_position, view->camera);
        clear_active_processes(context);
        SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, new_p, next_active, 0);
      }
    }

    gather_processes_from_trie(context);
  }

  return handled;
}



Define_Keybind(
  DeleteProcess, ,
  Keybind_Behavior_Alternate, 0, _Null,
  KEY_D, Modifier_Key_Control, 0,
  "Delete the selected processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  Process_Selection selection = env->selection;


  if (check_keybind(context, keybind_action_REF(DeleteProcess), selection)) {
    handled = 1;
    // delete processes
    for (Process *a = context->active_processes.first; a != 0;) {
      Process *next_active = a->next_active;
      delete_process(context, a);
      a = next_active;
    }
    clear_active_processes(context);
  }

  return handled;
}



Define_Keybind(
  CycleProcessDisplay, ,
  Keybind_Behavior_Alternate, 0, _Null,
  KEY_TAB, 0, 0,
  "Cycle through special displays for selected processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  Process_Selection selection = env->selection;


  if (check_keybind(context, keybind_action_REF(CycleProcessDisplay), selection)) {
    handled = 1;
    // cycle through special process types (cups/caps/empty)
    for (Process *a = context->active_processes.first; a != 0; a = a->next_active) {
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

  return handled;
}



Define_Keybind(
  ToggleDisplayMode, ,
  Keybind_Behavior_Alternate, 0, _Null,
  KEY_M, Modifier_Key_Control, 0,
  "Toggle between 'classic' and 'rounded' display modes."
  ) {
  B32 handler = 0;
  Context *context = env->context;
  Process_Selection selection = env->selection;


  if (check_keybind(context, keybind_action_REF(ToggleDisplayMode), selection) == Keybind_Result_Enter) {
    handler = 1;
    Toggle_Flag(context->flags, Context_Flag_RoundedShapes);
  }

  return handler;
}




Define_Keybind(
  CopyProcess, ,
  Keybind_Behavior_Alternate, 0, _Null,
  KEY_C, Modifier_Key_Control, 0,
  "Copy selected processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  Process_Selection selection = env->selection;


  if (check_keybind(context, keybind_action_REF(CopyProcess), selection) == Keybind_Result_Enter) {
    handled = 1;
    copy_active_processes(context);
  }

  return handled;
}



Define_Keybind(
  PasteProcess, ,
  Keybind_Behavior_Alternate, 0, _Null,
  KEY_V, Modifier_Key_Control, 0,
  "Paste copied processes, centered at the mouse."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  Process_Selection selection = env->selection;


  if (check_keybind(context, keybind_action_REF(PasteProcess), selection) == Keybind_Result_Enter) {
    handled = 1;
    paste_processes(context);
  }

  return handled;
}




Define_Keybind(
  Undo, ,
  Keybind_Behavior_Alternate, 0, _Null,
  KEY_Z, Modifier_Key_Control, 0,
  "Performs undo on the proc-trie."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  Process_Selection selection = env->selection;


  if (check_keybind(context, keybind_action_REF(Undo), selection) == Keybind_Result_Enter) {
    handled = 1;
    proc_trie_undo(context->proc_trie);
    gather_processes_from_trie(context);
  }

  return handled;
}



Define_Keybind(
  Redo, ,
  Keybind_Behavior_Alternate, 0, _Null,
  KEY_Z, Modifier_Key_Control|Modifier_Key_Shift, 0,
  "Performs redo on the proc-trie."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  Process_Selection selection = env->selection;


  if (check_keybind(context, keybind_action_REF(Redo), selection) == Keybind_Result_Enter) {
    handled = 1;
    proc_trie_redo(context->proc_trie);
    gather_processes_from_trie(context);
  }

  return handled;
}









///////////////////////////
// Stack Handling Funcs (Hacky stuff at the moment...)
///////////////////////////
// TODO: Find a way to merge some of these stack-related keybind-definitions together.
Define_Keybind(
  BoundDesiredKbResStack_Enter, _HACKY,
  Keybind_Behavior_Alternate, 0, _Null, 0, 0, 0,
  ""
  ) {
  env->old_kb_res = env->desired_kb_res;
  env->desired_kb_res = Keybind_Result_Enter;
  {
    Keybind_Handle(env, Bound);
  }
  env->desired_kb_res = env->old_kb_res;
  return 0;
}
Define_Keybind(
  BoundDesiredKbResStack_Exit, _HACKY,
  Keybind_Behavior_Alternate, 0, _Null, 0, 0, 0,
  ""
  ) {
  env->old_kb_res = env->desired_kb_res;
  env->desired_kb_res = Keybind_Result_Exit;
  {
    Keybind_Handle(env, Bound);
  }
  env->desired_kb_res = env->old_kb_res;
  return 0;
}

Define_Keybind(
  MaybeSetHotProcessDesiredKbResStack, _HACKY,
  Keybind_Behavior_Alternate, 0, ForAllProcesses, 0, 0,
  Ui_Constraint_ActionNotOccured,
  ""
  ) {
  env->old_kb_res = env->desired_kb_res;
  env->desired_kb_res = Keybind_Result_Enter;
  {
    Keybind_Handle(env, MaybeSetHotProcess);
  }
  env->desired_kb_res = env->old_kb_res;
  return 0;
}
