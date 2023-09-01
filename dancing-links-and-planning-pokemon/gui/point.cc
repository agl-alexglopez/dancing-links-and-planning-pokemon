#include "point.hh"

namespace Gui {

std::ostream& operator<<( std::ostream& out, const Point& p )
{
  return out << "{" << p.x << "," << p.y << "}";
}

bool operator==( const Point& p1, const Point& p2 )
{
  return p1.x == p2.x && p1.y == p2.y;
}

bool operator!=( const Point& p1, const Point& p2 )
{
  return !( p1 == p2 );
}

bool operator<( const Point& p1, const Point& p2 )
{
  return p1.x < p2.x || ( p1.x == p2.x && p1.y < p2.y );
}

bool operator<=( const Point& p1, const Point& p2 )
{
  return !( p2 < p1 );
}

bool operator>( const Point& p1, const Point& p2 )
{
  return p2 < p1;
}

bool operator>=( const Point& p1, const Point& p2 )
{
  return !( p1 < p2 );
}

Point operator*( const Point& p1, float scale )
{
  return { p1.x * scale, p1.y * scale };
}

} // namespace Gui
