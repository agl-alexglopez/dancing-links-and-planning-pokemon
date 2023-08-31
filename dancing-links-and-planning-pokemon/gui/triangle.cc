#include "triangle.hh"

namespace Gui {

Triangle::Triangle( const std::vector<float>& vertex ) : vertex_( vertex ) {}

void Triangle::draw( Triangle::Index_count pos )
{
  Vertex::draw( { Vertex::Primitive::triangle, pos.index, pos.count } );
}

} // namespace Gui