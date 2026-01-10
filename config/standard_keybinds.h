/*
  "Standard" might not be the correct name for these keybinds.

  The goal of this file is to show some examples of custom keybinds.
*/




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

function void proc_ds_view_root_handler(
  void *maybe_context,
  Proc_Trie_Iterator *iter,
  Proc_Trie_Root *root
  ) {
  Context *context = (Context *)maybe_context;
  Proc_Trie_Trie *trie = context->proc_trie;

  B32 null_trie_ref = trie->ref == 0;

  Ensure_Process_Reference_Exists(trie);
  trie->ref->label = string_chunk_list_from_string8(context, str8_lit("Trie"));

  Ensure_Process_Reference_Exists(root);
  root->ref->label = string_chunk_list_from_string8(context, str8_lit("Root"));

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

  if (check_keybind(context, keybind_action_REF(ToggleDataStructureView), selection) == Keybind_Result_Enter) {
    handled = 1;

    if (Get_Flag(context->flags, Context_Flag_DataStructureView)) {
      clear_processes(context);

      // restore processes
      context->processes = context->gross_temp_processes;
    }
    else {
      // save processes for when we toggle back
      context->gross_temp_processes = context->processes;

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
