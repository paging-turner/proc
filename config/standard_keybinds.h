/*
  "Standard" might not be the correct name for these keybinds.

  The goal of this file is to show some examples of custom keybinds.
*/

function void proc_ds_view_root_handler(
  Proc_Trie_Iterator *iter,
  Proc_Trie_Root *root
  ) {
  printf("  root %p\n", root);
}

function void proc_ds_view_node_handler(
  Proc_Trie_Iterator *iter,
  Proc_Trie_Node *node
  ) {
  if (iter->stack->next) {
    printf("    %p -> %p\n", iter->stack->next->node, node);
  }
}

Define_Keybind(
  ToggleDataStructureView,,
  Keybind_Behavior_Overwrite, 274,
  KEY_D, Modifier_Key_Control|Modifier_Key_Shift,
  Ui_Constraint_ActionNotOccured,
  "Toggle a 'Data Structure View', which uses processes to display a hot data-structure."
  ) {
  B32 handled = 0;

  if (check_keybind(context, keybind_action_REF(ToggleDataStructureView), selection) == Keybind_Result_Enter) {
    handled = 1;

    if (Get_Flag(context->flags, Context_Flag_DataStructureView)) {
    }
    else {
      printf("trie %p\n", context->proc_trie);
      proc_trie_crawl_trie(context->per_frame_arena,
                           context->proc_trie,
                           proc_ds_view_root_handler,
                           proc_ds_view_node_handler);
    }

    // TODO: Map the proc-trie to processes with the same topology.
    Toggle_Flag(context->flags, Context_Flag_DataStructureView);
  }

  return handled;
}
