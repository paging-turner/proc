////////////////////////////////////////////////
////////////////////////////////////////////////
///////////   BASE IMPLEMENTATION   ////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

////////////////////////////////
// Functions: Numerical/Math

// infinity

MR4TH_SYMBOL B32
is_inf_or_nan_F32(F32 x){
  U32 u = *(U32*)&x;
  B32 result = ((u&0x7f800000) == 0x7f800000);
  return(result);
}

MR4TH_SYMBOL B32
is_inf_or_nan_F64(F64 x){
  U64 u = *(U64*)&x;
  B64 result = ((u&0x7ff0000000000000) == 0x7ff0000000000000);
  return(result);
}

// float signs, rounding, and modulus

MR4TH_SYMBOL F32
abs_F32(F32 x){
  union{ F32 f; U32 u; } r;
  r.f = x;
  r.u &= 0x7fffffff;
  return(r.f);
}

MR4TH_SYMBOL F64
abs_F64(F64 x){
  union{ F64 f; U64 u; } r;
  r.f = x;
  r.u &= 0x7fffffffffffffff;
  return(r.f);
}

MR4TH_SYMBOL F32
sign_F32(F32 x){
  union{ F32 f; U32 u; } r;
  r.f = x;
  F32 result = (r.u&0x80000000)?-1.f:1.f;
  return(result);
}

MR4TH_SYMBOL F64
sign_F64(F64 x){
  union{ F64 f; U32 u; } r;
  r.f = x;
  F64 result = (r.u&0x8000000000000000)?-1.:1.;
  return(result);
}

MR4TH_SYMBOL F32
trunc_F32(F32 x){
  // TODO(allen): does this always work?
  //              can we do better?
  return((F32)(S32)x);
}

MR4TH_SYMBOL F32
floor_F32(F32 x){
  F32 r = (F32)((S32)x);
  if (r > x){
    r -= 1.f;
  }
  return(r);
}

MR4TH_SYMBOL F32
ceil_F32(F32 x){
  F32 r = (F32)((S32)x);
  if (r < x){
    r += 1.f;
  }
  return(r);
}

MR4TH_SYMBOL F32
mod_F32(F32 x, F32 m){
  Assert(m > 0.f);
  //                               +   |         -
  F32 q = (x/m);       //        (x/m) |       (x/m)
  S32 q_s32 = (S32)q;  //   floor(x/m) |   ceil(x/m)
  F32 r = m*(F32)q_s32;// m*floor(x/m) | m*ceil(x/m)
  //                        <= x       |   >= x
  F32 d = x - r;
  if (d < 0){
    d += m;
  }
  return(d);
}

MR4TH_SYMBOL F32
frac_F32(F32 x){
  //                           +  |      -
  F32 q = (x);        //      (x) |     (x)
  S32 q_s32 = (S32)q; // floor(x) | ceil(x)
  F32 r = (F32)q_s32; // floor(x) | ceil(x)
  //                       <= x   |  >= x
  F32 d = x - r;
  return(d);
}

// transcendental functions

#include <math.h>

MR4TH_SYMBOL F32
sin_rad_F32(F32 x){
  return(sinf(x));
}

MR4TH_SYMBOL F32
cos_rad_F32(F32 x){
  return(cosf(x));
}

MR4TH_SYMBOL F32
sin_F32(F32 x){
  return(sinf(x*tau_F32));
}

MR4TH_SYMBOL F32
cos_F32(F32 x){
  return(cosf(x*tau_F32));
}

MR4TH_SYMBOL F32
tan_F32(F32 x){
  return(tanf(x*tau_F32));
}

MR4TH_SYMBOL F32
atan_F32(F32 x){
  return(atanf(x)/tau_F32);
}

MR4TH_SYMBOL F32
atan2_F32(F32 x, F32 y){
  return(atan2f(y, x)/tau_F32);
}

MR4TH_SYMBOL F32
sqrt_F32(F32 x){
  return(sqrtf(x));
}

MR4TH_SYMBOL F32
ln_F32(F32 x){
  return(logf(x));
}

MR4TH_SYMBOL F32
pow_F32(F32 base, F32 x){
  return(powf(base, x));
}

MR4TH_SYMBOL F64
sin_F64(F64 x){
  return(sin(x*tau_F64));
}

MR4TH_SYMBOL F64
cos_F64(F64 x){
  return(cos(x*tau_F64));
}

MR4TH_SYMBOL F64
tan_F64(F64 x){
  return(tan(x*tau_F64));
}

MR4TH_SYMBOL F64
atan_F64(F64 x){
  return(atan(x)/tau_F64);
}

MR4TH_SYMBOL F64
atan2_F64(F64 x, F64 y){
  return(atan2(x, y)/tau_F64);
}

MR4TH_SYMBOL F64
sqrt_F64(F64 x){
  return(sqrt(x));
}

MR4TH_SYMBOL F64
ln_F64(F64 x){
  return(log(x));
}

MR4TH_SYMBOL F64
pow_F64(F64 base, F64 x){
  return(powf(base, x));
}

// linear interpolation

MR4TH_SYMBOL F32
lerp(F32 a, F32 t, F32 b){
  F32 x = a + (b - a)*t;
  return(x);
}

MR4TH_SYMBOL F32
unlerp(F32 a, F32 x, F32 b){
  F32 t = 0.f;
  if (a != b){
    t = (x - a)/(b - a);
  }
  return(t);
}

MR4TH_SYMBOL F32
lerp_range(RangeF32 r, F32 t){
  F32 x = r.min + (r.max - r.min)*t;
  return(x);
}

// integer rounding

MR4TH_SYMBOL U32
next_pow2_U32(U32 x){
  U32 result = x;
  if (result == 0){
    result = 1;
  }
  else{
    // TODO(allen): speed up with bit scan operand?
    result -= 1;
    result |= (result >>  1);
    result |= (result >>  2);
    result |= (result >>  4);
    result |= (result >>  8);
    result |= (result >> 16);
    result += 1;
  }
  return(result);
}

MR4TH_SYMBOL U64
next_pow2_U64(U32 x){
  U64 result = x;
  if (result == 0){
    result = 1;
  }
  else{
    // TODO(allen): speed up with bit scan operand?
    result -= 1;
    result |= (result >>  1);
    result |= (result >>  2);
    result |= (result >>  4);
    result |= (result >>  8);
    result |= (result >> 16);
    result |= (result >> 32);
    result += 1;
  }
  return(result);
}

MR4TH_SYMBOL S64
sign_extend_S64(S64 x, U32 bitidx){
  U32 shiftn = (63 - bitidx);
  x = x << shiftn;
  x = x >> shiftn;
  return(x);
}

// gaussian

MR4TH_SYMBOL F32
math_gaussian(F32 sigma, F32 x){
  // 1/(sqrt(2*pi)*sigma) * e^(-x*x/(2*sigma*sigma))

  F32 sqrt_2pi = sqrt_F32(tau_F32);
  F32 inv_sqrt_2pi_sigma = 1.f/(sqrt_2pi*sigma);
  F32 sigma_p2 = sigma*sigma;

  F32 exponent = -x*x/(2*sigma_p2);
  F32 power = pow_F32(e_F32, exponent);

  F32 result = inv_sqrt_2pi_sigma*power;
  return(result);
}

MR4TH_SYMBOL Array_F32
math_gaussian_kernel(Arena *arena, F32 sigma, U32 extra_reach){
  Assert(sigma > 0.f);

  // allocate
  U32 reach = (U32)ceil_F32(sigma) + extra_reach;
  U64 count = reach*2 + 1;
  F32 *vals = push_array(arena, F32, count);

  // calculate guassian samples & sum
  F32 sum = 0;
  {
    F32 sqrt_2pi = sqrt_F32(tau_F32);
    F32 inv_sqrt_2pi_sigma = 1.f/(sqrt_2pi*sigma);
    F32 sigma_p2 = sigma*sigma;

    F32 mid = (F32)reach;
    F32 *val_ptr = vals;
    for (U64 i = 0; i < count; i += 1, val_ptr += 1){
      F32 x = (F32)i - mid;
      F32 exponent = -x*x/(2*sigma_p2);
      F32 power = pow_F32(e_F32, exponent);
      F32 g = inv_sqrt_2pi_sigma*power;

      sum += g;
      *val_ptr = g;
    }
  }

  // normalize kernel
  {
    F32 *val_ptr = vals;
    for (U64 i = 0; i < count; i += 1, val_ptr += 1){
      *val_ptr = *val_ptr/sum;
    }
  }

  // return result
  Array_F32 result = {0};
  result.vals = vals;
  result.count = count;
  return(result);
}

////////////////////////////////
// Functions: Compound Types

// compound type constructors

MR4TH_SYMBOL V2S32
v2s32(S32 x, S32 y){
  V2S32 r = {x, y};
  return(r);
}

MR4TH_SYMBOL V2F32
v2f32(F32 x, F32 y){
  V2F32 r = {x, y};
  return(r);
}

MR4TH_SYMBOL V3F32
v3f32(F32 x, F32 y, F32 z){
  V3F32 r = {x, y, z};
  return(r);
}

MR4TH_SYMBOL V4F32
v4f32(F32 x, F32 y, F32 z, F32 w){
  V4F32 r = {x, y, z, w};
  return(r);
}

MR4TH_SYMBOL RangeF32
rangef32(F32 min, F32 max){
  RangeF32 r = {min, max};
  if (max < min){
    r.min = max;
    r.max = min;
  }
  return(r);
}

MR4TH_SYMBOL RectS32
rects32(S32 x0, S32 y0, S32 x1, S32 y1){
  RectS32 r = {x0, y0, x1, y1};
  if (x1 < x0){
    r.x0 = x1;
    r.x1 = x0;
  }
  if (y1 < y0){
    r.y0 = y1;
    r.y1 = y0;
  }
  return(r);
}

MR4TH_SYMBOL RectF32
rectf32(F32 x0, F32 y0, F32 x1, F32 y1){
  RectF32 r = {x0, y0, x1, y1};
  if (x1 < x0){
    r.x0 = x1;
    r.x1 = x0;
  }
  if (y1 < y0){
    r.y0 = y1;
    r.y1 = y0;
  }
  return(r);
}

MR4TH_SYMBOL RectF32
rectf32_corners(V2F32 p0, V2F32 p1){
  RectF32 r = rectf32(v2_expanded(p0), v2_expanded(p1));
  return(r);
}

MR4TH_SYMBOL RectF32
rectf32_ranges(RangeF32 x, RangeF32 y){
  RectF32 rect = {x.min, y.min, x.max, y.max};
  return(rect);
}

// 2d vectors

MR4TH_SYMBOL V2F32
v2f32_add(V2F32 a, V2F32 b){
  V2F32 result = {a.x + b.x, a.y + b.y};
  return(result);
}

MR4TH_SYMBOL V2F32
v2f32_sub(V2F32 a, V2F32 b){
  V2F32 result = {a.x - b.x, a.y - b.y};
  return(result);
}

MR4TH_SYMBOL V2F32
v2f32_scalar_mul(F32 s, V2F32 v){
  V2F32 result = {s*v.x, s*v.y};
  return(result);
}

MR4TH_SYMBOL V2F32
v2f32_lerp(V2F32 a, F32 t, V2F32 b){
  V2F32 result = {
    a.x + (b.x - a.x)*t,
    a.y + (b.y - a.y)*t,
  };
  return(result);
}

MR4TH_SYMBOL V2S32
v2s32_add(V2S32 a, V2S32 b){
  V2S32 result = {a.x + b.x, a.y + b.y};
  return(result);
}

MR4TH_SYMBOL V2S32
v2s32_sub(V2S32 a, V2S32 b){
  V2S32 result = {a.x - b.x, a.y - b.y};
  return(result);
}

MR4TH_SYMBOL V2F32
v2f32_polar(F32 theta, F32 radius){
  V2F32 result = {radius*cos_F32(theta), radius*sin_F32(theta)};
  return(result);
}

MR4TH_SYMBOL F32
angle_from_v2f32(V2F32 v){
  F32 result = atan2_F32(v.x, v.y);
  return(result);
}

MR4TH_SYMBOL F32
length_from_v2f32(V2F32 v){
  F32 result = sqrt_F32(v.x*v.x + v.y*v.y);
  return(result);
}

MR4TH_SYMBOL V2F32
v2f32_unit(V2F32 v){
  F32 length = length_from_v2f32(v);
  F32 norm = (length > 0.f)?(1.f/length):0.f;
  V2F32 result = {v.x*norm, v.y*norm};
  return(result);
}


// 3d vectors

MR4TH_SYMBOL V3F32
v3f32_add(V3F32 a, V3F32 b){
  V3F32 result = {a.x + b.x, a.y + b.y, a.z + b.z};
  return(result);
}

MR4TH_SYMBOL V3F32
v3f32_sub(V3F32 a, V3F32 b){
  V3F32 result = {a.x - b.x, a.y - b.y, a.z - b.z};
  return(result);
}

MR4TH_SYMBOL V3F32
v3f32_scalar_mul(F32 s, V3F32 v){
  V3F32 result = {s*v.x, s*v.y, s*v.z};
  return(result);
}

MR4TH_SYMBOL F32
length_from_v3f32(V3F32 v){
  F32 result = sqrt_F32(v.x*v.x + v.y*v.y + v.z*v.z);
  return(result);
}

MR4TH_SYMBOL V3F32
v3f32_spherical(F32 theta_xz, F32 theta_yz, F32 radius){
  F32 sxz = sin_F32(theta_xz);
  F32 syz = sin_F32(theta_yz);
  F32 cxz = cos_F32(theta_xz);
  F32 cyz = cos_F32(theta_yz);

  V3F32 result = {0};
  result.x = radius*sxz;
  result.y = radius*syz;
  result.z = radius*cxz*cyz;
  return(result);
}

MR4TH_SYMBOL V3F32
v3f32_unit(V3F32 v){
  F32 length = length_from_v3f32(v);
  F32 norm = (length > 0.f)?(1.f/length):0.f;
  V3F32 result = {v.x*norm, v.y*norm, v.z*norm};
  return(result);
}

MR4TH_SYMBOL V3F32
v3f32_cross(V3F32 a, V3F32 b){
  V3F32 result = {
    a.y*b.z - a.z*b.y,
    a.z*b.x - a.x*b.z,
    a.x*b.y - a.y*b.x,
  };
  return(result);
}


// 4x4 matrix

MR4TH_SYMBOL B32
mat4x4_inv(F32 *in, F32 *out){
  /* credit: https://github.com/niswegmann/small-matrix-inverse/tree/master */

  out[0] =
    + in[ 5]*in[10]*in[15]
    - in[ 5]*in[11]*in[14]
    - in[ 9]*in[ 6]*in[15]
    + in[ 9]*in[ 7]*in[14]
    + in[13]*in[ 6]*in[11]
    - in[13]*in[ 7]*in[10];

  out[1] =
    - in[ 1]*in[10]*in[15]
    + in[ 1]*in[11]*in[14]
    + in[ 9]*in[ 2]*in[15]
    - in[ 9]*in[ 3]*in[14]
    - in[13]*in[ 2]*in[11]
    + in[13]*in[ 3]*in[10];

  out[2] =
    + in[ 1]*in[ 6]*in[15]
    - in[ 1]*in[ 7]*in[14]
    - in[ 5]*in[ 2]*in[15]
    + in[ 5]*in[ 3]*in[14]
    + in[13]*in[ 2]*in[ 7]
    - in[13]*in[ 3]*in[ 6];

  out[3] =
    - in[ 1]*in[ 6]*in[11]
    + in[ 1]*in[ 7]*in[10]
    + in[ 5]*in[ 2]*in[11]
    - in[ 5]*in[ 3]*in[10]
    - in[ 9]*in[ 2]*in[ 7]
    + in[ 9]*in[ 3]*in[ 6];

  out[4] =
    - in[ 4]*in[10]*in[15]
    + in[ 4]*in[11]*in[14]
    + in[ 8]*in[ 6]*in[15]
    - in[ 8]*in[ 7]*in[14]
    - in[12]*in[ 6]*in[11]
    + in[12]*in[ 7]*in[10];

  out[5] =
    + in[ 0]*in[10]*in[15]
    - in[ 0]*in[11]*in[14]
    - in[ 8]*in[ 2]*in[15]
    + in[ 8]*in[ 3]*in[14]
    + in[12]*in[ 2]*in[11]
    - in[12]*in[ 3]*in[10];

  out[6] =
    - in[ 0]*in[ 6]*in[15]
    + in[ 0]*in[ 7]*in[14]
    + in[ 4]*in[ 2]*in[15]
    - in[ 4]*in[ 3]*in[14]
    - in[12]*in[ 2]*in[ 7]
    + in[12]*in[ 3]*in[ 6];

  out[7] =
    + in[ 0]*in[ 6]*in[11]
    - in[ 0]*in[ 7]*in[10]
    - in[ 4]*in[ 2]*in[11]
    + in[ 4]*in[ 3]*in[10]
    + in[ 8]*in[ 2]*in[ 7]
    - in[ 8]*in[ 3]*in[ 6];

  out[8] =
    + in[ 4]*in[ 9]*in[15]
    - in[ 4]*in[11]*in[13]
    - in[ 8]*in[ 5]*in[15]
    + in[ 8]*in[ 7]*in[13]
    + in[12]*in[ 5]*in[11]
    - in[12]*in[ 7]*in[ 9];

  out[9] =
    - in[ 0]*in[ 9]*in[15]
    + in[ 0]*in[11]*in[13]
    + in[ 8]*in[ 1]*in[15]
    - in[ 8]*in[ 3]*in[13]
    - in[12]*in[ 1]*in[11]
    + in[12]*in[ 3]*in[ 9];

  out[10] =
    + in[ 0]*in[ 5]*in[15]
    - in[ 0]*in[ 7]*in[13]
    - in[ 4]*in[ 1]*in[15]
    + in[ 4]*in[ 3]*in[13]
    + in[12]*in[ 1]*in[ 7]
    - in[12]*in[ 3]*in[ 5];

  out[11] =
    - in[ 0]*in[ 5]*in[11]
    + in[ 0]*in[ 7]*in[ 9]
    + in[ 4]*in[ 1]*in[11]
    - in[ 4]*in[ 3]*in[ 9]
    - in[ 8]*in[ 1]*in[ 7]
    + in[ 8]*in[ 3]*in[ 5];

  out[12] =
    - in[ 4]*in[ 9]*in[14]
    + in[ 4]*in[10]*in[13]
    + in[ 8]*in[ 5]*in[14]
    - in[ 8]*in[ 6]*in[13]
    - in[12]*in[ 5]*in[10]
    + in[12]*in[ 6]*in[ 9];

  out[13] =
    + in[ 0]*in[ 9]*in[14]
    - in[ 0]*in[10]*in[13]
    - in[ 8]*in[ 1]*in[14]
    + in[ 8]*in[ 2]*in[13]
    + in[12]*in[ 1]*in[10]
    - in[12]*in[ 2]*in[ 9];

  out[14] =
    - in[ 0]*in[ 5]*in[14]
    + in[ 0]*in[ 6]*in[13]
    + in[ 4]*in[ 1]*in[14]
    - in[ 4]*in[ 2]*in[13]
    - in[12]*in[ 1]*in[ 6]
    + in[12]*in[ 2]*in[ 5];

  out[15] =
    + in[ 0]*in[ 5]*in[10]
    - in[ 0]*in[ 6]*in[ 9]
    - in[ 4]*in[ 1]*in[10]
    + in[ 4]*in[ 2]*in[ 9]
    + in[ 8]*in[ 1]*in[ 6]
    - in[ 8]*in[ 2]*in[ 5];

  F32 det = in[0]*out[0] + in[1]*out[4] + in[2]*out[8] + in[3]*out[12];

  B32 result = 0;
  if (det != 0.f){
    F32 invdet = 1.f/det;
    for (U32 i = 0; i < 16; i += 1){
      out[i] *= invdet;
    }
    result = 1;
  }

  return(result);
}

MR4TH_SYMBOL void
mat4x4_mul(F32 *a, F32 *b, F32 *out){
  out[ 0] = a[ 0]*b[ 0] + a[ 1]*b[ 4] + a[ 2]*b[ 8] + a[ 3]*b[12];
  out[ 1] = a[ 0]*b[ 1] + a[ 1]*b[ 5] + a[ 2]*b[ 9] + a[ 3]*b[13];
  out[ 2] = a[ 0]*b[ 2] + a[ 1]*b[ 6] + a[ 2]*b[10] + a[ 3]*b[14];
  out[ 3] = a[ 0]*b[ 3] + a[ 1]*b[ 7] + a[ 2]*b[11] + a[ 3]*b[15];
  out[ 4] = a[ 4]*b[ 0] + a[ 5]*b[ 4] + a[ 6]*b[ 8] + a[ 7]*b[12];
  out[ 5] = a[ 4]*b[ 1] + a[ 5]*b[ 5] + a[ 6]*b[ 9] + a[ 7]*b[13];
  out[ 6] = a[ 4]*b[ 2] + a[ 5]*b[ 6] + a[ 6]*b[10] + a[ 7]*b[14];
  out[ 7] = a[ 4]*b[ 3] + a[ 5]*b[ 7] + a[ 6]*b[11] + a[ 7]*b[15];
  out[ 8] = a[ 8]*b[ 0] + a[ 9]*b[ 4] + a[10]*b[ 8] + a[11]*b[12];
  out[ 9] = a[ 8]*b[ 1] + a[ 9]*b[ 5] + a[10]*b[ 9] + a[11]*b[13];
  out[10] = a[ 8]*b[ 2] + a[ 9]*b[ 6] + a[10]*b[10] + a[11]*b[14];
  out[11] = a[ 8]*b[ 3] + a[ 9]*b[ 7] + a[10]*b[11] + a[11]*b[15];
  out[12] = a[12]*b[ 0] + a[13]*b[ 4] + a[14]*b[ 8] + a[15]*b[12];
  out[13] = a[12]*b[ 1] + a[13]*b[ 5] + a[14]*b[ 9] + a[15]*b[13];
  out[14] = a[12]*b[ 2] + a[13]*b[ 6] + a[14]*b[10] + a[15]*b[14];
  out[15] = a[12]*b[ 3] + a[13]*b[ 7] + a[14]*b[11] + a[15]*b[15];
}


// rectangle layout

MR4TH_SYMBOL B32
rectf32_contains_v2f32(RectF32 rect, V2F32 p){
  B32 result = (rect.x0 <= p.x && p.x < rect.x1 &&
                rect.y0 <= p.y && p.y < rect.y1);
  return(result);
}

MR4TH_SYMBOL B32
rectf32_overlaps_rectf32(RectF32 a, RectF32 b){
  B32 x = (b.x0 < a.x1 && a.x0 < b.x1);
  B32 y = (b.y0 < a.y1 && a.y0 < b.y1);
  return(x && y);
}

MR4TH_SYMBOL B32
rects32_contains_v2s32(RectS32 rect, V2S32 p){
  B32 result = (rect.x0 <= p.x && p.x < rect.x1 &&
                rect.y0 <= p.y && p.y < rect.y1);
  return(result);
}

MR4TH_SYMBOL V2S32
dim_from_rects32(RectS32 r){
  V2S32 result = {r.x1 - r.x0, r.y1 - r.y0};
  if (result.x < 0){
    result.x = -result.x;
  }
  if (result.y < 0){
    result.y = -result.y;
  }
  return(result);
}

MR4TH_SYMBOL V2F32
dim_from_rectf32(RectF32 r){
  V2F32 result = {r.x1 - r.x0, r.y1 - r.y0};
  if (result.x < 0){
    result.x = -result.x;
  }
  if (result.y < 0){
    result.y = -result.y;
  }
  return(result);
}

MR4TH_SYMBOL RectF32
rectf32_in_rectf32_clamp(RectF32 container, V2F32 p0, V2F32 dim){
  V2F32 container_dim = dim_from_rectf32(container);
  F32 w = ClampTop(dim.x, container_dim.x);
  F32 h = ClampTop(dim.y, container_dim.y);

  F32 x0 = Clamp(container.x0, p0.x, container.x1 - w);
  F32 y0 = Clamp(container.y0, p0.y, container.y1 - h);

  RectF32 result = {x0, y0, x0 + w, y0 + h};
  return(result);
}

MR4TH_SYMBOL RectF32
rectf32_in_rectf32_keep_aspect_ratio(RectF32 container, V2F32 dim){
  RectF32 result = {0};
  if (dim.x > 0 && dim.y > 0){
    V2F32 container_dim = dim_from_rectf32(container);
    F32 x_score = container_dim.x/dim.x;
    F32 y_score = container_dim.y/dim.y;
    F32 score = Min(x_score, y_score);
    V2F32 scaled_half_dim = v2f32_scalar_mul(score*0.5f, dim);
    V2F32 center = v2f32((container.x0 + container.x1)*0.5f,
                         (container.y0 + container.y1)*0.5f);
    result = rectf32(center.x - scaled_half_dim.x, center.y - scaled_half_dim.y,
                     center.x + scaled_half_dim.x, center.y + scaled_half_dim.y);
  }
  return(result);
}

MR4TH_SYMBOL RectF32
rectf32_intersect(RectF32 a, RectF32 b){
  RectF32 result = {0};
  result.x0 = Max(a.x0, b.x0);
  result.y0 = Max(a.y0, b.y0);
  result.x1 = Min(a.x1, b.x1);
  result.y1 = Min(a.y1, b.y1);
  return(result);
}

////////////////////////////////
// Functions: Memory Operations

MR4TH_SYMBOL void
memory_zero(void *ptr, U64 size){
  U64  z64 = size/8;
  U64  z8  = size%8;

  U64 *p64 = (U64*)ptr;
  for (;z64 > 0;){
    *p64 = 0;
    p64 += 1;
    z64 -= 1;
  }
  U8 *p8 = (U8*)p64;
  for (;z8 > 0;){
    *p8 = 0;
    p8 += 1;
    z8 -= 1;
  }
}

MR4TH_SYMBOL void
memory_fill(void *ptr, U64 size, U8 fillbyte){
  U64 fillqword = fillbyte;
  {
    fillqword |= fillqword << 8;
    fillqword |= fillqword << 16;
    fillqword |= fillqword << 32;
  }

  U64  z64 = size/8;
  U64  z8  = size%8;

  U64 *p64 = (U64*)ptr;
  for (;z64 > 0;){
    *p64 = fillqword;
    p64 += 1;
    z64 -= 1;
  }
  U8 *p8 = (U8*)p64;
  for (;z8 > 0;){
    *p8 = fillbyte;
    p8 += 1;
    z8 -= 1;
  }
}

MR4TH_SYMBOL B32
memory_match(void *a, void *b, U64 size){
  U64  z64 = size/8;
  U64  z8  = size%8;

  U64 *a64 = (U64*)a;
  U64 *b64 = (U64*)b;
  for (;z64 > 0;){
    if (*a64 != *b64){
      return(0);
    }
    a64 += 1;
    b64 += 1;
    z64 -= 1;
  }

  U8 *a8 = (U8*)a64;
  U8 *b8 = (U8*)b64;
  for (;z8 > 0;){
    if (*a8 != *b8){
      return(0);
    }
    a8 += 1;
    b8 += 1;
    z8 -= 1;
  }

  return(1);
}

MR4TH_SYMBOL void*
memory_move(void *dst, void *src, U64 size){
  U64  z64 = size/8;
  U64  z8  = size%8;

  // backwards
  if ((U8*)src < (U8*)dst){
    U8 *dst8 = ((U8*)dst) + size;
    U8 *src8 = ((U8*)src) + size;
    for (;z8 > 0;){
      dst8 -= 1;
      src8 -= 1;
      *dst8 = *src8;
      z8 -= 1;
    }
    U64 *dst64 = (U64*)dst8;
    U64 *src64 = (U64*)src8;
    for (;z64 > 0;){
      dst64 -= 1;
      src64 -= 1;
      *dst64 = *src64;
      z64 -= 1;
    }
  }

  // forwards
  else if ((U8*)src > (U8*)dst){
    U64 *dst64 = (U64*)dst;
    U64 *src64 = (U64*)src;
    for (;z64 > 0;){
      *dst64 = *src64;
      dst64 += 1;
      src64 += 1;
      z64 -= 1;
    }
    U8 *dst8 = (U8*)dst64;
    U8 *src8 = (U8*)src64;
    for (;z8 > 0;){
      *dst8 = *src8;
      dst8 += 1;
      src8 += 1;
      z8 -= 1;
    }
  }

  return(dst);
}

////////////////////////////////
// Functions: Strings

// characters

MR4TH_SYMBOL B32
str8_char_is_whitespace(U8 c){
  return(c == ' ' || c == '\n' || c == '\t' ||
         c == '\r' || c == '\f' || c == '\v');
}

MR4TH_SYMBOL B32
str8_char_is_slash(U8 c){
  return(c == '/' || c == '\\');
}

MR4TH_SYMBOL B32
str8_char_is_digit(U8 c){
  return('0' <= c && c <= '9');
}

MR4TH_SYMBOL U8
str8_char_uppercase(U8 c){
  if ('a' <= c && c <= 'z'){
    c += 'A' - 'a';
  }
  return(c);
}

MR4TH_SYMBOL U8
str8_char_lowercase(U8 c){
  if ('A' <= c && c <= 'Z'){
    c += 'a' - 'A';
  }
  return(c);
}

// in-place constructors

MR4TH_SYMBOL String8
str8(U8 *str, U64 size){
  String8 result = {str, size};
  return(result);
}

MR4TH_SYMBOL String8
str8_range(U8 *first, U8 *opl){
  String8 result = {first, (U64)(opl - first)};
  return(result);
}

MR4TH_SYMBOL String8
str8_cstring(U8 *cstr){
  U8 *ptr = cstr;
  for (;*ptr != 0; ptr += 1);
  String8 result = str8_range(cstr, ptr);
  return(result);
}

MR4TH_SYMBOL String8
str8_cstring_capped(U8 *cstr, U8 *opl){
  U8 *ptr = cstr;
  for (;ptr < opl && *ptr != 0; ptr += 1);
  String8 result = str8_range(cstr, ptr);
  return(result);
}

MR4TH_SYMBOL String8
str8_prefix(String8 str, U64 size){
  U64 size_clamped = ClampTop(size, str.size);
  String8 result = {str.str, size_clamped};
  return(result);
}

MR4TH_SYMBOL String8
str8_chop(String8 str, U64 amount){
  U64 amount_clamped = ClampTop(amount, str.size);
  U64 remaining_size = str.size - amount_clamped;
  String8 result = {str.str, remaining_size};
  return(result);
}

MR4TH_SYMBOL String8
str8_postfix(String8 str, U64 size){
  U64 size_clamped = ClampTop(size, str.size);
  U64 skip_to = str.size - size_clamped;
  String8 result = {str.str + skip_to, size_clamped};
  return(result);
}

MR4TH_SYMBOL String8
str8_skip(String8 str, U64 amount){
  U64 amount_clamped = ClampTop(amount, str.size);
  U64 remaining_size = str.size - amount_clamped;
  String8 result = {str.str + amount_clamped, remaining_size};
  return(result);
}

MR4TH_SYMBOL String8
str8_substr_opl(String8 str, U64 first, U64 opl){
  U64 opl_clamped = ClampTop(opl, str.size);
  U64 first_clamped = ClampTop(first, opl_clamped);
  String8 result = {str.str + first_clamped, opl_clamped - first_clamped};
  return(result);
}

MR4TH_SYMBOL String8
str8_substr_size(String8 str, U64 first, U64 size){
  String8 result = str8_substr_opl(str, first, first + size);
  return(result);
}

MR4TH_SYMBOL String8
str8_skip_chop_whitespace(String8 str){
  String8 result = str;
  if (result.size > 0){
    U8 *sptr = str.str;
    U8 *eptr = str.str + str.size - 1;
    for (;sptr < eptr && str8_char_is_whitespace(*sptr); sptr += 1);
    for (;sptr < eptr && str8_char_is_whitespace(*eptr); eptr -= 1);
    result = str8_range(sptr, eptr + 1);
  }
  return(result);
}

MR4TH_SYMBOL String16
str16(U16 *str, U64 size){
  String16 result = {str, size};
  return(result);
}

MR4TH_SYMBOL String16
str16_cstring(U16 *cstr){
  U16 *ptr = cstr;
  for (;*ptr != 0; ptr += 1);
  String16 result = {cstr, (U64)(ptr - cstr)};
  return(result);
}

MR4TH_SYMBOL String32
str32(U32 *str, U64 size){
  String32 result = {str, size};
  return(result);
}

// hash

MR4TH_SYMBOL U64
str8_hash(String8 str){
  U64 hash = 5381;
  for (U8 *ptr = str.str, *opl = str.str + str.size;
       ptr < opl;
       ptr += 1){
    U8 c = *ptr;
    hash = (hash*33)^c;
  }
  return(hash);
}

// path helpers

MR4TH_SYMBOL String8
str8_chop_last_slash(String8 string){
  String8 result = string;
  if (string.size > 0){
    // pos one past last slash
    U64 pos = string.size;
    for (S64 i = string.size - 1; i >= 0; i -= 1){
      if (str8_char_is_slash(string.str[i])){
        pos = i;
        break;
      }
    }

    // chop result string
    result.size = pos;
  }
  return(result);
}

////////////////////////////////
// Functions: Serializer/Deserializer Helpers

MR4TH_SYMBOL B32
dser_read(String8 data, U64 off, void *dst, U64 size){
  B32 result = 0;
  if (off + size <= data.size){
    result = 1;
    MemoryCopy(dst, data.str + off, size);
  }
  return(result);
}

////////////////////////////////
// Functions: Symbolic Constants

MR4TH_SYMBOL OperatingSystem
operating_system_from_context(void){
  OperatingSystem result = OperatingSystem_Null;
#if OS_WINDOWS
  result = OperatingSystem_Windows;
#elif OS_LINUX
  result = OperatingSystem_Linux;
#elif OS_MAC
  result = OperatingSystem_Mac;
#endif
  return(result);
}

MR4TH_SYMBOL Architecture
architecture_from_context(void){
  Architecture result = Architecture_Null;
#if ARCH_X64
  result = Architecture_X64;
#elif ARCH_X86
  result = Architecture_X86;
#elif ARCH_ARM
  result = Architecture_Arm;
#elif ARCH_ARM64
  result = Architecture_Arm64;
#endif
  return(result);
}

MR4TH_SYMBOL char*
string_from_operating_system(OperatingSystem os){
  char *result = "(null)";
  switch (os){
    case OperatingSystem_Windows:
    {
      result = "windows";
    }break;
    case OperatingSystem_Linux:
    {
      result = "linux";
    }break;
    case OperatingSystem_Mac:
    {
      result = "mac";
    }break;
  }
  return(result);
}

MR4TH_SYMBOL char*
string_from_architecture(Architecture arch){
  char *result = "(null)";
  switch (arch){
    case Architecture_X64:
    {
      result = "x64";
    }break;
    case Architecture_X86:
    {
      result = "x86";
    }break;
    case Architecture_Arm:
    {
      result = "arm";
    }break;
    case Architecture_Arm64:
    {
      result = "armm64";
    }break;
  }
  return(result);
}

MR4TH_SYMBOL char*
string_from_month(Month month){
  char *result = "(null)";
  switch (month){
    case Month_Jan:
    {
      result = "jan";
    }break;
    case Month_Feb:
    {
      result = "feb";
    }break;
    case Month_Mar:
    {
      result = "mar";
    }break;
    case Month_Apr:
    {
      result = "apr";
    }break;
    case Month_May:
    {
      result = "may";
    }break;
    case Month_Jun:
    {
      result = "jun";
    }break;
    case Month_Jul:
    {
      result = "jul";
    }break;
    case Month_Aug:
    {
      result = "aug";
    }break;
    case Month_Sep:
    {
      result = "sep";
    }break;
    case Month_Oct:
    {
      result = "oct";
    }break;
    case Month_Nov:
    {
      result = "nov";
    }break;
    case Month_Dec:
    {
      result = "dec";
    }break;
  }
  return(result);
}

MR4TH_SYMBOL char*
string_from_day_of_week(DayOfWeek day_of_week){
  char *result = "(null)";
  switch (day_of_week){
    case DayOfWeek_Sunday:
    {
      result = "sunday";
    }break;
    case DayOfWeek_Monday:
    {
      result = "monday";
    }break;
    case DayOfWeek_Tuesday:
    {
      result = "tuesday";
    }break;
    case DayOfWeek_Wednesday:
    {
      result = "wednesday";
    }break;
    case DayOfWeek_Thursday:
    {
      result = "thursday";
    }break;
    case DayOfWeek_Friday:
    {
      result = "friday";
    }break;
    case DayOfWeek_Saturday:
    {
      result = "saturday";
    }break;
  }
  return(result);
}

////////////////////////////////
// Functions: Time

MR4TH_SYMBOL DenseTime
dense_time_from_date_time(DateTime *in){
  U32 year_encoded = (U32)((S32)in->year + 0x8000);
  DenseTime result = 0;
  result += year_encoded;
  result *= 12;
  result += (in->mon - 1);
  result *= 31;
  result += (in->day - 1);
  result *= 24;
  result += in->hour;
  result *= 60;
  result += in->min;
  result *= 61;
  result += in->sec;
  result *= 1000;
  result += in->msec;
  return(result);
}

MR4TH_SYMBOL DateTime
date_time_from_dense_time(DenseTime in){
  DateTime result = {0};
  result.msec = in%1000;
  in /= 1000;
  result.sec = in%61;
  in /= 61;
  result.min = in%60;
  in /= 60;
  result.hour = in%24;
  in /= 24;
  result.day = (in%31) + 1;
  in /= 31;
  result.mon = (in%12) + 1;
  in /= 12;
  S32 year_encoded = (S32)in;
  result.year = (year_encoded - 0x8000);
  return(result);
}

////////////////////////////////
// Functions: Arena

#define MEM_INITIAL_COMMIT KB(4)
#define MEM_INTERNAL_MIN_SIZE AlignUpPow2(sizeof(Arena), MR4TH_MEM_MAX_ALIGN)

// arena pre-requisites

StaticAssert(sizeof(Arena) <= MEM_INITIAL_COMMIT,
             mem_check_arena_size);

StaticAssert(IsPow2OrZero(MR4TH_MEM_COMMIT_BLOCK_SIZE) &&
             MR4TH_MEM_COMMIT_BLOCK_SIZE != 0,
             mem_check_commit_block_size);

StaticAssert(IsPow2OrZero(MR4TH_MEM_MAX_ALIGN) && MR4TH_MEM_MAX_ALIGN != 0,
             mem_check_max_align);

// arena implementation

MR4TH_SYMBOL Arena*
arena_alloc_reserve(U64 reserve_size, B32 growing){
  ProfBeginFunc();

  Arena *result = 0;
  if (reserve_size >= MEM_INITIAL_COMMIT){
    void *memory = os_memory_reserve(reserve_size);
    if (os_memory_commit(memory, MEM_INITIAL_COMMIT)){
      AsanPoison(memory, reserve_size);
      AsanUnpoison(memory, MEM_INTERNAL_MIN_SIZE);
      result = (Arena*)memory;
      result->current = result;
      result->prev = 0;
      result->alignment = sizeof(void*);
      result->growing = growing;
      result->base_pos = 0;
      result->chunk_cap = reserve_size;
      result->chunk_pos = MEM_INTERNAL_MIN_SIZE;
      result->chunk_commit_pos = MEM_INITIAL_COMMIT;
    }
  }
  Assert(result != 0);

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL Arena*
arena_alloc(void){
  Arena *result = arena_alloc_reserve(MR4TH_MEM_DEFAULT_RESERVE_SIZE, 1);
  return(result);
}

MR4TH_SYMBOL void
arena_release(Arena *arena){
  ProfBeginFunc();

  Arena *ptr = arena->current;
  for (;ptr != 0;){
    Arena *prev = ptr->prev;
    AsanPoison(ptr, ptr->chunk_cap);
    os_memory_release(ptr, ptr->chunk_cap);
    ptr = prev;
  }

  ProfEndFunc();
}

MR4TH_SYMBOL void*
arena_push_no_zero(Arena *arena, U64 size){
  ProfBeginFunc();

  void *result = 0;

  Arena *current = arena->current;

  // allocate new chunk if necessary
  if (arena->growing){
    U64 next_chunk_pos = AlignUpPow2(current->chunk_pos, arena->alignment);
    next_chunk_pos += size;
    if (next_chunk_pos > current->chunk_cap){
      U64 new_reserve_size = MR4TH_MEM_DEFAULT_RESERVE_SIZE;
      U64 enough_to_fit = size + MEM_INTERNAL_MIN_SIZE;
      if (new_reserve_size < enough_to_fit){
        new_reserve_size = AlignUpPow2(enough_to_fit, KB(4));
      }

      void *memory = os_memory_reserve(new_reserve_size);
      if (os_memory_commit(memory, MEM_INITIAL_COMMIT)){
        AsanPoison(memory, new_reserve_size);
        AsanUnpoison(memory, MEM_INTERNAL_MIN_SIZE);
        Arena *new_chunk = (Arena*)memory;
        new_chunk->prev = current;
        new_chunk->base_pos = current->base_pos + current->chunk_cap;
        new_chunk->chunk_cap = new_reserve_size;
        new_chunk->chunk_pos = MEM_INTERNAL_MIN_SIZE;
        new_chunk->chunk_commit_pos = MEM_INITIAL_COMMIT;
        current = arena->current = new_chunk;
      }
    }
  }

  {
    // if there is room in this chunk's reserve ...
    U64 result_pos = AlignUpPow2(current->chunk_pos, arena->alignment);
    U64 next_chunk_pos = result_pos + size;
    if (next_chunk_pos <= current->chunk_cap){

      // commit more memory if necessary
      if (next_chunk_pos > current->chunk_commit_pos){
        U64 next_commit_pos_aligned =
          AlignUpPow2(next_chunk_pos, MR4TH_MEM_COMMIT_BLOCK_SIZE);
        U64 next_commit_pos =
          ClampTop(next_commit_pos_aligned, current->chunk_cap);
        U64 commit_size = next_commit_pos - current->chunk_commit_pos;
        if (os_memory_commit((U8*)current + current->chunk_commit_pos, commit_size)){
          current->chunk_commit_pos = next_commit_pos;
        }
      }

      // if there is room in the commit range, return memory & advance pos
      if (next_chunk_pos <= current->chunk_commit_pos){
        AsanUnpoison((U8*)current + current->chunk_pos,
                     next_chunk_pos - current->chunk_pos);
        result = (U8*)current + result_pos;
        current->chunk_pos = next_chunk_pos;
      }
    }
  }

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL void
arena_pop_to(Arena *arena, U64 pos){
  ProfBeginFunc();

  Arena *current = arena->current;
  U64 total_pos = current->base_pos + current->chunk_pos;
  if (pos < total_pos){
    // release all chunks that begin after this pos
    U64 clamped_total_pos = ClampBot(pos, MEM_INTERNAL_MIN_SIZE);
    for (; clamped_total_pos < current->base_pos; ){
      Arena *prev = current->prev;
      AsanPoison(current, current->chunk_cap);
      os_memory_release(current, current->chunk_cap);
      current = prev;
    }

    // update arena's current
    {
      arena->current = current;
    }

    // update the chunk position of the chunk in
    //  the chain that contains this position.
    {
      U64 chunk_pos = clamped_total_pos - current->base_pos;
      U64 clamped_chunk_pos = ClampBot(chunk_pos, MEM_INTERNAL_MIN_SIZE);
      AsanPoison((U8*)current + clamped_chunk_pos,
                 current->chunk_pos - clamped_chunk_pos);
      current->chunk_pos = clamped_chunk_pos;
    }
  }

  ProfEndFunc();
}

MR4TH_SYMBOL U64
arena_current_pos(Arena *arena){
  Arena *current = arena->current;
  U64 result = current->base_pos + current->chunk_pos;
  return(result);
}

MR4TH_SYMBOL void*
arena_push(Arena *arena, U64 size){
  void *result = arena_push_no_zero(arena, size);
  MemoryZero(result, size);
  return(result);
}

MR4TH_SYMBOL void
arena_align(Arena *arena, U64 pow2_align){
  Assert(IsPow2OrZero(pow2_align) && pow2_align != 0 &&
         pow2_align <= MR4TH_MEM_MAX_ALIGN);
  Arena *current = arena->current;
  U64 p = current->chunk_pos;
  U64 p_aligned = AlignUpPow2(p, pow2_align);
  U64 z = p_aligned - p;
  if (z > 0){
    arena_push(arena, z);
  }
}

MR4TH_SYMBOL void
arena_pop_amount(Arena *arena, U64 amount){
  Arena *current = arena->current;
  U64 total_pos = current->base_pos + current->chunk_pos;
  if (amount <= total_pos){
    U64 new_pos = total_pos - amount;
    arena_pop_to(arena, new_pos);
  }
}

// temp helper functions

MR4TH_SYMBOL ArenaTemp
arena_begin_temp(Arena *arena){
  U64 pos = arena_current_pos(arena);
  ArenaTemp temp = {arena, pos};
  return(temp);
}

MR4TH_SYMBOL void
arena_end_temp(ArenaTemp *temp){
  arena_pop_to(temp->arena, temp->pos);
}

// scratch

MR4TH_SYM_COMPTIME MR4TH_THREADVAR
Arena *arena__scratch_pool[MR4TH_MEM_SCRATCH_POOL_COUNT] = {0};

#if MR4TH_DEFINE_RUNTIME_SYMBOLS

MR4TH_SYM_RUNTIME ArenaTemp
arena_get_scratch(Arena **conflict_array, U32 count){
  ProfBeginFunc();

  // init on first time
  if (arena__scratch_pool[0] == 0){
    Arena **scratch_slot = arena__scratch_pool;
    for (U64 i = 0;
         i < MR4TH_MEM_SCRATCH_POOL_COUNT;
         i += 1, scratch_slot += 1){
      *scratch_slot = arena_alloc();
    }
  }

  // get non-conflicting arena
  ArenaTemp result = {0};
  Arena **scratch_slot = arena__scratch_pool;
  for (U64 i = 0;
       i < MR4TH_MEM_SCRATCH_POOL_COUNT;
       i += 1, scratch_slot += 1){
    B32 is_non_conflict = 1;
    Arena **conflict_ptr = conflict_array;
    for (U32 j = 0; j < count; j += 1, conflict_ptr += 1){
      if (*scratch_slot == *conflict_ptr){
        is_non_conflict = 0;
        break;
      }
    }
    if (is_non_conflict){
      result = arena_begin_temp(*scratch_slot);
      break;
    }
  }

  ProfEndFunc();
  return(result);
}

#else /* MR4TH_DEFINE_RUNTIME_SYMBOLS */

MR4TH_SHARED_FUNC_INITIALIZER(arena_get_scratch);

#endif


////////////////////////////////
// Functions: Strings

// compound constructors

MR4TH_SYMBOL void
str8_list_push_explicit(String8List *list, String8 string,
                        String8Node *node_memory){
  node_memory->string = string;
  SLLQueuePush(list->first, list->last, node_memory);
  list->node_count += 1;
  list->total_size += string.size;
}

MR4TH_SYMBOL void
str8_list_push(Arena *arena, String8List *list, String8 string){
  String8Node *node = push_array(arena, String8Node, 1);
  str8_list_push_explicit(list, string, node);
}

MR4TH_SYMBOL void
str8_list_push_front_explicit(String8List *list, String8 string,
                              String8Node *node_memory){
  node_memory->string = string;
  SLLQueuePushFront(list->first, list->last, node_memory);
  list->node_count += 1;
  list->total_size += string.size;
}

MR4TH_SYMBOL void
str8_list_push_front(Arena *arena, String8List *list, String8 string){
  String8Node *node = push_array(arena, String8Node, 1);
  str8_list_push_front_explicit(list, string, node);
}

MR4TH_SYMBOL String8List
str8_list_copy(Arena *arena, String8List *list){
  String8List result = {0};
  for (String8Node *node = list->first;
       node != 0;
       node = node->next){
    String8 string = str8_push_copy(arena, node->string);
    str8_list_push(arena, &result, string);
  }
  return(result);
}

MR4TH_SYMBOL String8
str8_join(Arena *arena, String8List *list,
          StringJoin *join_optional){
  ProfBeginFunc();

  // setup join parameters
  MR4TH_SYM_COMPTIME StringJoin dummy_join = {0};
  StringJoin *join = join_optional;
  if (join == 0){
    join = &dummy_join;
  }

  // compute total size
  U64 size = (join->pre.size +
              join->post.size +
              ((list->node_count>0)?
               (join->mid.size*(list->node_count - 1)):0) +
              list->total_size);

  // begin string build
  U8 *str = push_array(arena, U8, size + 1);
  U8 *ptr = str;

  // write pre
  MemoryCopy(ptr, join->pre.str, join->pre.size);
  ptr += join->pre.size;

  B32 is_mid = 0;
  for (String8Node *node = list->first;
       node != 0;
       node = node->next){
    // write mid
    if (is_mid){
      MemoryCopy(ptr, join->mid.str, join->mid.size);
      ptr += join->mid.size;
    }

    // write node string
    MemoryCopy(ptr, node->string.str, node->string.size);
    ptr += node->string.size;

    is_mid = 1;
  }

  // write post
  MemoryCopy(ptr, join->post.str, join->post.size);
  ptr += join->post.size;

  // write null
  *ptr = 0;

  String8 result = str8(str, size);
  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL String8List
str8_split(Arena *arena, String8 string, U8 *splits, U32 count){
  ProfBeginFunc();

  String8List result = {0};

  U8 *ptr = string.str;
  U8 *word_first = ptr;
  U8 *opl = string.str + string.size;
  for (;ptr < opl; ptr += 1){
    // is this a split
    U8 byte = *ptr;
    B32 is_split_byte = 0;
    for (U32 i = 0; i < count; i += 1){
      if (byte == splits[i]){
        is_split_byte = 1;
        break;
      }
    }

    if (is_split_byte){
      // try to emit word, advance word first pointer
      if (word_first < ptr){
        str8_list_push(arena, &result, str8_range(word_first, ptr));
      }
      word_first = ptr + 1;
    }
  }

  // try to emit final word
  if (word_first < ptr){
    str8_list_push(arena, &result, str8_range(word_first, ptr));
  }

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL String8
str8_pushfv(Arena *arena, char *fmt, va_list args){
  ProfBeginFunc();

  // in case we need to try a second time
  va_list args2;
  va_copy(args2, args);

  // try to build the string in 1024 bytes
  U64 buffer_size = 1024;
  U8 *buffer = push_array(arena, U8, buffer_size);
  U64 actual_size = m4_vsnprintf((char*)buffer, buffer_size, fmt, args);

  String8 result = {0};
  if (actual_size < buffer_size){
    // if first try worked, put back what we didn't use and finish
    arena_pop_amount(arena, buffer_size - actual_size - 1);
    result = str8(buffer, actual_size);
  }
  else{
    // if first try failed, reset and try again with correct size
    arena_pop_amount(arena, buffer_size);
    U8 *fixed_buffer = push_array(arena, U8, actual_size + 1);
    U64 final_size = m4_vsnprintf((char*)fixed_buffer, actual_size + 1, fmt, args2);
    result = str8(fixed_buffer, final_size);
  }

  // end args2
  va_end(args2);

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL String8
str8_pushf(Arena *arena, char *fmt, ...){
  va_list args;
  va_start(args, fmt);
  String8 result = str8_pushfv(arena, fmt, args);
  va_end(args);
  return(result);
}

MR4TH_SYMBOL void
str8_list_pushf(Arena *arena, String8List *list, char *fmt, ...){
  va_list args;
  va_start(args, fmt);
  String8 string = str8_pushfv(arena, fmt, args);
  va_end(args);
  str8_list_push(arena, list, string);
}

MR4TH_SYMBOL String8
str8_push_copy(Arena *arena, String8 string){
  String8 result = {0};
  result.str = push_array(arena, U8, string.size + 1);
  result.size = string.size;
  MemoryCopy(result.str, string.str, string.size);
  result.str[result.size] = 0;
  return(result);
}

// operators

MR4TH_SYMBOL B32
str8_match(String8 a, String8 b, StringMatchFlags flags){
  ProfBeginFunc();

  B32 result = 0;
  if ((flags & StringMatchFlag_PrefixMatch) != 0 || a.size == b.size){
    U64 size = a.size;
    if ((flags & StringMatchFlag_PrefixMatch) != 0){
      size = Min(a.size, b.size);
    }

    result = 1;
    B32 no_case = ((flags & StringMatchFlag_NoCase) != 0);
    for (U64 i = 0; i < size; i += 1){
      U8 ac = a.str[i];
      U8 bc = b.str[i];
      if (no_case){
        ac = str8_char_uppercase(ac);
        bc = str8_char_uppercase(bc);
      }
      if (ac != bc){
        result = 0;
        break;
      }
    }
  }

  ProfEndFunc();
  return(result);
}

// unicode

MR4TH_SYMBOL StringDecode
str_decode_utf8(U8 *str, U32 cap){
  ProfBeginFunc();

  MR4TH_SYM_COMPTIME U8 length[] = {
    1, 1, 1, 1, // 000xx
    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1,
    0, 0, 0, 0, // 100xx
    0, 0, 0, 0,
    2, 2, 2, 2, // 110xx
    3, 3,       // 1110x
    4,          // 11110
    0,          // 11111
  };
  MR4TH_SYM_COMPTIME U8 first_byte_mask[] = { 0, 0x7F, 0x1F, 0x0F, 0x07 };
  MR4TH_SYM_COMPTIME U8 final_shift[] = { 0, 18, 12, 6, 0 };

  StringDecode result = {0};
  if (cap > 0){
    result.codepoint = '#';
    result.size = 1;

    U8 byte = str[0];
    U8 l = length[byte >> 3];
    if (0 < l && l <= cap){
      U32 cp = (byte & first_byte_mask[l]) << 18;
      switch (l){
        case 4: cp |= ((str[3] & 0x3F) << 0);
        case 3: cp |= ((str[2] & 0x3F) << 6);
        case 2: cp |= ((str[1] & 0x3F) << 12);
        default: break;
      }
      cp >>= final_shift[l];

      result.codepoint = cp;
      result.size = l;
    }
  }

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL U32
str_encode_utf8(U8 *dst, U32 codepoint){
  ProfBeginFunc();

  U32 size = 0;
  if (codepoint < (1 << 7)){
    dst[0] = codepoint;
    size = 1;
  }
  else if (codepoint < (1 << 11)){
    dst[0] = 0xC0 | (codepoint >> 6);
    dst[1] = 0x80 | (codepoint & 0x3F);
    size = 2;
  }
  else if (codepoint < (1 << 16)){
    dst[0] = 0xE0 | (codepoint >> 12);
    dst[1] = 0x80 | ((codepoint >> 6) & 0x3F);
    dst[2] = 0x80 | (codepoint & 0x3F);
    size = 3;
  }
  else if (codepoint < (1 << 21)){
    dst[0] = 0xF0 | (codepoint >> 18);
    dst[1] = 0x80 | ((codepoint >> 12) & 0x3F);
    dst[2] = 0x80 | ((codepoint >> 6) & 0x3F);
    dst[3] = 0x80 | (codepoint & 0x3F);
    size = 4;
  }
  else{
    dst[0] = '#';
    size = 1;
  }

  ProfEndFunc();
  return(size);
}

MR4TH_SYMBOL StringDecode
str_decode_utf16(U16 *str, U32 cap){
  ProfBeginFunc();

  StringDecode result = {'#', 1};
  U16 x = str[0];
  if (x < 0xD800 || 0xDFFF < x){
    result.codepoint = x;
  }
  else if (cap >= 2){
    U16 y = str[1];
    if (0xD800 <= x && x < 0xDC00 &&
        0xDC00 <= y && y < 0xE000){
      U16 xj = x - 0xD800;
      U16 yj = y - 0xDc00;
      U32 xy = (xj << 10) | yj;
      result.codepoint = xy + 0x10000;
      result.size = 2;
    }
  }

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL U32
str_encode_utf16(U16 *dst, U32 codepoint){
  ProfBeginFunc();

  U32 size = 0;
  if (codepoint < 0x10000){
    dst[0] = codepoint;
    size = 1;
  }
  else{
    U32 cpj = codepoint - 0x10000;
    dst[0] = (cpj >> 10) + 0xD800;
    dst[1] = (cpj & 0x3FF) + 0xDC00;
    size = 2;
  }

  ProfEndFunc();
  return(size);
}

MR4TH_SYMBOL String32
str32_from_str8(Arena *arena, String8 string){
  ProfBeginFunc();

  U32 *memory = push_array(arena, U32, string.size + 1);

  U32 *dptr = memory;
  U8 *ptr = string.str;
  U8 *opl = string.str + string.size;
  for (; ptr < opl;){
    StringDecode decode = str_decode_utf8(ptr, (U64)(opl - ptr));
    *dptr = decode.codepoint;
    ptr += decode.size;
    dptr += 1;
  }

  *dptr = 0;

  U64 alloc_count = string.size + 1;
  U64 string_count = (U64)(dptr - memory);
  U64 unused_count = alloc_count - string_count - 1;
  arena_pop_amount(arena, unused_count*sizeof(*memory));

  String32 result = {memory, string_count};
  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL String8
str8_from_str32(Arena *arena, String32 string){
  ProfBeginFunc();

  U8 *memory = push_array(arena, U8, string.size*4 + 1);

  U8 *dptr = memory;
  U32 *ptr = string.str;
  U32 *opl = string.str + string.size;
  for (; ptr < opl;){
    U32 size = str_encode_utf8(dptr, *ptr);
    ptr += 1;
    dptr += size;
  }

  *dptr = 0;

  U64 alloc_count = string.size*4 + 1;
  U64 string_count = (U64)(dptr - memory);
  U64 unused_count = alloc_count - string_count - 1;
  arena_pop_amount(arena, unused_count*sizeof(*memory));

  String8 result = {memory, string_count};
  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL String16
str16_from_str8(Arena *arena, String8 string){
  ProfBeginFunc();
  U16 *memory = push_array(arena, U16, string.size*2 + 1);

  U16 *dptr = memory;
  U8 *ptr = string.str;
  U8 *opl = string.str + string.size;
  for (; ptr < opl;){
    StringDecode decode = str_decode_utf8(ptr, (U64)(opl - ptr));
    U32 enc_size = str_encode_utf16(dptr, decode.codepoint);
    ptr += decode.size;
    dptr += enc_size;
  }

  *dptr = 0;

  U64 alloc_count = string.size*2 + 1;
  U64 string_count = (U64)(dptr - memory);
  U64 unused_count = alloc_count - string_count - 1;
  arena_pop_amount(arena, unused_count*sizeof(*memory));

  String16 result = {memory, string_count};
  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL String8
str8_from_str16(Arena *arena, String16 string){
  ProfBeginFunc();
  U8 *memory = push_array(arena, U8, string.size*3 + 1);

  U8 *dptr = memory;
  U16 *ptr = string.str;
  U16 *opl = string.str + string.size;
  for (; ptr < opl;){
    StringDecode decode = str_decode_utf16(ptr, (U64)(opl - ptr));
    U16 enc_size = str_encode_utf8(dptr, decode.codepoint);
    ptr += decode.size;
    dptr += enc_size;
  }

  *dptr = 0;

  U64 alloc_count = string.size*3 + 1;
  U64 string_count = (U64)(dptr - memory);
  U64 unused_count = alloc_count - string_count - 1;
  arena_pop_amount(arena, unused_count*sizeof(*memory));

  String8 result = {memory, string_count};
  ProfEndFunc();
  return(result);
}

// common dumping helpers

MR4TH_SYMBOL String8
str8_join_flags(Arena *arena, String8List *list){
  StringJoin join = {0};
  join.mid = str8_lit(" | ");
  String8 result = str8_join(arena, list, &join);
  if (result.size == 0){
    result = str8_lit("0");
  }
  return(result);
}

// numeric conversion

MR4TH_SYMBOL B32
str8_is_u64(String8 string, U32 radix){
  Assert(2 <= radix && radix <= 16);

  B32 result = 1;
  {
    U8 *ptr = string.str;
    U8 *opl = string.str + string.size;
    for (; ptr < opl; ptr += 1){
      U8 num = 0xFF;

      // extract
      U8 s = *ptr - '0';
      if (s <= 9){
        num = s;
      }
      else{
        s = *ptr - 'A';
        if (s <= 5){
          num = 0xA + s;
        }
        else{
          s = *ptr - 'a';
          if (s <= 5){
            num = 0xA + s;
          }
        }
      }

      // bad parse check
      if (num >= radix){
        result = 0;
        break;
      }
    }
  }

  return(result);
}

MR4TH_SYMBOL U64
u64_from_str8(String8 string, U32 radix){
  Assert(2 <= radix && radix <= 16);

  U64 result = 0;
  {
    U8 *ptr = string.str;
    U8 *opl = string.str + string.size;
    for (; ptr < opl; ptr += 1){
      U8 num = 0xFF;

      // extract
      U8 s = *ptr - '0';
      if (s <= 9){
        num = s;
      }
      else{
        s = *ptr - 'A';
        if (s <= 5){
          num = 0xA + s;
        }
        else{
          s = *ptr - 'a';
          if (s <= 5){
            num = 0xA + s;
          }
        }
      }

      // bad parse check
      if (num >= radix){
        result = 0;
        break;
      }

      // increment result
      result *= radix;
      result += num;
    }
  }

  return(result);
}

MR4TH_SYMBOL U64
u64_from_str8_c_syntax(String8 string){
  U8 *ptr = string.str;
  U8 *opl = string.str + string.size;

  U32 radix = 10;
  if (ptr < opl && *ptr == '0'){
    radix = 010;
    ptr += 1;
    if (ptr < opl && *ptr == 'x'){
      radix = 0x10;
      ptr += 1;
    }
    else if (ptr < opl && *ptr == 'b'){
      radix = 2;
      ptr += 1;
    }
  }

  U64 result = u64_from_str8(str8_range(ptr, opl), radix);
  return(result);
}

MR4TH_SYMBOL S64
s64_from_str8_c_syntax(String8 string){
  U8 *ptr = string.str;
  U8 *opl = string.str + string.size;

  B32 negative = 0;
  if (ptr < opl){
    if (*ptr == '-'){
      negative = 1;
      ptr += 1;
    }
    else if (*ptr == '+'){
      ptr += 1;
    }
  }

  S64 result = u64_from_str8_c_syntax(str8_range(ptr, opl));
  if (negative){
    result = -result;
  }

  return(result);
}

MR4TH_SYMBOL F64
f64_from_str8(String8 string){
  F64 result = 0.f;

  U8 *ptr = string.str;
  U8 *opl = string.str + string.size;

  B32 negative = 0;
  if (ptr < opl){
    if (*ptr == '-'){
      negative = 1;
      ptr += 1;
    }
    else if (*ptr == '+'){
      ptr += 1;
    }
  }

  for (; ptr < opl; ptr += 1){
    if (*ptr == '.'){
      ptr += 1;
      break;
    }
    U32 x = 0;
    if ('0' <= *ptr && *ptr <= '9'){
      x = *ptr - '0';
    }
    result *= 10.f;
    result += x;
  }

  F32 mul = 0.1f;
  for (; ptr < opl; ptr += 1){
    U32 x = 0;
    if ('0' <= *ptr && *ptr <= '9'){
      x = *ptr - '0';
    }
    result += x*mul;
    mul /= 10.f;
  }

  if (negative){
    result = -result;
  }

  return(result);
}

// path helpers

MR4TH_SYMBOL String8
str8_file_name_from_path(String8 full_file_name){
  String8 result = {0};
  if (full_file_name.size > 0){
    U8 *opl = full_file_name.str + full_file_name.size;
    U8 *ptr = opl;
    for (;ptr > full_file_name.str; ptr -= 1){
      U8 c = ptr[-1];
      if (c == '/' || c == '\\'){
        break;
      }
    }
    result = str8_range(ptr, opl);
  }
  return(result);
}

MR4TH_SYMBOL String8
str8_base_name_from_file_name(String8 file_name){
  String8 result = {0};
  if (file_name.size > 0){
    U8 *opl = file_name.str + file_name.size;
    U8 *ptr = file_name.str;
    for (;ptr < opl; ptr += 1){
      U8 c = ptr[0];
      if (c == '.'){
        break;
      }
    }
    result = str8_range(file_name.str, ptr);
  }
  return(result);
}

////////////////////////////////
// Functions: Command Line Parsing

MR4TH_SYM_COMPTIME MR4TH_READ_ONLY
CMDLN_Params cmdln_params_nil = {0};

MR4TH_SYMBOL CMDLN*
cmdln_from_args(Arena *arena, String8List *args){
  CMDLN *cmdln = push_array(arena, CMDLN, 1);
  cmdln->raw = str8_list_copy(arena, args);

  String8Node *node = cmdln->raw.first;

  // first string is 'program'
  if (node != 0){
    cmdln->program = node->string;
    node = node->next;
  }

  // consume string nodes
  B32 forced_input = 0;
  for (;node != 0;){
    String8 string = str8_skip_chop_whitespace(node->string);
    node = node->next;

    // check if argument is a flag
    B32 is_flag = 0;
    if (!forced_input){
      is_flag = (string.size != 0 && string.str[0] == '-');
    }

    // parse flag
    if (is_flag){

      // long flag
      B32 is_long_flag = (string.size > 1 && string.str[1] == '-');
      if (is_long_flag){

        // end 'normal' mode
        B32 double_dash = (string.size == 2);
        if (double_dash){
          forced_input = 1;
        }

        // parse long flag
        if (!double_dash){
          String8 flag_whole = str8_skip(string, 2);

          // parameter delimter
          U64 delim = flag_whole.size;
          for (U8 *ptr = flag_whole.str, *opl = flag_whole.str + flag_whole.size;
               ptr < opl; ptr += 1){
            if (*ptr == '=' || *ptr == ':'){
              delim = (U64)(ptr - flag_whole.str);
              break;
            }
          }

          // split flag at delimiter
          String8 flag_name = str8_prefix(flag_whole, delim);
          String8 flag_param = str8_skip(flag_whole, delim + 1);

          // if have a param delimiter at end
          // then use the next argument as the flag_param
          if (delim == flag_whole.size - 1){
            if (node != 0){
              flag_param = node->string;
              node = node->next;
            }
          }

          // parse parameters
          CMDLN_Params *params = cmdln_params_from_string(arena, flag_param);

          // store flag node
          {
            CMDLN_Node *cmdlnnode = push_array(arena, CMDLN_Node, 1);
            SLLQueuePush(cmdln->first, cmdln->last, cmdlnnode);
            cmdlnnode->string = flag_name;
            cmdlnnode->params = params;
            cmdln->flag_count += 1;
          }
        }
      }

      // short flags
      if (!is_long_flag){
        String8 short_flags = str8_skip(string, 1);

        U8 *flag = short_flags.str;
        U8 *flagopl = short_flags.str + short_flags.size;
        for (; flag < flagopl; flag += 1){
          U8 *flagptr = flag;

          // check for parameters
          String8 flag_param = {0};
          if (flag + 1 < flagopl &&
              (flag[1] == '=' || flag[1] == ':')){
            flag_param = str8_range(flag + 2, flagopl);
            if (flag_param.size == 0){
              if (node != 0){
                flag_param = node->string;
                node = node->next;
              }
            }

            // kill the flag loop after finding parameters
            flag = flagopl;
          }

          // parse parameters
          CMDLN_Params *params = cmdln_params_from_string(arena, flag_param);

          // store flag node
          {
            CMDLN_Node *cmdlnnode = push_array(arena, CMDLN_Node, 1);
            SLLQueuePush(cmdln->first, cmdln->last, cmdlnnode);
            cmdlnnode->string = str8(flagptr, 1);
            cmdlnnode->params = params;
            cmdln->flag_count += 1;
          }
        }
      }
    }

    // parse input
    if (!is_flag){
      String8 input_string = string;

      // store input node
      {
        CMDLN_Node *cmdlnnode = push_array(arena, CMDLN_Node, 1);
        SLLQueuePush(cmdln->first, cmdln->last, cmdlnnode);
        cmdlnnode->string = input_string;
        cmdln->input_count += 1;
      }
    }
  }

  // pointer arrays
  cmdln->inputs = push_array(arena, CMDLN_Node*, cmdln->input_count);
  cmdln->flags  = push_array(arena, CMDLN_Node*, cmdln->flag_count);
  {
    CMDLN_Node **inputptr = cmdln->inputs;
    CMDLN_Node **flagptr  = cmdln->flags;
    for (CMDLN_Node *node = cmdln->first;
         node != 0;
         node = node->next){
      if (node->params == 0){
        *inputptr = node;
        inputptr += 1;
      }
      else{
        *flagptr = node;
        flagptr += 1;
      }
    }
  }

  return(cmdln);
}

MR4TH_SYMBOL CMDLN_Params*
cmdln_params_from_string(Arena *arena, String8 flag_param){
  CMDLN_Params *params = &cmdln_params_nil;
  if (flag_param.size > 0){
    params = push_array(arena, CMDLN_Params, 1);
    params->raw = flag_param;
    params->list = str8_split(arena, flag_param, (U8*)",", 1);
  }
  return(params);
}

MR4TH_SYMBOL U64
cmdln_input_count(CMDLN *cmdln){
  return(cmdln->input_count);
}

MR4TH_SYMBOL String8
cmdln_input_from_idx(CMDLN *cmdln, U64 idx){
  String8 result = {0};
  if (idx < cmdln->input_count){
    result = cmdln->inputs[idx]->string;
  }
  return(result);
}

MR4TH_SYMBOL U64
cmdln_flag_count(CMDLN *cmdln){
  return(cmdln->flag_count);
}

MR4TH_SYMBOL CMDLN_Flag*
cmdln_flag_from_idx(CMDLN *cmdln, U64 idx){
  CMDLN_Flag *result = 0;
  if (idx < cmdln->flag_count){
    result = cmdln->flags[idx];
  }
  return(result);
}

MR4TH_SYMBOL CMDLN_Params*
cmdln_get_params(CMDLN *cmdln, String8 flagstr, char abbrev){
  CMDLN_Params *result = 0;
  for (CMDLN_Node *node = cmdln->first;
       node != 0;
       node = node->next){
    if (node->params != 0){
      if (str8_match(flagstr, node->string, 0) ||
          (node->string.size == 1 && node->string.str[0] == (U8)abbrev)){
        result = node->params;
        break;
      }
    }
  }
  return(result);
}

MR4TH_SYMBOL B32
cmdln_has_flag(CMDLN *cmdln, String8 flagstr, char abbrev){
  CMDLN_Params *params = cmdln_get_params(cmdln, flagstr, abbrev);
  B32 result = (params != 0);
  return(result);
}

MR4TH_SYMBOL String8
cmdln_get_str8(CMDLN *cmdln, String8 flagstr, char abbrev){
  CMDLN_Params *params = cmdln_get_params(cmdln, flagstr, abbrev);
  String8 result = {0};
  if (params != 0){
    result = params->raw;
  }
  return(result);
}

MR4TH_SYMBOL S64
cmdln_get_s64(CMDLN *cmdln, String8 flagstr, char abbrev){
  String8 str = cmdln_get_str8(cmdln, flagstr, abbrev);
  S64 result = cmdln_s64_from_str8(str);
  return(result);
}

MR4TH_SYMBOL F64
cmdln_get_f64(CMDLN *cmdln, String8 flagstr, char abbrev){
  String8 str = cmdln_get_str8(cmdln, flagstr, abbrev);
  F64 result = cmdln_f64_from_str8(str);
  return(result);
}

MR4TH_SYMBOL S64
cmdln_s64_from_str8(String8 valstr){
  S64 result = s64_from_str8_c_syntax(valstr);
  return(result);
}

MR4TH_SYMBOL F64
cmdln_f64_from_str8(String8 valstr){
  F64 result = f64_from_str8(valstr);
  return(result);
}

MR4TH_SYMBOL void
cmdln_dump(Arena *arena, String8List *out, CMDLN *cmdln, U32 indent){
  // raw
  str8_list_pushf(arena, out, "%Nraw:\n", indent);
  for (String8Node *node = cmdln->raw.first;
       node != 0;
       node = node->next){
    str8_list_pushf(arena, out, "%N%S\n", indent + 1, node->string);
  }

  // program
  str8_list_pushf(arena, out, "%Nprogram: %S\n", indent, cmdln->program);

  // input nodes
  str8_list_pushf(arena, out, "%Nnodes:\n", indent);
  for (CMDLN_Node *cmdlnnode = cmdln->first;
       cmdlnnode != 0;
       cmdlnnode = cmdlnnode->next){
    if (cmdlnnode->params == 0){
      str8_list_pushf(arena, out, "%N[input] %S\n", indent + 1, cmdlnnode->string);
    }
    else{
      str8_list_pushf(arena, out, "%N[flag ] %S\n", indent + 1, cmdlnnode->string);
      for (String8Node *node = cmdlnnode->params->list.first;
           node != 0;
           node = node->next){
        str8_list_pushf(arena, out, "%N[param] %S\n", indent + 2, node->string);
      }
    }
  }
}

////////////////////////////////
// Functions: Sort

MR4TH_SYMBOL void
sort_merge(void *data_raw, U32 itemsize, U32 count,
           SORT_Compare *compare, void *udata){
  ArenaTemp scratch = arena_get_scratch(0, 0);

  typedef struct SortRange{
    struct SortRange *next;
    U32 first;
    U32 opl;
  } SortRange;

  U8 *data = (U8*)data_raw;

  // identify already-sorted ranges
  SortRange *first_range = 0;
  SortRange *last_range = 0;
  {
    U64 i = 0;
    for (;i < count;){
      U64 first = i;
      U8 *ptr = data + itemsize*i;
      for (i += 1; i < count; i += 1, ptr += itemsize){
        if (compare(ptr + itemsize, ptr, udata) < 0){
          break;
        }
      }
      U64 opl = i;

      SortRange *range = push_array(scratch.arena, SortRange, 1);
      SLLQueuePush(first_range, last_range, range);
      range->first = first;
      range->opl = opl;
    }
  }

  // setup a swap buffer
  U8 *swap_data = push_array(scratch.arena, U8, count*itemsize);

  // sort from src to dst by merging
  U8 *src_data = data;
  U8  *dst_data = swap_data;
  for (;;){
    // check if we're done
    if (first_range == last_range){
      break;
    }

    // transfer ranges to a stack and reset the queue
    SortRange *range_stack = first_range;
    first_range = 0;
    last_range = 0;

    // merge neighboring ranges
    for (;;){
      // range popping
      SortRange *range1 = range_stack;
      if (range1 == 0){
        break;
      }
      SLLStackPop(range_stack);
      SortRange *range2 = range_stack;
      if (range2 == 0){
        U32 rfirst = range1->first;
        MemoryCopy(dst_data + rfirst*itemsize, src_data + rfirst*itemsize,
                   itemsize*(range1->opl - rfirst));
        SLLQueuePush(first_range, last_range, range1);
        break;
      }
      SLLStackPop(range_stack);

      Assert(range1->opl == range2->first);

      // array merge
      U64 i      = range1->first;
      U64 j1     = range1->first;
      U64 j1_opl = range1->opl;
      U64 j2     = range2->first;
      U64 j2_opl = range2->opl;

      for (;j1 < j1_opl && j2 < j2_opl;){
        U8* o1 = src_data + j1*itemsize;
        U8* o2 = src_data + j2*itemsize;
        if (compare(o1, o2, udata) <= 0){
          MemoryCopy(dst_data + i*itemsize, o1, itemsize);
          j1 += 1;
        }
        else{
          MemoryCopy(dst_data + i*itemsize, o2, itemsize);
          j2 += 1;
        }
        i += 1;
      }
      for (;j1 < j1_opl;){
        MemoryCopy(dst_data + i*itemsize, src_data + j1*itemsize, itemsize);
        j1 += 1;
        i += 1;
      }
      for (;j2 < j2_opl;){
        MemoryCopy(dst_data + i*itemsize, src_data + j2*itemsize, itemsize);
        j2 += 1;
        i += 1;
      }

      // combine into a single range on range queue
      range1->opl = range2->opl;
      SLLQueuePush(first_range, last_range, range1);
    }

    // after a pass swap src & dst
    Swap(U8*, src_data, dst_data);
  }

  // transfer data if it's in the wrong buffer
  if (src_data != data){
    MemoryCopy(data, src_data, itemsize*count);
  }

  arena_release_scratch(&scratch);
}

////////////////////////////////
// Functions: Log

#if MR4TH_DEFINE_RUNTIME_SYMBOLS

MR4TH_SYM_COMPTIME MR4TH_THREADVAR
LOG_ThreadVars *log_vars = 0;

MR4TH_SYM_RUNTIME void
log_accum_begin(LOG_LogToProc *proc, void *uptr){
  LOG_ThreadVars *vars = log_vars;
  if (vars == 0){
    Arena *arena = arena_alloc();
    vars = log_vars = push_array(arena, LOG_ThreadVars, 1);
    vars->arena = arena;
  }

  U64 pos = arena_current_pos(vars->arena);
  LOG_Node *node = push_array(vars->arena, LOG_Node, 1);
  node->pos = pos;
  node->logto = proc;
  node->uptr = uptr;
  SLLStackPush(vars->stack, node);
}

MR4TH_SYM_RUNTIME B32
log_gathering(void){
  LOG_ThreadVars *vars = log_vars;
  return(vars != 0 && vars->stack != 0);
}

MR4TH_SYM_RUNTIME void
log_emit(String8 message){
  LOG_ThreadVars *vars = log_vars;
  if (vars != 0 && vars->stack != 0){
    LOG_Node *node = vars->stack;
    String8 msg_copy = str8_push_copy(vars->arena, message);
    str8_list_push(vars->arena, &node->log, msg_copy);
    if (node->logto != 0){
      node->logto(node->uptr, msg_copy);
    }
  }
}

MR4TH_SYM_RUNTIME void
log_emitf(char *fmt, ...){
  LOG_ThreadVars *vars = log_vars;
  if (vars != 0 && vars->stack != 0){
    LOG_Node *node = vars->stack;
    va_list args;
    va_start(args, fmt);
    String8 string = str8_pushfv(vars->arena, fmt, args);
    va_end(args);
    str8_list_push(vars->arena, &node->log, string);
    if (node->logto != 0){
      node->logto(node->uptr, string);
    }
  }
}

MR4TH_SYM_RUNTIME String8
log_accum_end(Arena *arena){
  String8 result = {0};
  LOG_ThreadVars *vars = log_vars;
  if (vars != 0){
    LOG_Node *node = vars->stack;
    if (node != 0){
      result = str8_join(arena, &node->log, 0);
      SLLStackPop(vars->stack);
      arena_pop_to(vars->arena, node->pos);
    }
  }
  return(result);
}

#else /* !MR4TH_DEFINE_RUNTIME_SYMBOLS */

MR4TH_SHARED_FUNC_INITIALIZER(log_accum_begin);
MR4TH_SHARED_FUNC_INITIALIZER(log_gathering);
MR4TH_SHARED_FUNC_INITIALIZER(log_emit);
MR4TH_SHARED_FUNC_INITIALIZER(log_emitf);
MR4TH_SHARED_FUNC_INITIALIZER(log_accum_end);

#endif

////////////////////////////////
// Functions: Errors

// IMPORTANT: It is important that strings get pushed
// *on top* of the node that holds them within the arena.
// basically: Assert((U8*)node < (U8*)node->error.str);
// except that stops working if the arena is chained...

#if MR4TH_DEFINE_RUNTIME_SYMBOLS

MR4TH_SYM_COMPTIME MR4TH_THREADVAR
ER_ThreadVars *er_vars = 0;

MR4TH_SYMBOL void
er_accum_begin(void){
  ER_ThreadVars *vars = er_vars;
  if (vars == 0){
    Arena *arena = arena_alloc_reserve(KB(64), 0);
    vars = er_vars = push_array(arena, ER_ThreadVars, 1);
    vars->arena = arena;
  }

  U64 pos = arena_current_pos(vars->arena);
  ER_Node *node = push_array(vars->arena, ER_Node, 1);
  if (node == 0){
    vars->over_stack += 1;
  }
  else{
    node->pos = pos;
    SLLStackPush(vars->stack, node);
  }
}

MR4TH_SYMBOL void
er_emit(String8 error){
  ER_ThreadVars *vars = er_vars;
  if (vars != 0 &&
      vars->over_stack == 0){
    ER_Node *node = vars->stack;
    if (node != 0 && node->error.size == 0){
      node->error = str8_push_copy(vars->arena, error);
    }
  }
}

MR4TH_SYMBOL void
er_emitf(char *fmt, ...){
  ER_ThreadVars *vars = er_vars;
  if (vars != 0 &&
      vars->over_stack == 0){
    ER_Node *node = vars->stack;
    if (node != 0 && node->error.size == 0){
      va_list args;
      va_start(args, fmt);
      String8 string = str8_pushfv(vars->arena, fmt, args);
      va_end(args);
      node->error = string;
    }
  }
}

MR4TH_SYMBOL String8
er_accum_end(Arena *arena){
  String8 result = {0};
  ER_ThreadVars *vars = er_vars;
  if (vars != 0){
    if (vars->over_stack == 0){
      ER_Node *node = vars->stack;
      if (node != 0){
        result = str8_push_copy(arena, node->error);
        SLLStackPop(vars->stack);
        arena_pop_to(vars->arena, node->pos);
      }
    }
    else{
      vars->over_stack -= 1;
    }
  }
  return(result);
}

#else /* MR4TH_DEFINE_RUNTIME_SYMBOLS */

MR4TH_SHARED_FUNC_INITIALIZER(er_accum_begin);
MR4TH_SHARED_FUNC_INITIALIZER(er_emit);
MR4TH_SHARED_FUNC_INITIALIZER(er_emitf);
MR4TH_SHARED_FUNC_INITIALIZER(er_accum_end);

#endif



////////////////////////////////////////////////
////////////////////////////////////////////////
////////////   stdio.h EXTENSIONS   ////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

#ifdef MR4TH_BASE_STDIO_H

////////////////////////////////
// Functions: File Handle Printf

MR4TH_SYMBOL void
m4_printf(char *fmt, ...){
  ArenaTemp scratch = arena_get_scratch(0, 0);
  va_list args;
  va_start(args, fmt);
  String8 str = str8_pushfv(scratch.arena, fmt, args);
  va_end(args);
  fwrite(str.str, 1, str.size, stdout);
  arena_release_scratch(&scratch);
}

MR4TH_SYMBOL void
m4_fprintf(FILE *file, char *fmt, ...){
  ArenaTemp scratch = arena_get_scratch(0, 0);
  va_list args;
  va_start(args, fmt);
  String8 str = str8_pushfv(scratch.arena, fmt, args);
  va_end(args);
  fwrite(str.str, 1, str.size, file);
  arena_release_scratch(&scratch);
}

MR4TH_SYMBOL void
m4_print_str8list(String8List *out){
  m4_fprint_str8list(stdout, out);
}

MR4TH_SYMBOL void
m4_fprint_str8list(FILE *file, String8List *out){
  for (String8Node *node = out->first;
       node != 0;
       node = node->next){
    fwrite(node->string.str, node->string.size, 1, file);
  }
}

////////////////////////////////
// LogToProc for (FILE*)

MR4TH_SYMBOL void
cstd_logto_file_handle(void *uptr, String8 str){
  fwrite(str.str, 1, str.size, (FILE*)uptr);
}

#endif


////////////////////////////////////////////////
////////////////////////////////////////////////
////////   OS IMPLEMENTATION SHARED   //////////
////////////////////////////////////////////////
////////////////////////////////////////////////

////////////////////////////////
// Helper Implementation: File Handling

MR4TH_SYMBOL B32
os_file_write(String8 file_name, String8 data){
  String8Node node = {0};
  node.string = data;
  B32 result = os_file_write_list(file_name, &node);
  return(result);
}



////////////////////////////////////////////////
////////////////////////////////////////////////
/////////   OS IMPLEMENTATION WIN32   //////////
////////////////////////////////////////////////
////////////////////////////////////////////////

#if OS_WINDOWS

////////////////////////////////
// Win32 Implementation: Global Variables

MR4TH_SYM_COMPTIME U64 w32_ticks_per_second = 1;

MR4TH_SYM_COMPTIME Arena *w32_perm_arena = 0;
MR4TH_SYM_COMPTIME String8 w32_binary_path = {0};
MR4TH_SYM_COMPTIME String8 w32_user_path = {0};
MR4TH_SYM_COMPTIME String8 w32_temp_path = {0};

MR4TH_SYM_COMPTIME String8List w32_cmd_line = {0};

////////////////////////////////
// Win32 Implementation: Process Setup

MR4TH_SYMBOL void
os_main_init(int argc, char **argv){
  ProfBeginFunc();

  // setup precision time
  LARGE_INTEGER perf_freq = {0};
  if (QueryPerformanceFrequency(&perf_freq)){
    w32_ticks_per_second = ((U64)perf_freq.HighPart << 32) | perf_freq.LowPart;
  }
  timeBeginPeriod(1);

  // arena
  w32_perm_arena = arena_alloc();

  // command line arguments
  for (int i = 0; i < argc; i += 1){
    String8 arg = str8_cstring((U8*)argv[i]);
    str8_list_push(w32_perm_arena, &w32_cmd_line, arg);
  }

  // paths
  ArenaTemp scratch = arena_get_scratch(0, 0);

  // binary path
  {
    DWORD cap = 2048;
    U16 *buffer = 0;
    DWORD size = 0;
    for (U64 r = 0; r < 4; r += 1, cap *= 4){
      U16 *try_buffer = push_array(scratch.arena, U16, cap);
      DWORD try_size = GetModuleFileNameW(0, (WCHAR*)try_buffer, cap);
      if (try_size == cap && GetLastError() == ERROR_INSUFFICIENT_BUFFER){
        arena_end_temp(&scratch);
      }
      else{
        buffer = try_buffer;
        size = try_size;
        break;
      }
    }

    String8 full_path = str8_from_str16(scratch.arena, str16(buffer, size));
    String8 binary_path = str8_chop_last_slash(full_path);
    w32_binary_path = str8_push_copy(w32_perm_arena, binary_path);
  }

  // user data
  {
    HANDLE token = GetCurrentProcessToken();
    DWORD cap = 2048;
    U16 *buffer = push_array(scratch.arena, U16, cap);
    if (!GetUserProfileDirectoryW(token, (WCHAR*)buffer, &cap)){
      arena_end_temp(&scratch);
      buffer = push_array(scratch.arena, U16, cap + 1);
      if (GetUserProfileDirectoryW(token, (WCHAR*)buffer, &cap)){
        buffer = 0;
      }
    }

    if (buffer != 0){
      // the docs make it sound like we can only count on
      // cap getting the size on failure; so we're just going to cstring
      // this to be safe.
      w32_user_path = str8_from_str16(w32_perm_arena, str16_cstring(buffer));
    }
  }

  // temp data
  {
    DWORD cap = 2048;
    U16 *buffer = push_array(scratch.arena, U16, cap);
    DWORD size = GetTempPathW(cap, (WCHAR*)buffer);
    if (size >= cap){
      arena_end_temp(&scratch);
      buffer = push_array(scratch.arena, U16, size + 1);
      size = GetTempPathW(size + 1, (WCHAR*)buffer);
    }

    // size - 1, because this particular string function
    // in the Win32 API is different from the others and it includes
    // the trailing backslash. We want consistency, so the "- 1" removes it.
    w32_temp_path = str8_from_str16(w32_perm_arena, str16(buffer, size - 1));
  }

  arena_release_scratch(&scratch);

  ProfEndFunc();
}

MR4TH_SYMBOL String8List
os_command_line_arguments(void){
  String8List result = w32_cmd_line;
  return(result);
}

MR4TH_SYMBOL void
os_exit_process(U32 code){
  ProfClose();
  ExitProcess(code);
}

////////////////////////////////
// Win32 Implementation: Memory Functions

MR4TH_SYMBOL void*
os_memory_reserve(U64 size){
  void *result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
  return(result);
}

MR4TH_SYMBOL B32
os_memory_commit(void *ptr, U64 size){
  B32 result = (VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != 0);
  return(result);
}

MR4TH_SYMBOL void
os_memory_decommit(void *ptr, U64 size){
  VirtualFree(ptr, size, MEM_DECOMMIT);
}

MR4TH_SYMBOL void
os_memory_release(void *ptr, U64 size){
  VirtualFree(ptr, 0, MEM_RELEASE);
}

////////////////////////////////
// Win32 Implementation: File Handling

MR4TH_SYMBOL String8
os_file_read(Arena *arena, String8 file_name){
  ProfBeginFunc();

  // get handle
  ArenaTemp scratch = arena_get_scratch(&arena, 1);
  String16 file_name16 = str16_from_str8(scratch.arena, file_name);
  HANDLE file = CreateFileW((WCHAR*)file_name16.str,
                            GENERIC_READ, 0, 0,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                            0);

  String8 result = {0};
  if (file != INVALID_HANDLE_VALUE){
    // get size
    DWORD hi_size = 0;
    DWORD lo_size = GetFileSize(file, &hi_size);
    U64 total_size = (((U64)hi_size) << 32) | (U64)lo_size;

    // allocate buffer
    ArenaTemp restore_point = arena_begin_temp(arena);
    U8 *buffer = push_array_no_zero(arena, U8, total_size + 1);

    // read
    U8 *ptr = buffer;
    U8 *opl = buffer + total_size;
    B32 success = 1;
    for (;ptr < opl;){
      U64 total_to_read = (U64)(opl - ptr);
      DWORD to_read = (DWORD)total_to_read;
      if (total_to_read > max_U32){
        to_read = max_U32;
      }
      DWORD actual_read = 0;
      if (!ReadFile(file, ptr, to_read, &actual_read, 0)){
        success = 0;
        break;
      }
      ptr += actual_read;
    }

    // set result or reset memory
    if (success){
      buffer[total_size] = 0;
      result.str = buffer;
      result.size = total_size;
    }
    else{
      arena_end_temp(&restore_point);
    }

    CloseHandle(file);
  }

  arena_release_scratch(&scratch);

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL B32
os_file_write_list(String8 file_name, String8Node *first_node){
  ProfBeginFunc();

  // get handle
  ArenaTemp scratch = arena_get_scratch(0, 0);
  String16 file_name16 = str16_from_str8(scratch.arena, file_name);
  HANDLE file = CreateFileW((WCHAR*)file_name16.str,
                            GENERIC_WRITE, 0, 0,
                            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                            0);

  B32 result = 0;
  if (file != INVALID_HANDLE_VALUE){
    result = 1;

    for (String8Node *node = first_node;
         node != 0;
         node = node->next){
      U8 *ptr = node->string.str;
      U8 *opl = ptr + node->string.size;
      for (;ptr < opl;){
        U64 total_to_write = (U64)(opl - ptr);
        DWORD to_write = total_to_write;
        if (total_to_write > max_U32){
          to_write = max_U32;
        }
        DWORD actual_write = 0;
        if (!WriteFile(file, ptr, to_write, &actual_write, 0)){
          result = 0;
          goto dblbreak;
        }
        ptr += actual_write;
      }
    }
    dblbreak:;

    CloseHandle(file);
  }

  arena_release_scratch(&scratch);

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL FileProperties
os_file_properties(String8 file_name){
  ProfBeginFunc();

  // convert name
  ArenaTemp scratch = arena_get_scratch(0, 0);
  String16 file_name16 = str16_from_str8(scratch.arena, file_name);

  // get attribs and convert to properties
  FileProperties result = {0};
  WIN32_FILE_ATTRIBUTE_DATA attribs = {0};
  if (GetFileAttributesExW((WCHAR*)file_name16.str, GetFileExInfoStandard,
                           &attribs)){
    result.size = ((U64)attribs.nFileSizeHigh << 32) | (U64)attribs.nFileSizeLow;
    result.flags = w32_prop_flags_from_attribs(attribs.dwFileAttributes);
    result.create_time = w32_dense_time_from_file_time(&attribs.ftCreationTime);
    result.modify_time = w32_dense_time_from_file_time(&attribs.ftLastWriteTime);
    result.access = w32_access_from_attributes(attribs.dwFileAttributes);
  }

  arena_release_scratch(&scratch);

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL B32
os_file_delete(String8 file_name){
  ProfBeginFunc();

  // convert name
  ArenaTemp scratch = arena_get_scratch(0, 0);
  String16 file_name16 = str16_from_str8(scratch.arena, file_name);
  // delete file
  B32 result = DeleteFileW((WCHAR*)file_name16.str);
  arena_release_scratch(&scratch);

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL B32
os_file_rename(String8 og_name, String8 new_name){
  ProfBeginFunc();

  // convert name
  ArenaTemp scratch = arena_get_scratch(0, 0);
  String16 og_name16 = str16_from_str8(scratch.arena, og_name);
  String16 new_name16 = str16_from_str8(scratch.arena, new_name);
  // rename file
  B32 result = MoveFileW((WCHAR*)og_name16.str, (WCHAR*)new_name16.str);
  arena_release_scratch(&scratch);

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL B32
os_file_make_directory(String8 path){
  ProfBeginFunc();

  // convert name
  ArenaTemp scratch = arena_get_scratch(0, 0);
  String16 path16 = str16_from_str8(scratch.arena, path);
  // make directory
  B32 result = CreateDirectoryW((WCHAR*)path16.str, 0);
  arena_release_scratch(&scratch);

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL B32
os_file_delete_directory(String8 path){
  ProfBeginFunc();

  // convert name
  ArenaTemp scratch = arena_get_scratch(0, 0);
  String16 path16 = str16_from_str8(scratch.arena, path);
  // make directory
  B32 result = RemoveDirectoryW((WCHAR*)path16.str);
  arena_release_scratch(&scratch);

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL OS_FileIter
os_file_iter_init(String8 path){
  ProfBeginFunc();

  // convert name
  String8Node nodes[2];
  String8List list = {0};
  str8_list_push_explicit(&list, path, nodes + 0);
  str8_list_push_explicit(&list, str8_lit("\\*"), nodes + 1);
  ArenaTemp scratch = arena_get_scratch(0, 0);
  String8 path_star = str8_join(scratch.arena, &list, 0);
  // TODO(allen): Better unicode conversions here
  String16 path16 = str16_from_str8(scratch.arena, path_star);

  // store into iter
  OS_FileIter result = {0};
  W32_FileIter *w32_iter = (W32_FileIter*)&result;
  w32_iter->handle = FindFirstFileW((WCHAR*)path16.str, &w32_iter->find_data);
  arena_release_scratch(&scratch);

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL B32
os_file_iter_next(Arena *arena, OS_FileIter *iter,
                  String8 *name, FileProperties *props){
  ProfBeginFunc();

  B32 result = 0;

  W32_FileIter *w32_iter = (W32_FileIter*)iter;
  if (w32_iter->handle != 0 &&
      w32_iter->handle != INVALID_HANDLE_VALUE){
    for (;!w32_iter->done;){
      // check for . and ..
      WCHAR *file_name = w32_iter->find_data.cFileName;
      B32 is_dot = (file_name[0] == '.' && file_name[1] == 0);
      B32 is_dotdot = (file_name[0] == '.' && file_name[1] == '.' &&
                       file_name[2] == 0);

      // setup to emit
      B32 emit = (!is_dot && !is_dotdot);
      WIN32_FIND_DATAW data = {0};
      if (emit){
        MemoryCopyStruct(&data, &w32_iter->find_data);
      }

      // increment the iterator
      if (!FindNextFileW(w32_iter->handle, &w32_iter->find_data)){
        w32_iter->done = 1;
      }

      // do the emit if we saved one earlier
      if (emit){
        *name = str8_from_str16(arena, str16_cstring((U16*)data.cFileName));
        props->size = ((U64)data.nFileSizeHigh << 32) | (U64)data.nFileSizeLow;
        props->flags = w32_prop_flags_from_attribs(data.dwFileAttributes);
        props->create_time = w32_dense_time_from_file_time(&data.ftCreationTime);
        props->modify_time = w32_dense_time_from_file_time(&data.ftLastWriteTime);
        props->access = w32_access_from_attributes(data.dwFileAttributes);
        result = 1;
        break;
      }
    }
  }

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL void
os_file_iter_end(OS_FileIter *iter){
  ProfBeginFunc();

  W32_FileIter *w32_iter = (W32_FileIter*)iter;
  if (w32_iter->handle != 0 &&
      w32_iter->handle != INVALID_HANDLE_VALUE){
    FindClose(w32_iter->handle);
  }

  ProfEndFunc();
}

MR4TH_SYMBOL String8
os_file_path(Arena *arena, OS_SystemPath path){
  ProfBeginFunc();

  String8 result = {0};

  switch (path){
    case OS_SystemPath_CurrentDirectory:
    {
      ArenaTemp scratch = arena_get_scratch(&arena, 1);
      DWORD cap = 2048;
      U16 *buffer = push_array(scratch.arena, U16, cap);
      DWORD size = GetCurrentDirectoryW(cap, (WCHAR*)buffer);
      if (size >= cap){
        arena_end_temp(&scratch);
        buffer = push_array(scratch.arena, U16, size + 1);
        size = GetCurrentDirectoryW(size + 1, (WCHAR*)buffer);
      }
      result = str8_from_str16(arena, str16(buffer, size));
      arena_release_scratch(&scratch);
    }break;

    case OS_SystemPath_Binary:
    {
      result = str8_push_copy(arena, w32_binary_path);
    }break;

    case OS_SystemPath_UserData:
    {
      result = str8_push_copy(arena, w32_user_path);
    }break;

    case OS_SystemPath_TempData:
    {
      result = str8_push_copy(arena, w32_temp_path);
    }break;
  }

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL String8
os_get_absolute_path_from_str8_list(Arena *arena, String8List rel_path_parts){
  String8 result = (String8){0};
  Assert(!"TODO");
  return result;
}

MR4TH_SYMBOL String8
os_get_absolute_path(Arena *arena, String8 rel_path){
  String8 result = (String8){0};
  Assert(!"TODO");
  return result;
}

MR4TH_SYMBOL void
os_set_current_directory(String8 path){
  ArenaTemp scratch = arena_get_scratch(0, 0);

  String16 path16 = str16_from_str8(scratch.arena, path);
  SetCurrentDirectoryW((WCHAR*)path16.str);

  arena_release_scratch(&scratch);
}


////////////////////////////////
// Win32 Implementation: Time

MR4TH_SYMBOL DateTime
os_now_universal_time(void){
  SYSTEMTIME system_time = {0};
  GetSystemTime(&system_time);
  DateTime result = w32_date_time_from_system_time(&system_time);
  return(result);
}

MR4TH_SYMBOL DateTime
os_local_time_from_universal(DateTime *univ_date_time){
  SYSTEMTIME univ_system_time = w32_system_time_from_date_time(univ_date_time);
  FILETIME univ_file_time = {0};
  SystemTimeToFileTime(&univ_system_time, &univ_file_time);
  FILETIME local_file_time = {0};
  FileTimeToLocalFileTime(&univ_file_time, &local_file_time);
  SYSTEMTIME local_system_time = {0};
  FileTimeToSystemTime(&local_file_time, &local_system_time);
  DateTime result = w32_date_time_from_system_time(&local_system_time);
  return(result);
}

MR4TH_SYMBOL DateTime
os_universal_time_from_local(DateTime *local_date_time){
  SYSTEMTIME local_system_time = w32_system_time_from_date_time(local_date_time);
  FILETIME local_file_time = {0};
  SystemTimeToFileTime(&local_system_time, &local_file_time);
  FILETIME univ_file_time = {0};
  LocalFileTimeToFileTime(&local_file_time, &univ_file_time);
  SYSTEMTIME univ_system_time = {0};
  FileTimeToSystemTime(&univ_file_time, &univ_system_time);
  DateTime result = w32_date_time_from_system_time(&univ_system_time);
  return(result);
}

MR4TH_SYMBOL U32
os_time_stamp_32_from_date_time(DateTime *date_time){
  SYSTEMTIME system_time = w32_system_time_from_date_time(date_time);
  FILETIME file_time = {0};
  SystemTimeToFileTime(&system_time, &file_time);
  U64 file_time64 = ( (U64) file_time.dwLowDateTime +
                     ((U64)(file_time.dwHighDateTime) << 32ll)
                     );
  U32 result = (U32) ((file_time64 - 116444736000000000ll)/10000000ll);
  return(result);
}


MR4TH_SYMBOL U64
os_now_ticks(void){
  U64 result = 0;
  LARGE_INTEGER perf_counter = {0};
  if (QueryPerformanceCounter(&perf_counter)){
    result = ((U64)perf_counter.HighPart << 32) | perf_counter.LowPart;
  }
  return(result);
}

MR4TH_SYMBOL void
os_microseconds_over_ticks(U64 *num_out, U64 *denom_out){
  *num_out = Million(1);
  *denom_out = w32_ticks_per_second;
}

MR4TH_SYMBOL void
os_sleep_milliseconds(U32 t){
  Sleep(t);
}

////////////////////////////////
// Win32 Implementation: Libraries

MR4TH_SYMBOL OS_Library*
os_lib_load(String8 path){
  ArenaTemp scratch = arena_get_scratch(0, 0);
  String16 path16 = str16_from_str8(scratch.arena, path);
  OS_Library *result = (OS_Library*)(LoadLibraryW((WCHAR*)path16.str));
  arena_release_scratch(&scratch);
  return(result);
}

MR4TH_SYMBOL VoidFunc*
os_lib_get_proc(OS_Library *lib, char *name){
  HMODULE module = (HMODULE)(lib);
  VoidFunc *result = (VoidFunc*)(GetProcAddress(module, name));
  return(result);
}

MR4TH_SYMBOL void
os_lib_release(OS_Library *lib){
  HMODULE module = (HMODULE)(lib);
  FreeLibrary(module);
}

MR4TH_SYMBOL OS_Library*
os_lib_from_addr(void *addr){
  HMODULE module = 0;
  GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                     (LPWSTR)addr, &module);
  return((OS_Library*)module);
}

MR4TH_SYMBOL RangeAddr
os_lib_image_range(OS_Library *lib){
  RangeAddr result = {0};
  HANDLE process = GetCurrentProcess();
  MODULEINFO module_info = {0};
  if (GetModuleInformation(process, (HMODULE)lib,
                           &module_info, sizeof(module_info))){
    result.first = (U8*)module_info.lpBaseOfDll;
    result.opl   = result.first + module_info.SizeOfImage;
  }
  return(result);
}

MR4TH_SYMBOL String8
os_this_image(void){
  String8 result = {0};
  OS_Library *this_lib = os_lib_from_addr(os_this_image);
  if (this_lib != 0){
    RangeAddr this_lib_range = os_lib_image_range(this_lib);
    result = str8_range(this_lib_range.first, this_lib_range.opl);
  }
  return(result);
}

////////////////////////////////
// Win32 Implementation: Entropy

MR4TH_SYMBOL void
os_get_entropy(void *data, U64 size){
  HCRYPTPROV prov = 0;
  CryptAcquireContextW(&prov, 0, 0, PROV_DSS, CRYPT_VERIFYCONTEXT);
  CryptGenRandom(prov, size, (BYTE*)data);
  CryptReleaseContext(prov, 0);
}

////////////////////////////////
// Win32 Implementation: Clipboard

MR4TH_SYMBOL void
os_clipboard_set_str8(String8 text){
  HWND wnd = NULL;
  if (OpenClipboard(wnd)){
    // TODO(allen): error strings here maybe
    if (EmptyClipboard()){
      HANDLE memhandle = GlobalAlloc(GMEM_MOVEABLE, text.size + 1);
      if (memhandle){
        void *dst = GlobalLock(memhandle);
        MemoryCopy(dst, text.str, text.size);
        ((U8*)dst)[text.size] = 0;
        GlobalUnlock(memhandle);
        SetClipboardData(CF_TEXT, memhandle);
      }
    }
    CloseClipboard();
  }
}

////////////////////////////////
// Win32 Functions: Specialized Init for WinMain

MR4TH_SYM_COMPTIME HINSTANCE w32_instance = 0;

MR4TH_SYMBOL void
w32_WinMain_init(HINSTANCE hInstance,
                 HINSTANCE hPrevInstance,
                 LPSTR     lpCmdLine,
                 int       nShowCmd){
  int argc = __argc;
  char **argv = __argv;
  w32_instance = hInstance;
  os_main_init(argc, argv);
}

MR4TH_SYMBOL HINSTANCE
w32_get_instance(void){
  if (w32_instance == 0){
    w32_instance = GetModuleHandle(0);
  }
  return(w32_instance);
}

////////////////////////////////
// Time Helpers

MR4TH_SYMBOL DateTime
w32_date_time_from_system_time(SYSTEMTIME *in){
  DateTime result = {0};
  result.year = in->wYear;
  result.mon  = (U8)in->wMonth;
  result.day  = in->wDay;
  result.hour = in->wHour;
  result.min  = in->wMinute;
  result.sec  = in->wSecond;
  result.msec = in->wMilliseconds;
  return(result);
}

MR4TH_SYMBOL SYSTEMTIME
w32_system_time_from_date_time(DateTime *in){
  SYSTEMTIME result = {0};
  result.wYear = in->year;
  result.wMonth = in->mon;
  result.wDay = in->day;
  result.wHour = in->hour;
  result.wMinute = in->min;
  result.wSecond = in->sec;
  result.wMilliseconds = in->msec;
  return(result);
}

MR4TH_SYMBOL DenseTime
w32_dense_time_from_file_time(FILETIME *file_time){
  SYSTEMTIME system_time = {0};
  FileTimeToSystemTime(file_time, &system_time);
  DateTime date_time = w32_date_time_from_system_time(&system_time);
  DenseTime result = dense_time_from_date_time(&date_time);
  return(result);
}

////////////////////////////////
// Win32 Functions: File Helpers

MR4TH_SYMBOL FilePropertyFlags
w32_prop_flags_from_attribs(DWORD attribs){
  FilePropertyFlags result = 0;
  if (attribs & FILE_ATTRIBUTE_DIRECTORY){
    result |= FilePropertyFlag_IsFolder;
  }
  return(result);
}

MR4TH_SYMBOL DataAccessFlags
w32_access_from_attributes(DWORD attribs){
  DataAccessFlags result = DataAccessFlag_Read|DataAccessFlag_Execute;
  if (!(attribs & FILE_ATTRIBUTE_READONLY)){
    result |= DataAccessFlag_Write;
  }
  return(result);
}

#endif /* OS_WINDOWS */

//////////////////////////////////////////////
//////////////////////////////////////////////
/////////   OS IMPLEMENTATION MAC   //////////
//////////////////////////////////////////////
//////////////////////////////////////////////

#if OS_MAC

////////////////////////////////
// Mac Implementation: Memory Functions

MR4TH_SYMBOL void*
os_memory_reserve(U64 size){
  void *result = 0;
  uint8_t *addr = 0;
  int file_descriptor = -1;
  int offset = 0;

  result = mmap(addr, size, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, file_descriptor, offset);
  // We, probably want to return 0 instead of MAP_FAILED, right?
  result = result == MAP_FAILED ? 0 : result;

  return result;
}

MR4TH_SYMBOL B32
os_memory_commit(void *ptr, U64 size){
  B32 result = (ptr != 0);
  // TODO: Should we call madvise here to let the OS know we are about to use the memory?
  return result;
}

MR4TH_SYMBOL void
os_memory_decommit(void *ptr, U64 size){
  madvise(ptr, size, MADV_DONTNEED);
}

MR4TH_SYMBOL void
os_memory_release(void *ptr, U64 size){
  munmap(ptr, size);
}


MR4TH_SYMBOL String8
os_file_read(Arena *arena, String8 file_name){
  ProfBeginFunc();

  String8 result = {0};
  S32 fd = open((char *)file_name.str, O_RDONLY);

  if (fd > -1) {
    B32 success = 1;
    struct stat file_stats;

    if (fstat(fd, &file_stats) == 0) {
      U64 total_size = file_stats.st_size;
      ArenaTemp restore_point = arena_begin_temp(arena);
      U8 *buffer = arena_push_no_zero(arena, total_size + 1);

      if (buffer) {
        if (!read(fd, buffer, file_stats.st_size)) {
          success = 0;
        }
      } else {
        success = 0;
      }

      if (success){
        buffer[total_size] = 0;
        result.str = buffer;
        result.size = total_size;
      }
      else{
        arena_end_temp(&restore_point);
      }
    }
  }

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL B32
os_file_write_list(String8 file_name, String8Node *first_node){
  ProfBeginFunc();
  B32 result = 0;

  // get handle
  U32 mode_if_creating_file = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;
  S32 fd = open((char *)file_name.str, O_WRONLY|O_CREAT, mode_if_creating_file);

  if (fd > -1) {
    result = 1;

    for (String8Node *node = first_node;
         node != 0;
         node = node->next){
      U8 *ptr = node->string.str;
      U8 *opl = ptr + node->string.size;
      size_t total_to_write = (size_t)(opl - ptr);
      ssize_t actual_write = write(fd, ptr, total_to_write);

      if (actual_write == -1) {
        result = 0;
        break;
      }
    }

    close(fd);
  }

  ProfEndFunc();
  return result;
}

MR4TH_SYMBOL B32
os_file_make_directory(String8 path){
  ProfBeginFunc();

  // make directory
  U32 mode = S_IRWXU | S_IRGRP|S_IXGRP | S_IROTH|S_IXOTH;
  S32 mkdir_result = mkdir((char *)path.str, mode);

  B32 success = (mkdir_result == 0) ? 1 : 0;

  ProfEndFunc();
  return(success);
}


MR4TH_SYMBOL String8
os_file_path(Arena *arena, OS_SystemPath path){
  ProfBeginFunc();

  String8 result = {0};

  switch (path){
    case OS_SystemPath_CurrentDirectory:
    {
      S32 cap = PATH_MAX;
      U8 *buffer = arena_push(arena, cap);
      char *path_pointer = getcwd((char *)buffer, cap);
      if (path_pointer) {
        result.str = (U8 *)path_pointer;
        // figure out the path size
        for (S32 i = 0; i < cap; ++i) {
          if (result.str[i] == 0) {
            break;
          }
          result.size += 1;
        }
      }
    }break;

    case OS_SystemPath_Binary:
    {
      Assert(!"TODO");
      /* result = str8_push_copy(arena, w32_binary_path); */
    }break;

    case OS_SystemPath_UserData:
    {
      Assert(!"TODO");
      /* result = str8_push_copy(arena, w32_user_path); */
    }break;

    case OS_SystemPath_TempData:
    {
      Assert(!"TODO");
      /* result = str8_push_copy(arena, w32_temp_path); */
    }break;
  }

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL String8
os_get_absolute_path_from_str8_list(Arena *arena, String8List rel_path_parts){
  String8 result = (String8){0};
  ArenaTemp scratch = arena_get_scratch(0, 0);

  String8Node null_node = (String8Node){0};
  str8_list_push_explicit(&rel_path_parts, str8_lit("\0"), &null_node);
  String8 rel_path = str8_join(scratch.arena, &rel_path_parts, 0);

  U8 *abs_path = arena_push(arena, PATH_MAX);
  char *resolved_path = realpath((char *)rel_path.str, (char *)abs_path);

  if (resolved_path) {
    result.str = (U8 *)resolved_path;
    // figure out the path size
    for (S32 i = 0; i < PATH_MAX; ++i) {
      if (result.str[i] == 0) {
        break;
      }
      result.size += 1;
    }
  }

  arena_release_scratch(&scratch);
  return result;
}

MR4TH_SYMBOL String8
os_get_absolute_path(Arena *arena, String8 rel_path){
  String8List list;
  String8Node node;
  node.next = 0;
  node.string = rel_path;
  list.first = &node;
  list.last = &node;
  list.node_count = 1;
  list.total_size = rel_path.size;

  return os_get_absolute_path_from_str8_list(arena, list);
}

MR4TH_SYMBOL void
os_set_current_directory(String8 path){
  chdir((char *)path.str);
}


////////////////////////////////
// Mac Implementation: Libraries

MR4TH_SYMBOL OS_Library*
os_lib_from_addr(void *addr){
  OS_Library *module = 0;
  Dl_info dl_info;

  if (dladdr(addr, &dl_info)) {
    module = dl_info.dli_fbase;
  }

  return((OS_Library*)module);
}

MR4TH_SYMBOL RangeAddr
os_lib_image_range(OS_Library *lib){
  // TODO: Right now os_lib_image_range only returns the range up to the end of the load-commands. I guess we need to parse into each command to get the size of the rest of the image? Is there a better way?
  RangeAddr result = {0};

  U64 command_offset = sizeof(struct mach_header_64);
  U64 total_size = 0;
  U8 *start_of_lib = (U8 *)lib;

  if (lib) {
    struct mach_header_64 *header = (struct mach_header_64 *)lib;
    result.first = start_of_lib;

#if 0
    // loop through load_commands to get sizes
    for (U32 count = 0; count < header->ncmds; ++count) {
      struct load_command *command = (struct load_command *)(start_of_lib + command_offset);

      total_size += command->cmdsize;
      command_offset += command->cmdsize;
    }
    result.opl = start_of_lib + total_size;
#else
    result.opl = start_of_lib + header->sizeofcmds;
#endif
  }

  return result;
}

MR4TH_SYMBOL String8
os_this_image(void){
  String8 result = {0};
#if 1
  OS_Library *this_lib = os_lib_from_addr(os_this_image);
  if (this_lib != 0){
    RangeAddr this_lib_range = os_lib_image_range(this_lib);
    result = str8_range(this_lib_range.first, this_lib_range.opl);
  }
#endif
  return(result);
}



////////////////////////////////
// Mac Implementation: File Handling
// TODO: File handling should also work for Linux.
MR4TH_SYMBOL OS_FileIter
os_file_iter_init(String8 path){
  ProfBeginFunc();

  // store into iter
  OS_FileIter result = {0};
  Mac_FileIter *mac_iter = (Mac_FileIter*)&result;
  mac_iter->base_path = path;
  mac_iter->handle = opendir((const char *)mac_iter->base_path.str);

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL B32
os_file_iter_next(Arena *arena, OS_FileIter *iter,
                  String8 *name, FileProperties *props){
  ProfBeginFunc();

  B32 result = 0;

  Mac_FileIter *mac_iter = (Mac_FileIter*)iter;
  if (mac_iter->handle != 0){
    for (;!mac_iter->done;){
      B32 emit = 1;
      struct dirent *dir = readdir(mac_iter->handle);

      if (dir) {
        // check for . and ..
        char *file_name = (char *)dir->d_name;
        B32 is_dot = (file_name[0] == '.' && file_name[1] == 0);
        B32 is_dotdot = (file_name[0] == '.' && file_name[1] == '.' &&
                         file_name[2] == 0);

        // setup to emit
        emit = (!is_dot && !is_dotdot);
      } else {
        // no more dirs, we are done
        mac_iter->done = 1;
        emit = 0;
      }

      if (emit){
        // join base-path and current dir
        ArenaTemp scratch = arena_get_scratch(0, 0);
        String8Node nodes[4];
        String8List list = {0};
        str8_list_push_explicit(&list, mac_iter->base_path, nodes + 0);
        *name = str8_cstring((U8 *)dir->d_name);
        // TODO: Check if base-path ends with path-separator, and if not, insert the path-separator.
        str8_list_push_explicit(&list, *name, nodes + 1);
        str8_list_push_explicit(&list, str8_lit("\0"), nodes + 2); // HACK: TODO: there's a better way to ensure null-termination, right?
        String8 full_path = str8_join(scratch.arena, &list, 0);
        // get the file stats in order to fill out the file-props struct.
        struct stat file_stats;
        if (lstat((char *)full_path.str, &file_stats) == 0) {
          props->size = file_stats.st_size;
          B32 is_directory = (file_stats.st_mode & S_IFMT) == S_IFDIR;
          props->flags = is_directory ? FilePropertyFlag_IsFolder : 0;
          props->create_time = file_stats.st_ctime; // NOTE: This is not exactly the creation time...
          props->modify_time = file_stats.st_mtime;
          props->access = file_stats.st_atime;
          result = 1;
        }
        break;
      }
    }
  }

  ProfEndFunc();
  return(result);
}

MR4TH_SYMBOL void
os_file_iter_end(OS_FileIter *iter){
  ProfBeginFunc();

  Mac_FileIter *mac_iter = (Mac_FileIter*)iter;
  if (mac_iter->handle != 0) {
    closedir(mac_iter->handle);
  }

  ProfEndFunc();
}


#endif /* OS_MAC */



////////////////////////////////////////////////
////////////////////////////////////////////////
//  stb_sprintf.h STB_SPRINTF_IMPLEMENTATION  //
////////////////////////////////////////////////
////////////////////////////////////////////////

/*
 **
** NOTE(allen): Modifications:
**  TODO(allen)
**
*/

#define stbsp__uint32 unsigned int
#define stbsp__int32 signed int

#ifdef _MSC_VER
#define stbsp__uint64 unsigned __int64
#define stbsp__int64 signed __int64
#else
#define stbsp__uint64 unsigned long long
#define stbsp__int64 signed long long
#endif
#define stbsp__uint16 unsigned short

#ifndef stbsp__uintptr
#if defined(__ppc64__) || defined(__powerpc64__) || defined(__aarch64__) || defined(_M_X64) || defined(__x86_64__) || defined(__x86_64) || defined(__s390x__)
#define stbsp__uintptr stbsp__uint64
#else
#define stbsp__uintptr stbsp__uint32
#endif
#endif

#ifndef STB_SPRINTF_MSVC_MODE // used for MSVC2013 and earlier (MSVC2015 matches GCC)
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define STB_SPRINTF_MSVC_MODE
#endif
#endif

#ifdef STB_SPRINTF_NOUNALIGNED // define this before inclusion to force stbsp_sprintf to always use aligned accesses
#define STBSP__UNALIGNED(code)
#else
#define STBSP__UNALIGNED(code) code
#endif

// internal float utility functions
static stbsp__int32 stbsp__real_to_str(char const **start, stbsp__uint32 *len, char *out, stbsp__int32 *decimal_pos, double value, stbsp__uint32 frac_digits);
static stbsp__int32 stbsp__real_to_parts(stbsp__int64 *bits, stbsp__int32 *expo, double value);
#define STBSP__SPECIAL 0x7000

static char stbsp__period = '.';
static char stbsp__comma = ',';
static struct
{
  short temp; // force next field to be 2-byte aligned
  char pair[201];
} stbsp__digitpair =
{
  0,
  "00010203040506070809101112131415161718192021222324"
    "25262728293031323334353637383940414243444546474849"
    "50515253545556575859606162636465666768697071727374"
    "75767778798081828384858687888990919293949596979899"
};

STBSP__PUBLICDEF void STB_SPRINTF_DECORATE(set_separators)(char pcomma, char pperiod)
{
  stbsp__period = pperiod;
  stbsp__comma = pcomma;
}

#define STBSP__LEFTJUST 1
#define STBSP__LEADINGPLUS 2
#define STBSP__LEADINGSPACE 4
#define STBSP__LEADING_0X 8
#define STBSP__LEADINGZERO 16
#define STBSP__INTMAX 32
#define STBSP__TRIPLET_COMMA 64
#define STBSP__NEGATIVE 128
#define STBSP__MEMORY_SIZES 256
#define STBSP__HALFWIDTH 512

static void stbsp__lead_sign(stbsp__uint32 fl, char *sign)
{
  sign[0] = 0;
  if (fl & STBSP__NEGATIVE) {
    sign[0] = 1;
    sign[1] = '-';
  } else if (fl & STBSP__LEADINGSPACE) {
    sign[0] = 1;
    sign[1] = ' ';
  } else if (fl & STBSP__LEADINGPLUS) {
    sign[0] = 1;
    sign[1] = '+';
  }
}

static STBSP__ASAN stbsp__uint32 stbsp__strlen_limited(char const *s, stbsp__uint32 limit)
{
  char const * sn = s;

  // get up to 4-byte alignment
  for (;;) {
    if (((stbsp__uintptr)sn & 3) == 0)
      break;

    if (!limit || *sn == 0)
      return (stbsp__uint32)(sn - s);

    ++sn;
    --limit;
  }

  // scan over 4 bytes at a time to find terminating 0
  // this will intentionally scan up to 3 bytes past the end of buffers,
  // but becase it works 4B aligned, it will never cross page boundaries
  // (hence the STBSP__ASAN markup; the over-read here is intentional
  // and harmless)
  while (limit >= 4) {
    stbsp__uint32 v = *(stbsp__uint32 *)sn;
    // bit hack to find if there's a 0 byte in there
    if ((v - 0x01010101) & (~v) & 0x80808080UL)
      break;

    sn += 4;
    limit -= 4;
  }

  // handle the last few characters to find actual size
  while (limit && *sn) {
    ++sn;
    --limit;
  }

  return (stbsp__uint32)(sn - s);
}

STBSP__PUBLICDEF int STB_SPRINTF_DECORATE(vsprintfcb)(STBSP_SPRINTFCB *callback, void *user, char *buf, char const *fmt, va_list va)
{
  static char hex[] = "0123456789abcdefxp";
  static char hexu[] = "0123456789ABCDEFXP";
  static char spaces[] =
    "                                "
    "                                "
    "                                "
    "                                ";
  char *bf;
  char const *f;
  int tlen = 0;

  bf = buf;
  f = fmt;
  for (;;) {
    stbsp__int32 fw, pr, tz;
    stbsp__uint32 fl;

    // macros for the callback buffer stuff
#define stbsp__chk_cb_bufL(bytes)                        \
{                                                     \
int len = (int)(bf - buf);                         \
if ((len + (bytes)) >= STB_SPRINTF_MIN) {          \
tlen += len;                                    \
if (0 == (bf = buf = callback(buf, user, len))) \
goto done;                                   \
}                                                  \
}
#define stbsp__chk_cb_buf(bytes)    \
{                                \
if (callback) {               \
stbsp__chk_cb_bufL(bytes); \
}                             \
}
#define stbsp__flush_cb()                      \
{                                           \
stbsp__chk_cb_bufL(STB_SPRINTF_MIN - 1); \
} // flush if there is even one byte in the buffer
#define stbsp__cb_buf_clamp(cl, v)                \
cl = v;                                        \
if (callback) {                                \
int lg = STB_SPRINTF_MIN - (int)(bf - buf); \
if (cl > lg)                                \
cl = lg;                                 \
}

    // fast copy everything up to the next % (or end of string)
    for (;;) {
      while (((stbsp__uintptr)f) & 3) {
        schk1:
        if (f[0] == '%')
          goto scandd;
        schk2:
        if (f[0] == 0)
          goto endfmt;
        stbsp__chk_cb_buf(1);
        *bf++ = f[0];
        ++f;
      }
      for (;;) {
        // Check if the next 4 bytes contain %(0x25) or end of string.
        // Using the 'hasless' trick:
        // https://graphics.stanford.edu/~seander/bithacks.html#HasLessInWord
        stbsp__uint32 v, c;
        v = *(stbsp__uint32 *)f;
        c = (~v) & 0x80808080;
        if (((v ^ 0x25252525) - 0x01010101) & c)
          goto schk1;
        if ((v - 0x01010101) & c)
          goto schk2;
        if (callback)
          if ((STB_SPRINTF_MIN - (int)(bf - buf)) < 4)
          goto schk1;
#ifdef STB_SPRINTF_NOUNALIGNED
        if(((stbsp__uintptr)bf) & 3) {
          bf[0] = f[0];
          bf[1] = f[1];
          bf[2] = f[2];
          bf[3] = f[3];
        } else
#endif
        {
          *(stbsp__uint32 *)bf = v;
        }
        bf += 4;
        f += 4;
      }
    }
    scandd:

    ++f;

    // ok, we have a percent, read the modifiers first
    fw = 0;
    pr = -1;
    fl = 0;
    tz = 0;

    // flags
    for (;;) {
      switch (f[0]) {
        // if we have left justify
        case '-':
        fl |= STBSP__LEFTJUST;
        ++f;
        continue;
        // if we have leading plus
        case '+':
        fl |= STBSP__LEADINGPLUS;
        ++f;
        continue;
        // if we have leading space
        case ' ':
        fl |= STBSP__LEADINGSPACE;
        ++f;
        continue;
        // if we have leading 0x
        case '#':
        fl |= STBSP__LEADING_0X;
        ++f;
        continue;
        // if we have thousand commas
        case '\'':
        fl |= STBSP__TRIPLET_COMMA;
        ++f;
        continue;
        // if we have memory sizes
        case '$':
        fl |= STBSP__MEMORY_SIZES;
        ++f;
        continue;
        // if we have leading zero
        case '0':
        fl |= STBSP__LEADINGZERO;
        ++f;
        goto flags_done;
        default: goto flags_done;
      }
    }
    flags_done:

    // get the field width
    if (f[0] == '*') {
      fw = va_arg(va, stbsp__uint32);
      ++f;
    } else {
      while ((f[0] >= '0') && (f[0] <= '9')) {
        fw = fw * 10 + f[0] - '0';
        f++;
      }
    }
    // get the precision
    if (f[0] == '.') {
      ++f;
      if (f[0] == '*') {
        pr = va_arg(va, stbsp__uint32);
        ++f;
      } else {
        pr = 0;
        while ((f[0] >= '0') && (f[0] <= '9')) {
          pr = pr * 10 + f[0] - '0';
          f++;
        }
      }
    }

    // handle integer size overrides
    switch (f[0]) {
      // are we halfwidth?
      case 'h':
      fl |= STBSP__HALFWIDTH;
      ++f;
      if (f[0] == 'h')
        ++f;  // QUARTERWIDTH
      break;
      // are we 64-bit (unix style)
      case 'l':
      fl |= ((sizeof(long) == 8) ? STBSP__INTMAX : 0);
      ++f;
      if (f[0] == 'l') {
        fl |= STBSP__INTMAX;
        ++f;
      }
      break;
      // are we 64-bit on intmax? (c99)
      case 'j':
      fl |= (sizeof(size_t) == 8) ? STBSP__INTMAX : 0;
      ++f;
      break;
      // are we 64-bit on size_t or ptrdiff_t? (c99)
      case 'z':
      fl |= (sizeof(ptrdiff_t) == 8) ? STBSP__INTMAX : 0;
      ++f;
      break;
      case 't':
      fl |= (sizeof(ptrdiff_t) == 8) ? STBSP__INTMAX : 0;
      ++f;
      break;
      // are we 64-bit (msft style)
      case 'I':
      if ((f[1] == '6') && (f[2] == '4')) {
        fl |= STBSP__INTMAX;
        f += 3;
      } else if ((f[1] == '3') && (f[2] == '2')) {
        f += 3;
      } else {
        fl |= ((sizeof(void *) == 8) ? STBSP__INTMAX : 0);
        ++f;
      }
      break;
      default: break;
    }

    // handle each replacement
    switch (f[0]) {
#define STBSP__NUMSZ 512 // big enough for e308 (with commas) or e-307
      char num[STBSP__NUMSZ];
      char lead[8];
      char tail[8];
      char *s;
      char const *h;
      stbsp__uint32 l, n, cs;
      stbsp__uint64 n64;
      double fv;
      stbsp__int32 dp;
      char const *sn;

      // NOTE(allen): MODIFICATION
      // %S prints str (parameter: String8 str)
      case 'S':
      {
        String8 str = va_arg(va, String8);
        s = (char*)str.str;
        l = (pr >= 0 && pr < str.size) ? pr : (U32)str.size;
        lead[0] = 0;
        tail[0] = 0;
        pr = 0;
        dp = 0;
        cs = 0;
        goto scopy;
      }

      // NOTE(allen): MODIFICATION
      // %N prints n spaces (parameter: int n)
      case 'N':
      {
        // TODO(allen): there's probably a better way to set this
        // up so that it doesn't rely on the buffer of spaces -
        // perhaps a way to apply the space filling in the left justify path?
        l = va_arg(va, int);
        l = (sizeof(spaces) - 1 < l) ? (sizeof(spaces) - 1) : l;
        l = (pr >= 0 && pr < l) ? pr : l;
        s = spaces;
        lead[0] = 0;
        tail[0] = 0;
        pr = 0;
        dp = 0;
        cs = 0;
        goto scopy;
      }

      case 's':
      {
        // get the string
        s = va_arg(va, char *);
        if (s == 0)
          s = (char *)"null";
        // get the length, limited to desired precision
        // always limit to ~0u chars since our counts are 32b
        l = stbsp__strlen_limited(s, (pr >= 0) ? pr : ~0u);
        lead[0] = 0;
        tail[0] = 0;
        pr = 0;
        dp = 0;
        cs = 0;
        // copy the string in
        goto scopy;
      }

      case 'c': // char
      {
        // get the character
        s = num + STBSP__NUMSZ - 1;
        *s = (char)va_arg(va, int);
        l = 1;
        lead[0] = 0;
        tail[0] = 0;
        pr = 0;
        dp = 0;
        cs = 0;
        goto scopy;
      }

      case 'n': // weird write-bytes specifier
      {
        int *d = va_arg(va, int *);
        *d = tlen + (int)(bf - buf);
      } break;

      case 'A': // hex float
      case 'a': // hex float
      {
        h = (f[0] == 'A') ? hexu : hex;
        fv = va_arg(va, double);
        if (pr == -1)
          pr = 6; // default is 6
        // read the double into a string
        if (stbsp__real_to_parts((stbsp__int64 *)&n64, &dp, fv))
          fl |= STBSP__NEGATIVE;

        s = num + 64;

        stbsp__lead_sign(fl, lead);

        if (dp == -1023)
          dp = (n64) ? -1022 : 0;
        else
          n64 |= (((stbsp__uint64)1) << 52);
        n64 <<= (64 - 56);
        if (pr < 15)
          n64 += ((((stbsp__uint64)8) << 56) >> (pr * 4));
        // add leading chars

#ifdef STB_SPRINTF_MSVC_MODE
        *s++ = '0';
        *s++ = 'x';
#else
        lead[1 + lead[0]] = '0';
        lead[2 + lead[0]] = 'x';
        lead[0] += 2;
#endif
        *s++ = h[(n64 >> 60) & 15];
        n64 <<= 4;
        if (pr)
          *s++ = stbsp__period;
        sn = s;

        // print the bits
        n = pr;
        if (n > 13)
          n = 13;
        if (pr > (stbsp__int32)n)
          tz = pr - n;
        pr = 0;
        while (n--) {
          *s++ = h[(n64 >> 60) & 15];
          n64 <<= 4;
        }

        // print the expo
        tail[1] = h[17];
        if (dp < 0) {
          tail[2] = '-';
          dp = -dp;
        } else
          tail[2] = '+';
        n = (dp >= 1000) ? 6 : ((dp >= 100) ? 5 : ((dp >= 10) ? 4 : 3));
        tail[0] = (char)n;
        for (;;) {
          tail[n] = '0' + dp % 10;
          if (n <= 3)
            break;
          --n;
          dp /= 10;
        }

        dp = (int)(s - sn);
        l = (int)(s - (num + 64));
        s = num + 64;
        cs = 1 + (3 << 24);
        goto scopy;
      }

      case 'G': // float
      case 'g': // float
      {
        h = (f[0] == 'G') ? hexu : hex;
        fv = va_arg(va, double);
        if (pr == -1)
          pr = 6;
        else if (pr == 0)
          pr = 1; // default is 6
        // read the double into a string
        if (stbsp__real_to_str(&sn, &l, num, &dp, fv, (pr - 1) | 0x80000000))
          fl |= STBSP__NEGATIVE;

        // clamp the precision and delete extra zeros after clamp
        n = pr;
        if (l > (stbsp__uint32)pr)
          l = pr;
        while ((l > 1) && (pr) && (sn[l - 1] == '0')) {
          --pr;
          --l;
        }

        // should we use %e
        if ((dp <= -4) || (dp > (stbsp__int32)n)) {
          if (pr > (stbsp__int32)l)
            pr = l - 1;
          else if (pr)
            --pr; // when using %e, there is one digit before the decimal
          goto doexpfromg;
        }
        // this is the insane action to get the pr to match %g semantics for %f
        if (dp > 0) {
          pr = (dp < (stbsp__int32)l) ? l - dp : 0;
        } else {
          pr = -dp + ((pr > (stbsp__int32)l) ? (stbsp__int32) l : pr);
        }
        goto dofloatfromg;
      }

      case 'E': // float
      case 'e': // float
      {
        h = (f[0] == 'E') ? hexu : hex;
        fv = va_arg(va, double);
        if (pr == -1)
          pr = 6; // default is 6
        // read the double into a string
        if (stbsp__real_to_str(&sn, &l, num, &dp, fv, pr | 0x80000000))
          fl |= STBSP__NEGATIVE;
        goto doexpfromg;
      }

      doexpfromg:
      {
        tail[0] = 0;
        stbsp__lead_sign(fl, lead);
        if (dp == STBSP__SPECIAL) {
          s = (char *)sn;
          cs = 0;
          pr = 0;
          goto scopy;
        }
        s = num + 64;
        // handle leading chars
        *s++ = sn[0];

        if (pr)
          *s++ = stbsp__period;

        // handle after decimal
        if ((l - 1) > (stbsp__uint32)pr)
          l = pr + 1;
        for (n = 1; n < l; n++)
          *s++ = sn[n];
        // trailing zeros
        tz = pr - (l - 1);
        pr = 0;
        // dump expo
        tail[1] = h[0xe];
        dp -= 1;
        if (dp < 0) {
          tail[2] = '-';
          dp = -dp;
        } else
          tail[2] = '+';
#ifdef STB_SPRINTF_MSVC_MODE
        n = 5;
#else
        n = (dp >= 100) ? 5 : 4;
#endif
        tail[0] = (char)n;
        for (;;) {
          tail[n] = '0' + dp % 10;
          if (n <= 3)
            break;
          --n;
          dp /= 10;
        }
        cs = 1 + (3 << 24); // how many tens
        goto flt_lead;
      }

      case 'f': // float
      {
        fv = va_arg(va, double);
        goto doafloat;
      }

      doafloat:
      {
        // do kilos
        if (fl & STBSP__MEMORY_SIZES) {
          double divisor = 1024.f;
          while (fl < 0x4000000) {
            if ((fv < divisor) && (fv > -divisor))
              break;
            fv /= divisor;
            fl += 0x1000000;
          }
        }
        if (pr == -1)
          pr = 6; // default is 6
        // read the double into a string
        if (stbsp__real_to_str(&sn, &l, num, &dp, fv, pr))
          fl |= STBSP__NEGATIVE;
      }

      dofloatfromg:
      {
        tail[0] = 0;
        stbsp__lead_sign(fl, lead);
        if (dp == STBSP__SPECIAL) {
          s = (char *)sn;
          cs = 0;
          pr = 0;
          goto scopy;
        }
        s = num + 64;

        // handle the three decimal varieties
        if (dp <= 0) {
          stbsp__int32 i;
          // handle 0.000*000xxxx
          *s++ = '0';
          if (pr)
            *s++ = stbsp__period;
          n = -dp;
          if ((stbsp__int32)n > pr)
            n = pr;
          i = n;
          while (i) {
            if ((((stbsp__uintptr)s) & 3) == 0)
              break;
            *s++ = '0';
            --i;
          }
          while (i >= 4) {
            *(stbsp__uint32 *)s = 0x30303030;
            s += 4;
            i -= 4;
          }
          while (i) {
            *s++ = '0';
            --i;
          }
          if ((stbsp__int32)(l + n) > pr)
            l = pr - n;
          i = l;
          while (i) {
            *s++ = *sn++;
            --i;
          }
          tz = pr - (n + l);
          cs = 1 + (3 << 24); // how many tens did we write (for commas below)
        } else {
          cs = (fl & STBSP__TRIPLET_COMMA) ? ((600 - (stbsp__uint32)dp) % 3) : 0;
          if ((stbsp__uint32)dp >= l) {
            // handle xxxx000*000.0
            n = 0;
            for (;;) {
              if ((fl & STBSP__TRIPLET_COMMA) && (++cs == 4)) {
                cs = 0;
                *s++ = stbsp__comma;
              } else {
                *s++ = sn[n];
                ++n;
                if (n >= l)
                  break;
              }
            }
            if (n < (stbsp__uint32)dp) {
              n = dp - n;
              if ((fl & STBSP__TRIPLET_COMMA) == 0) {
                while (n) {
                  if ((((stbsp__uintptr)s) & 3) == 0)
                    break;
                  *s++ = '0';
                  --n;
                }
                while (n >= 4) {
                  *(stbsp__uint32 *)s = 0x30303030;
                  s += 4;
                  n -= 4;
                }
              }
              while (n) {
                if ((fl & STBSP__TRIPLET_COMMA) && (++cs == 4)) {
                  cs = 0;
                  *s++ = stbsp__comma;
                } else {
                  *s++ = '0';
                  --n;
                }
              }
            }
            cs = (int)(s - (num + 64)) + (3 << 24); // cs is how many tens
            if (pr) {
              *s++ = stbsp__period;
              tz = pr;
            }
          } else {
            // handle xxxxx.xxxx000*000
            n = 0;
            for (;;) {
              if ((fl & STBSP__TRIPLET_COMMA) && (++cs == 4)) {
                cs = 0;
                *s++ = stbsp__comma;
              } else {
                *s++ = sn[n];
                ++n;
                if (n >= (stbsp__uint32)dp)
                  break;
              }
            }
            cs = (int)(s - (num + 64)) + (3 << 24); // cs is how many tens
            if (pr)
              *s++ = stbsp__period;
            if ((l - dp) > (stbsp__uint32)pr)
              l = pr + dp;
            while (n < l) {
              *s++ = sn[n];
              ++n;
            }
            tz = pr - (l - dp);
          }
        }
        pr = 0;

        // handle k,m,g,t
        if (fl & STBSP__MEMORY_SIZES) {
          tail[0] = 0;
          {
            char idx = 1;
            tail[idx] = ' ';
            idx++;
            tail[idx] = " KMGT"[fl >> 24];
            idx++;
            tail[idx] = 'b';
            tail[0] = idx;
          }
        }
        goto flt_lead;
      }

      flt_lead:
      {
        // get the length that we copied
        l = (stbsp__uint32)(s - (num + 64));
        s = num + 64;
        goto scopy;
      }

      case 'B': // upper binary
      case 'b': // lower binary
      {
        h = (f[0] == 'B') ? hexu : hex;
        lead[0] = 0;
        if (fl & STBSP__LEADING_0X) {
          lead[0] = 2;
          lead[1] = '0';
          lead[2] = h[0xb];
        }
        l = (8 << 4) | (1 << 8);
        goto radixnum;
      }

      case 'o': // octal
      {
        h = hexu;
        lead[0] = 0;
        if (fl & STBSP__LEADING_0X) {
          lead[0] = 1;
          lead[1] = '0';
        }
        l = (3 << 4) | (3 << 8);
        goto radixnum;
      }

      case 'p': // pointer
      {
        fl |= (sizeof(void *) == 8) ? STBSP__INTMAX : 0;
        pr = sizeof(void *) * 2;
        fl &= ~STBSP__LEADINGZERO; // 'p' only prints the pointer with zeros
        // fall through - to X
      }

      case 'X': // upper hex
      case 'x': // lower hex
      {
        h = (f[0] == 'X') ? hexu : hex;
        l = (4 << 4) | (4 << 8);
        lead[0] = 0;
        if (fl & STBSP__LEADING_0X) {
          lead[0] = 2;
          lead[1] = '0';
          lead[2] = h[16];
        }
        goto radixnum;
      }

      radixnum:
      {
        // get the number
        if (fl & STBSP__INTMAX)
          n64 = va_arg(va, stbsp__uint64);
        else
          n64 = va_arg(va, stbsp__uint32);

        s = num + STBSP__NUMSZ;
        dp = 0;
        // clear tail, and clear leading if value is zero
        tail[0] = 0;
        if (n64 == 0) {
          lead[0] = 0;
          if (pr == 0) {
            l = 0;
            cs = 0;
            goto scopy;
          }
        }
        // convert to string
        for (;;) {
          *--s = h[n64 & ((1 << (l >> 8)) - 1)];
          n64 >>= (l >> 8);
          if (!((n64) || ((stbsp__int32)((num + STBSP__NUMSZ) - s) < pr)))
            break;
          if (fl & STBSP__TRIPLET_COMMA) {
            ++l;
            if ((l & 15) == ((l >> 4) & 15)) {
              l &= ~15;
              *--s = stbsp__comma;
            }
          }
        };
        // get the tens and the comma pos
        cs = (stbsp__uint32)((num + STBSP__NUMSZ) - s) + ((((l >> 4) & 15)) << 24);
        // get the length that we copied
        l = (stbsp__uint32)((num + STBSP__NUMSZ) - s);
        // copy it
        goto scopy;
      }

      case 'u': // unsigned
      case 'i':
      case 'd': // integer
      {
        // get the integer and abs it
        if (fl & STBSP__INTMAX) {
          stbsp__int64 i64 = va_arg(va, stbsp__int64);
          n64 = (stbsp__uint64)i64;
          if ((f[0] != 'u') && (i64 < 0)) {
            n64 = (stbsp__uint64)-i64;
            fl |= STBSP__NEGATIVE;
          }
        } else {
          stbsp__int32 i = va_arg(va, stbsp__int32);
          n64 = (stbsp__uint32)i;
          if ((f[0] != 'u') && (i < 0)) {
            n64 = (stbsp__uint32)-i;
            fl |= STBSP__NEGATIVE;
          }
        }

        if (fl & STBSP__MEMORY_SIZES) {
          if (n64 < 1024)
            pr = 0;
          else if (pr == -1)
            pr = 1;
          fv = (double)(stbsp__int64)n64;
          goto doafloat;
        }

        // convert to string
        s = num + STBSP__NUMSZ;
        l = 0;

        for (;;) {
          // do in 32-bit chunks (avoid lots of 64-bit divides even with constant denominators)
          char *o = s - 8;
          if (n64 >= 100000000) {
            n = (stbsp__uint32)(n64 % 100000000);
            n64 /= 100000000;
          } else {
            n = (stbsp__uint32)n64;
            n64 = 0;
          }
          if ((fl & STBSP__TRIPLET_COMMA) == 0) {
            do {
              s -= 2;
              *(stbsp__uint16 *)s = *(stbsp__uint16 *)&stbsp__digitpair.pair[(n % 100) * 2];
              n /= 100;
            } while (n);
          }
          while (n) {
            if ((fl & STBSP__TRIPLET_COMMA) && (l++ == 3)) {
              l = 0;
              *--s = stbsp__comma;
              --o;
            } else {
              *--s = (char)(n % 10) + '0';
              n /= 10;
            }
          }
          if (n64 == 0) {
            if ((s[0] == '0') && (s != (num + STBSP__NUMSZ)))
              ++s;
            break;
          }
          while (s != o)
            if ((fl & STBSP__TRIPLET_COMMA) && (l++ == 3)) {
            l = 0;
            *--s = stbsp__comma;
            --o;
          } else {
            *--s = '0';
          }
        }

        tail[0] = 0;
        stbsp__lead_sign(fl, lead);

        // get the length that we copied
        l = (stbsp__uint32)((num + STBSP__NUMSZ) - s);
        if (l == 0) {
          *--s = '0';
          l = 1;
        }
        cs = l + (3 << 24);
        if (pr < 0)
          pr = 0;
        goto scopy;
      }

      default: // unknown, just copy code
      {
        s = num + STBSP__NUMSZ - 1;
        *s = f[0];
        l = 1;
        fw = fl = 0;
        lead[0] = 0;
        tail[0] = 0;
        pr = 0;
        dp = 0;
        cs = 0;
        goto scopy;
      }

      scopy:
      // get fw=leading/trailing space, pr=leading zeros
      if (pr < (stbsp__int32)l)
        pr = l;
      n = pr + lead[0] + tail[0] + tz;
      if (fw < (stbsp__int32)n)
        fw = n;
      fw -= n;
      pr -= l;

      // handle right justify and leading zeros
      if ((fl & STBSP__LEFTJUST) == 0) {
        if (fl & STBSP__LEADINGZERO) // if leading zeros, everything is in pr
        {
          pr = (fw > pr) ? fw : pr;
          fw = 0;
        } else {
          fl &= ~STBSP__TRIPLET_COMMA; // if no leading zeros, then no commas
        }
      }

      // copy the spaces and/or zeros
      if (fw + pr) {
        stbsp__int32 i;
        stbsp__uint32 c;

        // copy leading spaces (or when doing %8.4d stuff)
        if ((fl & STBSP__LEFTJUST) == 0)
          while (fw > 0) {
          stbsp__cb_buf_clamp(i, fw);
          fw -= i;
          while (i) {
            if ((((stbsp__uintptr)bf) & 3) == 0)
              break;
            *bf++ = ' ';
            --i;
          }
          while (i >= 4) {
            *(stbsp__uint32 *)bf = 0x20202020;
            bf += 4;
            i -= 4;
          }
          while (i) {
            *bf++ = ' ';
            --i;
          }
          stbsp__chk_cb_buf(1);
        }

        // copy leader
        sn = lead + 1;
        while (lead[0]) {
          stbsp__cb_buf_clamp(i, lead[0]);
          lead[0] -= (char)i;
          while (i) {
            *bf++ = *sn++;
            --i;
          }
          stbsp__chk_cb_buf(1);
        }

        // copy leading zeros
        c = cs >> 24;
        cs &= 0xffffff;
        cs = (fl & STBSP__TRIPLET_COMMA) ? ((stbsp__uint32)(c - ((pr + cs) % (c + 1)))) : 0;
        while (pr > 0) {
          stbsp__cb_buf_clamp(i, pr);
          pr -= i;
          if ((fl & STBSP__TRIPLET_COMMA) == 0) {
            while (i) {
              if ((((stbsp__uintptr)bf) & 3) == 0)
                break;
              *bf++ = '0';
              --i;
            }
            while (i >= 4) {
              *(stbsp__uint32 *)bf = 0x30303030;
              bf += 4;
              i -= 4;
            }
          }
          while (i) {
            if ((fl & STBSP__TRIPLET_COMMA) && (cs++ == c)) {
              cs = 0;
              *bf++ = stbsp__comma;
            } else
              *bf++ = '0';
            --i;
          }
          stbsp__chk_cb_buf(1);
        }
      }

      // copy leader if there is still one
      sn = lead + 1;
      while (lead[0]) {
        stbsp__int32 i;
        stbsp__cb_buf_clamp(i, lead[0]);
        lead[0] -= (char)i;
        while (i) {
          *bf++ = *sn++;
          --i;
        }
        stbsp__chk_cb_buf(1);
      }

      // copy the string
      n = l;
      while (n) {
        stbsp__int32 i;
        stbsp__cb_buf_clamp(i, n);
        n -= i;
        STBSP__UNALIGNED(while (i >= 4) {
                           *(stbsp__uint32 volatile *)bf = *(stbsp__uint32 volatile *)s;
                           bf += 4;
                           s += 4;
                           i -= 4;
                         })
          while (i) {
          *bf++ = *s++;
          --i;
        }
        stbsp__chk_cb_buf(1);
      }

      // copy trailing zeros
      while (tz) {
        stbsp__int32 i;
        stbsp__cb_buf_clamp(i, tz);
        tz -= i;
        while (i) {
          if ((((stbsp__uintptr)bf) & 3) == 0)
            break;
          *bf++ = '0';
          --i;
        }
        while (i >= 4) {
          *(stbsp__uint32 *)bf = 0x30303030;
          bf += 4;
          i -= 4;
        }
        while (i) {
          *bf++ = '0';
          --i;
        }
        stbsp__chk_cb_buf(1);
      }

      // copy tail if there is one
      sn = tail + 1;
      while (tail[0]) {
        stbsp__int32 i;
        stbsp__cb_buf_clamp(i, tail[0]);
        tail[0] -= (char)i;
        while (i) {
          *bf++ = *sn++;
          --i;
        }
        stbsp__chk_cb_buf(1);
      }

      // handle the left justify
      if (fl & STBSP__LEFTJUST)
        if (fw > 0) {
        while (fw) {
          stbsp__int32 i;
          stbsp__cb_buf_clamp(i, fw);
          fw -= i;
          while (i) {
            if ((((stbsp__uintptr)bf) & 3) == 0)
              break;
            *bf++ = ' ';
            --i;
          }
          while (i >= 4) {
            *(stbsp__uint32 *)bf = 0x20202020;
            bf += 4;
            i -= 4;
          }
          while (i--)
            *bf++ = ' ';
          stbsp__chk_cb_buf(1);
        }
      }
      break;
    }
    ++f;
  }
  endfmt:

  if (!callback)
    *bf = 0;
  else
    stbsp__flush_cb();

  done:
  return tlen + (int)(bf - buf);
}

// cleanup
#undef STBSP__LEFTJUST
#undef STBSP__LEADINGPLUS
#undef STBSP__LEADINGSPACE
#undef STBSP__LEADING_0X
#undef STBSP__LEADINGZERO
#undef STBSP__INTMAX
#undef STBSP__TRIPLET_COMMA
#undef STBSP__NEGATIVE
#undef STBSP__METRIC_SUFFIX
#undef STBSP__NUMSZ
#undef stbsp__chk_cb_bufL
#undef stbsp__chk_cb_buf
#undef stbsp__flush_cb
#undef stbsp__cb_buf_clamp

// ============================================================================
//   wrapper functions

STBSP__PUBLICDEF int STB_SPRINTF_DECORATE(sprintf)(char *buf, char const *fmt, ...)
{
  int result;
  va_list va;
  va_start(va, fmt);
  result = STB_SPRINTF_DECORATE(vsprintfcb)(0, 0, buf, fmt, va);
  va_end(va);
  return result;
}

typedef struct stbsp__context {
  char *buf;
  int count;
  int length;
  char tmp[STB_SPRINTF_MIN];
} stbsp__context;

static char *stbsp__clamp_callback(const char *buf, void *user, int len)
{
  stbsp__context *c = (stbsp__context *)user;
  c->length += len;

  if (len > c->count)
    len = c->count;

  if (len) {
    if (buf != c->buf) {
      const char *s, *se;
      char *d;
      d = c->buf;
      s = buf;
      se = buf + len;
      do {
        *d++ = *s++;
      } while (s < se);
    }
    c->buf += len;
    c->count -= len;
  }

  if (c->count <= 0)
    return c->tmp;
  return (c->count >= STB_SPRINTF_MIN) ? c->buf : c->tmp; // go direct into buffer if you can
}

static char * stbsp__count_clamp_callback( const char * buf, void * user, int len )
{
  stbsp__context * c = (stbsp__context*)user;
  (void) sizeof(buf);

  c->length += len;
  return c->tmp; // go direct into buffer if you can
}

STBSP__PUBLICDEF int STB_SPRINTF_DECORATE( vsnprintf )( char * buf, int count, char const * fmt, va_list va )
{
  stbsp__context c;

  if ( (count == 0) && !buf )
  {
    c.length = 0;

    STB_SPRINTF_DECORATE( vsprintfcb )( stbsp__count_clamp_callback, &c, c.tmp, fmt, va );
  }
  else
  {
    int l;

    c.buf = buf;
    c.count = count;
    c.length = 0;

    STB_SPRINTF_DECORATE( vsprintfcb )( stbsp__clamp_callback, &c, stbsp__clamp_callback(0,&c,0), fmt, va );

    // zero-terminate
    l = (int)( c.buf - buf );
    if ( l >= count ) // should never be greater, only equal (or less) than count
      l = count - 1;
    buf[l] = 0;
  }

  return c.length;
}

STBSP__PUBLICDEF int STB_SPRINTF_DECORATE(snprintf)(char *buf, int count, char const *fmt, ...)
{
  int result;
  va_list va;
  va_start(va, fmt);

  result = STB_SPRINTF_DECORATE(vsnprintf)(buf, count, fmt, va);
  va_end(va);

  return result;
}

STBSP__PUBLICDEF int STB_SPRINTF_DECORATE(vsprintf)(char *buf, char const *fmt, va_list va)
{
  return STB_SPRINTF_DECORATE(vsprintfcb)(0, 0, buf, fmt, va);
}

// =======================================================================
//   low level float utility functions

// copies d to bits w/ strict aliasing (this compiles to nothing on /Ox)
#define STBSP__COPYFP(dest, src)                   \
{                                               \
int cn;                                      \
for (cn = 0; cn < 8; cn++)                   \
((char *)&dest)[cn] = ((char *)&src)[cn]; \
}

// get float info
static stbsp__int32 stbsp__real_to_parts(stbsp__int64 *bits, stbsp__int32 *expo, double value)
{
  double d;
  stbsp__int64 b = 0;

  // load value and round at the frac_digits
  d = value;

  STBSP__COPYFP(b, d);

  *bits = b & ((((stbsp__uint64)1) << 52) - 1);
  *expo = (stbsp__int32)(((b >> 52) & 2047) - 1023);

  return (stbsp__int32)((stbsp__uint64) b >> 63);
}

static double const stbsp__bot[23] = {
  1e+000, 1e+001, 1e+002, 1e+003, 1e+004, 1e+005, 1e+006, 1e+007, 1e+008, 1e+009, 1e+010, 1e+011,
  1e+012, 1e+013, 1e+014, 1e+015, 1e+016, 1e+017, 1e+018, 1e+019, 1e+020, 1e+021, 1e+022
};
static double const stbsp__negbot[22] = {
  1e-001, 1e-002, 1e-003, 1e-004, 1e-005, 1e-006, 1e-007, 1e-008, 1e-009, 1e-010, 1e-011,
  1e-012, 1e-013, 1e-014, 1e-015, 1e-016, 1e-017, 1e-018, 1e-019, 1e-020, 1e-021, 1e-022
};
static double const stbsp__negboterr[22] = {
  -5.551115123125783e-018,  -2.0816681711721684e-019, -2.0816681711721686e-020, -4.7921736023859299e-021, -8.1803053914031305e-022, 4.5251888174113741e-023,
  4.5251888174113739e-024,  -2.0922560830128471e-025, -6.2281591457779853e-026, -3.6432197315497743e-027, 6.0503030718060191e-028,  2.0113352370744385e-029,
  -3.0373745563400371e-030, 1.1806906454401013e-032,  -7.7705399876661076e-032, 2.0902213275965398e-033,  -7.1542424054621921e-034, -7.1542424054621926e-035,
  2.4754073164739869e-036,  5.4846728545790429e-037,  9.2462547772103625e-038,  -4.8596774326570872e-039
};
static double const stbsp__top[13] = {
  1e+023, 1e+046, 1e+069, 1e+092, 1e+115, 1e+138, 1e+161, 1e+184, 1e+207, 1e+230, 1e+253, 1e+276, 1e+299
};
static double const stbsp__negtop[13] = {
  1e-023, 1e-046, 1e-069, 1e-092, 1e-115, 1e-138, 1e-161, 1e-184, 1e-207, 1e-230, 1e-253, 1e-276, 1e-299
};
static double const stbsp__toperr[13] = {
  8388608,
  6.8601809640529717e+028,
  -7.253143638152921e+052,
  -4.3377296974619174e+075,
  -1.5559416129466825e+098,
  -3.2841562489204913e+121,
  -3.7745893248228135e+144,
  -1.7356668416969134e+167,
  -3.8893577551088374e+190,
  -9.9566444326005119e+213,
  6.3641293062232429e+236,
  -5.2069140800249813e+259,
  -5.2504760255204387e+282
};
static double const stbsp__negtoperr[13] = {
  3.9565301985100693e-040,  -2.299904345391321e-063,  3.6506201437945798e-086,  1.1875228833981544e-109,
  -5.0644902316928607e-132, -6.7156837247865426e-155, -2.812077463003139e-178,  -5.7778912386589953e-201,
  7.4997100559334532e-224,  -4.6439668915134491e-247, -6.3691100762962136e-270, -9.436808465446358e-293,
  8.0970921678014997e-317
};

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
static stbsp__uint64 const stbsp__powten[20] = {
  1,
  10,
  100,
  1000,
  10000,
  100000,
  1000000,
  10000000,
  100000000,
  1000000000,
  10000000000,
  100000000000,
  1000000000000,
  10000000000000,
  100000000000000,
  1000000000000000,
  10000000000000000,
  100000000000000000,
  1000000000000000000,
  10000000000000000000U
};
#define stbsp__tento19th ((stbsp__uint64)1000000000000000000)
#else
static stbsp__uint64 const stbsp__powten[20] = {
  1,
  10,
  100,
  1000,
  10000,
  100000,
  1000000,
  10000000,
  100000000,
  1000000000,
  10000000000ULL,
  100000000000ULL,
  1000000000000ULL,
  10000000000000ULL,
  100000000000000ULL,
  1000000000000000ULL,
  10000000000000000ULL,
  100000000000000000ULL,
  1000000000000000000ULL,
  10000000000000000000ULL
};
#define stbsp__tento19th (1000000000000000000ULL)
#endif

#define stbsp__ddmulthi(oh, ol, xh, yh)                            \
{                                                               \
double ahi = 0, alo, bhi = 0, blo;                           \
stbsp__int64 bt;                                             \
oh = xh * yh;                                                \
STBSP__COPYFP(bt, xh);                                       \
bt &= ((~(stbsp__uint64)0) << 27);                           \
STBSP__COPYFP(ahi, bt);                                      \
alo = xh - ahi;                                              \
STBSP__COPYFP(bt, yh);                                       \
bt &= ((~(stbsp__uint64)0) << 27);                           \
STBSP__COPYFP(bhi, bt);                                      \
blo = yh - bhi;                                              \
ol = ((ahi * bhi - oh) + ahi * blo + alo * bhi) + alo * blo; \
}

#define stbsp__ddtoS64(ob, xh, xl)          \
{                                        \
double ahi = 0, alo, vh, t;           \
ob = (stbsp__int64)xh;                \
vh = (double)ob;                      \
ahi = (xh - vh);                      \
t = (ahi - xh);                       \
alo = (xh - (ahi - t)) - (vh + t);    \
ob += (stbsp__int64)(ahi + alo + xl); \
}

#define stbsp__ddrenorm(oh, ol) \
{                            \
double s;                 \
s = oh + ol;              \
ol = ol - (s - oh);       \
oh = s;                   \
}

#define stbsp__ddmultlo(oh, ol, xh, xl, yh, yl) ol = ol + (xh * yl + xl * yh);

#define stbsp__ddmultlos(oh, ol, xh, yl) ol = ol + (xh * yl);

static void stbsp__raise_to_power10(double *ohi, double *olo, double d, stbsp__int32 power) // power can be -323 to +350
{
  double ph, pl;
  if ((power >= 0) && (power <= 22)) {
    stbsp__ddmulthi(ph, pl, d, stbsp__bot[power]);
  } else {
    stbsp__int32 e, et, eb;
    double p2h, p2l;

    e = power;
    if (power < 0)
      e = -e;
    et = (e * 0x2c9) >> 14; /* %23 */
    if (et > 13)
      et = 13;
    eb = e - (et * 23);

    ph = d;
    pl = 0.0;
    if (power < 0) {
      if (eb) {
        --eb;
        stbsp__ddmulthi(ph, pl, d, stbsp__negbot[eb]);
        stbsp__ddmultlos(ph, pl, d, stbsp__negboterr[eb]);
      }
      if (et) {
        stbsp__ddrenorm(ph, pl);
        --et;
        stbsp__ddmulthi(p2h, p2l, ph, stbsp__negtop[et]);
        stbsp__ddmultlo(p2h, p2l, ph, pl, stbsp__negtop[et], stbsp__negtoperr[et]);
        ph = p2h;
        pl = p2l;
      }
    } else {
      if (eb) {
        e = eb;
        if (eb > 22)
          eb = 22;
        e -= eb;
        stbsp__ddmulthi(ph, pl, d, stbsp__bot[eb]);
        if (e) {
          stbsp__ddrenorm(ph, pl);
          stbsp__ddmulthi(p2h, p2l, ph, stbsp__bot[e]);
          stbsp__ddmultlos(p2h, p2l, stbsp__bot[e], pl);
          ph = p2h;
          pl = p2l;
        }
      }
      if (et) {
        stbsp__ddrenorm(ph, pl);
        --et;
        stbsp__ddmulthi(p2h, p2l, ph, stbsp__top[et]);
        stbsp__ddmultlo(p2h, p2l, ph, pl, stbsp__top[et], stbsp__toperr[et]);
        ph = p2h;
        pl = p2l;
      }
    }
  }
  stbsp__ddrenorm(ph, pl);
  *ohi = ph;
  *olo = pl;
}

// given a float value, returns the significant bits in bits, and the position of the
//   decimal point in decimal_pos.  +/-INF and NAN are specified by special values
//   returned in the decimal_pos parameter.
// frac_digits is absolute normally, but if you want from first significant digits (got %g and %e), or in 0x80000000
static stbsp__int32 stbsp__real_to_str(char const **start, stbsp__uint32 *len, char *out, stbsp__int32 *decimal_pos, double value, stbsp__uint32 frac_digits)
{
  double d;
  stbsp__int64 bits = 0;
  stbsp__int32 expo, e, ng, tens;

  d = value;
  STBSP__COPYFP(bits, d);
  expo = (stbsp__int32)((bits >> 52) & 2047);
  ng = (stbsp__int32)((stbsp__uint64) bits >> 63);
  if (ng)
    d = -d;

  if (expo == 2047) // is nan or inf?
  {
    *start = (bits & ((((stbsp__uint64)1) << 52) - 1)) ? "NaN" : "Inf";
    *decimal_pos = STBSP__SPECIAL;
    *len = 3;
    return ng;
  }

  if (expo == 0) // is zero or denormal
  {
    if (((stbsp__uint64) bits << 1) == 0) // do zero
    {
      *decimal_pos = 1;
      *start = out;
      out[0] = '0';
      *len = 1;
      return ng;
    }
    // find the right expo for denormals
    {
      stbsp__int64 v = ((stbsp__uint64)1) << 51;
      while ((bits & v) == 0) {
        --expo;
        v >>= 1;
      }
    }
  }

  // find the decimal exponent as well as the decimal bits of the value
  {
    double ph, pl;

    // log10 estimate - very specifically tweaked to hit or undershoot by no more than 1 of log10 of all expos 1..2046
    tens = expo - 1023;
    tens = (tens < 0) ? ((tens * 617) / 2048) : (((tens * 1233) / 4096) + 1);

    // move the significant bits into position and stick them into an int
    stbsp__raise_to_power10(&ph, &pl, d, 18 - tens);

    // get full as much precision from double-double as possible
    stbsp__ddtoS64(bits, ph, pl);

    // check if we undershot
    if (((stbsp__uint64)bits) >= stbsp__tento19th)
      ++tens;
  }

  // now do the rounding in integer land
  frac_digits = (frac_digits & 0x80000000) ? ((frac_digits & 0x7ffffff) + 1) : (tens + frac_digits);
  if ((frac_digits < 24)) {
    stbsp__uint32 dg = 1;
    if ((stbsp__uint64)bits >= stbsp__powten[9])
      dg = 10;
    while ((stbsp__uint64)bits >= stbsp__powten[dg]) {
      ++dg;
      if (dg == 20)
        goto noround;
    }
    if (frac_digits < dg) {
      stbsp__uint64 r;
      // add 0.5 at the right position and round
      e = dg - frac_digits;
      if ((stbsp__uint32)e >= 24)
        goto noround;
      r = stbsp__powten[e];
      bits = bits + (r / 2);
      if ((stbsp__uint64)bits >= stbsp__powten[dg])
        ++tens;
      bits /= r;
    }
    noround:;
  }

  // kill long trailing runs of zeros
  if (bits) {
    stbsp__uint32 n;
    for (;;) {
      if (bits <= 0xffffffff)
        break;
      if (bits % 1000)
        goto donez;
      bits /= 1000;
    }
    n = (stbsp__uint32)bits;
    while ((n % 1000) == 0)
      n /= 1000;
    bits = n;
    donez:;
  }

  // convert to string
  out += 64;
  e = 0;
  for (;;) {
    stbsp__uint32 n;
    char *o = out - 8;
    // do the conversion in chunks of U32s (avoid most 64-bit divides, worth it, constant denomiators be damned)
    if (bits >= 100000000) {
      n = (stbsp__uint32)(bits % 100000000);
      bits /= 100000000;
    } else {
      n = (stbsp__uint32)bits;
      bits = 0;
    }
    while (n) {
      out -= 2;
      *(stbsp__uint16 *)out = *(stbsp__uint16 *)&stbsp__digitpair.pair[(n % 100) * 2];
      n /= 100;
      e += 2;
    }
    if (bits == 0) {
      if ((e) && (out[0] == '0')) {
        ++out;
        --e;
      }
      break;
    }
    while (out != o) {
      *--out = '0';
      ++e;
    }
  }

  *decimal_pos = tens;
  *start = out;
  *len = e;
  return ng;
}

#undef stbsp__ddmulthi
#undef stbsp__ddrenorm
#undef stbsp__ddmultlo
#undef stbsp__ddmultlos
#undef STBSP__SPECIAL
#undef STBSP__COPYFP

// clean up
#undef stbsp__uint16
#undef stbsp__uint32
#undef stbsp__int32
#undef stbsp__uint64
#undef stbsp__int64
#undef STBSP__UNALIGNED


////////////////////////////////////////////////
////////////////////////////////////////////////
//////////   LOAD RUNTIME SYMBOLS   ////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

#if !MR4TH_DEFINE_RUNTIME_SYMBOLS

MR4TH_BEFORE_MAIN(base_before_main){

  // load module
#if OS_WINDOWS
  HMODULE module = LoadLibraryA("mr4th_base.dll");
  B32 good = (module != 0);
#elif OS_LINUX
  void *module = dlopen("$ORIGIN/mr4th_base.so", RTLD_NOW);
  B32 good = (module != 0);
#elif OS_MAC
  void *module = dlopen("$ORIGIN/mr4th_base.so", RTLD_NOW);
  B32 good = (module != 0);
#else
# error Missing runtime symbol loading for this OS
#endif

  if (good){

    // define (OS_)LOAD_SYMBOL
#if OS_WINDOWS
# define OS_LOAD_SYMBOL(n) *(VoidFunc**)(&n) = (VoidFunc*)GetProcAddress(module, #n)
#elif OS_LINUX
# define OS_LOAD_SYMBOL(n) *(VoidFunc**)(&n) = (VoidFunc*)dlsym(module, #n)
#elif OS_MAC
# define OS_LOAD_SYMBOL(n) *(VoidFunc**)(&n) = (VoidFunc*)dlsym(module, #n)
#else
# error Missing OS_LOAD_SYMBOL macro for runtime symbol loading on this OS
#endif

#define LOAD_SYMBOL(n) (OS_LOAD_SYMBOL(n), ((n)==0)?(good=0):1)

    // Load: Arena
    LOAD_SYMBOL(arena_get_scratch);

    // Load: Log
    LOAD_SYMBOL(log_accum_begin);
    LOAD_SYMBOL(log_gathering);
    LOAD_SYMBOL(log_emit);
    LOAD_SYMBOL(log_emitf);
    LOAD_SYMBOL(log_accum_end);

    // Load: Errors
    LOAD_SYMBOL(er_accum_begin);
    LOAD_SYMBOL(er_emit);
    LOAD_SYMBOL(er_emitf);
    LOAD_SYMBOL(er_accum_end);

#undef OS_LOAD_SYMBOL
#undef LOAD_SYMBOL
  }

  // TODO(allen): if (!good) OS specific fatal error message handling
}

#endif
