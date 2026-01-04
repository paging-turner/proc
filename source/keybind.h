//////////////////////////////////
// Keybind Declarations
//////////////////////////////////

typedef enum {
  Ui_Constraint__Null            = 0,
  Ui_Constraint_HoverProcess     = (1 << 1),
  Ui_Constraint_NoHotProcess     = (1 << 2),
  Ui_Constraint_ExitOnKeyup      = (1 << 3),
  Ui_Constraint_ActionNotOccured = (1 << 4),
} Ui_Constraint;

// NOTE: These enum values start with values higher than raylib's highest KEY_* value, which is in the 300s
typedef enum {
  Key_Kind_Mouse0 = 1000,
  Key_Kind_Mouse1,
  Key_Kind_MouseWheelUp,
  Key_Kind_MouseWheelDown,
} Key_Kind;

typedef enum {
  Modifier_Key__Null    = 0,
  Modifier_Key_Control  = (1 << 0),
  Modifier_Key_Shift    = (1 << 2),
  Modifier_Key_Alt      = (1 << 3),
} Modifier_Key;


typedef struct Keybind {
  U32 bind;
  U32 key_kind; // Uses raylib's KEY_* enum and some special ones for mouse-keys
  U32 modifiers;
  Ui_Constraint constraint;
  String8 name;
  String8 description;
} Keybind;

typedef enum {
  Keybind_Result__Null,
  Keybind_Result_Enter,
  Keybind_Result_Exit,
} Keybind_Result;

typedef enum {
  Keybind_Config_Token_Kind__Null,
  Keybind_Config_Token_Kind_Identifier,
  Keybind_Config_Token_Kind_Equal,
  Keybind_Config_Token_Kind_Semicolon,
} Keybind_Config_Token_Kind;

typedef struct Keybind_Config_Token Keybind_Config_Token;
struct Keybind_Config_Token {
  Keybind_Config_Token_Kind kind;
  String8 name;
  Keybind_Config_Token *next;
};

typedef struct {
  Keybind_Config_Token *first;
  Keybind_Config_Token *last;
} Keybind_Config_Tokens;




#define SYMBOL_SET_DEFINE keybind
#define keybind_Type      Keybind
#define keybind_section   "_prckbnd"
#define keybind_ID(N)    SymbolID(keybind, N)
#define keybind_RAW(N)   SymbolRaw(keybind, N)
#define keybind_DECL(N)  SymbolDeclare(keybind, N)
#define keybind_REF(N)   SymbolMetadata(keybind, N)
#include "../libraries/mr4th/src/mr4th_symbol_set.define.h"


typedef enum {
  Proc_Keybind_Def_Default,
  Proc_Keybind_Def_Custom,
} Proc_Keybind_Def_Kind;


// TODO: Gather all keybinds in a single before-main, that way users can override/customize keybinds.
#define Define_Keybind(keybind_name, bind_value, k, m, c, d)\
  static keybind_DECL(keybind_name);\
  MR4TH_BEFORE_MAIN(proc_keybind_##bind_value##_##keybind_name){\
    keybind_Type *keybind = keybind_REF(keybind_name);\
    B32 both_zero = (U32)(bind_value) == 0 && keybind->bind == 0;\
    B32 stronger_bind = (U32)(bind_value) >= keybind->bind;\
    if (both_zero || stronger_bind) {\
      keybind->bind = (bind_value);\
      keybind->key_kind = (k);\
      keybind->modifiers = (m);\
      keybind->constraint = (c);\
      keybind->name = str8_lit(Stringify(keybind_name));\
      keybind->description = str8_lit(d);\
    }\
  }




Define_Keybind(Bound,
               Proc_Keybind_Def_Default,
               Key_Kind_Mouse0, 0,
               Ui_Constraint_NoHotProcess|Ui_Constraint_ExitOnKeyup,
               "Select multiple processes by drawing a rectangle with your mouse.");

Define_Keybind(Pan,
               Proc_Keybind_Def_Default,
               Key_Kind_Mouse1, 0,
               Ui_Constraint_ExitOnKeyup,
               "Slide your field of view by moving your mouse.");

Define_Keybind(ZoomIn,
               Proc_Keybind_Def_Default,
               Key_Kind_MouseWheelUp, 0,
               Ui_Constraint_ActionNotOccured,
               "Zoom your field of view in to make objects appear closer.");

Define_Keybind(ZoomOut,
               Proc_Keybind_Def_Default,
               Key_Kind_MouseWheelDown, 0,
               Ui_Constraint_ActionNotOccured,
               "Zoom your field of view out to make objects appear further.");

Define_Keybind(SelectSingleProcess,
               Proc_Keybind_Def_Default,
               Key_Kind_Mouse0, 0,
               Ui_Constraint_HoverProcess|Ui_Constraint_ExitOnKeyup,
               "Select a single process.");

Define_Keybind(SelectAnotherProcess,
               Proc_Keybind_Def_Default,
               Key_Kind_Mouse0, Modifier_Key_Control,
               Ui_Constraint_HoverProcess,
               "Add a process to the selected processes.");

Define_Keybind(CancelSelection,
               Proc_Keybind_Def_Default,
               Key_Kind_Mouse0, 0,
               Ui_Constraint_NoHotProcess,
               "Clear out the selected processes.");

Define_Keybind(CreateProcess,
               Proc_Keybind_Def_Default,
               Key_Kind_Mouse0, Modifier_Key_Control,
               Ui_Constraint_NoHotProcess,
               "Create a new process.");

Define_Keybind(DeleteProcess,
               Proc_Keybind_Def_Default,
               KEY_D, Modifier_Key_Control, 0,
               "Delete the selected processes.");

Define_Keybind(CycleProcessDisplay,
               Proc_Keybind_Def_Default,
               KEY_TAB, 0, 0,
               "Cycle through special displays for selected processes.");

Define_Keybind(ToggleDisplayMode,
               Proc_Keybind_Def_Default,
               KEY_M, Modifier_Key_Control, 0,
               "Toggle between 'classic' and 'rounded' display modes.");

Define_Keybind(CopyProcess,
               Proc_Keybind_Def_Default,
               KEY_C, Modifier_Key_Control, 0,
               "Copy selected processes.");

Define_Keybind(PasteProcess,
               Proc_Keybind_Def_Default,
               KEY_V, Modifier_Key_Control, 0,
               "Paste copied processes, centered at the mouse.");

Define_Keybind(ToggleTestRecording,
               Proc_Keybind_Def_Default,
               KEY_T, Modifier_Key_Control, 0,
               "Toggle on/off recording of user inputs for creating end-to-end tests.");


Define_Keybind(Undo,
               Proc_Keybind_Def_Default,
               KEY_Z, Modifier_Key_Control, 0,
               "Performs undo on the proc-trie.");

Define_Keybind(Redo,
               Proc_Keybind_Def_Default,
               KEY_Z, Modifier_Key_Control|Modifier_Key_Shift, 0,
               "Performs redo on the proc-trie.");
