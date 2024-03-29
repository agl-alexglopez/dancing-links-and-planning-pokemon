#include "triangle.hh"
#include "point.hh"
#include "shader.hh"
#include "util.hh"
#include "vertex.hh"

#include <vector>

namespace Gui {

Triangle::Triangle(Vertex_fragment shaders, Triangle::Vertices points)
    : positions_{{points.p1.x, points.p1.y, points.p2.x, points.p2.y,
                  points.p3.x, points.p3.y}},
      vertex_{positions_}, shaders_(shaders)
{}

void
Triangle::draw()
{
    Vertex::draw({Vertex::Primitive::triangle, 0, 3});
}

} // namespace Gui
