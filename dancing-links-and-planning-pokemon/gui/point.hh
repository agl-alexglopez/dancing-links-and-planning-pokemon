#pragma once
#ifndef POINT_HH
#define POINT_HH

#include <iostream>

namespace Gui {

class Point
{
public:
  Point() = default;
  Point( float user_x, float user_y ) : x( user_x ), y( user_y ) {}
  float x { 0 };
  float y { 0 };
}; // class Point

std::ostream& operator<<( std::ostream& out, const Point& p );
bool operator==( const Point& p1, const Point& p2 );
bool operator!=( const Point& p1, const Point& p2 );
bool operator<( const Point& p1, const Point& p2 );
bool operator<=( const Point& p1, const Point& p2 );
bool operator>( const Point& p1, const Point& p2 );
bool operator>=( const Point& p1, const Point& p2 );
Point operator*( const Point& p1, float scale );

} // namespace Gui

namespace std {
template<>
struct hash<Gui::Point>
{
  size_t operator()( const Gui::Point& p ) const noexcept
  {
    return hash<float>()( p.x ) ^ ( hash<float>()( p.y ) << 1U );
  }
};
} // namespace std

#endif // POINT_HH
