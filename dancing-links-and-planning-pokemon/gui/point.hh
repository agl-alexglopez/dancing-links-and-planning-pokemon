#ifndef POINT_HH
#define POINT_HH

#include <iostream>

namespace Gui {

class Point
{
public:
  Point( double user_x, double user_y ) : x( user_x ), y( user_y ) {}
  double x { 0 };
  double y { 0 };
}; // class Point

std::ostream& operator<<( std::ostream& out, const Point& p );
bool operator==( const Point& p1, const Point& p2 );
bool operator!=( const Point& p1, const Point& p2 );
bool operator<( const Point& p1, const Point& p2 );
bool operator<=( const Point& p1, const Point& p2 );
bool operator>( const Point& p1, const Point& p2 );
bool operator>=( const Point& p1, const Point& p2 );
Point operator*( const Point& p1, double scale );

} // namespace Gui

namespace std {
template<>
struct hash<Gui::Point>
{
  size_t operator()( const Gui::Point& p ) const noexcept
  {
    return hash<double>()( p.x ) ^ ( hash<double>()( p.y ) << 1U );
  }
};
} // namespace std

#endif // POINT_HH
