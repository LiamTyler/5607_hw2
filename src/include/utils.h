#ifndef SRC_INCLUDE_UTILS_H_
#define SRC_INCLUDE_UTILS_H_

#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include <iostream>

inline std::istream& operator>>(std::istream& in, glm::vec3& v) {
    in >> v.x >> v.y >> v.z;
    return in;
}

inline std::ostream& operator<<(std::ostream& out, const glm::vec3& v) {
    out << v.x << " " << v.y << " " << v.z;
    return out ;
}

#endif  // SRC_INCLUDE_UTILS_H_

