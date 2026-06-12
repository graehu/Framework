
#define TRACY_ENABLE 1
#include "tracy/Tracy.hpp"

#define PROFILE ZoneScoped
#define PROFILE_SCOPE(name) ZoneScopedN(name)
#define PROFILE_FRAME FrameMark