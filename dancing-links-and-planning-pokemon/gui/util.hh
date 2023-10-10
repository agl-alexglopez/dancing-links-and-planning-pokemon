#ifndef UTIL_HH
#define UTIL_HH

#include <string_view>

namespace Gui {

struct Vertex_fragment
{
  std::string_view vert;
  std::string_view frag;
};

struct Index_count
{
  int index;
  int count;
};

} // namespace Gui

#endif // UTIL_HH
