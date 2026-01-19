/*
  "Standard" might not be the correct name for these keybinds.

  The goal of this file is to show some examples of custom keybinds.
*/



////////////////////////////////////////
// Process-Connection By Clicking
////////////////////////////////////////
Define_Keybind(
  ProcessConnectionByModClick,,
  Keybind_Behavior_Alternate, 34,
  Key_Kind_Mouse0, Modifier_Key_Super,
  (Ui_Constraint_ActionNotOccured |
   Ui_Constraint_HotProcess |
   Ui_Constraint_ActiveProcesses),
  "Connect clicked process to all active processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  Process_Selection selection = env->selection;

  if (Test_Keybind(env, ProcessConnectionByModClick, Enter)) {
    handled = 1;

    Assert(context->active_processes.first);
    Assert(context->hot_process);

    for (Process *a = context->active_processes.first; a != 0; a = a->next_active) {
      connect_processes(context, a, context->hot_process);
    }
  }

  return handled;
}





//////////////////////////////////
// Data-Structure Viewer
//////////////////////////////////

#define Create_Process_Reference(a)\
  Stmnt(\
    (a)->ref = create_detached_process(context);\
    (a)->ref->ref = (a);\
    )

#define Ensure_Process_Reference_Exists(a)\
  Stmnt(\
    if ((a)->ref == 0) {\
      Create_Process_Reference(a);\
    })

#define Push_Ds_View_Process(p)\
  SLLQueuePush(context->ds_view_processes.first, context->ds_view_processes.last, (p))

function void proc_ds_view_root_clear_handler(
  void *maybe_context,
  Proc_Trie_Iterator *iter,
  Proc_Trie_Root *root
  ) {
  Context *context = (Context *)maybe_context;
  Proc_Trie_Trie *trie = context->proc_trie;

  if (root) {
    root->ref = 0;
  }
}

function void proc_ds_view_node_clear_handler(
  void *maybe_context,
  Proc_Trie_Iterator *iter,
  Proc_Trie_Node *node
  ) {
  Context *context = (Context *)maybe_context;
  Proc_Trie_Trie *trie = context->proc_trie;

  if (node) {
    node->ref = 0;
  }
}

function void proc_ds_view_root_handler(
  void *maybe_context,
  Proc_Trie_Iterator *iter,
  Proc_Trie_Root *root
  ) {
  Context *context = (Context *)maybe_context;
  Proc_Trie_Trie *trie = context->proc_trie;

  B32 null_trie_ref = trie->ref == 0;

  // ensure trie exists
  Ensure_Process_Reference_Exists(trie);
  trie->ref->label = string_chunk_list_from_string8(context, str8_lit("Trie"));

  // ensure root exists
  Ensure_Process_Reference_Exists(root);
  root->ref->label = string_chunk_list_from_string8(context, str8_lit("Root"));

  // positioning
  {
    // @Speed
    F32 padding = 20.0f;
    if (root->prev_edit && root->prev_edit->ref) {
      Process *prev_edit_p = root->prev_edit->ref;
      Process_Shape shape = get_process_shape(context, prev_edit_p);
      Vector2 p_size = get_process_size(context, prev_edit_p, shape);
      root->ref->position = (Vector2){prev_edit_p->position.x + (p_size.x + padding),
                                      prev_edit_p->position.y};
    }
    else {
      Process_Shape shape = get_process_shape(context, trie->ref);
      Vector2 p_size = get_process_size(context, trie->ref, shape);
      root->ref->position = (Vector2){ trie->ref->position.x,
                                       trie->ref->position.y - (p_size.y + padding)};
    }
  }

  Process *wire = connect_detached_processes(context, trie->ref, root->ref);

  if (null_trie_ref) {
    Push_Ds_View_Process(trie->ref);
  }
  Push_Ds_View_Process(root->ref);
  Push_Ds_View_Process(wire);
}


function void proc_ds_view_node_handler(
  void *maybe_context,
  Proc_Trie_Iterator *iter,
  Proc_Trie_Node *node
  ) {
  Context *context = (Context *)maybe_context;
}

Define_Keybind(
  ToggleDataStructureView,,
  Keybind_Behavior_Overwrite, 274,
  KEY_D, Modifier_Key_Control|Modifier_Key_Shift,
  Ui_Constraint_ActionNotOccured,
  "Toggle a 'Data Structure View', which shows the proc-trie using processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  Process_Selection selection = env->selection;


  if (check_keybind(context, keybind_action_REF(ToggleDataStructureView), selection) == Keybind_Result_Enter) {
    handled = 1;

    if (Get_Flag(context->flags, Context_Flag_DataStructureView)) {
      for (Process *p = context->processes.first; p != 0; p = p->next) {
        remove_string_chunk_list(context, &p->label);
      }
      clear_processes(context);

      proc_trie_crawl_trie(context->per_frame_arena,
                           context->proc_trie,
                           proc_ds_view_root_clear_handler,
                           proc_ds_view_node_clear_handler,
                           context);

      clear_ds_view_process_list(context);

      context->proc_trie->ref = 0;

      // restore processes
      context->processes = context->gross_temp_processes;
    }
    else {
      // save processes for when we toggle back
      context->gross_temp_processes = context->processes;
      clear_processes(context);

      clear_ds_view_process_list(context);

      proc_trie_crawl_trie(context->per_frame_arena,
                           context->proc_trie,
                           proc_ds_view_root_handler,
                           proc_ds_view_node_handler,
                           context);

      // patch in the data-structure view processes
      context->processes = context->ds_view_processes;
    }

    // TODO: Map the proc-trie to processes with the same topology.
    Toggle_Flag(context->flags, Context_Flag_DataStructureView);
  }

  return handled;
}
