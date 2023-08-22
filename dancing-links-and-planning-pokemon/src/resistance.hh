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
 * Author: Alexander Lopez
 * File: Resistance.h
 * ------------------
 * This file contains a utility class for defining a Pokemon Resistance. The intended use is for
 * these resistances to be associated with keys. The keys should either be attack types or defensive
 * types, so this is not a generic class.
 */
#ifndef RESISTANCE_HH
#define RESISTANCE_HH
#include "type_encoding.hh"
#include <ostream>
#include <string>

namespace Dancing_links {

enum Multiplier {
    /* It would not make sense for someone to let a multiplier in a Resistance default to
     * IMMUNE, because that is a valuable multiplier to have for a Pokemon. Make sure you
     * initialize multipliers unless you want an EMPTY_ placeholder.
     */
    emp=0,
    imm,
    f14,  // x0.25 damage aka the fraction 1/4
    f12,  // x0.5 damage aka the fraction 1/2
    nrm,
    dbl,
    qdr
};

class Resistance {
public:

    Resistance(const Type_encoding& type, const Multiplier& multiplier);

    Resistance(const Resistance& other) = default;
    Resistance(Resistance&& other) noexcept = default;
    Resistance& operator=(Resistance&& other) = default;
    Resistance& operator=(const Resistance& other) = default;
    ~Resistance() = default;

    Type_encoding type() const;

    Multiplier multiplier() const;

    bool operator< (const Resistance& rhs) const {
        return this->type() < rhs.type();
    }
    bool operator== (const Resistance& rhs) const {
        return this->type() == rhs.type() && this->multiplier() == rhs.multiplier();
    }
    bool operator> (const Resistance& rhs) const {
        return rhs < *this;
    }
    bool operator>= (const Resistance& rhs) const {
        return !(*this < rhs);
    }
    bool operator<= (const Resistance& rhs) const {
        return !(*this > rhs);
    }
    bool operator!= (const Resistance& rhs) const {
        return !(*this == rhs);
    }
private:
    Type_encoding type_;
    Multiplier multiplier_;

};


std::ostream& operator<<(std::ostream& out, const Resistance& res);
std::ostream& operator<<(std::ostream& out, const Multiplier& mult);

}// namespace Dancing_links

#endif // RESISTANCE_HH
