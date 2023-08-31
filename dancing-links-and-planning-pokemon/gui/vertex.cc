#include "vertex.hh"

#include <GL/glew.h>

#include <iostream>
#include <utility>

namespace Gui {

Vertex::Vertex( std::vector<float> vertex ) : vertex_ { std::move( vertex ) }
{
  glGenBuffers( 1, &id_ );
  glBindBuffer( GL_ARRAY_BUFFER, id_ );
  glBufferData( GL_ARRAY_BUFFER, vertex_.size() * sizeof( float ), vertex_.data(), GL_STATIC_DRAW );
  glEnableVertexAttribArray( 0 );
  glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 2, 0 );
}

void Vertex::draw( Vertex::Draw_command command )
{
  if ( command.mode == Primitive::triangle ) {
    glDrawArrays( GL_TRIANGLES, command.index, command.count );
  } else {
    std::cerr << "unknown vertex draw command was issued to vertex.\n";
  }
}

} // namespace Gui