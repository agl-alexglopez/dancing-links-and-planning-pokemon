#include "vertex.hh"

#include <GL/glew.h>

#include <cstdint>
#include <iostream>
#include <span>
#include <utility>
#include <vector>

namespace Gui {

Vertex::Vertex( std::span<float> vertex ) : vertex_ { vertex }
{
  glGenBuffers( 1, &id_ );
  glBindBuffer( GL_ARRAY_BUFFER, id_ );
  glBufferData( GL_ARRAY_BUFFER, vertex_.size_bytes(), vertex_.data(), GL_STATIC_DRAW ); // NOLINT
  glEnableVertexAttribArray( 0 );
  glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 2, nullptr );
}

Vertex::Vertex( std::span<float> vertex, std::span<const uint32_t> indices )
  : vertex_ { vertex }, index_ { indices }
{
  glGenBuffers( 1, &id_ );
  glBindBuffer( GL_ARRAY_BUFFER, id_ );
  glBufferData( GL_ARRAY_BUFFER, index_.size_bytes() * 2, vertex_.data(), GL_STATIC_DRAW ); // NOLINT
  glEnableVertexAttribArray( 0 );
  glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 2, nullptr );
  glGenBuffers( 1, &index_id_ );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index_id_ );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER, index_.size_bytes(), index_.data(), GL_STATIC_DRAW ); // NOLINT
}

void Vertex::draw( Vertex::Draw_command command )
{
  switch ( command.mode ) {
    case Primitive::triangle:
      glDrawArrays( GL_TRIANGLES, command.index, command.count );
      break;
    case Primitive::quad:
      glDrawElements( GL_TRIANGLES, command.count, GL_UNSIGNED_INT, nullptr );
      break;
    default:
      std::cerr << "unknown vertex draw command was issued to vertex.\n";
  };
}

} // namespace Gui
