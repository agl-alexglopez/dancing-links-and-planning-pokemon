#pragma once
#ifndef SHADER_HH
#define SHADER_HH

#include "util.hh"

#include <GL/glew.h>
#include <cstdint>
#include <string_view>

namespace Gui {

class Shader
{

public:
  Shader() = default;
  explicit Shader( Vertex_fragment vf );
  Shader& operator=( const Shader& other ) = delete;
  Shader& operator=( const Shader&& other ) noexcept = delete;
  Shader( const Shader& other ) = delete;
  Shader( const Shader&& other ) noexcept = delete;
  ~Shader();

private:
  uint32_t program_ { 0 };
  uint32_t vertex_shader_ { 0 };
  uint32_t fragment_shader_ { 0 };
};

} // namespace Gui

#endif // SHADER_HH
