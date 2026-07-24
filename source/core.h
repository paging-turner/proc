#ifndef PROC_CORE_INCLUDE_H
# define PROC_CORE_INCLUDE_H
//
// NOTE: Ideally we would just use a single core/base codebase like Mr4th's, but that doesn't support non-Windows OSs at the moment. One of these days we should bite the bullet and implement some stuff for Linux/Mac.





#define function static
#define global_variable static
#define Kilobytes(n) (1024 * (n))
#define Megabytes(n) (1024 * Kilobytes(n))
#define Gigabytes(n) (1024 * Megabytes(n))

#define    Set_Flag(flags, flag) ((flags) |=  (flag))
#define  Unset_Flag(flags, flag) ((flags) &= ~(flag))
#define    Get_Flag(flags, flag) ((flags) &   (flag))
#define    Get_Flag_Bool(flags, flag) (Get_Flag((flags), (flag)) ? 1 : 0)
#define Toggle_Flag(flags, flag) ((flags) ^=  (flag))
#define Assign_Flag(flags, flag, bool)\
  ((bool)\
   ? Set_Flag((flags), (flag))\
   : Unset_Flag((flags), (flag)))

#define Zero_Struct(type) (type){0}

// Freeze_Member is used to ensure a struct is not changed. Used as an alarm for structs sensitive to internal changes, like ones that are serialized.
#define Freeze_Member(_struct, member, byte_offset)\
  StaticAssert(offsetof(_struct, member) == byte_offset, Freeze_##_struct##_##member)




//////////////////////////////////////
// Cycle detection
//////////////////////////////////////
#define Define_Cycle_Detector_Function(func_name, list_type, next)\
  static B32 func_name(list_type* head) {\
    list_type *slow = head;\
    list_type *fast = head;\
    while (slow && fast && fast->next) {\
      slow = slow->next;\
      fast = fast->next->next;\
      if (slow == fast) {\
        return 1;\
      }\
    }\
    return 0;\
  }




//////////////////////////////////////
// String Chunks
//////////////////////////////////////

#define String_Chunk_Size 8

typedef struct String_Chunk String_Chunk;
struct String_Chunk {
  String_Chunk *next;
  U64 cold_index;
  U8 str_array[String_Chunk_Size];
};

typedef struct String_Chunk_List String_Chunk_List;
struct String_Chunk_List {
  String_Chunk *first;
  String_Chunk *last;
  // TODO: Add in node_count and total_size so we don't have to calculate them on use of the list
  /* U64 node_count; */
  /* U64 total_size; */
};

Define_Cycle_Detector_Function(
  string_chunk_has_cycles,
  String_Chunk, next);


typedef struct {
  U64 size;
  U64 offset;
} Cold_String;



///////////////////////////////////////
// Function Declarations
///////////////////////////////////////
function B32 c_strings_equal(char *a, char *b);

function F32 which_side_of_line(Vector2 a, Vector2 b, Vector2 p);
function F32 which_side_of_bezier(Vector2 first_point, Vector2 second_point, Vector2 first_control, Vector2 second_control, Vector2 test_point);

function Vector2 get_bezier_point(Vector2 first_point, Vector2 second_point, Vector2 first_control, Vector2 second_control, F32 t);
function S32 create_bezier_triangle_fan(Vector2 first_point, Vector2 second_point, Vector2 first_control, Vector2 second_control, Vector2 *points, S32 max_points, S32 triangle_count);


///////////////////////////////////////
// These type definitions are getting pretty messy......
///////////////////////////////////////
#define Connected_Path_Point_Count_From_Point_Count(pc)\
 ((pc)>=3 ? (4 + 4*((pc)-2)) : 0)

typedef struct Connected_Path {
  U32 point_count;
  Vector2 *points;
} Connected_Path;



////////////////
//  NOT REAL UTF-8
//    TODO: implement utf-8 editing
////////////////
#define Ascii_Alpha_Xlist\
  X('A', 'a') X('B', 'b') X('C', 'c') X('D', 'd') X('E', 'e') X('F', 'f') X('G', 'g') X('H', 'h') X('I', 'i') X('J', 'j')\
  X('K', 'k') X('L', 'l') X('M', 'm') X('N', 'n') X('O', 'o') X('P', 'p') X('Q', 'q') X('R', 'r') X('S', 's') X('T', 't')\
  X('U', 'u') X('V', 'v') X('W', 'w') X('X', 'x') X('Y', 'y') X('Z', 'z')

#define Ascii_Symbol_Xlist\
  X('0', ')') X('1', '!') X('2', '@') X('3', '#') X('4', '$') X('5', '%') X('6', '^') X('7', '&') X('8', '*') X('9', '(')\
  X('`', '~') X('-', '_') X('=', '+') X('[', '{') X(']', '}') X('\\', '|')\
  X(',', '<') X('.', '>') X('/', '?') X(' ', ' ') X(';', ':') X('\'', '"')


static U8 ascii_char_lookup[256][2] = {
#define X(upper, lower)\
  [upper][0] = lower,\
  [upper][1] = upper,
  Ascii_Alpha_Xlist
#undef X

#define X(no_shift_char, shift_char)\
  [no_shift_char][0] = no_shift_char,\
  [no_shift_char][1] = shift_char,
  Ascii_Symbol_Xlist
#undef X
};

#define Is_Ascii_Range(c) ((c) >= 0 && (c)<=255)
#define Is_Editable_Char(c)  (Is_Ascii_Range(c)?is_editable_char_lookup[c&0xff]:0)



// HACK: We should instead just make functions that construct c-strings from string8 and then print the c-string!!!
function void print_string8(String8 string) {
  for (S32 i = 0; i < string.size; ++i) {
    printf("%c", string.str[i]);
  }
}

function void print_string8_list(String8List string_list) {
  String8Node *cycle_check = 0;
  for (String8Node *node = string_list.first; node != 0; node = node->next){
    print_string8(node->string);
  }
}

function void print_string_chunk_list(String_Chunk_List list) {
  for (String_Chunk *chunk = list.first; chunk != 0; chunk = chunk->next) {
    for (S32 i = 0; i < String_Chunk_Size; ++i) {
      char c = chunk->str_array[i];
      if (c == 0) {
        goto dblbreak;
      } else {
        printf("%c", c);
      }
    }
  }
  dblbreak:;
}



function U64 get_total_size_of_string_chunk_list(String_Chunk_List *scl) {
  Assert(!string_chunk_has_cycles(scl ? scl->first : 0));
  String_Chunk *sc = scl ? scl->first : 0;

  // @Speed
  // get the total size
  U64 total_size = 0;
  String_Chunk *cycle_check = 0;
  for (; sc != 0; sc = sc->next) {
    if (sc == scl->last) {
      S32 char_count = 1;
      for (;; ++char_count) {
        if (sc->str_array[char_count-1] == 0) {
          break;
        }
      }
      total_size += char_count;
    } else {
      total_size += String_Chunk_Size;
    }
  }

  return total_size;
}



function U8 *c_string_from_string_chunk_list(Arena *arena, String_Chunk_List *scl) {
  U64 total_size = get_total_size_of_string_chunk_list(scl);
  // add 1 byte for null-terminator
  total_size += 1;

  U8 *c_string;

  if (total_size) {
    c_string = arena_push_no_zero(arena, total_size);
    U64 c_string_index = 0;

    String_Chunk *cycle_check = 0;
    String_Chunk *sc = scl ? scl->first : 0;
    for (; sc != 0; sc = sc->next) {
      for (S32 i = 0; i < String_Chunk_Size; ++i) {
        if (c_string_index > total_size) {
          printf("[ Error ] creating c-string from string-chunk-list: c_string_index exceeds total size of string-chunk-list.\n");
          break;
        }

        c_string[c_string_index] = sc->str_array[i];

        if (c_string[c_string_index] == 0) {
          break;
        }

        c_string_index += 1;
      }
    }

    // null-terminate
    c_string[total_size-1] = 0;
  } else {
    c_string = (U8 *)"";
  }

  return c_string;
}


function String8 string8_from_string_chunk_list(Arena *arena, String_Chunk_List *scl) {
  U64 total_size = get_total_size_of_string_chunk_list(scl);
  // add 1 byte for null-terminator
  total_size += 1;

  String8 string8;
  string8.size = total_size;
  string8.str = arena_push_no_zero(arena, total_size);

  U64 string_index = 0;

  for (String_Chunk *sc = scl->first; sc != 0; sc = sc->next) {
    for (S32 i = 0; i < String_Chunk_Size; ++i) {
      string8.str[string_index] = sc->str_array[i];
      string_index += 1;
    }
  }

  return string8;
}



/*
  Figure out which "side" of the line "p" is at. This is useful for collision/bounds checking.

  "a" and "b" define the line and "p" is the point in question.

  Uses the 3d determinant to figure this out, setting z values to 1

     | ax ay az |
 det(| bx by bz |) = ax*by*pz + ay*bz*px + az*bx*py - az*by*px - ay*bx*pz - ax*bz*py
     | px py pz |

  Setting the z's to 1 we get:
      ax*by + ay*px + bx*py - by*px - ay*bx - ax*py

*/
function F32 which_side_of_line(Vector2 a, Vector2 b, Vector2 p) {
  F32 side = a.x*b.y + a.y*p.x + b.x*p.y - b.y*p.x - a.y*b.x - a.x*p.y;
  return side;
}




/*
  Figure out which "side" of a cubic bezier-line "p" is at. The implicit form of the bezier curve is taken from: https://www.mare.ee/indrek/misc/2d.pdf

  I'm out of my depth here, but we basically need to get an implicit form of the bezier curve so that we can plug in an x and y and get back a scalar. Positive is one side of the curve, and negative is the other.

  After testing this, you DO get good information about which side if you are close, but you also get some STRANGE results if you are far away from the curve. Basically, the curve is extended beyond the control points, so you will still need to do some other checks to use this functions effectively. Good luck!
*/
function F32 which_side_of_bezier(Vector2 first_point, Vector2 second_point, Vector2 first_control, Vector2 second_control, Vector2 test_point) {
  // NOTE: The source text defined p0-p3 with control points as p1 and p2. Renaming to make copy-paste easier.
  Vector2 p0 = first_point;
  Vector2 p1 = first_control;
  Vector2 p2 = second_control;
  Vector2 p3 = second_point;

  // NOTE: what are these coefficients?? :(
  F32 a3 = p3.x - p0.x - 3.0f *  p2.x + 3.0f *  p1.x;
  F32 a2 = -6.0f * p1.x + 3.0f *  p0.x + 3.0f *  p2.x;
  F32 a1 = -3.0f * p0.x + 3.0f *  p1.x;
  F32 a0 = p0.x;
  F32 b3 = p3.y - p0.y - 3.0f *  p2.y + 3.0f *  p1.y;
  F32 b2 = -6.0f * p1.y + 3.0f *  p0.y + 3.0f *  p2.y;
  F32 b1 = -3.0f * p0.y + 3.0f *  p1.y;
  F32 b0 = p0.y;

  // NOTE: Sometimes you just need to trust random mathemeticians on the internet........
  F32 v_xxx = b3 *b3 *b3;
  F32 v_xxy = -3.0f*a3 *b3 *b3;
  F32 v_xyy = 3.0f* b3 * a3 * a3;
  F32 v_yyy = -a3 * a3 * a3;
  F32 v_xx = -3.0f*a3 *b1*b2 *b3 + a1 *b2*b3 *b3 - a2 *b3 *b2 *b2
    + 2.0f* a2 *b1 *b3*b3 + 3.0f* a3 *b0*b3 *b3 + a3 *b2*b2 *b2 - 3.0f* a0 *b3 *b3 *b3;
  F32 v_xy = a1 * a3 *b2*b3 - a2 * a3 *b1*b3 - 6.0f* b0 *b3* a3 * a3
    - 3.0f* a1 * a2 *b3*b3 - 2.0f* a2 * a3 *b2 *b2 + 2.0f* b2*b3 * a2 * a2
    + 3.0f* b1*b2 * a3 * a3 + 6.0f* a0 * a3 *b3 *b3;
  F32 v_yy = 3.0f* a1 * a2 * a3 *b3 + a3 *b2* a2 * a2 - a2 *b1* a3 * a3
    - 3.0f* a0 *b3 * a3 * a3 - 2.0f* a1 *b2* a3 * a3 - b3 * a2 * a2 * a2 + 3.0f* b0* a3 * a3 * a3;
  F32 v_x = a2 * a3 *b0*b1 *b3 - a1 * a2 *b1 *b2*b3 - a1 * a3 *b0*b2 *b3
    + 6.0f* a0 * a3 *b1*b2 *b3 + b1 * a1 * a1 *b3*b3 + b3* a2 * a2 *b1 *b1
    + 3.0f* b3* a3 * a3 *b0 *b0 + a1 * a3 *b1 *b2*b2 - a2 * a3 *b2*b1 *b1
    - 6.0f* a0 * a3 *b0*b3 *b3 - 4.0f* a0 * a2 *b1 *b3 *b3 - 3.0f* b0 *b1*b2 * a3 * a3
    - 2.0f* a0 * a1 *b2*b3 *b3 - 2.0f* a1 * a3 *b3 *b1 *b1 - 2.0f* b0 *b2*b3 * a2 * a2
    + 2.0f* a0 * a2 *b3*b2 *b2 + 2.0f* a2 * a3 *b0 *b2 *b2 + 3.0f* a1 * a2 *b0 *b3*b3
    + a3 * a3 *b1 *b1*b1 + 3.0f* a0 * a0 *b3 *b3*b3 - 2.0f* a0 * a3 *b2 *b2 *b2;
  F32 v_y = a0 * a2 * a3 *b1 *b3 + a1 * a2 * a3 *b1*b2 - a0 * a1 * a3 *b2 *b3
    - 6.0f* a1 * a2 * a3 *b0 *b3 - a1 * a1 * a1 *b3*b3 - 3.0f* a3 * a3 * a3 *b0 *b0
    - a1 * a3 * a3 *b1*b1 - a3 * a1 * a1 *b2 *b2 - 3.0f* a3 * a0 * a0 *b3 *b3
    + a2 *b2*b3 * a1 * a1 - a1 *b1 *b3* a2 * a2 - 3.0f* a0 *b1 *b2* a3 * a3
    - 2.0f* a0 *b2 *b3* a2 * a2 - 2.0f* a3 *b0 *b2 * a2 * a2 + 2.0f* a0 * a2 * a3 *b2*b2
    + 2.0f* a2 *b0 *b1* a3 * a3 + 2.0f* a3 *b1 *b3 * a1 * a1 + 3.0f* a0 * a1 * a2 *b3*b3
    + 4.0f* a1 *b0 *b2* a3 * a3 + 6.0f* a0 *b0 *b3 * a3 * a3 + 2.0f* b0 *b3* a2 * a2 * a2;
  F32 v_0 = a0 * a1 * a2 *b1 *b2*b3 + a0 * a1 * a3 *b0 *b2*b3 - a0 * a2 * a3 *b0 *b1 *b3
    - a1 * a2 * a3 *b0*b1 *b2 + b0 * a1 * a1 * a1 *b3 *b3 - b3 * a2 * a2 * a2 *b0 *b0
    + a1 *b0* a3 * a3 *b1 *b1 + a1 *b2* a0 * a0 *b3 *b3 + a3 *b0* a1 * a1 *b2 *b2
    + a3 *b2* a2 * a2 *b0 *b0 - a0 *b1* a1 * a1 *b3 *b3 - a0 *b3* a2 * a2 *b1 *b1
    - a2 *b1* a3 * a3 *b0 *b0 - a2 *b3* a0 * a0 *b2 *b2 - 3.0f* a0 *b3 * a3 * a3 *b0 *b0
    - 2.0f* a1 *b2 * a3 * a3 *b0*b0 + 2.0f* a2 *b1 * a0 * a0 *b3*b3
    + 3.0f* a3 *b0 * a0 * a0 *b3*b3 + a0 * a2 * a3 *b2 *b1*b1 + a1 *b0 *b1*b3 * a2 * a2
    - a0 * a1 * a3 *b1*b2 *b2 - a2 *b0*b2 *b3* a1 * a1 - 3.0f* a0 * a1 * a2 *b0 *b3 *b3
    - 3.0f* a3 *b1 *b2*b3 * a0 * a0 - 2.0f* a0 * a2 * a3 *b0 *b2*b2
    - 2.0f* a3 *b0 *b1*b3 * a1 * a1 + 2.0f* a0 * a1 * a3 *b3 *b1*b1
    + 2.0f* a0 *b0 *b2*b3 * a2 * a2 + 3.0f* a0 *b0 *b1 *b2 * a3 * a3
    + 3.0f* a1 * a2 * a3 *b3 *b0*b0 + a3 * a3 * a3 *b0 *b0*b0 - a0 * a0 * a0 *b3 *b3 *b3
    + a3 * a0 * a0 *b2*b2 *b2 - a0 * a3 * a3 *b1*b1 *b1;
  // NOTE: Really should study up on this stuff and give better annotations...
  F32 x = test_point.x;
  F32 y = test_point.y;
  F32 xx = x*x;
  F32 yy = y*y;
  F32 xxx = x*x*x;
  F32 yyy = y*y*y;

  // NOTE: Have faith
  F32 side = v_xxx*xxx + v_xxy*xx*y + v_xyy*x*yy + v_yyy*yyy + v_xx*xx + v_xy*x*y + v_yy*yy + v_x*x + v_y*y + v_0;

  return side;
}



function Vector2 get_bezier_point(Vector2 first_point, Vector2 second_point, Vector2 first_control, Vector2 second_control, F32 t) {
  Vector2 result = (Vector2){0};

  if (t >= 0.0f && t <= 1.0f) {
    F32 it = 1.0f - t;
    Vector2 a = Vector2Scale(first_point, it*it*it);
    Vector2 b = Vector2Scale(first_control, 3.0f*it*it*t);
    Vector2 c = Vector2Scale(second_control, 3.0f*it*t*t);
    Vector2 d = Vector2Scale(second_point, t*t*t);

    result = Vector2Add(a, Vector2Add(b, Vector2Add(c, d)));
  }

  return result;
}



/*
  This assumes that the bezier is convex, otherwise the triangles might get wonky.
*/
function S32 create_bezier_triangle_fan(
  Vector2 first_point,
  Vector2 second_point,
  Vector2 first_control,
  Vector2 second_control,
  Vector2 *points,
  S32 max_points,
  S32 triangle_count
  ) {
  S32 point_count = 0;
  S32 needed_point_count = triangle_count + 2;

  if (needed_point_count <= max_points) {
    point_count = needed_point_count;
    points[point_count-1] = first_point;
    points[0] = second_point;
    for (S32 i = 1; i < point_count-1; ++i) {
      F32 amount = (point_count-1) - i;
      F32 t = (F32)amount / (F32)(point_count-1);
      Vector2 p = get_bezier_point(first_point, second_point, first_control, second_control, t);
      points[i] = p;
    }
  }

  return point_count;
}



function Connected_Path get_connected_path(
  Arena *arena,
  Vector2 *points,
  U32 point_count,
  F32 thickness,
  B32 closed
  ) {
  Connected_Path result = (Connected_Path){0};
  F32 half_thickness = 0.5f * thickness;

  if (closed) {
    point_count += 2;
  }

  if (point_count >= 3) {
    result.point_count = Connected_Path_Point_Count_From_Point_Count(point_count);

    if ((result.points = push_array(arena, Vector2, result.point_count))) {
      U32 iter_count = point_count - 2;

      for (U32 i = 0; i < iter_count; ++i) {
        U32 ri = 4*i;

        Vector2 a = points[i];
        Vector2 b;
        Vector2 c;
        if (closed) {
          if (i == iter_count-2) {
            b = points[i+1];
            c = points[0];
          }
          else if (i == iter_count-1) {
            b = points[0];
            c = points[1];
          }
          else {
            b = points[i+1];
            c = points[i+2];
          }
        }
        else {
          b = points[i+1];
          c = points[i+2];
        }

        Vector2 a_to_b = Vector2Normalize(Vector2Subtract(b, a));
        Vector2 b_to_c = Vector2Normalize(Vector2Subtract(c, b));

        Vector2 a_to_b_perp = (Vector2){-a_to_b.y, a_to_b.x};
        Vector2 b_to_c_perp = (Vector2){-b_to_c.y, b_to_c.x};
        Vector2 c_to_b_perp = (Vector2){b_to_c.y, -b_to_c.x};

        Vector2  a0 = Vector2Add(a, Vector2Scale(a_to_b_perp, -half_thickness));
        Vector2  a1 = Vector2Add(a, Vector2Scale(a_to_b_perp,  half_thickness));
        Vector2 ba0 = Vector2Add(b, Vector2Scale(a_to_b_perp, -half_thickness));
        Vector2 ba1 = Vector2Add(b, Vector2Scale(a_to_b_perp,  half_thickness));
        Vector2 bc0 = Vector2Add(b, Vector2Scale(b_to_c_perp, -half_thickness));
        Vector2 bc1 = Vector2Add(b, Vector2Scale(b_to_c_perp,  half_thickness));
        Vector2  c0 = Vector2Add(c, Vector2Scale(c_to_b_perp,  half_thickness));
        Vector2  c1 = Vector2Add(c, Vector2Scale(c_to_b_perp, -half_thickness));

        F32 a_to_bc0_dist_sq = Vector2DistanceSqr(bc0, a);
        F32 a_to_bc1_dist_sq = Vector2DistanceSqr(bc1, a);

        // initial 2 points
        if (i == 0) {
          result.points[0] = a0;
          result.points[1] = a1;
        }

        if (a_to_bc0_dist_sq < a_to_bc1_dist_sq) {
          Vector2 collision_point = (Vector2){0};
          if (!CheckCollisionLines(a0, ba0, bc0, c0, &collision_point)) {
            // fallback
            collision_point = a0;
          }

          result.points[ri+2] =
            Vector2Add(a0, Vector2Scale(Vector2Subtract(collision_point, a0), 0.5f));
          result.points[ri+3] = ba1;
          result.points[ri+4] = collision_point;
          result.points[ri+5] = bc1;
        }
        else {
          Vector2 collision_point = (Vector2){0};
          if (!CheckCollisionLines(a1, ba1, bc1, c1, &collision_point)) {
            // fallback
            collision_point = c1;
          }

          result.points[ri+2] = ba0;
          result.points[ri+3] = collision_point;
          result.points[ri+4] = bc0;
          result.points[ri+5] =
            Vector2Add(collision_point, Vector2Scale(Vector2Subtract(c1, collision_point), 0.5f));
        }

        // final 2 points
        if (i == iter_count-1) {
          result.points[result.point_count-2] = c0;
          result.points[result.point_count-1] = c1;
        }
      }
    }
  }

  return result;
}



function B32 arena_has_space_for(Arena *arena, U64 size) {
  Arena *current = arena->current;
  B32 has_space = 0;

  U64 result_pos = AlignUpPow2(current->chunk_pos, arena->alignment);
  U64 next_chunk_pos = result_pos + size;
  if (next_chunk_pos <= current->chunk_cap){
    has_space = 1;
  }

  return has_space;
}


#endif // PROC_CORE_INCLUDE_H
