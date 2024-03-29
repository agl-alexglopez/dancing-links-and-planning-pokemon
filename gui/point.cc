#include "point.hh"
#include <compare>
#include <iostream>

namespace Gui {

std::ostream &
operator<<(std::ostream &out, const Point &p)
{
    return out << "{" << p.x << "," << p.y << "}";
}

bool
operator==(const Point &lhs, const Point &rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

std::partial_ordering
operator<=>(const Point &lhs, const Point &rhs)
{
    const auto cmp = lhs.x <=> rhs.x;
    return cmp == std::partial_ordering::equivalent ? lhs.y <=> rhs.y : cmp;
}

Point
operator*(const Point &p1, float scale)
{
    return {p1.x * scale, p1.y * scale};
}

} // namespace Gui
