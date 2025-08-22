// NOTE: Ideally we would just use a single core/base codebase like Mr4th's, but that doesn't support non-Windows OSs at the moment. One of these days we should bite the bullet and implement some stuff for Linux/Mac.



#define function static
#define global_variable static
#define Kilobytes(n) (1024 * (n))
#define Megabytes(n) (1024 * Kilobytes(n))
#define Gigabytes(n) (1024 * Megabytes(n))

#define    Set_Flag(flags, flag) ((flags) |=  (flag))
#define  Unset_Flag(flags, flag) ((flags) &= ~(flag))
#define    Get_Flag(flags, flag) ((flags) &   (flag))
#define Toggle_Flag(flags, flag) ((flags) ^=  (flag))




////////////////
//  NOT REAL UTF-8
//    TODO: implement utf-8 editing
////////////////
#define Editable_Character_Xlist\
  X('a') X('b') X('c') X('d') X('e') X('f') X('g') X('h') X('i') X('j')\
  X('k') X('l') X('m') X('n') X('o') X('p') X('q') X('r') X('s') X('t')\
  X('u') X('v') X('w') X('x') X('y') X('z')\
  X('A') X('B') X('C') X('D') X('E') X('F') X('G') X('H') X('I') X('J') \
  X('K') X('L') X('M') X('N') X('O') X('P') X('Q') X('R') X('S') X('T')\
  X('U') X('V') X('W') X('X') X('Y') X('Z')\
  X('0') X('1') X('2') X('3') X('4') X('5') X('6') X('7') X('8') X('9')\
  X('`') X('~') X('!') X('@') X('#') X('$') X('%') X('^') X('&') X('*')\
  X('(') X(')') X('-') X('_') X('=') X('+') X(';') X(':') X(' ')\
  X('\'') X('"') X(',') X('<') X('.') X('>') X('/') X('?') X('\\') X('|')

U8 is_editable_char_lookup[256] = {
#define X(c)\
  [c] = 1,
  Editable_Character_Xlist
#undef X
};

#define Is_Ascii_Range(c) ((c) >= 0 && (c)<=255)
#define Is_Editable_Char(c)  (Is_Ascii_Range(c)?is_editable_char_lookup[c&0xff]:0)


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
