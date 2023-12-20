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
 * ---------------------
 */
#pragma once
#include <span>
#ifndef TYPE_ENCODING_HH
#define TYPE_ENCODING_HH

#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>

namespace Dancing_links {

class Type_encoding
{

public:
  Type_encoding() = default;
  // If encoding cannot be found encoding_ is set the falsey value 0.
  Type_encoding( std::string_view type ); // NOLINT
  [[nodiscard]] uint32_t encoding() const;
  [[nodiscard]] std::pair<std::string_view, std::string_view> decode_type() const;
  [[nodiscard]] std::string to_string() const;
  [[nodiscard]] static std::span<const std::string_view> type_table();

  bool operator==( Type_encoding rhs ) const;
  std::strong_ordering operator<=>( Type_encoding rhs ) const;

private:
  uint32_t encoding_;
  static uint64_t type_bit_index( std::string_view type );
  // Any and all Type_encodings will have one global string_view of the type strings for decoding.
  static constexpr std::array<std::string_view, 18> type_encoding_table = {
    // lexicographicly organized table. 17th index is the highest lexicographic value "Water."
    "Bug",
    "Dark",
    "Dragon",
    "Electric",
    "Fairy",
    "Fighting",
    "Fire",
    "Flying",
    "Ghost",
    "Grass",
    "Ground",
    "Ice",
    "Normal",
    "Poison",
    "Psychic",
    "Rock",
    "Steel",
    "Water",
  };
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

#endif // TYPE_ENCODING_HH
