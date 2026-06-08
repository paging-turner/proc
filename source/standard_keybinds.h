/*
  "Standard" might not be the correct name for these keybinds.

  The goal of this file is to show some examples of custom keybinds.
*/




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
   Ui_Constraint_ActiveProcesses));

Define_Keybind_Order(ProcessConnectionByModClick_Standard, Before, ForAllProcessInteractions_Default);

Define_Keybind_Action(
  ProcessConnectionByModClick,
  "Connect clicked process to all active processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  /* Process_Selection selection = env->selection; */

  if (Test_Keybind(env, Enter)) {
    handled = 1;

    Assert(context->active_processes.first);
    Assert(context->hot_process.process);

    // @Speed
    U32 active_count = 0;
    Process **sorted_processes = 0;
    {
      for (Process *a = context->active_processes.first; a != 0; a = a->next_active) {
        if (!Get_Flag(a->flags, Process_Flag_Wire)) {
          active_count += 1;
        }
      }
      sorted_processes = arena_push(context->temp_arena, active_count*sizeof(Process *));
      U32 i = 0;
      for (Process *a = context->active_processes.first; a != 0; a = a->next_active) {
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
        conn_res = connect_processes_no_gather(context, sorted_processes[i], conn_res.in);
      }
    }

    gather_processes_from_trie(context);
  }

  return handled;
}





//////////////////////////////////
// Data-Structure Viewer
//////////////////////////////////

Define_Keybind(
  ToggleDataStructureView, Standard,
  Keybind_Behavior_Overwrite, OnlyOnce,
  KEY_D, Modifier_Key_Control|Modifier_Key_Shift,
  Ui_Constraint_ActionNotOccured);

Define_Keybind_Order(ToggleDataStructureView_Standard, Before, ForAllProcessInteractions_Default);

Define_Keybind_Action(
  ToggleDataStructureView,
  "Toggle a 'Data Structure View', which shows the proc-trie using processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  /* Process_Selection selection = env->selection; */

  if (Test_Keybind(env, Enter)) {
    // TODO: Map the proc-trie to processes with the same referential structure.
    Toggle_Flag(context->flags, Context_Flag_DataStructureView);
  }

  return handled;
}





Define_Keybind_And_Action(
  SelectRootFromUndoTrie, Standard,
  Keybind_Behavior_Alternate, ForAllProcesses,
  Key_Kind_Mouse0, 0,
  Ui_Constraint_HotProcess|Ui_Constraint_ExitOnKeyup|Ui_Constraint_ActionNotOccured,
  "Select root from undo trie."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  Process_Selection selection = env->selection;

  if (Test_Keybind(env, Enter)) {
    if (selection.view == context->views + View_Kind_Trie) {
      B32 in_selection = selection.type == Process_Selection_In;
      B32 out_selection = selection.type == Process_Selection_Out;

      if (selection.type == Process_Selection_Process) {
        if (selection.process->ref) {
          handled = 1;

          context->proc_trie->current_root = selection.process->ref;
          gather_processes_from_trie(context);
        }
      }
    }
  }

  return handled;
}



Define_Keybind(
  DeleteProcess, Easy,
  Keybind_Behavior_Alternate, OnlyOnce,
  KEY_BACKSPACE, 0, 0);
