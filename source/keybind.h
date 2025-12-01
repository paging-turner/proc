//////////////////////////////////
// Keybind Declarations
//////////////////////////////////

#define Keybind_Xlist\
  X(Bound, Key_Kind_Mouse0, 0,\
    Ui_Constraint_NoHotProcess|Ui_Constraint_ExitOnKeyup,\
    "Select multiple processes by drawing a rectangle with your mouse.")\
\
  X(Pan, Key_Kind_Mouse1, 0,\
    Ui_Constraint_ExitOnKeyup,\
    "Slide your field of view by moving your mouse.")\
\
  X(ZoomIn, Key_Kind_MouseWheelUp, 0,\
    Ui_Constraint_ActionNotOccured,\
    "Zoom your field of view in to make objects appear closer.")\
\
  X(ZoomOut, Key_Kind_MouseWheelDown, 0,\
    Ui_Constraint_ActionNotOccured,\
    "Zoom your field of view out to make objects appear further.")\
\
  X(SelectSingleProcess, Key_Kind_Mouse0, 0,\
    Ui_Constraint_HoverProcess|Ui_Constraint_ExitOnKeyup,\
    "Select a single process.")\
\
  X(SelectAnotherProcess, Key_Kind_Mouse0, Modifier_Key_Control,\
    Ui_Constraint_HoverProcess,\
    "Add a process to the selected processes.")\
\
  X(CancelSelection, Key_Kind_Mouse0, 0,\
    Ui_Constraint_NoHotProcess,\
    "Clear out the selected processes.")\
\
  X(CreateProcess, Key_Kind_Mouse0, Modifier_Key_Control,\
    Ui_Constraint_NoHotProcess,\
    "Create a new process.")\
\
  X(DeleteProcess, KEY_D, Modifier_Key_Control, 0,\
    "Delete the selected processes.")\
\
  X(CycleProcessDisplay, KEY_TAB, 0, 0,\
    "Cycle through special displays for selected processes.")\
\
  X(ToggleDisplayMode, KEY_M, Modifier_Key_Control, 0,\
    "Toggle between 'classic' and 'rounded' display modes.")\
\
  X(CopyProcess, KEY_C, Modifier_Key_Control, 0,\
    "Copy selected processes.")\
\
  X(PasteProcess, KEY_V, Modifier_Key_Control, 0,\
    "Paste copied processes, centered at the mouse.")\
\
  X(ToggleTestRecording, KEY_T, Modifier_Key_Control, 0,\
    "Toggle on/off recording of user inputs for creating end-to-end tests.")


typedef enum {
  Ui_Feature__Null,
#define X(feature, ...)\
  Ui_Feature_##feature,
  Keybind_Xlist
#undef X
  Ui_Feature__Count,
} Ui_Feature;

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

typedef struct {
  Ui_Feature feature;
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


#define Key_Kind_From_String_Xlist\
  X(Space     , KEY_SPACE)\
  X(Escape    , KEY_ESCAPE)\
  X(Enter     , KEY_ENTER)\
  X(Tab       , KEY_TAB)\
  X(Backspace , KEY_BACKSPACE)\
  X(Insert    , KEY_INSERT)\
  X(Delete    , KEY_DELETE)



//////////////////////////////////
// Globals
//////////////////////////////////

global_variable Keybind global_keybind_lookup[Ui_Feature__Count];






//////////////////////////////////
// Function Declarations
//////////////////////////////////

function void load_keybinds(Context *context);















function Ui_Feature ui_feature_from_identifier(String8 identifier) {
  Ui_Feature feature = 0;

  for (S32 f = 1; f < Ui_Feature__Count; ++f) {
    Keybind keybind = global_keybind_lookup[f];
    if (str8_match(identifier, keybind.name, 0)) {
      feature = f;
      break;
    }
  }

  return feature;
}

function Modifier_Key modifier_key_from_identifier(String8 identifier) {
  Modifier_Key modifier = 0;

  if (str8_match(identifier, str8_lit("control"), StringMatchFlag_NoCase)) {
    modifier = Modifier_Key_Control;
  } else if (str8_match(identifier, str8_lit("shift"), StringMatchFlag_NoCase)) {
    modifier = Modifier_Key_Shift;
  } else if (str8_match(identifier, str8_lit("alt"), StringMatchFlag_NoCase)) {
    modifier = Modifier_Key_Alt;
  }

  return modifier;
}


function U32 key_kind_from_identifier(String8 identifier) {
  U32 key_kind = 0;

  if (identifier.size == 1) {
    key_kind = identifier.str[0];
  } else if (str8_match(identifier, str8_lit("space"), StringMatchFlag_NoCase)) {
    key_kind = KEY_SPACE;
  } else if (str8_match(identifier, str8_lit("escape"), StringMatchFlag_NoCase)) {
    key_kind = KEY_ESCAPE;
  } else if (str8_match(identifier, str8_lit("enter"), StringMatchFlag_NoCase)) {
    key_kind = KEY_ENTER;
  } else if (str8_match(identifier, str8_lit("tab"), StringMatchFlag_NoCase)) {
    key_kind = KEY_TAB;
  } else if (str8_match(identifier, str8_lit("backspace"), StringMatchFlag_NoCase)) {
    key_kind = KEY_BACKSPACE;
  } else if (str8_match(identifier, str8_lit("insert"), StringMatchFlag_NoCase)) {
    key_kind = KEY_INSERT;
  } else if (str8_match(identifier, str8_lit("delete"), StringMatchFlag_NoCase)) {
    key_kind = KEY_DELETE;
  }

  if (key_kind >= 'a' && key_kind <= 'z') {
    // turn to uppercase
    key_kind -= 32;
  }

  return key_kind;
}


function B32 char_is_identifier_char(U8 c) {
  B32 is_upper = c >= 'A' && c <= 'Z';
  B32 is_lower = c >= 'a' && c <= 'z';
  B32 is_special = (c == '_');
  B32 is_feature_char = is_upper || is_lower || is_special;

  return is_feature_char;
}


typedef enum {
  Keybind_Config_Tokenizer_State_Begin,
  Keybind_Config_Tokenizer_State_Identifier,
  Keybind_Config_Tokenizer_State_Comment,
  Keybind_Config_Tokenizer_State_Error,
} Keybind_Config_Tokenizer_State;


function Keybind_Config_Tokens tokenize_keybinds_config_file(Context *context) {
  Arena *ta = context->temp_arena;
  Keybind_Config_Tokens tokens = {0};

  // load custom keybinds from config file
  FILE *f = fopen((char *)Keybind_Config_Filepath.str, "rb");
  if (f) {
    // read file into temp arena
    fseek(f, 0, SEEK_END);
    S32 file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    U8 *file_str = arena_push_no_zero(context->temp_arena, file_size);
    fread(file_str, 1, file_size, f);

    Keybind_Config_Tokenizer_State state = 0;
    Keybind_Config_Token *new_token = 0;

#define S(name) Keybind_Config_Tokenizer_State_##name
    for (S32 i = 0; i < file_size; ++i) {
      switch(state) {
      case S(Begin): {
        // skip space
        while (str8_char_is_whitespace(file_str[i]) && i < file_size) {
          ++i;
        }
        if (i >= file_size || file_str[i] == 0) {
          break;
        }
        // parse next token
        if (file_str[i] == '#') {
          state = S(Comment);
        } else if (file_str[i] == '=') {
          new_token = push_struct(ta, Keybind_Config_Token);
          if (new_token) {
            SLLQueuePush(tokens.first, tokens.last, new_token);
            new_token->kind = Keybind_Config_Token_Kind_Equal;
          } else {
            state = S(Error);
          }
        } else if (file_str[i] == ';') {
          new_token = push_struct(ta, Keybind_Config_Token);
          if (new_token) {
            SLLQueuePush(tokens.first, tokens.last, new_token);
            new_token->kind = Keybind_Config_Token_Kind_Semicolon;
          } else {
            state = S(Error);
          }
        } else if (char_is_identifier_char(file_str[i])) {
          state = S(Identifier);
          new_token = push_struct(ta, Keybind_Config_Token);
          if (new_token) {
            SLLQueuePush(tokens.first, tokens.last, new_token);
            new_token->kind = Keybind_Config_Token_Kind_Identifier;
            new_token->name.str = file_str + i;
            new_token->name.size = 1;
          } else {
            state = S(Error);
          }
        } else {
          state = S(Error);
        }
      } break;
      case S(Identifier): {
        if (char_is_identifier_char(file_str[i])) {
          if (new_token) {
            new_token->name.size += 1;
          } else {
            state = S(Error);
          }
        } else {
          state = S(Begin);
          i -= 1;
        }
      } break;
      case S(Comment): {
        while (i < file_size) {
          if (file_str[i] == '\n') {
            state = S(Begin);
            break;
          } else {
            i += 1;
          }
        }
        state = S(Begin);
      } break;
      default: state = S(Error);
      }

      if (state == S(Error)) {
        tokens = (Keybind_Config_Tokens){0};
        break;
      }
    }
#undef S
  }

  return tokens;
}


typedef enum {
  Keybind_Config_Parse_State_Begin,
  Keybind_Config_Parse_State_Error,
  Keybind_Config_Parse_State_LineError,
  Keybind_Config_Parse_State_Equals,
  Keybind_Config_Parse_State_Keybind,
} Keybind_Config_Parse_State;

function void parse_keybind_config_tokens(Context *context, Keybind_Config_Tokens tokens) {
  Keybind_Config_Parse_State state = Keybind_Config_Parse_State_Begin;
  Ui_Feature feature = 0;
  Keybind keybind = {0};

#define S(name) Keybind_Config_Parse_State_##name
  for (Keybind_Config_Token *token = tokens.first; token != 0; token = token->next) {
    switch(state) {
    case S(Begin): {
      keybind = (Keybind){0};
      if (token->kind == Keybind_Config_Token_Kind_Identifier) {
        feature = ui_feature_from_identifier(token->name);
        if (feature) {
          keybind.feature = feature;
          state = S(Equals);
        } else {
          printf("[ Keybind Config Parse Error ] Unknown feature '");
          print_string8(token->name);
          printf("'\n");
          state = S(LineError);
        }
      } else {
        printf("[ Keybind Config Parse Error ] Expected a feature but got token of value '");
        print_string8(token->name);
        printf("'\n");
        state = S(LineError);
      }
    } break;
    case S(Equals): {
      if (token->kind == Keybind_Config_Token_Kind_Equal) {
        state = S(Keybind);
      } else {
        printf("[ Keybind Config Parse Error ] Expected an equals '=' after feature name, but got token with value '");
        print_string8(token->name);
        printf("'\n");
        state = S(Error);
      }
    } break;
    case S(Keybind): {
      Modifier_Key modifier = modifier_key_from_identifier(token->name);
      if (modifier) {
        Set_Flag(keybind.modifiers, modifier);
      } else if (token->kind == Keybind_Config_Token_Kind_Semicolon) {
        // overwrite keybinding
        global_keybind_lookup[keybind.feature].modifiers = keybind.modifiers;
        global_keybind_lookup[keybind.feature].key_kind = keybind.key_kind;
        state = S(Begin);
      } else {
        U32 key_kind = key_kind_from_identifier(token->name);

        if (key_kind) {
          if (keybind.key_kind) {
            Keybind ref_keybind = global_keybind_lookup[keybind.feature];
            printf("[ Keybind Config Parse Error ] Key-kind already defined. While defining feature '");
            print_string8(ref_keybind.name);
            printf("'\n");
            state = S(LineError);
          } else {
            keybind.key_kind = key_kind;
          }
        } else {
          printf("[ Keybind Config Parse Error ] Unknown key-kind '");
          print_string8(token->name);
          printf("'\n");
          state = S(LineError);
        }
      }
    } break;
    case S(LineError): {
      if (token->kind == Keybind_Config_Token_Kind_Semicolon) {
        state = S(Begin);
      }
    } break;
    default: state = S(Error);
    }

    if (state == S(Error)) {
      break;
    }
  }
#undef S
}



function void load_keybinds(Context *context) {
  // keybind setup
#define X(feature, key_kind, modifiers, constraint, description)        \
  global_keybind_lookup[Ui_Feature_##feature] = (Keybind){Ui_Feature_##feature, key_kind, modifiers, constraint, str8_lit(#feature), str8_lit(description)};
  Keybind_Xlist;
#undef X

  Keybind_Config_Tokens tokens = tokenize_keybinds_config_file(context);
  parse_keybind_config_tokens(context, tokens);
}
