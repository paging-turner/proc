/*
  Eventually, this file will contain all of the functions available to the custom-keybind environment.
*/


function void remove_string_chunk_list(Context *context, String_Chunk_List *scl) {
  if (scl->first && scl->last) {
    if (context->free_strings.first && context->free_strings.last) {
      context->free_strings.last->next = scl->first;
      context->free_strings.last = scl->last;
    } else {
      context->free_strings.first = scl->first;
      context->free_strings.last = scl->last;
    }

    scl->first = 0;
    scl->last = 0;
  }
}

function void clear_process_list(Context *context, Process_List *list) {
  if (list && list->first) {
    for (Process *p = list->first; p != 0;) {
      Process *next = p->next;
      if (Get_Flag(p->flags, Process_Flag_IsDetached)) {
        SLLQueuePush(context->free_processes.first, context->free_processes.last, p);
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

function void clear_active_process_list(Process_List *list) {
  if (list && list->first) {
    for (Process *p = list->first; p != 0;) {
      Process *next = p->next_active;
      p->next_active = 0;
      p = next;
    }

    list->first = 0;
    list->last = 0;
  }
}



function void clear_active_processes(Context *context) {
  clear_active_process_list(&context->active_processes);
}



function Process *push_permanent_process(Context *context) {
  Process *p = push_struct(context->permanent_arena, Process);

  return p;
}



function Process *create_detached_process(Context *context) {
  Process *p = context->free_processes.first;

  if (p) {
    SLLQueuePop(context->free_processes.first, context->free_processes.last);
  } else {
    p = push_permanent_process(context);
  }

  if (p) {
    *p = (Process){0};
    Set_Flag(p->flags, Process_Flag_IsDetached);
  } else {
    p = The_Null_Process();
  }

  return p;
}
