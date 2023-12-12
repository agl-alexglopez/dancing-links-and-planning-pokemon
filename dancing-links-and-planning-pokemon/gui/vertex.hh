#pragma once
#ifndef VERTEX_HH
#define VERTEX_HH

#include <cstdint>
#include <span>

namespace Gui {

class Vertex
{

public:
  enum class Primitive
  {
    triangle,
    quad,
  };

  struct Draw_command
  {
    Primitive mode;
    int index;
    int count;
  };

  ~Vertex();
  explicit Vertex( std::span<float> vertex );
  explicit Vertex( std::span<float> vertex, std::span<const uint32_t> indices );
  static void draw( Draw_command command );

  Vertex( const Vertex& ) = delete;
  Vertex( Vertex&& ) = delete;
  Vertex& operator=( const Vertex& ) = delete;
  Vertex& operator=( Vertex&& ) = delete;

private:
  std::span<float> vertex_;
  std::span<const uint32_t> index_;
  uint32_t vertex_id_ { 0 };
  uint32_t index_id_ { 0 };
  static constexpr uint32_t quad_indices = 6;
};

} // namespace Gui

#endif // VERTEX_HH
