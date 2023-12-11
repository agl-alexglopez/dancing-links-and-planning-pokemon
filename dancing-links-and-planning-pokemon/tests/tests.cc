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
#include "map_parser.hh"
#include "point.hh"
#include "pokemon_links.hh"
#include "pokemon_parser.hh"
#include "ranked_set.hh"
#include "resistance.hh"
#include "type_encoding.hh"

#include <gtest/gtest.h>

#include <cstdint>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <unordered_map>
#include <vector>

/* * * * * * * *     All Operators We Overloaded Simply for Testing/Debugging       * * * * * * * */

namespace Dancing_links {
namespace Dx = Dancing_links;

std::ostream& operator<<( std::ostream& os, const std::set<Ranked_set<Type_encoding>>& solution )
{
  for ( const auto& i : solution ) {
    os << i;
  }
  return os;
}

std::ostream& operator<<( std::ostream& os, const std::vector<Type_encoding>& types )
{
  for ( const auto& t : types ) {
    os << t << ',';
  }
  return os;
}

std::ostream& operator<<( std::ostream& os, const std::set<Type_encoding>& types )
{
  for ( const auto& t : types ) {
    os << t << ',';
  }
  return os;
}

bool operator==( const Pokemon_links::Poke_link& lhs, const Pokemon_links::Poke_link& rhs )
{
  return lhs.top_or_len == rhs.top_or_len && lhs.up == rhs.up && lhs.down == rhs.down
         && lhs.multiplier == rhs.multiplier && lhs.tag == rhs.tag;
}

bool operator!=( const Pokemon_links::Poke_link& lhs, const Pokemon_links::Poke_link& rhs )
{
  return !( lhs == rhs );
}

std::ostream& operator<<( std::ostream& os, const Pokemon_links::Poke_link& link )
{
  return os << "{" << link.top_or_len << ", " << link.up << ", " << link.down << ", " << link.multiplier << "},";
}

bool operator==( const Pokemon_links::Type_name& lhs, const Pokemon_links::Type_name& rhs )
{
  return lhs.name == rhs.name && lhs.left == rhs.left && lhs.right == rhs.right;
}

bool operator!=( const Pokemon_links::Type_name& lhs, const Pokemon_links::Type_name& rhs )
{
  return !( lhs == rhs );
}

std::ostream& operator<<( std::ostream& os, const Pokemon_links::Type_name& type )
{
  return os << "{ name: " << type.name << ", left: " << type.left << ", right: " << type.right << " }";
}

bool operator==( const std::vector<Pokemon_links::Poke_link>& lhs,
                 const std::vector<Pokemon_links::Poke_link>& rhs )
{
  if ( lhs.size() != rhs.size() ) {
    return false;
  }
  for ( uint64_t i = 0; i < lhs.size(); i++ ) {
    if ( lhs[i] != rhs[i] ) {
      return false;
    }
  }
  return true;
}

bool operator!=( const std::vector<Pokemon_links::Poke_link>& lhs,
                 const std::vector<Pokemon_links::Poke_link>& rhs )
{
  return !( lhs == rhs );
}

bool operator==( const std::vector<Pokemon_links::Type_name>& lhs,
                 const std::vector<Pokemon_links::Type_name>& rhs )
{
  if ( lhs.size() != rhs.size() ) {
    return false;
  }
  for ( uint64_t i = 0; i < lhs.size(); i++ ) {
    if ( lhs[i] != rhs[i] ) {
      return false;
    }
  }
  return true;
}

bool operator!=( const std::vector<Pokemon_links::Type_name>& lhs,
                 const std::vector<Pokemon_links::Type_name>& rhs )
{
  return !( lhs == rhs );
}

std::ostream& operator<<( std::ostream& os, const std::vector<Pokemon_links::Poke_link>& links )
{
  os << "DLX ARRAY\n";
  for ( const auto& ln : links ) {
    if ( ln.top_or_len < 0 ) {
      os << "\n";
    }
    os << "{" << ln.top_or_len << "," << ln.up << "," << ln.down << "," << ln.multiplier << "," << ln.tag << "},";
  }
  os << "\n";
  return os;
}

std::ostream& operator<<( std::ostream& os, const std::vector<Pokemon_links::Type_name>& items )
{
  os << "LOOKUP TABLE\n";
  for ( const auto& item : items ) {
    os << "{\"" << item.name << "\"," << item.left << "," << item.right << "},\n";
  }
  os << "\n";
  return os;
}

bool operator==( const Pokemon_links::Encoding_index& lhs, const Pokemon_links::Encoding_index& rhs )
{
  return lhs.name == rhs.name && lhs.index == rhs.index;
}

bool operator!=( const Pokemon_links::Encoding_index& lhs, const Pokemon_links::Encoding_index& rhs )
{
  return !( lhs == rhs );
}

std::ostream& operator<<( std::ostream& os, const Pokemon_links::Encoding_index& n_n )
{
  return os << "{\"" << n_n.name << "\"," << n_n.index << "}";
}

bool operator==( const std::vector<Pokemon_links::Encoding_index>& lhs,
                 const std::vector<Pokemon_links::Encoding_index>& rhs )
{
  if ( lhs.size() != rhs.size() ) {
    return false;
  }
  for ( uint64_t i = 0; i < lhs.size(); i++ ) {
    if ( lhs[i] != rhs[i] ) {
      return false;
    }
  }
  return true;
}

bool operator!=( const std::vector<Pokemon_links::Encoding_index>& lhs,
                 const std::vector<Pokemon_links::Encoding_index>& rhs )
{
  return !( lhs == rhs );
}

std::ostream& operator<<( std::ostream& os, const std::vector<Pokemon_links::Encoding_index>& n_n )
{
  for ( const auto& i : n_n ) {
    os << i;
  }
  return os << "\n";
}

/* * * * * * * * * * * * * * * * * * * * *    Parser Tests     * * * * * * * * * * * * * * * * * * * * * * * * * */

TEST( ParserTests, CheckMapParserLoadsInMapCorrectly )
{
  Pokemon_test expected = {
    .interactions {},
    .gen_map = {
      .network = {
        {"G1", {"G2", "G8"}},
        {"G2", {"G1", "G6"}},
        {"G3", {"G6", "G5"}},
        {"G4", {"G6", "G5"}},
        {"G5", {"G4", "G3"}},
        {"G6", {"G2", "G4", "G3"}},
        {"G7", {}},
        {"G8", {"G1", "E4"}},
        {"E4", {"G8"}},
      },
      .city_locations = {
        {"G1", Gui::Point( 3.0, 4.0 )},
        {"G2", Gui::Point( 8.0, 3.0 )},
        {"G3", Gui::Point( 8.0, 7.0 )},
        {"G4", Gui::Point( 6.0, 5.0 )},
        {"G5", Gui::Point( 6.0, 10.0 )},
        {"G6", Gui::Point( 8.0, 5.0 )},
        {"G7", Gui::Point( 3.0, 11.0 )},
        {"G8", Gui::Point( 3.0, 7.0 )},
        {"E4", Gui::Point( 1.0, 3.0 )},
      },
    },
  };
  std::ifstream kanto_map( "data/dst/Gen-1-Kanto.dst" );
  EXPECT_EQ( kanto_map.is_open(), true );
  Pokemon_test load_gen_1 = load_pokemon_generation( kanto_map );
  EXPECT_EQ( load_gen_1.gen_map.network, expected.gen_map.network );
  EXPECT_EQ( load_gen_1.gen_map.city_locations, expected.gen_map.city_locations );
}

TEST( ParserTests, CheckLoadingMapTypingWorksCorrectly )
{
  // Gen 1 only has 33 types but it would still be a huge map to hard code by hand.
  std::ifstream kanto_map( "data/dst/Gen-1-Kanto.dst" );
  EXPECT_EQ( kanto_map.is_open(), true );
  Pokemon_test load_gen_1 = load_pokemon_generation( kanto_map );
  // If our parser picks up all the items we should be on the right track.
  EXPECT_EQ( load_gen_1.interactions.size(), 33 );
  for ( const auto& [_, resistances] : load_gen_1.interactions ) {
    // And if our set of resistances is not empty the parser is picking up the nested data well.
    EXPECT_EQ( resistances.size() >= 2, true );
  }
}

TEST( ParserTests, LoadTwoSubsetsOfGyms )
{
  const std::string gen_1_map { "Gen-1-Kanto.dst" };
  const std::set<std::string> selected_gyms = { "G1", "G2" };
  const std::set<Dx::Type_encoding> g1_g2_attack = { Dx::Type_encoding( "Normal" ), Dx::Type_encoding( "Water" ) };
  const std::set<Dx::Type_encoding> g1_g2_defense
    = { Dx::Type_encoding( "Ground-Rock" ), Dx::Type_encoding( "Water" ) };

  EXPECT_EQ( load_selected_gyms_attacks( gen_1_map, selected_gyms ), g1_g2_attack );
  EXPECT_EQ( load_selected_gyms_defenses( gen_1_map, selected_gyms ), g1_g2_defense );
}

/* * * * * * * * * * *   Test the Type Encoding We Use To Represent Pokemon Types in Bits  * * * * * * * * * * * */

TEST( InternalTests, EasiestTypeEncodingLexographicallyIsBug )
{
  const uint32_t hex_type = 0x20000;
  const std::string_view str_type = "Bug";
  const Dx::Type_encoding code( str_type );
  EXPECT_EQ( hex_type, code.encoding() );
  EXPECT_EQ( str_type, code.decode_type().first );
  const std::string_view empty {};
  EXPECT_EQ( empty, code.decode_type().second );
}

TEST( InternalTests, TestTheNextSimplistDualTypeBugDark )
{
  const uint32_t hex_type = 0x30000;
  const std::string_view str_type = "Bug-Dark";
  const Dx::Type_encoding code( str_type );
  EXPECT_EQ( hex_type, code.encoding() );
  EXPECT_EQ( std::string_view( "Bug" ), code.decode_type().first );
  EXPECT_EQ( std::string_view( "Dark" ), code.decode_type().second );
}

TEST( InternalTests, TestForOffByOneErrorsWithFirstAndLastIndexTypeBugWater )
{
  const uint32_t hex_type = 0x20001;
  const std::string_view str_type = "Bug-Water";
  const Dx::Type_encoding code( str_type );
  EXPECT_EQ( hex_type, code.encoding() );
  EXPECT_EQ( std::string_view( "Bug" ), code.decode_type().first );
  EXPECT_EQ( std::string_view( "Water" ), code.decode_type().second );
}

TEST( InternalTests, TestEveryPossibleCombinationOfTypings )
{
  /* Not all of these type combinations exist yet in Pokemon and that's ok. It sounds like there
   * would be alot but it only comes out to 171 unique combinations. Unique here means that
   * types order does not matter so Water-Bug is the same as Bug-Water and is only counted once.
   */
  const uint64_t bug = 0x20000;
  const uint64_t table_size = Dx::Type_encoding::type_encoding_table.size();
  for ( uint64_t bit1 = bug, type1 = table_size - 1; bit1 != 0; bit1 >>= 1, type1-- ) {

    const std::string check_single_type( Dx::Type_encoding::type_encoding_table.at( type1 ) );
    const Dx::Type_encoding single_type_encoding( check_single_type );
    EXPECT_EQ( single_type_encoding.encoding(), bit1 );
    EXPECT_EQ( single_type_encoding.decode_type().first, check_single_type );
    EXPECT_EQ( single_type_encoding.decode_type().second, std::string_view {} );

    for ( uint64_t bit2 = bit1 >> 1, type2 = type1 - 1; bit2 != 0; bit2 >>= 1, type2-- ) {

      const auto t2 = std::string( Dx::Type_encoding::type_encoding_table.at( type2 ) );
      const auto check_dual_type = std::string( check_single_type ).append( "-" ).append( t2 );
      Dx::Type_encoding dual_type_encoding( check_dual_type );
      EXPECT_EQ( dual_type_encoding.encoding(), bit1 | bit2 );
      /* I discourage the use of methods that create heap strings whenever possible. I use
       * string_views internally to report back typing for human readability when requested
       * in a GUI via ostream. This way I only need to create one character "-" on the heap to
       * join the two string_views of the lookup table in the stream. I don't have a use case
       * for creating strings yet but will add it if needed.
       */
      std::ostringstream capture_type;
      capture_type << dual_type_encoding;
      const std::string dual_type_string( capture_type.str() );
      EXPECT_EQ( dual_type_string, check_dual_type );
    }
  }
}

TEST( InternalTests, CompareMyEncodingDecodingSpeed )
{

  /* I was curious to see the how stashing all possible encodings and decodings we could encounter
   * in hash tables would stack up against my current encoding method. As expected the hash table
   * performs consistently well, especially becuase both encoding directions are precomputed and
   * thus have the same lookup time. Run the test to see for yourself, but my encoding method
   * is definitely slower than the hashing method, but I think it is competitive coming in at
   * around twice as slow as hashing. Surprisingly, I make up for that loss in speed in the
   * decoding phase by halving the time it takes a hash table to decode a type encoding. So, this
   * helps balance out the impact of slower encoding. However, the major benefit is that the
   * information I need to perform my encoding and decoding is trivially small compared to the
   * hash maps that require 177 entries which consist of one string and one encoding. So each
   * map contains 177 strings and 177 Type_encodings for a total across two maps of 708 individual
   * pieces of information we need to store. In contrast, my method only needs an array size
   * variable and an array of 18 stack strings that the compiler will likely place in the
   * read-only memory segment, much less wasteful.
   */

  std::unordered_map<std::string, Dx::Type_encoding> encode_map;
  std::unordered_map<Dx::Type_encoding, std::string> decode_map;

  // Generate all possible unique type combinations and place them in the maps.
  const uint64_t bug = 0x20000;
  const uint64_t table_size = Dx::Type_encoding::type_encoding_table.size();
  for ( uint64_t bit1 = bug, type1 = table_size - 1; bit1 != 0; bit1 >>= 1, type1-- ) {
    const std::string check_single_type( Dx::Type_encoding::type_encoding_table.at( type1 ) );
    const Dx::Type_encoding single_type_encoding( check_single_type );
    encode_map.insert( { check_single_type, single_type_encoding } );
    decode_map.insert( { single_type_encoding, check_single_type } );
    for ( uint64_t bit2 = bit1 >> 1, type2 = type1 - 1; bit2 != 0; bit2 >>= 1, type2-- ) {
      const auto t2 = std::string( Dx::Type_encoding::type_encoding_table.at( type2 ) );
      const auto check_dual_type = std::string( check_single_type ).append( "-" ).append( t2 );
      const Dx::Type_encoding dual_type_encoding( check_dual_type );
      encode_map.insert( { check_dual_type, dual_type_encoding } );
      decode_map.insert( { dual_type_encoding, check_dual_type } );
    }
  }

  // Generate 1,000,000 random types to encode and store them so both techniques use same data.
  const int num_requests = 1000000;
  std::random_device rd;
  std::mt19937 gen( rd() );
  // Our first type will always be in the table and be a valid type index.
  std::uniform_int_distribution<> first_type( 0, 17 );
  // We will only access an index with our second type if it is less than the first.
  std::uniform_int_distribution<> second_type( 0, 17 );
  int single_type_total = 0;
  int dual_type_total = 0;

  std::vector<std::string> to_encode( num_requests );
  for ( int i = 0; i < num_requests; i++ ) {
    const int type1 = first_type( gen );
    const int type2 = second_type( gen );
    single_type_total++;
    std::string type( Dx::Type_encoding::type_encoding_table.at( type1 ) );
    // This ensures we get a decent amount of single and dual types into the mix.
    if ( type2 < type1 ) {
      std::string t2( Dx::Type_encoding::type_encoding_table.at( type2 ) );
      type += "-" + t2;
      dual_type_total++;
      single_type_total--;
    }
    to_encode[i] = type;
  }

  std::cerr << "\n";

  std::cerr << "Generated " << num_requests << " random types: " << single_type_total << " single types, "
            << dual_type_total << " dual types\n";

  std::cerr << "----------START TIMER SESSION-------------\n";

  // Time 1,000,000 encodings with both methods.

  std::vector<Dx::Type_encoding> type_encodings( to_encode.size() );
  const std::clock_t start = std::clock();
  for ( uint64_t i = 0; i < to_encode.size(); i++ ) {
    type_encodings[i] = Dx::Type_encoding( to_encode[i] );
  }
  const std::clock_t end = std::clock();
  std::cerr << "Type_encoding encode method(ms): "
            << 1000.0 * ( static_cast<double>( end - start ) ) / CLOCKS_PER_SEC << "\n";

  std::vector<Dx::Type_encoding> hash_encodings( num_requests );
  const std::clock_t start2 = std::clock();
  for ( uint64_t i = 0; i < to_encode.size(); i++ ) {
    hash_encodings[i] = encode_map[to_encode[i]];
  }
  const std::clock_t end2 = std::clock();
  std::cerr << "Hashing encode method(ms): " << 1000.0 * ( static_cast<double>( end2 - start2 ) ) / CLOCKS_PER_SEC
            << "\n";

  // Time 1,000,000 decodings with both methods.

  std::vector<std::pair<std::string_view, std::string_view>> type_decodings( type_encodings.size() );
  const std::clock_t start3 = std::clock();
  for ( uint64_t i = 0; i < type_decodings.size(); i++ ) {
    type_decodings[i] = type_encodings[i].decode_type();
  }
  const std::clock_t end3 = std::clock();
  std::cerr << "Type_encoding decoding method(ms): "
            << 1000.0 * ( static_cast<double>( end3 - start3 ) ) / CLOCKS_PER_SEC << "\n";

  std::vector<std::string_view> hash_decodings( hash_encodings.size() );
  const std::clock_t start4 = std::clock();
  for ( uint64_t i = 0; i < hash_decodings.size(); i++ ) {
    hash_decodings[i] = decode_map[hash_encodings[i]];
  }
  const std::clock_t end4 = std::clock();
  std::cerr << "Hash decoding method(ms): " << 1000.0 * ( static_cast<double>( end4 - start4 ) ) / CLOCKS_PER_SEC
            << "\n";

  std::cerr << "\n";

  EXPECT_EQ( to_encode.size(), type_encodings.size() );
  EXPECT_EQ( to_encode.size(), hash_encodings.size() );
  EXPECT_EQ( hash_encodings.size(), hash_decodings.size() );
  EXPECT_EQ( type_encodings.size(), type_decodings.size() );
}

/* * * * * * * * * * * * * * *      Tests Below This Point      * * * * * * * * * * * * * * * * * */

/* These type names completely crowd out our test cases when I construct the dlx grids in the test
 * making them hard to read. They stretch too far horizontally so I am creating these name codes
 * here that should only be used in this translation unit for readablity and convenience when
 * building tests. Refer here if terminology in the tests is confusing. Also, hovering over the
 * code in a test case in QT should show you the full constexpr for Resistances.
 */

namespace {

constexpr Dx::Multiplier em = Dx::emp;
constexpr Dx::Multiplier im = Dx::imm;
constexpr Dx::Multiplier f4 = Dx::f14;
constexpr Dx::Multiplier f2 = Dx::f12;
constexpr Dx::Multiplier nm = Dx::nrm;
constexpr Dx::Multiplier db = Dx::dbl;
constexpr Dx::Multiplier qd = Dx::qdr;

} // namespace

/* * * * * * * * * * * * * * * * * *   Defense Links Init   * * * * * * * * * * * * * * * * * * * */

TEST( InternalTests, InitializeSmallDefensiveLinks )
{
  /*
   *
   *          Fire   Normal    Water   <-Attack
   *  Ghost          x0.0              <-Defense
   *  Water   x0.5             x0.5
   *
   */
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types {
    { { "Ghost" }, { { { "Fire" }, nm }, { { "Normal" }, im }, { { "Water" }, nm } } },
    { { "Water" }, { { { "Fire" }, f2 }, { { "Normal" }, nm }, { { "Water" }, f2 } } },
  };

  const std::vector<Dx::Pokemon_links::Encoding_index> option_table
    = { { Dx::Type_encoding( "" ), 0 }, { { "Ghost" }, 4 }, { { "Water" }, 6 } };
  const std::vector<Dx::Pokemon_links::Type_name> item_table = {
    { { "" }, 3, 1 },
    { { "Fire" }, 0, 2 },
    { { "Normal" }, 1, 3 },
    { { "Water" }, 2, 0 },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx = {
    //   0                   1Fire               2Normal             3Water
    { 0, 0, 0, em, 0 },  { 1, 7, 7, em, 0 }, { 1, 5, 5, em, 0 }, { 1, 8, 8, em, 0 },    
    //  4Ghost               5Zero
    { -1, 0, 5, em, 0 }, { 2, 2, 2, im, 0 },
    //  6Water               7Half               8Half
    { -2, 5, 8, em, 0 }, { 1, 1, 1, f2, 0 }, { 3, 3, 3, f2, 0 }, 
    //     9
    { INT_MIN, 7, UINT64_MAX, em, 0 },
  };
  // clang-format on
  const Dx::Pokemon_links links( types, Dx::Pokemon_links::defense );
  EXPECT_EQ( option_table, links.option_table_ );
  EXPECT_EQ( item_table, links.item_table_ );
  EXPECT_EQ( dlx, links.links_ );
}

TEST( InternalTests, InitializeAWorldWhereThereAreOnlySingleTypes )
{
  /*
   *
   *            Electric  Fire  Grass  Ice   Normal  Water
   *  Dragon     x0.5     x0.5  x0.5                 x0.5
   *  Electric   x0.5
   *  Ghost                                  x0.0
   *  Ice                              x0.5
   *
   */
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types {
    {
      { "Dragon" },
      { { { "Normal" }, nm },
        { { "Fire" }, f2 },
        { { "Water" }, f2 },
        { { "Electric" }, f2 },
        { { "Grass" }, f2 },
        { { "Ice" }, db } },
    },
    {
      { "Electric" },
      { { { "Normal" }, nm },
        { { "Fire" }, nm },
        { { "Water" }, nm },
        { { "Electric" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm } },
    },
    {
      { "Ghost" },
      { { { "Normal" }, im },
        { { "Fire" }, nm },
        { { "Water" }, nm },
        { { "Electric" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, nm } },
    },
    {
      { "Ice" },
      { { { "Normal" }, nm },
        { { "Fire" }, nm },
        { { "Water" }, nm },
        { { "Electric" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, f2 } },
    },
  };

  const std::vector<Dx::Pokemon_links::Encoding_index> option_table
    = { { { "" }, 0 }, { { "Dragon" }, 7 }, { { "Electric" }, 12 }, { { "Ghost" }, 14 }, { { "Ice" }, 16 } };
  const std::vector<Dx::Pokemon_links::Type_name> item_table = {
    { { "" }, 6, 1 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 1, 3 },
    { { "Grass" }, 2, 4 },
    { { "Ice" }, 3, 5 },
    { { "Normal" }, 4, 6 },
    { { "Water" }, 5, 0 },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx = {
    //    0           1Electric       2Fire        3Grass          4Ice          5Normal        6Water
    {0,0,0,em,0},   {2,13,8,em,0}, {1,9,9,em,0}, {1,10,10,em,0},{1,17,17,em,0},{1,15,15,em,0},{1,11,11,em,0},
    //  7Dragon        8half          9half        10half                                       11half
    {-1,0,11,em,0}, {1,1,13,f2,0}, {2,2,2,f2,0}, {3,3,3,f2,0},                                {6,6,6,f2,0}, 
    //  12Electric     13half
    {-2,8,13,em,0}, {1,8,1,f2,0},   
    //  14Ghost                                                                  15immune
    {-3,13,15,em,0},                                                           {5,5,5,im,0},    
    //  16Ice                                                     17half
    {-4,15,17,em,0},                                           {4,4,4,f2,0},    
    //  18
    { INT_MIN, 17, UINT64_MAX, em, 0 },
  };
  // clang-format on
  const Dx::Pokemon_links links( types, Dx::Pokemon_links::defense );
  EXPECT_EQ( option_table, links.option_table_ );
  EXPECT_EQ( item_table, links.item_table_ );
  EXPECT_EQ( dlx, links.links_ );
}

/* * * * * * * * * * * * * * * *   Defense Links Cover/Uncover      * * * * * * * * * * * * * * * */

TEST( InternalTests, CoverElectricWithDragonEliminatesElectricOptionUncoverResets )
{
  /*
   *
   *            Electric  Fire  Grass  Ice   Normal  Water
   *  Dragon     x0.5     x0.5  x0.5                 x0.5
   *  Electric   x0.5
   *  Ghost                                  x0.0
   *  Ice                              x0.5
   *
   */
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types {
    {
      { "Dragon" },
      { { { "Normal" }, nm },
        { { "Fire" }, f2 },
        { { "Water" }, f2 },
        { { "Electric" }, f2 },
        { { "Grass" }, f2 },
        { { "Ice" }, db } },
    },
    {
      { "Electric" },
      { { { "Normal" }, nm },
        { { "Fire" }, nm },
        { { "Water" }, nm },
        { { "Electric" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm } },
    },
    {
      { "Ghost" },
      { { { "Normal" }, im },
        { { "Fire" }, nm },
        { { "Water" }, nm },
        { { "Electric" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, nm } },
    },
    {
      { "Ice" },
      { { { "Normal" }, nm },
        { { "Fire" }, nm },
        { { "Water" }, nm },
        { { "Electric" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, f2 } },
    },
  };

  const std::vector<Dx::Pokemon_links::Encoding_index> option_table
    = { { { "" }, 0 }, { { "Dragon" }, 7 }, { { "Electric" }, 12 }, { { "Ghost" }, 14 }, { { "Ice" }, 16 } };
  const std::vector<Dx::Pokemon_links::Type_name> item_table = {
    { { "" }, 6, 1 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 1, 3 },
    { { "Grass" }, 2, 4 },
    { { "Ice" }, 3, 5 },
    { { "Normal" }, 4, 6 },
    { { "Water" }, 5, 0 },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx = {
    //   0             1Electric       2Fire       3Grass          4Ice          5Normal          6Water
    {0,0,0,em,0},   {2,13,8,em,0}, {1,9,9,em,0}, {1,10,10,em,0}, {1,17,17,em,0},{1,15,15,em,0}, {1,11,11,em,0},
    //   7Dragon       8half          9half       10half                                          11half
    {-1,0,11,em,0}, {1,1,13,f2,0}, {2,2,2,f2,0},  {3,3,3,f2,0},                                 {6,6,6,f2,0},
    //   12Electric    13half
    {-2,8,13,em,0}, {1,8,1,f2,0},
    //   14Ghost                                                                 15immune
    {-3,13,15,em,0},                                                            {5,5,5,im,0},
    //   16Ice                                                    17half
    {-4,15,17,em,0},                                             {4,4,4,f2,0},    
    //   18
    {INT_MIN,17,UINT64_MAX,em,0},
  };
  // clang-format on
  Dx::Pokemon_links links( types, Dx::Pokemon_links::defense );
  EXPECT_EQ( option_table, links.option_table_ );
  EXPECT_EQ( item_table, links.item_table_ );
  EXPECT_EQ( dlx, links.links_ );

  const std::vector<Dx::Pokemon_links::Type_name> item_cover_electric = {
    { { "" }, 5, 4 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 0, 3 },
    { { "Grass" }, 0, 4 },
    { { "Ice" }, 0, 5 },
    { { "Normal" }, 4, 0 },
    { { "Water" }, 5, 0 },
  };
  // clang-format off
  /*
   *             Ice   Normal
   *  Ghost             x0.0
   *  Ice        x0.5
   *
   */
  const std::vector<Dx::Pokemon_links::Poke_link> dlx_cover_electric = {
    //  0             1Electric       2Fire        3Grass           4Ice           5Normal          6Water
    {0,0,0,em,0},    {2,13,8,em,0}, {1,9,9,em,0}, {1,10,10,em,0}, {1,17,17,em,0}, {1,15,15,em,0}, {1,11,11,em,0}, 
    //  7Dragon       8half          9half         10half                                          11half
    {-1,0,11,em,0},  {1,1,13,f2,0}, {2,2,2,f2,0}, {3,3,3,f2,0},                                   {6,6,6,f2,0}, 
    //  12Electric    13half
    {-2,8,13,em,0},  {1,8,1,f2,0}, 
    //  14Ghost                                                                    15immune
    {-3,13,15,em,0},                                                              {5,5,5,im,0},
    //  16Ice                                                       17half
    {-4,15,17,em,0},                                              {4,4,4,f2,0}, 
    //  18
    {INT_MIN,17,UINT64_MAX,em,0},
  };
  // clang-format on
  Dx::Pokemon_links::Encoding_score pick = links.cover_type( 8 );
  EXPECT_EQ( pick.score, 12 );
  EXPECT_EQ( pick.name.decode_type().first, "Dragon" );
  EXPECT_EQ( item_cover_electric, links.item_table_ );
  EXPECT_EQ( dlx_cover_electric, links.links_ );
  links.uncover_type( 8 );
  EXPECT_EQ( item_table, links.item_table_ );
  EXPECT_EQ( dlx, links.links_ );
}

TEST( InternalTests, CoverElectricWithElectricToCauseHidingOfManyOptions )
{
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
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types {
    { { "Electric" },
      { { { "Electric" }, f2 },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, nm } } },
    {
      { "Fire" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, f2 },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } },
    },
    {
      { "Grass" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } },
    },
    {
      { "Ice" },
      { { { "Electric" }, nm },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, f2 },
        { { "Normal" }, nm },
        { { "Water" }, f2 } },
    },
    {
      { "Normal" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, f2 },
        { { "Water" }, nm } },
    },
    {
      { "Water" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } },
    },
  };
  Dx::Pokemon_links links( types, Dx::Pokemon_links::defense );
  const std::vector<Dx::Pokemon_links::Type_name> headers = {
    { { "" }, 6, 1 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 1, 3 },
    { { "Grass" }, 2, 4 },
    { { "Ice" }, 3, 5 },
    { { "Normal" }, 4, 6 },
    { { "Water" }, 5, 0 },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx = {
    // 0              1Electric      2Fire          3Grass          4Ice            5Normal        6Water
    {0,0,0,em,0},   {3,21,8,em,0}, {3,24,9,em,0}, {1,12,12,em,0}, {1,18,18,em,0}, {1,22,22,em,0}, {4,25,13,em,0},    
    // 7Electric       8              9
    {-1,0,9,em,0},  {1,1,11,f2,0}, {2,2,15,f2,0},    
    // 10Fire         11                               12                                           13
    {-2,8,13,em,0}, {1,8,21,f2,0},                {3,3,3,f2,0},                                   {6,6,16,f2,0},    
    // 14Grass                       15                                                              16
    {-3,11,16,em,0},               {2,9,24,f2,0},                                                 {6,13,19,f2,0},    
    // 17Ice                                                        18                               19
    {-4,15,19,em,0},                                              {4,4,4,f2,0},                   {6,16,25,f2,0},    
    // 20Normal       21                                                             22
    {-5,18,22,em,0},{1,11,1,f2,0},                                                {5,5,5,f2,0},    
    // 23Water                       24                                                               25
    {-6,21,25,em,0},               {2,15,2,f2,0},                                                 {6,19,6,f2,0},
    {INT_MIN,24,UINT64_MAX,em,0},  
  };
  // clang-format on
  EXPECT_EQ( headers, links.item_table_ );
  EXPECT_EQ( dlx, links.links_ );

  const std::vector<Dx::Pokemon_links::Type_name> headers_cover_electric = {
    { { "" }, 6, 3 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 0, 3 },
    { { "Grass" }, 0, 4 },
    { { "Ice" }, 3, 5 },
    { { "Normal" }, 4, 6 },
    { { "Water" }, 5, 0 },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx_cover_electric = {
    /*
     *
     *        Grass   Ice    Normal  Water
     *  Ice           x0.5           x0.5
     *
     *
     */
    // 0              1Electric       2Fire           3Grass        4Ice           5Normal        6Water
    {0,0,0,em,0},    {3,21,8,em,0}, {3,24,9,em,0}, {0,3,3,em,0}, {1,18,18,em,0}, {0,5,5,em,0}, {1,19,19,em,0}, 
    // 7Electric        8             9
    {-1,0,9,em,0},   {1,1,11,f2,0}, {2,2,15,f2,0}, 
    // 10Fire           11                            12                                           13
    {-2,8,13,em,0},  {1,8,21,f2,0},                {3,3,3,f2,0},                               {6,6,16,f2,0}, 
    // 14Grass                        15                                                           16
    {-3,11,16,em,0},                {2,9,24,f2,0},                                             {6,6,19,f2,0}, 
    // 17Ice                                                        18                            19
    {-4,15,19,em,0},                                             {4,4,4,f2,0},                 {6,6,6,f2,0}, 
    // 20Normal        21                                                            22
    {-5,18,22,em,0}, {1,11,1,f2,0},                                               {5,5,5,f2,0}, 
    // 23Water                        24                                                            25
    {-6,21,25,em,0},                {2,15,2,f2,0},                                             {6,19,6,f2,0},
    {INT_MIN,24,UINT64_MAX,em,0},
  };
  // clang-format on
  const Dx::Pokemon_links::Encoding_score pick = links.cover_type( 8 );
  EXPECT_EQ( pick.score, 6 );
  EXPECT_EQ( pick.name.decode_type().first, "Electric" );
  EXPECT_EQ( headers_cover_electric, links.item_table_ );
  EXPECT_EQ( dlx_cover_electric, links.links_ );

  links.uncover_type( 8 );
  EXPECT_EQ( headers, links.item_table_ );
  EXPECT_EQ( dlx, links.links_ );
}

/* * * * * * * * * * * * *      Solve the Defensive Cover Problem       * * * * * * * * * * * * * */

TEST( InternalTests, ThereAreTwoExactCoversForThisTypingCombo )
{
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
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types {
    {
      { "Electric" },
      { { { "Electric" }, f2 }, { { "Grass" }, nm }, { { "Ice" }, nm }, { { "Normal" }, nm }, { { "Water" }, nm } },
    },
    {
      { "Ghost" },
      { { { "Electric" }, nm }, { { "Grass" }, nm }, { { "Ice" }, nm }, { { "Normal" }, im }, { { "Water" }, nm } },
    },
    {
      { "Ground" },
      { { { "Electric" }, im }, { { "Grass" }, nm }, { { "Ice" }, nm }, { { "Normal" }, nm }, { { "Water" }, nm } },
    },
    {
      { "Ice" },
      { { { "Electric" }, nm }, { { "Grass" }, nm }, { { "Ice" }, f2 }, { { "Normal" }, nm }, { { "Water" }, nm } },
    },
    {
      { "Poison" },
      { { { "Electric" }, nm }, { { "Grass" }, f2 }, { { "Ice" }, nm }, { { "Normal" }, nm }, { { "Water" }, nm } },
    },
    {
      { "Water" },
      { { { "Electric" }, nm }, { { "Grass" }, db }, { { "Ice" }, f2 }, { { "Normal" }, nm }, { { "Water" }, f2 } },
    },
  };
  Dx::Pokemon_links links( types, Dx::Pokemon_links::defense );
  const std::set<Ranked_set<Dx::Type_encoding>> correct
    = { { 11, { { "Ghost" }, { "Ground" }, { "Poison" }, { "Water" } } },
        { 13, { { "Electric" }, { "Ghost" }, { "Poison" }, { "Water" } } } };
  EXPECT_EQ( links.get_exact_coverages( 6 ), correct );
}

TEST( InternalTests, ThereIsOneExactAndAFewOverlappingCoversHereExactCoverFirst )
{
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
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types = {
    /* In reality maps will have every type present in every key. But I know the internals
     * of my implementation and will just enter all types for the first key to make entering
     * the rest of the test cases easier.
     */
    {
      { "Bug-Ghost" },
      {
        { { "Electric" }, nm },
        { { "Fire" }, nm },
        { { "Grass" }, f2 },
        { { "Ice" }, nm },
        { { "Normal" }, im },
        { { "Water" }, nm },
      },
    },
    {
      { "Electric-Grass" },
      {
        { { "Electric" }, f4 },
        { { "Grass" }, f2 },
        { { "Water" }, f2 },
      },
    },
    {
      { "Fire-Flying" },
      {
        { { "Fire" }, f2 },
        { { "Grass" }, f4 },
      },
    },
    {
      { "Ground-Water" },
      {
        { { "Electric" }, im },
        { { "Fire" }, f2 },
      },
    },
    {
      { "Ice-Psychic" },
      {
        { { "Ice" }, f2 },
      },
    },
    {
      { "Ice-Water" },
      {
        { { "Ice" }, f4 },
        { { "Water" }, f2 },
      },
    },
  };
  const std::vector<Dx::Pokemon_links::Encoding_index> options = {
    { { "" }, 0 },
    { { "Bug-Ghost" }, 7 },
    { { "Electric-Grass" }, 10 },
    { { "Fire-Flying" }, 14 },
    { { "Ground-Water" }, 17 },
    { { "Ice-Psychic" }, 20 },
    { { "Ice-Water" }, 22 },
  };
  const std::vector<Dx::Pokemon_links::Type_name> items = {
    { { "" }, 6, 1 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 1, 3 },
    { { "Grass" }, 2, 4 },
    { { "Ice" }, 3, 5 },
    { { "Normal" }, 4, 6 },
    { { "Water" }, 5, 0 },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx = {
    // 0                 1Electric       2Fire          3Grass         4Ice          5Normal       6Water
    {0,0,0,em,0},    {2,18,11,em,0}, {2,19,15,em,0}, {3,16,8,em,0}, {2,23,21,em,0}, {1,9,9,em,0}, {2,24,13,em,0}, 
    // 7Bug-Ghost                                         8                            9
    {-1,0,9,em,0},                                   {3,3,12,f2,0},                 {5,5,5,im,0}, 
    // 10Electric-Grass   11                             12                                         13
    {-2,8,13,em,0},  {1,1,18,f4,0},                  {3,8,16,f2,0},                               {6,6,24,f2,0}, 
    // 14Fire-Flying                      15             16
    {-3,11,16,em,0},                 {2,2,19,f2,0},  {3,12,3,f4,0}, 
    // 17Ground-Water       18            19
    {-4,15,19,em,0}, {1,11,1,im,0},  {2,15,2,f2,0}, 
    // 20Ice-Psychic                                                     21
    {-5,18,21,em,0},                                                {4,4,23,f2,0}, 
    // 22Ice-Water                                                       23                          24
    {-6,21,24,em,0},                                                {4,21,4,f4,0},                {6,13,6,f2,0}, 
    // 25
    {INT_MIN,23,UINT64_MAX,em,0},
  };
  // clang-format on
  Dx::Pokemon_links links( types, Dx::Pokemon_links::defense );
  EXPECT_EQ( links.option_table_, options );
  EXPECT_EQ( links.item_table_, items );
  EXPECT_EQ( links.links_, dlx );
  const std::set<Ranked_set<Dx::Type_encoding>> result = links.get_exact_coverages( 6 );
  const std::set<Ranked_set<Dx::Type_encoding>> correct
    = { { 13, { { "Bug-Ghost" }, { "Ground-Water" }, { "Ice-Water" } } } };
  EXPECT_EQ( correct, result );
}

TEST( InternalTests, AllAlgorithmsThatOperateOnTheseLinksShouldCleanupAndRestoreStateAfter )
{
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
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types = {
    /* In reality maps will have every type present in every key. But I know the internals
     * of my implementation and will just enter all types for the first key to make entering
     * the rest of the test cases easier.
     */
    {
      { "Bug-Ghost" },
      { { { "Electric" }, nm },
        { { "Fire" }, nm },
        { { "Grass" }, f2 },
        { { "Ice" }, nm },
        { { "Normal" }, im },
        { { "Water" }, nm } },
    },
    {
      { "Electric-Grass" },
      { { { "Electric" }, f4 }, { { "Grass" }, f2 }, { { "Water" }, f2 } },
    },
    {
      { "Fire-Flying" },
      { { { "Fire" }, f2 }, { { "Grass" }, f4 } },
    },
    {
      { "Ground-Water" },
      { { { "Electric" }, im }, { { "Fire" }, f2 } },
    },
    {
      { "Ice-Psychic" },
      { { { "Ice" }, f2 } },
    },
    {
      { "Ice-Water" },
      { { { "Ice" }, f4 }, { { "Water" }, f2 } },
    },
  };
  const std::vector<Dx::Pokemon_links::Encoding_index> options = {
    { { "" }, 0 },
    { { "Bug-Ghost" }, 7 },
    { { "Electric-Grass" }, 10 },
    { { "Fire-Flying" }, 14 },
    { { "Ground-Water" }, 17 },
    { { "Ice-Psychic" }, 20 },
    { { "Ice-Water" }, 22 },
  };
  const std::vector<Dx::Pokemon_links::Type_name> items = {
    { { "" }, 6, 1 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 1, 3 },
    { { "Grass" }, 2, 4 },
    { { "Ice" }, 3, 5 },
    { { "Normal" }, 4, 6 },
    { { "Water" }, 5, 0 },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx = {
    // 0                1Electric       2Fire          3Grass         4Ice            5Normal       6Water
    {0,0,0,em,0},    {2,18,11,em,0}, {2,19,15,em,0}, {3,16,8,em,0}, {2,23,21,em,0}, {1,9,9,em,0}, {2,24,13,em,0}, 
    // 7Bug-Ghost                                         8                            9
    {-1,0,9,em,0},                                   {3,3,12,f2,0},                 {5,5,5,im,0}, 
    // 10Electric-Grass     11                            12                                         13
    {-2,8,13,em,0},  {1,1,18,f4,0},                  {3,8,16,f2,0},                               {6,6,24,f2,0}, 
    // 14Fire-Flying                       15             16
    {-3,11,16,em,0},                 {2,2,19,f2,0},  {3,12,3,f4,0}, 
    // 17Ground-Water       18             19
    {-4,15,19,em,0}, {1,11,1,im,0},  {2,15,2,f2,0}, 
    // 20Ice-Psychic                                                     21
    {-5,18,21,em,0},                                                {4,4,23,f2,0}, 
    // 22Ice-Water                                                       23                          24
    {-6,21,24,em,0},                                                {4,21,4,f4,0},                {6,13,6,f2,0}, 
    // 25
    { INT_MIN, 23, UINT64_MAX, em, 0 },
  };
  // clang-format on
  Dx::Pokemon_links links( types, Dx::Pokemon_links::defense );
  EXPECT_EQ( links.option_table_, options );
  EXPECT_EQ( links.item_table_, items );
  EXPECT_EQ( links.links_, dlx );
  const std::set<Ranked_set<Dx::Type_encoding>> result = links.get_exact_coverages( 6 );
  const std::set<Ranked_set<Dx::Type_encoding>> correct
    = { { 13, { { "Bug-Ghost" }, { "Ground-Water" }, { "Ice-Water" } } } };
  EXPECT_EQ( correct, result );
  // Did we cleanup correctly when the algorithm was done?
  EXPECT_EQ( links.option_table_, options );
  EXPECT_EQ( links.item_table_, items );
  EXPECT_EQ( links.links_, dlx );
}

/* * * * * * * * * * * * * * * * * *   Attack Links Init    * * * * * * * * * * * * * * * * * * * */

/* The good news about this section is that we only have to test that we can correctly initialize
 * the network by inverting the attack types and defense types. Then, the algorithm runs
 * identically and we can use the same functions for this problem.
 */

TEST( InternalTests, InitializationOfAttackDancingLinks )
{
  /*
   *
   *                    Fire-Flying   Ground-Grass   Ground-Rock   <-Defense
   *         Electric       2X
   *         Fire                         2x
   *         Water          2x                         4x          <-Attack
   *
   */
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types {
    { { "Ground-Rock" }, { { { "Electric" }, im }, { { "Fire" }, nm }, { { "Water" }, qd } } },
    { { "Ground-Grass" }, { { { "Electric" }, im }, { { "Fire" }, db }, { { "Water" }, nm } } },
    { { "Fire-Flying" }, { { { "Electric" }, db }, { { "Fire" }, f2 }, { { "Water" }, db } } },
  };
  const std::vector<Dx::Pokemon_links::Encoding_index> option_table
    = { { { "" }, 0 }, { { "Electric" }, 4 }, { { "Fire" }, 6 }, { { "Water" }, 8 } };
  const std::vector<Dx::Pokemon_links::Type_name> item_table = {
    { { "" }, 3, 1 },
    { { "Fire-Flying" }, 0, 2 },
    { { "Ground-Grass" }, 1, 3 },
    { { "Ground-Rock" }, 2, 0 },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx {
    // 0           1Fire-Flying   2Ground-Grass   3Ground-Rock
    {0,0,0,em,0},  {2,9,5,em,0},  {1,7,7,em,0},  {1,10,10,em,0},  
    // 4Electric     5Double
    {-1,0,5,em,0}, {1,1,9,db,0},  
    // 6Fire                        7Double
    {-2,5,7,em,0},                {2,2,2,db,0},  
    // 8Water        9Double                       10Quadru
    {-3,7,10,em,0},{1,5,1,db,0},                  {3,3,3,qd,0},
    {INT_MIN,9,UINT64_MAX,em,0},
  };
  // clang-format on
  const Dx::Pokemon_links links( types, Dx::Pokemon_links::attack );
  EXPECT_EQ( links.option_table_, option_table );
  EXPECT_EQ( links.item_table_, item_table );
  EXPECT_EQ( links.links_, dlx );
}

TEST( InternalTests, AtLeastTestThatWeCanRecognizeASuccessfulAttackCoverage )
{
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
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types = {
    { { "Electric" }, { { { "Ground" }, db } } },
    { { "Fire" }, { { { "Ground" }, db } } },
    { { "Grass" }, { { { "Ice" }, db }, { { "Poison" }, db } } },
    { { "Ice" }, { { { "Fighting" }, db } } },
    { { "Normal" }, { { { "Fighting" }, db } } },
    { { "Water" }, { { { "Grass" }, db } } },
  };
  const std::set<Ranked_set<Dx::Type_encoding>> solutions
    = { { 30, { { "Fighting" }, { "Grass" }, { "Ground" }, { "Ice" } } },
        { 30, { { "Fighting" }, { "Grass" }, { "Ground" }, { "Poison" } } } };
  Dx::Pokemon_links links( types, Dx::Pokemon_links::attack );
  EXPECT_EQ( links.get_exact_coverages( 24 ), solutions );
}

TEST( InternalTests, ThereIsOneExactCoverHere )
{
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
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types = {
    {
      { "Bug-Ghost" },
      { { { "Electric" }, nm },
        { { "Fire" }, db },
        { { "Grass" }, f2 },
        { { "Ice" }, nm },
        { { "Normal" }, im },
        { { "Water" }, nm } },
    },
    {
      { "Electric-Grass" },
      { { { "Electric" }, f4 },
        { { "Fire" }, db },
        { { "Grass" }, f2 },
        { { "Ice" }, f2 },
        { { "Normal" }, nm },
        { { "Water" }, f2 } },
    },
    {
      { "Fire-Flying" },
      { { { "Electric" }, db },
        { { "Fire" }, f2 },
        { { "Grass" }, f4 },
        { { "Ice" }, f2 },
        { { "Normal" }, nm },
        { { "Water" }, db } },
    },
    {
      { "Ground-Water" },
      { { { "Electric" }, im },
        { { "Fire" }, f2 },
        { { "Grass" }, qd },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, nm } },
    },
    {
      { "Ice-Psychic" },
      { { { "Electric" }, nm },
        { { "Fire" }, db },
        { { "Grass" }, nm },
        { { "Ice" }, f2 },
        { { "Normal" }, nm },
        { { "Water" }, nm } },
    },
    {
      { "Ice-Water" },
      { { { "Electric" }, db },
        { { "Fire" }, nm },
        { { "Grass" }, db },
        { { "Ice" }, f2 },
        { { "Normal" }, nm },
        { { "Water" }, f2 } },
    },
  };
  Dx::Pokemon_links links( types, Dx::Pokemon_links::attack );
  const std::set<Ranked_set<Dx::Type_encoding>> result = links.get_exact_coverages( 24 );
  const std::set<Ranked_set<Dx::Type_encoding>> correct = { { 31, { { "Fire" }, { "Grass" }, { "Water" } } } };
  EXPECT_EQ( result, correct );
}

/* * * * * * * * * * *    Finding a Weak Coverage that Allows Overlap     * * * * * * * * * * * * */

TEST( InternalTests, TestTheDepthTagApproachToOverlappingCoverage )
{
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
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types {
    {
      { "Electric" },
      { { { "Electric" }, f2 },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, nm } },
    },
    {
      { "Fire" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, f2 },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } },
    },
    {
      { "Grass" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } },
    },
    {
      { "Ice" },
      { { { "Electric" }, nm },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, f2 },
        { { "Normal" }, nm },
        { { "Water" }, f2 } },
    },
    {
      { "Normal" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, f2 },
        { { "Water" }, nm } },
    },
    {
      { "Water" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } },
    },
  };

  const std::vector<Dx::Pokemon_links::Type_name> headers {
    { { "" }, 6, 1 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 1, 3 },
    { { "Grass" }, 2, 4 },
    { { "Ice" }, 3, 5 },
    { { "Normal" }, 4, 6 },
    { { "Water" }, 5, 0 },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx = {
    //  0              1Electric       2Fire           3Grass        4Ice           5Normal          6Water
    {0,0,0,em,0},    {3,21,8,em,0}, {3,24,9,em,0}, {1,12,12,em,0}, {1,18,18,em,0}, {1,22,22,em,0}, {4,25,13,em,0}, 
    // 7Electric          8             9
    {-1,0,9,em,0},   {1,1,11,f2,0}, {2,2,15,f2,0}, 
    // 10Fire         11                               12                                            13
    {-2,8,13,em,0},  {1,8,21,f2,0},                {3,3,3,f2,0},                                   {6,6,16,f2,0}, 
    // 14Grass                          15                                                           16
    {-3,11,16,em,0},                {2,9,24,f2,0},                                                 {6,13,19,f2,0}, 
    // 17Ice                                                          18                            19
    {-4,15,19,em,0},                                               {4,4,4,f2,0},                   {6,16,25,f2,0}, 
    // 20Normal          21                                                           22
    {-5,18,22,em,0}, {1,11,1,f2,0},                                                {5,5,5,f2,0}, 
    // 23Water                         24                                                            25
    {-6,21,25,em,0},                {2,15,2,f2,0},                                                 {6,19,6,f2,0}, 
    //       26
    { INT_MIN, 24, UINT64_MAX, em, 0 },
  };
  // clang-format on
  Dx::Pokemon_links links( types, Dx::Pokemon_links::defense );
  EXPECT_EQ( links.links_, dlx );
  EXPECT_EQ( links.item_table_, headers );

  const Dx::Pokemon_links::Encoding_score choice = links.overlapping_cover_type( { 8, 6 } );
  EXPECT_EQ( choice.score, 6 );
  EXPECT_EQ( choice.name.decode_type().first, "Electric" );
  const std::vector<Dx::Pokemon_links::Type_name> headers_cover_electric {
    { { "" }, 6, 3 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 0, 3 },
    { { "Grass" }, 0, 4 },
    { { "Ice" }, 3, 5 },
    { { "Normal" }, 4, 6 },
    { { "Water" }, 5, 0 },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx_cover_electric = {
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
    // 0              1Electric        2Fire          3Grass         4Ice            5Normal        6Water
    {0,0,0,em,0},    {3,21,8,em,6}, {3,24,9,em,6}, {1,12,12,em,0}, {1,18,18,em,0}, {1,22,22,em,0}, {4,25,13,em,0}, 
    // 7Electric         8              9
    {-1,0,9,em,0},   {1,1,11,f2,6}, {2,2,15,f2,6}, 
    // 10Fire            11                          12                                               13
    {-2,8,13,em,0},  {1,8,21,f2,0},                {3,3,3,f2,0},                                   {6,6,16,f2,0}, 
    // 14Grass                        15                                                             16
    {-3,11,16,em,0},                {2,9,24,f2,0},                                                 {6,13,19,f2,0}, 
    // 17Ice                                                          18                             19
    {-4,15,19,em,0},                                               {4,4,4,f2,0},                   {6,16,25,f2,0}, 
    // 20Normal       21                                                              22
    {-5,18,22,em,0}, {1,11,1,f2,0},                                                 {5,5,5,f2,0}, 
    // 23Water                         24                                                            25
    {-6,21,25,em,0},                {2,15,2,f2,0},                                                 {6,19,6,f2,0}, 
    // 26
    { INT_MIN, 24, UINT64_MAX, em, 0 },
  };
  // clang-format on
  EXPECT_EQ( links.item_table_, headers_cover_electric );
  EXPECT_EQ( links.links_, dlx_cover_electric );

  const Dx::Pokemon_links::Encoding_score choice2 = links.overlapping_cover_type( { 12, 5 } );
  EXPECT_EQ( choice2.score, 6 );
  EXPECT_EQ( choice2.name.decode_type().first, "Fire" );
  const std::vector<Dx::Pokemon_links::Type_name> headers_cover_grass {
    { { "" }, 5, 4 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 0, 3 },
    { { "Grass" }, 0, 4 },
    { { "Ice" }, 0, 5 },
    { { "Normal" }, 4, 0 },
    { { "Water" }, 5, 0 },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx_cover_grass = {
    /*
     *
     *            Ice   Normal
     *  Grass
     *  Ice       x0.5
     *  Normal          x0.5
     *  Water
     *
     */
    // 0              1Electric         2Fire           3Grass         4Ice          5Normal        6Water
    {0,0,0,em,0},    {3,21,8,em,6}, {3,24,9,em,6}, {1,12,12,em,5}, {1,18,18,em,0}, {1,22,22,em,0}, {4,25,13,em,5}, 
    // 7Electric         8             9
    {-1,0,9,em,0},   {1,1,11,f2,6}, {2,2,15,f2,6}, 
    // 10Fire           11                             12                                             13
    {-2,8,13,em,0},  {1,8,21,f2,5},                {3,3,3,f2,5},                                   {6,6,16,f2,5}, 
    // 14Grass                         15                                                             16
    {-3,11,16,em,0},                {2,9,24,f2,0},                                                 {6,13,19,f2,0}, 
    // 17Ice                                                          18                              19
    {-4,15,19,em,0},                                               {4,4,4,f2,0},                   {6,16,25,f2,0}, 
    // 20Normal         21                                                             22
    {-5,18,22,em,0}, {1,11,1,f2,0},                                                {5,5,5,f2,0}, 
    // 23Water                          24                                                             25
    {-6,21,25,em,0},                {2,15,2,f2,0},                                                 {6,19,6,f2,0}, 
    // 26
    { INT_MIN, 24, UINT64_MAX, em, 0 },
  };
  // clang-format on
  EXPECT_EQ( links.item_table_, headers_cover_grass );
  EXPECT_EQ( links.links_, dlx_cover_grass );
  links.overlapping_uncover_type( 12 );
  EXPECT_EQ( links.item_table_, headers_cover_electric );
  EXPECT_EQ( links.links_, dlx_cover_electric );
  links.overlapping_uncover_type( 8 );
  EXPECT_EQ( links.links_, dlx );
  EXPECT_EQ( links.item_table_, headers );
}

TEST( InternalTests, OverlappingAllowsTwoTypesToCoverSameOpposingTypeIEFireAndElectric )
{
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
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types {
    { { "Electric" },
      { { { "Electric" }, f2 },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, nm } } },
    { { "Fire" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, f2 },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Grass" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Ice" },
      { { { "Electric" }, nm },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, f2 },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Normal" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, f2 },
        { { "Water" }, nm } } },
    { { "Water" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
  };
  Dx::Pokemon_links links( types, Dx::Pokemon_links::defense );
  const std::set<Ranked_set<Dx::Type_encoding>> result = links.get_overlapping_coverages( 6 );
  const std::set<Ranked_set<Dx::Type_encoding>> correct
    = { { 18, { { "Electric" }, { "Fire" }, { "Ice" }, { "Normal" } } },
        { 18, { { "Fire" }, { "Grass" }, { "Ice" }, { "Normal" } } },
        { 18, { { "Fire" }, { "Ice" }, { "Normal" }, { "Water" } } } };
  EXPECT_EQ( correct, result );
}

TEST( InternalTests, ThereAreAFewOverlappingCoversHere )
{
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
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types = {
    /* In reality maps will have every type present in every key. But I know the internals
     * of my implementation and will just enter all types for the first key to make entering
     * the rest of the test cases easier.
     */
    { { "Bug-Ghost" },
      { { { "Electric" }, nm },
        { { "Fire" }, nm },
        { { "Grass" }, f2 },
        { { "Ice" }, nm },
        { { "Normal" }, im },
        { { "Water" }, nm } } },
    { { "Electric-Grass" }, { { { "Electric" }, f4 }, { { "Grass" }, f2 }, { { "Water" }, f2 } } },
    { { "Fire-Flying" }, { { { "Fire" }, f2 }, { { "Grass" }, f4 } } },
    { { "Ground-Water" }, { { { "Electric" }, im }, { { "Fire" }, f2 } } },
    { { "Ice-Psychic" }, { { { "Ice" }, f2 } } },
    { { "Ice-Water" }, { { { "Ice" }, f4 }, { { "Water" }, f2 } } },
  };
  const std::vector<Dx::Pokemon_links::Type_name> headers = {
    { { "" }, 6, 1 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 1, 3 },
    { { "Grass" }, 2, 4 },
    { { "Ice" }, 3, 5 },
    { { "Normal" }, 4, 6 },
    { { "Water" }, 5, 0 },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx = {
    // 0               1Electric       2Fire            3Grass        4Ice             5Normal       6Water
    {0,0,0,em,0},    {2,18,11,em,0}, {2,19,15,em,0}, {3,16,8,em,0}, {2,23,21,em,0}, {1,9,9,em,0}, {2,24,13,em,0}, 
    // 7Bug-Ghost                                         8                            9
    {-1,0,9,em,0},                                   {3,3,12,f2,0},                 {5,5,5,im,0}, 
    // 10Elec-Fly        11                                 12                                          13
    {-2,8,13,em,0},  {1,1,18,f4,0},                  {3,8,16,f2,0},                               {6,6,24,f2,0}, 
    // 14Fire-Fly                        15               16
    {-3,11,16,em,0},                 {2,2,19,f2,0},  {3,12,3,f4,0}, 
    // 17Ground-Water    18                19
    {-4,15,19,em,0}, {1,11,1,im,0},  {2,15,2,f2,0}, 
    // 20Ice-Psych                                                      21
    {-5,18,21,em,0},                                                {4,4,23,f2,0}, 
    // 22Ice-Water                                                      23                           24
    {-6,21,24,em,0},                                                {4,21,4,f4,0},                {6,13,6,f2,0}, 
    // 25
    {INT_MIN,23,UINT64_MAX,em,0},
  };
  // clang-format on
  Dx::Pokemon_links links( types, Dx::Pokemon_links::defense );
  const std::set<Ranked_set<Dx::Type_encoding>> result = links.get_overlapping_coverages( 6 );
  const std::set<Ranked_set<Dx::Type_encoding>> correct
    = { { 13, { { "Bug-Ghost" }, { "Ground-Water" }, { "Ice-Water" } } },
        { 14, { { "Bug-Ghost" }, { "Electric-Grass" }, { "Fire-Flying" }, { "Ice-Water" } } },
        { 14, { { "Bug-Ghost" }, { "Electric-Grass" }, { "Ground-Water" }, { "Ice-Psychic" } } },
        { 14, { { "Bug-Ghost" }, { "Electric-Grass" }, { "Ground-Water" }, { "Ice-Water" } } },
        { 14, { { "Bug-Ghost" }, { "Ground-Water" }, { "Ice-Psychic" }, { "Ice-Water" } } },
        { 15, { { "Bug-Ghost" }, { "Electric-Grass" }, { "Fire-Flying" }, { "Ice-Psychic" } } },
        { 15, { { "Bug-Ghost" }, { "Electric-Grass" }, { "Ground-Water" }, { "Ice-Psychic" } } } };
  EXPECT_EQ( correct, result );
  // Make sure the overlapping algorithms clean up the tables when done.
  EXPECT_EQ( links.item_table_, headers );
  EXPECT_EQ( links.links_, dlx );
}

/* * * * * * * *    Test the Utility Functions for the Fun User Ops We Support      * * * * * * * */

TEST( InternalTests, TestBinarySearchOnTheItemTable )
{
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
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types {
    { { "Electric" },
      { { { "Electric" }, f2 },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, nm } } },
    { { "Fire" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, f2 },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Grass" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Ice" },
      { { { "Electric" }, nm },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, f2 },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Normal" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, f2 },
        { { "Water" }, nm } } },
    { { "Water" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
  };
  const std::vector<Dx::Pokemon_links::Type_name> headers {
    { { "" }, 6, 1 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 1, 3 },
    { { "Grass" }, 2, 4 },
    { { "Ice" }, 3, 5 },
    { { "Normal" }, 4, 6 },
    { { "Water" }, 5, 0 },
  };
  const Dx::Pokemon_links links( types, Dx::Pokemon_links::defense );
  EXPECT_EQ( links.find_item_index( Dx::Type_encoding( std::string { "Electric" } ) ), 1 );
  EXPECT_EQ( links.find_item_index( Dx::Type_encoding( std::string { "Fire" } ) ), 2 );
  EXPECT_EQ( links.find_item_index( Dx::Type_encoding( std::string { "Grass" } ) ), 3 );
  EXPECT_EQ( links.find_item_index( Dx::Type_encoding( std::string { "Ice" } ) ), 4 );
  EXPECT_EQ( links.find_item_index( Dx::Type_encoding( std::string { "Normal" } ) ), 5 );
  EXPECT_EQ( links.find_item_index( Dx::Type_encoding( std::string { "Water" } ) ), 6 );
  EXPECT_EQ( links.find_item_index( Dx::Type_encoding( std::string { "Flamio" } ) ), 0 );
}

TEST( InternalTests, TestBinarySearchOnTheOptionTable )
{
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
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types {
    { { "Electric" },
      { { { "Electric" }, f2 },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, nm } } },
    { { "Fire" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, f2 },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Grass" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Ice" },
      { { { "Electric" }, nm },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, f2 },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Normal" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, f2 },
        { { "Water" }, nm } } },
    { { "Water" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
  };
  const std::vector<Dx::Pokemon_links::Type_name> headers {
    { { "" }, 6, 1 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 1, 3 },
    { { "Grass" }, 2, 4 },
    { { "Ice" }, 3, 5 },
    { { "Normal" }, 4, 6 },
    { { "Water" }, 5, 0 },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx = {
    // 0             1Electric      2Fire           3Grass        4Ice           5Normal        6Water
    {0,0,0,em,0},   {3,21,8,em,0},{3,24,9,em,0},{1,12,12,em,0},{1,18,18,em,0},{1,22,22,em,0},{4,25,13,em,0},
    // 7Electric       8             9
    {-1,0,9,em,0},  {1,1,11,f2,0},{2,2,15,f2,0},
    // 10Fire          11                            12                                          13
    {-2,8,13,em,0}, {1,8,21,f2,0},              {3,3,3,f2,0},                                {6,6,16,f2,0},
    // 14Grass                      15                                                           16
    {-3,11,16,em,0},              {2,9,24,f2,0},                                             {6,13,19,f2,0},
    // 17Ice                                                      18                             19
    {-4,15,19,em,0},                                           {4,4,4,f2,0},                 {6,16,25,f2,0},
    // 20Normal        21                                                          22
    {-5,18,22,em,0},{1,11,1,f2,0},                                             {5,5,5,f2,0},
    // 23Water                      24                                                           25
    {-6,21,25,em,0},              {2,15,2,f2,0},                                             {6,19,6,f2,0},
    //       26
    {INT_MIN,24,UINT64_MAX,em,0},
  };
  // clang-format on
  const Dx::Pokemon_links links( types, Dx::Pokemon_links::defense );
  EXPECT_EQ( links.find_option_index( Dx::Type_encoding( std::string( "Electric" ) ) ), 7 );
  EXPECT_EQ( links.find_option_index( Dx::Type_encoding( std::string( "Fire" ) ) ), 10 );
  EXPECT_EQ( links.find_option_index( Dx::Type_encoding( std::string( "Grass" ) ) ), 14 );
  EXPECT_EQ( links.find_option_index( Dx::Type_encoding( std::string( "Ice" ) ) ), 17 );
  EXPECT_EQ( links.find_option_index( Dx::Type_encoding( std::string( "Normal" ) ) ), 20 );
  EXPECT_EQ( links.find_option_index( Dx::Type_encoding( std::string( "Water" ) ) ), 23 );
  EXPECT_EQ( links.find_option_index( Dx::Type_encoding( std::string( "Flamio" ) ) ), 0 );
}

/* * * * * * * *      Test the Hiding of Options and Items the User Can Use         * * * * * * * */

TEST( InternalTests, TestHidingAnItemFromTheWorld )
{
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
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types {
    { { "Electric" },
      { { { "Electric" }, f2 },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, nm } } },
    { { "Fire" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, f2 },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Grass" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Ice" },
      { { { "Electric" }, nm },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, f2 },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Normal" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, f2 },
        { { "Water" }, nm } } },
    { { "Water" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
  };
  const std::vector<Dx::Pokemon_links::Type_name> headers {
    { { "" }, 6, 1 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 1, 3 },
    { { "Grass" }, 2, 4 },
    { { "Ice" }, 3, 5 },
    { { "Normal" }, 4, 6 },
    { { "Water" }, 5, 0 },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx = {
    // 0              1Electric     2Fire         3Grass          4Ice          5Normal        6Water
    {0,0,0,em,0},   {3,21,8,em,0},{3,24,9,em,0},{1,12,12,em,0},{1,18,18,em,0},{1,22,22,em,0},{4,25,13,em,0},
    // 7Electric       8             9
    {-1,0,9,em,0},  {1,1,11,f2,0},{2,2,15,f2,0},
    // 10Fire         11                          12                                             13
    {-2,8,13,em,0}, {1,8,21,f2,0},              {3,3,3,f2,0},                                {6,6,16,f2,0},
    // 14Grass                      15                                                          16
    {-3,11,16,em,0},              {2,9,24,f2,0},                                             {6,13,19,f2,0},
    // 17Ice                                                     18                              19
    {-4,15,19,em,0},                                           {4,4,4,f2,0},                 {6,16,25,f2,0},
    // 20Normal        21                                                        22
    {-5,18,22,em,0},{1,11,1,f2,0},                                            {5,5,5,f2,0},
    // 23Water                       24                                                          25
    {-6,21,25,em,0},              {2,15,2,f2,0},                                             {6,19,6,f2,0},
    //       26
    {INT_MIN,24,UINT64_MAX,em,0},
  };
  // clang-format on
  Dx::Pokemon_links links( types, Dx::Pokemon_links::defense );
  EXPECT_EQ( true, links.hide_requested_item( Dx::Type_encoding( std::string( "Fire" ) ) ) );
  const std::vector<Dx::Pokemon_links::Type_name> headers_hide_fire {
    { { "" }, 6, 1 },
    { { "Electric" }, 0, 3 },
    { { "Fire" }, 1, 3 },
    { { "Grass" }, 1, 4 },
    { { "Ice" }, 3, 5 },
    { { "Normal" }, 4, 6 },
    { { "Water" }, 5, 0 },
  };
  const int8_t hd = Dx::Pokemon_links::hidden;
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx_hide_fire = {
    // 0              1Electric      2Fire            3Grass        4Ice           5Normal        6Water
    {0,0,0,em,0},   {3,21,8,em,0},{3,24,9,em,hd},{1,12,12,em,0},{1,18,18,em,0},{1,22,22,em,0},{4,25,13,em,0},
    // 7Electric       8             9
    {-1,0,9,em,0},  {1,1,11,f2,0},{2,2,15,f2,0},
    // 10Fire          11                             12                                          13
    {-2,8,13,em,0}, {1,8,21,f2,0},               {3,3,3,f2,0},                                {6,6,16,f2,0},
    // 14Grass                       15                                                           16
    {-3,11,16,em,0},              {2,9,24,f2,0},                                              {6,13,19,f2,0},
    // 17Ice                                                        18                            19
    {-4,15,19,em,0},                                            {4,4,4,f2,0},                 {6,16,25,f2,0},
    // 20Normal        21                                                          22
    {-5,18,22,em,0},{1,11,1,f2,0},                                             {5,5,5,f2,0},
    // 23Water                       24                                                           25
    {-6,21,25,em,0},              {2,15,2,f2,0},                                              {6,19,6,f2,0},
    // 26
    {INT_MIN,24,UINT64_MAX,em,0},
  };
  // clang-format on
  EXPECT_EQ( links.links_, dlx_hide_fire );
  EXPECT_EQ( links.item_table_, headers_hide_fire );
  EXPECT_EQ( false, links.hide_requested_item( Dx::Type_encoding( std::string( "Fire" ) ) ) );
  EXPECT_EQ( links.links_, dlx_hide_fire );
  EXPECT_EQ( links.peek_hid_item().decode_type().first, "Fire" );
  EXPECT_EQ( links.get_num_hid_items(), 1 );
  // Test our unhide and reset functions.
  links.pop_hid_item();
  EXPECT_EQ( links.links_, dlx );
  EXPECT_EQ( links.item_table_, headers );
  EXPECT_EQ( true, links.hid_items_empty() );
  EXPECT_EQ( links.get_num_hid_items(), 0 );
  EXPECT_EQ( true, links.hide_requested_item( Dx::Type_encoding( std::string( "Fire" ) ) ) );
  links.reset_items();
  EXPECT_EQ( links.links_, dlx );
  EXPECT_EQ( links.item_table_, headers );
  EXPECT_EQ( true, links.hid_items_empty() );
  EXPECT_EQ( links.get_num_hid_items(), 0 );
}

TEST( InternalTests, TestHidingGrassAndIceAndThenResetTheLinks )
{
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
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types {
    { { "Electric" },
      { { { "Electric" }, f2 },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, nm } } },
    { { "Fire" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, f2 },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Grass" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Ice" },
      { { { "Electric" }, nm },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, f2 },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Normal" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, f2 },
        { { "Water" }, nm } } },
    { { "Water" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx = {
    // 0             1Electric     2Fire         3Grass          4Ice          5Normal        6Water
    {0,0,0,em,0},   {3,21,8,em,0},{3,24,9,em,0},{1,12,12,em,0},{1,18,18,em,0},{1,22,22,em,0},{4,25,13,em,0},
    // 7Electric        8             9
    {-1,0,9,em,0},  {1,1,11,f2,0},{2,2,15,f2,0},
    // 10Fire         11                          12                                           13
    {-2,8,13,em,0}, {1,8,21,f2,0},              {3,3,3,f2,0},                                {6,6,16,f2,0},
    // 14Grass                       15                                                        16
    {-3,11,16,em,0},              {2,9,24,f2,0},                                             {6,13,19,f2,0},
    // 17Ice                                                     18                            19
    {-4,15,19,em,0},                                           {4,4,4,f2,0},                 {6,16,25,f2,0},
    // 20Normal        21                                                        22
    {-5,18,22,em,0},{1,11,1,f2,0},                                            {5,5,5,f2,0},
    // 23Water                       24                                                          25
    {-6,21,25,em,0},              {2,15,2,f2,0},                                             {6,19,6,f2,0},
    //       26
    {INT_MIN,24,UINT64_MAX,em,0},
  };
  // clang-format on
  Dx::Pokemon_links links( types, Dx::Pokemon_links::defense );
  EXPECT_EQ( true, links.hide_requested_option( { { "Grass" }, { "Ice" } } ) );
  const int hd = Dx::Pokemon_links::hidden;
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx_hide_option_ice_grass = {
    // 0               1Electric     2Fire           3Grass        4Ice         5Normal        6Water
    {0,0,0,em,0},    {3,21,8,em,0},{2,24,9,em,0},{1,12,12,em,0},{0,4,4,em,0},{1,22,22,em,0},{2,25,13,em,0},
    // 7Electric       8             9
    {-1,0,9,em,0},   {1,1,11,f2,0},{2,2,24,f2,0},
    // 10Fire          11                            12                                        13
    {-2,8,13,em,0},  {1,8,21,f2,0},              {3,3,3,f2,0},                              {6,6,25,f2,0},
    // 14Grass                       15                                                        16
    {-3,11,16,em,hd},              {2,9,24,f2,0},                                           {6,13,19,f2,0},
    // 17Ice                                                       18                          19
    {-4,15,19,em,hd},                                           {4,4,4,f2,0},               {6,13,25,f2,0},
    // 20Normal        21                                                       22
    {-5,18,22,em,0}, {1,11,1,f2,0},                                          {5,5,5,f2,0},
    // 23Water                         24                                                       25
    {-6,21,25,em,0},               {2,9,2,f2,0},                                            {6,13,6,f2,0},
    // 26
    {INT_MIN,24,UINT64_MAX,em,0},
  };
  // clang-format on
  EXPECT_EQ( links.links_, dlx_hide_option_ice_grass );
  links.reset_options();
  EXPECT_EQ( links.links_, dlx );
  EXPECT_EQ( true, links.hid_items_empty() );
  EXPECT_EQ( links.get_num_hid_options(), 0 );
}

TEST( InternalTests, TestHidingAnOptionFromTheWorld )
{
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
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types {
    { { "Electric" },
      { { { "Electric" }, f2 },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, nm } } },
    { { "Fire" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, f2 },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Grass" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Ice" },
      { { { "Electric" }, nm },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, f2 },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Normal" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, f2 },
        { { "Water" }, nm } } },
    { { "Water" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx = {
    // 0              1Electric     2Fire         3Grass          4Ice          5Normal        6Water
    {0,0,0,em,0},   {3,21,8,em,0},{3,24,9,em,0},{1,12,12,em,0},{1,18,18,em,0},{1,22,22,em,0},{4,25,13,em,0},
    // 7Electric       8             9
    {-1,0,9,em,0},  {1,1,11,f2,0},{2,2,15,f2,0},
    // 10Fire          11                          12                                           13
    {-2,8,13,em,0}, {1,8,21,f2,0},              {3,3,3,f2,0},                                {6,6,16,f2,0},
    // 14Grass                       15                                                          16
    {-3,11,16,em,0},              {2,9,24,f2,0},                                             {6,13,19,f2,0},
    // 17Ice                                                     18                             19
    {-4,15,19,em,0},                                           {4,4,4,f2,0},                 {6,16,25,f2,0},
    // 20Normal         21                                                        22
    {-5,18,22,em,0},{1,11,1,f2,0},                                            {5,5,5,f2,0},
    // 23Water                       24                                                          25
    {-6,21,25,em,0},              {2,15,2,f2,0},                                             {6,19,6,f2,0},
    //       26
    {INT_MIN,24,UINT64_MAX,em,0},
  };
  // clang-format on
  Dx::Pokemon_links links( types, Dx::Pokemon_links::defense );

  EXPECT_EQ( true, links.hide_requested_option( Dx::Type_encoding( std::string( "Fire" ) ) ) );
  const Dx::Type_encoding fire( "Fire" );
  const Dx::Type_encoding flipper( "Fire" );
  const std::vector<Dx::Type_encoding> fire_flipper { fire, flipper };
  std::vector<Dx::Type_encoding> failed_to_hide = {};
  EXPECT_EQ( false, links.hide_requested_option( { fire, flipper } ) );
  EXPECT_EQ( false, links.hide_requested_option( { fire, flipper }, failed_to_hide ) );
  EXPECT_EQ( failed_to_hide, fire_flipper );

  const int hd = Dx::Pokemon_links::hidden;
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx_hide_option_fire = {
    // 0              1Electric     2Fire         3Grass          4Ice          5Normal        6Water
    {0,0,0,em,0},   {2,21,8,em,0},{3,24,9,em,0},{0,3,3,em,0},{1,18,18,em,0},{1,22,22,em,0},{3,25,16,em,0},
    // 7Electric        8             9
    {-1,0,9,em,0},  {1,1,21,f2,0},{2,2,15,f2,0},
    // 10Fire          11                          12                                           13
    {-2,8,13,em,hd},{1,8,21,f2,0},              {3,3,3,f2,0},                              {6,6,16,f2,0},
    // 14Grass                      15                                                         16
    {-3,11,16,em,0},              {2,9,24,f2,0},                                           {6,6,19,f2,0},
    // 17Ice                                                     18                            19
    {-4,15,19,em,0},                                         {4,4,4,f2,0},                 {6,16,25,f2,0},
    // 20Normal       21                                                        22
    {-5,18,22,em,0},{1,8,1,f2,0},                                           {5,5,5,f2,0},
    // 23Water                      24                                                         25
    {-6,21,25,em,0},              {2,15,2,f2,0},                                           {6,19,6,f2,0},
    //       26
    {INT_MIN,24,UINT64_MAX,em,0},
  };
  // clang-format on
  EXPECT_EQ( links.links_, dlx_hide_option_fire );
  EXPECT_EQ( false, links.hide_requested_option( fire ) );
  EXPECT_EQ( links.links_, dlx_hide_option_fire );
  EXPECT_EQ( links.peek_hid_option(), fire );
  EXPECT_EQ( links.get_num_hid_options(), 1 );
  links.pop_hid_option();
  EXPECT_EQ( links.links_, dlx );
  EXPECT_EQ( true, links.hid_items_empty() );
  EXPECT_EQ( links.get_num_hid_options(), 0 );
  EXPECT_EQ( true, links.hide_requested_option( fire ) );
  links.reset_options();
  EXPECT_EQ( links.links_, dlx );
  EXPECT_EQ( true, links.hid_items_empty() );
  EXPECT_EQ( links.get_num_hid_options(), 0 );
}

TEST( InternalTests, TestHidingAnItemFromTheWorldAndThenSolvingBothTypesOfCover )
{
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
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types {
    { { "Electric" },
      { { { "Electric" }, f2 },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, nm } } },
    { { "Fire" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, f2 },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, db } } },
    { { "Grass" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Ice" },
      { { { "Electric" }, nm },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, f2 },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Normal" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, f2 },
        { { "Water" }, nm } } },
    { { "Water" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
  };
  const std::vector<Dx::Pokemon_links::Type_name> headers {
    { { "" }, 6, 1 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 1, 3 },
    { { "Grass" }, 2, 4 },
    { { "Ice" }, 3, 5 },
    { { "Normal" }, 4, 6 },
    { { "Water" }, 5, 0 },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx = {
    // 0               1Electric    2Fire         3Grass         4Ice          5Normal         6Water
    {0,0,0,em,0},   {3,20,8,em,0},{3,23,9,em,0},{1,12,12,em,0},{1,17,17,em,0},{1,21,21,em,0},{3,24,15,em,0},
    // 7Electric
    {-1,0,9,em,0},  {1,1,11,f2,0},{2,2,14,f2,0},
    // 10Fire
    {-2,8,12,em,0}, {1,8,20,f2,0},              {3,3,3,f2,0},
    // 13Grass
    {-3,11,15,em,0},              {2,9,23,f2,0},                                             {6,6,18,f2,0},
    // 16Ice
    {-4,14,18,em,0},                                           {4,4,4,f2,0},                 {6,15,24,f2,0},
    // 19Normal
    {-5,17,21,em,0},{1,11,1,f2,0},                                            {5,5,5,f2,0},
    // 22Water
    {-6,20,24,em,0},              {2,14,2,f2,0},                                             {6,18,6,f2,0},
    // 25
    {INT_MIN,23,UINT64_MAX,em,0},
  };
  // clang-format on
  Dx::Pokemon_links links( types, Dx::Pokemon_links::defense );
  const Dx::Type_encoding electric( "Electric" );
  EXPECT_EQ( true, links.hide_requested_item( electric ) );
  const std::vector<Dx::Pokemon_links::Type_name> headers_hide_electric {
    { { "" }, 6, 2 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 0, 3 },
    { { "Grass" }, 2, 4 },
    { { "Ice" }, 3, 5 },
    { { "Normal" }, 4, 6 },
    { { "Water" }, 5, 0 },
  };
  const int hd = Dx::Pokemon_links::hidden;
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx_hide_electric = {
    // 0             1Electric     2Fire         3Grass         4Ice          5Normal         6Water
    {0,0,0,em,0},  {3,20,8,em,hd},{3,23,9,em,0},{1,12,12,em,0},{1,17,17,em,0},{1,21,21,em,0},{3,24,15,em,0},    
    //7Electric
    {-1,0,9,em,0}, {1,1,11,f2,0}, {2,2,14,f2,0},
    //10Fire
    {-2,8,12,em,0},{1,8,20,f2,0},               {3,3,3,f2,0},
    //13Grass
    {-3,11,15,em,0},              {2,9,23,f2,0},                                             {6,6,18,f2,0},
    //16Ice
    {-4,14,18,em,0},                                           {4,4,4,f2,0},                 {6,15,24,f2,0},
    //19Normal
    {-5,17,21,em,0},{1,11,1,f2,0},                                            {5,5,5,f2,0},
    //22Water
    {-6,20,24,em,0},              {2,14,2,f2,0},                                             {6,18,6,f2,0},
    // 25
    {INT_MIN,23,UINT64_MAX,em,0},
  };
  // clang-format on
  const std::set<Ranked_set<Type_encoding>> exact {
    { 15, { { "Electric" }, { "Fire" }, { "Ice" }, { "Normal" } } } };
  const std::set<Ranked_set<Type_encoding>> overlapping {
    { 15, { { "Electric" }, { "Fire" }, { "Ice" }, { "Normal" } } },
    { 15, { { "Fire" }, { "Grass" }, { "Ice" }, { "Normal" } } },
    { 15, { { "Fire" }, { "Ice" }, { "Normal" }, { "Water" } } } };
  EXPECT_EQ( links.links_, dlx_hide_electric );
  EXPECT_EQ( links.item_table_, headers_hide_electric );
  EXPECT_EQ( links.get_num_items(), 5 );
  EXPECT_EQ( links.get_exact_coverages( 6 ), exact );
  EXPECT_EQ( links.get_overlapping_coverages( 6 ), overlapping );
}

TEST( InternalTests, TestHidingTwoItemsFromTheWorldAndThenSolvingBothTypesOfCover )
{
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
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types {
    { { "Electric" },
      { { { "Electric" }, f2 },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, nm } } },
    { { "Fire" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, f2 },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, db } } },
    { { "Grass" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Ice" },
      { { { "Electric" }, nm },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, f2 },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Normal" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, f2 },
        { { "Water" }, nm } } },
    { { "Water" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
  };
  const std::vector<Dx::Pokemon_links::Type_name> headers {
    { { "" }, 6, 1 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 1, 3 },
    { { "Grass" }, 2, 4 },
    { { "Ice" }, 3, 5 },
    { { "Normal" }, 4, 6 },
    { { "Water" }, 5, 0 },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx = {
    // 0               1Electric    2Fire         3Grass         4Ice          5Normal         6Water
    {0,0,0,em,0},   {3,20,8,em,0},{3,23,9,em,0},{1,12,12,em,0},{1,17,17,em,0},{1,21,21,em,0},{3,24,15,em,0},
    // 7Electric
    {-1,0,9,em,0},  {1,1,11,f2,0},{2,2,14,f2,0},
    // 10Fire
    {-2,8,12,em,0}, {1,8,20,f2,0},              {3,3,3,f2,0},
    // 13Grass
    {-3,11,15,em,0},              {2,9,23,f2,0},                                             {6,6,18,f2,0},
    // 16Ice
    {-4,14,18,em,0},                                           {4,4,4,f2,0},                 {6,15,24,f2,0},
    // 19Normal
    {-5,17,21,em,0},{1,11,1,f2,0},                                            {5,5,5,f2,0},
    // 22Water
    {-6,20,24,em,0},              {2,14,2,f2,0},                                             {6,18,6,f2,0},
    // 25
    {INT_MIN,23,UINT64_MAX,em,0},
  };
  // clang-format on
  Dx::Pokemon_links links( types, Dx::Pokemon_links::defense );
  const Dx::Type_encoding electric( "Electric" );
  EXPECT_EQ( true, links.hide_requested_item( electric ) );
  std::vector<Dx::Type_encoding> fail_output = {};
  const Dx::Type_encoding grass( "Grass" );
  const Dx::Type_encoding grassy( "Grassy" );
  const Dx::Type_encoding cloudy( "Cloudy" );
  const Dx::Type_encoding rainy( "Rainy" );
  const std::vector<Dx::Type_encoding> hide_request { grass, electric, grassy, cloudy, rainy };
  const std::vector<Dx::Type_encoding> should_fail { electric, grassy, cloudy, rainy };
  EXPECT_EQ( false, links.hide_requested_item( hide_request, fail_output ) );
  EXPECT_EQ( fail_output, should_fail );
  const std::vector<Dx::Pokemon_links::Type_name> headers_hide_electric_and_grass {
    { { "" }, 6, 2 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 0, 4 },
    { { "Grass" }, 2, 4 },
    { { "Ice" }, 2, 5 },
    { { "Normal" }, 4, 6 },
    { { "Water" }, 5, 0 },
  };
  const int hd = Dx::Pokemon_links::hidden;
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx_hide_electric_and_grass = {
    // 0               1Electric    2Fire         3Grass            4Ice          5Normal         6Water
    {0,0,0,em,0},   {3,20,8,em,hd},{3,23,9,em,0},{1,12,12,em,hd},{1,17,17,em,0},{1,21,21,em,0},{3,24,15,em,0},
    // 7Electric
    {-1,0,9,em,0},  {1,1,11,f2,0},{2,2,14,f2,0},
    // 10Fire
    {-2,8,12,em,0}, {1,8,20,f2,0},               {3,3,3,f2,0},
    // 13Grass
    {-3,11,15,em,0},              {2,9,23,f2,0},                                               {6,6,18,f2,0},
    // 16Ice
    {-4,14,18,em,0},                                             {4,4,4,f2,0},                 {6,15,24,f2,0},
    // 19Normal
    {-5,17,21,em,0},{1,11,1,f2,0},                                              {5,5,5,f2,0},
    // 22Water
    {-6,20,24,em,0},              {2,14,2,f2,0},                                               {6,18,6,f2,0},
    // 25
    {INT_MIN,23,UINT64_MAX,em,0},
  };
  // clang-format on
  const std::set<Ranked_set<Type_encoding>> exact { { 12, { { "Electric" }, { "Ice" }, { "Normal" } } } };
  const std::set<Ranked_set<Type_encoding>> overlapping { { 12, { { "Electric" }, { "Ice" }, { "Normal" } } },
                                                          { 12, { { "Grass" }, { "Ice" }, { "Normal" } } },
                                                          { 12, { { "Ice" }, { "Normal" }, { "Water" } } } };
  EXPECT_EQ( links.links_, dlx_hide_electric_and_grass );
  EXPECT_EQ( links.item_table_, headers_hide_electric_and_grass );
  EXPECT_EQ( links.get_num_items(), 4 );
  EXPECT_EQ( links.get_exact_coverages( 6 ), exact );
  EXPECT_EQ( links.get_overlapping_coverages( 6 ), overlapping );
}

TEST( InternalTests, TestTheHidingAllTheItemsExceptForTheOnesTheUserWantsToKeep )
{
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
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types {
    { { "Electric" },
      { { { "Electric" }, f2 },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, nm } } },
    { { "Fire" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, f2 },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, db } } },
    { { "Grass" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Ice" },
      { { { "Electric" }, nm },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, f2 },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Normal" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, f2 },
        { { "Water" }, nm } } },
    { { "Water" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
  };
  const std::vector<Dx::Pokemon_links::Type_name> headers {
    { { "" }, 6, 1 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 1, 3 },
    { { "Grass" }, 2, 4 },
    { { "Ice" }, 3, 5 },
    { { "Normal" }, 4, 6 },
    { { "Water" }, 5, 0 },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx = {
    // 0               1Electric    2Fire         3Grass         4Ice          5Normal         6Water
    {0,0,0,em,0},   {3,20,8,em,0},{3,23,9,em,0},{1,12,12,em,0},{1,17,17,em,0},{1,21,21,em,0},{3,24,15,em,0},
    // 7Electric
    {-1,0,9,em,0},  {1,1,11,f2,0},{2,2,14,f2,0},
    // 10Fire
    {-2,8,12,em,0}, {1,8,20,f2,0},              {3,3,3,f2,0},
    // 13Grass
    {-3,11,15,em,0},              {2,9,23,f2,0},                                             {6,6,18,f2,0},
    // 16Ice
    {-4,14,18,em,0},                                           {4,4,4,f2,0},                 {6,15,24,f2,0},
    // 19Normal
    {-5,17,21,em,0},{1,11,1,f2,0},                                            {5,5,5,f2,0},
    // 22Water
    {-6,20,24,em,0},              {2,14,2,f2,0},                                             {6,18,6,f2,0},
    // 25
    {INT_MIN,23,UINT64_MAX,em,0},
  };
  // clang-format on
  Dx::Pokemon_links links( types, Dx::Pokemon_links::defense );
  links.hide_all_items_except( { Dx::Type_encoding( "Water" ) } );
  const std::vector<Dx::Pokemon_links::Type_name> headers_hide_except_water {
    { { "" }, 6, 6 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 0, 3 },
    { { "Grass" }, 0, 4 },
    { { "Ice" }, 0, 5 },
    { { "Normal" }, 0, 6 },
    { { "Water" }, 0, 0 },
  };
  const int hd = Dx::Pokemon_links::hidden;
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx_hide_except_water = {
    // 0               1Electric      2Fire           3Grass          4Ice           5Normal         6Water
    {0,0,0,em,0},   {3,20,8,em,hd},{3,23,9,em,hd},{1,12,12,em,hd},{1,17,17,em,hd},{1,21,21,em,hd},{3,24,15,em,0},
    // 7Electric
    {-1,0,9,em,0},  {1,1,11,f2,0}, {2,2,14,f2,0},
    // 10Fire
    {-2,8,12,em,0}, {1,8,20,f2,0},                {3,3,3,f2,0},
    // 13Grass
    {-3,11,15,em,0},               {2,9,23,f2,0},                                                 {6,6,18,f2,0},
    // 16Ice
    {-4,14,18,em,0},                                              {4,4,4,f2,0},                   {6,15,24,f2,0},
    // 19Normal
    {-5,17,21,em,0},{1,11,1,f2,0},                                                {5,5,5,f2,0},
    // 22Water
    {-6,20,24,em,0},               {2,14,2,f2,0},                                                 {6,18,6,f2,0},
    // 25
    {INT_MIN,23,UINT64_MAX,em,0},
  };
  // clang-format on
  const std::set<Ranked_set<Dx::Type_encoding>> exact {
    { 3, { { "Grass" } } }, { 3, { { "Ice" } } }, { 3, { { "Water" } } } };
  const std::set<Ranked_set<Dx::Type_encoding>> overlapping {
    { 3, { { "Grass" } } }, { 3, { { "Ice" } } }, { 3, { { "Water" } } } };
  EXPECT_EQ( links.links_, dlx_hide_except_water );
  EXPECT_EQ( links.item_table_, headers_hide_except_water );
  EXPECT_EQ( links.get_num_items(), 1 );
  EXPECT_EQ( links.get_exact_coverages( 6 ), exact );
  EXPECT_EQ( links.get_overlapping_coverages( 6 ), overlapping );
  links.reset_items();
  EXPECT_EQ( links.links_, dlx );
  EXPECT_EQ( links.item_table_, headers );
  EXPECT_EQ( links.get_num_hid_items(), 0 );
}

TEST( InternalTests, TestHidingAllOptionsAndItemsExactThenOverlappingSolution )
{
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
  const std::map<Dx::Type_encoding, std::set<Dx::Resistance>> types {
    { { "Electric" },
      { { { "Electric" }, f2 },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, nm } } },
    { { "Fire" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, f2 },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, db } } },
    { { "Grass" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Ice" },
      { { { "Electric" }, nm },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, f2 },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
    { { "Normal" },
      { { { "Electric" }, f2 },
        { { "Fire" }, nm },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, f2 },
        { { "Water" }, nm } } },
    { { "Water" },
      { { { "Electric" }, nm },
        { { "Fire" }, f2 },
        { { "Grass" }, nm },
        { { "Ice" }, nm },
        { { "Normal" }, nm },
        { { "Water" }, f2 } } },
  };
  const std::vector<Dx::Pokemon_links::Type_name> headers {
    { { "" }, 6, 1 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 1, 3 },
    { { "Grass" }, 2, 4 },
    { { "Ice" }, 3, 5 },
    { { "Normal" }, 4, 6 },
    { { "Water" }, 5, 0 },
  };
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx = {
    // 0               1Electric    2Fire         3Grass         4Ice          5Normal         6Water
    {0,0,0,em,0},   {3,20,8,em,0},{3,23,9,em,0},{1,12,12,em,0},{1,17,17,em,0},{1,21,21,em,0},{3,24,15,em,0},
    // 7Electric
    {-1,0,9,em,0},  {1,1,11,f2,0},{2,2,14,f2,0},
    // 10Fire
    {-2,8,12,em,0}, {1,8,20,f2,0},              {3,3,3,f2,0},
    // 13Grass
    {-3,11,15,em,0},              {2,9,23,f2,0},                                             {6,6,18,f2,0},
    // 16Ice
    {-4,14,18,em,0},                                           {4,4,4,f2,0},                 {6,15,24,f2,0},
    // 19Normal
    {-5,17,21,em,0},{1,11,1,f2,0},                                            {5,5,5,f2,0},
    // 22Water
    {-6,20,24,em,0},              {2,14,2,f2,0},                                             {6,18,6,f2,0},
    // 25
    {INT_MIN,23,UINT64_MAX,em,0},
  };
  // clang-format on
  Dx::Pokemon_links links( types, Dx::Pokemon_links::defense );
  const Dx::Type_encoding water( "Water" );
  const Dx::Type_encoding grass( "Grass" );
  const Dx::Type_encoding electric( "Electric" );
  const Dx::Type_encoding fire( "Fire" );
  const Dx::Type_encoding ice( "Ice" );
  const Dx::Type_encoding normal( "Normal" );
  links.hide_all_items_except( { water } );
  links.hide_all_options_except( { grass } );
  EXPECT_EQ( links.get_num_hid_items(), 5 );
  EXPECT_EQ( links.get_num_hid_options(), 5 );
  EXPECT_EQ( true, links.has_item( water ) );
  EXPECT_EQ( true, links.has_option( grass ) );
  EXPECT_EQ( false, links.has_item( grass ) );
  EXPECT_EQ( false, links.has_item( electric ) );
  EXPECT_EQ( false, links.has_option( electric ) );
  EXPECT_EQ( false, links.has_item( fire ) );
  EXPECT_EQ( false, links.has_option( fire ) );
  EXPECT_EQ( false, links.has_item( ice ) );
  EXPECT_EQ( false, links.has_option( ice ) );
  EXPECT_EQ( false, links.has_item( normal ) );
  EXPECT_EQ( false, links.has_option( normal ) );
  EXPECT_EQ( false, links.has_option( water ) );
  const std::vector<Dx::Pokemon_links::Type_name> headers_hide_except_water {
    { { "" }, 6, 6 },
    { { "Electric" }, 0, 2 },
    { { "Fire" }, 0, 3 },
    { { "Grass" }, 0, 4 },
    { { "Ice" }, 0, 5 },
    { { "Normal" }, 0, 6 },
    { { "Water" }, 0, 0 },
  };
  const int8_t hd = Dx::Pokemon_links::hidden;
  // clang-format off
  const std::vector<Dx::Pokemon_links::Poke_link> dlx_hide_except_water = {
    // 0               1Electric        2Fire         3Grass         4Ice         5Normal       6Water
    {0,0,0,em,0},    {0,1,1,em,hd},{1,14,14,em,hd},{0,3,3,em,hd},{0,4,4,em,hd},{0,5,5,em,hd},{1,15,15,em,0},
    // 7Electric
    {-1,0,9,em,hd},  {1,1,11,f2,0},{2,2,14,f2,0},
    // 10Fire
    {-2,8,12,em,hd}, {1,1,20,f2,0},                {3,3,3,f2,0},
    // 13Grass
    {-3,11,15,em,0},               {2,2,2,f2,0},                                             {6,6,6,f2,0},
    // 16Ice
    {-4,14,18,em,hd},                                            {4,4,4,f2,0},               {6,15,24,f2,0},
    // 19Normal
    {-5,17,21,em,hd},{1,1,1,f2,0},                                             {5,5,5,f2,0},
    // 22Water
    {-6,20,24,em,hd},              {2,14,2,f2,0},                                            {6,15,6,f2,0},
    // 25
    {INT_MIN,23,UINT64_MAX,em,0},
  };
  // clang-format on
  const std::set<Ranked_set<Dx::Type_encoding>> answer { { 3, { { "Grass" } } } };
  EXPECT_EQ( links.links_, dlx_hide_except_water );
  EXPECT_EQ( links.item_table_, headers_hide_except_water );
  EXPECT_EQ( links.get_num_items(), 1 );
  EXPECT_EQ( links.get_num_options(), 1 );
  EXPECT_EQ( links.get_exact_coverages( 6 ), answer );
  EXPECT_EQ( links.get_overlapping_coverages( 6 ), answer );
  links.reset_items();
  links.reset_options();
  EXPECT_EQ( links.links_, dlx );
  EXPECT_EQ( links.item_table_, headers );
  EXPECT_EQ( links.get_num_hid_items(), 0 );
  EXPECT_EQ( links.get_num_hid_options(), 0 );
  links.hide_all_items_except( { water } );
  links.hide_all_options_except( { grass } );
  links.reset_items_options();
  EXPECT_EQ( links.links_, dlx );
  EXPECT_EQ( links.item_table_, headers );
  EXPECT_EQ( links.get_num_hid_items(), 0 );
  EXPECT_EQ( links.get_num_hid_options(), 0 );
}

} // namespace Dancing_links
