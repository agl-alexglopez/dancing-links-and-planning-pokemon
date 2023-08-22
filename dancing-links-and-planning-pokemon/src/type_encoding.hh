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
 *
 * Author: Alexander G. Lopez
 * File: Type_encoding.h
 * --------------------------
 * This file contains a simple type used to encode Pokemon types into a 32 bit unsigned integer.
 * These algorithms may generate many solutions. Instead of storing strings with the type names
 * in our internal data structures and solution sets we will encode the types we are working
 * with into bits. From the 0th index bit, zero being the least significant bit position, we
 * have the following types.
 *
 *  0      1    2     3       4      5     6     7     8     9     10    11     12     13     14       15   16   17
 * Water,Steel,Rock,Psychic,Poison,Normal,Ice,Ground,Grass,Ghost,Flying,Fire,Fighting,Fairy,Electric,Dragon,Dark,Bug
 *
 * We have to be slightly creative with the bit representation to ensure lexicographic ordering of
 * the encoding even when it is in its unsigned integer form. The lowest string by lexicographic
 * order, Bug, will actually be our highest order bit and largest value. This ensures that any dual
 * typing that starts with "Bug" will always be a larger number than one that starts with "Dark,"
 * for example. In the same way, any string that starts wit "Bug", would always be sorted
 * lexicographicly before one that starts with "Dark." Then, we simply reverse the less than
 * operator for the Type_encoding and we can use this type as keys in sets, in maps, or elements in
 * binary searches and they will behave as if they are strings, but all comparisons are much more
 * efficient. Consider why this would NOT work if we put the "Bug" bit at the Least Significant Bit
 * position, the 0th index
 *
 * This means that we can be consistent when decoding the bits back to a string with the
 * lexicographic ordering I am using for the entire project. As an example the Dragon-Flying type
 * would be the following hex and binary value.
 *
 *         |---------------------1
 *         |      |-------------------1
 *      Dragon-Flying = 0x84 = 0b10000100
 *
 * Storing these types masked in a simple integer makes searching for them in our links, adding
 * or removing them to sets, or copying the dancing links class instance  much easier and
 * faster. We then simply provide a method to convert the encoding back to a string and we only
 * have to use that method when output is desired at the last moment, like when we want to print
 * the type names to a GUI in an ostream. Also, because we need to maintain a table of strings
 * somewhere to do our initial encoding we will just provide a string_view of the table entries
 * that make up our types. We dont have to create any heap strings.
 */
#ifndef TYPEENCODING_HH
#define TYPEENCODING_HH

#include <gtest/gtest_prod.h>

#include <array>
#include <cstdint>
#include <ostream>
#include <string_view>
#include <utility>

namespace Dancing_links {

class Type_encoding
{

public:
  Type_encoding() = default;
  // If encoding cannot be found encoding_ is set the falsey value 0.
  Type_encoding( std::string_view type ); // NOLINT
  uint32_t encoding() const;
  std::pair<std::string_view, std::string_view> decode_type() const;

  bool operator==( Type_encoding rhs ) const;
  bool operator!=( Type_encoding rhs ) const;
  bool operator<( Type_encoding rhs ) const;
  bool operator>( Type_encoding rhs ) const;
  bool operator<=( Type_encoding rhs ) const;
  bool operator>=( Type_encoding rhs ) const;

private:
  uint32_t encoding_;
  static uint8_t binsearch_bit_index( std::string_view type );
  // Any and all Type_encodings will have one global string_view of the type strings for decoding.
  static constexpr std::array<std::string_view, 18> type_encoding_table_ = {
    // lexicographicly organized table. 17th index is the first lexicographic order Bug.
    "Water",
    "Steel",
    "Rock",
    "Psychic",
    "Poison",
    "Normal",
    "Ice",
    "Ground",
    "Grass",
    "Ghost",
    "Flying",
    "Fire",
    "Fighting",
    "Fairy",
    "Electric",
    "Dragon",
    "Dark",
    "Bug" };

  // I know, internal tests are bad but these were for my own curiosity. I will try to delete internal tests.
  FRIEND_TEST( InternalTests, TestEveryPossibleCombinationOfTypings );
  FRIEND_TEST( InternalTests, CompareMyEncodingDecodingSpeed );
};

/* * * * * * * * * *      Overloaded Operator for a String View       * * * * * * * * * * * * * * */

std::ostream& operator<<( std::ostream& out, Type_encoding tp );

} // namespace Dancing_links

/* * * * * * * * * *          Type_encodings Should be Hashable          * * * * * * * * * * * * * */

namespace std {

template<>
struct hash<Dancing_links::Type_encoding>
{
  size_t operator()( Dancing_links::Type_encoding type ) const noexcept
  {
    return std::hash<uint32_t> {}( type.encoding() );
  }
};

} // namespace std

#endif // TYPEENCODING_HH
