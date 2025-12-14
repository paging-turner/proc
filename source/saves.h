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
  U64 string_offset;
  U64 string_size;

  Cold_Process_Id in;
  Cold_Process_Id out;

  U32 which_in;
  U32 which_out;

  U8 unused_bytes[8];
} Cold_Process;

// "Freeze" Cold_Process
StaticAssert(sizeof(Cold_Process) == 64, cold_process_should_be_64_bytes);
Freeze_Member(Cold_Process, flags        ,  0);
Freeze_Member(Cold_Process, position     ,  4);
Freeze_Member(Cold_Process, string_offset, 16);
Freeze_Member(Cold_Process, string_size  , 24);
Freeze_Member(Cold_Process, in           , 32);
Freeze_Member(Cold_Process, out          , 40);
Freeze_Member(Cold_Process, which_in     , 48);
Freeze_Member(Cold_Process, which_out    , 52);
Freeze_Member(Cold_Process, unused_bytes , 56);


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
      cold_process->string_offset = string_cold_offset;

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
        cold_process->string_size += String_Chunk_Size;
      }

      process_cold_index += 1;
    }

    set_as_current_file(context, file_name);

    String8 file_name_str8 = string8_from_string_chunk_list(context->temp_arena, &file_name);
    os_file_write(file_name_str8, save_file_data);
  }
}

function void write_save_file(Context *context, Arena *arena, String_Chunk_List file_name) {
  write_save_file_v1(context, arena, file_name);
}

function void open_file_and_replace_processes_v1(Context *context, String_Chunk_List file_name_list) {
  os_set_current_directory(Saves_Filepath);
  String8 file_name = string8_from_string_chunk_list(context->temp_arena, &file_name_list);
  String8 file_data = os_file_read(context->temp_arena, file_name);

  if (file_data.str && file_data.size > sizeof(Save_File_Header)) {
    Save_File_Header *header = (Save_File_Header *)file_data.str;

    if (header->magic_number == Save_File_Magic_Number) {
      if (header->version == 1) {

        Cold_Process *first_cold_process = Save_File_Start_Of_Processes_V1(file_data.str);
        U8 *start_of_cold_string = Save_File_Start_Of_Strings_V1(file_data.str, header->process_count);
        U64 string_offset = 0;

        clear_processes(context);

        Process **process_lookup = arena_push(context->temp_arena, header->process_count*sizeof(Process *));

        if (process_lookup) {
          for (U64 i = 0; i < header->process_count; ++i) {
            Process *new_process = create_process(context);
            process_lookup[i] = new_process;
            Cold_Process *cold_process = first_cold_process + i;
            new_process->flags = cold_process->flags;
            new_process->position = cold_process->position;
            String8 cold_string = (String8){start_of_cold_string + string_offset, cold_process->string_size};
            new_process->label = string_chunk_list_from_string8(context, cold_string);
            new_process->cold_index = i;
            new_process->which_in = cold_process->which_in;
            new_process->which_out = cold_process->which_out;
          }

          for (U64 i = 0; i < header->process_count; ++i) {
            Cold_Process *cold_process = first_cold_process + i;

            if (cold_process->in) {
              process_lookup[i]->in = process_lookup[cold_process->in];
            }
            if (cold_process->out) {
              process_lookup[i]->out = process_lookup[cold_process->out];
            }
          }
        } else {
          printf("[ Error ] Pushing the process_lookup while loading proc file\n");
        }
      } else {
        // TODO: Really, we should do some kind of switch here, to figure out the version and parse it accordingly.
        printf("[ Error ] We only handle version 1 of the file right now, but this header declares a version of %d.\n", header->version);
      }
    } else {
      printf("[ Error ] Expected save file magic number of 0x%x but got 0x%x instead.\n", Save_File_Magic_Number, header->magic_number);
    }
  }
}


function void open_file_and_replace_processes(Context *context, String_Chunk_List file_name_list) {
  open_file_and_replace_processes_v1(context, file_name_list);
}
