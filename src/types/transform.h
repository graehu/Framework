#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "vec3f.h"
#include "mat4x4f.h"

namespace core
{
 struct transform
 {
 transform(vec3f p, mat4x4f m) : position(p), orient(m) {}

    vec3f to_local(const vec3f& _point) const
    {
       return (orient)*_point + position;
    }
    vec3f position;
    //todo: this should be a quat....
    mat4x4f orient;
 };  
}
#endif//TRANSFORM_H
