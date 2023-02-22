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
 * Author: Alex Lopez
 * File: PokemonParser.h
 * ---------------------
 * This file handles getting Pokemon Data from JSON files and turning it into C++ std compliant
 * data forms like maps and sets. This implementation is heavily dependent on QT libraries for
 * parsing the JSON. I chose to turn the data into C++ std forms to use with the program, even
 * though it would have been possible to just read the data from the files in directly during the
 * program runtime, in case QT becomes deprecated or I want to move this to a CLI implementation.
 * Then, I just need to rewrite the parser that gets me data from JSON files rather than rewrite
 * the whole program.
 */
#ifndef POKEMONPARSER_H
#define POKEMONPARSER_H
#include <map>
#include <set>
#include <string>
#include <vector>
#include "MapParser.h"
#include "TypeEncoding.h"
#include "TypeResistance.h"

namespace Dx = DancingLinks;

/* Leave a comment at the first line of the Pokemon Generation .dst file you want to construct with
 * the pokemon generation in base 10 numbers. This will determine the types availabe for the given
 * map and generation cover problem. Any of the 9 generations are acceptable. If the
 * first line does not match one of these strings I will just include all pokemon types in the cover
 * problem. Like this.
 *
 * # 1
 * # 2
 * # 3
 * # 4
 * # 5
 * # 6
 * # 7
 * # 8
 * # 9
 *
 * This also means you should start every file with some sort of comment labelling it with
 * identifying info, even if there is not generation specification.
 */

struct PokemonTest {
    /* This map will hold all types--dual types included--and a map of defense multipliers ranging
     * from x0.0,x0.25,x0.5,x1.0,x2.0,x4.0. In these maps will be the type as the key and
     * all the single attack types that have that multiplier against the current type. There are
     * 18 single attack types. The highest level map holds 162 type keys that exist in Pokemon as
     * of December 2022. However, depending on which generation map you decide to draw some types
     * might be missing. For example, generation one Pokemon did not have types like Fairy, Dark,
     * or Steel.
     */
    std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> interactions;
    MapTest genMap;
};

/**
 * @brief loadPokemonGeneration  builds the PokemonTest needed to interact with a generation's map
 *                               in the Pokemon Planning GUI.
 * @param source                 the file with the map that gives us info on which gen to build.
 * @return                       the completed pokemon test with map drawing and Pokemon info.
 */
PokemonTest loadPokemonGeneration(std::istream& source);

/**
 * @brief loadSelectedGymsDefenses  when interacting with the GUI, the user can choose subsets of
 *                                  gyms on the current map they are viewing. If they make these
 *                                  selections we can load in the defensive types that are present
 *                                  at those gyms. These are intended to be fed to a PokemonLinks
 *                                  solver as the items we must defend against.
 * @param selectedMap               the current .dst file we are viewing.
 * @param selectedGyms              the gyms G1-E4 that we are considering attacking.
 * @return                          the set of all defensive types present in the selection of gyms.
 */
std::set<Dx::TypeEncoding>
loadSelectedGymsDefenses(const std::string& selectedMap,
                           const std::set<std::string>& selectedGyms);

/**
 * @brief loadSelectedGymsAttacks  the user interacting with the GUI may want to defend themselves
 *                                 from only a selection of gyms. We will get the gym info for the
 *                                 selected map and return the types of attacks present across all
 *                                 of those selections. This set can then be passed to the
 *                                 PokemonLinks dancing links class as a second parameter.
 * @param selectedMap              the current map the user interacts with.
 * @param selected                 the gyms they have selected.
 * @return                         a set of all attack types present across those gyms.
 */
std::set<Dx::TypeEncoding>
loadSelectedGymsAttacks(const std::string& selectedMap,
                          const std::set<std::string>& selected);

#endif // POKEMONPARSER_H
