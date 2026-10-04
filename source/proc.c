/*
   Proc is a diagram editor styled after the diagrams in "Picturing Quantum Processes" found at https://www.cs.ox.ac.uk/people/aleks.kissinger/PQP.pdf
*/


#include <stdio.h> // printf

// TODO: Points_Per_Wire should probably be dynamic...
#define Points_Per_Wire 24
#include "../source/_include_platform.h"
#include "../libraries/ryn_prof.h"
#include "../source/render.h"
#include "../source/proc.h"

Define_Cycle_Detector_Function(
  process_list_has_cycles,
  Process, next);

Define_Cycle_Detector_Function(
  active_process_list_has_cycles,
  Process, next_active);






//////////////////////////////////////
// Globals
//////////////////////////////////////

global_variable Vector2 global_window_size;
global_variable String8 Saves_Filepath;
global_variable String8 Build_Filepath;

#include "../source/keybind.h"
#include "../source/ui.h"
#include "../source/saves.h"

global_variable F32 global_process_wire_padding = 8.0f;
global_variable F32 global_process_wire_spacing = 22.0f;

#define Default_Box_Size 10.0f
global_variable F32 global_box_size = Default_Box_Size;
global_variable F32 global_box_half_size = 0.5f*Default_Box_Size;

#define Default_Shape_Size 40.0f
global_variable F32 global_shape_size = Default_Shape_Size;
global_variable F32 global_shape_half_size = 0.5f*Default_Shape_Size;

global_variable F32 global_line_thickness;
global_variable F32 global_active_line_thickness;

global_variable F32 global_process_font_size = 16.0f;

global_variable Color global_background_color;

global_variable S32 global_shape_fan_triangle_count = 12;

global_variable F32 global_panel_text_input_min_height;
global_variable Color global_container_bg_color;
global_variable Color global_process_bg_color;


#define Half_Circle_Fudge 1.32f
#define Half_Circle_Radius_Fudge 1.0f

















function void remove_copy_process_list(Context *context, Process_List *list) {
  // TODO: @Speed can probably do some fancy stuff with just the ends of the list?
  for (Process *p = list->first; p != 0;) {
    Process *next_process = p->next;
    remove_process_from_process_list(&context->copy_processes, p);
    p = next_process;
  }
}




function Process *add_process_to_copy_list(
  Context *context,
  Process *p,
  Vector2 *copy_center,
  F32 *copy_count
  ) {
  WhatIsThis wit = (WhatIsThis){0}; // TODO: handle passing/returning wits
  Process *copied_p = create_detached_process(&wit);
  *copied_p = *p;
  p->to_copied = copied_p;
  *copy_center = Vector2Add(*copy_center, p->position);
  *copy_count += 1.0f;
  SLLQueuePush(context->copy_processes.first, context->copy_processes.last, copied_p);

  return copied_p;
}


function void copy_active_processes(Context *context, View *view) {
  B32 error = 0;
  Vector2 copy_center = (Vector2){0};
  F32 copy_count = 0.0f;

  // remove whatever processes were already in the copy-list
  remove_copy_process_list(context, &context->copy_processes);

  // copy processes from active-list to copy-list
  for (Process *a = view->and_whats_this.active_processes.first; a != 0; a = a->next_active) {
    if (Get_Flag(a->flags, Process_Flag_Wire)) {
      // add connected processes if they have not been added yet
      for (S32 conn = 0; conn < Process_Connection__Count; ++conn) {
        if (a->conn[conn] && a->conn[conn]->to_copied == 0) {
          B32 found_conn = 0;
          for (Process *test_p = view->and_whats_this.active_processes.first; test_p != 0; test_p = test_p->next_active) {
            if (test_p == a->conn[conn]) {
              found_conn = 1;
              // add connected process to copied list
              add_process_to_copy_list(context, a->conn[conn], &copy_center, &copy_count);
              break;
            }
          }
          if (!found_conn) {
            // add invisible process to copied list
            Process *copied_p = add_process_to_copy_list(context, a->conn[conn], &copy_center, &copy_count);
            Set_Flag(copied_p->flags, Process_Flag_Empty);
          }
        }
      }
      // add wire to copied-list
      WhatIsThis wit = (WhatIsThis){0}; // TODO: handle passing/returning wits
      Process *copied_wire = create_detached_process(&wit);
      *copied_wire = *a;
      // connect copied wire to copied processes
      for (S32 conn = 0; conn < Process_Connection__Count; ++conn) {
        if (copied_wire->conn[conn]) {
          copied_wire->conn[conn] = copied_wire->conn[conn]->to_copied;
        }
      }
      SLLQueuePush(context->copy_processes.first, context->copy_processes.last, copied_wire);
    } else if (!a->to_copied) {
      // only add process if it hasn't already been added by a connected wire
      add_process_to_copy_list(context, a, &copy_center, &copy_count);
    }
  }

  // remove all to_copied fields
  for (Process *p = view->and_whats_this.processes.first; p != 0; p = p->next) {
    p->to_copied = 0;
  }

  // TODO: @Speed
  // fix-up copied wire positions (in the cases that only some of the wires between two processes are copied)
  for (Process *c = context->copy_processes.first; c != 0; c = c->next) {
    if (!Get_Flag(c->flags, Process_Flag_Wire)) {
      for (S32 conn = 0; conn < Process_Connection__Count; ++conn) {
        // get connected wire count
        S32 conn_count = 0;
        for (Process *w = context->copy_processes.first; w != 0; w = w->next) {
          if (Get_Flag(w->flags, Process_Flag_Wire) && w->conn[conn] == c) {
            conn_count += 1;
          }
        }
        // adjust process' conn count
        c->conn_count[conn] = conn_count;
        // we have to loop wire-count times to re-assign each wire
        for (S32 min_conn = 0; min_conn < conn_count; ++min_conn) {
          Process *min_wire = 0;
          // find the next connected wire
          for (Process *w = context->copy_processes.first; w != 0; w = w->next) {
            if (Get_Flag(w->flags, Process_Flag_Wire) && w->conn[conn] == c) {
              if (w->which_conn[conn] >= min_conn) {
                if (min_wire == 0 || w->which_conn[conn] < min_wire->which_conn[conn]) {
                  min_wire = w;
                }
              }
            }
          }
          // adjust the wire's connection
          if (min_wire) {
            min_wire->which_conn[conn] = min_conn;
          }
        }
      }
    }
  }

  // post-copy processing
  for (Process *c = context->copy_processes.first; c != 0; c = c->next) {
    // make any process with more than one connection (in or out) visible
    {
      B32 more_than_one_connection = 0;
      for (S32 conn = 0; conn < Process_Connection__Count; ++conn) {
        if (c->conn_count[conn] > 1) {
          more_than_one_connection = 1;
          break;
        }
      }
      if (more_than_one_connection) {
        Unset_Flag(c->flags, Process_Flag_Empty);
      }
    }

    // copy label
    if (c->label) {
      // TODO: ........
      /* c->label = copy_piece_table(c->label); */
    }
  }

  if (error) {
    remove_copy_process_list(context, &context->copy_processes);
  } else {
    if (copy_count > 0.0f) {
      copy_center = Vector2Scale(copy_center, 1.0f/copy_count);
    } else {
      // TODO: If copy_count is 0 then something went wrong and maybe we should just bail?
      copy_center = (Vector2){0};
    }
    context->copy_center = copy_center;
  }
}


function void paste_processes(Context *context, View *view) {
  // TODO: bounds check the view
  if (Get_Flag(view->flags, View_Flag_Active) &&
      Get_Flag(view->flags, View_Flag_Editable)) {
    Vector2 mouse_world_pos = GetScreenToWorld2D(context->ui_state.mouse_position, view->camera);
    Vector2 center_delta = Vector2Subtract(mouse_world_pos, context->copy_center);

    U32 process_count = 0;
    for (Process *p = context->copy_processes.first; p != 0; p = p->next) {
      process_count += 1;
    }

    if (process_count) {
      WhatIsThis wit = (WhatIsThis){0}; // TODO: handle passing/returning wits
      Process *ps = create_processes(&wit);
      U32 i = 0;

      for (Process *p = context->copy_processes.first; p != 0; p = p->next) {
        if (i >= process_count) {
          break;
        }
        Process *new_p = ps + i;
        i += 1;
        *new_p = *p;
        new_p->position = Vector2Add(p->position, center_delta);
      }
    }
  }

  context->copy_processes.first = 0;
  context->copy_processes.last = 0;
}



function B32 is_active_process(Context *context, View *view, Process *p) {
  B32 is_active = 0;

  if (Get_Flag(p->flags, Process_Flag_IsActive)) {
    is_active = 1;
  }
  else {
    for (Process *test_p = view->and_whats_this.active_processes.first; test_p != 0; test_p = test_p->next_active) {
      if (test_p == p) {
        is_active = 1;
        break;
      }
    }
  }

  return is_active;
}


function Process *find_process_connection(
  Context *context,
  View *view,
  Process *p,
  Process_Connection conn,
  U32 which_conn
  ) {
  Process *target_wire = 0;

  for (Process *w = view->and_whats_this.processes.first; w != 0; w = w->next) {
    if (Get_Flag(w->flags, Process_Flag_Wire)) {
      if ((p == w->conn[conn]) && (w->which_conn[conn] == which_conn)) {
        target_wire = w;
        break;
      }
    }
  }

  return target_wire;
}








//////////////////////////
// Debug diagnostics
//////////////////////////
function void check_process_list(Process_List list) {
  Assert(!process_list_has_cycles(list.first));
  Assert(!process_list_has_cycles(list.last));
}








function B32 rectangle_contains_point(Rectangle r, Vector2 p) {
  F32 x2 = r.x + r.width;
  F32 y2 = r.y + r.height;
  B32 contains = (p.x >= r.x) && (p.y >= r.y) && (p.x <= x2) && (p.y <= y2);
  return contains;
}


function String_Chunk_List string_chunk_list_from_string8(Context *context, String8 string8) {
  String_Chunk_List list = (String_Chunk_List){0};

  U64 remaining_size = string8.size;
  U64 string8_index = 0;

  for (;;) {
    if (remaining_size == 0) {
      break;
    }

    String_Chunk *chunk = create_string_chunk(context->what_is_this.permanent_arena, &context->free_strings);
    SLLQueuePush(list.first, list.last, chunk);

    U64 amount_to_write = Min(remaining_size, String_Chunk_Size);
    remaining_size -= amount_to_write;

    for (S32 i = 0; i < amount_to_write; ++i) {
      chunk->str_array[i] = string8.str[string8_index];
      string8_index += 1;
    }
  }

  // add null-termination chunk if the last byte is not 0
  if (list.last && list.last->str_array[String_Chunk_Size-1] != 0) {
    String_Chunk *chunk = create_string_chunk(context->what_is_this.permanent_arena, &context->free_strings);
    SLLQueuePush(list.first, list.last, chunk);
  }

  return list;
}







function V2_Chunk *create_v2_chunk(Context *context) {
  V2_Chunk *chunk = context->free_v2_chunks;

  if (chunk) {
    SLLStackPop(context->free_v2_chunks);
    *chunk = (V2_Chunk){0};
  }
  else {
    chunk = push_struct(context->what_is_this.permanent_arena, V2_Chunk);
  }

  return chunk;
}


function void free_v2_chunk(Context *context, V2_Chunk *chunk) {
  if (chunk) {
    SLLStackPush(context->free_v2_chunks, chunk);
  }
}


function Vector2 *get_fresh_v2_from_v2_chunk(Context *context, V2_Chunk *chunk) {
  Vector2 *result = 0;

  if (chunk) {
    if (chunk->count >= V2_Chunk_Size) {
      // TODO: push new chunk
      V2_Chunk *new_chunk = create_v2_chunk(context);
      if (new_chunk) {
        result = new_chunk->e;
        new_chunk->count = 1;
      }
    }
    else {
      result = chunk->e + chunk->count;
      chunk->count += 1;
    }
  }

  return result;
}
























////////////////////////////////////////
// UI Functions
////////////////////////////////////////

function void clear_ui_state(Context *context, View *view) {
  context->save_file_list.first = 0;
  context->save_file_list.last = 0;

  arena_pop_to(context->ui_arena, 0);
  view->and_whats_this.do_undo.trie = proc_trie_create_trie(context->ui_arena);
  /* gather_processes_from_trie(context, view); */
  gather_processes_from_trie(&context->what_is_this);
}



function void set_open_file_as_active_element(Context *context, Process *_element) {
}


function void set_save_file_as_as_active_element(Context *context, View *view, Process *element) {
  global_ui_procs[Global_Ui_Proc_Id_top_menu_box].ref =
    &global_ui_procs[Global_Ui_Proc_Id_save_file_as_box];
  Set_Flag(global_ui_procs[Global_Ui_Proc_Id_save_file_as_box].flags, Process_Flag_IsActive);
}


function void handle_label_editing(Context *context, View *view, Process_List ps) {
  // TODO: we need more than ascii text editing at some point.....
  U32 key = 0;
  U32 k = 0;
  B32 shift_down = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
  B32 should_update_process = context->edit_timeout <= 0.0f;
  Process_List new_active_list = (Process_List){0};
  B32 editing_occured = 0;
  WhatIsThis wit = (WhatIsThis){0}; // TODO: handle wits

  while ((key = context->ui_state.key_presses[k++])) {
    for (Process *a = ps.first; a != 0; a = a->next_active) {
      if (view && Get_Flag(a->flags, Process_Flag_TextEdit)) {
        Process edit_a;

        if (should_update_process) {
          context->edit_timeout = context->time_to_wait_for_label_edit;
          Editable_Process editable_a = get_editable_process(view->and_whats_this.do_undo.edit_list, a);
          edit_a = editable_a.process;
        }
        else {
          edit_a = *a;
        }

        B32 is_ascii = key > 0 && key < 256;
        U8 c = ascii_char_lookup[key&0xff][shift_down];
        if (is_ascii && c != 0) {
          // insert character
          if (edit_a.label == 0) {
            edit_a.label = push_struct(context->what_is_this.permanent_arena, Piece_Table);
          }

          if (edit_a.label) {
            piece_table_insert(&wit, edit_a.label, edit_a.label_cursor, (String8){&c, 1});
            edit_a.label_cursor += 1;
            editing_occured = 1;
          }
          else {
            printf("[ Error ] Null label while inserting text.\n");
          }
        } else if (key == KEY_BACKSPACE) {
          // handle backspace
          if (edit_a.label_cursor > 0) {
            piece_table_delete(&wit, edit_a.label, edit_a.label_cursor, 1);
            edit_a.label_cursor -= 1;
            editing_occured = 1;
          }
        }
        else if (key == KEY_LEFT && edit_a.label_cursor > 0) {
          edit_a.label_cursor -= 1;
        }
        else if (key == KEY_RIGHT && edit_a.label_cursor < edit_a.label->text_size) {
          edit_a.label_cursor += 1;
        }
        debug_check_piece_table(edit_a.label);

        // update active proc
        if (should_update_process && editing_occured) {
          add_process_to_process_edit_list(&wit, a, Proc_Trie_Edit_Update, edit_a);
        }
        else {
          *a = edit_a;
        }
      }
    }
  }

  if (should_update_process && editing_occured) {
    /* gather_processes_from_trie(context, view); */
    gather_processes_from_trie(&context->what_is_this);
  }
}




function Vector2 get_ui_box_inner_position(Context *context, Process *box) {
  Vector2 position = Vector2Add(Vector2Add(box->position, box->ui_box.position), box->ui_box.scroll_offset);
  return position;
}


function Vector2 get_box_size(Process *box) {
  Process *box_parent = box->next;
  B32 stretch = Get_Flag(box->flags, Ui_Box_Flag_Stretch);
  Vector2 size = box->ui_box.size;

  if (stretch && box_parent) {
    Vector2 parent_size = get_box_size(box_parent);
    // TODO: Do we need to recursively call get_box_size here??
    if (box->ui_box.layout == Ui_Layout_Vertical) {
      size.x = parent_size.x;
    } else if (box->ui_box.layout == Ui_Layout_Horizontal) {
      size.y = parent_size.y;
    }
  }

  if (box->ui_box.min_size.x > 0.0f) {
    size.x = Max(size.x, box->ui_box.min_size.x);
  }
  if (box->ui_box.min_size.y > 0.0f) {
    size.y = Max(size.y, box->ui_box.min_size.y);
  }
  if (box->ui_box.max_size.x > 0.0f) {
    size.x = Min(size.x, box->ui_box.max_size.x);
  }
  if (box->ui_box.max_size.y > 0.0f) {
    size.y = Min(size.y, box->ui_box.max_size.y);
  }

  return size;
}




function void set_ui_box_size(Process *box, Vector2 size, B32 set_box_x, B32 set_box_y) {
  if (set_box_x) {
    if (box->ui_box.layout == Ui_Layout_Horizontal) {
      box->ui_box.size.x += size.x;
    } else {
      box->ui_box.size.x = Max(box->ui_box.size.x, size.x);
    }
  }
  if (set_box_y) {
    if (box->ui_box.layout == Ui_Layout_Vertical) {
      box->ui_box.size.y += size.y;
    } else {
      box->ui_box.size.y = Max(box->ui_box.size.y, size.y);
    }
  }
}



function Process *create_button(Arena *arena, Vector2 position, String_Chunk_List label) {
  Process *button = push_struct(arena, Process);
  Vector2 padding = global_button_padding;
  F32 font_size = global_panel_font_size;

  if (button) {
    U8 *label_c_string = c_string_from_string_chunk_list(render_GlobalTempArena, &label);
    S32 text_width = MeasureText((char *)label_c_string, font_size);
    button->ui_box.size.x = text_width + 2.0f*padding.x;
    button->ui_box.size.y = font_size + 2.0f*padding.y;

    Set_Flag(button->flags, Process_Flag_Clickable|Process_Flag_FitToText);
    button->position = position;
    button->label_c_string = label_c_string;
  }

  return button;
}






function View *create_view(Arena *arena) {
  View *result = push_struct(arena, View);

  if (result) {
    result->and_whats_this.do_undo.arena = arena;
    result->camera.zoom = 1.0f;
  }

  return result;
}









////////////////////////////////////////
// File actions
////////////////////////////////////////

function void clear_save_files(Context *context) {
}


function S32 collect_save_files(Context *context) {
  Arena *uia = context->ui_arena;
  S32 save_file_count = 0;

  // gather the current files in the "saves" folder
  FileProperties file_props = Zero_Struct(FileProperties);
  String8 file_name = Zero_Struct(String8);
  OS_FileIter file_iter = os_file_iter_init(Saves_Filepath);
  while(os_file_iter_next(uia, &file_iter, &file_name, &file_props)) {
    if (!Get_Flag(file_props.flags, FilePropertyFlag_IsFolder)) {
      String_Chunk_List label = string_chunk_list_from_string8(context, file_name);
      Process *element = create_button(uia, (Vector2){0}, label);

      if (element) {
        SLLQueuePush(context->save_file_list.first, context->save_file_list.last, element);
        save_file_count += 1;
      }
    }
  }

  return save_file_count;
}


function void save_file(Context *context, View *view, Process *element) {
  if (context->save_file_name) {
    write_save_file(context, view, context->what_is_this.per_frame_arena, context->save_file_name);
  }
}




function void handle_copy(Context *context, View *view, Process *element) {
  copy_active_processes(context, view);
}


function void handle_paste(Context *context, View *view, Process *element) {
  paste_processes(context, view);
}






function Vector2 get_percentage_between_points(Vector2 p0, Vector2 p1, F32 percentage) {
  Vector2 norm_delta = Vector2Normalize(Vector2Subtract(p1, p0));
  F32 distance_along_delta = percentage * Vector2Distance(p1, p0);
  Vector2 center = Vector2Add(p0, Vector2Scale(norm_delta, distance_along_delta));

  return center;
}



function Vector2 get_process_position(Context *context, View *view, Process *process) {
  // TODO: we probably want to just call world-to-screen inside here...
  Vector2 position = process->position;
  B32 is_active = is_active_process(context, view, process);
  B32 is_dragging = Get_Flag(context->flags, Context_Flag_Dragging);

  if (is_active && is_dragging) {
    Vector2 delta = Vector2Subtract(context->ui_state.mouse_position, context->ui_state.active_position);
    position = Vector2Add(position, Vector2Scale(delta, 1.0f/view->camera.zoom));
  }

  return position;
}



function Vector2 get_wire_position_from_conn_index(
  Context *context,
  View *view,
  Process *connected_process,
  Process_Shape shape,
  Process_Connection conn,
  U32 conn_index
  ) {
  F32 padding = view->camera.zoom * global_process_wire_padding;
  Vector2 p0;
  Vector2 p1;

  switch(conn) {
  case Process_Connection_In: {
    if (shape.kind == Process_Shape_HalfCircle) {
      p0 = shape.points[0];
      p1 = shape.points[shape.point_count-1];
    } else if (shape.point_count == 4) {
      p0 = shape.points[2];
      p1 = shape.points[3];
    } else {
      p0 = shape.points[2];
      p1 = shape.points[1];
    }
  } break;
  case Process_Connection_Out: {
    if (shape.kind == Process_Shape_HalfCircle) {
      p0 = shape.points[shape.point_count-1];
      p1 = shape.points[0];
    } else {
      p0 = shape.points[0];
      p1 = shape.points[1];
    }
  } break;
  }

  // HACK: ensure that delta will point to the right
  if (p0.x < p1.x) {
    Swap(Vector2, p0, p1);
  }

  Vector2 delta = Vector2Subtract(p0, p1);
  Vector2 delta_norm = Vector2Normalize(delta);
  F32 inner_distance = fmax(0.0f, Vector2Distance(p0, p1) - 2.0f*padding);
  F32 chunk_size = inner_distance / (F32)(connected_process->conn_count[conn]+1);
  F32 distance_from_point = padding + chunk_size*(F32)(conn_index+1);

  Vector2 wire_position = Vector2Add(p1, Vector2Scale(delta_norm, distance_from_point));

  return wire_position;
}




function Vector2 get_wire_position_from_wire(
  Context *context,
  View *view,
  Process *wire,
  Process_Shape shape,
  Process_Connection conn
  ) {
  Vector2 pos = (Vector2){0};

  B32 out_being_dragged = (conn == Process_Connection_Out &&
                           Get_Flag(wire->flags, Process_Flag_Drag_Out));
  B32 in_being_dragged = (conn == Process_Connection_In &&
                           Get_Flag(wire->flags, Process_Flag_Drag_In));

  if (out_being_dragged || in_being_dragged) {
    pos = context->ui_state.mouse_position;
  }
  else {
    Process *connected_process = wire->conn[conn];
    U32 conn_index = wire->which_conn[conn];

    pos = get_wire_position_from_conn_index(
      context,
      view,
      connected_process,
      shape,
      conn,
      conn_index);
  }

  return pos;
}


function Rectangle get_wire_box(Context *context, View *view, Vector2 position) {
  F32 size = view->camera.zoom * global_box_size;
  F32 half_size = view->camera.zoom * global_box_half_size;
  Rectangle box = (Rectangle){position.x-half_size, position.y-half_size, size, size};
  return box;
}


function Rectangle get_new_wire_box(Context *context, View *view, Process *p, Process_Shape shape) {
  Vector2 position = shape.new_wire_position;
  F32 size = view->camera.zoom * global_box_size;
  F32 half_size = view->camera.zoom * global_box_half_size;

  Rectangle new_wire_box = (Rectangle){position.x - half_size,
                                       position.y - half_size,
                                       size, size};

  return new_wire_box;
}


function Rectangle get_selection_rectangle(Context *context) {
  F32 x = fmin(context->ui_state.active_position.x, context->ui_state.mouse_position.x);
  F32 y = fmin(context->ui_state.active_position.y, context->ui_state.mouse_position.y);
  F32 x1 = fmax(context->ui_state.active_position.x, context->ui_state.mouse_position.x);
  F32 y1 = fmax(context->ui_state.active_position.y, context->ui_state.mouse_position.y);
  Rectangle selection_rect = (Rectangle){x, y, x1-x, y1-y};

  return selection_rect;
}


function Process *get_wire_from_selection(Context *context, View *view, Process_Selection selection) {
  Process *wire = 0;

  for (Process *p = view->and_whats_this.processes.first; p != 0; p = p->next) {
    if (Get_Flag(p->flags, Process_Flag_Wire)) {
      if (selection.type == Process_Selection_In &&
          p->in == selection.process &&
          p->which_in == selection.index) {
        // matching in-wire
        wire = p;
        break;
      } else if (selection.type == Process_Selection_Out &&
                 p->out == selection.process &&
                 p->which_out == selection.index) {
        // matching out-wire
        wire = p;
        break;
      }
    }
  }

  if (wire == 0) {
    wire = The_Null_Process();
  }

  return wire;
}




function void remove_process_from_active_processes(Context *context, View *view, Process *p) {
  if (view->and_whats_this.active_processes.first == p) {
    SLLQueuePop_NZ(view->and_whats_this.active_processes.first, view->and_whats_this.active_processes.last, next_active, 0);
  } else {
    for (Process *test_p = view->and_whats_this.active_processes.first; test_p != 0; test_p = test_p->next_active) {
      if (test_p->next_active == p) {
        test_p->next_active = p->next_active;
        if (p == view->and_whats_this.active_processes.last) {
          view->and_whats_this.active_processes.last = test_p;
        }
        break;
      }
    }
  }
}




function Half_Circle_Points get_half_circle_points(
  Context *context,
  View *view,
  Process_Shape shape,
  Process *p,
  Vector2 position,
  S32 text_width,
  B32 downward
  ) {
  Half_Circle_Points half_circle_points;
  F32 padding = view->camera.zoom * global_process_wire_padding;
  F32 spacing = view->camera.zoom * global_process_wire_spacing;

  F32 half_height = view->camera.zoom * global_shape_half_size;

  F32 conn_count = (F32)(downward ? p->out_count : p->in_count);
  F32 width = (2.0f*padding + conn_count*spacing);
  F32 half_width = 0.5f*(width);
  // fit shape to text, if the text is wider than the shape
  if ((F32)text_width > half_width) {
    half_width = 0.8f*(2.0f*padding + text_width);
  }

  F32 multiplier = downward ? -1.0f : 1.0f;
  F32 x_offset = multiplier * half_width;
  F32 y_offset = multiplier * half_height;

  Vector2 first_point = (Vector2){position.x-x_offset, position.y+y_offset};
  Vector2 second_point = (Vector2){position.x+x_offset, position.y+y_offset};

  half_circle_points.first_point = first_point;
  half_circle_points.second_point = second_point;
  half_circle_points.first_control = (Vector2){first_point.x, first_point.y-2.0f*y_offset};
  half_circle_points.second_control = (Vector2){second_point.x, second_point.y-2.0f*y_offset};

  half_circle_points.middle_of_curve = get_bezier_point(
    first_point,
    second_point,
    half_circle_points.first_control,
    half_circle_points.second_control,
    0.5f);

  half_circle_points.middle_of_line = get_percentage_between_points(first_point, second_point, 0.5f);

  return half_circle_points;
}


function void fill_out_half_circle_shape(
  Context *context,
  View *view,
  Process_Shape *shape,
  Process *p,
  Vector2 position,
  S32 text_width,
  B32 downward
  ) {
  Half_Circle_Points half_circle_points = get_half_circle_points(context, view, *shape, p, position, text_width, downward);

  shape->kind = Process_Shape_HalfCircle;
  shape->triangle_count = global_shape_fan_triangle_count;
  shape->downward = downward;

  shape->first_control = half_circle_points.first_control;
  shape->second_control = half_circle_points.second_control;

  Vector2 first_point = half_circle_points.first_point;
  Vector2 second_point = half_circle_points.second_point;

  Vector2 middle_of_curve = half_circle_points.middle_of_curve;
  Vector2 middle_of_line = half_circle_points.middle_of_line;

  shape->center = get_percentage_between_points(middle_of_curve, middle_of_line, 0.5);

  if (downward) {
    shape->new_wire_position = half_circle_points.first_point;
  } else {
    shape->new_wire_position = middle_of_curve;
  }

  shape->point_count = create_bezier_triangle_fan(
    first_point, second_point,
    shape->first_control, shape->second_control,
    shape->points, Process_Shape_Max_Points, shape->triangle_count);
}


function Vector2 get_process_size(
  Context *context,
  Process *p,
  Process_Shape shape
  ) {
  Vector2 size = (Vector2){0};

  F32 min_x = inf_F32;
  F32 min_y = inf_F32;
  F32 max_x = neg_inf_F32;
  F32 max_y = neg_inf_F32;

  if (shape.kind == Process_Shape_Circle) {
    min_x = shape.center.x - shape.radius;
    min_y = shape.center.y - shape.radius;
    max_x = shape.center.x + shape.radius;
    max_y = shape.center.y + shape.radius;
  }
  else {
    for (S32 i = 0; i < shape.point_count; ++i) {
      Vector2 *point = shape.points +i;
      min_x = Min(min_x, point->x);
      min_y = Min(min_y, point->y);
      max_x = Max(max_x, point->x);
      max_y = Max(max_y, point->y);
    }
  }

  size.x = max_x - min_x;
  size.y = max_y - min_y;

  return size;
}






function Bezier_Points get_wire_bezier_points(
  Context *context,
  View *view,
  Process *w,
  B32 is_active
  ) {
  Bezier_Points result = (Bezier_Points){0};

  { // gather inner points into pre-buffer
    B32 is_dragging = Get_Flag(context->flags, Context_Flag_Dragging) && is_active;
    B32 hacky_inner_position_set = w->inner_positions && w->inner_positions->count;
    B32 use_inner_position = is_dragging || hacky_inner_position_set;

    if (w->out && w->in) {
      Process_Shape out_shape = get_process_shape(context, view, w->out);
      Process_Shape in_shape = get_process_shape(context, view, w->in);
      Vector2 out_position = get_wire_position_from_wire(context, view, w, out_shape, Process_Connection_Out);
      Vector2 in_position = get_wire_position_from_wire(context, view, w, in_shape, Process_Connection_In);

      if (use_inner_position) {
        // TODO: don't just use the first inner-position
        Vector2 inner_position;
        if (is_dragging) {
          inner_position = GetScreenToWorld2D(context->ui_state.mouse_position, view->camera);
        }
        else if (w->inner_positions) {
          inner_position = w->inner_positions->e[0];
        }
        else {
          inner_position = (Vector2){0};
        }

        // TODO: this will need to eventually handle arbitrary inner-positions
        Vector2 *points = push_array(context->what_is_this.per_frame_arena, Vector2, 3);
        if (points) {
          result.controls.point_count = 3;
          result.controls.points = points;
          result.controls.points[0] = out_position;
          result.controls.points[1] = GetWorldToScreen2D(inner_position, view->camera);
          result.controls.points[2] = in_position;
        }
      }
      else {
        Vector2 *points = push_array(context->what_is_this.per_frame_arena, Vector2, 2);
        if (points) {
          result.controls.point_count = 2;
          result.controls.points = points;
          result.controls.points[0] = out_position;
          result.controls.points[1] = in_position;
        }
      }
    }
  }

  if (result.controls.point_count >= 2) {
    U32 bez_curve_count = result.controls.point_count-1;
    Buffer_V2 *inner_bez_buffers = push_array(context->what_is_this.per_frame_arena, Buffer_V2, bez_curve_count);
    U32 total_bez_points = 0;

    if (inner_bez_buffers) {
      for (U32 i = 0; i < bez_curve_count; ++i) {
        Vector2 out_position = result.controls.points[i];
        Vector2 in_position = result.controls.points[i+1];

        // TODO: we need to handle inner-controls differently
        Vector2 out_control = out_position;
        out_control.y -= view->camera.zoom * 30.0f;
        Vector2 in_control = in_position;
        in_control.y += view->camera.zoom * 30.0f;

        // TODO:
        inner_bez_buffers[i] = get_bezier_points(context->render_arena, out_position, in_position, out_control, in_control);
        if (i != bez_curve_count-1) {
          // avoid adding duplicate bez-points
          inner_bez_buffers[i].point_count -= 1;
        }
        total_bez_points += inner_bez_buffers[i].point_count;
      }
    }

    // fill out the resulting buffer's points
    if (total_bez_points) {
      result.path.points = push_array(context->render_arena, Vector2, total_bez_points);
      if (result.path.points) {
        result.path.point_count = total_bez_points;
        U32 point_index = 0;
        for (U32 i = 0; i < bez_curve_count; ++i) {
          Buffer_V2 *inner_buffer = inner_bez_buffers + i;
          for (U32 j = 0; j < inner_buffer->point_count; ++j) {
            result.path.points[point_index] = inner_buffer->points[j];
            point_index += 1;
          }
        }
      }
    }
  }
  else {
    // @Copypasta "draw wires"
    Process_Shape out_shape = get_process_shape(context, view, w->out);
    Process_Shape in_shape = get_process_shape(context, view, w->in);

    Vector2 out_position = get_wire_position_from_wire(context, view, w, out_shape, Process_Connection_Out);
    Vector2 in_position = get_wire_position_from_wire(context, view, w, in_shape, Process_Connection_In);

    Vector2 out_control = out_position;
    out_control.y -= view->camera.zoom * 30.0f;
    Vector2 in_control = in_position;
    in_control.y += view->camera.zoom * 30.0f;

    result.path = get_bezier_points(context->render_arena, out_position, in_position, out_control, in_control);
  }

  return result;
}




function Process_Shape get_process_shape(
  Context *context,
  View *view,
  Process *p
  ) {
  // TODO: process wire shapes, so that we can make wires hot by hovering
  Process_Shape shape = {0};
  U64 arena_pop_pos = arena_current_pos(context->what_is_this.per_frame_arena);
  if (p == 0) goto error;

  F32 font_size = view->camera.zoom * global_process_font_size;
  String8 string = piece_table_get_string(context->what_is_this.per_frame_arena, p->label);
  U8 *label_c_string = string.str;
  S32 text_width = MeasureText((char *)label_c_string, font_size);

  Vector2 position = get_process_position(context, view, p);
  position = GetWorldToScreen2D(position, view->camera);

  F32 half_size = view->camera.zoom * global_shape_half_size;
  F32 quarter_size = view->camera.zoom * global_shape_size / 4.0f;
  F32 padding = view->camera.zoom * global_process_wire_padding;
  F32 spacing = view->camera.zoom * global_process_wire_spacing;

  S32 has_in = p->in_count > 0;
  S32 has_out = p->out_count > 0;
  B32 as_box = Get_Flag(p->flags, Process_Flag_IsBox);

  B32 rounded = Get_Flag(context->flags, Context_Flag_RoundedShapes);

  if (Get_Flag(p->flags, Process_Flag_Wire)) {
    B32 is_active = is_active_process(context, view, p) || context->hot_process.process == p;
    F32 thickness = is_active ? global_active_line_thickness : global_line_thickness;
    thickness *= view->camera.zoom;

    Bezier_Points bez_points = get_wire_bezier_points(context, view, p, is_active);
    Buffer_V2 strip_points = get_connected_path(context->render_arena, bez_points.path.points, bez_points.path.point_count, thickness, 0);

    shape.kind = Process_Shape_TriangleStrip;
    shape.points_ptr = strip_points.points;
    shape.point_count = strip_points.point_count;
    shape.triangle_count = Triangle_Count_From_Point_Count(strip_points.point_count);
  }
  else if (as_box || (has_in && has_out)) {
    // rectangular
    F32 max_conn = as_box ? 1.0f : (F32)Max(p->in_count, p->out_count);
    F32 half_width = 0.5f*(2.0f*padding + max_conn*spacing);
    // fit shape to text if text is wide enough
    if ((F32)text_width > half_width) {
      half_width = 0.5f*(2.0f*padding + (F32)text_width);
    }
    shape.kind = Process_Shape_TriangleFan;
    shape.point_count = 4;
    shape.triangle_count = 2;
    shape.points[0].x = position.x + half_width;
    shape.points[0].y = position.y - half_size;
    shape.points[1].x = position.x - half_width;
    shape.points[1].y = position.y - half_size;
    shape.points[2].x = position.x - half_width;
    shape.points[2].y = position.y + half_size;
    shape.points[3].x = position.x + half_width;
    shape.points[3].y = position.y + half_size;
    shape.center = get_percentage_between_points(shape.points[0], shape.points[2], 0.5f);
    shape.new_wire_position = shape.points[0];
  } else if (has_in) {
    F32 width = (2.0f*padding + p->in_count*spacing);
    F32 half_width = 0.5f*(width);
    // fit shape to text if text is wide enough
    if ((F32)text_width > 0.5f*half_width) {
      half_width = (2.0f*padding + (F32)text_width);
    }
    if (rounded) {
      // upward half-circle
      fill_out_half_circle_shape(context, view, &shape, p, position, text_width, 0);
    } else {
      // upward triangle
      shape.kind = Process_Shape_TriangleFan;
      shape.point_count = 3;
      shape.triangle_count = 1;
      shape.points[0].x = position.x;
      shape.points[0].y = position.y - quarter_size;
      shape.points[1].x = position.x - half_width;
      shape.points[1].y = position.y + half_size;
      shape.points[2].x = position.x + half_width;
      shape.points[2].y = position.y + half_size;
      Vector2 outer_mid = get_percentage_between_points(shape.points[1], shape.points[2], 0.5f);
      shape.center = get_percentage_between_points(shape.points[0], outer_mid, 0.66f);
      shape.new_wire_position = shape.points[0];
    }
  } else if (has_out) {
    F32 width = (2.0f*padding + p->out_count*spacing);
    F32 half_width = 0.5f*(width);
    // fit shape to text if text is wide enough
    if ((F32)text_width > 0.5f*half_width) {
      half_width = (2.0f*padding + (F32)text_width);
    }
    if (rounded) {
      // downward half-circle
      fill_out_half_circle_shape(context, view, &shape, p, position, text_width, 1);
    } else {
      // downward triangle
      shape.kind = Process_Shape_TriangleFan;
      shape.point_count = 3;
      shape.triangle_count = 1;
      shape.points[0].x = position.x + half_width;
      shape.points[0].y = position.y - half_size;
      shape.points[1].x = position.x - half_width;
      shape.points[1].y = position.y - half_size;
      shape.points[2].x = position.x;
      shape.points[2].y = position.y + quarter_size;
      Vector2 outer_mid = get_percentage_between_points(shape.points[0], shape.points[1], 0.5f);
      shape.center = get_percentage_between_points(shape.points[2], outer_mid, 0.66f);
      shape.new_wire_position = shape.points[0];
    }
  } else {
    if (rounded) {
      // circle
      // TODO: Setup the circle in a different way so that it can fit text. Like an ellipse with beziers...
      shape.kind = Process_Shape_Circle;
      shape.center = position;
      shape.radius = half_size*0.7f;
      shape.new_wire_position = (Vector2){shape.center.x, shape.center.y - shape.radius};
    } else {
      // diamond
      // fit shape to text if text is wide enough
      F32 half_size_x = half_size;
      if ((F32)text_width > half_size) {
        half_size_x = (F32)text_width;
      }
      shape.kind = Process_Shape_TriangleFan;
      shape.point_count = 4;
      shape.triangle_count = 2;
      shape.points[0].x = position.x;
      shape.points[0].y = position.y - half_size;
      shape.points[1].x = position.x - half_size_x;
      shape.points[1].y = position.y;
      shape.points[2].x = position.x;
      shape.points[2].y = position.y + half_size;
      shape.points[3].x = position.x + half_size_x;
      shape.points[3].y = position.y;
      shape.center = position;
      shape.new_wire_position = shape.points[0];
    }
  }

error:;
  arena_pop_to(context->what_is_this.per_frame_arena, arena_pop_pos);

  return shape;
}


function B32 triangle_fan_contains_point(
  Vector2 *points,
  S32 triangle_count,
  Vector2 point
  ) {
  B32 contains = 0;

  for (S32 i = 1; i <= triangle_count; ++i) {
    F32 side1 = which_side_of_line(points[0], points[i], point);
    F32 side2 = which_side_of_line(points[i], points[i+1], point);
    F32 side3 = which_side_of_line(points[i+1], points[0], point);

    if (side1 < 0.0f && side2 < 0.0f && side3 < 0.0f) {
      contains = 1;
      break;
    }
  }

  return contains;
}


function B32 triangle_strip_contains_point(
  Vector2 *points,
  S32 triangle_count,
  Vector2 point
  ) {
  B32 contains = 0;

  for (S32 i = 0; i < triangle_count; ++i) {
    F32 side1 = which_side_of_line(points[i], points[i+1], point);
    F32 side2 = which_side_of_line(points[i+1], points[i+2], point);
    F32 side3 = which_side_of_line(points[i+2], points[i], point);

    if (i % 2 == 0) {
      if (side1 < 0.0f && side2 < 0.0f && side3 < 0.0f) {
        contains = 1;
        break;
      }
    } else {
      if (side1 > 0.0f && side2 > 0.0f && side3 > 0.0f) {
        contains = 1;
        break;
      }
    }
  }

  return contains;
}


function B32 process_shape_contains_point(
  Context *context,
  Process_Shape shape,
  Vector2 point
  ) {
  B32 contains = 0;

  switch(shape.kind) {
  case Process_Shape_Circle: {
    F32 distance = Vector2Distance(shape.center, point);
    contains = distance <= shape.radius;
  } break;
  case Process_Shape_HalfCircle: {
    contains = triangle_fan_contains_point(shape.points, shape.triangle_count, point);
  } break;
  case Process_Shape_TriangleFan: {
    contains = triangle_fan_contains_point(shape.points, shape.triangle_count, point);
  } break;
  case Process_Shape_TriangleStrip: {
    contains = triangle_strip_contains_point(shape.points_ptr, shape.triangle_count, point);
  } break;
  default: Assert(0);
  }

  return contains;
}






function Process_Selection get_process_selection(Context *context, View *view, Process *p) {
  Ui_State *ui_state = &context->ui_state;
  Process_Selection selection = {0};
  selection.index = -1;
  selection.process = p;
  selection.view = view;

  Process_Shape shape = get_process_shape(context, view, p);
  Rectangle new_wire_box = get_new_wire_box(context, view, p, shape);

  if (!Get_Flag(ui_state->flags, Ui_State_Flag_hot_id_assigned)) {
    if (rectangle_contains_point(new_wire_box, context->ui_state.mouse_position)) {
      // check new-wire-box
      selection.type = Process_Selection_NewWire;
      context->hot_process.process = p;
      selection.hot_id_assigned = 1;
    } else {
      // check in wire-boxes from proc
      for (U32 i = 0; i < p->in_count; ++i) {
        Vector2 in_position = get_wire_position_from_conn_index(context, view, p, shape, Process_Connection_In, i);
        Rectangle r = get_wire_box(context, view, in_position);
        if (rectangle_contains_point(r, context->ui_state.mouse_position)) {
          selection.type = Process_Selection_In;
          selection.index = i;
          Process *wire = get_wire_from_selection(context, view, selection);
          context->hot_process.process = wire;
          selection.hot_id_assigned = 1;
          break;
        }
      }

      if (selection.type == 0) {
        // check out wire-boxes from proc
        for (U32 i = 0; i < p->out_count; ++i) {
          Vector2 out_position = get_wire_position_from_conn_index(context, view, p, shape, Process_Connection_Out, i);
          Rectangle r = get_wire_box(context, view, out_position);
          if (rectangle_contains_point(r, context->ui_state.mouse_position)) {
            selection.type = Process_Selection_Out;
            selection.index = i;
            Process *wire = get_wire_from_selection(context, view, selection);
            context->hot_process.process = wire;
            selection.hot_id_assigned = 1;
            break;
          }
        }
      }

      if (Get_Flag(p->flags, Process_Flag_Wire)) {
        // check out wire-box from wire
        if (selection.type == 0) {
          Vector2 out_position = get_wire_position_from_wire(context, view, p, shape, Process_Connection_Out);
          Rectangle r = get_wire_box(context, view, out_position);
          if (rectangle_contains_point(r, context->ui_state.mouse_position)) {
            selection.type = Process_Selection_Out;
            context->hot_process.process = p;
            selection.hot_id_assigned = 1;
          }
        }
        // check in wire-box from wire
        if (selection.type == 0) {
          Vector2 in_position = get_wire_position_from_wire(context, view, p, shape, Process_Connection_In);
          Rectangle r = get_wire_box(context, view, in_position);
          if (rectangle_contains_point(r, context->ui_state.mouse_position)) {
            selection.type = Process_Selection_In;
            context->hot_process.process = p;
            selection.hot_id_assigned = 1;
          }
        }
      }

      if (selection.type == 0) {
        if (process_shape_contains_point(context, shape, context->ui_state.mouse_position)) {
          // process selection
          selection.type = Process_Selection_Process;
          context->hot_process.process = p;
          selection.hot_id_assigned = 1;
        }
      }
    }
  }

  return selection;
}















function void draw_circular_process(Context *context, Vector2 center, F32 radius, F32 thickness, Color bg_color, Color stroke_color) {
  Render_Context *rc = &context->process_render_context;

  render_DrawCircle(rc, center, radius, bg_color);
  render_DrawCircleLines(rc, center.x, center.y, radius, thickness, stroke_color);
}


function void draw_process_with_triangle_fan(Context *context, Process_Shape shape, F32 thickness, Color bg_color, Color stroke_color) {
  Assert(shape.triangle_count == (shape.point_count - 2));
  Render_Context *rc = &context->process_render_context;

  // draw background
  render_DrawTriangleFan(rc, shape.points, shape.point_count, bg_color);

  // draw path
  Buffer_V2 path = get_connected_path(context->render_arena, shape.points, shape.point_count, thickness, 1);
  render_DrawTriangleStrip(rc, path.points, path.point_count, stroke_color);
}









function S32 debug_process_list_count(Process_List list) {
  S32 count = 0;
  for (Process *p = list.first; p != 0; p = p->next) {
    count += 1;
  }
  return count;
}


function S32 debug_process_active_list_count(Process_List list) {
  S32 count = 0;
  for (Process *p = list.first; p != 0; p = p->next_active) {
    count += 1;
  }
  return count;
}











function void set_global_window_render_size(void) {
  Vector2 dpi_scale = GetWindowScaleDPI();
  global_window_size.x = (F32)GetRenderWidth() / dpi_scale.x;
  global_window_size.y = (F32)GetRenderHeight() / dpi_scale.x;
}





// NOTE; Kahn's algorithm
function void create_keybind_array(Context *context) {
  Arena *arena = context->what_is_this.permanent_arena;

  Keybind *keybind_set_first = 0;
  Keybind *keybind_set_last = 0;

  Keybind *keybind_final_first = 0;
  Keybind *keybind_final_last = 0;

  U32 order_count = SymbolCount(Keybind_Order_Sym);

  // add keybinds with no preceding keybind
  for (U32 k = 0; k < SymbolCount(Keybind_Sym); ++k) {
    Keybind *keybind = SymbolMetadataFromID(Keybind_Sym, k+1);

    if (keybind->handle) {
      B32 has_preceding_keybind = 0;

      for (U32 o = 0; o < order_count; ++o) {
        Keybind_Order *order = SymbolMetadataFromID(Keybind_Order_Sym, o+1);
        if (Keybind_Order_Is_Valid(order)) {
          if (Keybind_Is_After(keybind, order)) {
            has_preceding_keybind = 1;
            break;
          }
        }
      }

      if (!has_preceding_keybind) {
        SLLQueuePush(keybind_set_first, keybind_set_last, keybind);
      }
    }
  }

  // remove invalid orders
  for (U32 o = 0; o < order_count; ++o) {
    Keybind_Order *order = SymbolMetadataFromID(Keybind_Order_Sym, o+1);
    if (!Keybind_Order_Is_Valid(order)) {
      order->removed = 1;
    }
  }

  // sort keybinds
  for (;;) {
    Keybind *stack_keybind = keybind_set_first;
    SLLStackPop(keybind_set_first);

    if (stack_keybind) {
      SLLQueuePush(keybind_final_first, keybind_final_last, stack_keybind);
      context->keybind_count += 1;

      for (U32 o = 0; o < order_count; ++o) {
        Keybind_Order *order = SymbolMetadataFromID(Keybind_Order_Sym, o+1);
        Keybind *other_keybind = (order->keybind_a == stack_keybind) ? order->keybind_b : order->keybind_a;

        if (Keybind_Order_Is_Valid(order)) {
          if (!order->removed) {
            // other keybind comes after current keybind
            if (Keybind_Is_Before(stack_keybind, order) &&
                Keybind_Is_After(other_keybind, order)) {
              order->removed = 1;
              B32 has_no_preceding_keybinds = 1;

              for (U32 test_o = 0; test_o < order_count; ++test_o) {
                Keybind_Order *test_order = SymbolMetadataFromID(Keybind_Order_Sym, test_o+1);
                if (!test_order->removed) {
                  if (Keybind_Is_After(other_keybind, test_order)) {
                    has_no_preceding_keybinds = 0;
                    break;
                  }
                }
              }

              if (has_no_preceding_keybinds) {
                SLLQueuePush(keybind_set_first, keybind_set_last, other_keybind);
              }
            }
          }
        }
      }
    }
    else {
      break;
    }
  }

  if (context->keybind_count) {
    context->keybinds = push_array(arena, Keybind, context->keybind_count);

    if (context->keybinds) {
      U32 index = 0;
      for (Keybind *keybind = keybind_final_first; keybind != 0; keybind = keybind->next) {
        if (index < context->keybind_count) {
          context->keybinds[index] = *keybind;
          index += 1;
        }
        else {
          printf("[ Error ] Keybind count mismatch.\n");
          break;
        }
      }
    }
    else {
      printf("[ Error ] While pushing keybind array.\n");
    }
  }

  // check if there are un-removed edges
  for (U32 o = 0; o < order_count; ++o) {
    Keybind_Order *order = SymbolMetadataFromID(Keybind_Order_Sym, o+1);
    if (Keybind_Order_Is_Valid(order)) {
      if (!order->removed) {
        B32 is_before_kind = order->kind == Keybind_Order_Before;
        Keybind *before_keybind = is_before_kind ? order->keybind_a : order->keybind_b;
        Keybind *after_keybind = is_before_kind ? order->keybind_b : order->keybind_a;
        printf("[ Warning ] Cycles exist in keybinds! The order '%s'->'%s' is not satisfied.\n", before_keybind->name.str, after_keybind->name.str);
      }
    }
  }
}










//////////////////////////////////////////
// Main
//////////////////////////////////////////
int main(void) {
  //////////////////////////////////////////
  // Init
  //////////////////////////////////////////
  Context context = (Context){0};
  Render_Context *prc = 0;
  uint64_t cpu_freq;

  {
    { // Init window
      InitWindow(800, 500, "proc");
      SetExitKey(0);
      SetWindowState(FLAG_WINDOW_RESIZABLE);
      SetTargetFPS(60);
    }

    { // init context
      context.render_arena    = arena_alloc_reserve(Megabytes(10), 0);
      context.what_is_this.permanent_arena = arena_alloc_reserve(Megabytes(100), 0);
      context.ui_arena        = arena_alloc_reserve(Megabytes(1), 0);
      /* context.temp_arena      = arena_alloc_reserve(Megabytes(10), 0); */
      context.what_is_this.per_frame_arena = arena_alloc_reserve(Megabytes(10), 0);

      context.ui_render_context.arena = context.render_arena;
      context.process_render_context.arena = context.render_arena;
      prc = &context.process_render_context;

      context.what_is_this.gen_id = 1;
      context.time_to_wait_for_label_edit = 1.0f;

      Set_Flag(context.flags, Context_Flag_AutoAlignChains);
      Set_Flag(context.flags, Context_Flag_DataStructureView);
    }

    { // init keybinds
      create_keybind_array(&context);
    }

    { // init globals
      S32 monitor_id = GetCurrentMonitor();
      S32 screen_width = GetMonitorWidth(monitor_id);
      S32 screen_height = GetMonitorHeight(monitor_id);

#if 1 || defined(DarkMode)
      global_background_color = (Color){180, 180, 170, 255};
#else
      global_background_color = (Color){220, 220, 200, 255};
#endif

      global_window_size.x = 0.7f*(F32)screen_width;
      global_window_size.y = 0.7f*(F32)screen_height;

      global_shape_size = global_window_size.x / 20.0f;
      global_shape_half_size = 0.5f*global_shape_size;

      global_box_size = global_shape_size*0.22f;
      global_box_half_size = 0.5f*global_box_size;

      global_process_wire_padding = 0.2f*global_shape_size;
      global_process_wire_spacing = 0.55f*global_shape_size;

      global_process_font_size = 0.4f*global_shape_size;
      global_panel_font_size = 0.35f*global_shape_size;

      global_line_thickness = 0.05f*global_shape_size;
      global_active_line_thickness = 0.1f*global_shape_size;

      global_panel_text_input_min_height = global_panel_font_size + 2.0f*global_button_padding.y;

      global_container_bg_color = (Color){170, 170, 170, 255};
      global_process_bg_color = (Color){190, 190, 199, 255};

      // init common filepaths
#if OS_WINDOWS
# define _ "\\"
#else
# define _ "/"
#endif
      Saves_Filepath = str8_comptime_lit(".."_"saves"_);
      Build_Filepath = str8_comptime_lit(".."_"build"_);
#undef _

      // ensure saves directory exists
      os_file_make_directory(Saves_Filepath);
    }

    { // init views
      View *root_view = create_view(context.what_is_this.permanent_arena);
      View *menu_view = create_view(context.what_is_this.permanent_arena);
      View *canvas_view = create_view(context.what_is_this.permanent_arena);
      if (root_view && menu_view && canvas_view) {
        F32 menu_height = global_panel_font_size;
        // menu view
        Set_Flag(menu_view->flags, View_Flag_Active);
        Set_Flag(menu_view->kind_flags, View_Kind_Flag_Ui);
        menu_view->screen_region.width = global_window_size.x;
        menu_view->screen_region.height = menu_height;
        menu_view->and_whats_this.do_undo.trie = proc_trie_create_trie(menu_view->and_whats_this.do_undo.arena);
        menu_view->color = (Color){50, 55, 50, 255};
        SLLQueuePush(root_view->first, root_view->last, menu_view);

        // canvas view
        Set_Flag(canvas_view->flags, View_Flag_Active|View_Flag_Panning|View_Flag_Editable);
        Set_Flag(canvas_view->kind_flags, View_Kind_Flag_Procs);
        canvas_view->screen_region.y = menu_height;
        canvas_view->screen_region.width = global_window_size.x;
        canvas_view->screen_region.height = global_window_size.y - menu_height;
        canvas_view->and_whats_this.do_undo.trie = proc_trie_create_trie(canvas_view->and_whats_this.do_undo.arena);
        canvas_view->color = (Color){130, 150, 130, 255};
        SLLQueuePush(root_view->first, root_view->last, canvas_view);

        // root view
        context.root_view = root_view;
      }
    }

    { // misc. init
      SetWindowSize(global_window_size.x, global_window_size.y);
      render_Initialize(context.what_is_this.per_frame_arena);
      set_global_window_render_size();
      cpu_freq = ryn_EstimateCpuFrequency(100);
    }
  }

  //////////////////////////////////////////
  // Main Loop
  //////////////////////////////////////////
  while (!WindowShouldClose()) {
    ryn_BeginProfile();
    ryn_BEGIN_TIMED_BLOCK(main_loop);

    if (IsWindowResized()) {
      set_global_window_render_size();
    }

    //////////////////////////////////////////
    // Handle User Input
    //////////////////////////////////////////
    F32 frame_time = GetTime();
    {
      // update ui-state
      {
        Ui_State *ui_state = &context.ui_state;

        ui_state->frame_delta = frame_time - ui_state->last_frame_time;
        ui_state->last_frame_time = frame_time;
        context.edit_timeout -= ui_state->frame_delta;

        ui_state->mouse_position = GetMousePosition();
        F32 mouse_move_threshold = 0.1f;
        ui_state->mouse_moved =
          ((fabs(ui_state->active_position.x - ui_state->mouse_position.x) > mouse_move_threshold) ||
           (fabs(ui_state->active_position.y - ui_state->mouse_position.y) > mouse_move_threshold));

        ui_state->mouse_wheel_movement = GetMouseWheelMoveV();
        ui_state->kb_action = 0;

        Assign_Flag(ui_state->flags, Ui_State_Flag_mouse0_pressed, IsMouseButtonPressed(0));
        Assign_Flag(ui_state->flags, Ui_State_Flag_mouse1_pressed, IsMouseButtonPressed(1));
        Assign_Flag(ui_state->flags, Ui_State_Flag_mouse0_down, IsMouseButtonDown(0));
        Assign_Flag(ui_state->flags, Ui_State_Flag_mouse1_down, IsMouseButtonDown(1));
        Unset_Flag(ui_state->flags, Ui_State_Flag_hot_id_assigned);
        {
          Assign_Flag(ui_state->modifier_flags, Ui_State_Modifier_Flag_control_down, IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL));
          Assign_Flag(ui_state->modifier_flags, Ui_State_Modifier_Flag_shift_down, IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT));
          Assign_Flag(ui_state->modifier_flags, Ui_State_Modifier_Flag_alt_down, IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT));
          Assign_Flag(ui_state->modifier_flags, Ui_State_Modifier_Flag_super_down, IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER));
        }
        Unset_Flag(ui_state->flags, Ui_State_Flag_action_occured);
      }

      // get key presses
      for (U32 k = 0; k < Max_Key_Presses_Per_Frame; ++k) {
        U32 key = GetKeyPressed();
        context.ui_state.key_presses[k] = key;

        if (key == 0) {
          break;
        }
      }

      if (!Get_Flag(context.ui_state.flags, Ui_State_Flag_action_occured)) {
        // environment
        Process_Selection selection = (Process_Selection){0};
        Keybind_Environment env_raw = create_keybind_environment(&context, selection);
        Keybind_Environment *env = &env_raw;
        env->moved_wire = 0;
        env->moved_wire_conn = 0;

        //////////////////////////////////////////
        // Handle Process Interaction
        //////////////////////////////////////////
        View_Iterate(stack, &context) {
          if (stack->view->first && stack->view->last) {
          }
          else {
            for (U32 i = 0; i < env->context->keybind_count; ++i) {
              Keybind *keybind = env->context->keybinds + i;
              env->keybind = keybind;
              env->view = stack->view;
              keybind_handle(env, keybind);
              check_process_list(stack->view->and_whats_this.active_processes);
            }
          }
        }
      }
    }


    { // check_context
      check_process_list(context.free_processes);
      check_process_list(context.copy_processes);
    }


    //////////////////////////////////////////
    // Draw
    //////////////////////////////////////////
    {
      render_ClearBackground(prc, global_background_color);

      //////////////////////////////////////////
      // Draw Processes
      //////////////////////////////////////////
      {
        Render_Context *rc = &context.process_render_context;

        Color bg_color = global_process_bg_color;
        Color invisible_bg_color = (Color){0, 0, 0, 0};
        Color stroke_color = (Color){0, 0, 0, 255};
        Color invisible_stroke_color = (Color){100, 100, 100, 255};
        Color text_color = (Color){0, 0, 0, 255};
        Color box_color = (Color){10, 190, 40, 255};
        Color box_hover_color = (Color){5, 250, 20, 255};
        B32 rounded = Get_Flag(context.flags, Context_Flag_RoundedShapes);

        View_Iterate(stack, &context) {
          B32 should_clip = Get_Flag(stack->view->flags, View_Flag_Clip);
          if (should_clip) {
            Vector2 position = (Vector2){stack->view->screen_region.x,
                                         stack->view->screen_region.y};
            Vector2 size = (Vector2){stack->view->screen_region.width,
                                     stack->view->screen_region.height};
            render_BeginScissorMode(rc, position, size);
          }

          Process *processes_to_draw = stack->view->and_whats_this.processes.first;
          F32 font_size = stack->view->camera.zoom * global_process_font_size;

          // @Speed
          // draw lines
          for (Process *p = processes_to_draw; p != 0; p = p->next) {
            if (Get_Flag(p->flags, Process_Flag_Line)) {
              if (p->in && p->out) {
                Vector2 in_pos = get_process_position(&context, stack->view, p->in);
                in_pos = GetWorldToScreen2D(in_pos, stack->view->camera);
                Vector2 out_pos = get_process_position(&context, stack->view, p->out);
                out_pos = GetWorldToScreen2D(out_pos, stack->view->camera);
                Color c = (Color){0, 0, 0, 255}; // TODO: use some existing color
                render_DrawLine(rc, in_pos.x, in_pos.y, out_pos.x, out_pos.y, 2.0f, c);
              }
            }
          }

          // draw processes
          for (Process *p = processes_to_draw; p != 0; p = p->next) {
            B32 is_wire = Get_Flag(p->flags, Process_Flag_Wire);

            // @Copypasta decl_ui_init
            String8 label_string = piece_table_get_string(context.what_is_this.per_frame_arena, p->label);
            if (label_string.str == 0 || label_string.size == 0) {
              if (p->label_c_string) {
                label_string = str8_lit(p->label_c_string);
              }
            }
            S32 text_width = MeasureText((char *)label_string.str, font_size);
            S32 cursor_offset = 0;
            if (p->label_cursor) {
              // @Speed: It's silly to copy this string just to measure where the cursor needs to be.......
              String8 label_cursor_string = str8_push_copy(context.what_is_this.per_frame_arena, label_string);
              Assert_If(p->label_cursor <= label_cursor_string.size) {
                printf("[ Error ] Process(%p) label-cursor (%d) greater than label-cursor-string size (%llu).\n", p, p->label_cursor, label_cursor_string.size);
                p->label_cursor = label_cursor_string.size;
              }

              label_cursor_string.str[p->label_cursor] = 0;
              cursor_offset = MeasureText((char *)label_cursor_string.str, font_size);
            }

            B32 is_invisible = Get_Flag(p->flags, Process_Flag_Invisible);

            if (!(is_wire || is_invisible)) {
              Process_Shape shape = get_process_shape(&context, stack->view, p);

              B32 is_hot = context.hot_process.process == p;
              B32 is_active = is_active_process(&context, stack->view, p);
              F32 thickness = (is_hot||is_active) ? global_active_line_thickness : global_line_thickness;
              thickness *= stack->view->camera.zoom;
              F32 cup_cap_control_offset = 10.0f*stack->view->camera.zoom;

              if (Get_Flag(p->flags, Process_Flag_Empty)) {
                // draw line through empty shape
                B32 upward = p->in_count == 1 && p->out_count == 0;
                B32 downward = p->in_count == 0 && p->out_count == 1;
                // only if it's valid
                if (upward || downward) {
                  Vector2 p0 = (Vector2){0};
                  Vector2 p1 = (Vector2){0};
                  if (rounded) {
                    // rounded half-circle
                    Vector2 position = get_process_position(&context, stack->view, p);
                    position = GetWorldToScreen2D(position, stack->view->camera);
                    Half_Circle_Points points = get_half_circle_points(&context, stack->view, shape, p, position, text_width, downward);
                    p0 = points.middle_of_line;
                    p1 = points.middle_of_curve;
                  } else {
                    if (upward) {
                      // upward triangle
                      p0 = get_percentage_between_points(shape.points[1], shape.points[2], 0.5f);
                      p1 = shape.points[0];
                    } else if (downward) {
                      // downward triangle
                      p0 = get_percentage_between_points(shape.points[0], shape.points[1], 0.5f);
                      p1 = shape.points[2];
                    }
                  }
                  render_DrawLineBezierCubic(rc, p0, p1, p1, p0, thickness, stroke_color, 0);
                } else if (!(label_string.str && label_string.str[0])) {
                  if (rounded) {
                    draw_circular_process(&context, shape.center, shape.radius, thickness, invisible_bg_color, invisible_stroke_color);
                  } else {
                    draw_process_with_triangle_fan(&context, shape, thickness, invisible_bg_color, invisible_stroke_color);
                  }
                }
              } else if (Get_Flag(p->flags, Process_Flag_Cup)) {
                // draw cup
                Vector2 pos0 = get_wire_position_from_conn_index(&context, stack->view, p, shape, Process_Connection_Out, 0);
                Vector2 pos1 = get_wire_position_from_conn_index(&context, stack->view, p, shape, Process_Connection_Out, 1);
                Vector2 ctrl0 = (Vector2){pos0.x, pos0.y+cup_cap_control_offset};
                Vector2 ctrl1 = (Vector2){pos1.x, pos1.y+cup_cap_control_offset};
                render_DrawLineBezierCubic(rc, pos0, pos1, ctrl0, ctrl1, thickness, stroke_color, 0);
              } else if (Get_Flag(p->flags, Process_Flag_Cap)) {
                // draw cap
                Vector2 pos0 = get_wire_position_from_conn_index(&context, stack->view, p, shape, Process_Connection_In, 0);
                Vector2 pos1 = get_wire_position_from_conn_index(&context, stack->view, p, shape, Process_Connection_In, 1);
                Vector2 ctrl0 = (Vector2){pos0.x, pos0.y-cup_cap_control_offset};
                Vector2 ctrl1 = (Vector2){pos1.x, pos1.y-cup_cap_control_offset};
                render_DrawLineBezierCubic(rc, pos0, pos1, ctrl0, ctrl1, thickness, stroke_color ,0);
              } else if (Get_Flag(p->flags, Process_Flag_Identity)) {
                // draw "identity" process (just a wire)
                Vector2 pos0 = get_wire_position_from_conn_index(&context, stack->view, p, shape, Process_Connection_In, 0);
                Vector2 pos1 = get_wire_position_from_conn_index(&context, stack->view, p, shape, Process_Connection_Out, 0);
                render_DrawLineBezierCubic(rc, pos0, pos1, pos1, pos0, thickness, stroke_color, 0);
              } else {
                switch(shape.kind) {
                case Process_Shape_TriangleFan: {
                  draw_process_with_triangle_fan(&context, shape, thickness, bg_color, stroke_color);
                } break;
                case Process_Shape_Circle: {
                  draw_circular_process(&context, shape.center, shape.radius, thickness, bg_color, stroke_color);
                } break;
                case Process_Shape_HalfCircle: {
                  // draw half-circle background
                  render_DrawTriangleFan(rc, shape.points, shape.point_count, bg_color);
                  Vector2 position = get_process_position(&context, stack->view, p);
                  position = GetWorldToScreen2D(position, stack->view->camera);
                  Half_Circle_Points hc_points = get_half_circle_points(&context, stack->view, shape, p, position, text_width, shape.downward);
                  render_DrawLineBezierCubic(rc, hc_points.first_point, hc_points.second_point, hc_points.first_control, hc_points.second_control, thickness, stroke_color, 1);
                } break;
                default: Assert(0);
                }
              }

              // draw label
              if (label_string.str && label_string.str[0]) {
                F32 text_x = shape.center.x-0.5f*text_width;
                F32 text_y = shape.center.y-0.5f*font_size;
                if (shape.kind == Process_Shape_HalfCircle) {
                  F32 flip = shape.downward ? -1.0f : 1.0f;
                  F32 fudge = 0.9f;
                  F32 offset = fudge * flip * (0.5f * shape.radius);
                  text_y -= offset;
                }
                render_DrawText(rc, (char *)label_string.str, text_x, text_y, font_size, text_color, 0);

                if (Get_Flag(p->flags, Process_Flag_TextEdit) && is_active) {
                  // draw cursor
                  render_DrawRectangle(
                    rc,
                    text_x+cursor_offset, text_y, 4.0f, font_size,
                    (Color){10, 40, 200, 255});
                }
              }

              // draw new-wire-box
              if (is_active || is_hot) {
                Rectangle new_wire_box = get_new_wire_box(&context, stack->view, p, shape);
                B32 new_wire_box_is_active = (
                  (is_active && Get_Flag(context.flags, Context_Flag_NewWire)) ||
                  rectangle_contains_point(new_wire_box, context.ui_state.mouse_position));
                Color color = new_wire_box_is_active ? box_hover_color : box_color;
                render_DrawRectangleRec(rc, new_wire_box, color);
              }
            }
          }

          // draw wires
          for (Process *p = processes_to_draw; p != 0; p = p->next) {
            B32 is_wire = Get_Flag(p->flags, Process_Flag_Wire);
            B32 is_invisible = Get_Flag(p->flags, Process_Flag_Invisible);

            if (is_wire && !is_invisible) {
              Process_Shape out_shape = get_process_shape(&context, stack->view, p->out);
              Process_Shape in_shape = get_process_shape(&context, stack->view, p->in);

              Vector2 out_position = get_wire_position_from_wire(&context, stack->view, p, out_shape, Process_Connection_Out);
              Vector2 in_position = get_wire_position_from_wire(&context, stack->view, p, in_shape, Process_Connection_In);

              B32 is_active = is_active_process(&context, stack->view, p) || context.hot_process.process == p;
              B32 connected_in_active = (is_active_process(&context, stack->view, p->in) ||
                                         context.hot_process.process == p->in);
              B32 connected_out_active = (is_active_process(&context, stack->view, p->out) ||
                                          context.hot_process.process == p->out);
              F32 thickness = is_active ? global_active_line_thickness : global_line_thickness;
              thickness *= stack->view->camera.zoom;

              Bezier_Points bez_points = get_wire_bezier_points(&context, stack->view, p, is_active);
              Buffer_V2 strip = get_connected_path(context.render_arena, bez_points.path.points, bez_points.path.point_count, thickness, 0);
              render_DrawTriangleStrip_P(rc, strip.points, strip.point_count, stroke_color);

              if (is_active) {
                // draw bezier controls
                for (U32 i = 0; i < bez_points.controls.point_count; ++i) {
                  Vector2 center = bez_points.controls.points[i];
                  F32 radius = 4.0f * stack->view->camera.zoom;
                  Color color = (Color){200, 20, 180, 255};
                  render_DrawCircleLines(rc, center.x, center.y, radius, 2.0f, color);
                }
              }

              // draw out wire-box
              if (connected_out_active || is_active) {
                Rectangle box = get_wire_box(&context, stack->view, out_position);
                Color c = is_active ? box_hover_color : box_color;
                render_DrawRectangleRec(rc, box, c);
              }

              // draw in wire-box
              if (connected_in_active || is_active) {
                Rectangle box = get_wire_box(&context, stack->view, in_position);
                Color c = is_active ? box_hover_color : box_color;
                render_DrawRectangleRec(rc, box, c);
              }
            }
          }

          // draw new wire
          if (Get_Flag(context.flags, Context_Flag_NewWire) &&
              stack->view->and_whats_this.active_processes.first) {
            Process_Shape shape = get_process_shape(&context, stack->view, stack->view->and_whats_this.active_processes.first);
            Vector2 position = shape.new_wire_position;

            Vector2 from_control = position;
            from_control.y -= stack->view->camera.zoom * 30.f;
            Vector2 to_control = context.ui_state.mouse_position;
            to_control.y += stack->view->camera.zoom * 30.0f;

            F32 thickness = stack->view->camera.zoom * global_line_thickness;

            render_DrawLineBezierCubic(rc, position, context.ui_state.mouse_position, from_control, to_control, thickness, stroke_color, 0);
          }

          if (should_clip) {
            render_EndScissorMode(rc);
          }
        }

        // draw selection rectangle
        if (Get_Flag(context.flags, Context_Flag_Bounding)) {
          Rectangle selection_rect = get_selection_rectangle(&context);
          Color selection_color = (Color){10, 30, 200, 50};

          render_DrawRectangleRec(rc, selection_rect, selection_color);
        }
      }

      //////////////////////////////////////////
      // Draw Info Panel
      //////////////////////////////////////////
      {
        Render_Context *rc = &context.ui_render_context;
        Color text_color = (Color){0, 0, 0, 255};
        F32 x = 5.0f;
        F32 y = 5.0f;
        F32 padding = 2.0f;

        {
#define Debug_Draw_Arena_Info(arena)                                    \
          render_DrawText(rc, TextFormat("%s (%3.1f%%) %llu/%llu\n",    \
                                         #arena,                        \
                                         (F32)context.arena->chunk_pos/(F32)context.arena->chunk_cap, \
                                         context.arena->chunk_pos, context.arena->chunk_cap), \
                          x, y, arena_font_size, text_color, 1);        \
          y -= arena_font_size + padding
 // end define Debug_Draw_Arena_Info(arena)

          S32 arena_font_size = 12;
          y = global_window_size.y - arena_font_size - padding;
          Debug_Draw_Arena_Info(render_arena);
          Debug_Draw_Arena_Info(what_is_this.permanent_arena);
          Debug_Draw_Arena_Info(ui_arena);
          Debug_Draw_Arena_Info(what_is_this.per_frame_arena);
#undef Debug_Draw_Arena_Info
        }
      }

      //////////////////////////////////////////
      // Run Render Commands
      //////////////////////////////////////////
      {
        BeginDrawing();
        render_Commands(&context.process_render_context);
        render_Commands(&context.ui_render_context);
      }
    }

    ryn_END_TIMED_BLOCK(main_loop);

    //////////////////////////////////////////
    // End Profiler
    //////////////////////////////////////////
    ryn_EndAndPrintProfile(cpu_freq);
    // clear profile timers
    for(uint32_t TimerIndex = 0; TimerIndex < SymbolCount(ryn_sym_timer); ++TimerIndex) {
      ryn_timer_data *Timer = SymbolMetadataFromID(ryn_sym_timer, TimerIndex+1);
      Timer->ElapsedExclusive = 0;
      Timer->ElapsedInclusive = 0;
      Timer->HitCount = 0;
      Timer->ProcessedByteCount = 0;
    }

    //////////////////////////////////////////
    // Cleanup
    //////////////////////////////////////////
    {
      arena_pop_to(context.render_arena, 0);
      arena_pop_to(context.what_is_this.per_frame_arena, 0);
      context.ui_render_context.command_list.first = 0;
      context.ui_render_context.command_list.last = 0;
      context.process_render_context.command_list.first = 0;
      context.process_render_context.command_list.last = 0;
    }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}






