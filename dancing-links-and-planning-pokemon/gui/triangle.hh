#pragma once
#ifndef TRIANGLE_HH
#    define TRIANGLE_HH

#    include "point.hh"
#    include "shader.hh"
#    include "util.hh"
#    include "vertex.hh"

#    include <vector>

namespace Gui {

class Triangle {
  public:
    struct Vertices
    {
        Point p1;
        Point p2;
        Point p3;
    };

    explicit Triangle(Vertex_fragment shaders, Vertices points);

    static void draw();

  private:
    std::vector<float> positions_;
    Vertex vertex_;
    Shader shaders_{};
};

} // namespace Gui

#endif // TRIANGLE_HH
