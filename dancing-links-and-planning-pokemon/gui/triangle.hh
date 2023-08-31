#ifndef TRIANGLE_HH
#define TRIANGLE_HH

#include "vertex.hh"

#include <vector>

namespace Gui {

class Triangle
{
public:
  struct Index_count
  {
    int index;
    int count;
  };

  explicit Triangle( const std::vector<float>& vertex );

  static void draw( Index_count pos );

private:
  Vertex vertex_ {};
};

} // namespace Gui

#endif // TRIANGLE_HH
