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
  ProcessConnectionByModClick, ,
  Keybind_Behavior_Alternate, 34, AtTheStart,
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
      conn_res.in = context->hot_process;
      for (U32 i = 0; i < active_count; ++i) {
        conn_res = connect_processes(context, sorted_processes[i], conn_res.in);
      }
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

#define Ensure_Process_Reference_Exists(a, l)\
  Stmnt(\
    if ((a)->ref == 0) {\
      Create_Process_Reference(a);\
      (a)->ref->label = string_chunk_list_from_string8(context, str8_lit(l));\
    })

#define Ensure_Process_Reference_Exists_No_Label(a)\
  Stmnt(\
    if ((a)->ref == 0) {\
      Create_Process_Reference(a);\
    })

#define Push_Ds_View_Process(p)\
  SLLQueuePush(context->views[View_Kind_Trie].processes.first,\
               context->views[View_Kind_Trie].processes.last,\
               (p))

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
  View *view = context->views + View_Kind_Trie;
  Proc_Trie_Trie *trie = context->proc_trie;

  B32 null_trie_ref = trie->ref == 0;
  S32 depth = proc_trie_get_depth_from_iterator(iter);

  // ensure trie exists
  Ensure_Process_Reference_Exists_No_Label(trie);
  trie->ref->ref_kind = Ref_Kind_ProcTrie;
  if (trie->ref && (trie->ref->label.first == 0 || trie->ref->label.last == 0)) {
    String8 label = str8_lit("trie");
    trie->ref->label = string_chunk_list_from_string8(context, label);
  }

  // ensure root exists
  Ensure_Process_Reference_Exists_No_Label(root);
  root->ref->ref_kind = Ref_Kind_ProcTrieRoot;
  root->depth = depth;
  if (root->ref && (root->ref->label.first == 0 || root->ref->label.last == 0)) {
    String8 label = str8_lit(Get_Trie_Root_C_String_From_Integer(depth));
    root->ref->label = string_chunk_list_from_string8(context, label);
  }

  // ensure first node exists
  Assert(root->node);
  if (root->node) {
    Ensure_Process_Reference_Exists_No_Label(root->node);
  }

  // positioning
  {
    // @Speed
    F32 padding = 20.0f;
    if (root->prev_edit && root->prev_edit->ref) {
      Process *prev_edit_p = root->prev_edit->ref;
      Process_Shape shape = get_process_shape(context, view, prev_edit_p);
      Vector2 p_size = get_process_size(context, prev_edit_p, shape);
      root->ref->position = (Vector2){prev_edit_p->position.x + (p_size.x + padding),
                                      prev_edit_p->position.y};
    }
    else {
      Process_Shape shape = get_process_shape(context, view, trie->ref);
      Vector2 p_size = get_process_size(context, trie->ref, shape);
      root->ref->position = (Vector2){ trie->ref->position.x,
                                       trie->ref->position.y - (p_size.y + padding)};
    }
  }

  Process *trie_to_root_wire = connect_detached_processes(context, trie->ref, root->ref);
  Process *root_to_first_node_wire = connect_detached_processes(context, root->ref, root->node->ref);

  if (null_trie_ref) {
    Push_Ds_View_Process(trie->ref);
  }
  Push_Ds_View_Process(root->ref);
  Push_Ds_View_Process(root->node->ref);
  Push_Ds_View_Process(trie_to_root_wire);
  Push_Ds_View_Process(root_to_first_node_wire);
}


function void proc_ds_view_node_handler(
  void *maybe_context,
  Proc_Trie_Iterator *iter,
  Proc_Trie_Node *node
  ) {
  Context *context = (Context *)maybe_context;
  S32 depth = proc_trie_get_depth_from_iterator(iter);

  Ensure_Process_Reference_Exists_No_Label(node);
  node->ref->ref_kind = Ref_Kind_ProcTrieNode;
  node->depth = depth;
  if (node->ref && (node->ref->label.first == 0 || node->ref->label.last == 0)) {
    String8 label = str8_lit(Get_Trie_Node_C_String_From_Integer(depth));
    node->ref->label = string_chunk_list_from_string8(context, label);
  }

  if (iter->stack->next) {
    B32 contains_node = 0;
    B32 contains_connection = 0;

    Process *in = node->ref;
    Assert(iter->stack->next->node->ref);
    Process *out = iter->stack->next->node->ref;

    for (Process *p = context->views[View_Kind_Trie].processes.first; p != 0; p = p->next) {
      if (p == node->ref) {
        contains_node = 1;
      }
      if (Get_Flag(p->flags, Process_Flag_Wire) && p->in == in && p->out == out) {
        contains_connection = 1;
      }
    }

    if (!contains_node) {
      Push_Ds_View_Process(node->ref);
    }
    if (!contains_connection) {
      Process *node_to_node_wire = connect_detached_processes(context, out, in);
      Push_Ds_View_Process(node_to_node_wire);
    }
  }
}

Define_Keybind(
  ToggleDataStructureView,,
  Keybind_Behavior_Overwrite, 274, AtTheStart,
  KEY_D, Modifier_Key_Control|Modifier_Key_Shift,
  Ui_Constraint_ActionNotOccured,
  "Toggle a 'Data Structure View', which shows the proc-trie using processes."
  ) {
  B32 handled = 0;
  Context *context = env->context;
  Process_Selection selection = env->selection;

  if (Test_Keybind(env, ToggleDataStructureView, Enter)) {
    // TODO: Map the proc-trie to processes with the same topology.
    Toggle_Flag(context->flags, Context_Flag_DataStructureView);
  }

  return handled;
}



Define_Keybind(
  AutoAlignOneInOneOut,,
  Keybind_Behavior_Overwrite, 1, AtTheStart,
  KEY_A, Modifier_Keys(Control, Super),
  Ui_Constraint_ActionNotOccured,
  "Align any following processes after a given node, but only if the following processes have a single in and a single out."
  ) {
  B32 handled = 0;

  if (Get_Flag(env->context->flags, Context_Flag_AutoAlignChains) ||
      Test_Keybind(env, AutoAlignOneInOneOut, Enter)) {
    Context *context = env->context;
    View *view = context->views + View_Kind_Trie;

    for (Process *p = view->processes.first; p != 0; p = p->next) {
      if (Get_Flag(p->flags, Process_Flag_Wire)) {
        Assert(p->in && p->out);
        if (p->in && p->in->in_count == 1 &&
            p->out && p->out->out_count == 1) {
          // copy position
          p->in->position = get_process_position(context, view, p->out);
        }
      }
      else if (p->ref && p->ref_kind) {
        // fix process y-position
        F32 y_pos = 0.0f;
        F32 y_scale = 100.0f;

        switch(p->ref_kind) {
        case Ref_Kind_ProcTrie: {
          y_pos = y_scale;
        } break;
        case Ref_Kind_ProcTrieNode: {
          y_pos = -y_scale * (F32)(((Proc_Trie_Node *)p->ref)->depth);
        } break;
        case Ref_Kind_ProcTrieRoot: {
          y_pos = -y_scale * (F32)(((Proc_Trie_Root *)p->ref)->depth);
        } break;
        }
        /* if (p->ref */
        p->position.y = y_pos;
      }
    }
  }

  return handled;
}
