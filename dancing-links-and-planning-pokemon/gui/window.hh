#ifndef WINDOW_HH
#define WINDOW_HH

#include <GL/glew.h> // NOLINT (glew must always come first!)
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

namespace Gui {

class Window
{

public:
  struct Window_args
  {
    int width;
    int height;
    std::string title;
    GLFWmonitor* monitor;
    GLFWwindow* share;
  };

  // Static functions usually for strictly glfw or GL code.
  static void clear();

  explicit Window( const Window_args& args );
  Window( const Window& ) = delete;
  Window& operator=( const Window& ) = delete;
  Window( Window&& other ) noexcept = default;
  Window& operator=( Window&& other ) noexcept = default;
  ~Window();
  void poll();
  bool error() const;
  bool should_close() const;

private:
  bool error_ { false };
  GLFWwindow* window_;
  GLFWmonitor* monitor_;
  GLFWwindow* share_;

}; // class Window

} // namespace Gui

#endif // WINDOW_HH
