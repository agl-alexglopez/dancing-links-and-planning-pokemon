#include "triangle.hh"

namespace Gui {

Triangle::Triangle( Vertex_fragment shaders, Triangle::Vertices points )
  : vertex_ { { points.p1.x, points.p1.y, points.p2.x, points.p2.y, points.p3.x, points.p3.y } },
    shaders_( shaders ) {}

void Triangle::draw( Index_count pos )
{
  Vertex::draw( { Vertex::Primitive::triangle, pos.index, pos.count } );
}

} // namespace Gui