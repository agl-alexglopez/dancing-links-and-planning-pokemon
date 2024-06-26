///
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
///
/// Author: Alexander G. Lopez
/// File: Type_encoding.cpp
/// --------------------------
/// This file contains the implementation behind the Type_encoding type. Notable
/// here is the use of binary search and bit shifting to acheive fast encoding
/// and decoding. I chose to use a linear search on an array of single types,
/// combining them to create dual types if necessary, over a large map that
/// contains all possible combinations for two reasons: there are only 18 single
/// types and the memory footprint is small. With an array of 18 elements, the
/// at worst two linear searches will be competitive with the runtime of any
/// hashmap implementation. We also do not bring in any overhead of the std
/// library implementation of a hashmap or hashing function. We simply store the
/// minimum possible information, a stack array of single type strings. I think
/// this is a good balance of performance and space efficiency. We have at worst
/// two linear searches to encode a type string and at worst two bit checks to
/// decode the encoding back to a string. This is fine.
module;
#include <array>
#include <bit>
#include <compare>
#include <cstdint>
#include <optional>
#include <ostream>
#include <span>
#include <string>
#include <string_view>
#include <utility>
export module dancing_links:type_encoding;

////////////////////////////////////////   Exported Interface

export namespace Dancing_links {

class Type_encoding {

  public:
    Type_encoding() = default;
    // If encoding cannot be found encoding_ is set the falsey value 0.
    Type_encoding(std::string_view type); // NOLINT
    [[nodiscard]] uint32_t encoding() const;
    [[nodiscard]] std::pair<std::string_view, std::string_view>
    decode_type() const;
    [[nodiscard]] std::pair<uint64_t, std::optional<uint64_t>>
    decode_indices() const;
    [[nodiscard]] std::string to_string() const;
    [[nodiscard]] static std::span<const std::string_view> type_table();

    bool operator==(Type_encoding rhs) const;
    std::strong_ordering operator<=>(Type_encoding rhs) const;

  private:
    uint32_t encoding_;
    static uint64_t type_bit_index(std::string_view type);
    // Any and all Type_encodings will have one global string_view of the type
    // strings for decoding.
    static constexpr std::array<std::string_view, 18> type_encoding_table = {
        // lexicographicly organized table. 17th index is the highest
        // lexicographic value "Water."
        "Bug",    "Dark",   "Dragon",  "Electric", "Fairy",  "Fighting",
        "Fire",   "Flying", "Ghost",   "Grass",    "Ground", "Ice",
        "Normal", "Poison", "Psychic", "Rock",     "Steel",  "Water",
    };
};

///////////////////      Overloaded Operator for a String View

std::ostream &operator<<(std::ostream &out, Type_encoding tp);

} // namespace Dancing_links

////////////////////////////////////////   Implementation

/////////////////////          Type_encodings Should be Hashable

namespace std {

template <> struct hash<Dancing_links::Type_encoding>
{
    size_t
    operator()(Dancing_links::Type_encoding type) const noexcept
    {
        return std::hash<uint32_t>{}(type.encoding());
    }
};

} // namespace std

namespace Dancing_links {

Type_encoding::Type_encoding(std::string_view type) : encoding_(0)
{
    if (type.empty())
    {
        return;
    }
    const uint64_t delim = type.find('-');
    uint64_t found = type_bit_index(type.substr(0, delim));
    if (found == type_encoding_table.size())
    {
        return;
    }
    encoding_ = 1U << found;
    if (delim == std::string::npos)
    {
        return;
    }
    found = type_bit_index(type.substr(delim + 1));
    if (found == type_encoding_table.size())
    {
        encoding_ = 0;
        return;
    }
    encoding_ |= (1U << found);
}

std::pair<std::string_view, std::string_view>
Type_encoding::decode_type() const
{
    if (!encoding_)
    {
        return {};
    }
    const uint32_t width = 31;
    const uint32_t lesser_lexicographic_bit_index = std::countr_zero(encoding_);
    const uint32_t greater_lexicographic_bit_index
        = width - std::countl_zero(encoding_);
    if (lesser_lexicographic_bit_index == greater_lexicographic_bit_index)
    {
        return {type_encoding_table.at(lesser_lexicographic_bit_index), {}};
    }
    return {
        type_encoding_table.at(lesser_lexicographic_bit_index),
        type_encoding_table.at(greater_lexicographic_bit_index),
    };
}

std::pair<uint64_t, std::optional<uint64_t>>
Type_encoding::decode_indices() const
{
    if (!encoding_)
    {
        return {};
    }
    const uint64_t width = 31;
    const uint64_t lesser_lexicographic_bit_index = std::countr_zero(encoding_);
    const uint64_t greater_lexicographic_bit_index
        = width - std::countl_zero(encoding_);
    if (lesser_lexicographic_bit_index == greater_lexicographic_bit_index)
    {
        return {lesser_lexicographic_bit_index, std::optional<uint64_t>{}};
    }
    return {lesser_lexicographic_bit_index, greater_lexicographic_bit_index};
}

uint64_t
Type_encoding::type_bit_index(std::string_view type)
{
    // Linear search seems slow but actually beats binary search by a TON
    // because table is small.
    uint64_t i = 0;
    for (const auto &t : type_encoding_table)
    {
        if (t == type)
        {
            return i;
        }
        ++i;
    }
    return i;
}

std::string
Type_encoding::to_string() const
{
    const std::pair<std::string_view, std::string_view> types = decode_type();
    if (types.second.empty())
    {
        return std::string(types.first);
    }
    return std::string(types.first).append("-").append(types.second);
}

uint32_t
Type_encoding::encoding() const
{
    return encoding_;
}

std::span<const std::string_view>
Type_encoding::type_table()
{
    return type_encoding_table;
}

bool
Type_encoding::operator==(Type_encoding rhs) const
{
    return this->encoding_ == rhs.encoding_;
}

std::strong_ordering
Type_encoding::operator<=>(Type_encoding rhs) const
{
    if (this->encoding_ == rhs.encoding_)
    {
        return std::strong_ordering::equal;
    }
    const auto rightmost_bit_cmp
        = std::countr_zero(this->encoding_) <=> std::countr_zero(rhs.encoding_);
    if (rightmost_bit_cmp != std::strong_ordering::equal)
    {
        return rightmost_bit_cmp;
    }
    // A single type that tied for the low bit will be sorted correctly as well
    // as any two dual types. For example this check ensures that "Bug" comes
    // before "Bug-Dark" while also sorting any two dual types.
    const auto leftmost_bit_cmp
        = std::countl_zero(this->encoding_) <=> std::countl_zero(rhs.encoding_);
    // Not a mistake! We want the bits in a uint32_t to be sorted like strings.
    // Checking from the left means fewer zeros is closer to largest
    // lexicographic value "Water." So we need to flip this comparison.
    if (leftmost_bit_cmp == std::strong_ordering::less)
    {
        return std::strong_ordering::greater;
    }
    if (leftmost_bit_cmp == std::strong_ordering::greater)
    {
        return std::strong_ordering::less;
    }
    return leftmost_bit_cmp;
}

// This operator is useful for debugging or guis. I can make heap string methods
// when needed.
std::ostream &
operator<<(std::ostream &out, Type_encoding tp)
{
    const std::pair<std::string_view, std::string_view> to_print
        = tp.decode_type();
    out << to_print.first;
    if (!to_print.second.empty())
    {
        out << '-' << to_print.second;
    }
    return out;
}

} // namespace Dancing_links
