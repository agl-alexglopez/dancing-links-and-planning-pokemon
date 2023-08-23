/**
 * MIT License
 *
 * Copyright (c) 2023 Alex G. Lopez
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "resistance.hh"
#include <utility>

namespace Dancing_links {

/* * * * * * * * * *      Free Functions for TypeResistance.h       * * * * * * * * * * * * * * * */

Type_encoding type( const Resistance& res )
{
  return res.type();
}

Multiplier multiplier( const Resistance& res )
{
  return res.multiplier();
}

/* * * * * * * * * * * * *     Resistance Helper Class      * * * * * * * * * * * * * * * * * * * */

Resistance::Resistance( const Type_encoding& type, const Multiplier& multiplier )
  : type_( type ), multiplier_( multiplier )
{}

Type_encoding Resistance::type() const
{
  return type_;
}

Multiplier Resistance::multiplier() const
{
  return multiplier_;
}

std::ostream& operator<<( std::ostream& out, const Resistance& res )
{
  out << res.type() << " x";
  switch ( res.multiplier() ) {
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

std::ostream& operator<<( std::ostream& out, const Multiplier& mult )
{
  out << "Resistance::";
  switch ( mult ) {
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
