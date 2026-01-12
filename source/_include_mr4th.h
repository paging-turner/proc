#define MR4TH_NO_INCLUDES 1
#define MR4TH_NO_CLAMP 1
#if !No_Assert
# define MR4TH_ASSERTS 1
#endif
#include "../libraries/mr4th/src/mr4th_base.h"
#define push_struct(a, s) arena_push((a), sizeof(s))
