#ifndef COLLISION_MANAGER_H
#define COLLISION_MANAGER_H

namespace physics
{
   class collision_manager
   {
     public:
      //Some sort of space partitioning.
      //all of the bodies.
      //on conllsion add collision data to bodies.
      //bodies move themselves.
      //work out next frames movement for bodies and do collisions based on that?
      //multi sampled collisions
      static void update();
   };
}
#endif//COLLISION_MANAGER_H
