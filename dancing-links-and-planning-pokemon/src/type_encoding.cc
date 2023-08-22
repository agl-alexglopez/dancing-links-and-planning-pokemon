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
 * File: Type_encoding.cpp
 * --------------------------
 * This file contains the implementation behind the Type_encoding type. Notable here is the use of
 * binary search and bit shifting to acheive fast encoding and decoding. I chose to use a binary
 * search on an array of single types, combining them to create dual types if necessary, over a
 * large map that contains all possible combinations for two reasons: there are only 18 single types
 * and the memory footprint is small. With an array of 18 elements, the at worst two binary searches
 * will be competitive with the runtime of any hashmap implementation. We also do not bring in any
 * overhead of the std library implementation of a hashmap or hashing function. We simply store
 * the minimum possible information, a stack array of single type strings. I think this is a good
 * balance of performance and space efficiency. We have at worst two binary searches to encode a
 * type string and at worst 16 bit shifts to decode the encoding back to a string. This is fine.
 */
#include "type_encoding.hh"

namespace Dancing_links {

Type_encoding::Type_encoding( std::string_view type ) : encoding_( 0 )
{
  if ( type.empty() ) {
    return;
  }
  const size_t delim = type.find( '-' );
  uint8_t found = binsearch_bit_index( type.substr( 0, delim ) );
  if ( found == type_encoding_table_.size() ) {
    return;
  }
  encoding_ = 1 << found;
  if ( delim == std::string::npos ) {
    return;
  }
  found = binsearch_bit_index( type.substr( delim + 1 ) );
  if ( found == type_encoding_table_.size() ) {
    encoding_ = 0;
    return;
  }
  encoding_ |= ( 1U << found );
}

/* As the worst case, it takes a few condition checks and 16 bit shifts to fully decode this type.
 *
 *       |----------------------1
 *       |    |-------------------------------------1
 *      Bug-Water = 0x20001 = 0b10000 0000 0000 00001
 */
std::pair<std::string_view, std::string_view> Type_encoding::decode_type() const
{
  if ( !encoding_ ) {
    return {};
  }
  uint32_t shift_copy = encoding_;
  uint8_t table_index = 0;
  while ( !( shift_copy & 1U ) ) {
    shift_copy >>= 1U;
    table_index++;
  }

  const std::string_view first_found = type_encoding_table_.at( table_index );
  if ( shift_copy == 1 ) {
    return { first_found, {} };
  }

  do {
    table_index++;
    shift_copy >>= 1U;
  } while ( !( shift_copy & 1U ) );

  return { type_encoding_table_.at( table_index ), first_found };
}

uint8_t Type_encoding::binsearch_bit_index( std::string_view type )
{
  for ( uint8_t remain = type_encoding_table_.size(), base = 0; remain; remain >>= 1 ) {
    const uint8_t index = base + ( remain >> 1 );
    const std::string_view found = type_encoding_table_.at( index );
    if ( found == type ) {
      return index;
    }
    // This should look weird! Lower lexicographic order is stored in higher order bits!
    if ( type < found ) {
      base = index + 1;
      remain--;
    }
  }
  return type_encoding_table_.size();
}

uint32_t Type_encoding::encoding() const
{
  return encoding_;
}

bool Type_encoding::operator==( Type_encoding rhs ) const
{
  return this->encoding_ == rhs.encoding_;
}

bool Type_encoding::operator!=( Type_encoding rhs ) const
{
  return !( *this == rhs );
}

// Not a mistake! We want the bits in a uint32_t to be sorted like strings. See file header.
bool Type_encoding::operator<( Type_encoding rhs ) const
{
  return this->encoding_ > rhs.encoding_;
}

bool Type_encoding::operator>( Type_encoding rhs ) const
{
  return rhs < *this;
}

bool Type_encoding::operator<=( Type_encoding rhs ) const
{
  return !( *this > rhs );
}

bool Type_encoding::operator>=( Type_encoding rhs ) const
{
  return !( *this < rhs );
}

// This operator is useful for the GUI application. I can make heap string methods when needed.
std::ostream& operator<<( std::ostream& out, Type_encoding tp )
{
  const std::pair<std::string_view, std::string_view> to_print = tp.decode_type();
  out << to_print.first;
  if ( !to_print.second.empty() ) {
    out << '-' << to_print.second;
  }
  return out;
}

} // namespace Dancing_links
