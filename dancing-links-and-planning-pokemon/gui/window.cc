#include "window.hh"
#include <GL/gl.h>
#include "GLFW/glfw3.h"

#include <iostream>

namespace Gui {

namespace {

void error_callback( int error, const char* description )
{
  std::cerr << "A window error occurred: " << description << "\n";
}

void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods )
{
  if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS ) {
    glfwSetWindowShouldClose( window, GLFW_TRUE );
  }
}

void frame_buffer_resize_callback( GLFWwindow* window, int width, int height )
{
  glViewport( 0, 0, width, height );
}

} // namespace

Window::Window( const Window::Window_args& args )
{
  glfwSetErrorCallback( error_callback );
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
  glfwMakeContextCurrent( window_ );
  glfwSetKeyCallback( window_, key_callback );
  glfwSetFramebufferSizeCallback( window_, frame_buffer_resize_callback );
}

Window::~Window()
{
  glfwTerminate();
}

bool Window::error() const
{
  return error_;
}

bool Window::should_close() const
{
  return glfwWindowShouldClose( window_ );
}

void Window::poll( const Window& window )
{
  glfwPollEvents();
  glfwSwapBuffers( window.window_ );
}

} // namespace Gui
