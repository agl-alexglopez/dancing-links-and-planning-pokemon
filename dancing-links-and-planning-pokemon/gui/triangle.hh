#ifndef TRIANGLE_HH
#define TRIANGLE_HH

#include "point.hh"
#include "shader.hh"
#include "util.hh"
#include "vertex.hh"

#include <vector>

namespace Gui {

class Triangle
{
public:

  struct Vertices
  {
    Point p1;
    Point p2;
    Point p3;
  };

  struct Index_count
  {
    int index;
    int count;
  };

  explicit Triangle( Vertex_fragment shaders, Vertices points );

  static void draw( Index_count pos );

private:
  Vertex vertex_ {};
  Shader shaders_ {};
};

} // namespace Gui

#endif // TRIANGLE_HH
