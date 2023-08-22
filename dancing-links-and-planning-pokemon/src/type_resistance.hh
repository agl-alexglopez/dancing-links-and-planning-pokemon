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
 * File: TypeResistance.h
 * ------------------
 * This file contains an interface for using a Pokemon Resistance. The intended use is for
 * these resistances to be associated with keys. The keys should either be attack types or defensive
 * types, so this is not a generic class. For example, you could have a defensive type in a map
 * like this:
 *
 *      {"Ground-Rock":{"Electric x0.0","Water x4","Fire x0.5"}},
 *
 * or the flipped version of the same setup for attack types like this:
 *
 *      {"Fire":{"Ground-Rock x0.5","Grass-Bug x4"}}.
 *
 * You can see that multiplier remains constant whether we have the defensive type as a key or as
 * a Resistance in the map. That is why I use this to help make answering questions of exact or
 * overlapping cover for both Attack and Defense Pokemon types faster and more convenient. A
 * resistance should always be tied to a key that it interacts with. Maybe there is a better way,
 * but this was a convenient way I found to deal with the added complexity of different multipliers
 * in the Pokemon Type Coverage Problem. Note that the keys are TypeEncodings, an effficient
 * representation of our string type. It can be easily converted to a pair conaining a string_view
 * of the single or dual type if needed. See the TypeEncoding header file for more info.
 */
#ifndef TYPE_RESISTANCE_HH
#define TYPE_RESISTANCE_HH
#include "resistance.hh"
#include "type_encoding.hh"

namespace Dancing_links {
class Resistance;

/**
 * @brief type  returns the TypeEncoding for the resistance. May be viewed as a pair of strings.
 * @param res   the Resistance instance.
 * @return      the TypeEncoding, an efficient representation of a type. May be printed as a string.
 */
Type_encoding type( const Resistance& res );

/**
 * @brief multiplier  returns the multiplier for a Resistance object.
 * @param res         the instance of a Resistance.
 * @return            EMPTY_,IMMUNE(x0),FRAC14(x.25),FRAC12(x.5),NORMAL(x1),DOUBLE(x2) or QUADRU(x4)
 */
Multiplier multiplier( const Resistance& res );

} // namespace Dancing_links

#endif // TYPE_RESISTANCE_H
