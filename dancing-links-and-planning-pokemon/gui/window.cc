#include "window.hh"

#include <iostream>

namespace Gui {

namespace {

void error_callback( int error, const char* description )
{
  std::cerr << "A window error occurred with num : " << error << "\n";
  std::cerr << "Description: " << description << "\n";
}

void key_callback( GLFWwindow* window,
                   int key, // NOLINT
                   int scancode [[maybe_unused]], // NOLINT
                   int action [[maybe_unused]], // NOLINT
                   int mods [[maybe_unused]] ) // NOLINT
{
  if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS ) {
    glfwSetWindowShouldClose( window, GLFW_TRUE );
  }
}

void frame_buffer_resize_callback( GLFWwindow* window [[maybe_unused]], int width, int height )
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
    return;
  }
  glfwMakeContextCurrent( window_ );
  if ( glewInit() != GLEW_OK ) {
    error_ = true;
    glfwTerminate();
    return;
  }
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

void Window::poll()
{
  glfwSwapBuffers( window_ );
  glfwPollEvents();
}

void Window::clear()
{
  glClear( GL_COLOR_BUFFER_BIT );
}

} // namespace Gui
