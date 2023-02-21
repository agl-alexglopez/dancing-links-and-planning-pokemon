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
 * File: TypeEncoding.h
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
 * We have to be slightly creative with the bit representation to ensure lexographical ordering of
 * the encoding even when it is in its unsigned integer form. The lowest string by lexographical
 * order, Bug, will actually be our highest order bit and largest value. This ensures that any dual
 * typing that contains Bug will always be a larger number than one that contains Dark, for example.
 * In the same way, any string that contains Bug, would always be sorted lexographically first than
 * one that contains Dark. Then, we simply reverse the less than operator for the TypeEncoding and
 * we can use this type as keys in sets, keys maps, or elements in binary searches and they will
 * behave as if they are strings, but all comparisons are much more efficient. This means that we
 * can be consistent when decoding the bits back to a string with the lexographical ordering I have
 * decided on for the entire project. As an example the Dragon-Flying type would be the following
 * hex and binary value.
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
#ifndef TYPEENCODING_H
#define TYPEENCODING_H
#include <string_view>
#include <ostream>
#include <vector>
#include <set>
#include "Utilities/RankedSet.h"

namespace DancingLinks {

const size_t TYPE_TABLE_SIZE = 18;
// Lexographically organized table. 17th index is the first lexographical order Bug.
const std::string TYPE_ENCODING_TABLE[TYPE_TABLE_SIZE] = {
    "Water","Steel","Rock","Psychic","Poison","Normal","Ice","Ground","Grass","Ghost","Flying",
    "Fire","Fighting","Fairy","Electric","Dragon","Dark","Bug"
};

struct TypeEncoding {
    uint32_t encoding_;
    TypeEncoding() = default;
    TypeEncoding(std::string_view type);
    std::pair<std::string_view,std::string_view> to_pair() const;
    bool operator==(TypeEncoding rhs) const {
        return this->encoding_ == rhs.encoding_;
    }
    bool operator!=(TypeEncoding rhs) const {
        return !(*this == rhs);
    }
    bool operator<(TypeEncoding rhs) const {
        return rhs.encoding_ < this->encoding_;
    }
    bool operator>(TypeEncoding rhs) const {
        return rhs < *this;
    }
    bool operator<=(TypeEncoding rhs) const {
        return !(*this > rhs);
    }
    bool operator>=(TypeEncoding rhs) const {
        return !(*this < rhs);
    }
};

std::ostream& operator<<(std::ostream& out, const TypeEncoding& tp);
std::ostream& operator<<(std::ostream& os, const std::set<RankedSet<TypeEncoding>>& solution);
std::ostream& operator<<(std::ostream& os, const std::vector<TypeEncoding>& types);
std::ostream& operator<<(std::ostream& os, const std::set<TypeEncoding>& types);

} // namespace DancingLinks

#endif // TYPEENCODING_H
