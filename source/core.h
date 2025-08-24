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
