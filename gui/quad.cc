#include "quad.hh"
#include "util.hh"
#include "vertex.hh"

#include <vector>

namespace Gui {

Quad::Quad(Vertex_fragment shaders, Quad::Vertices points)
    : positions_{{
          points.p1.x,
          points.p1.y,
          points.p2.x,
          points.p2.y,
          points.p3.x,
          points.p3.y,
          points.p4.x,
          points.p4.y,
      }},
      vertex_({positions_}, {indx}), shaders_(shaders)
{}

void
Quad::draw()
{
    Vertex::draw({
        .mode = Vertex::Primitive::quad,
        .index = 0,
        .count = 6,
    });
}

} // namespace Gui
