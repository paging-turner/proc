/*
  The structure is meant to hold 64-bit keys, allowing for insertion, deletion, and searching.
  It is best if the keys are relatively sequential, otherwise it will *not* be memory efficient.

  Honestly, I don't know if this is a trie or a radix tree or *what*... so maybe I should just give it some other kind of name and not worry about whether or not the name am correct.

 TODO:
   [ ] Figure out a better name than just "trie", since "trie" is too generic.
   [ ] Use memory pool for stack-nodes
   [-] Make persistant
     [x] Undo
     [x] Simple Redo
     [ ] Redo alternate branches
     [ ] Do a pass of docs after fixing bugs for undo/redo.
   [x] Allow for identifier renaming (for constructing multiple types of trie)
   [x] Allow configuration to store keys least-to-most significant byte or most-to-least.
   [x] Right now the keys are the values, but we probably want the option of having key/value pairs.
   [ ] BUG: Steady_Trie_Slot_Bits == 2  and  Steady_Trie_Root_Is_Lowest_Significant_Byte == 0 causes errors.
*/



////////////////////////////////////////
// Settings (see Undefining Settings) //
////////////////////////////////////////
#ifndef  Steady_Trie
# define Steady_Trie(identifier)  Steady_Trie_##identifier
#endif

#ifndef  steady_trie
# define steady_trie(identifier)  steady_trie_##identifier
#endif

#ifndef  Steady_Trie_Key_Bits
# define Steady_Trie_Key_Bits 64
#endif

#ifndef  Steady_Trie_Slot_Bits
# define Steady_Trie_Slot_Bits 2
#endif

#ifndef  Steady_Trie_Root_Is_Lowest_Significant_Byte
# define Steady_Trie_Root_Is_Lowest_Significant_Byte 1
#endif

#ifndef Steady_Trie_Use_Key_Value_Pair
# define Steady_Trie_Use_Key_Value_Pair 0
#endif

#ifndef Steady_Trie_Use_Debug_Log
# define Steady_Trie_Use_Debug_Log 0
#endif



////////////////////////////
// Api
////////////////////////////
// TODO: Fill out function/macro defs






////////////////////////////
// Computed Properties
////////////////////////////
#if Steady_Trie_Key_Bits == 64
typedef U64 Steady_Trie(Key);
#elif Steady_Trie_Key_Bits == 32
typedef U32 Steady_Trie(Key);
#else
# error Steady_Trie_Key_Bits can only be 32 or 64.
#endif

typedef U8 Steady_Trie(Slot_Type);

#define Steady_Trie_Slot_Count\
  (1 << Steady_Trie_Slot_Bits)

#define Steady_Trie_Max_Depth\
  (U32)(Steady_Trie_Key_Bits / Steady_Trie_Slot_Bits)

// TODO: The names for Steady_Trie_Single_Slot_Mask and Steady_Trie_Slot_Mask are confusing. Should probably rename!
#define Steady_Trie_Single_Slot_Mask\
  ((1<<Steady_Trie_Slot_Bits)-1)




////////////////////////////
// Helper Macros
////////////////////////////
#if Steady_Trie_Root_Is_Lowest_Significant_Byte
# define Steady_Trie_Slot_Mask(depth)\
  (~(((U64)1<<((depth+1)*Steady_Trie_Slot_Bits))-1))

# define Steady_Trie_Get_Slot_Shift(depth)\
  (depth)

# define Steady_Trie_Get_Next_Iter_Key_Chunk(key, key_shift, index)\
  (((key) << Steady_Trie_Slot_Bits) | ((index)-1))
#else
# define Steady_Trie_Slot_Mask(depth)\
  (~(max_U64<<(((Steady_Trie_Max_Depth-1)-depth)*Steady_Trie_Slot_Bits)))

# define Steady_Trie_Get_Slot_Shift(depth)\
  ((Steady_Trie_Max_Depth-1)-(depth))

# define Steady_Trie_Get_Next_Iter_Key_Chunk(key, key_shift, index)\
  ((key) | ((U64)((index)-1) << ((key_shift)*Steady_Trie_Slot_Bits)))
#endif // Steady_Trie_Root_Is_Lowest_Significant_Byte

#define Steady_Trie_Get_Initial_Iter_Key_Chunk(index)\
  (index)

#define Steady_Trie_Get_Slot_Value(key, depth)\
  (((key) >> (Steady_Trie_Get_Slot_Shift(depth)*Steady_Trie_Slot_Bits)) &\
   Steady_Trie_Single_Slot_Mask)

#define Steady_Trie_Is_Max_Depth(depth)\
  ((depth)==Steady_Trie_Max_Depth-1)

#define Steady_Trie_Is_Key_At_Final_Depth(key, depth)\
  (Steady_Trie_Is_Max_Depth(depth) || ((Steady_Trie_Slot_Mask(depth) & key) == 0))

#define Steady_Function static


#if Steady_Trie_Use_Debug_Log
# define Steady_Trie_Debug_Print(...)  printf(__VA_ARGS__)
#else
# define Steady_Trie_Debug_Print(...)
#endif


////////////////////////////
// Static Invariants
////////////////////////////
StaticAssert((Steady_Trie_Slot_Bits < Steady_Trie_Key_Bits),
             Ensure_Slot_Smaller_Than_Key);
StaticAssert((Steady_Trie_Key_Bits % Steady_Trie_Slot_Bits) == 0,
             Ensure_Slot_Divides_Key);
StaticAssert((8*sizeof(Steady_Trie(Slot_Type)) >= Steady_Trie_Slot_Bits),
             Ensure_Slot_Type_Fits_Slot_Bits);
StaticAssert((8*sizeof(Steady_Trie(Key)) == Steady_Trie_Key_Bits),
             Ensure_Key_Type_Equals_Key_Bits);

#ifndef Steady_Trie_Value_Type
# if Steady_Trie_Use_Key_Value_Pair
#  warning When using Steady_Trie_Use_Key_Value_Pair, Steady_Trie_Value_Type must also be defined, setting the type to be U64.
# endif
# define Steady_Trie_Value_Type U64
typedef U64 Steady_Trie(Value_Type);
#endif

#ifndef Steady_Trie_Default_Value
# if Steady_Trie_Use_Key_Value_Pair
#  warning When using Steady_Trie_Use_Key_Value_Pair, Steady_Trie_Default_Value must also be defined, setting the default to 0.
# endif
# define Steady_Trie_Default_Value 0
#endif

#if 0
# if Steady_Trie_Use_Key_Value_Pair
// Define steady_trie(values_equal) without an implementation to force the user to define the function with the custom types.
static B32 steady_trie(values_equal)(Steady_Trie_Value_Type v1, Steady_Trie_Value_Type v2);
# endif
#endif





typedef struct Steady_Trie(Node) {
  U8 occupied[Steady_Trie_Slot_Count]; // TODO: Bit-flags?
  struct Steady_Trie(Node) *slots[Steady_Trie_Slot_Count];
#if Steady_Trie_Use_Key_Value_Pair
  Steady_Trie_Value_Type values[Steady_Trie_Slot_Count];
#endif

  Steady_Trie_Value_Type *ref;
} Steady_Trie(Node);

typedef struct Steady_Trie(Root) {
  struct Steady_Trie(Root) *next_edit;
  struct Steady_Trie(Root) *prev_edit;
  struct Steady_Trie(Root) *next_branch;
  struct Steady_Trie(Root) *prev_branch;
  Steady_Trie(Node) *node;

  Steady_Trie_Value_Type *ref;
} Steady_Trie(Root);

typedef struct Steady_Trie(Root_Stack) {
  Steady_Trie(Root) *root;
  struct Steady_Trie(Root_Stack) *next;
} Steady_Trie(Root_Stack);

typedef struct Steady_Trie(Settings) {
  U32 key_bits;
  U32 slot_bits;
  B32 root_is_lowest_significant_byte;
  B32 use_key_value_pair;
  U32 slot_count;
  U32 max_depth;
} Steady_Trie(Settings);

typedef struct Steady_Trie(Trie) {
  Steady_Trie(Root) *root;
  Steady_Trie(Root) *current_root;
  Steady_Trie(Root) *edit_root;
  Steady_Trie(Settings) settings;

  Steady_Trie_Value_Type *ref;
} Steady_Trie(Trie);


typedef struct Steady_Trie(Stack_Node) {
  struct Steady_Trie(Stack_Node) *next;
  Steady_Trie(Node) *node;
  U32 index;
  U32 visited_plus_one;
} Steady_Trie(Stack_Node);

typedef struct Steady_Trie(Iterator) {
  Arena *arena;
  Steady_Trie(Stack_Node) *stack;
  Steady_Trie(Stack_Node) *free_stack; // TODO: Use free stack-nodes!

  Steady_Trie(Key) key;
} Steady_Trie(Iterator);

typedef enum Steady_Trie(Edit_Kind) {
  Steady_Trie(Edit_Insert),
  Steady_Trie(Edit_Delete),
  Steady_Trie(Edit_Search),
} Steady_Trie(Edit_Kind);

typedef struct Steady_Trie(Edit_Result) {
  B32 found;
  Steady_Trie_Value_Type *value;
} Steady_Trie(Edit_Result);




Steady_Function char *steady_trie(get_edit_kind_name)(
  Steady_Trie(Edit_Kind) edit_kind
  ) {
  char *name = "";

  switch (edit_kind) {
  case Steady_Trie(Edit_Insert): name = "Insert"; break;
  case Steady_Trie(Edit_Delete): name = "Delete"; break;
  case Steady_Trie(Edit_Search): name = "Search"; break;
  default: Assert(0);
  }

  return name;
}




Steady_Function Steady_Trie_Value_Type steady_trie(get_default_value)(void) {
  return Steady_Trie_Default_Value;
}


Steady_Function Steady_Trie(Stack_Node) *steady_trie(create_stack_node)(
  Arena *arena,
  Steady_Trie(Stack_Node) *free_stack
  ) {
  Steady_Trie(Stack_Node) *node = 0;
  // TODO: Implement and use

  return node;
}

Steady_Function void steady_trie(delete_stack_node)(
  Arena *arena,
  Steady_Trie(Stack_Node) *free_stack,
  Steady_Trie(Stack_Node) *node
  ) {
  // TODO: Implement and use
}


Steady_Function void steady_trie(iter_next)(Steady_Trie(Iterator) *iter) {
  // Do a depth-first search until we find the next occupied key.
  for (;;) {
    if (iter->stack && iter->stack->node) {
      if (iter->stack->index < Steady_Trie_Slot_Count) {
        B32 not_visited = ((iter->stack->visited_plus_one == 0) ||
                           (iter->stack->visited_plus_one-1 < iter->stack->index));
        B32 occupied = iter->stack->node->occupied[iter->stack->index];
        if (occupied && not_visited) {
          // NOTE: key_shift is only used for when the root is the most-significant byte.
          U64 key = Steady_Trie_Get_Initial_Iter_Key_Chunk(iter->stack->index);
          // Build up the key value by collecting all the indices on the stack.
          for (Steady_Trie(Stack_Node) *stack_node = iter->stack->next;
               stack_node != 0;
               stack_node = stack_node->next) {
            // NOTE: All of the nodes in the stack, aside from the current one, have had their indices iterated, so we need to subtract one from them. (This is alway why we do not process the current stack-node in the loop.)
            key = Steady_Trie_Get_Next_Iter_Key_Chunk(key, key_shift, stack_node->index);
          }
#if !Steady_Trie_Root_Is_Lowest_Significant_Byte
          // fill in zeros at the end
          key = key << ((Steady_Trie_Max_Depth - depth) * Steady_Trie_Slot_Bits);
#endif
          iter->key = key;
          iter->stack->visited_plus_one = iter->stack->index+1;
          break;
        }
        else if (iter->stack->node->slots[iter->stack->index]) {
          // Child slot is present, so descend.
          // TODO: Recycle free stack-nodes!!!!!
          Steady_Trie(Node) *next_node = iter->stack->node->slots[iter->stack->index];
          Steady_Trie(Stack_Node) *new_stack_node = arena_push(iter->arena, sizeof(Steady_Trie(Stack_Node)));
          iter->stack->index += 1;
          new_stack_node->node = next_node;
          SLLStackPush(iter->stack, new_stack_node);
        }
        else {
          iter->stack->index += 1;
        }
      }
      else {
        // Reached the end of the current node's slots, so go back up.
        SLLStackPop(iter->stack); // TODO: Recycle stack-nodes!!!!!!!!
      }
    }
    else {
      // The iter-stack or the stack's node is empty, so bail.
      break;
    }
  }
}


Steady_Function Steady_Trie(Iterator) *steady_trie(iter_init)(
  Arena *arena,
  Steady_Trie(Node) *root
  ) {
  Steady_Trie(Iterator) *iter = arena_push(arena, sizeof(Steady_Trie(Iterator)));
  Steady_Trie(Stack_Node) *stack = arena_push(arena, sizeof(Steady_Trie(Stack_Node)));

  if (iter && stack) {
    iter->arena = arena;
    iter->stack = stack;
    iter->stack->node = root;
  }

  steady_trie(iter_next)(iter);

  return iter;
}


Steady_Function B32 steady_trie(iter_test)(Steady_Trie(Iterator) *iter) {
  return iter && iter->stack;
}




Steady_Function void steady_trie(crawl_roots)(
  Arena *temp_arena,
  Steady_Trie(Trie) *trie,
  void (*handle_root)(Steady_Trie(Root) *root)
  ) {
  U64 pos = arena_current_pos(temp_arena);
  Steady_Trie(Root_Stack) *stack = arena_push(temp_arena, sizeof(Steady_Trie(Root_Stack)));
  if (stack) {
    stack->root = trie->root;
  }

  for (; stack != 0 && stack->root;) {
    handle_root(stack->root);

    if (stack->root->next_edit) {
      if (arena_has_space_for(temp_arena, sizeof(Steady_Trie(Root_Stack)))) {
        Steady_Trie(Root_Stack) *new_stack = arena_push(temp_arena, sizeof(Steady_Trie(Root_Stack)));
        stack->root = new_stack->root = stack->root->next_edit;
        SLLStackPush(stack, new_stack);
        stack->root = new_stack->root;
      }
      else {
        Steady_Trie_Debug_Print("[ ERROR ] Pushing Steady_Trie(Root_Stack) in steady_trie(crawl_roots)\n");
        break;
      }
    }
    else if (stack->root->next_branch) {
      stack->root = stack->root->next_branch;
    }
    else {
      SLLStackPop(stack);

      if (stack) {
        stack->root = stack->root->next_branch;
      }
    }
  }

  arena_pop_to(temp_arena, pos);
}




// TODO: Do we need to rename new_root_with_keys?
Steady_Function void steady_trie(new_root_with_keys)(
  Arena *arena,
  Steady_Trie(Trie) *trie,
  Steady_Trie(Key) *keys,
  U32 key_count
  ) {
  Steady_Trie_Debug_Print("new_root_with_keys\n");
  Steady_Trie(Root) *new_root = trie->edit_root;
  B32 began_with_no_edit_root = trie->edit_root == 0;

  if (began_with_no_edit_root) {
    new_root = arena_push(arena, sizeof(Steady_Trie(Root)));
    trie->edit_root = new_root;
  }

  if (trie->current_root && new_root) {
    Steady_Trie(Node) *new_node = trie->edit_root ? trie->edit_root->node : 0;

    if (began_with_no_edit_root) {
      new_node = arena_push(arena, sizeof(Steady_Trie(Node)));

      // Setup new node.
      *new_node = *trie->current_root->node;
      new_root->node = new_node;

      // Push new root.
      if (trie->current_root->next_edit) {
        // @Speed
        Steady_Trie(Root) *last_branch = trie->current_root;
        for (;last_branch->next_branch != 0;) {
          last_branch = last_branch->next_branch;
        }

        last_branch->next_branch = new_root;
        new_root->prev_branch = last_branch;
      }
      else {
        trie->current_root->next_edit = new_root;
        new_root->prev_edit = trie->current_root;
      }

      // Set current root
      trie->current_root = new_root;
    }

    // Fill out the root with new nodes.
    for (U32 k = 0; k < key_count; ++k) {
      Steady_Trie(Key) key = keys[k];

      // TODO: `current_node` and `current_new_node` are confusing names.
      Steady_Trie(Node) *current_node = trie->current_root->node;
      Steady_Trie(Node) *current_new_node = new_node;

      for (U32 d = 0; d < Steady_Trie_Max_Depth; ++d) {
        Steady_Trie(Slot_Type) slot_value = Steady_Trie_Get_Slot_Value(key, d);
        Assert(slot_value >= 0 && slot_value <= Steady_Trie_Single_Slot_Mask);
        B32 final_depth = Steady_Trie_Is_Key_At_Final_Depth(key, d);

        if (!final_depth && current_node->slots[slot_value] && current_new_node->slots[slot_value]) {
          // Copy and descend.
          Steady_Trie(Node) *new_node = arena_push(arena, sizeof(Steady_Trie(Node)));
          if (new_node) {
            current_node = current_node->slots[slot_value];
            *new_node = *current_node;
            current_new_node->slots[slot_value] = new_node;
            current_new_node = new_node;
          }
          else {
            // Arena failure.
            break;
          }
        }
        else {
          // Current trie ends here.
          break;
        }
      }
    }
  }
}


Steady_Function void steady_trie(new_root)(
  Arena *arena,
  Steady_Trie(Trie) *trie,
  Steady_Trie(Key) key
  ) {
  Steady_Trie_Debug_Print("new_root\n");
  Steady_Trie(Key) keys[] = {key};
  steady_trie(new_root_with_keys)(arena, trie, keys, 1);
}


Steady_Function Steady_Trie(Edit_Result) steady_trie(edit)(
  Arena *arena,
  Steady_Trie(Trie) *trie,
  Steady_Trie(Key) *keys,
  Steady_Trie_Value_Type *values,
  U32 count,
  Steady_Trie(Edit_Kind) edit_kind
  ) {
  Steady_Trie_Debug_Print("edit %s\n", steady_trie(get_edit_kind_name)(edit_kind));
  Steady_Trie(Edit_Result) result = (Steady_Trie(Edit_Result)){0};
  if (edit_kind != Steady_Trie(Edit_Search) && trie->edit_root == 0) {
    steady_trie(new_root_with_keys)(arena, trie, keys, count);
  }

  for (U32 i = 0; i < count; ++i) {
    Steady_Trie(Node) *node = trie->current_root->node;
    Steady_Trie(Key) key = keys[i];

    for (U32 d = 0; (d < Steady_Trie_Max_Depth) && node; ++d) {
      Steady_Trie(Slot_Type) slot_value = Steady_Trie_Get_Slot_Value(key, d);
      Assert(slot_value >= 0 && slot_value <= Steady_Trie_Single_Slot_Mask);

      if (Steady_Trie_Is_Key_At_Final_Depth(key, d)) {
        if (edit_kind == Steady_Trie(Edit_Insert)) {
          node->occupied[slot_value] = 1;
#if Steady_Trie_Use_Key_Value_Pair
          if (values) {
            Steady_Trie_Value_Type value = values[i];
            node->values[slot_value] = value;
          }
#endif
        }
        else if (edit_kind == Steady_Trie(Edit_Delete)) {
          node->occupied[slot_value] = 0;
        }
        else if (edit_kind == Steady_Trie(Edit_Search)) {
#if Steady_Trie_Use_Key_Value_Pair
          if (node->occupied[slot_value]) {
            result.value = &node->values[slot_value];
          }
#else
          result.found = node->occupied[slot_value] ? 1 : 0;
#endif
        }
        break;
      }
      else {
        if (edit_kind == Steady_Trie(Edit_Insert) && node->slots[slot_value] == 0) {
          // Add new node
          node->slots[slot_value] = arena_push(arena, sizeof(Steady_Trie(Node)));
        }

        // Descend
        node = node->slots[slot_value];
      }
    }
  }

  return result;
}



#if Steady_Trie_Use_Key_Value_Pair
Steady_Function Steady_Trie(Edit_Result) steady_trie(set)(
  Arena *arena,
  Steady_Trie(Trie) *trie,
  Steady_Trie(Key) key,
  Steady_Trie_Value_Type *value
  ) {
  Steady_Trie_Debug_Print("set\n");
  return steady_trie(edit)(arena, trie, &key, value, 1, Steady_Trie(Edit_Insert));
}
#else
Steady_Function Steady_Trie(Edit_Result) steady_trie(insert)(
  Arena *arena,
  Steady_Trie(Trie) *trie,
  Steady_Trie(Key) key
  ) {
  Steady_Trie_Debug_Print("insert\n");
  Steady_Trie_Value_Type value = steady_trie(get_default_value)();
  return steady_trie(edit)(arena, trie, &key, value, 1, Steady_Trie(Edit_Insert));
}
#endif

Steady_Function Steady_Trie(Edit_Result) steady_trie(delete)(
  Arena *arena,
  Steady_Trie(Trie) *trie,
  Steady_Trie(Key) key
  ) {
  Steady_Trie_Debug_Print("delete\n");
  Steady_Trie_Value_Type value = steady_trie(get_default_value)();
  return steady_trie(edit)(arena, trie, &key, &value, 1, Steady_Trie(Edit_Delete));
}

Steady_Function Steady_Trie(Edit_Result) steady_trie(search)(
  Arena *arena,
  Steady_Trie(Trie) *trie,
  Steady_Trie(Key) key
  ) {
  Steady_Trie_Debug_Print("search\n");
  Steady_Trie_Value_Type value = steady_trie(get_default_value)();
  return steady_trie(edit)(arena, trie, &key, &value, 1, Steady_Trie(Edit_Search));
}


Steady_Function Steady_Trie(Trie) *steady_trie(create_trie)(Arena *arena) {
  Steady_Trie_Debug_Print("create_trie\n");
  Steady_Trie(Trie) *trie = arena_push(arena, sizeof(Steady_Trie(Trie)));
  Steady_Trie(Node) *node = arena_push(arena, sizeof(Steady_Trie(Node)));
  Steady_Trie(Root) *root = arena_push(arena, sizeof(Steady_Trie(Root)));

  if (trie && node) {
    root->node = node;
    trie->root = root;
    trie->current_root = root;

    trie->settings.key_bits = Steady_Trie_Key_Bits;
    trie->settings.slot_bits = Steady_Trie_Slot_Bits;
    trie->settings.root_is_lowest_significant_byte = Steady_Trie_Root_Is_Lowest_Significant_Byte;
    trie->settings.use_key_value_pair = Steady_Trie_Use_Key_Value_Pair;
    trie->settings.slot_count = Steady_Trie_Slot_Count;
    trie->settings.max_depth = Steady_Trie_Max_Depth;
  }
  else {
    trie = 0;
  }

  return trie;
}




Steady_Function void steady_trie(undo)(Steady_Trie(Trie) *trie) {
  Steady_Trie_Debug_Print("undo\n");
  if (trie->current_root->prev_edit != 0) {
    trie->current_root = trie->current_root->prev_edit;
  }
  else if (trie->current_root->prev_branch != 0) {
    // @Speed
    Steady_Trie(Root) *first_branch = trie->current_root;
    for (;first_branch->prev_branch != 0;) {
      first_branch = first_branch->prev_branch;
    }

    trie->current_root = first_branch;
  }
}


Steady_Function void steady_trie(redo)(Steady_Trie(Trie) *trie) {
  Steady_Trie_Debug_Print("redo\n");
  // TODO: Allow for redoing other branches.
  if (trie->current_root->next_branch != 0) {
    // @Speed
    Steady_Trie(Root) *last_branch = trie->current_root;
    for (;last_branch->next_branch != 0;) {
      last_branch = last_branch->next_branch;
    }
    trie->current_root = last_branch;
  }
  else if (trie->current_root->next_edit != 0) {
    trie->current_root = trie->current_root->next_edit;
  }
}



Steady_Function void steady_trie(handle_root_crawl)(Steady_Trie(Root) *root) {
  Steady_Trie_Debug_Print("  root = %p\n", root);
}

Steady_Function void steady_trie(commit)(Steady_Trie(Trie) *trie) {
  Steady_Trie_Debug_Print("commit\n");
  Steady_Trie_Debug_Print("\n");

  if (trie->edit_root) {
    trie->current_root = trie->edit_root;
    trie->edit_root = 0;
  }
}





/////////////////////////////////////
// Tests
/////////////////////////////////////

Steady_Function B32 steady_trie(ensure_key_has_occupation)(
  Arena *arena,
  Steady_Trie(Trie) *trie,
  Steady_Trie(Key) key,
  B32 occupation
  ) {
  B32 errors = 0;

  if (trie->settings.use_key_value_pair) {
    Steady_Trie(Edit_Result) search_result = steady_trie(search)(arena, trie, key);
    Steady_Trie_Value_Type *value = search_result.value;

    if (occupation && !value) {
      Steady_Trie_Debug_Print("[ Error ] Null value at key %llu\n", (U64)key);
      errors = 1;
    }
#if 0
# if Steady_Trie_Use_Key_Value_Pair
    // TODO: Check that values are equal.
    else if (occupation && !steady_trie(values_equal)(*value, steady_trie(get_default_value)())) {
      Steady_Trie_Debug_Print("[ Error ] Mismatched value at key %llu.\n", (U64)key);
      errors = 1;
    }
# endif
#endif
    else if (!occupation && value) {
      Steady_Trie_Debug_Print("[ Error ] Found a value at key %llu, but was not expecting a value.\n",
             (U64)key);
      errors = 1;
    }
  }
  else {
    Steady_Trie(Edit_Result) search_result = steady_trie(search)(arena, trie, key);

    if (search_result.found != occupation) {
      Steady_Trie_Debug_Print("[ Error ] Expected key %llu to have occupation %d but it has occupation %d\n",
             (U64)key, occupation, search_result.found);
      errors = 1;
    }
  }

  return errors;
}


#define Steady_Trie_Print_Connection(gen, a, b)\
  printf("\"%llx_%llx\"->\"%llx_%llx\";\n", gen, (U64)(a), gen, (U64)(b))


#define Steady_Trie_Print_Leaf(gen, a, b, stack_index)\
  printf("\"%llx_%llx\"->\"%llx_%llx_%d\";\n", gen, (U64)(a), gen, (U64)(b), stack_index)


Steady_Function void steady_trie(crawl_trie)(
  Arena *arena,
  Steady_Trie(Trie) *trie,
  void (*root_handler)(void *caller_data, Steady_Trie(Iterator) *iter, Steady_Trie(Root) *root),
  void (*node_handler)(void *caller_data, Steady_Trie(Iterator) *iter, Steady_Trie(Node) *node),
  void *caller_data
  ) {
  for (Steady_Trie(Root) *root = trie->root; root != 0; root = root->next_edit) {
    Steady_Trie(Node) *prev_node = 0;

    // Init iterator
    Steady_Trie(Iterator) *iter = arena_push(arena, sizeof(Steady_Trie(Iterator)));
    Steady_Trie(Stack_Node) *stack = arena_push(arena, sizeof(Steady_Trie(Stack_Node)));
    if (iter && stack) {
      iter->arena = arena;
      iter->stack = stack;
      iter->stack->node = root->node;

      root_handler(caller_data, iter, root);

      if (iter && iter->stack && iter->stack->node) {
        node_handler(caller_data, iter, iter->stack->node);
      }

      for (;;) {
        if (iter && iter->stack && iter->stack->node) {
          if (iter->stack->index < Steady_Trie_Slot_Count) {
            B32 not_visited = ((iter->stack->visited_plus_one == 0) ||
                               (iter->stack->visited_plus_one-1 < iter->stack->index));
            B32 occupied = iter->stack->node->occupied[iter->stack->index];

            if (occupied && not_visited) {
              iter->stack->visited_plus_one = iter->stack->index+1;
            }
            else if (iter->stack->node->slots[iter->stack->index]) {
              // Child slot is present, so descend.
              // TODO: Recycle free stack-nodes!!!!!
              Steady_Trie(Node) *next_node = iter->stack->node->slots[iter->stack->index];
              Steady_Trie(Stack_Node) *new_stack_node = arena_push(iter->arena, sizeof(Steady_Trie(Stack_Node)));
              iter->stack->index += 1;
              new_stack_node->node = next_node;
              SLLStackPush(iter->stack, new_stack_node);
              node_handler(caller_data, iter, next_node);
            }
            else {
              iter->stack->index += 1;
            }
          }
          else {
            // Reached the end of the current node's slots, so go back up.
            SLLStackPop(iter->stack); // TODO: Recycle stack-nodes!!!!!!!!
          }
        }
        else {
          // The iter-stack or the stack's node is empty, so bail.
          break;
        }
      }
    }
  }
}



Steady_Function void steady_trie(print_trie)(
  Arena *arena,
  Steady_Trie(Trie) *trie,
  U64 generation
  ) {
  printf("digraph Trie {\n");
  char *ref_format = "%llx_%llx";
  for (Steady_Trie(Root) *root = trie->root; root != 0; root = root->next_edit) {
    Steady_Trie_Print_Connection(generation, trie, root);
    Steady_Trie(Node) *prev_node = 0;

    // Init iterator
    Steady_Trie(Iterator) *iter = arena_push(arena, sizeof(Steady_Trie(Iterator)));
    Steady_Trie(Stack_Node) *stack = arena_push(arena, sizeof(Steady_Trie(Stack_Node)));
    if (iter && stack) {
      iter->arena = arena;
      iter->stack = stack;
      iter->stack->node = root->node;

      Steady_Trie_Print_Connection(generation, root, iter->stack->node);

      for (;;) {
        if (iter->stack && iter->stack && iter->stack->node) {
          if (iter->stack->index < Steady_Trie_Slot_Count) {
            B32 not_visited = ((iter->stack->visited_plus_one == 0) ||
                               (iter->stack->visited_plus_one-1 < iter->stack->index));
            B32 occupied = iter->stack->node->occupied[iter->stack->index];
            if (occupied && not_visited) {
              iter->stack->visited_plus_one = iter->stack->index+1;
              Steady_Trie_Print_Leaf(generation, iter->stack->node, iter->stack->node, iter->stack->index);
            }
            else if (iter->stack->node->slots[iter->stack->index]) {
              // Child slot is present, so descend.
              // TODO: Recycle free stack-nodes!!!!!
              Steady_Trie(Node) *next_node = iter->stack->node->slots[iter->stack->index];
              Steady_Trie(Stack_Node) *new_stack_node = arena_push(iter->arena, sizeof(Steady_Trie(Stack_Node)));
              iter->stack->index += 1;
              new_stack_node->node = next_node;
              Steady_Trie_Print_Connection(generation, iter->stack->node, next_node);
              SLLStackPush(iter->stack, new_stack_node);
            }
            else {
              iter->stack->index += 1;
            }
          }
          else {
            // Reached the end of the current node's slots, so go back up.
            SLLStackPop(iter->stack); // TODO: Recycle stack-nodes!!!!!!!!
          }
        }
        else {
          // The iter-stack or the stack's node is empty, so bail.
          break;
        }
      }
    }
  }
  printf("}\n");
}


Steady_Function U32 steady_trie(run_tests)(Arena *arena) {
  /* Arena *arena = arena_alloc_reserve(Steady_Trie_Test_Arena_Size, 0); */
  Steady_Trie(Trie) *trie = steady_trie(create_trie)(arena);
  U32 error_count = 0;

  U64 a = 189;
  U64 b = 242;
  U64 c = 42387468;
  U64 d;
  if (trie->settings.key_bits == 64) {
    d = 12370169555311111083LLU;
  }
  else {
    d = 1237;
  }
  U64 e = 123701695;
  U64 f = 9287349786368457;

  U64 keys_to_add[] = {a, b, c, d};
  U64 keys_to_delete[] = {b, d};

  printf("\n\n\n");
  printf("=================\n");
  printf("== Begin tests ==\n");
  printf("=================\n");

  printf("Inserting keys...");
  for (U32 i = 0; i < ArrayCount(keys_to_add); ++i) {
    Steady_Trie(Key) key = keys_to_add[i];
    printf("%llu ", key);
#if Steady_Trie_Use_Key_Value_Pair
    Steady_Trie_Value_Type default_value = steady_trie(get_default_value)();
    steady_trie(set)(arena, trie, key, &default_value);
#else
    steady_trie(insert)(arena, trie, key);
#endif
  }
  printf("\n");

  printf("Checking key occupancy...\n");
  error_count += steady_trie(ensure_key_has_occupation)(arena, trie, a, 1);
  error_count += steady_trie(ensure_key_has_occupation)(arena, trie, b, 1);
  error_count += steady_trie(ensure_key_has_occupation)(arena, trie, c, 1);
  error_count += steady_trie(ensure_key_has_occupation)(arena, trie, d, 1);
  error_count += steady_trie(ensure_key_has_occupation)(arena, trie, e, 0);
  error_count += steady_trie(ensure_key_has_occupation)(arena, trie, f, 0);

  {
    printf("Checking iterated keys...\n");
    U32 match_count = 0;
    for (Steady_Trie(Iterator) *iter_name = steady_trie(iter_init)(arena, trie->current_root->node);
         steady_trie(iter_test)(iter_name);
         steady_trie(iter_next)(iter_name)) {
      B32 match = 0;
      for (U32 i = 0; i < ArrayCount(keys_to_add); ++i) {
        if (iter_name->key == keys_to_add[i]) {
          match_count += 1;
          match = 1;
          break;
        }
      }
      if (!match) {
        printf("[ Error ] Found unmatched key while iterating, %llu\n", (U64)iter_name->key);
        error_count += 1;
      }
    }
    if (match_count != ArrayCount(keys_to_add)) {
      printf("[ Error ] Expected to match %lu keys while iterating but only matched %d\n", ArrayCount(keys_to_add), match_count);
      error_count += 1;
    }
  }

  printf("Deleting keys...");
  for (U32 i = 0; i < ArrayCount(keys_to_delete); ++i) {
    Steady_Trie(Key) key = keys_to_add[i];
    printf("%llu ", key);
    steady_trie(delete)(arena, trie, keys_to_delete[i]);
  }
  printf("\n");

  {
    printf("Checking iterated keys...\n");
    U32 match_count = 0;
    for (Steady_Trie(Iterator) *iter_name = steady_trie(iter_init)(arena, trie->current_root->node);
         steady_trie(iter_test)(iter_name);
         steady_trie(iter_next)(iter_name)) {
      B32 to_add_match = 0;
      for (U32 i = 0; i < ArrayCount(keys_to_add); ++i) {
        if (iter_name->key == keys_to_add[i]) {
          match_count += 1;
          to_add_match = 1;
          break;
        }
      }
      if (!to_add_match) {
        printf("[ Error ] Found unmatched key while iterating, %llu\n", (U64)iter_name->key);
        error_count += 1;
      }
      B32 to_delete_match = 0;
      for (U32 i = 0; i < ArrayCount(keys_to_delete); ++i) {
        if (iter_name->key == keys_to_delete[i]) {
          to_delete_match = 1;
          break;
        }
      }
      if (to_delete_match) {
        printf("[ Error ] Found matched key that should have been deleted, %llu\n", (U64)iter_name->key);
        error_count += 1;
      }
    }
    if (match_count != ArrayCount(keys_to_add) - ArrayCount(keys_to_delete)) {
      printf("[ Error ] Expected to match %lu keys while iterating but only matched %d\n", ArrayCount(keys_to_add)-ArrayCount(keys_to_delete), match_count);
      error_count += 1;
    }
  }

  {
    printf("Checking key occupancy...\n");
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, a, 1);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, b, 0);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, c, 1);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, d, 0);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, e, 0);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, f, 0);
  }

  {
    printf("Undo one delete...\n");
    printf("undo with prev_edit=%p\n          prev_branch=%p\n", trie->current_root->prev_edit, trie->current_root->prev_branch);
    steady_trie(undo)(trie);
    printf("     into prev_edit=%p\n          prev_branch=%p\n", trie->current_root->prev_edit, trie->current_root->prev_branch);
  }

  {
    printf("Checking key occupancy...\n");
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, a, 1);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, b, 0);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, c, 1);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, d, 1);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, e, 0);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, f, 0);
  }

  printf("Undo one-more delete...\n");
  steady_trie(undo)(trie);

  {
    printf("Checking key occupancy...\n");
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, a, 1);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, b, 1);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, c, 1);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, d, 1);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, e, 0);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, f, 0);
  }

  printf("Redo one delete...\n");
  steady_trie(redo)(trie);

  {
    printf("Checking key occupancy...\n");
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, a, 1);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, b, 0);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, c, 1);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, d, 1);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, e, 0);
    error_count += steady_trie(ensure_key_has_occupation)(arena, trie, f, 0);
  }

  printf("==================\n");
  printf("== End of tests ==\n");
  printf("==================\n");
  if (error_count) {
    printf("%d FAILURES.\n", error_count);
  }
  else {
    printf("Tests PASSED.\n");
  }

  return error_count;
}










////////////////////////////////////////
// Undefining Settings (see Settings) //
////////////////////////////////////////
#undef Steady_Trie
#undef steady_trie
#undef Steady_Trie_Key_Bits
#undef Steady_Trie_Slot_Bits
#undef Steady_Trie_Root_Is_Lowest_Significant_Byte
#undef Steady_Trie_Use_Key_Value_Pair
#undef Steady_Trie_Use_Debug_Log

#undef Steady_Trie_Debug_Print

#undef Steady_Trie_Slot_Mask
#undef Steady_Trie_Get_Slot_Shift
#undef Steady_Trie_Get_Next_Iter_Key_Chunk
#undef Steady_Trie_Slot_Mask
#undef Steady_Trie_Get_Slot_Shift
#undef Steady_Trie_Get_Next_Iter_Key_Chunk
#undef Steady_Trie_Get_Initial_Iter_Key_Chunk
#undef Steady_Trie_Get_Slot_Value
#undef Steady_Trie_Is_Max_Depth
#undef Steady_Trie_Is_Key_At_Final_Depth
#undef Steady_Function

#undef Steady_Trie_Value_Type
#undef Steady_Trie_Default_Value
