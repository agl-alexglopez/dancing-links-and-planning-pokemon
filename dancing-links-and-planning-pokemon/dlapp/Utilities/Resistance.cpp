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
#include "Resistance.h"


/* * * * * * * * * * * * *     Resistance Helper Class      * * * * * * * * * * * * * * * * * * * */

Resistance::Resistance(const std::string& type, const Multiplier& multiplier) :
                       type_(type),
                       multiplier_(multiplier){}

std::string Resistance::type() const {
    return type_;
}

Resistance::Multiplier Resistance::multiplier() const {
    return multiplier_;
}

std::ostream& operator<<(std::ostream& out, const Resistance& res) {
    out << res.type() << " x";
    switch(res.multiplier()) {
        case Resistance::EMPTY_:
            out << "NIL";
        break;
        case Resistance::IMMUNE:
            out << "0.0";
        break;
        case Resistance::FRAC14:
            out << "0.25";
        break;
        case Resistance::FRAC12:
            out << "0.5";
        break;
        case Resistance::NORMAL:
            out << "1.0";
        break;
        case Resistance::DOUBLE:
            out << "2.0";
        break;
        case Resistance::QUADRU:
            out << "4.0";
        break;
    }
    out << std::endl;
    return out;
}

std::ostream& operator<<(std::ostream& out, const Resistance::Multiplier& mult) {
    out << "Resistance::";
    switch(mult) {
        case Resistance::EMPTY_:
            out << "EMPTY_";
        break;
        case Resistance::IMMUNE:
            out << "IMMUNE";
        break;
        case Resistance::FRAC14:
            out << "FRAC14";
        break;
        case Resistance::FRAC12:
            out << "FRAC12";
        break;
        case Resistance::NORMAL:
            out << "NORMAL";
        break;
        case Resistance::DOUBLE:
            out << "DOUBLE";
        break;
        case Resistance::QUADRU:
            out << "QUADRU";
        break;
    }
    return out;
}
