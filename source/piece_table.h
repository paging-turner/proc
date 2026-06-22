#define Piece_Table_Chunk_Size 1024


typedef struct Piece_Table_Chunk {
  U8 str_array[Piece_Table_Chunk_Size];
  U32 offset;
  B32 frozen; // is "frozen" this necessary?
} Piece_Table_Chunk;


typedef struct Piece_Table Piece_Table;
typedef struct Piece_Table_Section Piece_Table_Section;


#if 0
typedef enum Piece_Table_Row_Kind {
  Piece_Table_Row__Null,
  Piece_Table_Row_Chunk,
  Piece_Table_Row_Ref,
} Piece_Table_Row_Kind;
#endif


typedef struct Piece_Table_Row {
#if 0
  Piece_Table_Row_Kind kind;
#endif
  union {
    Piece_Table_Section *section;
    Piece_Table_Chunk *chunk;
  };
  U32 offset;
  U32 size;
} Piece_Table_Row;


#if 0
typedef struct Piece_Table_Row_Stack {
  struct Piece_Table_Row_Stack *next;
  Piece_Table *table;
  Piece_Table_Row *row;
  B32 visited;
} Piece_Table_Row_Stack;
#endif


#define Piece_Table_Row_Count 16


struct Piece_Table_Section {
  struct Piece_Table_Section *next;
  Piece_Table_Row rows[Piece_Table_Row_Count];
  U32 write_row_offset;
};


struct Piece_Table {
  Piece_Table_Section *first_section;
  Piece_Table_Section *last_section;
  Piece_Table_Chunk *edit_chunk;
};


typedef struct Piece_Table_Range {
  Piece_Table_Section *section;
  U64 row_offset;
  U64 row_count;
} Piece_Table_Range;



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
  }

  return row;
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

      // create new, empty edit-chunk
      if (space_in_edit_chunk == amount_of_text_to_copy) {
        table->edit_chunk->frozen = 1;
        table->edit_chunk->offset = Piece_Table_Chunk_Size;
        table->edit_chunk = push_struct(context->permanent_arena, Piece_Table_Chunk);
        if (table->edit_chunk == 0) {
          printf("[ Error ] Pushing Piece_Table_Chunk for table's edit-chunk. Empty edit-chunk\n");
        }
      }

      // write the row
      Piece_Table_Row *copy_row = piece_table_get_editable_row(context, table);
      if (copy_row) {
        copy_row->chunk = table->edit_chunk;
        copy_row->offset = table->edit_chunk->offset;
        copy_row->size = amount_of_text_to_copy;
        table->last_section->write_row_offset += 1;
        result_range.row_count += 1;
      }
      else {
        printf("[ Error ] Getting editable row. Copy full text into table.\n");
        break;
      }

      // update edit-chunk offset
      table->edit_chunk->offset += amount_of_text_to_copy;
      break;
    }
    else {
      // copy some of the text
      MemoryCopy(table->edit_chunk->str_array + table->edit_chunk->offset,
                 string_to_insert.str + amount_of_text_copied,
                 space_in_edit_chunk);
      amount_of_text_copied += space_in_edit_chunk;

      // new edit-chunk
      table->edit_chunk = push_struct(context->permanent_arena, Piece_Table_Chunk);
      if (table->edit_chunk == 0) {
        printf("[ Error ] Pushing Piece_Table_Chunk for table's edit-chunk. Empty edit-chunk\n");
        break;
      }

      // write the row
      Piece_Table_Row *copy_row = piece_table_get_editable_row(context, table);
      if (copy_row) {
        copy_row->chunk = table->edit_chunk;
        copy_row->offset = table->edit_chunk->offset;
        copy_row->size = space_in_edit_chunk;
        table->last_section->write_row_offset += 1;
        result_range.row_count += 1;
      }
      else {
        printf("[ Error ] Getting editable row. Copy part of text into table.\n");
        break;
      }

      // update edit-chunk offset
      table->edit_chunk->offset = space_in_edit_chunk;
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
  Assert(range.row_offset < Piece_Table_Row_Count);

  Piece_Table_Range result_range = (Piece_Table_Range){0};

  if (Piece_Table_Is_Empty(table)) {
    // table is empty, so just add the text
    U32 remainder = string_to_insert.size % Piece_Table_Chunk_Size;
    B32 remainder_count = remainder ? 1 : 0;
    U32 full_chunk_count = (string_to_insert.size / Piece_Table_Chunk_Size) + remainder_count;

    U32 current_row_offset = 0;
    Piece_Table_Section *current_section = 0;

    // copy full chunks
    for (U32 c = 0; c < full_chunk_count; ++c) {
      Piece_Table_Chunk *chunk = push_struct(context->permanent_arena, Piece_Table_Chunk);

      if (chunk) {
        Piece_Table_Row *copy_row = piece_table_get_editable_row(context, table);

        if (copy_row) {
          U32 row_size = 0;

          if (c+1 < full_chunk_count) {
            // copy a full chunk
            U64 string_offset = c * Piece_Table_Chunk_Size;
            MemoryCopy(table->edit_chunk->str_array,
                       string_to_insert.str + string_offset,
                       Piece_Table_Chunk_Size);
            chunk->frozen = 1;

            row_size = Piece_Table_Chunk_Size;
          }
          else if (remainder) {
            // copy the remainder
            table->edit_chunk = chunk;
            table->edit_chunk->offset = remainder;
            U64 string_offset = c * Piece_Table_Chunk_Size;
            MemoryCopy(table->edit_chunk->str_array,
                       string_to_insert.str + string_offset,
                       remainder);

            row_size = remainder;
          }

          // fill out the row
#if 0
          copy_row->kind = Piece_Table_Row_Chunk;
#endif
          copy_row->chunk = chunk;
          copy_row->offset = 0;
          copy_row->size = row_size;
          result_range.row_count += 1;

          // fill out the beginning section
          if (c == 0) {
            result_range.section = table->last_section;
          }
        }
        else {
          printf("[ Error ] Getting editable-row for empty Piece_Table.\n");
          result_range = (Piece_Table_Range){0};
          break;
        }
      }
      else {
        printf("[ Error ] Pushing Piece_Table_Chunk to empty Piece_Table.\n");
        result_range = (Piece_Table_Range){0};
        break;
      }
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

    if (result_range.section == 0) {
      printf("[ Error ] Pushing Piece_Table_Section for new copy-section. Init.\n");
    }
    else {
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

        if (!row_edited && current_text_offset == text_offset) {
          // copy current row
          Piece_Table_Row *copy_row = piece_table_get_editable_row(context, table);
          if (copy_row) {
            *copy_row = *current_row;
            table->last_section->write_row_offset += 1;
            result_range.row_count += 1;
          }
          else {
            printf("[ Error ] Getting editable row. No row split.\n");
          }

          // copy the new text
          result_range = piece_table_copy_text_into_table(context, table, result_range, string_to_insert);
          row_edited = 1;
        }
        else if (!row_edited && current_text_offset > text_offset) {
          U64 text_size_before = current_text_offset - text_offset;
          Assert(text_size_before < current_row->size);
          U64 text_size_after = current_row->size - text_size_before;

          // copy first part of current row
          Piece_Table_Row *copy_row = piece_table_get_editable_row(context, table);
          if (copy_row) {
            copy_row->chunk = current_row->chunk;
            copy_row->offset = current_row->offset;
            copy_row->size = text_size_before;
            table->last_section->write_row_offset += 1;
            result_range.row_count += 1;
          }
          else {
            printf("[ Error ] Pushing Piece_Table_Section for new copy-section. Row split first part.\n");
            break;
          }

          // copy new row
          result_range = piece_table_copy_text_into_table(context, table, result_range, string_to_insert);

          // copy last part of current row
          copy_row = piece_table_get_editable_row(context, table);
          if (copy_row) {
            copy_row->chunk = current_row->chunk;
            copy_row->offset = current_row->offset + text_size_before;
            copy_row->size = text_size_after;
            table->last_section->write_row_offset += 1;
            result_range.row_count += 1;
          }
          else {
            printf("[ Error ] Pushing Piece_Table_Section for new copy-section. Row split last part.\n");
            break;
          }

          row_edited = 1;
        }
        else {
          // copy current row
          Piece_Table_Row *copy_row = piece_table_get_editable_row(context, table);
          if (copy_row) {
            *copy_row = *current_row;
            table->last_section->write_row_offset += 1;
            result_range.row_count += 1;
          }
          else {
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





function void piece_table_delete(
  Piece_Table *table,
  U64 row_offset,
  U64 offset,
  U64 amount
  ) {
  Assert(!"TODO");
  Assert(row_offset < Piece_Table_Row_Count);
}




function void debug_print_piece_table(Piece_Table *table) {
  for (Piece_Table_Section *current_section = table->first_section;
       current_section != 0;
       current_section = current_section->next) {
    for (U32 r = 0; r < Piece_Table_Row_Count; ++r) {
      Piece_Table_Row *row = current_section->rows + r;

#if 0
      switch (row->kind) {
      case Piece_Table_Row_Chunk: {
        printf("Chunk %p %d %d\n", row->chunk, row->offset, row->size);
      } break;
      case Piece_Table_Row_Ref: {
        printf("  Ref %p %d %d\n", row->chunk, row->offset, row->size);
      } break;
      default: {
        r = Piece_Table_Row_Count;
        current_section = 0;
      }
      }
#else
      printf("Chunk %p %d %d\n", row->chunk, row->offset, row->size);
#endif
    }
  }
}




function void debug_print_piece_table_range(Piece_Table *table, Piece_Table_Range range) {
  Piece_Table_Section *current_section = range.section;
  U32 current_row_offset = range.row_offset;

  for (U32 i = 0; i < range.row_count; ++i) {
    Piece_Table_Row *row = current_section->rows + current_row_offset;

#if 0
    if (row->kind == Piece_Table_Row_Chunk) {
      for (U32 c = 0; c < row->size; ++c) {
        printf("%c", row->chunk->str_array[row->offset+c]);
      }
    }
    else if (row->kind == Piece_Table_Row_Ref) {
      Assert(!"TODO");
    }
    else {
      printf("[ Error ] Unknown row found while printing piece-table range.\n");
      break;
    }
#else
    for (U32 c = 0; c < row->size; ++c) {
      printf("%c", row->chunk->str_array[row->offset+c]);
    }
#endif

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
    else {
      current_row_offset += 1;
    }
  }
}















