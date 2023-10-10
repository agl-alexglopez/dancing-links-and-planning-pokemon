#include "quad.hh"

namespace Gui {

Quad::Quad( Vertex_fragment shaders, Quad::Vertices points )
  : positions_ { { points.p1.x,
                   points.p1.y,
                   points.p2.x,
                   points.p2.y,
                   points.p3.x,
                   points.p3.y,
                   points.p4.x,
                   points.p4.y } }
  , vertex_( { positions_ }, { indx_ } )
  , shaders_( shaders )
{}

void Quad::draw()
{
  Vertex::draw( { Vertex::Primitive::quad, 0, 6 } );
}

} // namespace Gui
