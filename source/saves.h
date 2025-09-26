/*
  The save file is just an array of process structs with a header.
  Once we convert labels/text from char-buffers to strings/string-lists, we will also need to store string data.

  If the Process struct is changed in a significant way, we will need to bump the version.

  Ideally, we would also create functions for loading old versions of files... here's hoping.
*/

#define Save_File_Magic_Number AsciiID4('p', 'r', 'o', 'c')
#define Save_File_Version 0

typedef struct {
  U32 magic_number;
  U32 version;
  U32 process_count;
} Save_File_Header;



#define Save_File_Size(process_count) (sizeof(Save_File_Header)+process_count*sizeof(Process))


//////////////////////////////////////
// Saves Functions
//////////////////////////////////////
function void write_save_file(Context *context, Arena *arena, String8 file_name);







function void write_save_file(Context *context, Arena *arena, String8 file_name) {
  // save-file sizing
  S32 process_count = 0;
  for (Process *p = context->processes.first; p != 0; p = p->next) {
    process_count += 1;
  }
  String8 save_file;
  save_file.size = Save_File_Size(process_count);
  save_file.str = arena_push(arena, save_file.size);

  if (save_file.str) {
    Save_File_Header *header = (Save_File_Header *)save_file.str;
    header->magic_number = Save_File_Magic_Number;
    header->version = Save_File_Version;
    header->process_count = process_count;
    Process *first_cold_process = (Process *)(header + 1);

    // assign cold indices
    U64 cold_index = 0;
    for (Process *p = context->processes.first; p != 0; p = p->next) {
      p->cold_index = cold_index;
      cold_index += 1;
    }

    // copy processes to save file
    cold_index = 0;
    for (Process *p = context->processes.first; p != 0; p = p->next) {
      Process *cold_process = first_cold_process + cold_index;
      *cold_process = *p;

      if (p->in) {
        cold_process->in = PtrFromInt(p->in->cold_index);
      }
      if (p->out) {
        cold_process->out = PtrFromInt(p->out->cold_index);
      }

      // zero out transient process-pointers
      cold_process->next = 0;
      cold_process->next_active = 0;
      cold_process->to_copied = 0;

      cold_index += 1;
    }

    os_file_write(file_name, save_file);
  }
}
