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


struct Piece_Table {
  Piece_Table_Row *first_row;
  Piece_Table_Row *last_row;
  Piece_Table_Chunk *insertion_chunk;
  U64 text_size;
};




function Piece_Table_Row *piece_table_create_row(Context *context) {
  Piece_Table_Row *row = 0;

  if (context->piece_table_memory.free_rows) {
    row = context->piece_table_memory.free_rows;
    SLLStackPop(context->piece_table_memory.free_rows);
    *row = (Piece_Table_Row){0};
  }
  else {
    row = push_struct(context->permanent_arena, Piece_Table_Row);
  }

  return row;
}



function Piece_Table_Chunk *piece_table_create_chunk(Context *context) {
  Piece_Table_Chunk *chunk = 0;

  if (context->piece_table_memory.free_chunks) {
    chunk = context->piece_table_memory.free_chunks;
    SLLStackPop(context->piece_table_memory.free_chunks);
    *chunk = (Piece_Table_Chunk){0};
  }
  else {
    chunk = push_struct(context->permanent_arena, Piece_Table_Chunk);
  }

  return chunk;
}



function B32 piece_table_ensure_insertion_chunk_exists(
  Context *context,
  Piece_Table *table
  ) {
  B32 error = 0;

  if (table->insertion_chunk == 0) {
    table->insertion_chunk = piece_table_create_chunk(context);
    if (table->insertion_chunk == 0) {
      error = 1;
    }
  }

  return error;
}



function void piece_table_insert_text_after_row(
  Context *context,
  Piece_Table *table,
  Piece_Table_Row *row,
  String8 text_to_insert
  ) {
  U64 amount_of_text_copied = 0;
  Piece_Table_Row *current_row = row;

  while (amount_of_text_copied < text_to_insert.size) {
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

    // insert the row
    Piece_Table_Row *new_row = piece_table_create_row(context);
    if (new_row) {
      new_row->chunk = table->insertion_chunk;
      new_row->offset = table->insertion_chunk->offset;
      new_row->size = amount_to_write;
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

    amount_of_text_copied += amount_to_write;
    table->insertion_chunk->offset += amount_to_write;
    table->text_size += amount_to_write;

    // Create a new insertion-chunk if we need one.
    if (table->insertion_chunk->offset == Piece_Table_Chunk_Size) {
      table->insertion_chunk = piece_table_create_chunk(context);
      if (table->insertion_chunk == 0) {
        printf("[ Error ] Creating Piece_Table_Chunk while inserting text after a row.\n");
        break;
      }
    }
  }
}




function void piece_table_insert(
  Context *context,
  Piece_Table *table,
  U64 text_offset,
  String8 text_to_insert
  ) {
  U64 current_text_offset = 0;

  if (piece_table_ensure_insertion_chunk_exists(context, table)) {
    printf("[ Error ] Ensuring piece-table has an insertion-chunk while inserting.\n");
    return;
  }

  if (table->first_row && table->last_row) {
    // insert text into existing rows
    List_For(Piece_Table_Row *, row, table->first_row) {
      current_text_offset += row->size;
      Piece_Table_Row *row_before = text_offset == 0 ? 0 : row;

      if (current_text_offset == text_offset) {
        // insert text between rows
        piece_table_insert_text_after_row(context, table, row_before, text_to_insert);
        break;
      }
      else if (current_text_offset > text_offset) {
        // split row and insert text
        Piece_Table_Row *new_row = piece_table_create_row(context);
        if (new_row) {
          U64 first_part_size = current_text_offset - text_offset;
          U64 last_part_size = row->size - first_part_size;
          // adjust size of current row
          row->size = first_part_size;
          // insert last part of current row as new row
          new_row->chunk = row->chunk;
          new_row->offset = row->offset + first_part_size;
          new_row->size = last_part_size;
          DLLInsert(table->first_row, table->last_row, row, new_row);
          // insert text between current row
          piece_table_insert_text_after_row(context, table, row_before, text_to_insert);
          break;
        }
        else {
          printf("[ Error ] Creating Piece_Table_Row while inserting text.\n");
          break;
        }
      }
    }
  }
  else {
    // table is empty, so just insert the text
    piece_table_insert_text_after_row(context, table, 0, text_to_insert);
  }
}



function void piece_table_delete(
  Context *context,
  Piece_Table *table,
  U64 text_offset,
  U64 size
  ) {
  B32 is_deleting = 0;
  U64 current_text_offset = 0;
  U64 end_text_offset = text_offset + size;

  if (piece_table_ensure_insertion_chunk_exists(context, table)) {
    printf("[ Error ] Ensuring piece-table has an insertion-chunk while inserting.\n");
    return;
  }

  Piece_Table_Row *delete_start_row = 0;

  // search for first row to begin deleting
  List_For(Piece_Table_Row *, row, table->first_row) {
    current_text_offset += row->size;

    Piece_Table_Row *row_before = text_offset == 0 ? 0 : row;
    if (current_text_offset == text_offset) {
      delete_start_row = row->next;
      break;
    }
    else if (current_text_offset > text_offset) {
      U64 deleted_size = current_text_offset - text_offset;
      if (deleted_size > size) {
        // split the row in two
        U64 non_first_size = current_text_offset - text_offset;
        row->size = Piece_Table_Chunk_Size - non_first_size;
        table->text_size -= size;
        Piece_Table_Row *new_row = piece_table_create_row(context);
        if (new_row) {
          new_row->chunk = row->chunk;
          new_row->offset = row->offset + row->size + size;
          new_row->size = deleted_size - size;
          DLLInsert(table->first_row, table->last_row, row, new_row);
        }
        else {
          printf("[ Error ] Creating new row while deleting text.\n");
          break;
        }
      }
      else {
        // decrease the row size
        U64 non_deleted_size = Piece_Table_Chunk_Size - deleted_size;
        row->size = non_deleted_size;
        delete_start_row = row->next;
        table->text_size -= deleted_size;
      }
      break;
    }
  }

  // delete some rows
  List_For(Piece_Table_Row *, row, delete_start_row) {
    current_text_offset += row->size;

    if (current_text_offset > end_text_offset) {
      U64 non_deleted_size = current_text_offset - end_text_offset;
      U64 deleted_size = Piece_Table_Chunk_Size - non_deleted_size;
      row->offset += deleted_size;
      row->size = non_deleted_size;
      table->text_size -= deleted_size;
      break;
    }
    else {
      DLLRemove(table->first_row, table->last_row, row);
      SLLStackPush(context->piece_table_memory.free_rows, row);
      table->text_size -= Piece_Table_Chunk_Size;
      if (current_text_offset == end_text_offset) {
        break;
      }
    }
  }
}



function U8 *piece_table_get_c_string(Arena *arena, Piece_Table *table) {
  U8 *c_string = (U8 *)"";
  U64 amount_written = 0;

  if (table->text_size) {
    c_string = arena_push(arena, table->text_size);
    if (c_string) {
      List_For(Piece_Table_Row *, row, table->first_row) {
        if (amount_written + row->size > table->text_size) {
          printf("[ Error ] Amount of text in piece-table is greater than the given piece-table's text-size. Getting c-string from piece-table.\n");
          c_string = (U8 *)"";
          break;
        }
        else {
          MemoryCopy(c_string + amount_written,
                     row->chunk->str_array + row->offset,
                     row->size);
          amount_written += row->size;
        }
      }
    }
    else {
      printf("[ Error ] Pushing c-string while getting c-string for piece-table.\n");
    }
  }

  return c_string;
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




function void debug_print_piece_table_range(Context *context, Piece_Table *table) {
  U8 *c_string = piece_table_get_c_string(context->temp_arena, table);
  printf("%s\n", c_string);
}
