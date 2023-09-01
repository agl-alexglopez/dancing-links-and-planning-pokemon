#ifndef UTIL_HH
#define UTIL_HH

#include <string_view>

namespace Gui {

struct Vertex_fragment
{
  std::string_view vert;
  std::string_view frag;
};

} // namespace Gui

#endif // UTIL_HH
