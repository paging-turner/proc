#include "../source/keybind.h"






Define_Keybind_And_Action(
  HandleActiveProcess, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  0, 0, 0,
  "handle active-process"
  ) {
  if (env->context) {
    if (env->context->active_processes.first) {
      if (!Get_Flag(env->context->ui_state.flags, Ui_State_Flag_action_occured)) {
        // process label editing
        handle_label_editing(env->context, env->context->active_processes);
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
  Ui_Constraint_NoHotProcess|Ui_Constraint_ExitOnKeyup,
  "Select multiple processes by drawing a rectangle with your mouse."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  Process_Selection selection = env->selection;
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

  return handled;
}

Define_Keybind_Order(Bound_Default, Before, ForAllProcessInteractions_Default);




Define_Keybind_And_Action(
  PerProcessBounding, Default,
  Keybind_Behavior_Alternate, ForAllProcesses,
  0, 0, 0,
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





Define_Keybind_And_Action(
  ZeroOutSelection, Default,
  Keybind_Behavior_Alternate, OnlyOnce, 0, 0, 0,
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
  Keybind_Behavior_Alternate, OnlyOnce, 0, 0, 0,
  "handle moved wire"
  ) {
  if (env->moved_wire && env->context) {
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
              add_wire_connection(env->context, env->moved_wire, connected_process, env->moved_wire_conn, which_conn);
              gather_processes_from_trie(env->context);
            }
          }
        }
      } else {
        Process *connected_process = hot_process;
        // move wire to last wire of process
        U32 which_conn = connected_process->conn_count[env->moved_wire_conn];
        if (env->moved_wire->conn[env->moved_wire_conn] != connected_process ||
            env->moved_wire->which_conn[env->moved_wire_conn] != which_conn) {
          add_wire_connection(env->context, env->moved_wire, connected_process, env->moved_wire_conn, which_conn);
          gather_processes_from_trie(env->context);
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
  Ui_Constraint_ExitOnKeyup,
  "Slide your field of view by moving your mouse."
  ) {
  B32 handled = 1;
  Context *context = env->context;

  Keybind_Result kb_res = Check_Keybind(env);

  for (S32 v = 0; v < View_Count; ++v) {
    View *view = context->views + v;
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

  if (Test_Keybind(env, Enter)) {
    for (S32 v = 0; v < View_Count; ++v) {
      View *view = context->views + v;
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
  Ui_Constraint_ActionNotOccured);


Define_Keybind(
  Zoom, DefaultOut,
  Keybind_Behavior_Alternate, OnlyOnce,
  Key_Kind_MouseWheelDown, 0,
  Ui_Constraint_ActionNotOccured);

Define_Keybind_Order(Zoom_DefaultIn, Before, ForAllProcessInteractions_Default);
Define_Keybind_Order(Zoom_DefaultOut, Before, ForAllProcessInteractions_Default);


Define_Keybind_And_Action(
  ForAllProcessInteractions, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  0, 0, 0,
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
          for (U32 i = 0; i < env->context->keybind_count; ++i) {
            Keybind *keybind = env->context->keybinds + i;
            env->keybind = keybind;

            if (keybind->for_all_processes) {
              keybind->handle(env);
            }
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
  Ui_Constraint_HotProcess|Ui_Constraint_ExitOnKeyup|Ui_Constraint_ActionNotOccured,
  "Select a single process."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  Process_Selection selection = env->selection;
  Keybind_Result kb_res = Check_Keybind(env);

  if (env->p == 0) {
    return 0;
  }

  if (kb_res == Keybind_Result_Enter) {
    if (selection.view == context->views + View_Kind_Procs) {
      handled = 1;
      B32 in_selection = selection.type == Process_Selection_In;
      B32 out_selection = selection.type == Process_Selection_Out;
      if (in_selection || out_selection) {
        // select wire
        Process *wire = get_wire_from_selection(context, selection);
        B32 is_active_wire = is_active_process(context, wire);

        if (wire) {
          U32 drag_flag = in_selection ? Process_Flag_Drag_In : Process_Flag_Drag_Out;
          Unset_Flag(context->flags, Context_Flag_NewWire);
          Set_Flag(wire->flags, drag_flag);
          context->ui_state.active_position = context->ui_state.mouse_position;
          if (!is_active_wire) {
            clear_active_processes(context);
            SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, wire, next_active, 0);
          }
        }
      } else if ((env->is_active || context->hot_process.process == env->p) &&
                 selection.type == Process_Selection_NewWire) {
        // begin new-wire
        Set_Flag(context->flags, Context_Flag_NewWire);
        if (!env->is_active) {
          clear_active_processes(context);
          SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, env->p, next_active, 0);
        }
      } else if (selection.type == Process_Selection_Process) {
        if (Get_Flag(context->flags, Context_Flag_NewWire) &&
            !Get_Flag(env->p->flags, Process_Flag_Wire)) {
          // connect processes
          connect_processes(context, context->active_processes.first, env->p);
          exit_add_wire_mode(context);
        } else {
          // select process
          if (!env->is_active) {
            clear_active_processes(context);
            SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, env->p, next_active, 0);
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
        for (Process *a = context->active_processes.first; a != 0; a = a->next_active) {
          if (Get_Flag(a->flags, Process_Flag_Wire)) {
            // TODO: save these changes into the proc-trie!!!!!
            Camera2D *camera = &env->view->camera;
            Vector2 mouse_world_position = GetScreenToWorld2D(context->ui_state.mouse_position, *camera);
            Editable_Process new_a = get_editable_process(context->process_edit_list, a);
            new_a.process.inner_position = mouse_world_position;
            add_process_to_process_edit_list(context, a, Proc_Trie_Edit_Update, new_a.process);
            updated = 1;
          }
          else {
            Vector2 new_position = get_process_position(context, &context->views[View_Kind_Procs], a);
            Editable_Process new_a = get_editable_process(context->process_edit_list, a);
            new_a.process.position = new_position;
            add_process_to_process_edit_list(context, a, Proc_Trie_Edit_Update, new_a.process);
            updated = 1;
          }
        }
        if (updated) {
          gather_processes_from_trie(context);
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

  return handled;
}









Define_Keybind_And_Action(
  SelectAnotherProcess, Default,
  Keybind_Behavior_Alternate, ForAllProcesses,
  Key_Kind_Mouse0, Modifier_Key_Control,
  Ui_Constraint_HoverProcess,
  "Add a process to the selected processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  Process_Selection selection = env->selection;

  if (Test_Keybind(env, Enter)) {
    handled = 1;
    if (selection.type == Process_Selection_In || selection.type == Process_Selection_Out) {
      Process *wire = get_wire_from_selection(context, selection);
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



Define_Keybind_And_Action(
  CancelSelection, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  Key_Kind_Mouse0, 0,
  Ui_Constraint_NoHotProcess,
  "Clear out the selected processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;

  if (check_keybind(env)) {
    handled = 1;
    exit_add_wire_mode(context);
  }

  return handled;
}

Define_Keybind_Order(CancelSelection_Default, After, ForAllProcessInteractions_Default);


Define_Keybind_And_Action(
  CreateProcess, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  Key_Kind_Mouse0, Modifier_Key_Control,
  Ui_Constraint_NoHotProcess,
  "Create a new process."
  ) {
  B32 handled = 0;
  Context *context = env->context;

  if (check_keybind(env)) {
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

Define_Keybind_Order(CreateProcess_Default, Before, ForAllProcessInteractions_Default);


Define_Keybind_And_Action(
  DeleteProcess, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  KEY_D, Modifier_Key_Control, 0,
  "Delete the selected processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;

  if (check_keybind(env)) {
    handled = 1;
    // delete processes
    for (Process *a = context->active_processes.first; a != 0;) {
      Process *next_active = a->next_active;
      delete_process(context, a, 0);
      a = next_active;
    }
    gather_processes_from_trie(context);
    clear_active_processes(context);
  }

  return handled;
}

Define_Keybind_Order(DeleteProcess_Default, Before, ForAllProcessInteractions_Default);



Define_Keybind_And_Action(
  CycleProcessDisplay, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  KEY_TAB, 0, 0,
  "Cycle through special displays for selected processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;

  if (check_keybind(env)) {
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

Define_Keybind_Order(CycleProcessDisplay_Default, Before, ForAllProcessInteractions_Default);


Define_Keybind_And_Action(
  ToggleDisplayMode, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  KEY_M, Modifier_Key_Control, 0,
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
  KEY_C, Modifier_Key_Control, 0,
  "Copy selected processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;

  if (check_keybind(env) == Keybind_Result_Enter) {
    handled = 1;
    copy_active_processes(context);
  }

  return handled;
}

Define_Keybind_Order(CopyProcess_Default, Before, ForAllProcessInteractions_Default);


Define_Keybind_And_Action(
  PasteProcess, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  KEY_V, Modifier_Key_Control, 0,
  "Paste copied processes, centered at the mouse."
  ) {
  B32 handled = 0;
  Context *context = env->context;

  if (check_keybind(env) == Keybind_Result_Enter) {
    handled = 1;
    paste_processes(context);
  }

  return handled;
}

Define_Keybind_Order(PasteProcess_Default, Before, ForAllProcessInteractions_Default);



Define_Keybind_And_Action(
  Undo, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  KEY_Z, Modifier_Key_Control, 0,
  "Performs undo on the proc-trie."
  ) {
  B32 handled = 0;
  Context *context = env->context;

  if (check_keybind(env) == Keybind_Result_Enter) {
    handled = 1;
    Set_Flag(env->context->ui_state.flags, Ui_State_Flag_action_occured);
    proc_trie_undo(context->proc_trie);
    gather_processes_from_trie(context);
  }

  return handled;
}

Define_Keybind_Order(Undo_Default, Before, ForAllProcessInteractions_Default);


Define_Keybind_And_Action(
  Redo, Default,
  Keybind_Behavior_Alternate, OnlyOnce,
  KEY_Z, Modifier_Key_Control|Modifier_Key_Shift, 0,
  "Performs redo on the proc-trie."
  ) {
  B32 handled = 0;
  Context *context = env->context;

  if (check_keybind(env) == Keybind_Result_Enter) {
    handled = 1;
    proc_trie_redo(context->proc_trie);
    gather_processes_from_trie(context);
  }

  return handled;
}

Define_Keybind_Order(Redo_Default, Before, ForAllProcessInteractions_Default);
