/// MIT License
///
/// Copyright (c) 2023 Alex G. Lopez
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
module;
#include <cstdint>
#include <ostream>
export module dancing_links:resistance;
import :type_encoding;

///////////////////////////////////////   Exported Interface

export namespace Dancing_links {

enum Multiplier : uint8_t
{
    /// It would not make sense for someone to let a multiplier in a Resistance
    /// default to IMMUNE, because that is a valuable multiplier to have for a
    /// Pokemon. Make sure you initialize multipliers unless you want an EMPTY_
    /// placeholder.
    emp = 0,
    imm,
    f14, // x0.25 damage aka the fraction 1/4
    f12, // x0.5 damage aka the fraction 1/2
    nrm, // normal
    dbl, // double or 2x damage
    qdr  // quadruple or 4x damage.
};

class Resistance {
  public:
    Resistance(Type_encoding const &type, Multiplier const &multiplier);

    Resistance(Resistance const &other) = default;
    Resistance(Resistance &&other) noexcept = default;
    Resistance &operator=(Resistance &&other) = default;
    Resistance &operator=(Resistance const &other) = default;
    ~Resistance() = default;

    [[nodiscard]] Type_encoding type() const;

    [[nodiscard]] Multiplier multiplier() const;

    bool
    operator<(Resistance const &rhs) const
    {
        return this->type() < rhs.type();
    }
    bool
    operator==(Resistance const &rhs) const
    {
        return this->type() == rhs.type()
               && this->multiplier() == rhs.multiplier();
    }
    bool
    operator>(Resistance const &rhs) const
    {
        return rhs < *this;
    }
    bool
    operator>=(Resistance const &rhs) const
    {
        return !(*this < rhs);
    }
    bool
    operator<=(Resistance const &rhs) const
    {
        return !(*this > rhs);
    }
    bool
    operator!=(Resistance const &rhs) const
    {
        return !(*this == rhs);
    }

  private:
    Type_encoding type_;
    Multiplier multiplier_;
};

std::ostream &operator<<(std::ostream &out, Resistance const &res);
std::ostream &operator<<(std::ostream &out, Multiplier const &mult);

////////////////////////      Free Functions for TypeResistance.h

Type_encoding
type(Resistance const &res)
{
    return res.type();
}

Multiplier
multiplier(Resistance const &res)
{
    return res.multiplier();
}

} // namespace Dancing_links

////////////////////////////////////////   Implementation

namespace Dancing_links {

/////////////////////////     Resistance Helper Class

Resistance::Resistance(Type_encoding const &type, Multiplier const &multiplier)
    : type_(type), multiplier_(multiplier)
{}

Type_encoding
Resistance::type() const
{
    return type_;
}

Multiplier
Resistance::multiplier() const
{
    return multiplier_;
}

std::ostream &
operator<<(std::ostream &out, Resistance const &res)
{
    out << res.type() << " x";
    switch (res.multiplier())
    {
    case emp:
        out << "NIL";
        break;
    case imm:
        out << "0.0";
        break;
    case f14:
        out << "0.25";
        break;
    case f12:
        out << "0.5";
        break;
    case nrm:
        out << "1.0";
        break;
    case dbl:
        out << "2.0";
        break;
    case qdr:
        out << "4.0";
        break;
    }
    return out;
}

std::ostream &
operator<<(std::ostream &out, Multiplier const &mult)
{
    out << "Resistance::";
    switch (mult)
    {
    case emp:
        out << "emp";
        break;
    case imm:
        out << "imm";
        break;
    case f14:
        out << "f14";
        break;
    case f12:
        out << "f12";
        break;
    case nrm:
        out << "nrm";
        break;
    case dbl:
        out << "dbl";
        break;
    case qdr:
        out << "qdr";
        break;
    }
    return out;
}

} // namespace Dancing_links
