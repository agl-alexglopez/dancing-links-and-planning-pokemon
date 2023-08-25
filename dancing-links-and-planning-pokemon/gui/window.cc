#include "window.hh"
#include "GLFW/glfw3.h"
#include <GL/gl.h>

namespace Gui {

Window::Window( const Window::Window_args& args ) {
  if ( !glfwInit() ) {
    error_ = true;
    return;
  }
  monitor_ = args.monitor;
  share_ = args.share;
  glfwWindowHint( GLFW_RESIZABLE, GLFW_TRUE );
  window_ = glfwCreateWindow( args.width, args.height, args.title.data(), monitor_, share_ );
  if ( !window_ ) {
    error_ = true;
    glfwTerminate();
  }
}

Window::~Window() {
  glfwTerminate();
}

bool Window::error() const {
  return error_;
}

bool Window::should_close() const {
  return glfwWindowShouldClose( window_ );
}


} // namespace Gui
