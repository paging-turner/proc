#define Save_File_Magic_Number AsciiID4('p', 'r', 'o', 'c')
#define Save_File_Version 0

typedef struct {
  U32 magic_number;
  U32 version;
  U32 process_count;
  union {
    U32 string_count; // TODO: Once we phase out string_count, we can remove this member, and hopefully before we have enough v1 files to care.
    U32 string_size;
  };
} Save_File_Header;

typedef U64 Cold_Process_Id;

// TODO: Begin to use specialized "cold" process struct.
typedef struct {
  B32 flags;
  Vector2 position;
  String8 label;

  Cold_Process_Id in;
  Cold_Process_Id out;

  U32 which_in;
  U32 which_out;

  U8 unused_bytes[8];
} Cold_Process;

// "Freeze" Cold_Process
StaticAssert(sizeof(Cold_Process) == 64, cold_process_should_be_64_bytes);
Freeze_Member(Cold_Process, flags       ,  0);
Freeze_Member(Cold_Process, position    ,  4);
Freeze_Member(Cold_Process, label       , 16);
Freeze_Member(Cold_Process, in          , 32);
Freeze_Member(Cold_Process, out         , 40);
Freeze_Member(Cold_Process, which_in    , 48);
Freeze_Member(Cold_Process, which_out   , 52);
Freeze_Member(Cold_Process, unused_bytes, 56);


#define Save_File_Size_V1(process_count, string_size)\
  (sizeof(Save_File_Header)+\
   process_count*sizeof(Cold_Process)+\
   string_size)

#define Save_File_Start_Of_Processes_V1(save_file_bytes)\
  (Cold_Process *)((save_file_bytes)+sizeof(Save_File_Header))

#define Save_File_Start_Of_Strings_V1(save_file_bytes, process_count)\
  (U8 *)((save_file_bytes)+(sizeof(Save_File_Header)+process_count*sizeof(Cold_Process)))

//////////////////////////////////////
// Saves Functions
//////////////////////////////////////
function void write_save_file(Context *context, Arena *arena, String_Chunk_List file_name);
function void open_file_and_replace_processes(Context *context, String_Chunk_List file_name_list);





function void set_as_current_file(Context *context, String_Chunk_List file_name) {
  context->save_file_name = file_name;
}

function void write_save_file_v1(Context *context, Arena *arena, String_Chunk_List file_name) {
  // TODO: ensure that the Saves_Filepath directory exists before writing a file into it.
  os_set_current_directory(Saves_Filepath);

  // save-file sizing and cold-indexing
  U64 process_cold_index = 0;
  U64 string_cold_size = 0;
  U64 string_cold_offset = 0;
  for (Process *p = context->processes.first; p != 0; p = p->next) {
    p->cold_index = process_cold_index+1;
    process_cold_index += 1;
    for (String_Chunk *s = p->label.first; s != 0; s = s->next) {
      string_cold_size += String_Chunk_Size; // TODO: At the last chunk, we can save space if the whole chunk is not used (if there are zeroes at the end).
    }
  }

  String8 save_file_data;
  save_file_data.size = Save_File_Size_V1(process_cold_index, string_cold_size);
  save_file_data.str = arena_push(arena, save_file_data.size);

  if (save_file_data.str) {
    Save_File_Header *header = (Save_File_Header *)save_file_data.str;
    header->magic_number = Save_File_Magic_Number;
    header->version = 1;
    header->process_count = process_cold_index;
    header->string_size = string_cold_size;

    Cold_Process *first_cold_process = Save_File_Start_Of_Processes_V1(save_file_data.str);
    U8 *start_of_cold_string = Save_File_Start_Of_Strings_V1(save_file_data.str, process_cold_index);

    process_cold_index = 0;
    for (Process *p = context->processes.first; p != 0; p = p->next) {
      Cold_Process *cold_process = first_cold_process + process_cold_index;

      cold_process->flags = p->flags;
      cold_process->position = p->position;
      cold_process->label.str = start_of_cold_string + string_cold_offset;

      if (p->in) {
        cold_process->in = p->in->cold_index;
      }
      if (p->out) {
        cold_process->out = p->out->cold_index;
      }

      cold_process->which_in = p->which_in;
      cold_process->which_out = p->which_out;

      // store string chunks
      for (String_Chunk *s = p->label.first; s != 0; s = s->next) {
        U8 *string_location = start_of_cold_string + string_cold_offset;
        memory_move(string_location, s->str_array, String_Chunk_Size);
        string_cold_offset += String_Chunk_Size;
        cold_process->label.size += String_Chunk_Size;
      }

      process_cold_index += 1;
    }

    set_as_current_file(context, file_name);

    String8 file_name_str8 = string8_from_string_chunk_list(context->temp_arena, &file_name);
    os_file_write(file_name_str8, save_file_data);
  }
}

#if 1
function void write_save_file(Context *context, Arena *arena, String_Chunk_List file_name) {
  write_save_file_v1(context, arena, file_name);
}
#else
function void write_save_file(Context *context, Arena *arena, String_Chunk_List file_name) {
  Assert(!"TODO: Re-write this to use Cold_Process struct");
  // TODO: ensure that the Saves_Filepath directory exists before writing a file into it.
  os_set_current_directory(Saves_Filepath);

  // save-file sizing and cold-indexing
  U64 process_cold_index = 0;
  U64 string_cold_index = 0;
  for (Process *p = context->processes.first; p != 0; p = p->next) {
    p->cold_index = process_cold_index+1;
    process_cold_index += 1;
    for (String_Chunk *s = p->label.first; s != 0; s = s->next) {
      s->cold_index = string_cold_index+1;
      string_cold_index += 1;
    }
  }

  String8 save_file_data;
  save_file_data.size = Save_File_Size_V1(process_cold_index, string_cold_index);
  save_file_data.str = arena_push(arena, save_file_data.size);

  set_as_current_file(context, file_name);
  if (save_file_data.str) {
    Save_File_Header *header = (Save_File_Header *)save_file_data.str;
    header->magic_number = Save_File_Magic_Number;
    header->version = Save_File_Version;
    header->process_count = process_cold_index;
    header->string_count = string_cold_index;
    Cold_Process *first_cold_process = Save_File_Start_Of_Processes_V1(save_file_data.str);
    String_Chunk *first_cold_string = Save_File_Start_Of_Strings_V1(save_file_data.str, process_cold_index);

    // copy processes to save file
    process_cold_index = 0;
    string_cold_index = 0;
    for (Process *p = context->processes.first; p != 0; p = p->next) {
      Cold_Process *cold_process = first_cold_process + process_cold_index;

      cold_process->flags = p->flags;
      cold_process->position = p->position;
      /* cold_process->label = p->label; */

      /* cold_process->in = p->in; */
      /* cold_process->out = p->out; */

      cold_process->which_in = p->which_in;
      cold_process->which_out = p->which_out;

      /* *cold_process = *p; */

      if (p->in) {
        cold_process->in = p->in->cold_index;
      }
      if (p->out) {
        cold_process->out = p->out->cold_index;
      }

      if (p->label.first) {
        /* cold_process->label.first = PtrFromInt(p->label.first->cold_index); */
      }
      if (p->label.last) {
        /* cold_process->label.last = PtrFromInt(p->label.last->cold_index); */
      }

      for (String_Chunk *s = p->label.first; s != 0; s = s->next) {
        String_Chunk *cold_string = first_cold_string + string_cold_index;
        *cold_string = *s;
        if (s->next) {
          cold_string->next = PtrFromInt(s->next->cold_index);
        }

        string_cold_index += 1;
      }

      // zero out transient process-pointers
      /* cold_process->next = 0; */
      /* cold_process->next_active = 0; */
      /* cold_process->to_copied = 0; */

      process_cold_index += 1;
    }

    String8 file_name_str8 = string8_from_string_chunk_list(context->temp_arena, &file_name);
    os_file_write(file_name_str8, save_file_data);
  }

  os_set_current_directory(Build_Filepath);
}
#endif

function void open_file_and_replace_processes_v1(Context *context, String_Chunk_List file_name_list) {
  os_set_current_directory(Saves_Filepath);
  String8 file_name = string8_from_string_chunk_list(context->temp_arena, &file_name_list);
  String8 file_data = os_file_read(context->temp_arena, file_name);

  if (file_data.str && file_data.size > sizeof(Save_File_Header)) {
    Save_File_Header *header = (Save_File_Header *)file_data.str;

    if (header->magic_number == Save_File_Magic_Number) {
      if (header->version == 1) {
        Assert(!"TODO");
      } else {
        // TODO: Really, we should do some kind of switch here, to figure out the version and parse it accordingly.
        printf("[ Error ] We only handle version 1 of the file right now, but this header declares a version of %d.\n", header->version);
      }
    } else {
      printf("[ Error ] Expected save file magic number of 0x%x but got 0x%x instead.\n", Save_File_Magic_Number, header->magic_number);
    }
  }
}

#if 0
function void open_file_and_replace_processes_v1_OLD(Context *context, String_Chunk_List file_name_list) {
  os_set_current_directory(Saves_Filepath);
  String8 file_name = string8_from_string_chunk_list(context->temp_arena, &file_name_list);
  String8 file_data = os_file_read(context->temp_arena, file_name);


  if (file_data.str && file_data.size > sizeof(Save_File_Header)) {
    Save_File_Header *header = (Save_File_Header *)file_data.str;
    Assert(header->version == 1);
    Process *first_cold_process = Save_File_Start_Of_Processes(file_data.str);
    String_Chunk *first_cold_string = Save_File_Start_Of_Strings(file_data.str, header->process_count);

    if (header->magic_number == Save_File_Magic_Number) {
      U64 process_array_size = (U8 *)first_cold_string - (U8 *)first_cold_process;
      U64 string_array_size = (file_data.str+file_data.size) - (U8 *)first_cold_string;
      if (process_array_size == sizeof(Process) * header->process_count &&
          string_array_size == sizeof(String_Chunk) * header->string_count) {
        Process *previous_process = 0;
        clear_processes(context);

        // create new processes
        for (S32 i = 0; i < header->process_count; ++i) {
          Process cold_process = first_cold_process[i];
          Process *new_process = create_process(context);
          *new_process = cold_process;

          if (previous_process) {
            previous_process->next = new_process;
          }

          previous_process = new_process;
        }

        // @Speed
        for (Process *p = context->processes.first; p != 0; p = p->next) {
          // convert cold-indices to pointers
          for (Process *r = context->processes.first; r != 0; r = r->next) {
            if (p->in && r->cold_index && IntFromPtr(p->in) == r->cold_index) {
              p->in = r;
            }
            if (p->out && r->cold_index && IntFromPtr(p->out) == r->cold_index) {
              p->out = r;
            }
          }

          if ((p->label.first == 0 && p->label.last != 0) ||
              (p->label.first != 0 && p->label.last == 0)){
            B32 uhoh = 1;
          }
          // create string-chunks and copy from cold-strings
          U64 sc_index = IntFromPtr(p->label.first);
          String_Chunk *previous_sc = 0;
          for (;;) {
            if (sc_index == 0 || sc_index > (header->string_count)) {
              break;
            }
            String_Chunk *cold_string = first_cold_string + sc_index-1;
            String_Chunk *new_sc = create_string_chunk(context);
            *new_sc = *cold_string;
            if (previous_sc) {
              previous_sc->next = new_sc;
            } else {
              p->label.first = new_sc;
            }
            sc_index = IntFromPtr(cold_string->next);
            previous_sc = new_sc;
          }
          p->label.last = previous_sc;
          if ((p->label.first == 0 && p->label.last != 0) ||
              (p->label.first != 0 && p->label.last == 0)){
            B32 uhoh = 1;
          }
        }
      } else {
        printf("[ Error ] Mismatched size of process array in save-file. ");
        printf("Header says %d processes for a size of %lu, but the actual size is %llu.\n", header->process_count, sizeof(Process) * header->process_count, process_array_size);
        printf("Header says %d strings for a size of %lu, but the actual size is %llu.\n", header->string_count, sizeof(String_Chunk) * header->string_count, string_array_size);
      }
    }
  }

  os_set_current_directory(Build_Filepath);
}
#endif

function void open_file_and_replace_processes(Context *context, String_Chunk_List file_name_list) {
  open_file_and_replace_processes_v1(context, file_name_list);
}
