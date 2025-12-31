// Copy and edit a version of this file as "proc/config/custom_keybinds.h".

// TODO: Delete Define_Keybind_ and just change the args in Define_Keybind
#define Define_Keybind_(bind_value, keybind_name, k, m, c, d)\
  Define_Keybind(bind_value, keybind_name, k, m, c, d)


Define_Keybind_(CreateProcess, Proc_Keybind_Def_Custom,
               Key_Kind_Mouse0, Modifier_Key_Shift,
               Ui_Constraint_NoHotProcess,
               "CUSTOM: Create a new process.");


Define_Keybind_(CreateProcess, Proc_Keybind_Def_Custom,
                KEY_L, Modifier_Key_Shift,
                Ui_Constraint_NoHotProcess,
                "CUSTOM: Create a new process.");
