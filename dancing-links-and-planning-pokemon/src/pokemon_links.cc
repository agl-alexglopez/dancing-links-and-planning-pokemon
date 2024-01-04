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
 * File: Pokemon_links.cpp
 * ----------------------
 * Contained in this file is my implementation of Algorithm X via dancing links as outlined by
 * Donald Knuth. The exact cover implementation is a faithful representation of the algorithm that
 * Knuth describes in the context of C++ and the Pokemon Type Coverage Problem. The Overlapping
 * Coverage implementation is a variation on exact cover that I use to generate coverage that allows
 * multiple options to cover some of the same items more than once. For a more detailed writeup see
 * the DancingLinks.h file and README.md in this repository.
 */
#include "pokemon_links.hh"
#include "ranked_set.hh"
#include "resistance.hh"
#include "type_encoding.hh"
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <optional>
#include <ranges>
#include <set>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Dancing_links {

/* * * * * * * * * * * * *       Convenience Callers for Encapsulation      * * * * * * * * * * * */

std::set<Ranked_set<Type_encoding>> exact_cover_functional( Pokemon_links& dlx, int choice_limit )
{
  return dlx.exact_coverages_functional( choice_limit );
}

std::set<Ranked_set<Type_encoding>> exact_cover_stack( Pokemon_links& dlx, int choice_limit )
{
  return dlx.exact_coverages_stack( choice_limit );
}

std::set<Ranked_set<Type_encoding>> overlapping_cover_functional( Pokemon_links& dlx, int choice_limit )
{
  return dlx.overlapping_coverages_functional( choice_limit );
}

std::set<Ranked_set<Type_encoding>> overlapping_cover_stack( Pokemon_links& dlx, int choice_limit )
{
  return dlx.overlapping_coverages_stack( choice_limit );
}

bool has_max_solutions( const Pokemon_links& dlx )
{
  return dlx.reached_output_limit();
}

uint64_t num_items( const Pokemon_links& dlx )
{
  return dlx.get_num_items();
}

bool has_item( const Pokemon_links& dlx, Type_encoding item )
{
  return dlx.has_item( item );
}

uint64_t num_options( const Pokemon_links& dlx )
{
  return dlx.get_num_options();
}

bool has_option( const Pokemon_links& dlx, Type_encoding option )
{
  return dlx.has_option( option );
}

Pokemon_links::Coverage_type coverage_type( const Pokemon_links& dlx )
{
  return dlx.get_links_type();
}

std::vector<Type_encoding> items( const Pokemon_links& dlx )
{
  return dlx.get_items();
}

std::vector<Type_encoding> options( const Pokemon_links& dlx )
{
  return dlx.get_options();
}

bool hide_item( Pokemon_links& dlx, Type_encoding to_hide )
{
  return dlx.hide_requested_item( to_hide );
}

bool hide_item( Pokemon_links& dlx, const std::vector<Type_encoding>& to_hide )
{
  return dlx.hide_requested_item( to_hide );
}

bool hide_item( Pokemon_links& dlx,
                const std::vector<Type_encoding>& to_hide,
                std::vector<Type_encoding>& failed_to_hide )
{
  return dlx.hide_requested_item( to_hide, failed_to_hide );
}

void hide_items_except( Pokemon_links& dlx, const std::set<Type_encoding>& to_keep )
{
  dlx.hide_all_items_except( to_keep );
}

uint64_t num_hid_items( const Pokemon_links& dlx )
{
  return dlx.get_num_hid_items();
}

Type_encoding peek_hid_item( const Pokemon_links& dlx )
{
  return dlx.peek_hid_item();
}

void pop_hid_item( Pokemon_links& dlx )
{
  dlx.pop_hid_item();
}

bool hid_items_empty( const Pokemon_links& dlx )
{
  return dlx.hid_items_empty();
}

std::vector<Type_encoding> hid_items( const Pokemon_links& dlx )
{
  return dlx.get_hid_items();
}

void reset_items( Pokemon_links& dlx )
{
  dlx.reset_items();
}

bool hide_option( Pokemon_links& dlx, Type_encoding to_hide )
{
  return dlx.hide_requested_option( to_hide );
}

bool hide_option( Pokemon_links& dlx, const std::vector<Type_encoding>& to_hide )
{
  return dlx.hide_requested_option( to_hide );
}

bool hide_option( Pokemon_links& dlx,
                  const std::vector<Type_encoding>& to_hide,
                  std::vector<Type_encoding>& failed_to_hide )
{
  return dlx.hide_requested_option( to_hide, failed_to_hide );
}

void hide_options_except( Pokemon_links& dlx, const std::set<Type_encoding>& to_keep )
{
  dlx.hide_all_options_except( to_keep );
}

uint64_t num_hid_options( const Pokemon_links& dlx )
{
  return dlx.get_num_hid_options();
}

Type_encoding peek_hid_option( const Pokemon_links& dlx )
{
  return dlx.peek_hid_option();
}

void pop_hid_option( Pokemon_links& dlx )
{
  dlx.pop_hid_option();
}

bool hid_options_empty( const Pokemon_links& dlx )
{
  return dlx.hid_options_empty();
}

std::vector<Type_encoding> hid_options( const Pokemon_links& dlx )
{
  return dlx.get_hid_options();
}

void reset_options( Pokemon_links& dlx )
{
  dlx.reset_options();
}

void reset_all( Pokemon_links& dlx )
{
  dlx.reset_items_options();
}

/* * * * * * * * * * * * * * * *    Algorithm X via Dancing Links   * * * * * * * * * * * * * * * */

std::set<Ranked_set<Type_encoding>> Pokemon_links::exact_coverages_stack( int choice_limit )
{
  hit_limit_ = false;
  if ( choice_limit <= 0 ) {
    return {};
  }
  std::set<Ranked_set<Type_encoding>> coverages = {};
  Ranked_set<Type_encoding> coverage {};
  coverage.reserve( choice_limit );
  const uint64_t start = choose_item();
  // A true recursive stack. We will only have O(depth) branches on the stack equivalent to current search path.
  std::vector<Branch> dfs { { start, start, {} } };
  dfs.reserve( choice_limit );
  while ( !dfs.empty() ) {
    Branch& cur = dfs.back();
    // If we return down the stack to any state again, it is time to move on from this option.
    // This also ensures that proper cleanup happens when we are done with the entire search space.
    if ( cur.score ) {
      uncover_type( cur.option );
      static_cast<void>( coverage.erase( cur.score.value().score, cur.score.value().name ) );
      ++choice_limit;
    }
    // This is a caching mechanism so that if we return to this level of recursion we will know how
    // many options we have tried already. See the for loop in the functional version if this is confusing.
    cur.option = links_[cur.option].down;
    if ( cur.option == cur.item ) {
      dfs.pop_back();
      continue;
    }
    cur.score = cover_type( cur.option );
    static_cast<void>( coverage.insert( cur.score.value().score, cur.score.value().name ) );
    --choice_limit;

    if ( item_table_[0].right == 0 && choice_limit >= 0 ) {
      coverages.insert( coverage );
      if ( coverages.size() != max_output_ ) {
        continue;
      }
      hit_limit_ = true;
      for ( const auto& i : dfs | std::views::reverse ) {
        uncover_type( i.option );
      }
      return coverages;
    }

    const uint64_t next_to_cover = choose_item();
    if ( !next_to_cover || choice_limit <= 0 ) {
      continue;
    }
    // We will know we encountered this branch for the first time if it does not have a score.
    dfs.emplace_back( next_to_cover, next_to_cover, std::optional<Encoding_score> {} );
  }
  return coverages;
}

std::set<Ranked_set<Type_encoding>> Pokemon_links::exact_coverages_functional( int choice_limit )
{
  std::set<Ranked_set<Type_encoding>> coverages = {};
  Ranked_set<Type_encoding> coverage {};
  hit_limit_ = false;
  exact_dlx_functional( coverages, coverage, choice_limit );
  return coverages;
}

void Pokemon_links::exact_dlx_functional( std::set<Ranked_set<Type_encoding>>& coverages, // NOLINT
                                          Ranked_set<Type_encoding>& coverage,
                                          int depth_limit )
{
  if ( item_table_[0].right == 0 && depth_limit >= 0 ) {
    coverages.insert( coverage );
    return;
  }
  // Depth limit is either the size of a Pokemon Team or the number of attack slots on a team.
  if ( depth_limit <= 0 ) {
    return;
  }
  const uint64_t item_to_cover = choose_item();
  // An item has become inaccessible due to our chosen options so far, undo.
  if ( !item_to_cover ) {
    return;
  }
  for ( uint64_t cur = links_[item_to_cover].down; cur != item_to_cover; cur = links_[cur].down ) {
    const Encoding_score score = cover_type( cur );
    static_cast<void>( coverage.insert( score.score, score.name ) );

    exact_dlx_functional( coverages, coverage, static_cast<int>( depth_limit - 1 ) );

    /* It is possible for these algorithms to produce many many sets. To make the Pokemon
     * Planner GUI more usable I cut off recursion if we are generating too many sets.
     */
    if ( coverages.size() == max_output_ ) {
      hit_limit_ = true;
      uncover_type( cur );
      return;
    }
    static_cast<void>( coverage.erase( score.score, score.name ) );
    uncover_type( cur );
  }
}

Pokemon_links::Encoding_score Pokemon_links::cover_type( uint64_t index_in_option )
{
  Encoding_score result = {};
  uint64_t i = index_in_option;
  bool row_lap = false;
  while ( !row_lap ) {
    const int top = links_[i].top_or_len;
    /* This is the next spacer node for the next option. We now know how to find the title of
     * our current option if we go back to the start of the chosen option and go left.
     */
    if ( top <= 0 ) {
      row_lap = ( i = links_[i].up ) == index_in_option;
      result.name = option_table_[std::abs( links_[i - 1].top_or_len )].name;
      continue;
    }
    if ( !links_[top].tag ) {
      const Type_name cur = item_table_[top];
      item_table_[cur.left].right = cur.right;
      item_table_[cur.right].left = cur.left;
      hide_options( i );
      /* If there is a better way to score the teams or attack schemes we build here would
       * be the place to change it. I just give points based on how good the resistance or
       * attack strength is. Immunity is better than quarter is better than half damage if
       * we are building defense. Quad is better than double damage if we are building
       * attack types. Points only change by increments of one. Seems fine?
       */
      result.score += links_[i].multiplier;
    }
    row_lap = ++i == index_in_option;
  }
  return result;
}

void Pokemon_links::uncover_type( uint64_t index_in_option )
{
  // Go left first so the in place link restoration of the doubly linked lookup table works.
  uint64_t i = --index_in_option;
  bool row_lap = false;
  while ( !row_lap ) {
    const int top = links_[i].top_or_len;
    if ( top <= 0 ) {
      row_lap = ( i = links_[i].down ) == index_in_option;
      continue;
    }
    if ( !links_[top].tag ) {
      const Type_name cur = item_table_[top];
      item_table_[cur.left].right = top;
      item_table_[cur.right].left = top;
      unhide_options( i );
    }
    row_lap = --i == index_in_option;
  }
}

/* The hide/unhide technique is what makes exact cover so much more restrictive and fast at
 * shrinking the problem. Notice how aggressively it eliminates the appearances of items across
 * other options. When compared to Overlapping Coverage, Exact Coverage answers a different
 * question but also shrinks the problem much more quickly.
 */

void Pokemon_links::hide_options( uint64_t index_in_option )
{
  for ( uint64_t row = links_[index_in_option].down; row != index_in_option; row = links_[row].down ) {
    if ( static_cast<int>( row ) == links_[index_in_option].top_or_len ) {
      continue;
    }
    for ( uint64_t col = row + 1; col != row; ) {
      const int top = links_[col].top_or_len;
      if ( top <= 0 ) {
        col = links_[col].up;
        continue;
      }
      // Some items may be hidden at any point by the user.
      if ( !links_[top].tag ) {
        const Poke_link cur = links_[col];
        links_[cur.up].down = cur.down;
        links_[cur.down].up = cur.up;
        --links_[top].top_or_len;
      }
      ++col;
    }
  }
}

void Pokemon_links::unhide_options( uint64_t index_in_option )
{
  for ( uint64_t row = links_[index_in_option].up; row != index_in_option; row = links_[row].up ) {
    if ( static_cast<int>( row ) == links_[index_in_option].top_or_len ) {
      continue;
    }
    for ( uint64_t col = row - 1; col != row; ) {
      const int top = links_[col].top_or_len;
      if ( top <= 0 ) {
        col = links_[col].down;
        continue;
      }
      // Some items may be hidden at any point by the user.
      if ( !links_[top].tag ) {
        const Poke_link cur = links_[col];
        links_[cur.up].down = col;
        links_[cur.down].up = col;
        ++links_[top].top_or_len;
      }
      --col;
    }
  }
}

/* * * * * * * * * * * *  Shared Choosing Heuristic for Both Techniques * * * * * * * * * * * * * */

uint64_t Pokemon_links::choose_item() const
{
  int32_t min = INT32_MAX;
  uint64_t chosen_index = 0;
  for ( uint64_t cur = item_table_[0].right; cur != 0; cur = item_table_[cur].right ) {
    // No way to reach this item. Bad past choices or impossible to solve.
    if ( links_[cur].top_or_len <= 0 ) {
      return 0;
    }
    if ( links_[cur].top_or_len < min ) {
      chosen_index = cur;
      min = links_[cur].top_or_len;
    }
  }
  return chosen_index;
}

/* * * * * * * * * * * *   Overlapping Coverage via Dancing Links   * * * * * * * * * * * * * * * */

std::set<Ranked_set<Type_encoding>> Pokemon_links::overlapping_coverages_stack( int choice_limit )
{
  hit_limit_ = false;
  if ( choice_limit <= 0 ) {
    return {};
  }
  std::set<Ranked_set<Type_encoding>> coverages = {};
  Ranked_set<Type_encoding> coverage {};
  coverage.reserve( choice_limit );
  const uint64_t start = choose_item();
  // A true recursive stack. We will only have O(depth) branches on the stack equivalent to current search path.
  std::vector<Branch> dfs { { start, start, {} } };
  dfs.reserve( choice_limit );
  while ( !dfs.empty() ) {
    Branch& cur = dfs.back();
    // If we return down the stack to any state again, it is time to move on from this option.
    // This also ensures that proper cleanup happens when we are done with the entire search space.
    if ( cur.score ) {
      overlapping_uncover_type( cur.option );
      static_cast<void>( coverage.erase( cur.score.value().score, cur.score.value().name ) );
      ++choice_limit;
    }
    // This is a caching mechanism so that if we return to this level of recursion we will know how
    // many options we have tried already. See the for loop in the functional version if this is confusing.
    cur.option = links_[cur.option].down;
    if ( cur.option == cur.item ) {
      dfs.pop_back();
      continue;
    }
    cur.score = overlapping_cover_type( { cur.option, choice_limit } );
    static_cast<void>( coverage.insert( cur.score.value().score, cur.score.value().name ) );
    --choice_limit;

    if ( item_table_[0].right == 0 && choice_limit >= 0 ) {
      coverages.insert( coverage );
      if ( coverages.size() != max_output_ ) {
        continue;
      }
      hit_limit_ = true;
      for ( const auto& i : dfs | std::views::reverse ) {
        overlapping_uncover_type( i.option );
      }
      return coverages;
    }

    const uint64_t next_to_cover = choose_item();
    if ( !next_to_cover || choice_limit <= 0 ) {
      continue;
    }
    // We will know we encountered this branch for the first time if it does not have a score.
    dfs.emplace_back( next_to_cover, next_to_cover, std::optional<Encoding_score> {} );
  }
  return coverages;
}

std::set<Ranked_set<Type_encoding>> Pokemon_links::overlapping_coverages_functional( int choice_limit )
{
  std::set<Ranked_set<Type_encoding>> coverages = {};
  Ranked_set<Type_encoding> coverage = {};
  hit_limit_ = false;
  overlapping_dlx_recursive( coverages, coverage, choice_limit );
  return coverages;
}

void Pokemon_links::overlapping_dlx_recursive( std::set<Ranked_set<Type_encoding>>& coverages, // NOLINT
                                               Ranked_set<Type_encoding>& coverage,
                                               int depth_tag )
{
  if ( item_table_[0].right == 0 && depth_tag >= 0 ) {
    coverages.insert( coverage );
    return;
  }
  if ( depth_tag <= 0 ) {
    return;
  }
  // In certain generations certain types have no weaknesses so we might return 0 here.
  const uint64_t item_to_cover = choose_item();
  if ( !item_to_cover ) {
    return;
  }

  for ( uint64_t cur = links_[item_to_cover].down; cur != item_to_cover; cur = links_[cur].down ) {
    const Encoding_score score = overlapping_cover_type( { cur, depth_tag } );
    static_cast<void>( coverage.insert( score.score, score.name ) );

    overlapping_dlx_recursive( coverages, coverage, static_cast<int>( depth_tag - 1 ) );

    /* It is possible for these algorithms to produce many many sets. To make the Pokemon
     * Planner GUI more usable I cut off recursion if we are generating too many sets.
     */
    if ( coverages.size() == max_output_ ) {
      hit_limit_ = true;
      overlapping_uncover_type( cur );
      return;
    }
    static_cast<void>( coverage.erase( score.score, score.name ) );
    overlapping_uncover_type( cur );
  }
}

/* Overlapping cover is much simpler at the cost of generating a tremendous number of solutions. We
 * only need to know which items and options are covered at which recursive levels because we are
 * more relaxed about leaving options available after items in those options have been covered by
 * other options.
 */

Pokemon_links::Encoding_score Pokemon_links::overlapping_cover_type( Pokemon_links::Cover_tag tag )
{
  uint64_t i = tag.index;
  bool row_lap = false;
  Encoding_score result = {};
  while ( !row_lap ) {
    const int top = links_[i].top_or_len;
    if ( top <= 0 ) {
      row_lap = ( i = links_[i].up ) == tag.index;
      result.name = option_table_[std::abs( links_[i - 1].top_or_len )].name;
      continue;
    }
    if ( !links_[top].tag ) {
      links_[top].tag = tag.tag;
      item_table_[item_table_[top].left].right = item_table_[top].right;
      item_table_[item_table_[top].right].left = item_table_[top].left;
      result.score += links_[i].multiplier;
    }
    if ( links_[top].tag != hidden ) {
      links_[i].tag = tag.tag;
    }
    row_lap = ++i == tag.index;
  }
  return result;
}

void Pokemon_links::overlapping_uncover_type( uint64_t index_in_option )
{
  uint64_t i = --index_in_option;
  bool row_lap = false;
  while ( !row_lap ) {
    const int top = links_[i].top_or_len;
    if ( top < 0 ) {
      row_lap = ( i = links_[i].down ) == index_in_option;
      continue;
    }
    if ( links_[top].tag == links_[i].tag ) {
      links_[top].tag = 0;
      item_table_[item_table_[top].left].right = top;
      item_table_[item_table_[top].right].left = top;
    }
    if ( links_[top].tag != hidden ) {
      links_[i].tag = 0;
    }
    row_lap = --i == index_in_option;
  }
}

/* * * * * * * * * * * * * * * * *        Utility Functions             * * * * * * * * * * * * * */

bool Pokemon_links::reached_output_limit() const
{
  return hit_limit_;
}

uint64_t Pokemon_links::get_num_items() const
{
  return num_items_;
}

uint64_t Pokemon_links::get_num_options() const
{
  return num_options_;
}

Pokemon_links::Coverage_type Pokemon_links::get_links_type() const
{
  return requested_cover_solution_;
}

std::vector<Type_encoding> Pokemon_links::get_items() const
{
  std::vector<Type_encoding> result = {};
  for ( uint64_t i = item_table_[0].right; i != 0; i = item_table_[i].right ) {
    result.push_back( item_table_[i].name );
  }
  return result;
}

std::vector<Type_encoding> Pokemon_links::get_hid_items() const
{
  std::vector<Type_encoding> result = {};
  result.reserve( hidden_items_.size() );
  for ( const auto& i : hidden_items_ ) {
    result.push_back( item_table_[i].name );
  }
  return result;
}

std::vector<Type_encoding> Pokemon_links::get_options() const
{
  std::vector<Type_encoding> result = {};
  // Hop from row title to row title, skip hidden options. Skip bookend node that is placeholder.
  for ( uint64_t i = item_table_.size(); i < links_.size() - 1; i = links_[i].down + 1 ) {
    if ( links_[i].tag != hidden ) {
      result.push_back( option_table_[std::abs( links_[i].top_or_len )].name );
    }
  }
  return result;
}

std::vector<Type_encoding> Pokemon_links::get_hid_options() const
{
  std::vector<Type_encoding> result = {};
  result.reserve( hidden_options_.size() );
  for ( const auto& i : hidden_options_ ) {
    result.push_back( option_table_[std::abs( links_[i].top_or_len )].name );
  }
  return result;
}

bool Pokemon_links::hide_requested_item( Type_encoding to_hide )
{
  const uint64_t lookup_index = find_item_index( to_hide );
  // Can't find or this item has already been hidden.
  if ( lookup_index && links_[lookup_index].tag != hidden ) {
    hidden_items_.push_back( lookup_index );
    hide_item( lookup_index );
    return true;
  }
  return false;
}

bool Pokemon_links::hide_requested_item( const std::vector<Type_encoding>& to_hide )
{
  bool result = true;
  for ( const auto& t : to_hide ) {
    if ( !hide_requested_item( t ) ) {
      result = false;
    }
  }
  return result;
}

bool Pokemon_links::hide_requested_item( const std::vector<Type_encoding>& to_hide,
                                         std::vector<Type_encoding>& failed_to_hide )
{
  bool result = true;
  for ( const auto& t : to_hide ) {
    if ( !hide_requested_item( t ) ) {
      result = false;
      failed_to_hide.push_back( t );
    }
  }
  return result;
}

void Pokemon_links::hide_all_items_except( const std::set<Type_encoding>& to_keep )
{
  for ( uint64_t i = item_table_[0].right; i != 0; i = item_table_[i].right ) {
    if ( !to_keep.contains( item_table_[i].name ) ) {
      hidden_items_.push_back( i );
      hide_item( i );
    }
  }
}

bool Pokemon_links::has_item( Type_encoding item ) const
{
  const uint64_t found = find_item_index( item );
  return found && links_[found].tag != hidden;
}

void Pokemon_links::pop_hid_item()
{
  if ( !hidden_items_.empty() ) {
    unhide_item( hidden_items_.back() );
    hidden_items_.pop_back();
  } else {
    std::cout << "No hidden items. Stack is empty.\n";
    throw;
  }
}

Type_encoding Pokemon_links::peek_hid_item() const
{
  if ( !hidden_items_.empty() ) {
    return item_table_[hidden_items_.back()].name;
  }
  std::cout << "No hidden items. Stack is empty.\n";
  throw;
}

bool Pokemon_links::hid_items_empty() const
{
  return hidden_items_.empty();
}

uint64_t Pokemon_links::get_num_hid_items() const
{
  return hidden_items_.size();
}

void Pokemon_links::reset_items()
{
  while ( !hidden_items_.empty() ) {
    unhide_item( hidden_items_.back() );
    hidden_items_.pop_back();
  }
}

bool Pokemon_links::hide_requested_option( Type_encoding to_hide )
{
  const uint64_t lookup_index = find_option_index( to_hide );
  // Couldn't find or this option has already been hidden.
  if ( lookup_index && links_[lookup_index].tag != hidden ) {
    hidden_options_.push_back( lookup_index );
    hide_option( lookup_index );
    return true;
  }
  return false;
}

bool Pokemon_links::hide_requested_option( const std::vector<Type_encoding>& to_hide )
{
  bool result = true;
  for ( const auto& h : to_hide ) {
    if ( !hide_requested_option( h ) ) {
      result = false;
    }
  }
  return result;
}

bool Pokemon_links::hide_requested_option( const std::vector<Type_encoding>& to_hide,
                                           std::vector<Type_encoding>& failed_to_hide )
{
  bool result = true;
  for ( const auto& h : to_hide ) {
    if ( !hide_requested_option( h ) ) {
      failed_to_hide.push_back( h );
      result = false;
    }
  }
  return result;
}

void Pokemon_links::hide_all_options_except( const std::set<Type_encoding>& to_keep )
{
  // We start i at the index of the first option spacer. This is after the column headers.
  for ( uint64_t i = item_table_.size(); i < links_.size() - 1; i = links_[i].down + 1 ) {
    if ( links_[i].tag != hidden && !to_keep.contains( option_table_[std::abs( links_[i].top_or_len )].name ) ) {
      hidden_options_.push_back( i );
      hide_option( i );
    }
  }
}

bool Pokemon_links::has_option( Type_encoding option ) const
{
  const uint64_t found = find_option_index( option );
  return found && links_[found].tag != hidden;
}

void Pokemon_links::pop_hid_option()
{
  if ( !hidden_options_.empty() ) {
    unhide_option( hidden_options_.back() );
    hidden_options_.pop_back();
  } else {
    std::cout << "No hidden items. Stack is empty.\n";
    throw;
  }
}

Type_encoding Pokemon_links::peek_hid_option() const
{
  if ( !hidden_options_.empty() ) {
    // Row spacer tiles in the links hold their name as a negative index in the optionTable_
    return option_table_[std::abs( links_[hidden_options_.back()].top_or_len )].name;
  }
  return Type_encoding { "" };
}

bool Pokemon_links::hid_options_empty() const
{
  return hidden_options_.empty();
}

uint64_t Pokemon_links::get_num_hid_options() const
{
  return hidden_options_.size();
}

void Pokemon_links::reset_options()
{
  while ( !hidden_options_.empty() ) {
    unhide_option( hidden_options_.back() );
    hidden_options_.pop_back();
  }
}

void Pokemon_links::reset_items_options()
{
  reset_items();
  reset_options();
}

void Pokemon_links::hide_item( uint64_t header_index )
{
  const Type_name cur_item = item_table_[header_index];
  item_table_[cur_item.left].right = cur_item.right;
  item_table_[cur_item.right].left = cur_item.left;
  links_[header_index].tag = hidden;
  num_items_--;
}

void Pokemon_links::unhide_item( uint64_t header_index )
{
  const Type_name cur_item = item_table_[header_index];
  item_table_[cur_item.left].right = header_index;
  item_table_[cur_item.right].left = header_index;
  links_[header_index].tag = 0;
  num_items_++;
}

void Pokemon_links::hide_option( uint64_t row_index )
{
  links_[row_index].tag = hidden;
  for ( uint64_t i = row_index + 1; links_[i].top_or_len > 0; ++i ) {
    const Poke_link cur = links_[i];
    links_[cur.up].down = cur.down;
    links_[cur.down].up = cur.up;
    links_[cur.top_or_len].top_or_len--;
  }
  num_options_--;
}

void Pokemon_links::unhide_option( uint64_t row_index )
{
  links_[row_index].tag = 0;
  for ( uint64_t i = row_index + 1; links_[i].top_or_len > 0; ++i ) {
    const Poke_link cur = links_[i];
    links_[cur.up].down = i;
    links_[cur.down].up = i;
    ++links_[cur.top_or_len].top_or_len;
  }
  ++num_options_;
}

uint64_t Pokemon_links::find_item_index( Type_encoding item ) const
{
  for ( uint64_t nremain = item_table_.size(), base = 0; nremain != 0; nremain >>= 1 ) {
    const uint64_t cur_index = base + ( nremain >> 1 );
    if ( item_table_[cur_index].name == item ) {
      // This is the index where we can find the header for this items column.
      return cur_index;
    }
    if ( item > item_table_[cur_index].name ) {
      base = cur_index + 1;
      nremain--;
    }
  }
  // We know zero holds no value in the itemTable_ and this can double as a falsey value.
  return 0;
}

uint64_t Pokemon_links::find_option_index( Type_encoding option ) const
{
  for ( uint64_t nremain = option_table_.size(), base = 0; nremain != 0; nremain >>= 1 ) {
    const uint64_t cur_index = base + ( nremain >> 1 );
    if ( option_table_[cur_index].name == option ) {
      // This is the index corresponding to the spacer node for an option in the links.
      return option_table_[cur_index].index;
    }
    if ( option > option_table_[cur_index].name ) {
      base = cur_index + 1;
      nremain--;
    }
  }
  // We know zero holds no value in the optionTable_ and this can double as a falsey value.
  return 0;
}

/* * * * * * * * * * * * * * * * *   Constructors and Links Build       * * * * * * * * * * * * * */

Pokemon_links::Pokemon_links( const std::map<Type_encoding, std::set<Resistance>>& type_interactions,
                              const Coverage_type requested_cover_solution )
  : requested_cover_solution_( requested_cover_solution )
{
  if ( requested_cover_solution == defense ) {
    build_defense_links( type_interactions );
  } else if ( requested_cover_solution == attack ) {
    build_attack_links( type_interactions );
  } else {
    std::cerr << "Invalid requested cover solution. Choose ATTACK or DEFENSE.\n";
    std::abort();
  }
}

Pokemon_links::Pokemon_links( const std::map<Type_encoding, std::set<Resistance>>& type_interactions,
                              const std::set<Type_encoding>& attack_types )
  : requested_cover_solution_( defense )
{
  if ( attack_types.empty() ) {
    build_defense_links( type_interactions );
  } else {

    /* If we want altered attack types to defend against, it is more efficient and explicit
     * to pass in their own set then eliminate them from the Generation map by making a
     * smaller copy.
     */

    std::map<Type_encoding, std::set<Resistance>> modified_interactions = {};
    for ( const auto& type : type_interactions ) {
      modified_interactions[type.first] = {};
      for ( const Resistance& t : type.second ) {
        if ( attack_types.contains( t.type() ) ) {
          modified_interactions[type.first].insert( t );
        }
      }
    }
    build_defense_links( modified_interactions );
  }
}

void Pokemon_links::build_defense_links( const std::map<Type_encoding, std::set<Resistance>>& type_interactions )
{
  // We always must gather all attack types available in this query
  std::set<Type_encoding> generation_types = {};
  for ( const Resistance& res : type_interactions.begin()->second ) {
    generation_types.insert( res.type() );
  }

  std::unordered_map<Type_encoding, uint64_t> column_builder = {};
  option_table_.push_back( { Type_encoding( "" ), 0 } );
  item_table_.push_back( { Type_encoding( "" ), 0, 1 } );
  links_.push_back( { 0, 0, 0, emp, 0 } );
  uint64_t index = 1;
  for ( const Type_encoding& type : generation_types ) {

    column_builder[type] = index;

    item_table_.push_back( { type, index - 1, index + 1 } );
    ++item_table_[0].left;

    links_.push_back( { 0, index, index, emp, 0 } );

    ++num_items_;
    ++index;
  }
  item_table_[item_table_.size() - 1].right = 0;

  initialize_columns( type_interactions, column_builder, requested_cover_solution_ );
}

void Pokemon_links::initialize_columns( const std::map<Type_encoding, std::set<Resistance>>& type_interactions,
                                        std::unordered_map<Type_encoding, uint64_t>& column_builder,
                                        Coverage_type requested_coverage )
{
  uint64_t previous_set_size = links_.size();
  uint64_t current_links_index = links_.size();
  int32_t type_lookup_index = 1;
  for ( const auto& type : type_interactions ) {

    const uint64_t type_title = current_links_index;
    int set_size = 0;
    // We will lookup our defense options in a seperate array with an O(1) index.
    links_.push_back(
      { -type_lookup_index, current_links_index - previous_set_size, current_links_index, emp, 0 } );
    option_table_.push_back( { type.first, current_links_index } );

    for ( const Resistance& single_type : type.second ) {

      /* Important consideration for this algorithm. I am only interested in damage
       * resistances better than normal. So "covered" for a pokemon team means you found at
       * most 6 Pokemon that give you some level of resistance to all types in the game and
       * no pokemon on your team overlap by resisting the same types. You could have Pokemon
       * with x0.0, x0.25, or x0.5 resistances, but no higher. Maybe we could lessen criteria?
       * Also, just flip this condition for the ATTACK version. We want damage better than
       * Normal, meaining x2 or x4.
       */

      if ( ( requested_coverage == defense ? single_type.multiplier() < nrm : nrm < single_type.multiplier() ) ) {
        ++current_links_index;
        ++links_[type_title].down;
        ++set_size;

        const Type_encoding s_type = single_type.type();
        ++links_[links_[column_builder[s_type]].down].top_or_len;

        // A single item in a circular doubly linked list points to itself.
        links_.push_back( { static_cast<int>( links_[column_builder[s_type]].down ),
                            current_links_index,
                            current_links_index,
                            single_type.multiplier(),
                            0 } );

        // This is the adjustment to the column header's up field for a given item.
        links_[links_[column_builder[s_type]].down].up = current_links_index;
        // The current node is the new tail in a vertical circular linked list for an item.
        links_[current_links_index].up = column_builder[s_type];
        links_[current_links_index].down = links_[column_builder[s_type]].down;
        // Update the old tail to reflect the new addition of an item in its option.
        links_[column_builder[s_type]].down = current_links_index;
        // Similar to a previous/current coding pattern but in an above/below column.
        column_builder[s_type] = current_links_index;
      }
    }
    ++type_lookup_index;
    ++current_links_index;
    ++num_options_;
    previous_set_size = set_size;
  }
  links_.push_back( { INT_MIN, current_links_index - previous_set_size, UINT64_MAX, emp, 0 } );
}

void Pokemon_links::build_attack_links( const std::map<Type_encoding, std::set<Resistance>>& type_interactions )
{
  option_table_.push_back( { Type_encoding( "" ), 0 } );
  item_table_.push_back( { Type_encoding( "" ), 0, 1 } );
  links_.push_back( { 0, 0, 0, emp, 0 } );
  uint64_t index = 1;

  /* An inverted map has the attack types as the keys and the damage they do to defensive types
   * as the set of Resistances. Once this is built just use the same builder function for cols.
   */

  std::map<Type_encoding, std::set<Resistance>> inverted_map = {};
  std::unordered_map<Type_encoding, uint64_t> column_builder = {};
  for ( const auto& interaction : type_interactions ) {
    column_builder[interaction.first] = index;
    item_table_.push_back( { interaction.first, index - 1, index + 1 } );
    ++item_table_[0].left;
    links_.push_back( { 0, index, index, emp, 0 } );
    ++num_items_;
    ++index;
    for ( const Resistance& atk : interaction.second ) {
      inverted_map[atk.type()].insert( { interaction.first, atk.multiplier() } );
    }
  }
  item_table_[item_table_.size() - 1].right = 0;
  initialize_columns( inverted_map, column_builder, requested_cover_solution_ );
}

} // namespace Dancing_links
