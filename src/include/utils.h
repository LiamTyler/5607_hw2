#ifndef SRC_INCLUDE_UTILS_H_
#define SRC_INCLUDE_UTILS_H_

#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include <iostream>

using glm::vec3;
using glm::vec4;

inline std::istream& operator>>(std::istream& in, vec3& v) {
    in >> v.x >> v.y >> v.z;
    return in;
}

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
    out << v.x << " " << v.y << " " << v.z;
    return out ;
}

#endif  // SRC_INCLUDE_UTILS_H_

