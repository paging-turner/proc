#define List_For_N(t, n, i, next)\
  for (t n = (i); n != 0; n = n->next)

#define List_For(t, n, i)\
  List_For_N(t, n, i, next)



#define Robust_Assertions 1
// NOTE: It's a little confusing that we do the if on the negation... maybe there's a better way to word this construct.
#if Robust_Assertions
# define Assert_If(exp)   if(!(exp))
#else
# define Assert_If(exp)   Assert(exp);if(0)
#endif



#define Piece_Table_Chunk_Size 8
struct Piece_Table_Chunk {
  U32 offset;
  U8 str_array[Piece_Table_Chunk_Size];
  struct Piece_Table_Chunk *next;
};


struct Piece_Table_Row {
  struct Piece_Table_Row *next;
  struct Piece_Table_Row *prev;
  Piece_Table_Chunk *chunk;
  U64 offset;
  U64 size;
};


typedef struct Piece_Table {
  Piece_Table_Row *first_row;
  Piece_Table_Row *last_row;
  Piece_Table_Chunk *insertion_chunk;
  U64 text_size;
} Piece_Table;






#define Process_Do_Undo_Kind_Flag_From_Kind(kind)\
  (((kind) > 0 && (kind) < Process_Do_Undo_Kind__Count) ? (1<<(kind)) : 0)

// TODO: Do _not_ hard-code Proc and Ui here, these need to be symbol-sets or something dynamic.
#define Process_Do_Undo_Kind_Xlist(X)\
  X(Proc) X(Ui)

enum Process_Do_Undo_Kind {
  Process_Do_Undo_Kind__Null,
#define X(kind)\
  Process_Do_Undo_Kind_##kind,
  Process_Do_Undo_Kind_Xlist(X)
#undef X
  Process_Do_Undo_Kind__Count,
};

enum Process_Do_Undo_Kind_Flag {
#define X(kind)\
  Process_Do_Undo_Kind_Flag_##kind = (1 << Process_Do_Undo_Kind_##kind),
  Process_Do_Undo_Kind_Xlist(X)
#undef X
};





// Process Trie
#define Use_Gen_Id_For_Trie_Key 1
#define Proc_Trie_Key_Bits               64
#define Proc_Trie_Slot_Bits              2
#if Use_Gen_Id_For_Trie_Key
# define Proc_Trie_Use_Key_Value_Pair    1
#else
# define Proc_Trie_Use_Key_Value_Pair    0
#endif
#define Steady_Trie_Use_Key_Value_Pair  Proc_Trie_Use_Key_Value_Pair
#define Steady_Trie(ident)               Proc_Trie_##ident
#define steady_trie(ident)               proc_trie_##ident
#define Steady_Trie_Root_Is_Least_Significant_Byte 1
#define Steady_Trie_Value_Type           Process
Process global_default_steady_trie_process;
#define Steady_Trie_Default_Value        (&global_default_steady_trie_process)
#define Steady_Trie_Use_Debug_Log        0
#include "../libraries/steady_trie.h"
#define Proc_Trie_Iterate(iter_name, arena, trie)\
  for (Proc_Trie_Iterator *iter_name = proc_trie_iter_init(arena, trie->current_root->node);\
       proc_trie_iter_test(iter_name);\
       proc_trie_iter_next(iter_name))






#define Process_Connection_Xlist\
  X(In, 0) X(Out, 1)

typedef enum Process_Connection {
#define X(conn, ...)\
  Process_Connection_##conn,
  Process_Connection_Xlist
#undef X
  Process_Connection__Count,
} Process_Connection;

typedef enum Process_Connection_Flag {
#define X(conn, i)\
  Process_Connection_Flag_##conn = (1 << i),
  Process_Connection_Xlist
#undef X
} Process_Connection_Flag;


struct Process {
  //////////////
  // Members that need to be saved when serializing.
  //////////////
  B32 flags;
  U64 gen_id;
  Vector2 position;

  V2_Chunk *inner_positions; // NOTE: used for extra points in a wire's curve

  Piece_Table *label;

  union {
    struct {
      Process *in;
      Process *out;
    };
    Process *conn[Process_Connection__Count];
  };

  union {
    struct {
      U32 which_in;
      U32 which_out;
    };
    U32 which_conn[Process_Connection__Count];
  };

  //////////////
  // Members that are "ephemeral", which can be constructed from serialized members.
  //     or it's for UI...
  //////////////
  union {
    struct {
      S32 in_count;
      S32 out_count;
    };
    S32 conn_count[Process_Connection__Count];
  };

  void (*func)(Context*, View*, Process*); // TODO: What do we do about this func? It's only used for UI elements, so maybe we should stop using Processes as UI elements and give up on the idea of process-ui?

  Vector2 margin;
  Ui_Box ui_box;

  Process *to_copied;

  Process *next;
  Process *next_active;
  Process *parent;

  U8 *label_c_string;
  U32 label_cursor;

  Ref_Kind ref_kind;
  void *ref;

  U64 cold_id;
};
typedef struct Process_Edit {
  Proc_Trie_Edit_Kind kind;
  Process *process;
  Process new_process;
  Process *new_process_ptr;
  struct Process_Edit *next;
} Process_Edit;

typedef struct Process_Edit_List {
  Process_Edit *first;
  Process_Edit *last;
} Process_Edit_List;

typedef struct Process_Do_Undo {
  Proc_Trie_Trie *trie;
  Process_Edit_List edit_list;
  Arena *arena;
} Process_Do_Undo;


typedef struct Process_List {
  Process *first;
  Process *last;
} Process_List;


typedef struct Editable_Process {
  B32 is_being_edited;
  Process process;
} Editable_Process;


typedef struct Connection_Result {
  Process *out;
  Process *in;
  Process *new_wire;
} Connection_Result;





typedef struct WhatIsThis {
  Arena *arena;
  Arena *permanent_arena;
  Arena *per_frame_arena;
  Process_Do_Undo do_undo;
  U64 gen_id;

  Process_List processes;
  U64 process_count;
  Process_List active_processes;
  Process_List free_processes;

  Piece_Table_Memory piece_table_memory;
} WhatIsThis;










global_variable Process global_null_process;
#define The_Null_Process() (global_null_process=(Process){0}, &global_null_process)

global_variable String_Chunk global_null_string_chunk;
#define The_Null_String_Chunk() (global_null_string_chunk=(String_Chunk){0}, &global_null_string_chunk)












function Process_Edit *process_edit_list_contains_process(
  Process_Edit_List proc_edit_list,
  Process *p
  ) {
  Process_Edit *result = 0;

  for (Process_Edit *proc_edit = proc_edit_list.first;
       proc_edit != 0;
       proc_edit = proc_edit->next) {
    if (proc_edit->process == p) {
      result = proc_edit;
      break;
    }
  }

  return result;
}



function B32 add_process_to_process_edit_list(
  WhatIsThis *wit,
  Process *p,
  Proc_Trie_Edit_Kind edit_kind,
  Process new_process
  ) {
  B32 overwritten = 0;
  Process_Edit *found_proc_edit = 0;

  // ensure that do-undo matches the do-undo of the passed-in process
  if (wit && wit->per_frame_arena) {
    for (Process_Edit *proc_edit = wit->do_undo.edit_list.first;
         proc_edit != 0;
         proc_edit = proc_edit->next) {
      if (proc_edit->process == p) {
        found_proc_edit = proc_edit;
        break;
      }
    }

    if (found_proc_edit == 0) {
      found_proc_edit = arena_push(wit->per_frame_arena, sizeof(Process_Edit));
      SLLQueuePush(wit->do_undo.edit_list.first, wit->do_undo.edit_list.last, found_proc_edit);
    }

    // TODO: Overwrite if we are deleting, if we are updating again... then we need to consider that an error or figure out a better way to merge updates.
    if (found_proc_edit) {
      if (found_proc_edit->kind != Proc_Trie_Edit_Delete) {
        found_proc_edit->process = p;
        found_proc_edit->kind = edit_kind;
        found_proc_edit->new_process = new_process;
        overwritten = 1;
      }
    }
  }

  return overwritten;
}




function void update_edited_wire_pointers(
  WhatIsThis *wit,
  Process_Edit *proc_edit,
  B32 inserting
  ) {
  // update the pointers of the wire if the connected processes have been updated
  for (Process_Edit *test_edit = wit->do_undo.edit_list.first;
       test_edit != 0;
       test_edit = test_edit->next) {
    if (!Get_Flag(test_edit->process->flags, Process_Flag_Wire)) {
      if (inserting) {
        if (proc_edit->process->in == test_edit->process) {
          proc_edit->process->in = test_edit->new_process_ptr;
        }

        if (proc_edit->process->out == test_edit->process) {
          proc_edit->process->out = test_edit->new_process_ptr;
        }
      }
      else {
        if (proc_edit->new_process.in == test_edit->process) {
          proc_edit->new_process.in = test_edit->new_process_ptr;
        }

        if (proc_edit->new_process.out == test_edit->process) {
          proc_edit->new_process.out = test_edit->new_process_ptr;
        }
      }
    }
  }
}



function Process *push_permanent_process(WhatIsThis *wit) {
  Process *p = 0;

  if (wit && wit->permanent_arena) {
    p = push_struct(wit->permanent_arena, Process);
    if (p) {
      p->gen_id = wit->gen_id++;
    }
  }

  return p;
}


function Process *create_process(WhatIsThis *wit) {
  Process *p = push_permanent_process(wit);

  if (p) {
    add_process_to_process_edit_list(wit, p, Proc_Trie_Edit_Insert, (Process){0});
  }

  return p;
}





function Editable_Process get_editable_process(
  Process_Edit_List edit_list,
  Process *p
  ) {
  Editable_Process editable_proc = (Editable_Process){0};

  for (Process_Edit *proc_edit = edit_list.first;
       proc_edit != 0;
       proc_edit = proc_edit->next) {
    if (proc_edit->process == p) {
      editable_proc.is_being_edited = 1;
      if (proc_edit->kind == Proc_Trie_Edit_Update) {
        editable_proc.process = proc_edit->new_process;
      }
      else {
        editable_proc.process = *proc_edit->process;
      }
    }
  }

  if (!editable_proc.is_being_edited) {
    editable_proc.process = *p;
  }

  return editable_proc;
}






function void apply_process_edits_by_kind(
  WhatIsThis *wit,
  B32 handle_wires
  ) {
  Assert(handle_wires == 0 || handle_wires == 1);
  if (wit == 0) goto error;
  /* Process_Do_Undo *do_undo = &view->do_undo; */
  /* Arena *do_undo_arena = do_undo->arena; */
  /* if (do_undo_arena == 0) goto error; */

  // TODO: @Speed
  for (Process_Edit *proc_edit = wit->do_undo.edit_list.first;
       proc_edit != 0;
       proc_edit = proc_edit->next) {
    B32 is_wire = Get_Flag(proc_edit->process->flags, Process_Flag_Wire) ? 1 : 0;
    B32 should_edit = !(handle_wires ^ is_wire);

    if (should_edit) {
      switch(proc_edit->kind) {
      case Proc_Trie_Edit_Insert: {
        Assert(proc_edit->process);

        if (is_wire) {
          update_edited_wire_pointers(wit, proc_edit, 1);
        }

#if Use_Gen_Id_For_Trie_Key
# if Proc_Trie_Use_Key_Value_Pair
        proc_trie_set(wit->permanent_arena, wit->do_undo.trie, proc_edit->process->gen_id, proc_edit->process);
# else
        Assert(!"This should not happen.....");
# endif
#else
# if Proc_Trie_Use_Key_Value_Pair
        Assert(!"This should not happen.....");
# else
        proc_trie_insert(wit->permanent_arena, do_undo->trie, IntFromPtr(proc_edit->process));
# endif
#endif
      } break;
      case Proc_Trie_Edit_Delete: {
#if Use_Gen_Id_For_Trie_Key
        proc_trie_delete(wit->permanent_arena, wit->do_undo.trie, proc_edit->process->gen_id);
#else
        proc_trie_delete(wit->permanent_arena, do_undo->trie, IntFromPtr(proc_edit->process));
#endif
      } break;
      case Proc_Trie_Edit_Update: {
        Process *new_p = push_permanent_process(wit);
        if (new_p) {
          proc_edit->new_process_ptr = new_p;

          if (is_wire) {
            update_edited_wire_pointers(wit, proc_edit, 0);
          }
          else {
            // update any non-edited wires connected to proc being updated
            for (Process *w = wit->processes.first; w != 0; w = w->next) {
              if (Get_Flag(w->flags, Process_Flag_Wire)) {
                B32 wire_in_edit_list = 0;
                // TODO: We should be able to call the new `get_editable_process` here, right?
                Process new_wire_lit = *w;
                // find current new-wire lit if it exists, and overwrite `new_wire_lit`
                for (Process_Edit *proc_edit = wit->do_undo.edit_list.first;
                     proc_edit != 0;
                     proc_edit = proc_edit->next) {
                  if (proc_edit->process == w) {
                    wire_in_edit_list = 1;
                    new_wire_lit = proc_edit->new_process;
                    break;
                  }
                }
                B32 in_match = w->in == proc_edit->process;
                B32 out_match = w->out == proc_edit->process;

                if (in_match) {
                  new_wire_lit.in = proc_edit->new_process_ptr;
                }

                if (out_match) {
                  new_wire_lit.out = proc_edit->new_process_ptr;
                }

                if (!wire_in_edit_list && (in_match || out_match)) {
                  // TODO: Use `get_editable_process` for new_wire_lit
                  add_process_to_process_edit_list(wit, w, Proc_Trie_Edit_Update, new_wire_lit);
                }
              }
            }
          }

          // copy proc
          U64 new_gen_id = new_p->gen_id;
          *new_p = proc_edit->new_process;
          new_p->gen_id = new_gen_id;
          // copy proc label
          if (new_p->label) {
            B32 error = 0;
            Piece_Table *new_piece_table = push_struct(wit->permanent_arena, Piece_Table);
            if (new_piece_table) {
              new_piece_table->text_size = new_p->label->text_size;
              new_piece_table->insertion_chunk = new_p->label->insertion_chunk;
              for (Piece_Table_Row *row = new_p->label->first_row;
                   row != 0;
                   row = row->next) {
                Piece_Table_Row *new_row = push_struct(wit->permanent_arena, Piece_Table_Row);
                if (new_row) {
                  *new_row = *row;
                  DLLPushBack(new_piece_table->first_row, new_piece_table->last_row, new_row);
                }
                else {
                  printf("[ Error ] Pushing Piece_Table_Row while copying piece-table in `apply_process_edits_by_kind`\n");
                  new_p->label = 0;
                  error = 1;
                  break;
                }
              }
            }
            else {
              printf("[ Error ] Pushing Piece_Table while copying piece-table in `apply_process_edits_by_kind`\n");
            }

            if (!error) {
              new_p->label = new_piece_table;
            }
          }

#if Use_Gen_Id_For_Trie_Key
          proc_trie_delete(wit->permanent_arena, wit->do_undo.trie, proc_edit->process->gen_id);
# if Proc_Trie_Use_Key_Value_Pair
          proc_trie_set(wit->permanent_arena, wit->do_undo.trie, new_p->gen_id, new_p);
# else
          Assert(!"This should not happen");
# endif
#else
          proc_trie_delete(wit->permanent_arena, do_undo->trie, IntFromPtr(proc_edit->process));
# if Proc_Trie_Use_Key_Value_Pair
          Assert(!"This should not happen");
# else
          proc_trie_insert(wit->permanent_arena, do_undo->trie, IntFromPtr(new_p));
# endif
#endif
        }
      } break;
      default: Assert(0);
      }
    }
  }
error:;
}




function void clear_process_list(Process_List *list, Process_List *free_list) {
  if (list && list->first) {
    for (Process *p = list->first; p != 0;) {
      Process *next = p->next;
      if (Get_Flag(p->flags, Process_Flag_IsDetached)) {
        SLLQueuePush(free_list->first, free_list->last, p);
      }
      else {
        p->next = 0;
      }
      p = next;
    }

    list->first = 0;
    list->last = 0;
  }
}



function void gather_processes_from_trie(WhatIsThis *wit) {
  if (wit == 0) goto error;
  Proc_Trie_Trie *trie = wit->do_undo.trie;
  if (trie == 0) goto error;

  { // apply process edits
    apply_process_edits_by_kind(wit, 0);
    apply_process_edits_by_kind(wit, 1);
  }

  // transfer active, edited procs
  Process_List new_active_procs = (Process_List){0};
  for (Process_Edit *proc_edit = wit->do_undo.edit_list.first;
       proc_edit != 0;
       proc_edit = proc_edit->next) {
    for (Process *a = wit->active_processes.first; a != 0; a = a->next_active) {
      if (proc_edit->process == a && proc_edit->new_process_ptr) {
        SLLQueuePush_NZ(new_active_procs.first, new_active_procs.last, proc_edit->new_process_ptr, next_active, 0);
      }
    }
  }

  wit->active_processes = new_active_procs;

  wit->do_undo.edit_list = (Process_Edit_List){0};
  proc_trie_commit(trie);

  { // what is this block?
    Arena *arena = wit->per_frame_arena;
    clear_process_list(&wit->processes, &wit->free_processes);

    wit->process_count = 0;
    for (Proc_Trie_Iterator *iter = proc_trie_iter_init(arena, trie->current_root->node);
         proc_trie_iter_test(iter);
         proc_trie_iter_next(iter)) {
#if Use_Gen_Id_For_Trie_Key
      Process *p = iter->value;
#else
      Process *p = (Process *)iter->key;
#endif
      if (p) {
        SLLQueuePush(wit->processes.first, wit->processes.last, p);
        wit->process_count += 1;
      }
    }

#if 0 // TODO: Find a way to switch between kinds of views and draw the data-structure view
    { // Update data-structure view processes
      Process_List *ds_proc_list = &view->processes;
      clear_process_list(context, ds_proc_list);

      for (Proc_Trie_Iterator *iter = proc_trie_iter_root_init(context->per_frame_arena, wit->do_undo.trie);
           proc_trie_iter_root_test(iter);
           proc_trie_iter_root_next(iter)) {
        Process *p = create_detached_process(context);
        p->position.x = (F32)iter->stack->indent * 60.0f;
        p->position.y = (F32)iter->stack->depth * 60.0f;
        if (iter->stack->root == wit->do_undo.trie->current_root) {
          Set_Flag(p->flags, Process_Flag_IsActive);
        }
        p->ref = iter->stack->root;
        { // set the label_c_string
          String8 gen_id_string = str8_lit(TextFormat("%llu", p->gen_id));
          if (gen_id_string.str && gen_id_string.size) {
            U8 *label_c_string = arena_push(context->per_frame_arena, gen_id_string.size);
            if (label_c_string) {
              MemoryCopy(label_c_string, gen_id_string.str, gen_id_string.size);
              p->label_c_string = label_c_string;
            }
          }
        }
        iter->stack->root->ref = p;
        SLLQueuePush(ds_proc_list->first, ds_proc_list->last, p);

        // add line to prev_edit
        if (iter->stack->root->prev_edit) {
          Process *l = create_detached_process(context);
          Set_Flag(l->flags, Process_Flag_Line);
          l->in = iter->stack->root->prev_edit->ref;
          l->out = iter->stack->root->ref;
          SLLQueuePush(ds_proc_list->first, ds_proc_list->last, l);
        }

        // add line to prev_branch
        if (iter->stack->root->prev_branch) {
          Process *l = create_detached_process(context);
          Set_Flag(l->flags, Process_Flag_Line);
          l->in = iter->stack->root->prev_branch->ref;
          l->out = iter->stack->root->ref;
          SLLQueuePush(ds_proc_list->first, ds_proc_list->last, l);
        }

      }
    }
#endif
  }
error:;
}








function void clear_active_process_list(WhatIsThis *wit, Process_List *list) {
  if (list && list->first) {
    for (Process *p = list->first; p != 0;) {
      Process *next = p->next_active;
      p->next_active = 0;
      Unset_Flag(p->flags, Process_Flag_IsActive);
      p = next;
    }

    list->first = 0;
    list->last = 0;
  }
}



function void clear_active_processes(WhatIsThis *wit, Process_List *active_processes) {
  clear_active_process_list(wit, active_processes);
}



function Process *create_detached_process(WhatIsThis *wit) {
  Process *p = wit->free_processes.first;

  if (p) {
    SLLQueuePop(wit->free_processes.first, wit->free_processes.last);
    // TODO: do we need to update the gen-id here??
  } else {
    p = push_permanent_process(wit);
  }

  if (p) {
    *p = (Process){0};
    Set_Flag(p->flags, Process_Flag_IsDetached);
    p->gen_id = wit->gen_id++;
  } else {
    p = The_Null_Process();
  }

  return p;
}



function Process *create_processes(WhatIsThis *wit) {
  Process *ps = push_array(wit->permanent_arena, Process, wit->process_count);

  if (ps) {
    for (U32 i = 0; i < wit->process_count; ++i) {
      Process *p = ps + i;
      p->gen_id = wit->gen_id++;
      add_process_to_process_edit_list(wit, p, Proc_Trie_Edit_Insert, (Process){0});
    }

    // TODO: if we end up allowing dynamic creation of procs for ui, we need to switch with kind of do-undo to pass to the gather func........
    gather_processes_from_trie(wit);
  }

  return ps;
}




function void remove_process_from_process_list(
  Process_List *list,
  Process *p
  ) {
  if (list->first == p) {
    SLLQueuePop(list->first, list->last);
  } else {
    for (Process *test_p = list->first; test_p != 0; test_p = test_p->next) {
      if (test_p->next == p) {
        test_p->next = p->next;
        if (p == list->last) {
          list->last = test_p;
        }
        break;
      }
    }
  }
}



function Piece_Table *copy_piece_table(Piece_Table *table) {
  Assert(!"TODO");
  return 0;
}




function String_Chunk *create_string_chunk(
  WhatIsThis *wit,
  Arena *permanent_arena,
  String_Chunk_List *free_strings
  ) {
  String_Chunk *c = free_strings->first;

  if (c) {
    SLLQueuePop(free_strings->first, free_strings->last);
  } else {
    c = push_struct(permanent_arena, String_Chunk);
  }

  if (c) {
    *c = (String_Chunk){0};
  } else {
    c = The_Null_String_Chunk();
  }

  return c;
}


function void free_string_chunk(
  WhatIsThis *wit,
  String_Chunk_List *free_strings,
  String_Chunk *chunk
  ) {
  SLLQueuePush(free_strings->first, free_strings->last, chunk);
  chunk->next = 0;
}




function void add_wire_connection(
  WhatIsThis *wit,
  Process *wire,
  Process *process,
  Process_Connection conn,
  U32 which_conn
  ) {
  if (wit && wire && process) {
    { // what is this block?
      {
        B32 wire_moved_to_same_process = wire->conn[conn] == process;
        B32 wire_is_to_the_left_of_itself = which_conn > wire->which_conn[conn];

        Editable_Process new_wire = get_editable_process(wit->do_undo.edit_list, wire);
        new_wire.process.conn[conn] = process;
        if (wire_moved_to_same_process && wire_is_to_the_left_of_itself) {
          new_wire.process.which_conn[conn] = which_conn - 1;
        }
        else {
          new_wire.process.which_conn[conn] = which_conn;
        }
        add_process_to_process_edit_list(wit, wire, Proc_Trie_Edit_Update, new_wire.process);
      }

      { // decrement currently connected process' conn-count
        Editable_Process new_process = get_editable_process(wit->do_undo.edit_list, wire->conn[conn]);
        new_process.process.conn_count[conn] -= 1;
        add_process_to_process_edit_list(wit, wire->conn[conn], Proc_Trie_Edit_Update, new_process.process);
      }

      { // increment newly connected process' conn-count
        Editable_Process new_process = get_editable_process(wit->do_undo.edit_list, process);
        new_process.process.conn_count[conn] += 1;
        add_process_to_process_edit_list(wit, process, Proc_Trie_Edit_Update, new_process.process);
      }

      for (Process *test_wire = wit->processes.first;
           test_wire != 0;
           test_wire = test_wire->next) {
        if (Get_Flag(test_wire->flags, Process_Flag_Wire)) {
          B32 not_the_moved_wire = wire != test_wire;
          B32 test_wire_to_the_right_of_old_process = test_wire->which_conn[conn] >= wire->which_conn[conn];
          B32 test_wire_to_the_right_of_new_process = test_wire->which_conn[conn] >= which_conn;
          B32 test_wire_connected_to_old_process = test_wire->conn[conn] == wire->conn[conn];
          B32 test_wire_connected_to_new_process = test_wire->conn[conn] == process;

          if (not_the_moved_wire) {
            // decrement which_conn
            if (test_wire_connected_to_old_process &&
                test_wire_to_the_right_of_old_process) {
              Editable_Process new_test_wire = get_editable_process(wit->do_undo.edit_list, test_wire);
              new_test_wire.process.which_conn[conn] -= 1;
              add_process_to_process_edit_list(wit, test_wire, Proc_Trie_Edit_Update, new_test_wire.process);
            }

            // increment which_conn
            if (test_wire_connected_to_new_process &&
                test_wire_to_the_right_of_new_process) {
              Editable_Process new_test_wire = get_editable_process(wit->do_undo.edit_list, test_wire);
              new_test_wire.process.which_conn[conn] += 1;
              add_process_to_process_edit_list(wit, test_wire, Proc_Trie_Edit_Update, new_test_wire.process);
            }
          }
        }
      }
    }
  }
}



function void handle_deleted_wire(
  WhatIsThis *wit,
  Process *wire,
  Process_Connection_Flag conn_flags
  ) {
  if (wire) {
    // Remove a wire and move wires to the right of the moved wire to the left.
    B32 in_matched = 0;
    B32 out_matched = 0;

    // TODO: Now that we set both remove_in/remove_out as true, we should clean up some code below.
    B32 remove_in = Get_Flag(conn_flags, Process_Connection_Flag_In);
    B32 remove_out = Get_Flag(conn_flags, Process_Connection_Flag_Out);

    for (Process *test_wire = wit->processes.first;
         test_wire != 0;
         test_wire = test_wire->next) {
      B32 should_replace = 0;
      B32 is_wire = Get_Flag(test_wire->flags, Process_Flag_Wire);

      if (is_wire && test_wire != wire) {
        Editable_Process new_test_wire = get_editable_process(wit->do_undo.edit_list, test_wire);

        // adjust in-connections that come after deleted wire
        if (remove_in && test_wire->in == wire->in) {
          if (test_wire->which_in > wire->which_in) {
            new_test_wire.process.which_in -= 1;
            should_replace = 1;
          }
          in_matched = 1;
        }

        // adjust out-connections that come after deleted wire
        if (remove_out && test_wire->out == wire->out) {
          if (test_wire->which_out > wire->which_out) {
            new_test_wire.process.which_out -= 1;
            should_replace = 1;
          }
          out_matched = 1;
        }

        if (should_replace) {
          add_process_to_process_edit_list(wit, test_wire, Proc_Trie_Edit_Update, new_test_wire.process);
        }
      }
    }

    B32 only_in_conn = wire->in != 0 && wire->which_in == 0;
    B32 only_out_conn = wire->out != 0 && wire->which_out == 0;

    // decrement process' in-count
    if (remove_in && (in_matched || only_in_conn)) {
      if (wire->in) {
        Editable_Process new_in = get_editable_process(wit->do_undo.edit_list, wire->in);
        new_in.process.in_count -= 1;
        add_process_to_process_edit_list(wit, wire->in, Proc_Trie_Edit_Update, new_in.process);
      }
    }

    // decrement process' out-count
    if (remove_out && (out_matched || only_out_conn)) {
      if (wire->out) {
        Editable_Process new_out = get_editable_process(wit->do_undo.edit_list, wire->out);
        new_out.process.out_count -= 1;
        add_process_to_process_edit_list(wit, wire->out, Proc_Trie_Edit_Update, new_out.process);
      }
    }
  }
}







function void delete_process(WhatIsThis *wit, Process *p, U32 which_conn_flags) {
  B32 p_overwritten = add_process_to_process_edit_list(wit, p, Proc_Trie_Edit_Delete, (Process){0});

  // if deleting a wire, adjust connected processes
  if (Get_Flag(p->flags, Process_Flag_Wire)) {
    Process_Connection_Flag which_conn_flags_resolved = which_conn_flags
      ? which_conn_flags
      : (Process_Connection_Flag_In | Process_Connection_Flag_Out);
    if (p_overwritten) {
      handle_deleted_wire(wit, p, which_conn_flags_resolved); // TODO: inline this function since it's only used in `delete_process`
    }
  }
  else {
    // check for wires connected to the deleted process, and delete those also
    for (Process *wire = wit->processes.first; wire != 0;) {
      B32 in_match = wire->in == p;
      B32 out_match = wire->out == p;
      B32 should_delete = 0;

      if (in_match || out_match) {
        B32 wire_overwritten = add_process_to_process_edit_list(wit, wire, Proc_Trie_Edit_Delete, (Process){0});
        if (wire_overwritten) {
          handle_deleted_wire(wit, wire, (Process_Connection_Flag_In|Process_Connection_Flag_Out));
        }
      }

      wire = wire->next;
    }
  }
}



function Process *connect_detached_processes(
  WhatIsThis *wit,
  Process *out,
  Process *in
  ) {
  Process *new_wire = 0;

  if (out && in) {
    new_wire = create_detached_process(wit);

    if (new_wire) {
      Set_Flag(new_wire->flags, Process_Flag_Wire);

      new_wire->out = out;
      new_wire->in = in;

      new_wire->which_out = out->out_count;
      new_wire->which_in = in->in_count;

      out->out_count += 1;
      in->in_count += 1;
    }
  }

  return new_wire;
}



function Connection_Result connect_processes_no_gather(
  WhatIsThis *wit,
  Process *out,
  Process *in
  ) {
  Connection_Result result = (Connection_Result){0};

  if (out && in) {
    result.new_wire = create_process(wit);

    if (result.new_wire) {
      Process_Edit *out_edit_proc = process_edit_list_contains_process(wit->do_undo.edit_list, out);
      Process_Edit *in_edit_proc = process_edit_list_contains_process(wit->do_undo.edit_list, in);

      if (out_edit_proc) {
        result.new_wire->which_out = out_edit_proc->new_process.out_count;
        out_edit_proc->new_process.out_count += 1;
      }
      else {
        Editable_Process new_out = get_editable_process(wit->do_undo.edit_list, out);
        result.new_wire->which_out = new_out.process.out_count;
        new_out.process.out_count += 1;
        add_process_to_process_edit_list(wit, out, Proc_Trie_Edit_Update, new_out.process);
      }

      if (in_edit_proc) {
        result.new_wire->which_in = in_edit_proc->new_process.in_count;
        in_edit_proc->new_process.in_count += 1;
      }
      else {
        Editable_Process new_in = get_editable_process(wit->do_undo.edit_list, in);
        result.new_wire->which_in = new_in.process.in_count;
        new_in.process.in_count += 1;
        add_process_to_process_edit_list(wit, in, Proc_Trie_Edit_Update, new_in.process);
      }

      result.out = out;
      result.in = in;

      Set_Flag(result.new_wire->flags, Process_Flag_Wire);
      result.new_wire->out = result.out;
      result.new_wire->in = result.in;
    }
  }

  return result;
}


function Connection_Result connect_processes(
  WhatIsThis *wit,
  Process *out,
  Process *in
  ) {
  Connection_Result result = connect_processes_no_gather(wit, out, in);

  gather_processes_from_trie(wit);

  return result;
}





/////////////////////////////////////////
// Piece Table BEGIN ////////////////////
/////////////////////////////////////////
function Piece_Table_Row *piece_table_create_row(WhatIsThis *wit) {
  Piece_Table_Row *row = 0;

  if (wit->piece_table_memory.free_rows) {
    row = wit->piece_table_memory.free_rows;
    SLLStackPop(wit->piece_table_memory.free_rows);
    *row = (Piece_Table_Row){0};
  }
  else {
    row = push_struct(wit->permanent_arena, Piece_Table_Row);
  }

  return row;
}



function Piece_Table_Chunk *piece_table_create_chunk(WhatIsThis *wit) {
  Piece_Table_Chunk *chunk = 0;

  if (wit && wit->permanent_arena) {
    if (wit->piece_table_memory.free_chunks) {
      chunk = wit->piece_table_memory.free_chunks;
      SLLStackPop(wit->piece_table_memory.free_chunks);
      *chunk = (Piece_Table_Chunk){0};
    }
    else {
      chunk = push_struct(wit->permanent_arena, Piece_Table_Chunk);
    }
  }

  return chunk;
}



function B32 piece_table_ensure_insertion_chunk_exists(
  WhatIsThis *wit,
  Piece_Table *table
  ) {
  B32 error = 0;

  if (table->insertion_chunk == 0) {
    table->insertion_chunk = piece_table_create_chunk(wit);
    if (table->insertion_chunk == 0) {
      error = 1;
    }
  }

  return error;
}



function void piece_table_insert_text_after_row(
  WhatIsThis *wit,
  Piece_Table *table,
  Piece_Table_Row *row,
  String8 text_to_insert
  ) {
  U64 amount_of_text_copied = 0;
  Piece_Table_Row *current_row = row;

  while (amount_of_text_copied < text_to_insert.size) {
    // Create a new insertion-chunk if we need one.
    if (table->insertion_chunk->offset == Piece_Table_Chunk_Size) {
      table->insertion_chunk = piece_table_create_chunk(wit);
      if (table->insertion_chunk == 0) {
        printf("[ Error ] Creating Piece_Table_Chunk while inserting text after a row.\n");
        break;
      }
    }

    U64 remaining_amount_to_write = text_to_insert.size - amount_of_text_copied;
    U64 space_in_chunk = Piece_Table_Chunk_Size - table->insertion_chunk->offset;

    U64 amount_to_write;
    if (remaining_amount_to_write <= space_in_chunk) {
      amount_to_write = remaining_amount_to_write;
    }
    else {
      amount_to_write = space_in_chunk;
    }

    // copy the text
    MemoryCopy(table->insertion_chunk->str_array + table->insertion_chunk->offset,
               text_to_insert.str + amount_of_text_copied,
               amount_to_write);

    if (row &&
        (row->chunk == table->insertion_chunk) &&
        (row->offset + row->size == table->insertion_chunk->offset)) {
      // extend the row
      row->size += amount_to_write;
    }
    else {
      // insert the row
      Piece_Table_Row *new_row = piece_table_create_row(wit);
      if (new_row) {
        new_row->chunk = table->insertion_chunk;
        new_row->offset = table->insertion_chunk->offset;
        new_row->size = amount_to_write;
        Assert(new_row->size != 0);
        if (table->first_row == 0 || table->last_row == 0 || current_row == 0) {
          DLLPushFront(table->first_row, table->last_row, new_row);
          current_row = new_row;
        }
        else {
          DLLInsert(table->first_row, table->last_row, current_row, new_row);
        }
      }
      else {
        printf("[ Error ] Creating Piece_Table_Row while inserting text after a row.\n");
        break;
      }
    }

    amount_of_text_copied += amount_to_write;
    table->insertion_chunk->offset += amount_to_write;
    table->text_size += amount_to_write;
  }
}



function void piece_table_insert(
  WhatIsThis *wit,
  Piece_Table *table,
  U64 text_offset,
  String8 text_to_insert
  ) {
  U64 current_text_offset = 0;

  if (piece_table_ensure_insertion_chunk_exists(wit, table)) {
    printf("[ Error ] Ensuring piece-table has an insertion-chunk while inserting.\n");
    return;
  }

  if (table->first_row && table->last_row) {
    // insert text into existing rows
    List_For(Piece_Table_Row *, row, table->first_row) {
      current_text_offset += row->size;
      Piece_Table_Row *row_before = text_offset == 0 ? 0 : row;

      if (text_offset == 0 || current_text_offset == text_offset) {
        // insert text between rows
        piece_table_insert_text_after_row(wit, table, row_before, text_to_insert);
        break;
      }
      else if (current_text_offset > text_offset) {
        U64 last_part_size = current_text_offset - text_offset;
        U64 first_part_size = row->size - last_part_size;
        // adjust size of current row
        row->size = first_part_size;
        if (last_part_size) {
          // split row and insert text
          Piece_Table_Row *new_row = piece_table_create_row(wit);
          if (new_row) {
            // insert last part of current row as new row
            new_row->chunk = row->chunk;
            new_row->offset = row->offset + first_part_size;
            new_row->size = last_part_size;
            DLLInsert(table->first_row, table->last_row, row, new_row);
          }
          else {
            printf("[ Error ] Creating Piece_Table_Row while inserting text.\n");
            break;
          }
        }
        // insert text
        piece_table_insert_text_after_row(wit, table, row_before, text_to_insert);
        break;
      }
    }
  }
  else {
    // table is empty, so just insert the text
    piece_table_insert_text_after_row(wit, table, 0, text_to_insert);
  }
}



function void piece_table_delete(
  WhatIsThis *wit,
  Piece_Table *table,
  U64 text_offset,
  U64 size
  ) {
  B32 is_deleting = 0;
  U64 current_text_offset = 0;
  U64 begin_text_offset = text_offset >= size ? text_offset - size : 0;

  if (piece_table_ensure_insertion_chunk_exists(wit, table)) {
    printf("[ Error ] Ensuring piece-table has an insertion-chunk while inserting.\n");
    return;
  }

  Piece_Table_Row *delete_start_row = 0;

  // search for first row to begin deleting
  List_For(Piece_Table_Row *, row, table->first_row) {
    current_text_offset += row->size;

    if (current_text_offset == begin_text_offset) {
      delete_start_row = row->next;
      break;
    }
    else if (current_text_offset > begin_text_offset) {
      Assert(current_text_offset >= row->size);
      U64 offset_at_start_of_row = current_text_offset - row->size;
      Assert(begin_text_offset >= offset_at_start_of_row);
      U64 amount_to_the_left_of_text_offset = text_offset - offset_at_start_of_row;
      Assert(row->size >= amount_to_the_left_of_text_offset);
      U64 amount_to_the_right_of_text_offset = row->size - amount_to_the_left_of_text_offset;
      if (amount_to_the_left_of_text_offset > size) {
        // split the row in two
        row->size = amount_to_the_left_of_text_offset - size;
        Assert(table->text_size >= size);
        table->text_size -= size;
        if (row->size == 0) {
          // row is empty, so remove it
          DLLRemove(table->first_row, table->last_row, row);
          SLLStackPush(wit->piece_table_memory.free_rows, row);
        }
        else {
          if (amount_to_the_right_of_text_offset > 0) {
            Piece_Table_Row *new_row = piece_table_create_row(wit);
            if (new_row) {
              new_row->chunk = row->chunk;
              new_row->offset = row->offset + row->size + size;
              new_row->size = amount_to_the_right_of_text_offset;
              DLLInsert(table->first_row, table->last_row, row, new_row);
            }
            else {
              printf("[ Error ] Creating new row while deleting text.\n");
              break;
            }
          }
        }
      }
      else {
        // decrease the row size
        if (row->size == amount_to_the_left_of_text_offset) {
          // row is empty, so remove it
          DLLRemove(table->first_row, table->last_row, row);
          SLLStackPush(wit->piece_table_memory.free_rows, row);
        }
        else {
          row->size -= amount_to_the_left_of_text_offset;
          row->offset += amount_to_the_left_of_text_offset;
        }
        Assert(table->text_size >= amount_to_the_left_of_text_offset);
        table->text_size -= amount_to_the_left_of_text_offset;
      }
      break;
    }
  }

  // delete some rows
  List_For(Piece_Table_Row *, row, delete_start_row) {
    current_text_offset += row->size;

    if (current_text_offset > text_offset) {
      Assert(current_text_offset >= text_offset);
      U64 non_deleted_size = current_text_offset - text_offset;
      Assert(row->size >= non_deleted_size);
      U64 deleted_size = row->size - non_deleted_size;
      row->offset += deleted_size;
      row->size = non_deleted_size;
      Assert(table->text_size >= size);
      table->text_size -= deleted_size;
      break;
    }
    else {
      Assert(table->text_size >= row->size);
      table->text_size -= row->size;
      DLLRemove(table->first_row, table->last_row, row);
      SLLStackPush(wit->piece_table_memory.free_rows, row);
      if (current_text_offset == text_offset) {
        break;
      }
    }
  }
}



function String8 piece_table_get_string(Arena *arena, Piece_Table *table) {
  String8 string = (String8){0};
  U64 amount_written = 0;

  if (table && table->text_size) {
    string.str = arena_push(arena, table->text_size+1);
    string.size = table->text_size;
    if (string.str) {
      B32 error = 0;
      List_For(Piece_Table_Row *, row, table->first_row) {
        if (amount_written + row->size > table->text_size) {
          printf("[ Error ] Amount of text in piece-table is greater than the given piece-table's text-size. Getting c-string from piece-table.\n");
          error = 1;
          break;
        }
        else {
          MemoryCopy(string.str + amount_written,
                     row->chunk->str_array + row->offset,
                     row->size);
          amount_written += row->size;
        }
      }
      if (error) {
        string = (String8){0};
      }
      else {
        string.str[amount_written] = 0;
      }
    }
    else {
      printf("[ Error ] Pushing c-string while getting c-string for piece-table.\n");
    }
  }

  return string;
}



function void debug_print_piece_table(Piece_Table *table) {
  printf("Piece Table %p\n", table);
  printf("  insertion_chunk %p\n", table?table->insertion_chunk:0);
  if (table) {
    List_For(Piece_Table_Row *, row, table->first_row) {
      printf("  row %p chunk %p %llu %llu\n", row, row->chunk, row->offset, row->size);
    }
  }
  printf("\n");
}



function void debug_print_piece_table_range(WhatIsThis *wit, Piece_Table *table) {
  String8 string = piece_table_get_string(wit->per_frame_arena, table);
  printf("%s\n", string.str);
}



function void debug_check_piece_table(Context *context, Piece_Table *table) {
  if (table) {
    U64 text_size_from_rows = 0;
    List_For(Piece_Table_Row *, row, table->first_row) {
      Assert(row->size > 0);
      text_size_from_rows += row->size;
    }
    Assert(text_size_from_rows == table->text_size);
  }
}
/////////////////////////////////////////
// Piece Table END //////////////////////
/////////////////////////////////////////
