#pragma once
#ifndef QUAD_HH
#define QUAD_HH

#include "point.hh"
#include "shader.hh"
#include "util.hh"
#include "vertex.hh"

#include <array>
#include <cstdint>
#include <vector>

namespace Gui {

class Quad
{
public:
  struct Vertices
  {
    Point p1;
    Point p2;
    Point p3;
    Point p4;
  };

  explicit Quad( Vertex_fragment shaders, Vertices points );

  static void draw();

private:
  static constexpr std::array<uint32_t, 6> indx { 0, 1, 2, 2, 3, 0 };
  std::vector<float> positions_ {};
  Vertex vertex_;
  Shader shaders_ {};
};

} // namespace Gui

#endif // QUAD_HH
