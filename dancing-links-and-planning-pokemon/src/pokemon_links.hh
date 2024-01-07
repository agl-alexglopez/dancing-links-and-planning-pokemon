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
 * File: Pokemon_links.h
 * --------------------------
 * This class defines a Pokemon_links object that can be used to solve the Pokemon Type Coverage
 * Problem with Donald Knuth's Algorithm X via Dancing Links. For detailed documentation read the
 * comment in the DancingLinks.h free function file for this class. Also the README.md for the
 * repository has a detailed writeup with images.
 */
#pragma once
#ifndef POKEMON_LINKS_HH
#define POKEMON_LINKS_HH
#include "ranked_set.hh"
#include "resistance.hh"
#include "type_encoding.hh"

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <unordered_map>
#include <vector>

namespace Dancing_links {

class Pokemon_links
{

public:
  static constexpr int hidden = -1;

  // The user is asking us for the defensive team they should build or attacks they need.
  enum Coverage_type
  {
    defense,
    attack
  };

  // This type, in a seperate vector, controls the base case of our recursion.
  struct Type_name
  {
    Type_encoding name;
    uint64_t left;
    uint64_t right;
  };

  // This type is entered into our dancing links array for the in place recursive algorithm.
  struct Poke_link
  {
    int32_t top_or_len;
    uint64_t up;
    uint64_t down;
    Multiplier multiplier; // x0.0, x0.25, x0.5, x1.0, x2, or x4 damage multipliers.
    int tag;               // We use this to efficiently generate overlapping sets.
  };

  struct Encoding_index
  {
    Type_encoding name;
    uint64_t index;
  };

  /**
   * @brief Pokemon_links            this constructor builds the necessary internal data structures
   *                                to run the exact cover via dancing links algorithm. We need
   *                                to build differently based on attack or defense. It is
   *                                important that the data is passed in with a map because we
   *                                need our dancing links items and options to be built and setup
   *                                in lexicographic order for some additional functionality and
   *                                runtime guarantees.
   * @param type_interactions        map of pokemon types and their resistances to attack types.
   * @param requested_cover_solution  ATTACK or DEFENSE. Build a team or choose attack types.
   */
  explicit Pokemon_links( const std::map<Type_encoding, std::set<Resistance>>& type_interactions,
                          Coverage_type requested_cover_solution );

  /**
   * @brief Pokemon_links      this alternative constructor is helpful when choosing a defensive
   *                           team based on a subset of attack types. For example, we could build
   *                           defenses against the attack types present at specific gyms. It is
   *                           important that the data is passed in with a map and set because we
   *                           need our dancing links items and options to be built and setup
   *                           in lexicographic order for some additional functionality and
   *                           runtime guarantees.
   * @param type_interactions  the map of types and their defenses for a given generation.
   * @param attack_types       the subset of attacks we must cover with choices of Pokemon teams.
   */
  explicit Pokemon_links( const std::map<Type_encoding, std::set<Resistance>>& type_interactions,
                          const std::set<Type_encoding>& attack_types );

  /* * * * * * * * * * * *  See Dancing_links.h for Documented Free Functions  * * * * * * * * * */

  [[nodiscard]] std::set<Ranked_set<Type_encoding>> exact_coverages_functional( int choice_limit );

  [[nodiscard]] std::set<Ranked_set<Type_encoding>> exact_coverages_stack( int choice_limit );

  [[nodiscard]] std::set<Ranked_set<Type_encoding>> overlapping_coverages_functional( int choice_limit );

  [[nodiscard]] std::set<Ranked_set<Type_encoding>> overlapping_coverages_stack( int choice_limit );

  [[nodiscard]] bool hide_requested_item( Type_encoding to_hide );

  [[nodiscard]] bool hide_requested_item( const std::vector<Type_encoding>& to_hide );

  [[nodiscard]] bool hide_requested_item( const std::vector<Type_encoding>& to_hide,
                                          std::vector<Type_encoding>& failed_to_hide );

  void hide_all_items_except( const std::set<Type_encoding>& to_keep );

  [[nodiscard]] bool has_item( Type_encoding item ) const;

  [[nodiscard]] Type_encoding peek_hid_item() const;

  void pop_hid_item();

  [[nodiscard]] bool hid_items_empty() const;

  [[nodiscard]] std::vector<Type_encoding> get_hid_items() const;

  [[nodiscard]] uint64_t get_num_hid_items() const;

  void reset_items();

  [[nodiscard]] bool hide_requested_option( Type_encoding to_hide );

  [[nodiscard]] bool hide_requested_option( const std::vector<Type_encoding>& to_hide );

  [[nodiscard]] bool hide_requested_option( const std::vector<Type_encoding>& to_hide,
                                            std::vector<Type_encoding>& failed_to_hide );

  void hide_all_options_except( const std::set<Type_encoding>& to_keep );

  [[nodiscard]] bool has_option( Type_encoding option ) const;

  [[nodiscard]] Type_encoding peek_hid_option() const;

  void pop_hid_option();

  [[nodiscard]] bool hid_options_empty() const;

  [[nodiscard]] std::vector<Type_encoding> get_hid_options() const;

  [[nodiscard]] uint64_t get_num_hid_options() const;

  void reset_options();

  void reset_items_options();

  [[nodiscard]] bool reached_output_limit() const;

  [[nodiscard]] std::vector<Type_encoding> get_items() const;

  [[nodiscard]] uint64_t get_num_items() const;

  [[nodiscard]] std::vector<Type_encoding> get_options() const;

  [[nodiscard]] uint64_t get_num_options() const;

  [[nodiscard]] Coverage_type get_links_type() const;

  [[nodiscard]] const std::vector<Poke_link>& links() const;

  [[nodiscard]] const std::vector<Type_name>& item_table() const;

  [[nodiscard]] const std::vector<Encoding_index>& option_table() const;

private:
  /* * * * * * * * * * *   Dancing Links Internals and Implementation   * * * * * * * * * * * * */

  struct Encoding_score
  {
    Type_encoding name;
    int32_t score;
  };

  struct Cover_tag
  {
    uint64_t index;
    int tag;
  };

  /// This is the way we can acheive an explicit stack based dancing links algorithm.
  struct Branch
  {
    uint64_t item {};
    uint64_t option {};
    std::optional<Encoding_score> score {};
  };

  /* These data structures contain the core logic of Algorithm X via dancing links. For more
   * detailed information, see the tests in the implementation. These help acheive in place
   * recursion. We can also play around with more advanced in place techniques like hiding options
   * and items at the users request and restoring them later in place. Finally, because the option
   * table and item table are sorted lexographically we can find any option or item in O(lgN). No
   * auxillary maps are needed.
   */
  std::vector<Encoding_index> option_table_ {}; // How we know the name of the option we chose.
  std::vector<Type_name> item_table_ {};        // How we know the names of our items.
  std::vector<Poke_link> links_ {};             // The links that dance!
  std::vector<uint64_t> hidden_items_ {};       // Treat as stack with user hidden Items.
  std::vector<uint64_t> hidden_options_ {};     // Treat as stack with user hidden Options.
  std::size_t max_output_ { 200'000 };          // Cutoff our solution generation for GUI usability.
  bool hit_limit_ { false };                    // How we report to a user that we cutoff more solutions
  uint64_t num_items_ { 0 };                    // What needs to be covered.
  uint64_t num_options_ { 0 };                  // Options we can choose from to cover items.
  Coverage_type requested_cover_solution_ {};   // The user is asking for ATTACK or DEFENSE

  /**
   * @brief exact_dlx_recursive fills the output parameters with every exact cover that can be
   *                            determined for defending against attack types or attacking
   *                            defensive types. Exact covers use options to cover every item
   *                            exactly once.
   * @param coverages           the output parameter that serves as the final solution.
   * @param coverage            the successfully coverages we find while the links dance.
   * @param depth_limit         size of a pokemon team or the number of attacks a team can have.
   */
  void exact_dlx_functional( std::set<Ranked_set<Type_encoding>>& coverages,
                             Ranked_set<Type_encoding>& coverage,
                             int depth_limit );

  /**
   * @brief overlapping_dlx_recursive fills the output parameter with every overlapping cover that
   *                                  can be determined for defending against attack types or
   *                                  attacking defensive types. Overlapping covers use any
   *                                  number of options within their depth limit to cover all
   *                                  items. Two options covering some overlapping items is
   *                                  acceptable. This is slower and I have no way to not generate
   *                                  duplicates other than using a set. Better ideas wanted.
   * @param coverages                 the output parameter as our final solution if found.
   * @param coverage                  the helper set that fills the output parameter.
   * @param depth_tag                  a tag used to signify the recursive depth. Used internally.
   */
  void overlapping_dlx_recursive( std::set<Ranked_set<Type_encoding>>& coverages,
                                  Ranked_set<Type_encoding>& coverage,
                                  int depth_tag );

  /**
   * @brief choose_item  choose an item to cover that appears the least across all options. If an
   *                    item becomes inaccessible over the course of recursion I signify this by
   *                    returning 0. That branch should fail at that point.
   * @return            the index in the lookup table and headers of links_ of the item to cover.
   */
  [[nodiscard]] uint64_t choose_item() const;

  /**
   * @brief cover_type      perform an exact cover as described by Donald Knuth, eliminating the
   *                       option we have chosen, covering all associated items, and eliminating
   *                       all other options that include those covered items.
   * @param index_in_option  the index in the array we use to start covering and eliminating links.
   * @return               every option we choose contributes to the strength of the Ranked_set
   *                       it becomes a part of. Return the strength contribution to the set
   *                       and the name of the option we chose.
   */
  [[nodiscard]] Encoding_score cover_type( uint64_t index_in_option );

  /**
   * @brief uncover_type    undoes the work of the exact cover operation returning the option,
   *                       the items it covered, and all other options that include the items we
   *                       covered back uint64_to the links.
   * @param index_in_option  the work will be undone for the same option if given same index.
   */
  void uncover_type( uint64_t index_in_option );

  /**
   * @brief hide_options    takes the options containing the option we chose out of the links. Do
   *                       this in order to cover every item exactly once and not overlap. This
   *                       is the vertical traversal of the links.
   * @param index_in_option  the index we start at in a given option.
   */
  void hide_options( uint64_t index_in_option );

  /**
   * @brief unhide_options  undoes the work done by the hide_options operation, returning the other
   *                       options containing covered items in an option back into the links.
   * @param index_in_option  the work will be undone for the same option if given same index.
   */
  void unhide_options( uint64_t index_in_option );

  /**
   * @brief overlapping_cover_type  performs a loose or "overlapping" cover of items in a dancing
   *                              links algorithm. We allow other options that cover items already
   *                              covered to stay accessible in the links leading to many more
   *                              solutions being found as multiple options can cover some of the
   *                              same items.
   * @param index_in_option         the index in array used to start covering and eliminating links.
   * @param depth_tag              to perform this type of coverage I use a depth tag to know which
   *                              items have already been covered in an option and which still
   *                              need coverage.
   * @return                      the score our option contributes to its Ranked_set and name.
   */
  [[nodiscard]] Encoding_score overlapping_cover_type( Cover_tag tag );

  /**
   * @brief overlapping_uncover_type  undoes the work of the loos cover operation. It uncovers items
   *                                that were covered by an option at the same level of recursion
   *                                in which they were covered, using the depth tags to note
   *                                levels.
   * @param index_in_option           the same index as cover operation will uncover same items.
   */
  void overlapping_uncover_type( uint64_t index_in_option );

  /**
   * @brief find_item_index  performs binary search on the sorted item array to find its index in
   *                       the links array as the column header.
   * @param item           the type item we search for depending on ATTACK or DEFENSE.
   * @return               the index in the item lookup table. This is same as header in links.
   */
  [[nodiscard]] uint64_t find_item_index( Type_encoding item ) const;

  /**
   * @brief find_item_index  performs binary search on the sorted option array to find its index in
   *                       the links array as the row spacer.
   * @param item           the type item we search for depending on ATTACK or DEFENSE.
   * @return               the index in the item option table. This is same as spacer in links.
   */
  [[nodiscard]] uint64_t find_option_index( Type_encoding option ) const;

  /**
   * @brief hide_item     hiding an item in the links means we simply tag its column header with a
   *                     special value that tells our algorithms to ignore items. O(1).
   * @param header_index  the index in the column header of the links that dance.
   */
  void hide_item( uint64_t header_index );

  /**
   * @brief unhide_item   unhiding items means we reset tag to indicate is back in the world. O(1).
   * @param header_index  the index of the column header for the dancing links array.
   */
  void unhide_item( uint64_t header_index );

  /**
   * @brief hide_option  hiding an option involves splicing it out of the up-down linked list. We
   *                    remove all items in this option from the world so the option is hidden.
   * @param row_index    the spacer row index in the row within the dancing links array.
   */
  void hide_option( uint64_t row_index );

  /**
   * @brief unhide_option  unhiding an option undoes the splicing operation. Undoing an option
   *                      must be done in last in first out order. User is expected to manage
   *                      hidden options in a stack.
   * @param row_index      the spacer row index in the row within the dancing links array.
   */
  void unhide_option( uint64_t row_index );

  /* * * * * * * * * * *   Dancing Links Instantiation and Building     * * * * * * * * * * * * */

  /**
   * @brief build_defense_links  defensive links have all typings for a generation as options and
   *                           all single attack types as items. We will build these links along
   *                           with auxillary vectors that help control recursion and record the
   *                           names of the items and options.
   * @param type_interactions   the map of interactions and resistances between types in a gen.
   */
  void build_defense_links( const std::map<Type_encoding, std::set<Resistance>>& type_interactions );

  /**
   * @brief build_attack_links  attack links have all single attack types for a generation as
   *                          options and all possible Pokemon typings as items in the links.
   * @param type_interactions  the map of interactions and resistances between types in a gen.
   */
  void build_attack_links( const std::map<Type_encoding, std::set<Resistance>>& type_interactions );

  /**
   * @brief initialize_columns  helper to build the options in our links and the appearances of the
   *                           items across these options.
   * @param type_interactions   the map of interactions and resistances between types in a gen.
   * @param column_builder      the helper data structure to build the columns.
   * @param requested_coverage  requested coverage to know which multipliers to pay attention to.
   */
  void initialize_columns( const std::map<Type_encoding, std::set<Resistance>>& type_interactions,
                           std::unordered_map<Type_encoding, uint64_t>& column_builder,
                           Coverage_type requested_coverage );

}; // class Pokemon_links

} // namespace Dancing_links

#endif // POKEMON_LINKS_HH
