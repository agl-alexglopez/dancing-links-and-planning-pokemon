#include "shader.hh"
#include "util.hh"

#include <GL/glew.h>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <vector>

namespace Gui {

namespace {

uint32_t compile_shader( uint32_t type, std::string_view src )
{
  const unsigned int id = glCreateShader( type );
  const char* s = src.data();
  glShaderSource( id, 1, &s, nullptr );
  glCompileShader( id );
  int result {};
  glGetShaderiv( id, GL_COMPILE_STATUS, &result );
  if ( result == GL_FALSE ) {
    int length {};
    glGetShaderiv( id, GL_INFO_LOG_LENGTH, &length );
    std::vector<char> msg( length );
    glGetShaderInfoLog( id, length, &length, msg.data() );
    std::cerr << "Failed to compile " << ( type == GL_VERTEX_SHADER ? "vertex" : "fragment" ) << " shader!\n";
    std::cerr << msg.data() << "\n";
    glDeleteShader( id );
    return 0;
  }
  return id;
}

} // namespace

Shader::~Shader()
{
  glDeleteProgram( program_ );
}

Shader::Shader( Vertex_fragment vf )
  : program_( glCreateProgram() )
  , vertex_shader_( compile_shader( GL_VERTEX_SHADER, vf.vert ) )
  , fragment_shader_( compile_shader( GL_FRAGMENT_SHADER, vf.frag ) )
{
  if ( !vertex_shader_ || !fragment_shader_ ) {
    std::cerr << "Nothing provided as shader data\n";
    std::abort();
  }
  glAttachShader( program_, vertex_shader_ );
  glAttachShader( program_, fragment_shader_ );
  glLinkProgram( program_ );
  glValidateProgram( program_ );
  glDeleteShader( vertex_shader_ );
  glDeleteShader( fragment_shader_ );
  glUseProgram( program_ );
}

} // namespace Gui
