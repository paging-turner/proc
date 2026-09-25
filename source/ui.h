#define Global_Button_Padding (Vector2){12.0f, 5.0f}
global_variable Vector2 global_button_padding = Global_Button_Padding;

#define Global_Panel_Font_Size 14.0f
global_variable F32 global_panel_font_size = Global_Panel_Font_Size;

global_variable Color global_button_dormant_bg_color = (Color){90, 70, 90, 255};
global_variable Color global_button_hot_bg_color = (Color){100, 80, 100, 255};
global_variable Color global_button_font_color = (Color){220, 220, 160, 255};















////////////////////////
// UI Globals
////////////////////////

// TODO: Symbol-sets?
#define Global_Ui_Proc_Kind_Xlist(X)\
  X(root)\
  X(top_menu_box)\
  X(sub_menu_box)\
  X(open_file_box)\
  X(open_file_confirm_box)\
  X(save_file_as_box)\
  X(file_menu_button)\
  X(open_file_button)\
  X(save_file_button)\
  X(save_as_file_button)\
  X(edit_menu_button)\
  X(copy_button)\
  X(paste_button)\
  X(open_file_label)\
  X(open_button)\
  X(cancel_button)\
  X(save_button)\
  X(save_file_as_text_input)

typedef enum Global_Ui_Proc_Id {
#define X(name, ...)\
  Global_Ui_Proc_Id_##name,
  Global_Ui_Proc_Kind_Xlist(X)
#undef X
} Global_Ui_Proc_Id;

global_variable Process global_ui_procs[] = {
  [Global_Ui_Proc_Id_file_menu_button] = (Process){
    .flags = (Process_Flag_Clickable |
              Process_Flag_FitToText |
              Process_Flag_UiDescendIfActive |
              Process_Flag_CanBeActive),
    .ui_box = {
      .layout = Ui_Layout_Vertical,
    },
    .label_c_string = (U8 *)"File"
  },
  [Global_Ui_Proc_Id_open_file_button] = (Process){
    .flags = (Process_Flag_Clickable | Process_Flag_FitToText),
    .label_c_string = (U8 *)"Open..."
  },
  [Global_Ui_Proc_Id_save_file_button] = (Process){
    .flags = (Process_Flag_Clickable | Process_Flag_FitToText),
    .label_c_string = (U8 *)"Save"
  },
  [Global_Ui_Proc_Id_save_as_file_button] = (Process){
    .flags = (Process_Flag_Clickable | Process_Flag_FitToText),
    .label_c_string = (U8 *)"Save As...",
    .func = set_save_file_as_as_active_element
  },
  [Global_Ui_Proc_Id_edit_menu_button] = (Process){
    .flags = (Process_Flag_Clickable |
              Process_Flag_FitToText |
              Process_Flag_UiDescendIfActive |
              Process_Flag_CanBeActive),
    .ui_box = {
      .layout = Ui_Layout_Vertical,
    },
    .label_c_string = (U8 *)"Edit",
  },
  [Global_Ui_Proc_Id_copy_button] = (Process){
    .flags = (Process_Flag_Clickable | Process_Flag_FitToText),
    .label_c_string = (U8 *)"Copy",
    .func = handle_copy,
  },
  [Global_Ui_Proc_Id_paste_button] = (Process){
    .flags = (Process_Flag_Clickable | Process_Flag_FitToText),
    .label_c_string = (U8 *)"Paste",
    .func = handle_paste
  },
  [Global_Ui_Proc_Id_open_file_label] = (Process){
    .flags = (Process_Flag_Clickable |
              Process_Flag_FitToText |
              Process_Flag_UiDescendIfActive |
              Process_Flag_CanBeActive),
    .label_c_string = (U8 *)"Open File...",
    .margin = (Vector2){5.0f, 8.0f},
  },
  [Global_Ui_Proc_Id_open_button] = (Process){
    .flags = Process_Flag_FitToText|Process_Flag_Clickable,
    .label_c_string = (U8 *)"Open",
    .margin = (Vector2){5.0f, 8.0f},
  },
  [Global_Ui_Proc_Id_cancel_button] = (Process){
    .flags = Process_Flag_FitToText|Process_Flag_Clickable,
    .label_c_string = (U8 *)"Cancel",
    .margin = (Vector2){5.0f, 8.0f},
  },
  [Global_Ui_Proc_Id_save_button] = (Process){
    .flags = Process_Flag_FitToText|Process_Flag_Clickable,
    .label_c_string = (U8 *)"Save",
    .margin = (Vector2){5.0f, 8.0f},
  },
  [Global_Ui_Proc_Id_save_file_as_text_input] = (Process){
    .flags = (Process_Flag_TextEdit|
              Process_Flag_Clickable|
              Process_Flag_CanBeActive),
    .ui_box = {
      .min_size = (Vector2){200.0f, Global_Panel_Font_Size+2.0f*Global_Button_Padding.y}
    }
  },
  [Global_Ui_Proc_Id_top_menu_box] = (Process){
    .flags = Process_Flag_IsBox,
    .ui_box = {
      .flags = Ui_Box_Flag_OnlyOneActive,
      .align = Ui_Align_TopLeft,
      .layout = Ui_Layout_Horizontal,
      .sizing = Ui_Sizing_FitContents,
    }
  },
  [Global_Ui_Proc_Id_sub_menu_box] = (Process){
    .flags = Process_Flag_IsBox,
    .ui_box = {
      .flags = Ui_Box_Flag_OnlyOneActive,
      .align = Ui_Align_TopLeft,
      .layout = Ui_Layout_Vertical,
      .sizing = Ui_Sizing_FitContentsX,
    }
  },
  [Global_Ui_Proc_Id_open_file_box] = (Process){
    .flags = (Process_Flag_IsBox|
              Process_Flag_UiDescendIfActive|
              Process_Flag_UiShowIfActive),
    .position = (Vector2){100.0f, 100.0f},
    .ui_box = {
      .kind = Ui_Box_Kind_OpenFile,
      .align = Ui_Align_TopLeft,
      .layout = Ui_Layout_Vertical,
      .sizing = Ui_Sizing_FitContents,
    }
  },
  [Global_Ui_Proc_Id_open_file_confirm_box] = (Process){
    .flags = Process_Flag_IsBox,
    .ui_box = {
      .align = Ui_Align_TopRight, // TODO: The right-alignment is broken... should fix that at some point...
      .layout = Ui_Layout_Horizontal,
      .sizing = Ui_Sizing_FitContents,
    }
  },
  [Global_Ui_Proc_Id_save_file_as_box] = (Process){
    .flags = (Process_Flag_IsBox|
              Process_Flag_UiDescendIfActive|
              Process_Flag_UiShowIfActive),
    .position = (Vector2){0.0f, 0.0f},
    .ui_box = {
      .kind = Ui_Box_Kind_SaveFileAs,
      .align = Ui_Align_TopLeft,
      .layout = Ui_Layout_Vertical,
      .sizing = Ui_Sizing_FitContents,
    }
  }
};












function Vector2 ui_get_element_size(Context *context, Process *element) {
  Vector2 size = (Vector2){0};

  if (element) {
    size = element->ui_box.size;
    F32 font_size = global_panel_font_size;
    Vector2 padding = global_button_padding;
    B32 fit_to_text = Get_Flag_Bool(element->flags, Process_Flag_FitToText);
    Vector2 min_size = element->ui_box.min_size;

    if (fit_to_text) {
      String8 text = (String8){0};

      if (element->label_c_string) {
        text = str8_lit(element->label_c_string);
      }
      else if (element->label) {
        text = piece_table_get_string(render_GlobalTempArena, element->label);
      }

      if (text.str) {
        size.x = (F32)MeasureText((char *)text.str, font_size) + 2.0f*padding.x;
        size.y = font_size + 2.0f*padding.y;
      }
    }

    if (size.x < min_size.x) {
      size.x = min_size.x;
    }
    if (size.y < min_size.y) {
      size.y = min_size.y;
    }

    size = Vector2Add(size, Vector2Scale(element->margin, 2.0f));

    // HACK: Round up because having values close to integers can cause visual "gaps" between rectangles and stuff...
    size.x = ceil_F32(size.x);
    size.y = ceil_F32(size.y);
  }

  return size;
}











function Process *ui_decl_init(
  Context *context,
  View *view,
  Process **parent_process,
  Vector2 padding,
  F32 menu_button_y_offset,
  Process *proc_to_copy,
  Process_Do_Undo_Kind do_undo_kind
  ) {
  Process *process = proc_to_copy;

  if (process && parent_process) {
    { // store old parent
      U64 gen_id = process->gen_id;
      process->parent = *parent_process;
      process->gen_id = gen_id;
    }
    process->ui_box.layout_offset = process->parent ? process->parent->ui_box.layout_offset : (Vector2){0};
    B32 was_active = Get_Flag(process->flags, Process_Flag_IsActive);

    Render_Context *rc = &context->ui_render_context;
    Ui_State *ui_state = &context->ui_state;

    B32 is_hot = 0;
    B32 in_bounds = 1;
    B32 hover_box = 1;

    F32 font_size = global_panel_font_size;
    Color dormant_bg_color = global_button_dormant_bg_color;
    Color hot_bg_color = global_button_hot_bg_color;
    Color font_color = global_button_font_color;

    {
      // set initial size
      process->ui_box.size = ui_get_element_size(context, process);

      if (process->ui_box.sizing == Ui_Sizing_FitContents || process->ui_box.sizing == Ui_Sizing_FitContentsX) {
        process->ui_box.size.x = 0.0f;
      }
      if (process->ui_box.sizing == Ui_Sizing_FitContents || process->ui_box.sizing == Ui_Sizing_FitContentsY) {
        process->ui_box.size.y = 0.0f;
      }
    }

    { // adjust parent size if fitting contents
      if (*parent_process) {
        B32 should_fit_x = ((*parent_process)->ui_box.sizing == Ui_Sizing_FitContents ||
                            (*parent_process)->ui_box.sizing == Ui_Sizing_FitContentsX);
        B32 should_fit_y = ((*parent_process)->ui_box.sizing == Ui_Sizing_FitContents ||
                            (*parent_process)->ui_box.sizing == Ui_Sizing_FitContentsY);

        B32 horizontal_layout = (*parent_process)->ui_box.layout == Ui_Layout_Horizontal;
        B32 vertical_layout = (*parent_process)->ui_box.layout == Ui_Layout_Vertical;

        if (should_fit_x) {
          if (horizontal_layout) {
            (*parent_process)->ui_box.size.x += process->ui_box.size.x;
          }
          else if (vertical_layout) {
            if ((*parent_process)->ui_box.size.x < process->ui_box.size.x) {
              (*parent_process)->ui_box.size.x = process->ui_box.size.x;
            }
          }
        }
        if (should_fit_y) {
          /* (*parent_process)->ui_box.size.y = (*parent_process)->ui_box.min_size.y; */
          if (horizontal_layout) {
            if ((*parent_process)->ui_box.size.y < process->ui_box.size.y) {
              (*parent_process)->ui_box.size.y = process->ui_box.size.y;
            }
          }
          else if (vertical_layout) {
            (*parent_process)->ui_box.size.y += process->ui_box.size.y;
          }
        }
      }
    }

    Rectangle element_rect = (Rectangle){
      process->position.x + process->ui_box.layout_offset.x + process->margin.x,
      process->position.y + process->ui_box.layout_offset.y + process->margin.y,
      process->ui_box.size.x-2.0f*process->margin.x,
      process->ui_box.size.y-2.0f*process->margin.y,
    };
    Rectangle box_rect;
    if ((*parent_process)) {
      box_rect = (Rectangle){(*parent_process)->position.x,
                             (*parent_process)->position.y,
                             (*parent_process)->ui_box.size.x,
                             (*parent_process)->ui_box.size.y};
    }
    else {
      box_rect = (Rectangle){0};
    }

    if ((*parent_process) && Get_Flag((*parent_process)->flags, Ui_Box_Flag_Clip)) {
      in_bounds = CheckCollisionRecs(element_rect, box_rect);
    }

    if (in_bounds) {
      B32 hover_element = rectangle_contains_point(element_rect, context->ui_state.mouse_position);
      if ((*parent_process)) {
        hover_box = (!Get_Flag(process->flags, Ui_Box_Flag_Clip) ||
                     rectangle_contains_point(box_rect, context->ui_state.mouse_position));
        is_hot = hover_element && hover_box;
      }

      if (Get_Flag(process->flags, Process_Flag_Clickable)) {
        { // draw clickable background
          B32 is_hot_bg_color = is_hot || process == context->selected_element;
          Color bg_color = is_hot_bg_color ? hot_bg_color : dormant_bg_color;
          if (is_hot) {
            context->hot_process.process = process;
          }

          if (element_rect.width > 0.0f && element_rect.height > 0.0f && bg_color.a > 0) {
            render_DrawRectangle(rc, element_rect.x, element_rect.y, element_rect.width, element_rect.height, bg_color);
          }
        }

        // handle click
        if (!Get_Flag(ui_state->flags, Ui_State_Flag_action_occured) && is_hot) {
          context->hot_process = (Process_Loc){0};

          if (IsMouseButtonPressed(0)) {
            Set_Flag(ui_state->flags, Ui_State_Flag_action_occured);
            // set as active
            if (Get_Flag(process->flags, Process_Flag_CanBeActive)) {
              Toggle_Flag(process->flags, Process_Flag_IsActive);
            }
            // call func
            if (process->func) {
              process->func(context, view, process);
            }
          }
        }
      }

#if 1
      // TODO: Allow label-editing during UI
      // @Copypasta Undo keybind handler
      if (Get_Flag(process->flags, Process_Flag_TextEdit)) {
        if (process->label == 0) {
          process->label = push_struct(context->ui_arena, Piece_Table);
        }
        if (process->label) {
          Process_List p_list = (Process_List){process, process};
          handle_label_editing(context, view, p_list);
        }
      }
#endif

      { // draw label
        // @Copypasta "draw processes"
        String8 label_string = piece_table_get_string(context->temp_arena, process->label);

        if (label_string.str == 0 || label_string.size == 0) {
          if (process->label_c_string) {
            label_string = str8_lit(process->label_c_string);
          }
        }

        if (label_string.str && label_string.size) {
          render_DrawText(rc, (char *)label_string.str, element_rect.x+padding.x+1.0f, element_rect.y+padding.y+1.0f, font_size, (Color){0, 0, 0, 255}, 0);
          render_DrawText(rc, (char *)label_string.str, element_rect.x+padding.x, element_rect.y+padding.y, font_size, font_color, 0);
        }
      }

      { // handle box layout
        if (process->parent && (process->parent->ui_box.layout == Ui_Layout_Horizontal)) {
          process->parent->ui_box.layout_offset.x += process->ui_box.size.x;
        }
        if (process->parent && (process->parent->ui_box.layout == Ui_Layout_Vertical)) {
          process->parent->ui_box.layout_offset.y += menu_button_y_offset;
        }
      }

      // only-one-active
      if (*parent_process &&
          Get_Flag((*parent_process)->ui_box.flags, Ui_Box_Flag_OnlyOneActive)) {
        B32 is_active = Get_Flag(process->flags, Process_Flag_IsActive);
        if (Get_Flag(context->ui_state.flags, Ui_State_Flag_action_occured)) {
          if (!was_active && is_active) {
            // set as active
            (*parent_process)->ref = process;
          }
          else if (was_active && !is_active) {
            // set as inactive
            (*parent_process)->ref = 0;
          }
        }

        if ((*parent_process)->ref && (*parent_process)->ref != process) {
          Unset_Flag(process->flags, Process_Flag_IsActive);
        }
      }

      // only desencd if active
      if (Get_Flag(process->flags, Process_Flag_UiDescendIfActive) &&
          (((*parent_process)->ref == 0) || ((*parent_process)->ref != process))) {
        process = 0;
      }
      else {
        *parent_process = process;
      }
    }
  }

  return process;
}



function void ui_decl_next(
  Context *context,
  Process **process_ptr,
  Process **parent_process
  ) {
  if (process_ptr && (*process_ptr)) {
    // draw box
    if (Get_Flag((*process_ptr)->flags, Process_Flag_IsBox)) {
      Render_Context *rc = &context->process_render_context;
      Vector2 pos = (*process_ptr)->position;
      Vector2 size = (*process_ptr)->ui_box.size;
      Color color = (*process_ptr)->ui_box.color;
      render_DrawRectangle(rc, pos.x, pos.y, size.x, size.y, color);
    }

    // restore the old parent-process
    if (parent_process) {
      *parent_process = (*process_ptr)->parent;
      (*process_ptr) = 0;
    }
  }
}





#define Ui_Push_Offset_X(n, o)\
  F32 old_offset_x__##n = (o).x

#define Ui_Push_Offset_Y(n, o)\
  F32 old_offset_y__##n = global_ui_procs[Global_Ui_Proc_Id_##n].ui_box.layout_offset.y;\
  global_ui_procs[Global_Ui_Proc_Id_##n].ui_box.layout_offset.y = (o)

#define Ui_Pop_Offset_X(n, o)\
  (o).x = old_offset_x__##n

#define Ui_Pop_Offset_Y(n)\
  global_ui_procs[Global_Ui_Proc_Id_##n].ui_box.layout_offset.y = old_offset_y__##n

#define Ui_Push_Offset(n, o)\
  Vector2 old_offset__##n = global_ui_procs[Global_Ui_Proc_Id_##n].ui_box.layout_offset;\
  global_ui_procs[Global_Ui_Proc_Id_##n].ui_box.layout_offset = (o)

#define Ui_Pop_Offset(n)\
  global_ui_procs[Global_Ui_Proc_Id_##n].ui_box.layout_offset = old_offset__##n


#define D(c, v, n)\
  for (Process *n = ui_decl_init((c), (v), &parent_process, padding, menu_button_y_offset, &global_ui_procs[Global_Ui_Proc_Id_##n], Process_Do_Undo_Kind_Ui);\
       n != 0;\
       ui_decl_next((c), &n, &parent_process))















Define_Keybind_And_Action(
  Do_Elements, Ui,
  Keybind_Behavior_Overwrite, OnlyOnce,
  0, 0, 0, View_Kind_Flag_Ui,
  "Do the UI."
  ) {
  B32 handled = 0;
  Context *c = env->context;
  View *v = env->view;

  if (c && v) {
    Render_Context *rc = &c->ui_render_context;
    Process *parent_process = 0;
    B32 mouse_pressed = Get_Flag(c->ui_state.flags, Ui_State_Flag_mouse0_pressed);

    Vector2 padding = global_button_padding;
    F32 menu_button_y_offset = global_panel_font_size + 2.0f*padding.y;

    D(c, v, top_menu_box) {
      Rectangle r = v->screen_region;
      render_command *command = render_DrawRectangle(rc, r.x, r.y, r.width, r.height, v->color);
      D(c, v, file_menu_button) {
        Ui_Push_Offset_Y(file_menu_button, file_menu_button->ui_box.layout_offset.y + menu_button_y_offset);
        {
          D(c, v, open_file_button);
          D(c, v, save_file_button);
          D(c, v, save_as_file_button);
        }
        Ui_Pop_Offset_Y(file_menu_button);
      }
      D(c, v, edit_menu_button) {
        Ui_Push_Offset_Y(edit_menu_button, edit_menu_button->ui_box.layout_offset.y + menu_button_y_offset);
        {
          D(c, v, copy_button);
          D(c, v, paste_button);
        }
        Ui_Pop_Offset_Y(edit_menu_button);
      }

      { // retroactively resize view background for top_menu_box
        v->screen_region = (Rectangle){
          top_menu_box->position.x,
          top_menu_box->position.y,
          global_window_size.x,
          top_menu_box->ui_box.size.y};
        command->Width = v->screen_region.width;
        command->Height = v->screen_region.height;
      }


#if 1
      D(c, v, open_file_box) {
        Ui_Push_Offset(open_file_box, Vector2Scale(global_window_size, 0.5f));
        {
          D(c, v, open_file_label);
          D(c, v, open_file_confirm_box) {
            top_menu_box->ui_box.layout_offset =
              open_file_box->ui_box.layout_offset;
            D(c, v, open_button);
            D(c, v, cancel_button);
          }
        }
        Ui_Pop_Offset(open_file_box);
      }
#endif
      {
        D(c, v, save_file_as_box) {
          Ui_Push_Offset(save_file_as_box, Vector2Scale(global_window_size, 0.5f));
          if (save_file_as_box) {
            save_file_as_box->position = save_file_as_box->ui_box.layout_offset;
          }
          {
            D(c, v, save_file_as_text_input);
            D(c, v, save_button);
          }
          Ui_Pop_Offset(save_file_as_box);
        }
      }
    }

    // clear out active refs if the user clicks "away" from UI
    if (mouse_pressed &&
        !Get_Flag(c->ui_state.flags, Ui_State_Flag_action_occured)) {
      Process *top_menu_box = global_ui_procs + Global_Ui_Proc_Id_top_menu_box;
      if (top_menu_box) {
        top_menu_box->ref = 0;
      }
    }
  }

  return handled;
}

#undef D
