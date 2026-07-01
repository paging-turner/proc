#define Piece_Table_Chunk_Size 8


typedef struct Piece_Table_Chunk {
  U8 str_array[Piece_Table_Chunk_Size];
  U32 offset;
  B32 frozen; // is "frozen" this necessary?
} Piece_Table_Chunk;


typedef struct Piece_Table_Row {
  Piece_Table_Chunk *chunk;
  U32 offset;
  U32 size;
} Piece_Table_Row;


#define Piece_Table_Row_Count 4


typedef struct Piece_Table_Section {
  struct Piece_Table_Section *next;
  Piece_Table_Row rows[Piece_Table_Row_Count];
  U32 write_row_offset;
} Piece_Table_Section;


typedef struct Piece_Table_Range {
  Piece_Table_Section *section;
  U64 row_offset;
  U64 row_count;
} Piece_Table_Range;


struct Piece_Table {
  Piece_Table_Section *first_section;
  Piece_Table_Section *last_section;
  Piece_Table_Chunk *edit_chunk;
  Piece_Table_Range range;
};




#define Piece_Table_Is_Empty(t)\
  ((t)->first_section == 0 || (t)->last_section == 0)




function Piece_Table_Section *piece_table_get_editable_section(
  Context *context,
  Piece_Table *table
  ) {
  Piece_Table_Section *result = table->last_section;

  if (table->last_section == 0 ||
      table->last_section->write_row_offset == Piece_Table_Row_Count) {
    Piece_Table_Section *new_section = push_struct(context->permanent_arena, Piece_Table_Section);
    if (new_section) {
      SLLQueuePush(table->first_section, table->last_section, new_section);
      result = new_section;
    }
    else {
      result = 0;
    }
  }

  return result;
}



function Piece_Table_Row *piece_table_get_editable_row(
  Context *context,
  Piece_Table *table
  ) {
  Piece_Table_Row *row = 0;
  Piece_Table_Section *section = piece_table_get_editable_section(context, table);

  if (section) {
    row = section->rows + section->write_row_offset;
    section->write_row_offset += 1;
  }

  return row;
}



function B32 piece_table_write_row(
  Context *context,
  Piece_Table *table,
  Piece_Table_Range *range,
  Piece_Table_Chunk *chunk,
  U32 offset,
  U32 size
  ) {
  B32 error = 0;

  if (size) {
    Piece_Table_Row *row = piece_table_get_editable_row(context, table);

    if (row) {
      row->chunk = chunk;
      row->offset = offset;
      row->size = size;
      range->row_count += 1;
    }
    else {
      error = 1;
    }
  }

  return error;
}



function Piece_Table_Range piece_table_copy_text_into_table(
  Context *context,
  Piece_Table *table,
  Piece_Table_Range range,
  String8 string_to_insert
  ) {
  Piece_Table_Range result_range = range;
  U64 amount_of_text_copied = 0;

  while (string_to_insert.size - amount_of_text_copied) {
    U64 space_in_edit_chunk = Piece_Table_Chunk_Size - table->edit_chunk->offset;
    U64 amount_of_text_to_copy = string_to_insert.size - amount_of_text_copied;

    if (space_in_edit_chunk >= amount_of_text_to_copy) {
      // copy all of the text
      MemoryCopy(table->edit_chunk->str_array + table->edit_chunk->offset,
                 string_to_insert.str + amount_of_text_copied,
                 amount_of_text_to_copy);

      // write the row
      if (piece_table_write_row(
            context, table, &result_range,
            table->edit_chunk,
            table->edit_chunk->offset,
            amount_of_text_to_copy)) {
        printf("[ Error ] Getting editable row. Copy full text into table.\n");
        break;
      }
      else {
        table->edit_chunk->offset += amount_of_text_to_copy;
      }

      // create new, empty edit-chunk
      if (space_in_edit_chunk == amount_of_text_to_copy) {
        table->edit_chunk->frozen = 1;
        table->edit_chunk->offset = Piece_Table_Chunk_Size;
        table->edit_chunk = push_struct(context->permanent_arena, Piece_Table_Chunk);
        if (table->edit_chunk == 0) {
          printf("[ Error ] Pushing Piece_Table_Chunk for table's edit-chunk. Empty edit-chunk\n");
        }
      }
      break;
    }
    else {
      // copy some of the text
      MemoryCopy(table->edit_chunk->str_array + table->edit_chunk->offset,
                 string_to_insert.str + amount_of_text_copied,
                 space_in_edit_chunk);
      amount_of_text_copied += space_in_edit_chunk;

      // write the row
      if (piece_table_write_row(
            context, table, &result_range,
            table->edit_chunk,
            table->edit_chunk->offset,
            space_in_edit_chunk)) {
        printf("[ Error ] Getting editable row. Copy part of text into table.\n");
        break;
      }
      else {
        table->edit_chunk->offset += space_in_edit_chunk;
      }

      // new edit-chunk
      table->edit_chunk = push_struct(context->permanent_arena, Piece_Table_Chunk);
      if (table->edit_chunk == 0) {
        printf("[ Error ] Pushing Piece_Table_Chunk for table's edit-chunk. Empty edit-chunk\n");
        break;
      }
    }
  }

  return result_range;
}




function Piece_Table_Range piece_table_insert(
  Context *context,
  Piece_Table *table,
  Piece_Table_Range range,
  U64 text_offset,
  String8 string_to_insert
  ) {
  Piece_Table_Range result_range = (Piece_Table_Range){0};

  Assert(range.row_offset < Piece_Table_Row_Count);

  if (Piece_Table_Is_Empty(table)) {
    table->edit_chunk = push_struct(context->permanent_arena, Piece_Table_Chunk);
    if (table->edit_chunk) {
      result_range = piece_table_copy_text_into_table(context, table, range, string_to_insert);
      result_range.section = table->last_section;
    }
    else {
      printf("[ Error ] Pushing Piece_Table_Chunk for edit-chunk of empty table.\n");
    }
  }
  else {
    // NOTE: for now, we do not use refs...
    B32 row_edited = 0;
    Piece_Table_Section *current_section = range.section;
    U32 current_row_offset = range.row_offset;
    U32 current_text_offset = 0;

    // fill out beginning section
    result_range.section = piece_table_get_editable_section(context, table);
    result_range.row_offset = result_range.section->write_row_offset;

    if (result_range.section == 0) {
      printf("[ Error ] Pushing Piece_Table_Section for new copy-section. Init.\n");
      result_range = (Piece_Table_Range){0};
    }
    else {
      for (U64 i = 0; i < range.row_count; ++i) {
        // update current-row trackers
        if (current_row_offset == Piece_Table_Row_Count) {
          current_row_offset = 0;
          Assert(current_section->next);
          current_section = current_section->next;
        }

        Piece_Table_Row *current_row = current_section->rows + current_row_offset;
        current_text_offset += current_row->size;

        if (!row_edited && current_text_offset == text_offset) {
          // copy current row
          if (piece_table_write_row(
                context, table, &result_range,
                current_row->chunk,
                current_row->offset,
                current_row->size)) {
            printf("[ Error ] Getting editable row. Insert. No row split.\n");
          }

          // copy the new text
          result_range = piece_table_copy_text_into_table(context, table, result_range, string_to_insert);
          row_edited = 1;
        }
        else if (!row_edited && current_text_offset > text_offset) {
          U64 text_size_after = current_text_offset - text_offset;
          Assert(text_size_after <= current_row->size);
          U64 text_size_before = current_row->size - text_size_after;

          // copy first part of current row
          if (piece_table_write_row(
                context, table, &result_range,
                current_row->chunk,
                current_row->offset,
                text_size_before)) {
            printf("[ Error ] Pushing Piece_Table_Section for new copy-section. Row split first part.\n");
            break;
          }

          // copy new row
          result_range = piece_table_copy_text_into_table(context, table, result_range, string_to_insert);

          // copy last part of current row
          if (piece_table_write_row(
                context, table, &result_range,
                current_row->chunk,
                current_row->offset + text_size_before,
                text_size_after)) {
            printf("[ Error ] Pushing Piece_Table_Section for new copy-section. Row split last part.\n");
            break;
          }

          row_edited = 1;
        }
        else {
          // copy current row
          if (piece_table_write_row(
                context, table, &result_range,
                current_row->chunk,
                current_row->offset,
                current_row->size)) {
            printf("[ Error ] Getting editable row. Row copy.\n");
          }
        }

        // increment row offset
        current_row_offset += 1;
      }
    }
  }

  return result_range;
}





function Piece_Table_Range piece_table_delete(
  Context *context,
  Piece_Table *table,
  Piece_Table_Range range,
  U64 text_offset,
  U64 amount_to_delete
  ) {
  Assert(range.row_offset < Piece_Table_Row_Count);
  Piece_Table_Range result_range = (Piece_Table_Range){0};

  if (Piece_Table_Is_Empty(table)) {
    // nothing to do, just return the same range that was passed in
    result_range = range;
  }
  else {
    enum Delete_Mode {
      Delete_Mode_Begin,
      Delete_Mode_Middle,
      Delete_Mode_End,
    };
    enum Delete_Mode mode = Delete_Mode_Begin;
    B32 row_edited = 0;
    Piece_Table_Section *current_section = range.section;
    U32 current_row_offset = range.row_offset;
    U32 current_text_offset = 0;
    U64 target_text_offset = text_offset;

    // init result-range
    result_range.section = piece_table_get_editable_section(context, table);
    result_range.row_offset = result_range.section->write_row_offset;

    for (U64 i = 0; i < range.row_count; ++i) {
      // update current-row trackers
      if (current_row_offset == Piece_Table_Row_Count) {
        current_row_offset = 0;
        Assert(current_section->next);
        current_section = current_section->next;
      }

      Piece_Table_Row *current_row = current_section->rows + current_row_offset;
      current_text_offset += current_row->size;

      if (mode != Delete_Mode_End && current_text_offset == target_text_offset) {
        if (mode == Delete_Mode_Begin) {
          mode = Delete_Mode_Middle;
          target_text_offset += amount_to_delete;

          // copy the row
          if (piece_table_write_row(
                context, table, &result_range,
                current_row->chunk,
                current_row->offset,
                current_row->size)) {
            printf("[ Error ] Getting editable row. Delete. No row split.\n");
          }
        }
        else if (mode == Delete_Mode_Middle) {
          mode = Delete_Mode_End;
        }
      }
      else if (mode != Delete_Mode_End && current_text_offset > target_text_offset) {
        U64 text_size_after = 0;
        if ((text_offset + amount_to_delete) < current_text_offset) {
          text_size_after = current_text_offset - (text_offset + amount_to_delete);
        }
        Assert(text_size_after <= current_row->size);
        U64 text_size_before = current_row->size - (text_size_after + amount_to_delete);

        if (mode == Delete_Mode_Begin) {
          // when in begin mode, copy the first part of the row
          mode = Delete_Mode_Middle;

          // copy first part of current row
          if (piece_table_write_row(
                context, table, &result_range,
                current_row->chunk,
                current_row->offset,
                text_size_before)) {
            printf("[ Error ] Pushing Piece_Table_Section for new copy-section. Delete. Row split first part.\n");
            break;
          }
        }
        else if (mode == Delete_Mode_Middle) {
          // when in middle mode, copy the last part of the row
          mode = Delete_Mode_End;
          // copy last part of current row
          if (piece_table_write_row(
                context, table, &result_range,
                current_row->chunk,
                current_row->offset + text_size_before,
                text_size_after)) {
            printf("[ Error ] Pushing Piece_Table_Section for new copy-section. Delete. Row split last part.\n");
            break;
          }
        }
      }
      else if (mode != Delete_Mode_Middle) {
        // copy current row
        if (piece_table_write_row(
              context, table, &result_range,
              current_row->chunk,
              current_row->offset,
              current_row->size)) {
          printf("[ Error ] Getting editable row. Delete. No row split.\n");
        }
      }

      // check if we are deleting text *within* the current section
      if ((mode != Delete_Mode_End) &&
          (text_offset + amount_to_delete < current_text_offset)) {
        Assert(Piece_Table_Chunk_Size > (text_offset + amount_to_delete));
        mode = Delete_Mode_End;
        if (piece_table_write_row(
              context, table, &result_range,
              current_row->chunk,
              text_offset + amount_to_delete,
              Piece_Table_Chunk_Size - (text_offset + amount_to_delete))) {
          printf("[ Error ] Getting editable row. Delete. Last part within a section.\n");
        }
      }

      // increment row offset
      current_row_offset += 1;
    }
  }

  return result_range;
}



function U8 *c_string_from_piece_table(Arena *arena, Piece_Table *table) {
  /* Assert(!"TODO"); */
  return (U8 *)"";
}



function void debug_print_piece_table(Piece_Table *table) {
  for (Piece_Table_Section *current_section = table->first_section;
       current_section != 0;
       current_section = current_section->next) {
    for (U32 r = 0; r < Piece_Table_Row_Count; ++r) {
      Piece_Table_Row *row = current_section->rows + r;
      char write_row_indicator = r == current_section->write_row_offset ? '#' : ' ';
      printf("%c Chunk %p %d %d\n", write_row_indicator, row->chunk, row->offset, row->size);
    }
  }
}




function void debug_print_piece_table_range(Piece_Table *table, Piece_Table_Range range) {
  Piece_Table_Section *current_section = range.section;
  U32 current_row_offset = range.row_offset;

  for (U32 i = 0; i < range.row_count; ++i) {
    if (current_row_offset == Piece_Table_Row_Count) {
      if (current_section->next) {
        current_section = current_section->next;
        current_row_offset = 0;
      }
      else {
        printf("[ Error ] Ran out of sections while printing piece-table.\n");
        break;
      }
    }

    Piece_Table_Row *row = current_section->rows + current_row_offset;

    for (U32 c = 0; c < row->size; ++c) {
      printf("%c", row->chunk->str_array[row->offset+c]);
    }

    current_row_offset += 1;
  }
}















