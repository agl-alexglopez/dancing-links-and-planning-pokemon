#ifndef WINDOW_HH
#define WINDOW_HH

#include <GLFW/glfw3.h>

#include <string>

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

  explicit Window( const Window_args& args );

  Window( const Window& ) = delete;
  Window& operator=( const Window& ) = delete;
  Window( Window&& other ) noexcept = default;
  Window& operator=( Window&& other ) noexcept = default;
  ~Window();
  bool error() const;
  bool should_close() const;

  static void poll( const Window& window );

private:
  bool error_ { false };
  GLFWwindow* window_;
  GLFWmonitor* monitor_;
  GLFWwindow* share_;

  void swap_bufers( const Window& window );

}; // class Window

} // namespace Gui

#endif // WINDOW_HH
