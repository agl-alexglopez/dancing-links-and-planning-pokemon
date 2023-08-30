#include "window.hh"
#include "GLFW/glfw3.h"
#include <GL/gl.h>

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

void Window::poll()
{
  glfwSwapBuffers( window_ );
  glfwPollEvents();
}

void Window::clear()
{
  glClear( GL_COLOR_BUFFER_BIT );
}

void Window::triangle_test()
{
  // Remember, this is not recommended use of old OpenGL. Just a test with colors and a shape.
  glBegin( GL_TRIANGLES );
  glColor3f( 1.0F, 0.0F, 0.0F );
  glVertex2f( -0.75F, -0.75F );
  glColor3f( 0.0F, 1.0F, 0.0F );
  glVertex2f( 0.0F, 0.75F );
  glColor3f( 0.0F, 0.0F, 1.0F );
  glVertex2f( 0.75F, -0.75F );
  glEnd();
}

} // namespace Gui
