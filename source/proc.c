/*
   Proc is a diagram editor styled after the diagrams in "Picturing Quantum Processes" found at https://www.cs.ox.ac.uk/people/aleks.kissinger/PQP.pdf
*/



#include <stdio.h> // printf, fopen

#define MR4TH_NO_INCLUDES 1
#define MR4TH_NO_CLAMP 1
#if !No_Assert
# define MR4TH_ASSERTS 1
#endif
#include "../libraries/mr4th/src/mr4th_base.h"
#define push_struct(a, s) arena_push((a), sizeof(s))



#if OS_WINDOWS
# include "../libraries/raylib-5.5_win32_msvc16/include/raylib.h"
# include "../libraries/raylib-5.5_win32_msvc16/include/raymath.h"
#elif OS_MAC
# include "../libraries/raylib-5.5_macos/include/raylib.h"
# include "../libraries/raylib-5.5_macos/include/raymath.h"
#else
# error We have not included the raylib release for this OS yet.
#endif

#include "../source/core.h"
#include "../source/render.h"


//////////////////////////////////////
// Paths
//////////////////////////////////////
global_variable String8 Keybind_Config_Filepath;
global_variable String8 Saves_Filepath;
global_variable String8 Build_Filepath;



#include "../source/proc.h"
#include "../source/keybind.h"
#include "../source/saves.h"






//////////////////////////////////////
// Globals
//////////////////////////////////////

global_variable Vector2 global_window_size;

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
global_variable F32 global_panel_font_size = 14.0f;

global_variable Color global_background_color;

global_variable S32 global_shape_fan_triangle_count = 12;

global_variable Vector2 global_button_padding;
global_variable Color global_button_dormant_bg_color;
global_variable Color global_button_hot_bg_color;
global_variable Color global_button_font_color;
global_variable Color global_container_bg_color;

global_variable Process global_null_process;
global_variable String_Chunk global_null_string_chunk;
#define The_Null_Process() (global_null_process=(Process){0}, &global_null_process)
#define The_Null_String_Chunk() (global_null_string_chunk=(String_Chunk){0}, &global_null_string_chunk)

#define Half_Circle_Fudge 1.32f
#define Half_Circle_Radius_Fudge 1.0f


////////////////////////
// UI Globals
////////////////////////

global_variable Process file_menu_button;
global_variable Process open_file_button;
global_variable Process save_file_button;
global_variable Process save_as_file_button;
global_variable Process edit_menu_button;
global_variable Process copy_button;
global_variable Process paste_button;

global_variable Process *menu_buttons[] = {
    [Top_Menu_Index(Menu_State_FileMenu)] = &file_menu_button,
    [Top_Menu_Index(Menu_State_EditMenu)] = &edit_menu_button,
};
global_variable Process *file_submenu[] = { &open_file_button, &save_file_button, &save_as_file_button };
global_variable Process *edit_submenu[] = { &copy_button, &paste_button };
global_variable Process **sub_menus[] = {
  [Top_Menu_Index(Menu_State_FileMenu)] = file_submenu,
  [Top_Menu_Index(Menu_State_EditMenu)] = edit_submenu,
};
global_variable U32 sub_menu_counts[] = { ArrayCount(file_submenu), ArrayCount(edit_submenu) };


global_variable Ui_Box top_menu_box = (Ui_Box){
  .align = Ui_Align_TopLeft,
  .layout = Ui_Layout_Horizontal,
};

global_variable Ui_Box sub_menu_box =  (Ui_Box){
  .align = Ui_Align_TopLeft,
  .layout = Ui_Layout_Vertical,
  .sizing = Ui_Sizing_FitContentsX,
};

// Open File UI
global_variable Ui_Box open_file_box = (Ui_Box){
  .position = (Vector2){100.0f, 100.0f},
  .min_size = (Vector2){300.0f, 0.0f},
  .align = Ui_Align_TopLeft,
  .layout = Ui_Layout_Vertical,
  .sizing = Ui_Sizing_FitContents,
  .flags = Ui_Box_Flag_ShouldDraw,
  .color = (Color){200.0f, 200.0f, 200.0f, 255.0f},
};
global_variable Ui_Box file_list_box = (Ui_Box){
  .align = Ui_Align_TopLeft,
  .layout = Ui_Layout_Vertical,
  .sizing = Ui_Sizing_FitContents,
  .flags = Ui_Box_Flag_Clip|Ui_Box_Flag_ScrollY|Ui_Box_Flag_Stretch,
  .max_size = (Vector2){0.0f, 100.0f},
};
global_variable Ui_Box open_file_confirm_box = (Ui_Box){
  .align = Ui_Align_TopRight, // TODO: The right-alignment is broken... should fix that at some point...
  .layout = Ui_Layout_Horizontal,
  .sizing = Ui_Sizing_FitContents,
};

global_variable Process open_file_label = (Process){
  .flags = Process_Flag_UseLabelCString|Process_Flag_FitToText,
  .label_c_string = (U8 *)"Open File...",
  .margin = (Vector2){5.0f, 8.0f},
};
global_variable Process open_button = (Process){
  .flags = Process_Flag_UseLabelCString|Process_Flag_FitToText|Process_Flag_Clickable,
  .label_c_string = (U8 *)"Open",
  .margin = (Vector2){5.0f, 8.0f},
};
global_variable Process cancel_button = (Process){
  .flags = Process_Flag_UseLabelCString|Process_Flag_FitToText|Process_Flag_Clickable,
  .label_c_string = (U8 *)"Cancel",
  .margin = (Vector2){5.0f, 8.0f},
};
global_variable Process save_button = (Process){
  .flags = Process_Flag_UseLabelCString|Process_Flag_FitToText|Process_Flag_Clickable,
  .label_c_string = (U8 *)"Save",
  .margin = (Vector2){5.0f, 8.0f},
};

// Save File As UI
global_variable Ui_Box save_file_as_box = (Ui_Box){
  .position = (Vector2){100.0f, 100.0f},
  .min_size = (Vector2){300.0f, 0.0f},
  .align = Ui_Align_TopLeft,
  .layout = Ui_Layout_Vertical,
  .sizing = Ui_Sizing_FitContents,
  .flags = Ui_Box_Flag_ShouldDraw,
  .color = (Color){200.0f, 200.0f, 200.0f, 255.0f},
};
global_variable Process  save_file_as_text_input = (Process){
  .flags = Process_Flag_TextEdit|Process_Flag_FitToText|Process_Flag_Clickable|Process_Flag_CanBeActive,
};
global_variable Ui_Box save_file_as_confirm_box = (Ui_Box){
  .align = Ui_Align_TopRight, // TODO: The right-alignment is broken... should fix that at some point...
  .layout = Ui_Layout_Horizontal,
  .sizing = Ui_Sizing_FitContents,
};





function B32 rectangle_contains_point(Rectangle r, Vector2 p) {
  F32 x2 = r.x + r.width;
  F32 y2 = r.y + r.height;
  B32 contains = (p.x >= r.x) && (p.y >= r.y) && (p.x <= x2) && (p.y <= y2);
  return contains;
}




function String_Chunk *create_string_chunk(Context *context) {
  String_Chunk *c = context->free_strings.first;

  if (c) {
    SLLQueuePop(context->free_strings.first,context->free_strings.last);
  } else {
    c = push_struct(context->permanent_arena, String_Chunk);
  }

  if (c) {
    *c = (String_Chunk){0};
  } else {
    c = The_Null_String_Chunk();
  }

  return c;
}

function void free_string_chunk(Context *context, String_Chunk *chunk) {
  SLLQueuePush(context->free_strings.first, context->free_strings.last, chunk);
  chunk->next = 0;
}


function String_Chunk_List string_chunk_list_from_string8(Context *context, String8 string8) {
  String_Chunk_List list = (String_Chunk_List){0};

  U64 remaining_size = string8.size;
  U64 string8_index = 0;

  for (;;) {
    if (remaining_size == 0) {
      break;
    }

    String_Chunk *chunk = create_string_chunk(context);
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
    String_Chunk *chunk = create_string_chunk(context);
    SLLQueuePush(list.first, list.last, chunk);
  }

  return list;
}


function void free_ui_element(Context *context, Process *e) {
  SLLQueuePush(context->free_ui_elements.first, context->free_ui_elements.last, e);
  e->next = 0;
}

function Process *create_ui_element(Context *context) {
  Process *e = context->free_ui_elements.first;

  if (e) {
    SLLQueuePop(context->free_ui_elements.first, context->free_ui_elements.last);
  } else {
    e = push_struct(context->permanent_arena, Process);
  }

  if (e) {
    *e = (Process){0};
  } else {
    e = The_Null_Process();
  }

  return e;
}



function Process *create_detached_process(Context *context) {
  Process *p = context->free_processes.first;

  if (p) {
    SLLQueuePop(context->free_processes.first, context->free_processes.last);
  } else {
    p = push_struct(context->permanent_arena, Process);
  }

  if (p) {
    *p = (Process){0};
  } else {
    p = The_Null_Process();
  }

  return p;
}

function Process *create_process(Context *context) {
  Process *p = create_detached_process(context);

  if (p) {
    SLLQueuePush(context->processes.first, context->processes.last, p);
  }

  return p;
}

function void clear_active_processes(Context *context) {
  if (context->active_processes.first) {
    for (Process *p = context->active_processes.first; p != 0;) {
      Process *next_active = p->next_active;
      p->next_active = 0;
      p = next_active;
    }
  }

  context->active_processes.first = 0;
  context->active_processes.last = 0;
}

function void remove_process_from_process_list(Context *context, Process_List *list, Process *p) {
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

  SLLQueuePush(context->free_processes.first, context->free_processes.last, p);
}

function void remove_string_chunk_list(Context *context, String_Chunk_List *scl) {
  if (scl->first && scl->last) {
    if (context->free_strings.first && context->free_strings.last) {
      context->free_strings.last->next = scl->first;
      context->free_strings.last = scl->last;
    } else {
      context->free_strings.first = scl->first;
      context->free_strings.last = scl->last;
    }
  }
}

function void free_ui_element_list(Context *context, Process_List *l) {
  // free the string-chunks
  for (Process *e = l->first; e != 0; e = e->next) {
    remove_string_chunk_list(context, &e->label);
  }

  // free the element-list
  if (l->first && l->last) {
    if (context->free_ui_elements.first && context->free_ui_elements.last) {
      context->free_ui_elements.last->next = l->first;
      context->free_ui_elements.last = l->last;
    } else {
      context->free_ui_elements.first = l->first;
      context->free_ui_elements.last = l->last;
    }
  }
}

function String_Chunk_List copy_string_chunk_list(Context *context, String_Chunk_List *scl) {
  String_Chunk_List result = (String_Chunk_List){0};

  for (String_Chunk *sc = scl->first; sc != 0; sc = sc->next) {
    String_Chunk *new_sc = create_string_chunk(context);
    if (new_sc) {
      *new_sc = *sc;
      SLLQueuePush(result.first, result.last, new_sc);
    }
  }

  return result;
}

function void clear_processes(Context *context) {
  clear_active_processes(context);

  for (Process *p = context->processes.first; p != 0;) {
    Process *next_process = p->next;
    remove_process_from_process_list(context, &context->processes, p);

    // remove string-chunks
    remove_string_chunk_list(context, &p->label);

    p = next_process;
  }
}

function void remove_copy_process_list(Context *context, Process_List *list) {
  // TODO: @Speed can probably do some fancy stuff with just the ends of the list?
  for (Process *p = list->first; p != 0;) {
    Process *next_process = p->next;
    remove_process_from_process_list(context, &context->copy_processes, p);
    p = next_process;
  }
}

function Process *add_process_to_copy_list(Context *context, Process *p, Vector2 *copy_center, F32 *copy_count) {
  Process *copied_p = create_detached_process(context);
  *copied_p = *p;
  p->to_copied = copied_p;
  *copy_center = Vector2Add(*copy_center, p->position);
  *copy_count += 1.0f;
  SLLQueuePush(context->copy_processes.first, context->copy_processes.last, copied_p);

  return copied_p;
}

function void copy_active_processes(Context *context) {
  B32 error = 0;
  Vector2 copy_center = (Vector2){0};
  F32 copy_count = 0.0f;

  // remove whatever processes were already in the copy-list
  remove_copy_process_list(context, &context->copy_processes);

  // copy processes from active-list to copy-list
  for (Process *a = context->active_processes.first; a != 0; a = a->next_active) {
    if (Get_Flag(a->flags, Process_Flag_Wire)) {
      // add connected processes if they have not been added yet
      for (S32 conn = 0; conn < Process_Connection__Count; ++conn) {
        if (a->conn[conn] && a->conn[conn]->to_copied == 0) {
          B32 found_conn = 0;
          for (Process *test_p = context->active_processes.first; test_p != 0; test_p = test_p->next_active) {
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
      Process *copied_wire = create_detached_process(context);
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
  for (Process *p = context->processes.first; p != 0; p = p->next) {
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
    if (c->label.first) {
      c->label = copy_string_chunk_list(context, &c->label);
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

function void paste_processes(Context *context) {
  Vector2 mouse_world_pos = GetScreenToWorld2D(context->ui_state.mouse_position, context->camera);
  Vector2 center_delta = Vector2Subtract(mouse_world_pos, context->copy_center);

  for (Process *p = context->copy_processes.first; p != 0;) {
    Process *next_p = p->next;
    if (!Get_Flag(p->flags, Process_Flag_Wire)) {
      p->position = Vector2Add(p->position, center_delta);
    }

    SLLQueuePush(context->processes.first, context->processes.last, p);
    p = next_p;
  }

  context->copy_processes.first = 0;
  context->copy_processes.last = 0;
}



function B32 is_active_process(Context *context, Process *p) {
  B32 is_active = 0;

  for (Process *test_p = context->active_processes.first; test_p != 0; test_p = test_p->next_active) {
    if (test_p == p) {
      is_active = 1;
      break;
    }
  }

  return is_active;
}





////////////////////////////////////////
// UI Functions
////////////////////////////////////////

function void clear_ui_state(Context *context) {
  // free string-chunks
  for (Process *e = context->save_file_list.first; e != 0; e = e->next) {
    remove_string_chunk_list(context, &e->label);
  }

  context->save_file_list.first = 0;
  context->save_file_list.last = 0;

  arena_pop_to(context->ui_arena, 0);
}

function void set_menu_state(Context *context, Menu_State menu_state) {
  switch(context->menu_state) {
  case Menu_State_OpenFile:
  case Menu_State_SaveFileAs: {
    clear_ui_state(context);
  } break;
  }

  switch(menu_state) {
  case Menu_State_OpenFile:
  case Menu_State_SaveFileAs: {
    collect_save_files(context);
  } break;
  }

  context->menu_state = menu_state;
}

function void set_menu_state_as_open_file(Context *context, Process *element) {
  set_menu_state(context, Menu_State_OpenFile);
}

function void set_menu_state_as_save_file_as(Context *context, Process *element) {
  set_menu_state(context, Menu_State_SaveFileAs);
}


function void handle_label_editing(Context *context, Process_List ps) {
  U32 key = 0;
  U32 k = 0;
  B32 shift_down = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

  while ((key = context->ui_state.key_presses[k++])) {
    for (Process *a = ps.first; a != 0; a = a->next_active) {
      if (Get_Flag(a->flags, Process_Flag_TextEdit)) {
        B32 is_ascii = key > 0 && key < 256;
        U8 c = ascii_char_lookup[key&0xff][shift_down];
        if (is_ascii && c != 0) {
          // push string-chunk if string list is empty
          if (a->label.last == 0) {
            String_Chunk *sc = create_string_chunk(context);
            SLLQueuePush(a->label.first, a->label.last, sc);
            a->label_cursor = 0;
          }
          // add char to label
          a->label.last->str_array[a->label_cursor] = c;
          a->label_cursor += 1;
          // push string-chunk if at the end of current chunk
          if (a->label_cursor == String_Chunk_Size) {
            String_Chunk *sc = create_string_chunk(context);
            SLLQueuePush(a->label.first, a->label.last, sc);
            a->label_cursor = 0;
          }
        } else if (key == KEY_BACKSPACE) {
          if (a->label_cursor == 0) {
            // only free string-chunk if it is _not_ the only chunk
            if (a->label.first != a->label.last) {
              String_Chunk *free_chunk = a->label.last;
              if (free_chunk) {
                a->label_cursor = String_Chunk_Size - 1;
                // remove last string-chunk
                for (String_Chunk *chunk = a->label.first; chunk != 0; chunk = chunk->next) {
                  if (chunk->next == a->label.last) {
                    chunk->next = 0;
                    a->label.last = chunk;
                    break;
                  }
                }
                // add unused string-chunk to free-list
                SLLQueuePush(context->free_strings.first, context->free_strings.last, free_chunk);
                // zero current character
                a->label.last->str_array[a->label_cursor] = 0;
              }
            }
          } else if (a->label.last) {
            // decrement and zero current character
            a->label_cursor -= 1;
            a->label.last->str_array[a->label_cursor] = 0;
          }
        }
      }
    }
  }
}



function Vector2 get_ui_element_size(Context *context, Process *element, B32 fit_to_text, U8 *label_c_string) {
  Vector2 size = element->size;
  U8 *label = label_c_string;
  F32 font_size = global_panel_font_size;
  Vector2 padding = global_button_padding;

  if (label == 0 && element->label.first && element->label.last) {
    label = c_string_from_string_chunk_list(render_GlobalTempArena, &element->label);
  }

  if (fit_to_text) {
    size.x = (F32)MeasureText((char *)label, font_size) + 2.0f*padding.x;
    size.y = font_size + 2.0f*padding.y;
  }

  size = Vector2Add(size, Vector2Scale(element->margin, 2.0f));

  // HACK: Round up because having values close to integers can cause visual "gaps" between rectangles and stuff...
  size.x = ceil_F32(size.x);
  size.y = ceil_F32(size.y);

  return size;
}


function Vector2 get_ui_box_inner_position(Context *context, Ui_Box *box) {
  Vector2 position = Vector2Add(Vector2Add(box->position, box->offset), box->scroll_offset);
  return position;
}


function Vector2 get_box_size(Ui_Box *box) {
  Ui_Box *parent_box = box->next;
  B32 stretch = Get_Flag(box->flags, Ui_Box_Flag_Stretch);
  Vector2 size = box->raw_size;

  if (stretch && parent_box) {
    Vector2 parent_size = get_box_size(parent_box);
    // TODO: Do we need to recursively call get_box_size here??
    if (box->layout == Ui_Layout_Vertical) {
      size.x = parent_size.x;
    } else if (box->layout == Ui_Layout_Horizontal) {
      size.y = parent_size.y;
    }
  }

  if (box->min_size.x > 0.0f) {
    size.x = Max(size.x, box->min_size.x);
  }
  if (box->min_size.y > 0.0f) {
    size.y = Max(size.y, box->min_size.y);
  }
  if (box->max_size.x > 0.0f) {
    size.x = Min(size.x, box->max_size.x);
  }
  if (box->max_size.y > 0.0f) {
    size.y = Min(size.y, box->max_size.y);
  }

  return size;
}

function B32 ui_box_should_set_x(Ui_Box *box) {
  B32 result = (box->sizing == Ui_Sizing_FitContents ||
                box->sizing == Ui_Sizing_FitContentsX);

  return result;
}

function B32 ui_box_should_set_y(Ui_Box *box) {
  B32 result = (box->sizing == Ui_Sizing_FitContents ||
                box->sizing == Ui_Sizing_FitContentsY);

  return result;
}

function void set_ui_box_size(Ui_Box *box, Vector2 size, B32 set_box_x, B32 set_box_y) {
  if (set_box_x) {
    if (box->layout == Ui_Layout_Horizontal) {
      box->raw_size.x += size.x;
    } else {
      box->raw_size.x = Max(box->raw_size.x, size.x);
    }
  }
  if (set_box_y) {
    if (box->layout == Ui_Layout_Vertical) {
      box->raw_size.y += size.y;
    } else {
      box->raw_size.y = Max(box->raw_size.y, size.y);
    }
  }
}


function B32 do_ui_element(Context *context, Process *element, B32 sizing) {
  B32 interacted = 0;
  B32 is_hot = 0;

  Render_Context *rc = &context->ui_render_context;
  Ui_State *ui_state = &context->ui_state;

  F32 font_size = global_panel_font_size;
  Vector2 padding = global_button_padding;
  Color dormant_bg_color = global_button_dormant_bg_color;
  Color hot_bg_color = global_button_hot_bg_color;
  Color font_color = global_button_font_color;

  Ui_Box *box = context->ui_box_stack.first;
  Ui_Align align = (box == 0) ? Ui_Default_Align : box->align;
  Ui_Layout layout = (box == 0) ? Ui_Default_Layout : box->layout;
  Vector2 box_position = (box == 0) ? Ui_Default_Position : box->position;
  Vector2 box_offset = (box == 0) ? Ui_Default_Offset : box->offset;

  B32 set_box_x = box && ui_box_should_set_x(box);
  B32 set_box_y = box && ui_box_should_set_y(box);

  if (sizing) {
    if (!Get_Flag(element->flags, Process_Flag_UseLabelCString)) {
      element->label_c_string = 0;
      if (element->label.first && element->label.last) {
        element->label_c_string = c_string_from_string_chunk_list(render_GlobalTempArena, &element->label);
      }
    }

    B32 fit_to_text = Get_Flag(element->flags, Process_Flag_FitToText);
    element->size = get_ui_element_size(context, element, fit_to_text, element->label_c_string);

    set_ui_box_size(box, element->size, set_box_x, set_box_y);

    switch (align) {
    case Ui_Align_Top: {
      box_position.x -= 0.5f * element->size.x;
    } break;
    case Ui_Align_TopLeft: {
    } break;
    case Ui_Align_Left: {
      box_position.y -= 0.5f * element->size.y;
    } break;
    case Ui_Align_BottomLeft: {
      box_position.y -= element->size.y;
    } break;
    case Ui_Align_Bottom: {
      box_position.x -= 0.5f * element->size.x;
      box_position.y -= element->size.y;
    } break;
    case Ui_Align_BottomRight: {
      box_position.x -= element->size.x;
      box_position.y -= element->size.y;
    } break;
    case Ui_Align_Right: {
      box_position.x -= element->size.x;
      box_position.y -= 0.5f * element->size.y;
    } break;
    case Ui_Align_TopRight: {
      box_position.x -= element->size.x;
    } break;
    }
  } else {
    // @Copypasta ui_box_end
    Vector2 next_offset;
    switch (layout) {
    default:
    case Ui_Layout_None: {
      next_offset = Zero_Struct(Vector2);
    } break;
    case Ui_Layout_Vertical: {
      next_offset = (Vector2){0.0f, element->size.y};
      element->position = get_ui_box_inner_position(context, box);
    } break;
    case Ui_Layout_Horizontal: {
      element->position = get_ui_box_inner_position(context, box);
      next_offset = (Vector2){element->size.x, 0.0f};
    } break;
    }

    box->offset = Vector2Add(box->offset, next_offset);
    Vector2 box_size = get_box_size(box);

    if (set_box_x && layout == Ui_Layout_Vertical) {
      element->size.x = box_size.x;
    }
    if (set_box_y && layout == Ui_Layout_Horizontal) {
      element->size.y = box_size.y;
    }

    Rectangle element_rect = (Rectangle){
      element->position.x+element->margin.x,
      element->position.y+element->margin.y,
      element->size.x-2.0f*element->margin.x,
      element->size.y-2.0f*element->margin.y,
    };
    Rectangle box_rect = (Rectangle){box->position.x, box->position.y, box_size.x, box_size.y};
    B32 in_bounds = 1;
    if (Get_Flag(box->flags, Ui_Box_Flag_Clip)) {
      in_bounds = CheckCollisionRecs(element_rect, box_rect);
    }

    if (in_bounds) {
      B32 hover_element = rectangle_contains_point(element_rect, context->ui_state.mouse_position);
      B32 hover_box = (!Get_Flag(box->flags, Ui_Box_Flag_Clip) ||
                       rectangle_contains_point(box_rect, context->ui_state.mouse_position));
      if (Get_Flag(element->flags, Process_Flag_Clickable) &&
          !Get_Flag(ui_state->flags, Ui_State_Flag_action_occured) &&
          hover_element && hover_box) {
        context->hot_process = 0;
        is_hot = 1;

        if (IsMouseButtonPressed(0)) {
          interacted = 1;
          Set_Flag(ui_state->flags, Ui_State_Flag_action_occured);
          if (Get_Flag(element->flags, Process_Flag_CanBeActive)) {
            clear_active_processes(context);
            SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, element, next_active, 0);
          }
        }
      }

      B32 is_hot_bg_color = is_hot || element == context->selected_element;
      Color bg_color = is_hot_bg_color ? hot_bg_color : dormant_bg_color;
      if (is_hot) {
        context->hot_process = element;
      }

      render_DrawRectangle(rc, element_rect.x, element_rect.y, element_rect.width, element_rect.height, bg_color);

      if (element->label_c_string) {
        render_DrawText(rc, (char *)element->label_c_string, element_rect.x+padding.x+1.0f, element_rect.y+padding.y+1.0f, font_size, (Color){0, 0, 0, 255}, 0);
        render_DrawText(rc, (char *)element->label_c_string, element_rect.x+padding.x, element_rect.y+padding.y, font_size, font_color, 0);
      }
    }
  }

  return interacted;
}


function Process *create_button(Arena *arena, Vector2 position, String_Chunk_List label) {
  Process *button = push_struct(arena, Process);
  Vector2 padding = global_button_padding;
  F32 font_size = global_panel_font_size;

  if (button) {
    button->label = label;
    U8 *label_c_string = c_string_from_string_chunk_list(render_GlobalTempArena, &button->label);
    S32 text_width = MeasureText((char *)label_c_string, font_size);
    button->size.x = text_width + 2.0f*padding.x;
    button->size.y = font_size + 2.0f*padding.y;

    Set_Flag(button->flags, Process_Flag_Clickable|Process_Flag_FitToText);
    button->position = position;
    button->label = label;
  }

  return button;
}



function void ui_box_begin(Context *context, Ui_Box *box, B32 sizing) {
  Render_Context *rc = &context->ui_render_context;
  Ui_State *ui_state = &context->ui_state;
  Ui_Box *parent_box = context->ui_box_stack.first;

  SLLQueuePushFront(context->ui_box_stack.first, context->ui_box_stack.last, box);

  if (sizing) {
    box->offset = (Vector2){0.0f, 0.0f};
    if (box->sizing == Ui_Sizing_FitContents || box->sizing == Ui_Sizing_FitContentsX) {
      box->raw_size.x = 0;
    }
    if (box->sizing == Ui_Sizing_FitContents || box->sizing == Ui_Sizing_FitContentsY) {
      box->raw_size.y = 0;
    }
  }

  if (!sizing) {
    Vector2 size = get_box_size(box);
    Rectangle box_rect = (Rectangle){box->position.x, box->position.y, size.x, size.y};
    // positioning
    if (parent_box) {
      box->position = get_ui_box_inner_position(context, parent_box);
    }
    if (Get_Flag(box->flags, Ui_Box_Flag_ShouldDraw)) {
      render_DrawRectangle(rc, box_rect.x, box_rect.y, box_rect.width, box_rect.height, box->color);
    }
    if (rectangle_contains_point(box_rect, ui_state->mouse_position)) {
      // handle scrolling
      if (Get_Flag(box->flags, Ui_Box_Flag_ScrollY) &&
          ui_state->mouse_wheel_movement.y != 0) {
        Set_Flag(ui_state->flags, Ui_State_Flag_action_occured);
        F32 max_scroll_offset = box->raw_size.y - size.y;
        box->scroll_offset.y += ui_state->mouse_wheel_movement.y;
        box->scroll_offset.y = Clamp(box->scroll_offset.y, -max_scroll_offset, 0.0f);
      }
    }
    if (Get_Flag(box->flags, Ui_Box_Flag_Clip)) {
      render_BeginScissorMode(rc, box->position, size);
    }
  }
}

function void ui_box_end(Context *context, Ui_Box *box, B32 sizing) {
  Render_Context *rc = &context->ui_render_context;

  if (box != context->ui_box_stack.first) {
    printf("[ Error ] Popping ui-box off of stack but given box does not match. Box passed in is %p while the box popped of the stack is %p .\n", box, context->ui_box_stack.first);
  }

  SLLQueuePop(context->ui_box_stack.first, context->ui_box_stack.last);

  Ui_Box *parent_box = context->ui_box_stack.first;

  if (sizing && parent_box) {
    B32 set_box_x = ui_box_should_set_x(parent_box);
    B32 set_box_y = ui_box_should_set_y(parent_box);
    Vector2 box_size = get_box_size(box);

    set_ui_box_size(parent_box, box_size, set_box_x, set_box_y);
  }

  if (!sizing) {
    if (parent_box) {
      // @Copypasta do_ui_element
      Vector2 box_size = get_box_size(box);
      Vector2 next_offset;
      switch (parent_box->layout) {
      default:
      case Ui_Layout_None: {
        next_offset = Zero_Struct(Vector2);
      } break;
      case Ui_Layout_Vertical: {
        next_offset = (Vector2){0.0f, box_size.y};
      } break;
      case Ui_Layout_Horizontal: {
        next_offset = (Vector2){box_size.x, 0.0f};
      } break;
      }
      parent_box->offset = Vector2Add(parent_box->offset, next_offset);
    }
    if (Get_Flag(box->flags, Ui_Box_Flag_Clip)) {
      render_EndScissorMode(rc);
    }
  }
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


function void save_file(Context *context, Process *element) {
  /* Assert(!"TODO"); */
  write_save_file(context, context->temp_arena, context->save_file_name);
}


function void do_open_file(Context *context, B32 sizing) {
  F32 padding = 2.0f;

  ui_box_begin(context, &open_file_box, sizing);
  {
    do_ui_element(context, &open_file_label, sizing);
    ui_box_begin(context, &file_list_box, sizing);
    {
      for (Process *file = context->save_file_list.first; file != 0; file = file->next) {
        if (do_ui_element(context, file, sizing)) {
          context->selected_element = file;
        }
      }
    }
    ui_box_end(context, &file_list_box, sizing);
    ui_box_begin(context, &open_file_confirm_box, sizing);
    {
      B32 open_clicked = do_ui_element(context, &open_button, sizing);
      B32 cancel_clicked = do_ui_element(context, &cancel_button, sizing);

      if (open_clicked) {
        open_file_and_replace_processes(context, context->selected_element->label);
      } else if (cancel_clicked) {
        set_menu_state(context, 0);
      }
    }
    ui_box_end(context, &open_file_confirm_box, sizing);
  }
  ui_box_end(context, &open_file_box, sizing);
}


function void do_save_file_as(Context *context, B32 sizing) {
  ui_box_begin(context, &save_file_as_box, sizing);
  {
    do_ui_element(context, &save_file_as_text_input, sizing);

    ui_box_begin(context, &save_file_as_confirm_box, sizing);
    {
      B32 save_clicked = do_ui_element(context, &save_button, sizing);
      B32 cancel_clicked = do_ui_element(context, &cancel_button, sizing);

      if (save_clicked) {
        set_as_current_file(context, save_file_as_text_input.label);
        save_file(context, &save_file_as_text_input);
        set_menu_state(context, 0);
      } else if (cancel_clicked) {
        set_menu_state(context, 0);
      }
    }
    ui_box_end(context, &save_file_as_confirm_box, sizing);
  }
  ui_box_end(context, &save_file_as_box, sizing);
}

function void handle_copy(Context *context, Process *element) {
  copy_active_processes(context);
}

function void handle_paste(Context *context, Process *element) {
  paste_processes(context);
}




function Vector2 get_percentage_between_points(Vector2 p0, Vector2 p1, F32 percentage) {
  Vector2 norm_delta = Vector2Normalize(Vector2Subtract(p1, p0));
  F32 distance_along_delta = percentage * Vector2Distance(p1, p0);
  Vector2 center = Vector2Add(p0, Vector2Scale(norm_delta, distance_along_delta));

  return center;
}







function Vector2 get_process_position(Context *context, Process *process) {
  Vector2 position = process->position;
  B32 is_active = is_active_process(context, process);
  B32 is_dragging = Get_Flag(context->flags, Context_Flag_Dragging);


  if (is_active && is_dragging) {
    Vector2 delta = Vector2Subtract(context->ui_state.mouse_position, context->active_position);
    position = Vector2Add(position, Vector2Scale(delta, 1.0f/context->camera.zoom));
  }

  return position;
}








function Vector2
get_process_wire_position(Context *context, Process *p, Process_Shape shape, Process_Connection conn, U32 wire_index) {
  F32 padding = context->camera.zoom * global_process_wire_padding;
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

  Vector2 delta = Vector2Subtract(p0, p1);
  Vector2 delta_norm = Vector2Normalize(delta);
  F32 inner_distance = fmax(0.0f, Vector2Distance(p0, p1) - 2.0f*padding);
  F32 chunk_size = inner_distance / (F32)(p->conn_count[conn]+1);
  F32 distance_from_point = padding + chunk_size*(F32)(wire_index+1);

  Vector2 wire_position = Vector2Add(p1, Vector2Scale(delta_norm, distance_from_point));

  return wire_position;
}



function Rectangle get_wire_box(Context *context, Vector2 position) {
  F32 size = context->camera.zoom * global_box_size;
  F32 half_size = context->camera.zoom * global_box_half_size;
  Rectangle box = (Rectangle){position.x-half_size, position.y-half_size, size, size};
  return box;
}



function Rectangle get_new_wire_box(Context *context, Process *p, Process_Shape shape) {
  Vector2 position = shape.new_wire_position;
  F32 size = context->camera.zoom * global_box_size;
  F32 half_size = context->camera.zoom * global_box_half_size;

  Rectangle new_wire_box = (Rectangle){position.x - half_size,
                                       position.y - half_size,
                                       size, size};

  return new_wire_box;
}


function Rectangle get_selection_rectangle(Context *context) {
  F32 x = fmin(context->active_position.x, context->ui_state.mouse_position.x);
  F32 y = fmin(context->active_position.y, context->ui_state.mouse_position.y);
  F32 x1 = fmax(context->active_position.x, context->ui_state.mouse_position.x);
  F32 y1 = fmax(context->active_position.y, context->ui_state.mouse_position.y);
  Rectangle selection_rect = (Rectangle){x, y, x1-x, y1-y};

  return selection_rect;
}




function Process *get_process_wire_by_selection(Context *context, Process_Selection selection) {
  Process *wire = 0;

  for (Process *p = context->processes.first; p != 0; p = p->next) {
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






function void remove_process_from_active_processes(Context *context, Process *p) {
  if (context->active_processes.first == p) {
    SLLQueuePop_NZ(context->active_processes.first, context->active_processes.last, next_active, 0);
  } else {
    for (Process *test_p = context->active_processes.first; test_p != 0; test_p = test_p->next_active) {
      if (test_p->next_active == p) {
        test_p->next_active = p->next_active;
        if (p == context->active_processes.last) {
          context->active_processes.last = test_p;
        }
        break;
      }
    }
  }
}


function void
add_wire_connection(Context *context, Process *wire, Process *process, Process_Connection conn, U32 which_conn) {
  // Add wire at the given connection index, moving any wires that come after that index over to the right.
  wire->conn[conn] = process;
  wire->which_conn[conn] = which_conn;

  for (Process *test_wire = context->processes.first; test_wire != 0; test_wire = test_wire->next) {
    if (wire != test_wire &&
        test_wire->conn[conn] == process &&
        test_wire->which_conn[conn] >= which_conn) {
      // increment the wire's which_conn if it comes at or after the added wire's which_conn
      test_wire->which_conn[conn] += 1;
    }
  }

  process->conn_count[conn] += 1;
}

function void
remove_wire_connection(Context *context, Process *wire, Process_Connection_Flag conn_flags) {
  // Remove a wire and move wires to the right of it to the left one.
  B32 in_matched = 0;
  B32 out_matched = 0;

  B32 remove_in = Get_Flag(conn_flags, Process_Connection_Flag_In);
  B32 remove_out = Get_Flag(conn_flags, Process_Connection_Flag_Out);

  for (Process *test_wire = context->processes.first; test_wire != 0; test_wire = test_wire->next) {
    // adjust in-connections that come after deleted wire
    if (remove_in && test_wire->in == wire->in) {
      if (test_wire->which_in > wire->which_in) {
        test_wire->which_in -= 1;
      }
      in_matched = 1;
    }

    // adjust out-connections that come after deleted wire
    if (remove_out && test_wire->out == wire->out) {
      if (test_wire->which_out > wire->which_out) {
        test_wire->which_out -= 1;
      }
      out_matched = 1;
    }
  }

  B32 only_in_conn = wire->in != 0 && wire->which_in == 0;
  B32 only_out_conn = wire->out != 0 && wire->which_out == 0;

  // decrement process' in-count
  if (remove_in && (in_matched || only_in_conn)) {
    if (wire->in) {
      wire->in->in_count -= 1;
    }
  }

  // decrement process' out-count
  if (remove_out && (out_matched || only_out_conn)) {
    if (wire->out) {
      wire->out->out_count -= 1;
    }
  }
}



function void delete_process(Context *context, Process *p) {
  // TODO: Do we really want to check if it is a live process? Maybe sometimes the program wants to delete a process that is *not* in the processes list and still want it to end up in the free-list.
  B32 is_live_process = 0;
  for (Process *test_p = context->processes.first; test_p != 0; test_p = test_p->next) {
    if (test_p == p) {
      is_live_process = 1;
      break;
    }
  }

  if (is_live_process) {
    // if deleting a wire, adjust connected processes
    if (Get_Flag(p->flags, Process_Flag_Wire)) {
      U32 both_conns = Process_Connection_Flag_In | Process_Connection_Flag_Out;
      remove_wire_connection(context, p, both_conns);
    }

    // remove p from processes
    remove_process_from_process_list(context, &context->processes, p);

    // free string-chunks from label
    remove_string_chunk_list(context, &p->label);

    // check for wires connected to the deleted process, and delete those also
    for (Process *wire = context->processes.first; wire != 0;) {
      B32 in_match = wire->in == p;
      B32 out_match = wire->out == p;
      B32 should_delete = 0;

      if (in_match || out_match) {
        if (!in_match) {
          // adjust in-connections to deleted wire
          for (Process *test_wire = context->processes.first; test_wire != 0; test_wire = test_wire->next) {
            if (test_wire->in == wire->in &&
                test_wire->which_in > wire->which_in) {
              test_wire->which_in -= 1;
            }
          }

          if (wire->in) {
            wire->in->in_count -= 1;
          }
        }

        if (!out_match) {
          // adjust out-connections to deleted wire
          for (Process *test_wire = context->processes.first; test_wire != 0; test_wire = test_wire->next) {
            if (test_wire->out == wire->out &&
                test_wire->which_out > wire->which_out) {
              test_wire->which_out -= 1;
            }
          }

          if (wire->out) {
            wire->out->out_count -= 1;
          }
        }

        should_delete = 1;
      }

      if (should_delete) {
        Process *next_process = wire->next;
        // remove wire from processes
        remove_process_from_process_list(context, &context->processes, wire);
        wire = next_process;
      } else {
        wire = wire->next;
      }
    }
  }
}






function void connect_processes(Context *context, Process *out, Process *in) {
  Process *new_wire = create_process(context);

  if (new_wire && out && in) {
    Set_Flag(new_wire->flags, Process_Flag_Wire);
    new_wire->out = out;
    new_wire->in = in;

    new_wire->which_out = out->out_count;
    new_wire->which_in = in->in_count;

    out->out_count += 1;
    in->in_count += 1;
  }
}


function Half_Circle_Points
get_half_circle_points(Context *context, Process_Shape shape, Process *p, Vector2 position, S32 text_width, B32 downward) {
  Half_Circle_Points half_circle_points;
  F32 padding = context->camera.zoom * global_process_wire_padding;
  F32 spacing = context->camera.zoom * global_process_wire_spacing;

  F32 height = context->camera.zoom * global_shape_size;
  F32 half_height = context->camera.zoom * global_shape_half_size;

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


function void
fill_out_half_circle_shape(Context *context, Process_Shape *shape, Process *p, Vector2 position, S32 text_width, B32 downward) {
  Half_Circle_Points half_circle_points = get_half_circle_points(context, *shape, p, position, text_width, downward);

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



function Process_Shape
get_process_shape(Context *context, Process *p) {
  Process_Shape shape = {0};
  U64 arena_pop_pos = arena_current_pos(context->temp_arena);

  F32 font_size = context->camera.zoom * global_process_font_size;
  U8 *label_c_string = c_string_from_string_chunk_list(context->temp_arena, &p->label);
  S32 text_width = MeasureText((char *)label_c_string, font_size);

  Vector2 position = get_process_position(context, p);
  position = GetWorldToScreen2D(position, context->camera);

  F32 half_size = context->camera.zoom * global_shape_half_size;
  F32 quarter_size = context->camera.zoom * global_shape_size / 4.0f;
  F32 padding = context->camera.zoom * global_process_wire_padding;
  F32 spacing = context->camera.zoom * global_process_wire_spacing;

  S32 has_in = p->in_count > 0;
  S32 has_out = p->out_count > 0;

  B32 rounded = Get_Flag(context->flags, Context_Flag_RoundedShapes);

  if (has_in && has_out) {
    // rectangular
    F32 max_conn = (F32)Max(p->in_count, p->out_count);
    F32 half_width = 0.5f*(2.0f*padding + max_conn*spacing);
    // fit shape to text if text is wide enough
    if ((F32)text_width > half_width) {
      half_width = 0.5f*(2.0f*padding + (F32)text_width);
    }
    shape.kind = Process_Shape_TriangleStrip;
    shape.point_count = 4;
    shape.triangle_count = 2;
    shape.points[0].x = position.x + half_width;
    shape.points[0].y = position.y - half_size;
    shape.points[1].x = position.x - half_width;
    shape.points[1].y = position.y - half_size;
    shape.points[2].x = position.x + half_width;
    shape.points[2].y = position.y + half_size;
    shape.points[3].x = position.x - half_width;
    shape.points[3].y = position.y + half_size;
    shape.center = get_percentage_between_points(shape.points[0], shape.points[3], 0.5f);
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
      fill_out_half_circle_shape(context, &shape, p, position, text_width, 0);
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
      fill_out_half_circle_shape(context, &shape, p, position, text_width, 1);
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

  arena_pop_to(context->temp_arena, arena_pop_pos);

  return shape;
}



function B32
triangle_fan_contains_point(Vector2 *points, S32 triangle_count, Vector2 point) {
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



function B32
triangle_strip_contains_point(Vector2 *points, S32 triangle_count, Vector2 point) {
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



function B32
process_shape_contains_point(Context *context, Process_Shape shape, Vector2 point) {
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
    contains = triangle_strip_contains_point(shape.points, shape.triangle_count, point);
  } break;
  default: Assert(0);
  }

  return contains;
}






function Process_Selection
get_process_selection(Context *context, Process *p) {
  Ui_State *ui_state = &context->ui_state;
  Process_Selection selection = {0};
  selection.index = -1;
  selection.process = p;

  Process_Shape shape = get_process_shape(context, p);
  Rectangle new_wire_box = get_new_wire_box(context, p, shape);

  if (!Get_Flag(ui_state->flags, Ui_State_Flag_hot_id_assigned)) {
    if (rectangle_contains_point(new_wire_box, context->ui_state.mouse_position)) {
      // check new-wire-box
      selection.type = Process_Selection_NewWire;
      context->hot_process = p;
      selection.hot_id_assigned = 1;
    } else {
      // check in wire-boxes
      for (U32 i = 0; i < p->in_count; ++i) {
        Vector2 in_position = get_process_wire_position(context, p, shape, Process_Connection_In, i);
        Rectangle r = get_wire_box(context, in_position);
        if (rectangle_contains_point(r, context->ui_state.mouse_position)) {
          selection.type = Process_Selection_In;
          selection.index = i;
          Process *wire = get_process_wire_by_selection(context, selection);
          context->hot_process = wire;
          selection.hot_id_assigned = 1;
          break;
        }
      }

      if (selection.type == 0) {
        // check out wire-boxes
        for (U32 i = 0; i < p->out_count; ++i) {
          Vector2 out_position = get_process_wire_position(context, p, shape, Process_Connection_Out, i);
          Rectangle r = get_wire_box(context, out_position);
          if (rectangle_contains_point(r, context->ui_state.mouse_position)) {
            selection.type = Process_Selection_Out;
            selection.index = i;
            Process *wire = get_process_wire_by_selection(context, selection);
            context->hot_process = wire;
            selection.hot_id_assigned = 1;
            break;
          }
        }
      }

      if (selection.type == 0 && !Get_Flag(p->flags, Process_Flag_Wire)) {
        if (process_shape_contains_point(context, shape, context->ui_state.mouse_position)) {
          // process selection
          selection.type = Process_Selection_Process;
          context->hot_process = p;
          selection.hot_id_assigned = 1;
        }
      }
    }
  }

  return selection;
}





function Keybind_Result check_keybind(Context *context, Ui_Feature feature, Process_Selection selection) {
  Keybind_Result result = 0;

  Keybind keybind = global_keybind_lookup[feature];
  Ui_State *ui_state = &context->ui_state;

  B32 key_is_pressed = 0;
  B32 key_is_down = 0;

  switch(keybind.key_kind) {
  case Key_Kind_Mouse0: {
    key_is_pressed = Get_Flag(ui_state->flags, Ui_State_Flag_mouse0_pressed);
    key_is_down = Get_Flag(ui_state->flags, Ui_State_Flag_mouse0_down);
  } break;
  case Key_Kind_Mouse1: {
    key_is_pressed = Get_Flag(ui_state->flags, Ui_State_Flag_mouse1_pressed);
    key_is_down = Get_Flag(ui_state->flags, Ui_State_Flag_mouse1_down);
  } break;
  case Key_Kind_MouseWheelUp: {
    key_is_pressed = ui_state->mouse_wheel_movement.y > 0.0f;
  } break;
  case Key_Kind_MouseWheelDown: {
    key_is_pressed = ui_state->mouse_wheel_movement.y < 0.0f;
  } break;
  default: {
    key_is_pressed = IsKeyPressed(keybind.key_kind);
    key_is_down = IsKeyDown(keybind.key_kind);
  } break;
  }

  B32 modifier_control = Get_Flag(keybind.modifiers, Modifier_Key_Control) ? 1 : 0;
  B32 modifier_shift = Get_Flag(keybind.modifiers, Modifier_Key_Shift) ? 1 : 0;
  B32 modifier_alt = Get_Flag(keybind.modifiers, Modifier_Key_Alt) ? 1 : 0;

  B32 modifier_matches = ((!(modifier_control ^ Get_Flag_Bool(ui_state->flags, Ui_State_Flag_control_down))) &&
                          (!(modifier_shift ^ Get_Flag_Bool(ui_state->flags, Ui_State_Flag_shift_down))) &&
                          (!(modifier_alt ^ Get_Flag_Bool(ui_state->flags, Ui_State_Flag_alt_down))));

  B32 constraint_hover_process =
    (Get_Flag(keybind.constraint, Ui_Constraint_HoverProcess)
     ? selection.type != 0
     : 1);
  B32 constraint_no_hover =
    (Get_Flag(keybind.constraint, Ui_Constraint_NoHotProcess)
     ? (context->hot_process == 0)
     : 1);
  B32 constraint_action_not_occured =
    (Get_Flag(keybind.constraint, Ui_Constraint_ActionNotOccured)
     ? (Get_Flag_Bool(ui_state->flags, Ui_State_Flag_action_occured) == 0)
     : 1);

  B32 constraints_met = (constraint_hover_process &&
                         constraint_no_hover &&
                         constraint_action_not_occured);

  if (key_is_pressed && modifier_matches && constraints_met) {
    result = Keybind_Result_Enter;
  }

  if (Get_Flag(keybind.constraint, Ui_Constraint_ExitOnKeyup)) {
    if (!key_is_down) {
      result = Keybind_Result_Exit;
    }
  }

  if (result == Keybind_Result_Enter) {
    Set_Flag(ui_state->flags, Ui_State_Flag_action_occured);
  }

  return result;
}



function Ui_State get_ui_state(Context *context) {
  Ui_State ui_state;
  ui_state.mouse_position = GetMousePosition(); // TODO: mouse_position should go in ui_state
  ui_state.mouse_wheel_movement = GetMouseWheelMoveV();

  Assign_Flag(ui_state.flags, Ui_State_Flag_mouse0_pressed, IsMouseButtonPressed(0));
  Assign_Flag(ui_state.flags, Ui_State_Flag_mouse1_pressed, IsMouseButtonPressed(1));
  Assign_Flag(ui_state.flags, Ui_State_Flag_mouse0_down, IsMouseButtonDown(0));
  Assign_Flag(ui_state.flags, Ui_State_Flag_mouse1_down, IsMouseButtonDown(1));
  Unset_Flag(ui_state.flags, Ui_State_Flag_hot_id_assigned);
  Assign_Flag(ui_state.flags, Ui_State_Flag_control_down, IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL));
  Assign_Flag(ui_state.flags, Ui_State_Flag_shift_down, IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT));
  Assign_Flag(ui_state.flags, Ui_State_Flag_alt_down, IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT));
  Unset_Flag(ui_state.flags, Ui_State_Flag_action_occured);

  return ui_state;
}


function void reset_ui_box(Context *context, Ui_Box *box) {
  box->raw_size = Zero_Struct(Vector2);
}





function Process create_lit_button(Context *context, String8 label, F32 x_pos, F32 y_pos) {
  Process button = Zero_Struct(Process);

  U32 flags = Process_Flag_Clickable | Process_Flag_FitToText;
  Set_Flag(button.flags, flags);

  button.position.x = x_pos;
  button.position.y = y_pos;

  button.label = string_chunk_list_from_string8(context, label);

  return button;
}

function void do_menu_ui(Context *context, B32 sizing) {
  S32 clicked_active_menu_button = -1;
  S32 hot_active_menu_button = -1;

  ui_box_begin(context, &top_menu_box, sizing);
  {
    for (U32 i = 0; i < Top_Menu_Count; ++i) {
      Process *menu_button = menu_buttons[i];
      if (do_ui_element(context, menu_button, sizing)) {
        clicked_active_menu_button = i;
      }

      if (context->hot_process == menu_button && Has_Active_Menu_Element(context)) {
        hot_active_menu_button = i;
      }
    }

    for (U32 i = 0; i < Top_Menu_Count; ++i) {
      Process *menu_button = menu_buttons[i];
      if (Top_Menu_Index(context->menu_state) == i) {
        Process **sub_menu = sub_menus[i];

        ui_box_begin(context, &sub_menu_box, sizing);
        // TODO: This sub_menu_box positioning is a little awkward...
        sub_menu_box.position = menu_button->position;
        sub_menu_box.position.y += menu_button->size.y;
        {
          for (U32 j = 0; j < sub_menu_counts[i]; ++j) {
            Process *sub_menu_button = sub_menu[j];
            if (do_ui_element(context, sub_menu_button, sizing)) {
              if (sub_menu_button->func) {
                sub_menu_button->func(context, sub_menu_button);
              }
            }
          }
        }
        ui_box_end(context, &sub_menu_box, sizing);
      }
    }
  }
  ui_box_end(context, &top_menu_box, sizing);

  // update active-menu-element
  if (clicked_active_menu_button > -1) {
    if (Top_Menu_Index(context->menu_state) == clicked_active_menu_button) {
      set_menu_state(context, 0);
    } else {
      set_menu_state(context, Menu_State_From_Top_Menu_Index(clicked_active_menu_button));
    }
  } else if (hot_active_menu_button > -1) {
    set_menu_state(context, Menu_State_From_Top_Menu_Index(hot_active_menu_button));
  }
}



function void handle_ui(Context *context) {
  reset_ui_box(context, &sub_menu_box);

  switch(context->menu_state) {
  case Menu_State_OpenFile: {
    do_open_file(context, 1);
    do_open_file(context, 0);
  } break;
  case Menu_State_SaveFileAs: {
    do_save_file_as(context, 1);
    do_save_file_as(context, 0);
  } break;
  }

  // TODO: Having the switch above, and then calling do_menu_ui... just feels off. Like maybe it should all be unified.
  // NOTE: @Speed We have to call UI code twice... onces for sizing and once for rendering/interaction.
  do_menu_ui(context, 1);
  do_menu_ui(context, 0);
}



function void handle_process_interaction(Context *context) {
  Ui_State *ui_state = &context->ui_state;
  Process_Selection selection = (Process_Selection){0};

  B32 should_stop_dragging = check_keybind(context, Ui_Feature_SelectSingleProcess, selection) == Keybind_Result_Exit;
  Process *moved_wire = 0;
  Process_Connection moved_wire_conn = 0;

  // initial bounding handling
  if (Get_Flag(context->flags, Context_Flag_Bounding)) {
    if (check_keybind(context, Ui_Feature_Bound, selection) == Keybind_Result_Exit) {
      Unset_Flag(context->flags, Context_Flag_Bounding);
    } else {
      clear_active_processes(context);
    }
  }

  // panning
  {
    if (check_keybind(context, Ui_Feature_Pan, selection) == Keybind_Result_Enter) {
      Set_Flag(context->flags, Context_Flag_Panning);
      context->active_position = context->ui_state.mouse_position;
    }

    if (Get_Flag(context->flags, Context_Flag_Panning)) {
      if (check_keybind(context, Ui_Feature_Pan, selection) == Keybind_Result_Exit) {
        Unset_Flag(context->flags, Context_Flag_Panning);
      } else {
        // Update camera position
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -1.0f/context->camera.zoom);
        context->camera.target = Vector2Add(context->camera.target, delta);
      }
    }
  }

  // zooming
  {
    B32 zoom_in = check_keybind(context, Ui_Feature_ZoomIn, selection) == Keybind_Result_Enter;
    B32 zoom_out = check_keybind(context, Ui_Feature_ZoomOut, selection) == Keybind_Result_Enter;

    if (zoom_in || zoom_out) {
      Keybind keybind_in = global_keybind_lookup[Ui_Feature_ZoomIn];
      Keybind keybind_out = global_keybind_lookup[Ui_Feature_ZoomOut];
      B32 in_wheel = (keybind_in.key_kind == Key_Kind_MouseWheelUp ||
                      keybind_in.key_kind == Key_Kind_MouseWheelDown);
      B32 out_wheel = (keybind_out.key_kind == Key_Kind_MouseWheelUp ||
                       keybind_out.key_kind == Key_Kind_MouseWheelDown);
      Vector2 mouse_world_position = GetScreenToWorld2D(context->ui_state.mouse_position, context->camera);
      context->camera.offset = context->ui_state.mouse_position;
      context->camera.target = mouse_world_position;
      F32 zoom_delta;

      // HACK: is it hacky that when check the mouse wheel and handle it differently like this? maybe not.
      if ((zoom_in && in_wheel) || (zoom_out && out_wheel)) {
        zoom_delta = -0.1f * ui_state->mouse_wheel_movement.y;
      } else if (zoom_in) {
        zoom_delta = 0.2f;
      } else {
        zoom_delta = -0.2f;
      }

      context->camera.zoom += zoom_delta;
      context->camera.zoom = Max(0.1f, context->camera.zoom);
    }
  }

  // process interaction
  for (Process *p = context->processes.first; p != 0; p = p->next) {
    selection = get_process_selection(context, p);
    Assign_Flag(ui_state->flags, Ui_State_Flag_hot_id_assigned, selection.hot_id_assigned || Get_Flag(ui_state->flags, Ui_State_Flag_hot_id_assigned));
    B32 is_active = is_active_process(context, p);

    // check if we need to stop dragging wire
    if (should_stop_dragging) {
      // unset drag flag
      B32 wire_drag_flag = Process_Flag_Drag_In | Process_Flag_Drag_Out;
      if (Get_Flag(p->flags, wire_drag_flag)) {
        B32 is_in = Get_Flag(p->flags, Process_Flag_Drag_In);
        Unset_Flag(p->flags, wire_drag_flag);
        moved_wire = p;
        moved_wire_conn = is_in ? Process_Connection_In : Process_Connection_Out;
      }
    }

    if (check_keybind(context, Ui_Feature_SelectSingleProcess, selection) == Keybind_Result_Enter) {
      B32 in_selection = selection.type == Process_Selection_In;
      B32 out_selection = selection.type == Process_Selection_Out;
      if (in_selection || out_selection) {
        // select wire
        Process *wire = get_process_wire_by_selection(context, selection);
        B32 is_active_wire = is_active_process(context, wire);

        if (wire) {
          U32 drag_flag = in_selection ? Process_Flag_Drag_In : Process_Flag_Drag_Out;
          Unset_Flag(context->flags, Context_Flag_NewWire);
          Set_Flag(wire->flags, drag_flag);
          context->active_position = context->ui_state.mouse_position;
          if (!is_active_wire) {
            clear_active_processes(context);
            SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, wire, next_active, 0);
          }
        }
      } else if ((is_active || context->hot_process == p) &&
                 selection.type == Process_Selection_NewWire) {
        // begin new-wire
        Set_Flag(context->flags, Context_Flag_NewWire);
        if (!is_active) {
          clear_active_processes(context);
          SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, p, next_active, 0);
        }
      } else if (selection.type == Process_Selection_Process) {
        if (Get_Flag(context->flags, Context_Flag_NewWire)) {
          // connect processes
          connect_processes(context, context->active_processes.first, p);
        } else {
          // select process
          context->hot_process = p;
          if (!is_active) {
            clear_active_processes(context);
            SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, p, next_active, 0);
          }
          Unset_Flag(context->flags, Context_Flag_NewWire);
          Set_Flag(context->flags, Context_Flag_Dragging);
          context->active_position = context->ui_state.mouse_position;
        }
      }
    } else if (check_keybind(context, Ui_Feature_SelectAnotherProcess, selection) == Keybind_Result_Enter) {
      // select another process
      if (selection.type == Process_Selection_In || selection.type == Process_Selection_Out) {
        Process *wire = get_process_wire_by_selection(context, selection);
        if (wire) {
          if (is_active_process(context, wire)) {
            remove_process_from_active_processes(context, wire);
          } else {
            SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, wire, next_active, 0);
          }
        }
      } else if (selection.type == Process_Selection_Process) {
        if (is_active_process(context, selection.process)) {
          remove_process_from_active_processes(context, selection.process);
        } else {
          SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, selection.process, next_active, 0);
        }
      }
    } else if (selection.type == Process_Selection_Process) {
      // process hover
      context->hot_process = p;
    }

    // bounding
    if (Get_Flag(context->flags, Context_Flag_Bounding)) {
      Rectangle selection_rectangle = get_selection_rectangle(context);

      if (Get_Flag(p->flags, Process_Flag_Wire)) {
        Process_Shape out_shape = get_process_shape(context, p->out);
        Process_Shape in_shape = get_process_shape(context, p->in);
        Vector2 out_position = get_process_wire_position(context, p->out, out_shape, Process_Connection_Out, p->which_out);
        Vector2 in_position = get_process_wire_position(context, p->in, in_shape, Process_Connection_In, p->which_in);

        if (rectangle_contains_point(selection_rectangle, out_position) ||
            rectangle_contains_point(selection_rectangle, in_position)) {
          SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, p, next_active, 0);
        }
      } else {
        Process_Shape shape = get_process_shape(context, p);

        if (rectangle_contains_point(selection_rectangle, shape.center)) {
          SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, p, next_active, 0);
        }
      }
    }
  }

  // zero out selection
  selection = (Process_Selection){0};
  // zero the old hot-id
  if (!Get_Flag(ui_state->flags, Ui_State_Flag_hot_id_assigned)) {
    context->hot_process = 0;
  }

  // more rectangle selection handling
  if (Get_Flag(context->flags, Context_Flag_Bounding)) {
    // add hot process to active processes
    if (context->hot_process) {
      B32 hot_is_active = is_active_process(context, context->hot_process);
      if (!hot_is_active) {
        SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, context->hot_process, next_active, 0);
      }
    }
  }

  if (IsMouseButtonPressed(0)) {
    B32 hmmm = 0;
  }
  // create process
  if (check_keybind(context, Ui_Feature_CreateProcess, selection)) {
    Process *new_p = create_process(context);
    if (new_p) {
      Set_Flag(new_p->flags, Process_Flag_TextEdit);
      new_p->position = GetScreenToWorld2D(context->ui_state.mouse_position, context->camera);
      clear_active_processes(context);
      SLLQueuePush_NZ(context->active_processes.first, context->active_processes.last, new_p, next_active, 0);
    }
  }

  // cancel selection
  if (check_keybind(context, Ui_Feature_CancelSelection, selection)) {
    clear_active_processes(context);
    Unset_Flag(context->flags, Context_Flag_NewWire);
  }

  // enter bounding
  if (check_keybind(context, Ui_Feature_Bound, selection) == Keybind_Result_Enter) {
    Set_Flag(context->flags, Context_Flag_Bounding);
    context->active_position = context->ui_state.mouse_position;
  }

  // toggle between rounded and triangular shapes
  if (check_keybind(context, Ui_Feature_ToggleDisplayMode, selection) == Keybind_Result_Enter) {
    Toggle_Flag(context->flags, Context_Flag_RoundedShapes);
  }

  // copy processes
  if (check_keybind(context, Ui_Feature_CopyProcess, selection) == Keybind_Result_Enter) {
    copy_active_processes(context);
  }
  // paste processes
  if (check_keybind(context, Ui_Feature_PasteProcess, selection) == Keybind_Result_Enter) {
    paste_processes(context);
  }

  // handle moved wire
  if (moved_wire && context->hot_process) {
    if (Get_Flag(context->hot_process->flags, Process_Flag_Wire)) {
      Process *connected_process = context->hot_process->conn[moved_wire_conn];
      if (connected_process) {
        // move wire to hovered wire
        U32 which_conn = context->hot_process->which_conn[moved_wire_conn];
        if (moved_wire != context->hot_process) {
          remove_wire_connection(context, moved_wire, (1<<moved_wire_conn));
          add_wire_connection(context, moved_wire, connected_process, moved_wire_conn, which_conn);
        }
      }
    } else {
      Process *connected_process = context->hot_process;
      // move wire to last wire of process
      U32 which_conn;
      if (moved_wire->conn[moved_wire_conn] == connected_process) {
        which_conn = connected_process->conn_count[moved_wire_conn] - 1;
      } else {
        which_conn = connected_process->conn_count[moved_wire_conn];
      }
      remove_wire_connection(context, moved_wire, (1<<moved_wire_conn));
      add_wire_connection(context, moved_wire, connected_process, moved_wire_conn, which_conn);
    }
  }

  // handle active-process
  if (context->active_processes.first) {
    B32 is_dragging = Get_Flag(context->flags, Context_Flag_Dragging);
    if (is_dragging && should_stop_dragging) {
      // update positions of active processes
      for (Process *a = context->active_processes.first; a != 0; a = a->next_active) {
        Vector2 new_position = get_process_position(context, a);
        a->position = new_position;
      }
      // stop dragging
      Unset_Flag(context->flags, Context_Flag_Dragging);
    } else if (check_keybind(context, Ui_Feature_CycleProcessDisplay, selection)) {
      // cycle through special process types (cups/caps/empty)
      for (Process *a = context->active_processes.first; a != 0; a = a->next_active) {
        if (!Get_Flag(a->flags, Process_Flag_Wire)) {
          U32 toggle_flags = (Process_Flag_Empty | Process_Flag_Cup | Process_Flag_Cap | Process_Flag_Identity);
          if (Get_Flag(a->flags, toggle_flags)) {
            // toggle off process-display flags first, before trying to toggle them on
            Unset_Flag(a->flags, toggle_flags);
          } else if ((a->in_count == 0 && a->out_count == 0) ||
                     (a->in_count == 1 && a->out_count == 0) ||
                     (a->in_count == 0 && a->out_count == 1)) {
            // toggle single in/out or unconnected process
            Toggle_Flag(a->flags, Process_Flag_Empty);
          } else if (a->in_count == 0 && a->out_count == 2) {
            Toggle_Flag(a->flags, Process_Flag_Cup);
          } else if (a->in_count == 2 && a->out_count == 0) {
            Toggle_Flag(a->flags, Process_Flag_Cap);
          } else if (a->in_count == 1 && a->out_count == 1) {
            Toggle_Flag(a->flags, Process_Flag_Identity);
          }
        }
      }
    } else if (check_keybind(context, Ui_Feature_DeleteProcess, selection)) {
      // delete processes
      for (Process *a = context->active_processes.first; a != 0;) {
        Process *next_active = a->next_active;
        delete_process(context, a);
        a = next_active;
      }
      clear_active_processes(context);
    } else if (!Get_Flag(ui_state->flags, Ui_State_Flag_action_occured)) {
      // process label editing
      handle_label_editing(context, context->active_processes);
    }
  }
}



function void handle_user_input(Context *context) {
  context->ui_state = get_ui_state(context);

  // get key presses
  for (U32 k = 0; k < Max_Key_Presses_Per_Frame; ++k) {
    U32 key = GetKeyPressed();
    context->ui_state.key_presses[k] = key;

    if (key == 0) {
      break;
    }
  }

  handle_ui(context);
  if (!Get_Flag(context->ui_state.flags, Ui_State_Flag_action_occured)) {
    handle_process_interaction(context);
  }
}


function void draw_circular_process(Context *context, Vector2 center, F32 radius, F32 thickness, Color bg_color, Color stroke_color) {
  Render_Context *rc = &context->process_render_context;

  render_DrawCircle(rc, center, radius, bg_color);
  F32 fudge = Half_Circle_Fudge*radius;
  Vector2 first_point = (Vector2){center.x-radius, center.y};
  Vector2 second_point = (Vector2){center.x+radius, center.y};
  Vector2 control0 = (Vector2){first_point.x, first_point.y-fudge};
  Vector2 control1 = (Vector2){second_point.x, second_point.y-fudge};
  render_DrawLineBezierCubic(rc, first_point, second_point, control0, control1, thickness, stroke_color);
  Vector2 control2 = (Vector2){first_point.x, first_point.y+fudge};
  Vector2 control3 = (Vector2){second_point.x, second_point.y+fudge};
  render_DrawLineBezierCubic(rc, first_point, second_point, control2, control3, thickness, stroke_color);
}




function void draw_process_with_triangle_fan(Context *context, Process_Shape shape, F32 thickness, Color bg_color, Color stroke_color) {
  Assert(shape.triangle_count == (shape.point_count - 2));
  Render_Context *rc = &context->process_render_context;

  // draw background
  render_DrawTriangleFan(rc, shape.points, shape.point_count, bg_color);

  // draw lines
  for (S32 i = 0; i < shape.point_count-1 && i < Process_Shape_Max_Points; ++i) {
    Vector2 p0 = shape.points[i];
    Vector2 p1 = shape.points[i+1];
    render_DrawLine(rc, p0.x, p0.y, p1.x, p1.y, thickness, stroke_color);
  }

  // draw line from last point to first point
  Vector2 p0 = shape.points[0];
  Vector2 p1 = shape.points[shape.point_count-1];
  render_DrawLine(rc, p0.x, p0.y, p1.x, p1.y, thickness, stroke_color);
}



function void draw_process_with_triangle_strip(Context *context, Process_Shape shape, F32 thickness, Color bg_color, Color stroke_color) {
  Render_Context *rc = &context->process_render_context;
  // draw process background
  render_DrawTriangleStrip(rc, shape.points, shape.point_count, bg_color);

  if (shape.triangle_count) {
    // draw first two lines
    Vector2 p0 = shape.points[0];
    Vector2 p1 = shape.points[1];
    Vector2 p2 = shape.points[2];
    render_DrawLine(rc, p0.x, p0.y, p1.x, p1.y, thickness, stroke_color);
    render_DrawLine(rc, p0.x, p0.y, p2.x, p2.y, thickness, stroke_color);

    // draw in-between lines
    for (S32 i = 1; i < shape.triangle_count; ++i) {
      Vector2 p0 = shape.points[i];
      Vector2 p1 = shape.points[i+2];
      render_DrawLine(rc, p0.x, p0.y, p1.x, p1.y, thickness, stroke_color);
    }

    // draw line connecting last two points
    p0 = shape.points[shape.point_count-1];
    p1 = shape.points[shape.point_count-2];
    render_DrawLine(rc, p0.x, p0.y, p1.x, p1.y, thickness, stroke_color);
  }
}


function void draw_processes(Context *context) {
  Render_Context *rc = &context->process_render_context;

  Color bg_color = (Color){255, 255, 255, 255};
  Color invisible_bg_color = (Color){0, 0, 0, 0};
  Color stroke_color = (Color){0, 0, 0, 255};
  Color invisible_stroke_color = (Color){0, 0, 0, 100};
  Color text_color = (Color){0, 0, 0, 255};
  Color box_color = (Color){10, 190, 40, 255};
  Color box_hover_color = (Color){5, 250, 20, 255};

  F32 font_size = context->camera.zoom * global_process_font_size;

  F32 padding = global_process_wire_padding;
  F32 spacing = global_process_wire_spacing;
  B32 rounded = Get_Flag(context->flags, Context_Flag_RoundedShapes);

  // draw processes
  for (Process *p = context->processes.first; p != 0; p = p->next) {
    B32 is_wire = Get_Flag(p->flags, Process_Flag_Wire);
    U8 *label_c_string = c_string_from_string_chunk_list(context->temp_arena, &p->label);
    S32 text_width = MeasureText((char *)label_c_string, font_size);

    if (!is_wire) {
      Process_Shape shape = get_process_shape(context, p);

      B32 is_hot = context->hot_process == p;
      B32 is_active = is_active_process(context, p);
      F32 thickness = (is_hot||is_active) ? global_active_line_thickness : global_line_thickness;
      thickness *= context->camera.zoom;
      F32 cup_cap_control_offset = 10.0f;

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
            Vector2 position = get_process_position(context, p);
            position = GetWorldToScreen2D(position, context->camera);
            Half_Circle_Points points = get_half_circle_points(context, shape, p, position, text_width, downward);
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
          render_DrawLineBezierCubic(rc, p0, p1, p1, p0, thickness, stroke_color);
        } else if (!label_c_string[0]) {
          if (rounded) {
            draw_circular_process(context, shape.center, shape.radius, thickness, invisible_bg_color, invisible_stroke_color);
          } else {
            draw_process_with_triangle_strip(context, shape, thickness, invisible_bg_color, invisible_stroke_color);
          }
        }
      } else if (Get_Flag(p->flags, Process_Flag_Cup)) {
        // draw cup
        Vector2 pos0 = get_process_wire_position(context, p, shape, Process_Connection_Out, 0);
        Vector2 pos1 = get_process_wire_position(context, p, shape, Process_Connection_Out, 1);
        Vector2 ctrl0 = (Vector2){pos0.x, pos0.y+cup_cap_control_offset};
        Vector2 ctrl1 = (Vector2){pos1.x, pos1.y+cup_cap_control_offset};
        render_DrawLineBezierCubic(rc, pos0, pos1, ctrl0, ctrl1, thickness, stroke_color);
      } else if (Get_Flag(p->flags, Process_Flag_Cap)) {
        // draw cap
        Vector2 pos0 = get_process_wire_position(context, p, shape, Process_Connection_In, 0);
        Vector2 pos1 = get_process_wire_position(context, p, shape, Process_Connection_In, 1);
        Vector2 ctrl0 = (Vector2){pos0.x, pos0.y-cup_cap_control_offset};
        Vector2 ctrl1 = (Vector2){pos1.x, pos1.y-cup_cap_control_offset};
        render_DrawLineBezierCubic(rc, pos0, pos1, ctrl0, ctrl1, thickness, stroke_color);
      } else if (Get_Flag(p->flags, Process_Flag_Identity)) {
        // draw "identity" process (just a wire)
        Vector2 pos0 = get_process_wire_position(context, p, shape, Process_Connection_In, 0);
        Vector2 pos1 = get_process_wire_position(context, p, shape, Process_Connection_Out, 0);
        render_DrawLineBezierCubic(rc, pos0, pos1, pos1, pos0, thickness, stroke_color);
      } else {
        switch(shape.kind) {
        case Process_Shape_TriangleStrip: {
          draw_process_with_triangle_strip(context, shape, thickness, bg_color, stroke_color);
        } break;
        case Process_Shape_TriangleFan: {
          draw_process_with_triangle_fan(context, shape, thickness, bg_color, stroke_color);
        } break;
        case Process_Shape_Circle: {
          draw_circular_process(context, shape.center, shape.radius, thickness, bg_color, stroke_color);
        } break;
        case Process_Shape_HalfCircle: {
          // draw half-circle background
          render_DrawTriangleFan(rc, shape.points, shape.point_count, bg_color);
          // draw half-circle lines
          for (S32 i = 0; i < shape.point_count-1; ++i) {
            Vector2 p0 = shape.points[i];
            Vector2 p1 = shape.points[i+1];
            render_DrawLine(rc, p0.x, p0.y, p1.x, p1.y, thickness, stroke_color);
          }
          // connect the line endpoints
          render_DrawLine(rc,
                          shape.points[0].x,
                          shape.points[0].y,
                          shape.points[shape.point_count-1].x,
                          shape.points[shape.point_count-1].y,
                          thickness, stroke_color);
        } break;
        default: Assert(0);
        }
      }

      // draw label
      if (label_c_string[0]) {
        F32 text_x = shape.center.x-0.5f*text_width;
        F32 text_y = shape.center.y-0.5f*font_size;
        if (shape.kind == Process_Shape_HalfCircle) {
          F32 flip = shape.downward ? -1.0f : 1.0f;
          F32 fudge = 0.9f;
          F32 offset = fudge * flip * (0.5f * shape.radius);
          text_y -= offset;
        }
        render_DrawText(rc, (char *)label_c_string, text_x, text_y, font_size, text_color, 0);
      }

      // draw new-wire-box
      if (is_active || is_hot) {
        Rectangle new_wire_box = get_new_wire_box(context, p, shape);
        B32 new_wire_box_is_active = (
          (is_active && Get_Flag(context->flags, Context_Flag_NewWire)) ||
          rectangle_contains_point(new_wire_box, context->ui_state.mouse_position));
        Color color = new_wire_box_is_active ? box_hover_color : box_color;
        render_DrawRectangleRec(rc, new_wire_box, color);
      }
    }
  }

  // draw wires
  for (Process *p = context->processes.first; p != 0; p = p->next) {
    B32 is_wire = Get_Flag(p->flags, Process_Flag_Wire);

    if (is_wire) {
      Process_Shape out_shape = get_process_shape(context, p->out);
      Process_Shape in_shape = get_process_shape(context, p->in);

      Vector2 out_position = get_process_wire_position(context, p->out, out_shape, Process_Connection_Out, p->which_out);
      Vector2 in_position = get_process_wire_position(context, p->in, in_shape, Process_Connection_In, p->which_in);
      if (Get_Flag(p->flags, Process_Flag_Drag_In)) {
        Vector2 delta = Vector2Subtract(context->ui_state.mouse_position, context->active_position);
        in_position = Vector2Add(in_position, delta);
      } else if (Get_Flag(p->flags, Process_Flag_Drag_Out)) {
        Vector2 delta = Vector2Subtract(context->ui_state.mouse_position, context->active_position);
        out_position = Vector2Add(out_position, delta);
      }

      Vector2 out_control = out_position;
      out_control.y -= context->camera.zoom * 30.0f;
      Vector2 in_control = in_position;
      in_control.y += context->camera.zoom * 30.0f;

      B32 is_active = is_active_process(context, p) || context->hot_process == p;
      B32 connected_in_active = (is_active_process(context, p->in) ||
                                 context->hot_process == p->in);
      B32 connected_out_active = (is_active_process(context, p->out) ||
                                  context->hot_process == p->out);
      F32 thickness = is_active ? global_active_line_thickness : global_line_thickness;
      thickness *= context->camera.zoom;

      // draw wire
      render_DrawLineBezierCubic(rc, out_position, in_position, out_control, in_control, thickness, stroke_color);

      // draw out wire-box
      if (connected_out_active || is_active) {
        Rectangle box = get_wire_box(context, out_position);
        Color c = is_active ? box_hover_color : box_color;
        render_DrawRectangleRec(rc, box, c);
      }

      // draw in wire-box
      if (connected_in_active || is_active) {
        Rectangle box = get_wire_box(context, in_position);
        Color c = is_active ? box_hover_color : box_color;
        render_DrawRectangleRec(rc, box, c);
      }
    }
  }

  // draw new wire
  if (Get_Flag(context->flags, Context_Flag_NewWire) && context->active_processes.first) {
    Process_Shape shape = get_process_shape(context, context->active_processes.first);
    Vector2 position = shape.new_wire_position;

    Vector2 from_control = position;
    from_control.y -= context->camera.zoom * 30.f;
    Vector2 to_control = context->ui_state.mouse_position;
    to_control.y += context->camera.zoom * 30.0f;

    F32 thickness = context->camera.zoom * global_line_thickness;

    render_DrawLineBezierCubic(rc, position, context->ui_state.mouse_position, from_control, to_control, thickness, stroke_color);
  }

  // draw selection rectangle
  if (Get_Flag(context->flags, Context_Flag_Bounding)) {
    Rectangle selection_rect = get_selection_rectangle(context);
    Color selection_color = (Color){10, 30, 200, 50};

    render_DrawRectangleRec(rc, selection_rect, selection_color);
  }
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

function void draw_info_panel(Context *context) {
  Render_Context *rc = &context->ui_render_context;
  Color text_color = (Color){0, 0, 0, 255};
  F32 x = 5.0f;
  F32 y = 5.0f;
  F32 padding = 2.0f;

#if 0
  if (context->active_processes.first) {
    for (Process *a = context->active_processes.first; a != 0; a = a->next_active) {
      char *format = a == context->active_processes.first ? "active-id = %p" : "            %p";
      const char *text = TextFormat(format, a);
      render_DrawText(rc, text, 5.0f, y, global_panel_font_size, text_color, 1);
      y += global_panel_font_size + padding;
    }
  }
#elif 0
  render_DrawText(rc, TextFormat("process count %d\n", debug_process_list_count(context->processes)),
                  x, y, global_panel_font_size, text_color, 1);
  y += global_panel_font_size + padding;
  render_DrawText(rc, TextFormat("active count %d\n", debug_process_active_list_count(context->active_processes)),
                  x, y, global_panel_font_size, text_color, 1);
  y += global_panel_font_size + padding;
  render_DrawText(rc, TextFormat("free count %d\n", debug_process_list_count(context->free_processes)),
                  x, y, global_panel_font_size, text_color, 1);
  y += global_panel_font_size + padding;
  render_DrawText(rc, TextFormat("copy count %d\n", debug_process_list_count(context->copy_processes)),
                  x, y, global_panel_font_size, text_color, 1);
  y += global_panel_font_size + padding;
#elif 1
  S32 arena_font_size = 12;
  y = global_window_size.y - arena_font_size - padding;
  render_DrawText(rc, TextFormat("per-frame arena %llu/%llu\n", context->per_frame_arena->chunk_pos, context->per_frame_arena->chunk_cap), x, y, arena_font_size, text_color, 1);
  y -= arena_font_size + padding;
  render_DrawText(rc, TextFormat("ui arena %llu/%llu\n", context->ui_arena->chunk_pos, context->ui_arena->chunk_cap), x, y, arena_font_size, text_color, 1);
  y -= arena_font_size + padding;
  render_DrawText(rc, TextFormat("temp arena %llu/%llu\n", context->temp_arena->chunk_pos, context->temp_arena->chunk_cap), x, y, arena_font_size, text_color, 1);
  y -= arena_font_size + padding;
  render_DrawText(rc, TextFormat("render arena %llu/%llu\n", context->render_arena->chunk_pos, context->render_arena->chunk_cap), x, y, arena_font_size, text_color, 1);
  y -= arena_font_size + padding;
  render_DrawText(rc, TextFormat("permanent arena %llu/%llu\n", context->permanent_arena->chunk_pos, context->permanent_arena->chunk_cap), x, y, arena_font_size, text_color, 1);
  y -= arena_font_size + padding;
#endif
}





function Context initialize_context(void) {
  Context context = (Context){0};

  context.render_arena = arena_alloc_reserve(Megabytes(1), 0);
  context.permanent_arena = arena_alloc_reserve(Megabytes(1), 0);
  context.temp_arena = arena_alloc_reserve(Megabytes(1), 0);
  context.ui_arena = arena_alloc_reserve(Megabytes(1), 0);
  context.per_frame_arena = arena_alloc_reserve(Megabytes(1), 0);
  context.test_arena = arena_alloc_reserve(Megabytes(1), 0);

  context.ui_render_context.arena = context.render_arena;
  context.process_render_context.arena = context.render_arena;

  context.camera.zoom = 1.0f;

  return context;
}







function void initialize_globals(Context *context) {
  S32 monitor_id = GetCurrentMonitor();
  S32 screen_width = GetMonitorWidth(monitor_id);
  S32 screen_height = GetMonitorHeight(monitor_id);

  global_background_color = (Color){220, 220, 200, 255};

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

  global_button_padding = (Vector2){12.0f, 5.0f};
  global_button_dormant_bg_color = (Color){90, 70, 90, 255};
  global_button_hot_bg_color = (Color){100, 80, 100, 255};
  global_button_font_color = (Color){220, 220, 160, 255};
  global_container_bg_color = (Color){170, 170, 170, 255};


  // init ui elements
  // TODO: Turn these into struct literal declarations if we can.
  file_menu_button = create_lit_button(context, str8_lit("File"), 0, 0);
  open_file_button = create_lit_button(context, str8_lit("Open..."), 0, 0);
  open_file_button.func = set_menu_state_as_open_file;
  save_file_button = create_lit_button(context, str8_lit("Save"), 0, 0);
  save_file_button.func = save_file;
  save_as_file_button = create_lit_button(context, str8_lit("Save As..."), 0, 0);
  save_as_file_button.func = set_menu_state_as_save_file_as;
  edit_menu_button = create_lit_button(context, str8_lit("Edit"), 0, 0);
  copy_button = create_lit_button(context, str8_lit("Copy"), 0, 0);
  copy_button.func = handle_copy;
  paste_button = create_lit_button(context, str8_lit("Paste"), 0, 0);
  paste_button.func = handle_paste;
  Assert(ArrayCount(menu_buttons) == ArrayCount(sub_menus) &&
         ArrayCount(sub_menus) == ArrayCount(sub_menu_counts));

  // init common filepaths
#if OS_WINDOWS
# define _ "\\"
#else
# define _ "/"
#endif
  Keybind_Config_Filepath = str8_comptime_lit(".."_"config"_"keybind.txt");
  Saves_Filepath = str8_comptime_lit(".."_"saves"_);
  Build_Filepath = str8_comptime_lit(".."_"build"_);
#undef _

  // ensure saves directory exists
  os_file_make_directory(Saves_Filepath);



  load_keybinds(context);
}







function void set_global_window_render_size(void) {
  Vector2 dpi_scale = GetWindowScaleDPI();
  global_window_size.x = (F32)GetRenderWidth() / dpi_scale.x;
  global_window_size.y = (F32)GetRenderHeight() / dpi_scale.x;
}


int main(void) {
  InitWindow(800, 500, "proc");
  SetExitKey(0);
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);

  Context context = initialize_context();
  initialize_globals(&context);
  SetWindowSize(global_window_size.x, global_window_size.y);

  Render_Context *prc = &context.process_render_context;
  render_Initialize(context.temp_arena);
  set_global_window_render_size();

  while (!WindowShouldClose()) {
    if (IsWindowResized()) {
      set_global_window_render_size();
    }

    handle_user_input(&context);

    render_ClearBackground(prc, global_background_color);
    draw_processes(&context);
#if 1
    draw_info_panel(&context);
#endif

    BeginDrawing();
    render_Commands(&context.process_render_context);
    render_Commands(&context.ui_render_context);

    // clear out per-frame stuff
    arena_pop_to(context.render_arena, 0);
    arena_pop_to(context.temp_arena, 0);
    arena_pop_to(context.per_frame_arena, 0);
    context.ui_render_context.command_list.first = 0;
    context.ui_render_context.command_list.last = 0;
    context.process_render_context.command_list.first = 0;
    context.process_render_context.command_list.last = 0;
    /* context.per_frame_ui_elements.first = 0; */
    /* context.per_frame_ui_elements.last = 0; */

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
