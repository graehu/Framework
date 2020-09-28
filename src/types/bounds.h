#include "vec3f.h"
#include <cstdlib>
struct bounds
{
   vec3f center;
   vec3f extents;
   float size = 0;
   bool touching(const bounds& _rhs)
   {
      //circum-sphere touching.
      float distance = (_rhs.center-center).length();
      float max_distance = _rhs.size+size;
      return distance < max_distance;
   }
   void encapsulate(vec3f point)
   {
      vec3f line = point-center;
      float absi = abs(line.i);
      float absj = abs(line.j);
      float absk = abs(line.k);
      extents.i = absi > extents.i ? absi : extents.i;
      extents.j = absj > extents.j ? absj : extents.j;
      extents.k = absk > extents.k ? absk : extents.k;
      size = extents.length();
   }
};
