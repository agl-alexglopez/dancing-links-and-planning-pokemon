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
 * File: TypeEncoding.cpp
 * --------------------------
 * This file contains the implementation behind the TypeEncoding type. Notable here is the use of
 * binary search and bit shifting to acheive fast encoding and decoding. I chose to use a binary
 * search on an array of single types, combining them to create dual types if necessary, over a
 * large map that contains all possible combinations for two reasons: there are only 18 single types
 * and the memory footprint is small. With an array of 18 elements, the at worst two binary searches
 * will be competitive with the runtime of any hashmap implementation. We also do not bring in any
 * overhead of the std library implementation of a hashmap or hashing function. We simply store
 * the minimum possible information, a stack array of single type strings. I think this is a good
 * balance of performance and space efficiency. We have at worst two binary searches to encode a
 * type string and at worst 8 bit shifts to decode the encoding back to a string. This is fine.
 */
#include "TypeEncoding.h"

namespace DancingLinks {

namespace {

// Half a byte is a nibble (aka nybl, nyble, nybble, half-byte...)
const uint32_t NYBL_SHIFT = 4;
const uint32_t NYBL_MASK = 0xf;
const uint32_t TYPE_MAX = 0x30000;

size_t binsearchBitIndex(std::string_view type) {
    for (size_t remain = TYPE_TABLE_SIZE, base = 0; remain; remain >>= 1) {
        size_t index = base + (remain >> 1);
        std::string_view found = TYPE_ENCODING_TABLE[index];
        if (found == type) {
            return index;
        }
        // This should look weird! Lower lexicographic order is stored in higher order bits. (*_*).
        if (type < found) {
            base = index + 1;
            remain--;
        }
    }
    return TYPE_TABLE_SIZE;
}


} // namespace


TypeEncoding::TypeEncoding(std::string_view type)
    : encoding_(0) {
    if (type == "") {
        return;
    }
    size_t delim = type.find('-');
    size_t found = binsearchBitIndex(type.substr(0, delim));
    if (found == TYPE_TABLE_SIZE) {
        return;
    }
    encoding_ = 1 << found;
    if (delim == std::string::npos) {
        return;
    }
    found = binsearchBitIndex(type.substr(delim + 1));
    if (found == TYPE_TABLE_SIZE) {
        encoding_ = 0;
        return;
    }
    encoding_ |= (1 << found);
}

/* Shifting a nyble at a time is a good balance. The second loop will have at most 3 shifts.
 * This could cut down the shifts to get to the bit we want significantly. Consider how many
 * shifts to get the indices of the following bits with normal single shifts and then with
 * nyble shifts. Nybl shifts are the size of a nybl, 4 bits.
 *
 *      Bug-Steel = 0x10001 = 0b10000 0000 0000 0001
 *
 * With single shifts it would take 16 bit shifts to get to the second type. With nyble shifts
 * we can get that bit in 4 shifts. At worst we have three additional shifts after a nybl shift.
 */
std::pair<std::string_view,std::string_view> TypeEncoding::to_pair() const {
    if (!this->encoding_) {
        return {};
    }
    uint32_t thisCopy = this->encoding_;
    int tableIndex = 0;
    while (thisCopy) {
        if (!(thisCopy & NYBL_MASK)) {
            thisCopy >>= NYBL_SHIFT;
            tableIndex += NYBL_SHIFT;
            continue;
        }
        for (; !(thisCopy & 1); tableIndex++, thisCopy >>= 1) {
        }
        break;
    }
    std::string_view decoded = TYPE_ENCODING_TABLE[tableIndex];
    if (thisCopy == 1) {
        return {decoded, {}};
    }
    thisCopy >>= 1;
    tableIndex++;
    while (thisCopy) {
        if (!(thisCopy & NYBL_MASK)) {
            thisCopy >>= NYBL_SHIFT;
            tableIndex += NYBL_SHIFT;
            continue;
        }
        for (; !(thisCopy & 1); tableIndex++, thisCopy >>= 1) {
        }
        break;
    }
    return {TYPE_ENCODING_TABLE[tableIndex], decoded};
}


} // namespace DancingLinks
