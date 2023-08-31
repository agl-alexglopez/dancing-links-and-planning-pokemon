#ifndef VERTEX_HH
#define VERTEX_HH

#include <vector>

namespace Gui {

class Vertex
{

public:
  enum class Primitive
  {
    triangle,
  };

  struct Draw_command
  {
    Primitive mode;
    int index;
    int count;
  };

  Vertex() = default;
  explicit Vertex( std::vector<float> vertex );
  static void draw( Draw_command command );

private:
  std::vector<float> vertex_;
  uint32_t id_ {};
};

} // namespace Gui

#endif // VERTEX_HH
