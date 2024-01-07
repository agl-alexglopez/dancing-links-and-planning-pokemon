module;
#include <compare>
#include <cstddef>
#include <functional>
#include <iostream>
export module dancing_links:point;

////////////////////////////////////////   Exported Interface   ///////////////////////////////////////////////////

export namespace Dancing_links {

class Point
{
public:
  Point() = default;
  Point( float user_x, float user_y ) : x( user_x ), y( user_y )
  {}
  float x { 0 };
  float y { 0 };
}; // class Point

std::ostream& operator<<( std::ostream& out, const Point& p );
bool operator==( const Point& lhs, const Point& rhs );
std::partial_ordering operator<=>( const Point& lhs, const Point& rhs );
Point operator*( const Point& p1, float scale );

} // namespace Dancing_links

///////////////////////////////////////   Implementation   ////////////////////////////////////////////////////////

namespace std {
template<>
struct hash<Dancing_links::Point>
{
  size_t operator()( const Dancing_links::Point& p ) const noexcept
  {
    return hash<float>()( p.x ) ^ ( hash<float>()( p.y ) << 1U );
  }
};
} // namespace std

namespace Dancing_links {

std::ostream& operator<<( std::ostream& out, const Point& p )
{
  return out << "{" << p.x << "," << p.y << "}";
}

bool operator==( const Point& lhs, const Point& rhs )
{
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

std::partial_ordering operator<=>( const Point& lhs, const Point& rhs )
{
  const auto cmp = lhs.x <=> rhs.x;
  return cmp == std::partial_ordering::equivalent ? lhs.y <=> rhs.y : cmp;
}

Point operator*( const Point& p1, float scale )
{
  return { p1.x * scale, p1.y * scale };
}

} // namespace Dancing_links
