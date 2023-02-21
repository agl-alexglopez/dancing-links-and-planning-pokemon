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
 * File: Tests.cpp
 * ------------------
 * Testing the Dancing Links data structures internally is easy becuase they are all vectors. I have
 * found it much easier to develop the algorithm quickly with internal testing rather than just
 * plain unit testing. You can learn a lot about how Dancing Links works by reading these tests.
 */
#include "GUI/SimpleTest.h"
#include "TypeEncoding.h"
#include "PokemonLinks.h"

/* * * * * * * *  Test the Type Encoding We Use To Represent Pokemon Types in Bits  * * * * * * * */

namespace Dx = DancingLinks;

STUDENT_TEST("Easiest type encoding lexographically is Bug.") {
    uint32_t hexType = 0x20000;
    std::string_view strType = "Bug";
    Dx::TypeEncoding code(strType);
    EXPECT_EQUAL(hexType, code.encoding_);
    EXPECT_EQUAL(strType, code.to_pair().first);
    std::string_view empty{};
    EXPECT_EQUAL(empty, code.to_pair().second);
}

STUDENT_TEST("Test the next simplist dual type Bug-Dark") {
    uint32_t hexType = 0x30000;
    std::string_view strType = "Bug-Dark";
    Dx::TypeEncoding code(strType);
    EXPECT_EQUAL(hexType, code.encoding_);
    EXPECT_EQUAL(std::string_view("Bug"), code.to_pair().first);
    EXPECT_EQUAL(std::string_view("Dark"), code.to_pair().second);
}

STUDENT_TEST("Test for off by one errors with first and last index type Bug-Water.") {
    uint32_t hexType = 0x20001;
    std::string_view strType = "Bug-Water";
    Dx::TypeEncoding code(strType);
    EXPECT_EQUAL(hexType, code.encoding_);
    EXPECT_EQUAL(std::string_view("Bug"), code.to_pair().first);
    EXPECT_EQUAL(std::string_view("Water"), code.to_pair().second);
}

STUDENT_TEST("Test every possible combination of typings.") {
    /* Not all of these type combinations exist yet in Pokemon and that's ok. It sounds like there
     * would be alot but it only comes out to 171 unique combinations. Unique here means that
     * types order does not matter so Water-Bug is the same as Bug-Water and is only counted once.
     */
    const uint32_t BUG = 0x20000;
    uint32_t tableSize = Dx::TYPE_TABLE_SIZE;
    for (uint32_t bit1 = BUG, type1 = tableSize - 1; bit1 != 0; bit1 >>= 1, type1--) {

        std::string checkSingleType(Dx::TYPE_ENCODING_TABLE[type1]);
        Dx::TypeEncoding singleTypeEncoding(checkSingleType);
        EXPECT_EQUAL(singleTypeEncoding.encoding_, bit1);
        EXPECT_EQUAL(singleTypeEncoding.to_pair().first, checkSingleType);
        EXPECT_EQUAL(singleTypeEncoding.to_pair().second, {});

        for (uint32_t bit2 = bit1 >> 1, type2 = type1 - 1; bit2 != 0; bit2 >>= 1, type2--) {

            std::string checkDualType = checkSingleType + "-" + Dx::TYPE_ENCODING_TABLE[type2];
            Dx::TypeEncoding dualTypeEncoding(checkDualType);
            EXPECT_EQUAL(dualTypeEncoding.encoding_, bit1 | bit2);
            /* I discourage the use of methods that create heap strings whenever possible. I use
             * string_views internally to report back typing for human readability when requested
             * in a GUI via ostream. This way I only need to create one character "-" on the heap to
             * join the two string_views of the lookup table in the stream. I don't have a use case
             * for creating strings yet but will add it if needed.
             */
            std::ostringstream captureType;
            captureType << dualTypeEncoding;
            std::string dualTypeString = captureType.str();
            EXPECT_EQUAL(dualTypeString, checkDualType);
        }
    }
}


/* * * * * * * * * * * * * * *      Tests Below This Point      * * * * * * * * * * * * * * * * * */


/* These type names completely crowd out our test cases when I construct the dlx grids in the test
 * making them hard to read. They stretch too far horizontally so I am creating these name codes
 * here that should only be used in this translation unit for readablity and convenience when
 * building tests. Refer here if terminology in the tests is confusing. Also, hovering over the
 * code in a test case in QT should show you the full constexpr for Resistances.
 */

namespace {

constexpr Dx::Multiplier EM = Dx::EMPTY_;
constexpr Dx::Multiplier IM = Dx::IMMUNE;
constexpr Dx::Multiplier F4 = Dx::FRAC14;
constexpr Dx::Multiplier F2 = Dx::FRAC12;
constexpr Dx::Multiplier NM = Dx::NORMAL;
constexpr Dx::Multiplier DB = Dx::DOUBLE;
constexpr Dx::Multiplier QD = Dx::QUADRU;

} // namespace


/* * * * * * * * * * * * * * * * * *   Defense Links Init   * * * * * * * * * * * * * * * * * * * */


STUDENT_TEST("Initialize small defensive links") {
    /*
     *
     *          Fire   Normal    Water   <-Attack
     *  Ghost          x0.0              <-Defense
     *  Water   x0.5             x0.5
     *
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types {
        {{"Ghost"}, {{{"Fire"},NM},{{"Normal"},IM},{{"Water"},NM}}},
        {{"Water"}, {{{"Fire"},F2},{{"Normal"},NM},{{"Water"},F2}}},
    };

    std::vector<Dx::PokemonLinks::encodingAndNum> optionTable = {
        {Dx::TypeEncoding(""),0},
        {{"Ghost"},4},
        {{"Water"},6}
    };
    std::vector<Dx::PokemonLinks::typeName> itemTable = {
        {{""},3,1},
        {{"Fire"},0,2},
        {{"Normal"},1,3},
        {{"Water"},2,0},
    };
    std::vector<Dx::PokemonLinks::pokeLink> dlx = {
        //   0           1Fire        2Normal      3Water
        {0,0,0,EM,0}, {1,7,7,EM,0},{1,5,5,EM,0},{1,8,8,EM,0},
        //  4Ghost                    5Zero
        {-1,0,5,EM,0},             {2,2,2,IM,0},
        //  6Water       7Half                     8Half
        {-2,5,8,EM,0},{1,1,1,F2,0},             {3,3,3,F2,0},
        //     9
        {INT_MIN,7,INT_MIN,EM,0} ,
    };
    Dx::PokemonLinks links(types, Dx::PokemonLinks::DEFENSE);
    EXPECT_EQUAL(optionTable, links.optionTable_);
    EXPECT_EQUAL(itemTable, links.itemTable_);
    EXPECT_EQUAL(dlx, links.links_);
}

STUDENT_TEST("Initialize a world where there are only single types.") {
    /*
     *
     *            Electric  Fire  Grass  Ice   Normal  Water
     *  Dragon     x0.5     x0.5  x0.5                 x0.5
     *  Electric   x0.5
     *  Ghost                                  x0.0
     *  Ice                              x0.5
     *
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types {
        {{"Dragon"}, {{{"Normal"},NM},{{"Fire"},F2},{{"Water"},F2},{{"Electric"},F2},{{"Grass"},F2},{{"Ice"},DB}}},
        {{"Electric"}, {{{"Normal"},NM},{{"Fire"},NM},{{"Water"},NM},{{"Electric"},F2},{{"Grass"},NM},{{"Ice"},NM}}},
        {{"Ghost"}, {{{"Normal"},IM},{{"Fire"},NM},{{"Water"},NM},{{"Electric"},NM},{{"Grass"},NM},{{"Ice"},NM}}},
        {{"Ice"}, {{{"Normal"},NM},{{"Fire"},NM},{{"Water"},NM},{{"Electric"},NM},{{"Grass"},NM},{{"Ice"},F2}}},
    };

    std::vector<Dx::PokemonLinks::encodingAndNum> optionTable = {
        {{""},0},
        {{"Dragon"},7},
        {{"Electric"},12},
        {{"Ghost"},14},
        {{"Ice"},16}
    };
    std::vector<Dx::PokemonLinks::typeName> itemTable = {
        {{""},6,1},
        {{"Electric"},0,2},
        {{"Fire"},1,3},
        {{"Grass"},2,4},
        {{"Ice"},3,5},
        {{"Normal"},4,6},
        {{"Water"},5,0},
    };
    std::vector<Dx::PokemonLinks::pokeLink> dlx = {
        //  0             1Electric       2Fire       3Grass          4Ice           5Normal        6Water
        {0,0,0,EM,0},   {2,13,8,EM,0}, {1,9,9,EM,0},{1,10,10,EM,0},{1,17,17,EM,0},{1,15,15,EM,0},{1,11,11,EM,0},
        //  7Dragon        8half          9half       10half                                        11half
        {-1,0,11,EM,0}, {1,1,13,F2,0}, {2,2,2,F2,0},{3,3,3,F2,0},                                {6,6,6,F2,0},
        //  12Electric     13half
        {-2,8,13,EM,0}, {1,8,1,F2,0},
        //  14Ghost                                                                  15immune
        {-3,13,15,EM,0},                                                          {5,5,5,IM,0},
        //  16Ice                                                     17half
        {-4,15,17,EM,0},                                            {4,4,4,F2,0},
        //  18
        {INT_MIN,17,INT_MIN,EM,0},
    };
    Dx::PokemonLinks links(types, Dx::PokemonLinks::DEFENSE);
    EXPECT_EQUAL(optionTable, links.optionTable_);
    EXPECT_EQUAL(itemTable, links.itemTable_);
    EXPECT_EQUAL(dlx, links.links_);
}


/* * * * * * * * * * * * * * * *   Defense Links Cover/Uncover      * * * * * * * * * * * * * * * */


STUDENT_TEST("Cover Electric with Dragon eliminates Electric Option. Uncover resets.") {
    /*
     *
     *            Electric  Fire  Grass  Ice   Normal  Water
     *  Dragon     x0.5     x0.5  x0.5                 x0.5
     *  Electric   x0.5
     *  Ghost                                  x0.0
     *  Ice                              x0.5
     *
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types {
        {{"Dragon"}, {{{"Normal"},NM},{{"Fire"},F2},{{"Water"},F2},{{"Electric"},F2},{{"Grass"},F2},{{"Ice"},DB}}},
        {{"Electric"}, {{{"Normal"},NM},{{"Fire"},NM},{{"Water"},NM},{{"Electric"},F2},{{"Grass"},NM},{{"Ice"},NM}}},
        {{"Ghost"}, {{{"Normal"},IM},{{"Fire"},NM},{{"Water"},NM},{{"Electric"},NM},{{"Grass"},NM},{{"Ice"},NM}}},
        {{"Ice"}, {{{"Normal"},NM},{{"Fire"},NM},{{"Water"},NM},{{"Electric"},NM},{{"Grass"},NM},{{"Ice"},F2}}},
    };

    std::vector<Dx::PokemonLinks::encodingAndNum> optionTable = {
        {{""},0},
        {{"Dragon"},7},
        {{"Electric"},12},
        {{"Ghost"},14},
        {{"Ice"},16}
    };
    std::vector<Dx::PokemonLinks::typeName> itemTable = {
        {{""},6,1},
        {{"Electric"},0,2},
        {{"Fire"},1,3},
        {{"Grass"},2,4},
        {{"Ice"},3,5},
        {{"Normal"},4,6},
        {{"Water"},5,0},
    };
    std::vector<Dx::PokemonLinks::pokeLink> dlx = {
        //   0            1Electric       2Fire       3Grass          4Ice         5Normal         6Water
        {0,0,0,EM,0},   {2,13,8,EM,0},{1,9,9,EM,0},{1,10,10,EM,0},{1,17,17,EM,0},{1,15,15,EM,0},{1,11,11,EM,0},
        //   7Dragon       8half          9half       10half                                       11half
        {-1,0,11,EM,0}, {1,1,13,F2,0},{2,2,2,F2,0},{3,3,3,F2,0},                                {6,6,6,F2,0},
        //   12Electric    13half
        {-2,8,13,EM,0}, {1,8,1,F2,0},
        //   14Ghost                                                               15immune
        {-3,13,15,EM,0},                                                         {5,5,5,IM,0},
        //   16Ice                                                   17half
        {-4,15,17,EM,0},                                          {4,4,4,F2,0},
        //   18
        {INT_MIN,17,INT_MIN,EM,0},
    };
    Dx::PokemonLinks links(types, Dx::PokemonLinks::DEFENSE);
    EXPECT_EQUAL(optionTable, links.optionTable_);
    EXPECT_EQUAL(itemTable, links.itemTable_);
    EXPECT_EQUAL(dlx, links.links_);

    std::vector<Dx::PokemonLinks::typeName> itemCoverElectric = {
        {{""},5,4},
        {{"Electric"},0,2},
        {{"Fire"},0,3},
        {{"Grass"},0,4},
        {{"Ice"},0,5},
        {{"Normal"},4,0},
        {{"Water"},5,0},
    };
    /*
     *             Ice   Normal
     *  Ghost             x0.0
     *  Ice        x0.5
     *
     */
    std::vector<Dx::PokemonLinks::pokeLink> dlxCoverElectric = {
        //  0             1Electric     2Fire        3Grass           4Ice         5Normal         6Water
        {0,0,0,EM,0},   {2,13,8,EM,0},{1,9,9,EM,0},{1,10,10,EM,0},{1,17,17,EM,0},{1,15,15,EM,0},{1,11,11,EM,0},
        //  7Dragon       8half         9half        10half                                        11half
        {-1,0,11,EM,0}, {1,1,13,F2,0},{2,2,2,F2,0},{3,3,3,F2,0},                                {6,6,6,F2,0},
        //  12Electric    13half
        {-2,8,13,EM,0}, {1,8,1,F2,0},
        //  14Ghost                                                                15immune
        {-3,13,15,EM,0},                                                         {5,5,5,IM,0},
        //  16Ice                                                   17half
        {-4,15,17,EM,0},                                          {4,4,4,F2,0},
        //  18
        {INT_MIN,17,INT_MIN,EM,0},
    };

    Dx::PokemonLinks::encodingAndNum pick = links.coverType(8);
    EXPECT_EQUAL(pick.num,12);
    EXPECT_EQUAL(pick.name.to_pair().first,"Dragon");
    EXPECT_EQUAL(itemCoverElectric, links.itemTable_);
    EXPECT_EQUAL(dlxCoverElectric, links.links_);

    links.uncoverType(8);
    EXPECT_EQUAL(itemTable, links.itemTable_);
    EXPECT_EQUAL(dlx, links.links_);
}

STUDENT_TEST("Cover Electric with Electric to cause hiding of many options.") {
    /*
     * This is just nonsense type weakness information in pairs to I can test the cover logic.
     *
     *            Electric  Fire  Grass  Ice   Normal  Water
     *  Electric   x0.5     x0.5
     *  Fire       x0.5           x0.5                 x0.5
     *  Grass               x0.5                       x0.5
     *  Ice                              x0.5          x0.5
     *  Normal     x0.5                        x0.5
     *  Water              x0.5                        x0.5
     *
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types {
        {{"Electric"}, {{{"Electric"},F2},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},NM}}},
        {{"Fire"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},F2},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Grass"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Ice"}, {{{"Electric"},NM},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},F2},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Normal"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},F2},{{"Water"},NM}}},
        {{"Water"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
    };
    Dx::PokemonLinks links (types, Dx::PokemonLinks::DEFENSE);
    std::vector<Dx::PokemonLinks::typeName> headers = {
        {{""},6,1},
        {{"Electric"},0,2},
        {{"Fire"},1,3},
        {{"Grass"},2,4},
        {{"Ice"},3,5},
        {{"Normal"},4,6},
        {{"Water"},5,0},
    };
    std::vector<Dx::PokemonLinks::pokeLink> dlx = {
        // 0              1Electric      2Fire         3Grass         4Ice            5Normal       6Water
        {0,0,0,EM,0},   {3,21,8,EM,0},{3,24,9,EM,0}, {1,12,12,EM,0},{1,18,18,EM,0},{1,22,22,EM,0},{4,25,13,EM,0},
        // 7Electric      8              9
        {-1,0,9,EM,0},  {1,1,11,F2,0},{2,2,15,F2,0},
        // 10Fire         11                           12                                           13
        {-2,8,13,EM,0}, {1,8,21,F2,0},               {3,3,3,F2,0},                                {6,6,16,F2,0},
        // 14Grass                       15                                                         16
        {-3,11,16,EM,0},              {2,9,24,F2,0},                                              {6,13,19,F2,0},
        // 17Ice                                                      18                            19
        {-4,15,19,EM,0},                                            {4,4,4,F2,0},                 {6,16,25,F2,0},
        // 20Normal       21                                                          22
        {-5,18,22,EM,0},{1,11,1,F2,0},                                              {5,5,5,F2,0},
        // 23Water                       24                                                         25
        {-6,21,25,EM,0},              {2,15,2,F2,0},                                              {6,19,6,F2,0},
        {INT_MIN,24,INT_MIN,EM,0},
    };
    EXPECT_EQUAL(headers, links.itemTable_);
    EXPECT_EQUAL(dlx, links.links_);

    std::vector<Dx::PokemonLinks::typeName> headersCoverElectric = {
        {{""},6,3},
        {{"Electric"},0,2},
        {{"Fire"},0,3},
        {{"Grass"},0,4},
        {{"Ice"},3,5},
        {{"Normal"},4,6},
        {{"Water"},5,0},
    };
    std::vector<Dx::PokemonLinks::pokeLink> dlxCoverElectric = {
        /*
         *
         *        Grass   Ice    Normal  Water
         *  Ice           x0.5           x0.5
         *
         *
         */
        // 0              1Electric     2Fire           3Grass      4Ice           5Normal        6Water
        {0,0,0,EM,0},   {3,21,8,EM,0},{3,24,9,EM,0}, {0,3,3,EM,0},{1,18,18,EM,0},{0,5,5,EM,0}, {1,19,19,EM,0},
        // 7Electric      8             9
        {-1,0,9,EM,0},  {1,1,11,F2,0},{2,2,15,F2,0},
        // 10Fire         11                            12                                        13
        {-2,8,13,EM,0}, {1,8,21,F2,0},               {3,3,3,F2,0},                             {6,6,16,F2,0},
        // 14Grass                        15                                                      16
        {-3,11,16,EM,0},              {2,9,24,F2,0},                                           {6,6,19,F2,0},
        // 17Ice                                                    18                            19
        {-4,15,19,EM,0},                                          {4,4,4,F2,0},                {6,6,6,F2,0},
        // 20Normal       21                                                         22
        {-5,18,22,EM,0},{1,11,1,F2,0},                                           {5,5,5,F2,0},
        // 23Water                        24                                                      25
        {-6,21,25,EM,0},              {2,15,2,F2,0},                                           {6,19,6,F2,0},
        {INT_MIN,24,INT_MIN,EM,0},
    };

    Dx::PokemonLinks::encodingAndNum pick = links.coverType(8);
    EXPECT_EQUAL(pick.num, 6);
    EXPECT_EQUAL(pick.name.to_pair().first,"Electric");
    EXPECT_EQUAL(headersCoverElectric, links.itemTable_);
    EXPECT_EQUAL(dlxCoverElectric, links.links_);

    links.uncoverType(8);
    EXPECT_EQUAL(headers, links.itemTable_);
    EXPECT_EQUAL(dlx, links.links_);
}


/* * * * * * * * * * * * *      Solve the Defensive Cover Problem       * * * * * * * * * * * * * */


STUDENT_TEST("There are two exact covers for this typing combo.") {
    /*
     *              Electric   Grass   Ice   Normal   Water
     *   Electric    x0.5
     *   Ghost                               x0.0
     *   Ground      x0.0
     *   Ice                           x0.5
     *   Poison                x0.5
     *   Water                         x0.5           x0.5
     *
     *   Exact Defensive Type Covers. 1 is better because Ground is immune to electric.
     *      1. Ghost, Ground, Poison, Water
     *      2. Electric, Ghost, Poison, Water
     *
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types {
        {{"Electric"}, {{{"Electric"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},NM}}},
        {{"Ghost"}, {{{"Electric"},NM},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},IM},{{"Water"},NM}}},
        {{"Ground"}, {{{"Electric"},IM},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},NM}}},
        {{"Ice"}, {{{"Electric"},NM},{{"Grass"},NM},{{"Ice"},F2},{{"Normal"},NM},{{"Water"},NM}}},
        {{"Poison"}, {{{"Electric"},NM},{{"Grass"},F2},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},NM}}},
        {{"Water"}, {{{"Electric"},NM},{{"Grass"},DB},{{"Ice"},F2},{{"Normal"},NM},{{"Water"},F2}}},
    };
    Dx::PokemonLinks links(types, Dx::PokemonLinks::DEFENSE);
    std::set<RankedSet<Dx::TypeEncoding>> correct = {{11,{{"Ghost"},{"Ground"},{"Poison"},{"Water"}}},
                                                                   {13,{{"Electric"},{"Ghost"},{"Poison"},{"Water"}}}};
    EXPECT_EQUAL(links.getExactCoverages(6), correct);
}

STUDENT_TEST("There is one exact and a few overlapping covers here. Exact cover first.") {
    /*
     *                     Electric    Fire    Grass    Ice    Normal    Water
     *
     *   Bug-Ghost                              x.5             x0
     *
     *   Electric-Grass     x.25                x.5                       x.5
     *
     *   Fire-Flying                   x.5      x.25
     *
     *   Ground-Water       x0         x.5
     *
     *   Ice-Psychic                                    x.5
     *
     *   Ice-Water                                      x.25              x.5
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types = {
        /* In reality maps will have every type present in every key. But I know the internals
         * of my implementation and will just enter all types for the first key to make entering
         * the rest of the test cases easier.
         */
        {{"Bug-Ghost"},{{{"Electric"},NM},{{"Fire"},NM},{{"Grass"},F2},{{"Ice"},NM},{{"Normal"},IM},{{"Water"},NM}}},
        {{"Electric-Grass"},{{{"Electric"},F4},{{"Grass"},F2},{{"Water"},F2}}},
        {{"Fire-Flying"},{{{"Fire"},F2},{{"Grass"},F4}}},
        {{"Ground-Water"},{{{"Electric"},IM},{{"Fire"},F2}}},
        {{"Ice-Psychic"},{{{"Ice"},F2}}},
        {{"Ice-Water"},{{{"Ice"},F4},{{"Water"},F2}}},
    };
    const std::vector<Dx::PokemonLinks::encodingAndNum> options = {
        {{""},0},
        {{"Bug-Ghost"},7},
        {{"Electric-Grass"},10},
        {{"Fire-Flying"},14},
        {{"Ground-Water"},17},
        {{"Ice-Psychic"},20},
        {{"Ice-Water"},22},
    };
    std::vector<Dx::PokemonLinks::typeName> items = {
        {{""},6,1},
        {{"Electric"},0,2},
        {{"Fire"},1,3},
        {{"Grass"},2,4},
        {{"Ice"},3,5},
        {{"Normal"},4,6},
        {{"Water"},5,0},
    };
    const std::vector<Dx::PokemonLinks::pokeLink> dlx = {
        // 0                 1Electric       2Fire          3Grass         4Ice          5Normal       6Water
        {0,0,0,EM,0},      {2,18,11,EM,0}, {2,19,15,EM,0},{3,16,8,EM,0},{2,23,21,EM,0},{1,9,9,EM,0},{2,24,13,EM,0},
        //7Bug-Ghost                                         8                            9
        {-1,0,9,EM,0},                                    {3,3,12,F2,0},               {5,5,5,IM,0},
        //10Electric-Grass     11                            12                                         13
        {-2,8,13,EM,0},    {1,1,18,F4,0},                 {3,8,16,F2,0},                            {6,6,24,F2,0},
        //14Fire-Flying                       15             16
        {-3,11,16,EM,0},                   {2,2,19,F2,0}, {3,12,3,F4,0},
        //17Ground-Water       18             19
        {-4,15,19,EM,0},   {1,11,1,IM,0},  {2,15,2,F2,0},
        //20Ice-Psychic                                                     21
        {-5,18,21,EM,0},                                                 {4,4,23,F2,0},
        //22Ice-Water                                                       23                          24
        {-6,21,24,EM,0},                                                 {4,21,4,F4,0},             {6,13,6,F2,0},
        //25
        {INT_MIN,23,INT_MIN,EM,0},
    };
    Dx::PokemonLinks links(types, Dx::PokemonLinks::DEFENSE);
    EXPECT_EQUAL(links.optionTable_, options);
    EXPECT_EQUAL(links.itemTable_, items);
    EXPECT_EQUAL(links.links_, dlx);
    std::set<RankedSet<Dx::TypeEncoding>> result = links.getExactCoverages(6);
    std::set<RankedSet<Dx::TypeEncoding>> correct = {{13,{{"Bug-Ghost"},
                                                                        {"Ground-Water"},
                                                                        {"Ice-Water"},}}};
    EXPECT_EQUAL(correct, result);
}


/* * * * * * * * * * * * * * * * * *   Attack Links Init    * * * * * * * * * * * * * * * * * * * */

/* The good news about this section is that we only have to test that we can correctly initialize
 * the network by inverting the attack types and defense types. Then, the algorithm runs
 * identically and we can use the same functions for this problem.
 */

STUDENT_TEST("Initialization of ATTACK dancing links.") {
    /*
     *
     *                    Fire-Flying   Ground-Grass   Ground-Rock   <-Defense
     *         Electric       2X
     *         Fire                         2x
     *         Water          2x                         4x          <-Attack
     *
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types {
        {{"Ground-Rock"}, {{{"Electric"},IM},{{"Fire"},NM},{{"Water"},QD}}},
        {{"Ground-Grass"}, {{{"Electric"},IM},{{"Fire"},DB},{{"Water"},NM}}},
        {{"Fire-Flying"}, {{{"Electric"},DB},{{"Fire"},F2},{{"Water"},DB}}},
    };
    const std::vector<Dx::PokemonLinks::encodingAndNum> optionTable = {
        {{""},0},
        {{"Electric"},4},
        {{"Fire"},6},
        {{"Water"},8}
    };
    const std::vector<Dx::PokemonLinks::typeName> itemTable = {
        {{""},3,1},
        {{"Fire-Flying"},0,2},
        {{"Ground-Grass"},1,3},
        {{"Ground-Rock"},2,0},
    };
    const std::vector<Dx::PokemonLinks::pokeLink> dlx {
        // 0           1Fire-Flying   2Ground-Grass   3Ground-Rock
        {0,0,0,EM,0},  {2,9,5,EM,0},  {1,7,7,EM,0},  {1,10,10,EM,0},
        // 4Electric     5Double
        {-1,0,5,EM,0}, {1,1,9,DB,0},
        // 6Fire                        7Double
        {-2,5,7,EM,0},                {2,2,2,DB,0},
        // 8Water        9Double                       10Quadru
        {-3,7,10,EM,0},{1,5,1,DB,0},                 {3,3,3,QD,0},
        {INT_MIN,9,INT_MIN,EM,0},
    };
    Dx::PokemonLinks links(types, Dx::PokemonLinks::ATTACK);
    EXPECT_EQUAL(links.optionTable_, optionTable);
    EXPECT_EQUAL(links.itemTable_, itemTable);
    EXPECT_EQUAL(links.links_, dlx);
}

STUDENT_TEST("At least test that we can recognize a successful attack coverage") {
    /*
     *
     *               Normal   Fire   Water   Electric   Grass   Ice     <- Defensive Types
     *    Fighting     x2                                        x2
     *    Grass                       x2                                <- Attack Types
     *    Ground               x2               x2
     *    Ice                                            x2
     *    Poison                                         x2
     *
     * There are two attack coverage schemes:
     *      Fighting, Grass, Ground, Ice
     *      Fighting, Grass, Ground, Poison
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types = {
        {{"Electric"}, {{{"Ground"},DB}}},
        {{"Fire"}, {{{"Ground"},DB}}},
        {{"Grass"}, {{{"Ice"},DB},{{"Poison"},DB}}},
        {{"Ice"}, {{{"Fighting"},DB}}},
        {{"Normal"}, {{{"Fighting"},DB}}},
        {{"Water"}, {{{"Grass"},DB}}},
    };
    std::set<RankedSet<Dx::TypeEncoding>> solutions = {{30, {{"Fighting"},{"Grass"},{"Ground"},{"Ice"}}},
                                                                     {30,{{"Fighting"},{"Grass"},{"Ground"},{"Poison"}}}};
    Dx::PokemonLinks links(types, Dx::PokemonLinks::ATTACK);
    EXPECT_EQUAL(links.getExactCoverages(24), solutions);
}

STUDENT_TEST("There is one exact and a few overlapping covers here. Exact cover first.") {
    /*
     *            Bug-Ghost   Electric-Grass   Fire-Flying   Ground-Water   Ice-Psychic   Ice-Water
     *
     * Electric                                    x2                                        x2
     *
     * Fire          x2               x2                                       x2
     *
     * Grass                                                      x4                         x2
     *
     * Ice                            x2
     *
     * Normal
     *
     * Water                                       x2
     *
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types = {
        {{"Bug-Ghost"},{{{"Electric"},NM},{{"Fire"},DB},{{"Grass"},F2},{{"Ice"},NM},{{"Normal"},IM},{{"Water"},NM}}},
        {{"Electric-Grass"},{{{"Electric"},F4},{{"Fire"},DB},{{"Grass"},F2},{{"Ice"},F2},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Fire-Flying"},{{{"Electric"},DB},{{"Fire"},F2},{{"Grass"},F4},{{"Ice"},F2},{{"Normal"},NM},{{"Water"},DB}}},
        {{"Ground-Water"},{{{"Electric"},IM},{{"Fire"},F2},{{"Grass"},QD},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},NM}}},
        {{"Ice-Psychic"},{{{"Electric"},NM},{{"Fire"},DB},{{"Grass"},NM},{{"Ice"},F2},{{"Normal"},NM},{{"Water"},NM}}},
        {{"Ice-Water"},{{{"Electric"},DB},{{"Fire"},NM},{{"Grass"},DB},{{"Ice"},F2},{{"Normal"},NM},{{"Water"},F2}}},
    };
    Dx::PokemonLinks links(types, Dx::PokemonLinks::ATTACK);
    std::set<RankedSet<Dx::TypeEncoding>> result = links.getExactCoverages(24);
    std::set<RankedSet<Dx::TypeEncoding>> correct = {
        {31,{{"Fire"},{"Grass"},{"Water"},}}
    };
    EXPECT_EQUAL(result, correct);
}


/* * * * * * * * * * *    Finding a Weak Coverage that Allows Overlap     * * * * * * * * * * * * */


STUDENT_TEST("Test the depth tag approach to overlapping coverage.") {
    /*
     * This is just nonsense type weakness information in pairs to I can test the cover logic.
     *
     *            Electric  Fire  Grass  Ice   Normal  Water
     *  Electric   x0.5     x0.5
     *  Fire       x0.5           x0.5                 x0.5
     *  Grass               x0.5                       x0.5
     *  Ice                              x0.5          x0.5
     *  Normal     x0.5                        x0.5
     *  Water              x0.5                        x0.5
     *
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types {
        {{"Electric"}, {{{"Electric"},F2},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},NM}}},
        {{"Fire"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},F2},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Grass"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Ice"}, {{{"Electric"},NM},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},F2},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Normal"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},F2},{{"Water"},NM}}},
        {{"Water"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
    };

    const std::vector<Dx::PokemonLinks::typeName> headers {
        {{""},6,1},
        {{"Electric"},0,2},
        {{"Fire"},1,3},
        {{"Grass"},2,4},
        {{"Ice"},3,5},
        {{"Normal"},4,6},
        {{"Water"},5,0},
    };
    const std::vector<Dx::PokemonLinks::pokeLink> dlx = {
        //  0            1Electric      2Fire           3Grass        4Ice           5Normal        6Water
        {0,0,0,EM,0},   {3,21,8,EM,0},{3,24,9,EM,0}, {1,12,12,EM,0},{1,18,18,EM,0},{1,22,22,EM,0},{4,25,13,EM,0},
        // 7Electric      8             9
        {-1,0,9,EM,0},  {1,1,11,F2,0},{2,2,15,F2,0},
        // 10Fire         11                            12                                          13
        {-2,8,13,EM,0}, {1,8,21,F2,0},               {3,3,3,F2,0},                                {6,6,16,F2,0},
        // 14Grass                      15                                                          16
        {-3,11,16,EM,0},              {2,9,24,F2,0},                                              {6,13,19,F2,0},
        // 17Ice                                                      18                            19
        {-4,15,19,EM,0},                                            {4,4,4,F2,0},                 {6,16,25,F2,0},
        // 20Normal       21                                                         22
        {-5,18,22,EM,0},{1,11,1,F2,0},                                             {5,5,5,F2,0},
        // 23Water                      24                                                          25
        {-6,21,25,EM,0},              {2,15,2,F2,0},                                              {6,19,6,F2,0},
        //       26
        {INT_MIN,24,INT_MIN,EM,0},
    };
    Dx::PokemonLinks links (types, Dx::PokemonLinks::DEFENSE);
    EXPECT_EQUAL(links.links_, dlx);
    EXPECT_EQUAL(links.itemTable_, headers);

    Dx::PokemonLinks::encodingAndNum choice = links.overlappingCoverType(8, 6);
    EXPECT_EQUAL(choice.num, 6);
    EXPECT_EQUAL(choice.name.to_pair().first, "Electric");
    const std::vector<Dx::PokemonLinks::typeName> headersCoverElectric {
        {{""},6,3},
        {{"Electric"},0,2},
        {{"Fire"},0,3},
        {{"Grass"},0,4},
        {{"Ice"},3,5},
        {{"Normal"},4,6},
        {{"Water"},5,0},
    };
    const std::vector<Dx::PokemonLinks::pokeLink> dlxCoverElectric = {
        /*
         *
         *            Grass   Ice   Normal  Water
         *  Fire       x0.5                 x0.5
         *  Grass                           x0.5
         *  Ice               x0.5          x0.5
         *  Normal                  x0.5
         *  Water                           x0.5
         *
         */
        // 0              1Electric      2Fire        3Grass         4Ice            5Normal        6Water
        {0,0,0,EM,0},   {3,21,8,EM,6},{3,24,9,EM,6},{1,12,12,EM,0},{1,18,18,EM,0},{1,22,22,EM,0},{4,25,13,EM,0},
        // 7Electric      8              9
        {-1,0,9,EM,0},  {1,1,11,F2,6},{2,2,15,F2,6},
        // 10Fire         11                          12                                            13
        {-2,8,13,EM,0}, {1,8,21,F2,0},              {3,3,3,F2,0},                                 {6,6,16,F2,0},
        // 14Grass                       15                                                         16
        {-3,11,16,EM,0},              {2,9,24,F2,0},                                              {6,13,19,F2,0},
        // 17Ice                                                     18                             19
        {-4,15,19,EM,0},                                           {4,4,4,F2,0},                  {6,16,25,F2,0},
        // 20Normal       21                                                         22
        {-5,18,22,EM,0},{1,11,1,F2,0},                                            {5,5,5,F2,0},
        // 23Water                       24                                                         25
        {-6,21,25,EM,0},              {2,15,2,F2,0},                                              {6,19,6,F2,0},
        // 26
        {INT_MIN,24,INT_MIN,EM,0},
    };
    EXPECT_EQUAL(links.itemTable_, headersCoverElectric);
    EXPECT_EQUAL(links.links_, dlxCoverElectric);

    Dx::PokemonLinks::encodingAndNum choice2 = links.overlappingCoverType(12, 5);
    EXPECT_EQUAL(choice2.num, 6);
    EXPECT_EQUAL(choice2.name.to_pair().first, "Fire");
    const std::vector<Dx::PokemonLinks::typeName> headersCoverGrass {
        {{""},5,4},
        {{"Electric"},0,2},
        {{"Fire"},0,3},
        {{"Grass"},0,4},
        {{"Ice"},0,5},
        {{"Normal"},4,0},
        {{"Water"},5,0},
    };
    const std::vector<Dx::PokemonLinks::pokeLink> dlxCoverGrass = {
        /*
         *
         *            Ice   Normal
         *  Grass
         *  Ice       x0.5
         *  Normal          x0.5
         *  Water
         *
         */
        // 0              1Electric     2Fire           3Grass         4Ice          5Normal        6Water
        {0,0,0,EM,0},   {3,21,8,EM,6},{3,24,9,EM,6}, {1,12,12,EM,5},{1,18,18,EM,0},{1,22,22,EM,0},{4,25,13,EM,5},
        // 7Electric      8             9
        {-1,0,9,EM,0},  {1,1,11,F2,6},{2,2,15,F2,6},
        // 10Fire         11                             12                                         13
        {-2,8,13,EM,0}, {1,8,21,F2,5},               {3,3,3,F2,5},                                {6,6,16,F2,5},
        // 14Grass                      15                                                          16
        {-3,11,16,EM,0},              {2,9,24,F2,0},                                              {6,13,19,F2,0},
        // 17Ice                                                       18                           19
        {-4,15,19,EM,0},                                             {4,4,4,F2,0},                {6,16,25,F2,0},
        // 20Normal                     21                                           22
        {-5,18,22,EM,0},{1,11,1,F2,0},                                             {5,5,5,F2,0},
        // 23Water                      24                                                          25
        {-6,21,25,EM,0},              {2,15,2,F2,0},                                              {6,19,6,F2,0},
        // 26
        {INT_MIN,24,INT_MIN,EM,0},
    };
    EXPECT_EQUAL(links.itemTable_, headersCoverGrass);
    EXPECT_EQUAL(links.links_, dlxCoverGrass);
    links.overlappingUncoverType(12);
    EXPECT_EQUAL(links.itemTable_, headersCoverElectric);
    EXPECT_EQUAL(links.links_, dlxCoverElectric);
    links.overlappingUncoverType(8);
    EXPECT_EQUAL(links.links_, dlx);
    EXPECT_EQUAL(links.itemTable_, headers);
}

STUDENT_TEST("Overlapping allows two types to cover same opposing type i.e. Fire and Electric") {
    /*
     * This is just nonsense type weakness information in pairs to I can test the cover logic.
     *
     *            Electric  Fire  Grass  Ice   Normal  Water
     *  Electric   x0.5     x0.5
     *  Fire       x0.5           x0.5                 x0.5
     *  Grass               x0.5                       x0.5
     *  Ice                              x0.5          x0.5
     *  Normal     x0.5                        x0.5
     *  Water              x0.5                        x0.5
     *
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types {
        {{"Electric"}, {{{"Electric"},F2},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},NM}}},
        {{"Fire"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},F2},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Grass"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Ice"}, {{{"Electric"},NM},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},F2},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Normal"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},F2},{{"Water"},NM}}},
        {{"Water"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
    };
    Dx::PokemonLinks links (types, Dx::PokemonLinks::DEFENSE);
    std::set<RankedSet<Dx::TypeEncoding>> result = links.getOverlappingCoverages(6);
    std::set<RankedSet<Dx::TypeEncoding>> correct = {
        {18,{{"Electric"},{"Fire"},{"Ice"},{"Normal"}}},
        {18,{{"Fire"},{"Grass"},{"Ice"},{"Normal"}}},
        {18,{{"Fire"},{"Ice"},{"Normal"},{"Water"}}}
    };
    EXPECT_EQUAL(correct, result);
}

STUDENT_TEST("There is one exact and a few overlapping covers here. Let's see overlapping.") {
    /*
     *                     Electric    Fire    Grass    Ice    Normal    Water
     *
     *   Bug-Ghost                              x.5             x0
     *
     *   Electric-Grass     x.25                x.5                       x.5
     *
     *   Fire-Flying                   x.5      x.25
     *
     *   Ground-Water       x0         x.5
     *
     *   Ice-Psychic                                    x.5
     *
     *   Ice-Water                                      x.25              x.5
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types = {
        /* In reality maps will have every type present in every key. But I know the internals
         * of my implementation and will just enter all types for the first key to make entering
         * the rest of the test cases easier.
         */
        {{"Bug-Ghost"},{{{"Electric"},NM},{{"Fire"},NM},{{"Grass"},F2},{{"Ice"},NM},{{"Normal"},IM},{{"Water"},NM}}},
        {{"Electric-Grass"},{{{"Electric"},F4},{{"Grass"},F2},{{"Water"},F2}}},
        {{"Fire-Flying"},{{{"Fire"},F2},{{"Grass"},F4}}},
        {{"Ground-Water"},{{{"Electric"},IM},{{"Fire"},F2}}},
        {{"Ice-Psychic"},{{{"Ice"},F2}}},
        {{"Ice-Water"},{{{"Ice"},F4},{{"Water"},F2}}},
    };
    Dx::PokemonLinks links(types, Dx::PokemonLinks::DEFENSE);
    std::set<RankedSet<Dx::TypeEncoding>> result = links.getOverlappingCoverages(6);
    std::set<RankedSet<Dx::TypeEncoding>> correct = {
        {13,{{"Bug-Ghost"},{"Ground-Water"},{"Ice-Water"}}},
        {14,{{"Bug-Ghost"},{"Electric-Grass"},{"Fire-Flying"},{"Ice-Water"}}},
        {14,{{"Bug-Ghost"},{"Electric-Grass"},{"Ground-Water"},{"Ice-Psychic"}}},
        {14,{{"Bug-Ghost"},{"Electric-Grass"},{"Ground-Water"},{"Ice-Water"}}},
        {14,{{"Bug-Ghost"},{"Ground-Water"},{"Ice-Psychic"},{"Ice-Water"}}},
        {15,{{"Bug-Ghost"},{"Electric-Grass"},{"Fire-Flying"},{"Ice-Psychic"}}},
        {15,{{"Bug-Ghost"},{"Electric-Grass"},{"Ground-Water"},{"Ice-Psychic"}}}
    };
    EXPECT_EQUAL(correct, result);
}


/* * * * * * * *    Test the Utility Functions for the Fun User Ops We Support      * * * * * * * */


STUDENT_TEST("Test binary search on the item table.") {
    /*
     * This is just nonsense type weakness information in pairs to I can test the cover logic.
     *
     *            Electric  Fire  Grass  Ice   Normal  Water
     *  Electric   x0.5     x0.5
     *  Fire       x0.5           x0.5                 x0.5
     *  Grass               x0.5                       x0.5
     *  Ice                              x0.5          x0.5
     *  Normal     x0.5                        x0.5
     *  Water              x0.5                        x0.5
     *
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types {
        {{"Electric"}, {{{"Electric"},F2},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},NM}}},
        {{"Fire"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},F2},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Grass"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Ice"}, {{{"Electric"},NM},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},F2},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Normal"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},F2},{{"Water"},NM}}},
        {{"Water"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
    };
    const std::vector<Dx::PokemonLinks::typeName> headers {
        {{""},6,1},
        {{"Electric"},0,2},
        {{"Fire"},1,3},
        {{"Grass"},2,4},
        {{"Ice"},3,5},
        {{"Normal"},4,6},
        {{"Water"},5,0},
    };
    Dx::PokemonLinks links(types, Dx::PokemonLinks::DEFENSE);
    EXPECT_EQUAL(links.findItemIndex(Dx::TypeEncoding(std::string{"Electric"})), 1);
    EXPECT_EQUAL(links.findItemIndex(Dx::TypeEncoding(std::string{"Fire"})), 2);
    EXPECT_EQUAL(links.findItemIndex(Dx::TypeEncoding(std::string{"Grass"})), 3);
    EXPECT_EQUAL(links.findItemIndex(Dx::TypeEncoding(std::string{"Ice"})), 4);
    EXPECT_EQUAL(links.findItemIndex(Dx::TypeEncoding(std::string{"Normal"})), 5);
    EXPECT_EQUAL(links.findItemIndex(Dx::TypeEncoding(std::string{"Water"})), 6);
    EXPECT_EQUAL(links.findItemIndex(Dx::TypeEncoding(std::string{"Flamio"})), 0);
}

STUDENT_TEST("Test binary search on the option table.") {
    /*
     * This is just nonsense type weakness information in pairs to I can test the cover logic.
     *
     *            Electric  Fire  Grass  Ice   Normal  Water
     *  Electric   x0.5     x0.5
     *  Fire       x0.5           x0.5                 x0.5
     *  Grass               x0.5                       x0.5
     *  Ice                              x0.5          x0.5
     *  Normal     x0.5                        x0.5
     *  Water              x0.5                        x0.5
     *
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types {
        {{"Electric"}, {{{"Electric"},F2},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},NM}}},
        {{"Fire"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},F2},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Grass"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Ice"}, {{{"Electric"},NM},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},F2},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Normal"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},F2},{{"Water"},NM}}},
        {{"Water"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
    };
    const std::vector<Dx::PokemonLinks::typeName> headers {
        {{""},6,1},
        {{"Electric"},0,2},
        {{"Fire"},1,3},
        {{"Grass"},2,4},
        {{"Ice"},3,5},
        {{"Normal"},4,6},
        {{"Water"},5,0},
    };
    const std::vector<Dx::PokemonLinks::pokeLink> dlx = {
        // 0             1Electric      2Fire           3Grass        4Ice           5Normal        6Water
        {0,0,0,EM,0},   {3,21,8,EM,0},{3,24,9,EM,0},{1,12,12,EM,0},{1,18,18,EM,0},{1,22,22,EM,0},{4,25,13,EM,0},
        // 7Electric      8             9
        {-1,0,9,EM,0},  {1,1,11,F2,0},{2,2,15,F2,0},
        // 10Fire         11                            12                                          13
        {-2,8,13,EM,0}, {1,8,21,F2,0},              {3,3,3,F2,0},                                {6,6,16,F2,0},
        // 14Grass                      15                                                         16
        {-3,11,16,EM,0},              {2,9,24,F2,0},                                             {6,13,19,F2,0},
        // 17Ice                                                      18                           19
        {-4,15,19,EM,0},                                           {4,4,4,F2,0},                 {6,16,25,F2,0},
        // 20Normal                     21                                            22
        {-5,18,22,EM,0},{1,11,1,F2,0},                                            {5,5,5,F2,0},
        // 23Water                      24                                                         25
        {-6,21,25,EM,0},              {2,15,2,F2,0},                                             {6,19,6,F2,0},
        //       26
        {INT_MIN,24,INT_MIN,EM,0},
    };
    Dx::PokemonLinks links(types, Dx::PokemonLinks::DEFENSE);
    EXPECT_EQUAL(links.findOptionIndex(Dx::TypeEncoding(std::string("Electric"))), 7);
    EXPECT_EQUAL(links.findOptionIndex(Dx::TypeEncoding(std::string("Fire"))), 10);
    EXPECT_EQUAL(links.findOptionIndex(Dx::TypeEncoding(std::string("Grass"))), 14);
    EXPECT_EQUAL(links.findOptionIndex(Dx::TypeEncoding(std::string("Ice"))), 17);
    EXPECT_EQUAL(links.findOptionIndex(Dx::TypeEncoding(std::string("Normal"))), 20);
    EXPECT_EQUAL(links.findOptionIndex(Dx::TypeEncoding(std::string("Water"))), 23);
    EXPECT_EQUAL(links.findOptionIndex(Dx::TypeEncoding(std::string("Flamio"))), 0);
}


/* * * * * * * *      Test the Hiding of Options and Items the User Can Use         * * * * * * * */


STUDENT_TEST("Test hiding an item from the world.") {
    /*
     * This is just nonsense type weakness information in pairs to I can test the cover logic.
     *
     *            Electric  Fire  Grass  Ice   Normal  Water
     *  Electric   x0.5     x0.5
     *  Fire       x0.5           x0.5                 x0.5
     *  Grass               x0.5                       x0.5
     *  Ice                              x0.5          x0.5
     *  Normal     x0.5                        x0.5
     *  Water              x0.5                        x0.5
     *
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types {
        {{"Electric"}, {{{"Electric"},F2},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},NM}}},
        {{"Fire"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},F2},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Grass"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Ice"}, {{{"Electric"},NM},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},F2},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Normal"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},F2},{{"Water"},NM}}},
        {{"Water"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
    };
    const std::vector<Dx::PokemonLinks::typeName> headers {
        {{""},6,1},
        {{"Electric"},0,2},
        {{"Fire"},1,3},
        {{"Grass"},2,4},
        {{"Ice"},3,5},
        {{"Normal"},4,6},
        {{"Water"},5,0},
    };
    const std::vector<Dx::PokemonLinks::pokeLink> dlx = {
        // 0             1Electric     2Fire         3Grass          4Ice          5Normal        6Water
        {0,0,0,EM,0},  {3,21,8,EM,0},{3,24,9,EM,0},{1,12,12,EM,0},{1,18,18,EM,0},{1,22,22,EM,0},{4,25,13,EM,0},
        // 7Electric     8             9
        {-1,0,9,EM,0}, {1,1,11,F2,0},{2,2,15,F2,0},
        // 10Fire        11                          12                                           13
        {-2,8,13,EM,0},{1,8,21,F2,0},              {3,3,3,F2,0},                                {6,6,16,F2,0},
        // 14Grass                    15                                                          16
        {-3,11,16,EM,0},             {2,9,24,F2,0},                                             {6,13,19,F2,0},
        // 17Ice                                                     18                           19
        {-4,15,19,EM,0},                                          {4,4,4,F2,0},                 {6,16,25,F2,0},
        // 20Normal      21                                                        22
        {-5,18,22,EM,0},{1,11,1,F2,0},                                           {5,5,5,F2,0},
        // 23Water                    24                                                          25
        {-6,21,25,EM,0},             {2,15,2,F2,0},                                             {6,19,6,F2,0},
        //       26
        {INT_MIN,24,INT_MIN,EM,0},
    };
    Dx::PokemonLinks links(types, Dx::PokemonLinks::DEFENSE);
    EXPECT(links.hideRequestedItem(Dx::TypeEncoding(std::string("Fire"))));
    const std::vector<Dx::PokemonLinks::typeName> headersHideFire {
        {{""},6,1},
        {{"Electric"},0,3},
        {{"Fire"},1,3},
        {{"Grass"},1,4},
        {{"Ice"},3,5},
        {{"Normal"},4,6},
        {{"Water"},5,0},
    };
    const int HD = Dx::PokemonLinks::HIDDEN;
    const std::vector<Dx::PokemonLinks::pokeLink> dlxHideFire = {
        //0               1Electric     2Fire            3Grass        4Ice           5Normal        6Water
        {0,0,0,EM,0},   {3,21,8,EM,0},{3,24,9,EM,HD}, {1,12,12,EM,0},{1,18,18,EM,0},{1,22,22,EM,0},{4,25,13,EM,0},
        //7Electric       8             9
        {-1,0,9,EM,0},  {1,1,11,F2,0},{2,2,15,F2,0},
        //10Fire          11                             12                                          13
        {-2,8,13,EM,0}, {1,8,21,F2,0},                {3,3,3,F2,0},                                {6,6,16,F2,0},
        //14Grass                       15                                                           16
        {-3,11,16,EM,0},              {2,9,24,F2,0},                                               {6,13,19,F2,0},
        //17Ice                                                        18                            19
        {-4,15,19,EM,0},                                             {4,4,4,F2,0},                 {6,16,25,F2,0},
        //20Normal        21                                                          22
        {-5,18,22,EM,0},{1,11,1,F2,0},                                              {5,5,5,F2,0},
        //23Water                       24                                                           25
        {-6,21,25,EM,0},              {2,15,2,F2,0},                                               {6,19,6,F2,0},
        //26
        {INT_MIN,24,INT_MIN,EM,0},
    };
    EXPECT_EQUAL(links.links_, dlxHideFire);
    EXPECT_EQUAL(links.itemTable_, headersHideFire);
    EXPECT(!links.hideRequestedItem(Dx::TypeEncoding(std::string("Fire"))));
    EXPECT_EQUAL(links.links_, dlxHideFire);
    EXPECT_EQUAL(links.peekHidItem().to_pair().first, "Fire");
    EXPECT_EQUAL(links.getNumHidItems(), 1);

    // Test our unhide and reset functions.
    links.popHidItem();
    EXPECT_EQUAL(links.links_, dlx);
    EXPECT_EQUAL(links.itemTable_, headers);
    EXPECT(links.hidItemsEmpty());
    EXPECT_EQUAL(links.getNumHidItems(), 0);
    EXPECT(links.hideRequestedItem(Dx::TypeEncoding(std::string("Fire"))));
    links.resetItems();
    EXPECT_EQUAL(links.links_, dlx);
    EXPECT_EQUAL(links.itemTable_, headers);
    EXPECT(links.hidItemsEmpty());
    EXPECT_EQUAL(links.getNumHidItems(), 0);
}

STUDENT_TEST("Test hiding Grass and Ice and then reset the links.") {
    /*
     * This is just nonsense type weakness information in pairs to I can test the cover logic.
     *
     *            Electric  Fire  Grass  Ice   Normal  Water
     *  Electric   x0.5     x0.5
     *  Fire       x0.5           x0.5                 x0.5
     *  Grass               x0.5                       x0.5
     *  Ice                              x0.5          x0.5
     *  Normal     x0.5                        x0.5
     *  Water              x0.5                        x0.5
     *
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types {
        {{"Electric"}, {{{"Electric"},F2},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},NM}}},
        {{"Fire"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},F2},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Grass"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Ice"}, {{{"Electric"},NM},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},F2},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Normal"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},F2},{{"Water"},NM}}},
        {{"Water"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
    };
    const std::vector<Dx::PokemonLinks::pokeLink> dlx = {
        // 0             1Electric     2Fire         3Grass          4Ice          5Normal        6Water
        {0,0,0,EM,0},  {3,21,8,EM,0},{3,24,9,EM,0},{1,12,12,EM,0},{1,18,18,EM,0},{1,22,22,EM,0},{4,25,13,EM,0},
        // 7Electric     8             9
        {-1,0,9,EM,0}, {1,1,11,F2,0},{2,2,15,F2,0},
        // 10Fire        11                          12                                           13
        {-2,8,13,EM,0},{1,8,21,F2,0},              {3,3,3,F2,0},                                {6,6,16,F2,0},
        // 14Grass                    15                                                          16
        {-3,11,16,EM,0},             {2,9,24,F2,0},                                             {6,13,19,F2,0},
        // 17Ice                                                     18                           19
        {-4,15,19,EM,0},                                          {4,4,4,F2,0},                 {6,16,25,F2,0},
        // 20Normal      21                                                        22
        {-5,18,22,EM,0},{1,11,1,F2,0},                                           {5,5,5,F2,0},
        // 23Water                    24                                                          25
        {-6,21,25,EM,0},             {2,15,2,F2,0},                                             {6,19,6,F2,0},
        //       26
        {INT_MIN,24,INT_MIN,EM,0},
    };
    Dx::PokemonLinks links(types, Dx::PokemonLinks::DEFENSE);
    EXPECT(links.hideRequestedOption({{"Grass"},{"Ice"}}));
    const int HD = Dx::PokemonLinks::HIDDEN;
    const std::vector<Dx::PokemonLinks::pokeLink> dlxHideOptionIceGrass = {
        // 0               1Electric     2Fire           3Grass        4Ice         5Normal        6Water
        {0,0,0,EM,0},    {3,21,8,EM,0},{2,24,9,EM,0}, {1,12,12,EM,0},{0,4,4,EM,0},{1,22,22,EM,0},{2,25,13,EM,0},
        // 7Electric       8             9
        {-1,0,9,EM,0},   {1,1,11,F2,0},{2,2,24,F2,0},
        // 10Fire          11                            12                                        13
        {-2,8,13,EM,0},  {1,8,21,F2,0},               {3,3,3,F2,0},                              {6,6,25,F2,0},
        // 14Grass                       15                                                        16
        {-3,11,16,EM,HD},              {2,9,24,F2,0},                                            {6,13,19,F2,0},
        // 17Ice                                                       18                          19
        {-4,15,19,EM,HD},                                            {4,4,4,F2,0},               {6,13,25,F2,0},
        // 20Normal        21                                                       22
        {-5,18,22,EM,0},{1,11,1,F2,0},                                            {5,5,5,F2,0},
        // 23Water                                       24                                        25
        {-6,21,25,EM,0},                              {2,9,2,F2,0},                              {6,13,6,F2,0},
        // 26
        {INT_MIN,24,INT_MIN,EM,0},
    };
    EXPECT_EQUAL(links.links_, dlxHideOptionIceGrass);
    links.resetOptions();
    EXPECT_EQUAL(links.links_, dlx);
    EXPECT(links.hidItemsEmpty());
    EXPECT_EQUAL(links.getNumHidOptions(), 0);
}

STUDENT_TEST("Test hiding an option from the world.") {
    /*
     * This is just nonsense type weakness information in pairs to I can test the cover logic.
     *
     *            Electric  Fire  Grass  Ice   Normal  Water
     *  Electric   x0.5     x0.5
     *  Fire       x0.5           x0.5                 x0.5
     *  Grass               x0.5                       x0.5
     *  Ice                              x0.5          x0.5
     *  Normal     x0.5                        x0.5
     *  Water              x0.5                        x0.5
     *
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types {
        {{"Electric"}, {{{"Electric"},F2},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},NM}}},
        {{"Fire"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},F2},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Grass"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Ice"}, {{{"Electric"},NM},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},F2},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Normal"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},F2},{{"Water"},NM}}},
        {{"Water"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
    };
    const std::vector<Dx::PokemonLinks::pokeLink> dlx = {
        // 0             1Electric     2Fire         3Grass          4Ice          5Normal        6Water
        {0,0,0,EM,0},  {3,21,8,EM,0},{3,24,9,EM,0},{1,12,12,EM,0},{1,18,18,EM,0},{1,22,22,EM,0},{4,25,13,EM,0},
        // 7Electric     8             9
        {-1,0,9,EM,0}, {1,1,11,F2,0},{2,2,15,F2,0},
        // 10Fire        11                          12                                           13
        {-2,8,13,EM,0},{1,8,21,F2,0},              {3,3,3,F2,0},                                {6,6,16,F2,0},
        // 14Grass                    15                                                          16
        {-3,11,16,EM,0},             {2,9,24,F2,0},                                             {6,13,19,F2,0},
        // 17Ice                                                     18                           19
        {-4,15,19,EM,0},                                          {4,4,4,F2,0},                 {6,16,25,F2,0},
        // 20Normal      21                                                        22
        {-5,18,22,EM,0},{1,11,1,F2,0},                                           {5,5,5,F2,0},
        // 23Water                    24                                                          25
        {-6,21,25,EM,0},             {2,15,2,F2,0},                                             {6,19,6,F2,0},
        //       26
        {INT_MIN,24,INT_MIN,EM,0},
    };
    Dx::PokemonLinks links(types, Dx::PokemonLinks::DEFENSE);

    EXPECT(links.hideRequestedOption(Dx::TypeEncoding(std::string("Fire"))));
    Dx::TypeEncoding fire("Fire");
    Dx::TypeEncoding flipper("Fire");
    std::vector<Dx::TypeEncoding> failedToHide = {};
    EXPECT(!links.hideRequestedOption({fire, flipper}));
    EXPECT(!links.hideRequestedOption({fire, flipper}, failedToHide));
    EXPECT_EQUAL(failedToHide, {fire, flipper});

    const int HD = Dx::PokemonLinks::HIDDEN;
    const std::vector<Dx::PokemonLinks::pokeLink> dlxHideOptionFire = {
        // 0              1Electric     2Fire         3Grass          4Ice          5Normal        6Water
        {0,0,0,EM,0},   {2,21,8,EM,0},{3,24,9,EM,0},{0,3,3,EM,0},{1,18,18,EM,0},{1,22,22,EM,0},{3,25,16,EM,0},
        // 7Electric      8             9
        {-1,0,9,EM,0},  {1,1,21,F2,0},{2,2,15,F2,0},
        // 10Fire         11                          12                                           13
        {-2,8,13,EM,HD},{1,8,21,F2,0},              {3,3,3,F2,0},                               {6,6,16,F2,0},
        // 14Grass                      15                                                         16
        {-3,11,16,EM,0},              {2,9,24,F2,0},                                            {6,6,19,F2,0},
        // 17Ice                                                     18                            19
        {-4,15,19,EM,0},                                          {4,4,4,F2,0},                 {6,16,25,F2,0},
        // 20Normal       21                                                        22
        {-5,18,22,EM,0},{1,8,1,F2,0},                                            {5,5,5,F2,0},
        // 23Water                      24                                                         25
        {-6,21,25,EM,0},              {2,15,2,F2,0},                                            {6,19,6,F2,0},
        //       26
        {INT_MIN,24,INT_MIN,EM,0},
    };
    EXPECT_EQUAL(links.links_, dlxHideOptionFire);
    EXPECT(!links.hideRequestedOption(fire));
    EXPECT_EQUAL(links.links_, dlxHideOptionFire);
    EXPECT_EQUAL(links.peekHidOption(), fire);
    EXPECT_EQUAL(links.getNumHidOptions(), 1);

    links.popHidOption();
    EXPECT_EQUAL(links.links_, dlx);
    EXPECT(links.hidItemsEmpty());
    EXPECT_EQUAL(links.getNumHidOptions(), 0);
    EXPECT(links.hideRequestedOption(fire));
    links.resetOptions();
    EXPECT_EQUAL(links.links_, dlx);
    EXPECT(links.hidItemsEmpty());
    EXPECT_EQUAL(links.getNumHidOptions(), 0);
}

STUDENT_TEST("Test hiding an item from the world and then solving both types of cover.") {
    /*
     * This is just nonsense type weakness information in pairs to I can test the cover logic.
     *
     *            Electric  Fire  Grass  Ice   Normal  Water
     *  Electric   x0.5     x0.5
     *  Fire       x0.5           x0.5
     *  Grass               x0.5                       x0.5
     *  Ice                              x0.5          x0.5
     *  Normal     x0.5                        x0.5
     *  Water              x0.5                        x0.5
     *
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types {
        {{"Electric"}, {{{"Electric"},F2},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},NM}}},
        {{"Fire"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},F2},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},DB}}},
        {{"Grass"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Ice"}, {{{"Electric"},NM},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},F2},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Normal"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},F2},{{"Water"},NM}}},
        {{"Water"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
    };
    const std::vector<Dx::PokemonLinks::typeName> headers {
        {{""},6,1},
        {{"Electric"},0,2},
        {{"Fire"},1,3},
        {{"Grass"},2,4},
        {{"Ice"},3,5},
        {{"Normal"},4,6},
        {{"Water"},5,0},
    };
    const std::vector<Dx::PokemonLinks::pokeLink> dlx = {
        // 0               1Electric    2Fire         3Grass         4Ice          5Normal         6Water
        {0,0,0,EM,0},   {3,20,8,EM,0},{3,23,9,EM,0},{1,12,12,EM,0},{1,17,17,EM,0},{1,21,21,EM,0},{3,24,15,EM,0},
        // 7Electric
        {-1,0,9,EM,0},  {1,1,11,F2,0},{2,2,14,F2,0},
        // 10Fire
        {-2,8,12,EM,0}, {1,8,20,F2,0},              {3,3,3,F2,0},
        // 13Grass
        {-3,11,15,EM,0},              {2,9,23,F2,0},                                             {6,6,18,F2,0},
        // 16Ice
        {-4,14,18,EM,0},                                           {4,4,4,F2,0},                 {6,15,24,F2,0},
        // 19Normal
        {-5,17,21,EM,0},{1,11,1,F2,0},                                            {5,5,5,F2,0},
        // 22Water
        {-6,20,24,EM,0},              {2,14,2,F2,0},                                             {6,18,6,F2,0},
        // 25
        {INT_MIN,23,INT_MIN,EM,0},
    };
    Dx::PokemonLinks links(types, Dx::PokemonLinks::DEFENSE);
    Dx::TypeEncoding electric("Electric");
    EXPECT(links.hideRequestedItem(electric));
    const std::vector<Dx::PokemonLinks::typeName> headersHideElectric {
        {{""},6,2},
        {{"Electric"},0,2},
        {{"Fire"},0,3},
        {{"Grass"},2,4},
        {{"Ice"},3,5},
        {{"Normal"},4,6},
        {{"Water"},5,0},
    };
    const int HD = Dx::PokemonLinks::HIDDEN;
    const std::vector<Dx::PokemonLinks::pokeLink> dlxHideElectric = {
        // 0               1Electric     2Fire         3Grass         4Ice          5Normal         6Water
        {0,0,0,EM,0},   {3,20,8,EM,HD},{3,23,9,EM,0},{1,12,12,EM,0},{1,17,17,EM,0},{1,21,21,EM,0},{3,24,15,EM,0},
        // 7Electric
        {-1,0,9,EM,0},  {1,1,11,F2,0}, {2,2,14,F2,0},
        // 10Fire
        {-2,8,12,EM,0}, {1,8,20,F2,0},               {3,3,3,F2,0},
        // 13Grass
        {-3,11,15,EM,0},               {2,9,23,F2,0},                                             {6,6,18,F2,0},
        // 16Ice
        {-4,14,18,EM,0},                                            {4,4,4,F2,0},                 {6,15,24,F2,0},
        // 19Normal
        {-5,17,21,EM,0},{1,11,1,F2,0},                                             {5,5,5,F2,0},
        // 22Water
        {-6,20,24,EM,0},               {2,14,2,F2,0},                                             {6,18,6,F2,0},
        // 25
        {INT_MIN,23,INT_MIN,EM,0},
    };
    EXPECT_EQUAL(links.links_, dlxHideElectric);
    EXPECT_EQUAL(links.itemTable_, headersHideElectric);
    EXPECT_EQUAL(links.getNumItems(), 5);
    EXPECT_EQUAL(links.getExactCoverages(6), {{15,{{"Electric"},{"Fire"},{"Ice"},{"Normal"}}}});
    EXPECT_EQUAL(links.getOverlappingCoverages(6), {{15,{{"Electric"},{"Fire"},{"Ice"},{"Normal"}}},
                                                    {15,{{"Fire"},{"Grass"},{"Ice"},{"Normal"}}},
                                                    {15,{{"Fire"},{"Ice"},{"Normal"},{"Water"}}}});
}

STUDENT_TEST("Test hiding two items from the world and then solving both types of cover.") {
    /*
     * This is just nonsense type weakness information in pairs to I can test the cover logic.
     *
     *            Electric  Fire  Grass  Ice   Normal  Water
     *  Electric   x0.5     x0.5
     *  Fire       x0.5           x0.5                 x0.5
     *  Grass               x0.5                       x0.5
     *  Ice                              x0.5          x0.5
     *  Normal     x0.5                        x0.5
     *  Water              x0.5                        x0.5
     *
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types {
        {{"Electric"}, {{{"Electric"},F2},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},NM}}},
        {{"Fire"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},F2},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},DB}}},
        {{"Grass"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Ice"}, {{{"Electric"},NM},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},F2},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Normal"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},F2},{{"Water"},NM}}},
        {{"Water"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
    };
    const std::vector<Dx::PokemonLinks::typeName> headers {
        {{""},6,1},
        {{"Electric"},0,2},
        {{"Fire"},1,3},
        {{"Grass"},2,4},
        {{"Ice"},3,5},
        {{"Normal"},4,6},
        {{"Water"},5,0},
    };
    const std::vector<Dx::PokemonLinks::pokeLink> dlx = {
        // 0               1Electric    2Fire         3Grass         4Ice          5Normal         6Water
        {0,0,0,EM,0},   {3,20,8,EM,0},{3,23,9,EM,0},{1,12,12,EM,0},{1,17,17,EM,0},{1,21,21,EM,0},{3,24,15,EM,0},
        // 7Electric
        {-1,0,9,EM,0},  {1,1,11,F2,0},{2,2,14,F2,0},
        // 10Fire
        {-2,8,12,EM,0}, {1,8,20,F2,0},              {3,3,3,F2,0},
        // 13Grass
        {-3,11,15,EM,0},              {2,9,23,F2,0},                                             {6,6,18,F2,0},
        // 16Ice
        {-4,14,18,EM,0},                                           {4,4,4,F2,0},                 {6,15,24,F2,0},
        // 19Normal
        {-5,17,21,EM,0},{1,11,1,F2,0},                                            {5,5,5,F2,0},
        // 22Water
        {-6,20,24,EM,0},              {2,14,2,F2,0},                                             {6,18,6,F2,0},
        // 25
        {INT_MIN,23,INT_MIN,EM,0},
    };
    Dx::PokemonLinks links(types, Dx::PokemonLinks::DEFENSE);
    Dx::TypeEncoding electric("Electric");
    EXPECT(links.hideRequestedItem(electric));
    std::vector<Dx::TypeEncoding> failOutput = {};
    Dx::TypeEncoding grass("Grass");
    Dx::TypeEncoding grassy("Grassy");
    Dx::TypeEncoding cloudy("Cloudy");
    Dx::TypeEncoding rainy("Rainy");
    EXPECT(!links.hideRequestedItem({grass, electric, grassy, cloudy, rainy}, failOutput));
    EXPECT_EQUAL(failOutput, {electric,grassy,cloudy,rainy});

    const std::vector<Dx::PokemonLinks::typeName> headersHideElectricAndGrass {
        {{""},6,2},
        {{"Electric"},0,2},
        {{"Fire"},0,4},
        {{"Grass"},2,4},
        {{"Ice"},2,5},
        {{"Normal"},4,6},
        {{"Water"},5,0},
    };
    const int HD = Dx::PokemonLinks::HIDDEN;
    const std::vector<Dx::PokemonLinks::pokeLink> dlxHideElectricAndGrass = {
        // 0               1Electric    2Fire         3Grass         4Ice          5Normal         6Water
        {0,0,0,EM,0},   {3,20,8,EM,HD},{3,23,9,EM,0},{1,12,12,EM,HD},{1,17,17,EM,0},{1,21,21,EM,0},{3,24,15,EM,0},
        // 7Electric
        {-1,0,9,EM,0},  {1,1,11,F2,0}, {2,2,14,F2,0},
        // 10Fire
        {-2,8,12,EM,0}, {1,8,20,F2,0},               {3,3,3,F2,0},
        // 13Grass
        {-3,11,15,EM,0},               {2,9,23,F2,0},                                             {6,6,18,F2,0},
        // 16Ice
        {-4,14,18,EM,0},                                            {4,4,4,F2,0},                 {6,15,24,F2,0},
        // 19Normal
        {-5,17,21,EM,0},{1,11,1,F2,0},                                             {5,5,5,F2,0},
        // 22Water
        {-6,20,24,EM,0},               {2,14,2,F2,0},                                             {6,18,6,F2,0},
        // 25
        {INT_MIN,23,INT_MIN,EM,0},
    };
    EXPECT_EQUAL(links.links_, dlxHideElectricAndGrass);
    EXPECT_EQUAL(links.itemTable_, headersHideElectricAndGrass);
    EXPECT_EQUAL(links.getNumItems(), 4);
    EXPECT_EQUAL(links.getExactCoverages(6), {{12, {{"Electric"},{"Ice"},{"Normal"}}}});
    EXPECT_EQUAL(links.getOverlappingCoverages(6), {{12,{{"Electric"},{"Ice"},{"Normal"}}},
                                                     {12,{{"Grass"},{"Ice"},{"Normal"}}},
                                                     {12,{{"Ice"},{"Normal"},{"Water"}}}});
}

STUDENT_TEST("Test the hiding all the items except for the ones the user wants to keep.") {
    /*
     * This is just nonsense type weakness information in pairs to I can test the cover logic.
     *
     *            Electric  Fire  Grass  Ice   Normal  Water
     *  Electric   x0.5     x0.5
     *  Fire       x0.5           x0.5
     *  Grass               x0.5                       x0.5
     *  Ice                              x0.5          x0.5
     *  Normal     x0.5                        x0.5
     *  Water              x0.5                        x0.5
     *
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types {
        {{"Electric"}, {{{"Electric"},F2},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},NM}}},
        {{"Fire"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},F2},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},DB}}},
        {{"Grass"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Ice"}, {{{"Electric"},NM},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},F2},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Normal"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},F2},{{"Water"},NM}}},
        {{"Water"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
    };
    const std::vector<Dx::PokemonLinks::typeName> headers {
        {{""},6,1},
        {{"Electric"},0,2},
        {{"Fire"},1,3},
        {{"Grass"},2,4},
        {{"Ice"},3,5},
        {{"Normal"},4,6},
        {{"Water"},5,0},
    };
    const std::vector<Dx::PokemonLinks::pokeLink> dlx = {
        // 0               1Electric    2Fire         3Grass         4Ice          5Normal         6Water
        {0,0,0,EM,0},   {3,20,8,EM,0},{3,23,9,EM,0},{1,12,12,EM,0},{1,17,17,EM,0},{1,21,21,EM,0},{3,24,15,EM,0},
        // 7Electric
        {-1,0,9,EM,0},  {1,1,11,F2,0},{2,2,14,F2,0},
        // 10Fire
        {-2,8,12,EM,0}, {1,8,20,F2,0},              {3,3,3,F2,0},
        // 13Grass
        {-3,11,15,EM,0},              {2,9,23,F2,0},                                             {6,6,18,F2,0},
        // 16Ice
        {-4,14,18,EM,0},                                           {4,4,4,F2,0},                 {6,15,24,F2,0},
        // 19Normal
        {-5,17,21,EM,0},{1,11,1,F2,0},                                            {5,5,5,F2,0},
        // 22Water
        {-6,20,24,EM,0},              {2,14,2,F2,0},                                             {6,18,6,F2,0},
        // 25
        {INT_MIN,23,INT_MIN,EM,0},
    };
    Dx::PokemonLinks links(types, Dx::PokemonLinks::DEFENSE);
    links.hideAllItemsExcept({Dx::TypeEncoding("Water")});
    const std::vector<Dx::PokemonLinks::typeName> headersHideExceptWater {
        {{""},6,6},
        {{"Electric"},0,2},
        {{"Fire"},0,3},
        {{"Grass"},0,4},
        {{"Ice"},0,5},
        {{"Normal"},0,6},
        {{"Water"},0,0},
    };
    const int HD = Dx::PokemonLinks::HIDDEN;
    const std::vector<Dx::PokemonLinks::pokeLink> dlxHideExceptWater = {
        // 0               1Electric      2Fire           3Grass          4Ice           5Normal         6Water
        {0,0,0,EM,0},   {3,20,8,EM,HD},{3,23,9,EM,HD},{1,12,12,EM,HD},{1,17,17,EM,HD},{1,21,21,EM,HD},{3,24,15,EM,0},
        // 7Electric
        {-1,0,9,EM,0},  {1,1,11,F2,0}, {2,2,14,F2,0},
        // 10Fire
        {-2,8,12,EM,0}, {1,8,20,F2,0},                {3,3,3,F2,0},
        // 13Grass
        {-3,11,15,EM,0},               {2,9,23,F2,0},                                                 {6,6,18,F2,0},
        // 16Ice
        {-4,14,18,EM,0},                                              {4,4,4,F2,0},                   {6,15,24,F2,0},
        // 19Normal
        {-5,17,21,EM,0},{1,11,1,F2,0},                                                {5,5,5,F2,0},
        // 22Water
        {-6,20,24,EM,0},              {2,14,2,F2,0},                                                  {6,18,6,F2,0},
        // 25
        {INT_MIN,23,INT_MIN,EM,0},
    };
    EXPECT_EQUAL(links.links_, dlxHideExceptWater);
    EXPECT_EQUAL(links.itemTable_, headersHideExceptWater);
    EXPECT_EQUAL(links.getNumItems(), 1);
    EXPECT_EQUAL(links.getExactCoverages(6), {{3, {{"Grass"}}},{3,{{"Ice"}}},{3,{{"Water"}}}});
    EXPECT_EQUAL(links.getOverlappingCoverages(6), {{3, {{"Grass"}}},{3,{{"Ice"}}},{3,{{"Water"}}}});
    links.resetItems();
    EXPECT_EQUAL(links.links_, dlx);
    EXPECT_EQUAL(links.itemTable_, headers);
    EXPECT_EQUAL(links.getNumHidItems(), 0);
}

STUDENT_TEST("Test hiding all options and items except one each. One exact/overlapping solution.") {
    /*
     * This is just nonsense type weakness information in pairs to I can test the cover logic.
     *
     *            Electric  Fire  Grass  Ice   Normal  Water
     *  Electric   x0.5     x0.5
     *  Fire       x0.5           x0.5
     *  Grass               x0.5                       x0.5
     *  Ice                              x0.5          x0.5
     *  Normal     x0.5                        x0.5
     *  Water              x0.5                        x0.5
     *
     */
    const std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> types {
        {{"Electric"}, {{{"Electric"},F2},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},NM}}},
        {{"Fire"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},F2},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},DB}}},
        {{"Grass"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Ice"}, {{{"Electric"},NM},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},F2},{{"Normal"},NM},{{"Water"},F2}}},
        {{"Normal"}, {{{"Electric"},F2},{{"Fire"},NM},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},F2},{{"Water"},NM}}},
        {{"Water"}, {{{"Electric"},NM},{{"Fire"},F2},{{"Grass"},NM},{{"Ice"},NM},{{"Normal"},NM},{{"Water"},F2}}},
    };
    const std::vector<Dx::PokemonLinks::typeName> headers {
        {{""},6,1},
        {{"Electric"},0,2},
        {{"Fire"},1,3},
        {{"Grass"},2,4},
        {{"Ice"},3,5},
        {{"Normal"},4,6},
        {{"Water"},5,0},
    };
    const std::vector<Dx::PokemonLinks::pokeLink> dlx = {
        // 0               1Electric    2Fire         3Grass         4Ice          5Normal         6Water
        {0,0,0,EM,0},   {3,20,8,EM,0},{3,23,9,EM,0},{1,12,12,EM,0},{1,17,17,EM,0},{1,21,21,EM,0},{3,24,15,EM,0},
        // 7Electric
        {-1,0,9,EM,0},  {1,1,11,F2,0},{2,2,14,F2,0},
        // 10Fire
        {-2,8,12,EM,0}, {1,8,20,F2,0},              {3,3,3,F2,0},
        // 13Grass
        {-3,11,15,EM,0},              {2,9,23,F2,0},                                             {6,6,18,F2,0},
        // 16Ice
        {-4,14,18,EM,0},                                           {4,4,4,F2,0},                 {6,15,24,F2,0},
        // 19Normal
        {-5,17,21,EM,0},{1,11,1,F2,0},                                            {5,5,5,F2,0},
        // 22Water
        {-6,20,24,EM,0},              {2,14,2,F2,0},                                             {6,18,6,F2,0},
        // 25
        {INT_MIN,23,INT_MIN,EM,0},
    };
    Dx::PokemonLinks links(types, Dx::PokemonLinks::DEFENSE);
    Dx::TypeEncoding water("Water");
    Dx::TypeEncoding grass("Grass");
    Dx::TypeEncoding electric("Electric");
    Dx::TypeEncoding fire("Fire");
    Dx::TypeEncoding ice("Ice");
    Dx::TypeEncoding normal("Normal");
    links.hideAllItemsExcept({water});
    links.hideAllOptionsExcept({grass});
    EXPECT_EQUAL(links.getNumHidItems(), 5);
    EXPECT_EQUAL(links.getNumHidOptions(), 5);
    EXPECT(links.hasItem(water));
    EXPECT(links.hasOption(grass));
    EXPECT(!links.hasItem(grass));
    EXPECT(!links.hasItem(electric));
    EXPECT(!links.hasOption(electric));
    EXPECT(!links.hasItem(fire));
    EXPECT(!links.hasOption(fire));
    EXPECT(!links.hasItem(ice));
    EXPECT(!links.hasOption(ice));
    EXPECT(!links.hasItem(normal));
    EXPECT(!links.hasOption(normal));
    EXPECT(!links.hasOption(water));
    const std::vector<Dx::PokemonLinks::typeName> headersHideExceptWater {
        {{""},6,6},
        {{"Electric"},0,2},
        {{"Fire"},0,3},
        {{"Grass"},0,4},
        {{"Ice"},0,5},
        {{"Normal"},0,6},
        {{"Water"},0,0},
    };
    const int HD = Dx::PokemonLinks::HIDDEN;
    const std::vector<Dx::PokemonLinks::pokeLink> dlxHideExceptWater = {
        // 0               1Electric        2Fire         3Grass         4Ice         5Normal       6Water
        {0,0,0,EM,0},    {0,1,1,EM,HD},{1,14,14,EM,HD},{0,3,3,EM,HD},{0,4,4,EM,HD},{0,5,5,EM,HD},{1,15,15,EM,0},
        // 7Electric
        {-1,0,9,EM,HD},  {1,1,11,F2,0},{2,2,14,F2,0},
        // 10Fire
        {-2,8,12,EM,HD}, {1,1,20,F2,0},                {3,3,3,F2,0},
        // 13Grass
        {-3,11,15,EM,0},               {2,2,2,F2,0},                                             {6,6,6,F2,0},
        // 16Ice
        {-4,14,18,EM,HD},                                            {4,4,4,F2,0},               {6,15,24,F2,0},
        // 19Normal
        {-5,17,21,EM,HD},{1,1,1,F2,0},                                             {5,5,5,F2,0},
        // 22Water
        {-6,20,24,EM,HD},              {2,14,2,F2,0},                                            {6,15,6,F2,0},
        // 25
        {INT_MIN,23,INT_MIN,EM,0},
    };
    EXPECT_EQUAL(links.links_, dlxHideExceptWater);
    EXPECT_EQUAL(links.itemTable_, headersHideExceptWater);
    EXPECT_EQUAL(links.getNumItems(), 1);
    EXPECT_EQUAL(links.getNumOptions(), 1);
    EXPECT_EQUAL(links.getExactCoverages(6), {{3, {{"Grass"}}}});
    EXPECT_EQUAL(links.getOverlappingCoverages(6), {{3, {{"Grass"}}}});
    links.resetItems();
    links.resetOptions();
    EXPECT_EQUAL(links.links_, dlx);
    EXPECT_EQUAL(links.itemTable_, headers);
    EXPECT_EQUAL(links.getNumHidItems(), 0);
    EXPECT_EQUAL(links.getNumHidOptions(), 0);
    links.hideAllItemsExcept({water});
    links.hideAllOptionsExcept({grass});
    links.resetItemsOptions();
    EXPECT_EQUAL(links.links_, dlx);
    EXPECT_EQUAL(links.itemTable_, headers);
    EXPECT_EQUAL(links.getNumHidItems(), 0);
    EXPECT_EQUAL(links.getNumHidOptions(), 0);
}
