/*
  "Standard" might not be the correct name for these keybinds.

  The goal of this file is to show some examples of custom keybinds.
*/




//////////////////////////////////
// Data-Structure Viewer
//////////////////////////////////

function void proc_ds_view_root_handler(
  void *maybe_context,
  Proc_Trie_Iterator *iter,
  Proc_Trie_Root *root
  ) {
  Context *context = (Context *)maybe_context;
  Process *p = create_detached_process(context);

  // setup references
  p->ref = root;
  root->ref = p;

  p->label = string_chunk_list_from_string8(context, str8_lit("Trie"));
  SLLQueuePush(context->ds_view_processes.first, context->ds_view_processes.last, p);
}


function void proc_ds_view_node_handler(
  void *maybe_context,
  Proc_Trie_Iterator *iter,
  Proc_Trie_Node *node
  ) {
  Context *context = (Context *)maybe_context;

  if (node->ref == 0) {
    node->ref = create_detached_process(context);
    node->ref->ref = node;
  }

#if 1
  if (iter->stack->next && iter->stack->next->node) {
    B32 found = 0;
    Proc_Trie_Node *in = node;
    Proc_Trie_Node *out = iter->stack->next->node;
    Assert(in->ref);
    Assert(out->ref);

    for (Process *p = context->ds_view_processes.first; p != 0; p = p->next) {
      if (Get_Flag(p->flags, Process_Flag_Wire)) {
        Assert(p->in->ref && p->out->ref);
        if ((p->in && p->in->ref == in) &&
            (p->out && p->out->ref == out)) {
          found = 1;
        }
      }
    }
    if (!found) {
      Process *p = create_detached_process(context);
      Process *w = connect_detached_processes(context, out->ref, in->ref);

      if (w && w->in && w->out && w->in->ref && w->out->ref) {
        w->in->ref = in;
        w->out->ref = out;
      }

      SLLQueuePush(context->ds_view_processes.first, context->ds_view_processes.last, p);
      SLLQueuePush(context->ds_view_processes.first, context->ds_view_processes.last, w);
    }
  }
#endif
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
