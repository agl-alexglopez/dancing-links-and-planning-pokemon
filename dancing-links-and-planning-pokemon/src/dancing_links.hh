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
 * File: DancingLinks.h
 * --------------------------
 * This class defines how to interact with a Pokemon_links object to solve the Pokemon Type Coverage
 * Problem with Donald Knuth's Algorithm X via Dancing Links. At the most abstract level, an exact
 * coverage is one in which we are able to cover each item in a set of items exactly once by
 * choosing options that contain those items. No two options that we pick can cover the same item.
 *
 * I apply this concept to the videogame Pokemon. In Pokemon, there are fundamental nature-related
 * types like Fire, Water, Flying, Fighting and many others. Depending on the Pokemon Generation
 * there can be anywhere from 15 to 18 of these single types. These types act as a large and
 * nuanced layer of complexity that goes over what is fundametally a game of rock-paper-scissors.
 * Types have advantages of over types or are weak to other types. Pokemon can themselves BE these
 * fundamental types. However, they may take on two types, such as Fire-Flying or Bug-Flying. While
 * these fundamental types could be combined to form 306 unique dual types, not to mention the
 * additional 15 to 18 single types, the developers of this game have not done so yet. Depending on
 * the generation, there are far fewer types than this, but there are still many. The most recent
 * release is up to 162 unique pokemon types.
 *
 * For the full ruleset of Pokemon look elswhere, but in brief what I do in this problem is
 * determine if there are Perfect and/or Overlapping type coverages for given sets of Attack and
 * Defense types. Attack types can only be those 15 to 18 single types mentioned earlier and
 * defensive types are these sames types with the additional possiblity of combining two types.
 *
 * An exact cover for Defense uses the following rules to determine if such a cover is possible.
 *
 *      - We may pick a maximum of 6 Pokemon for a team.
 *      - We defend ourselves from 15 to 18 attack types, depending on the generation of Pokemon.
 *      - Pick at most 6 pokemon that have resistances to every attack type exactly once.
 *      - Resistance means that when an attack type is used on our chosen type it does no more
 *        than x0.5 the damage of a normal attack. So, across all of our chosen pokemon can we
 *        be resistant to every attack type exactly once?
 *      - Some Resistances are better than others. For example, it is possible for a type to be
 *        immune to other types x0.0, very resistant x0.25, or resistant x0.5. These resistances
 *        are scored with x0.0 being given 1 point, x0.25 given 2 points, and x0.5 given 3 points.
 *        The goal is to keep the lowest score possible, NOT the highest.
 *      - Viable teams that we find will be ranked according to the scoring system described above.
 *
 * An exact cover for Attack uses the following rules to determine if an exact cover is possible.
 *
 *      - Every pokemon in a team can learn 4 attacks.
 *      - Attacks are only of a single type. There are no attacks that do two simultaneous types of
 *        damage.
 *      - This means that we can choose 24 attacks. This will never happen because we are always
 *        limited by the maximum number of attack types (15 to 18). So effectively, there is no
 *        depth limit for this search as we can choose every attack at most.
 *      - Our choice of attack types must be effective against every defensive type we are tasked
 *        with damaging exactly once. No two attack types may be effective against the same
 *        defensive type.
 *      - Attacks may be super-effective in two ways against another type: they do double damage x2
 *        or quadruple damage x4. These multipliers are scored with x2 damage earning 5 points and
 *        x4 damage earning 6 points. A higher score is better in this case, NOT a lower score.
 *      - Viable attack type selections will be ranked by the scoring system described above.
 *
 * An overlapping cover is another way to look at both of these problems. An overlapping cover means
 * that we allow overlap in the options we choose covering some of the same items. For example,
 * in the defensive case, our goal is to simply choose at most 6 Pokemon that will resist all
 * attack types in the game, we do not care about how much overlap there is in coverage between
 * the defensive types we choose. We only care about being resistant to all damage types across our
 * entire team.
 *
 * For the Overlapping version of this problem, we use the exact same scoring schemes for Defense
 * and Attack. Overlapping cover will produce a tremendous number of possibilities in some cases
 * so I have limited how long this algorithm can run with a maximum output. I have run out of
 * memory when generating combinations before and could not tell you how many Overlapping covers
 * are possible for some Pokemon Generations. Exact cover, in contrast, is usually much more
 * reasonable to determine because it is much more difficult to achieve. For an even more detailed
 * writeup please see the README.md for this repository, where I use images to help describe the
 * process.
 *
 * For more information on Algorithm X via Dancing Links as Knuth describes it please read his work.
 *
 *      The Art of Computer Programming,
 *      Volume 4B,
 *      Combinatorial Algorithms, Part 2,
 *      Sec. 7.2.2.1,
 *      Pg.65-70,
 *      Knuth
 */
#ifndef DANCING_LINKS_HH
#define DANCING_LINKS_HH
#include "pokemon_links.hh"
#include <cstdint>

namespace Dancing_links {
class Pokemon_links;

/* * * * * * * *        Free Functions for Client to Use with DLX Solvers       * * * * * * * * * */

/**
 * @brief solveExact_cover  an exact type coverage is one in which every "option" we choose
 *                         to cover a given set of "items" will cover each item exactly
 *                         once. For example, if building a team of Pokemon to defend
 *                         against attack types, no two Pokemon in that team could be
 *                         resistant to the same attack types. Sets are ranked by the
 *                         following criteria:
 *
 *                             For forming defensive teams we score types based on their
 *                             resistance to each attack type as follows.
 *
 *                                 - x0.0 multiplier is 1 point.
 *                                 - x0.25 multiplier is 2 points.
 *                                 - x0.50 multiplier is 3 points.
 *                                 - x1.0 multiplier or higher is not considered.
 *
 *                             The lower the score the stronger that typing is. For
 *                             forming our choices of attack types we score based on their
 *                             damage to each attack type as follows.
 *
 *                                 - x2 multiplier is 4 points.
 *                                 - x4 multiplier is 5 points.
 *                                 - x1.0 or lower is not considered.
 *
 *                             The higher the score the stronger those choices of attack
 *                             types are for attacking the selected defensive types.
 *
 * @return                 the set of Ranked Sets that form all solutions of exact cover.
 */
std::set<Ranked_set<Type_encoding>> solve_exact_cover( Pokemon_links& dlx, int8_t choice_limit );

/**
 * @brief solve_overlapping_cover an overlapping coverage is when we cover every "item"
 *                                with our choices of "options." It is allowable for two
 *                                options cover the same item twice, the goal is to cover
 *                                the items with any allowable choices. The scoring scheme
 *                                for generated sets is the same as described in the
 *                                exact cover version of the problem. Currently, this
 *                                solution is slow because it generates duplicate
 *                                Ranked_set solutions. I filter out duplicates by using a
 *                                set. I have not yet found a way to prevent generating
 *                                duplicate solutions with the Dancing Links method.
 * @return                        the set of Ranked Sets that form all overlapping covers.
 */
std::set<Ranked_set<Type_encoding>> solve_overlapping_cover( Pokemon_links& dlx, int8_t choice_limit );

/**
 * @brief has_max_solutions  exact and overlapping cover solutions are limited at a large number
 *                         for usability if entered into a GUI application. If we have
 *                         generated our maximum output the class will report this fact. This
 *                         means there may have been more solutions but they weren't generated.
 * @param dlx              the Pokemon_links object used to solve exact cover problems.
 * @return                 true if we reached the max limit false if not O(1).
 */
bool has_max_solutions( const Pokemon_links& dlx );

/**
 * @brief items  reports the items in the cover problem as a vector of types. May be attack types
 *               or defense types depending on the requested cover solution on constructing class.
 * @param dlx    the Pokemon_links object that is instantiated with a requested cover solution.
 * @return       the vector of types that are our items we must cover in the problem.
 */
std::vector<Type_encoding> items( const Pokemon_links& dlx );

/**
 * @brief numItems  the number of items that need cover in the current Pokemon_links object.
 * @param dlx       the Pokemon_links object that uses options to cover items.
 * @return          the int number of items to cover.
 */
uint64_t num_items( const Pokemon_links& dlx );

/**
 * @brief has_item  returns true if an item is contained the object. An item that is hidden CANNOT
 *                 be found by a membership test, it is hidden. O(lgN) where N is items.
 * @param dlx      the Pokemon_links object we examine.
 * @param item     the item we are trying to find.
 * @return         true if the item is present and not hidden false if not.
 */
bool has_item( const Pokemon_links& dlx, Type_encoding item );

/**
 * @brief options  reports the options available to us to cover our items in the cover problem.
 * @param dlx      the Pokemon_links object that is instantiated with a requested cover solution.
 * @return         the vector of types that are our options we can use to cover the items.
 */
std::vector<Type_encoding> options( const Pokemon_links& dlx );

/**
 * @brief num_options  the number of options we can choose from to cover the total items.
 * @param dlx         the Pokemon_links object that uses options to cover items.
 * @return            the int number of options we can choose from.
 */
int num_options( const Pokemon_links& dlx );

/**
 * @brief has_option  returns true if an option is contained in the object. An option that is hidden
 *                   CANNOT be found by a membership test. O(lgN) where N is options.
 * @param dlx        the Pokemon_links object we examine.
 * @param option     the option we are searching for.
 * @return           true if found and not hidden false if not.
 */
bool has_option( const Pokemon_links& dlx, Type_encoding option );

/**
 * @brief coverage_type  the coverage type the Pokemon_links is set to.
 * @param dlx           the Pokemon_links we have instantiated.
 * @return              ATTACK or DEFENSE, depeding on the request when building the class.
 */
Pokemon_links::Coverage_type coverage_type( const Pokemon_links& dlx );

/**
 * @brief hide_item  removes an item from the dancing links so that it no longer exists in any cover
 *                  problem. This is different than covering an item in Algorithm X. Here, the user
 *                  is requesting that only this item disappear. All other options that contain this
 *                  item retain their other items and remain in the world. We can restore this item
 *                  into the world later if desired. Interact with hidden items as you would a
 *                  stack. We must unhide the top of the stack of hidden items first. In place,
 *                  O(lgN) guarantee.
 * @param dlx       the Pokemon_links object we alter in place.
 * @param to_hide    the type item representing the item to hide depending on ATTACK or DEFENSE.
 * @return          true if the item was hidden false if it was already hidden or not found.
 */
bool hide_item( Pokemon_links& dlx, Type_encoding to_hide );

/**
 * @brief hide_item  hides all items specified from the vector as above. In place, O(NlgN) guarantee.
 * @param dlx       the Pokemon_links object we alter in place.
 * @param toHide    the type item representing the item to hide depending on ATTACK or DEFENSE.
 * @return          true if all items were hidden false if at least one was not.
 */
bool hide_item( Pokemon_links& dlx, const std::vector<Type_encoding>& to_hide );

/**
 * @brief hide_item      hides items specified from the vector as above. In place, O(NlgN) guarantee.
 * @param dlx           the Pokemon_links object we alter in place.
 * @param to_hide        the type representing the item to hide depending on ATTACK or DEFENSE.
 * @param failed_to_hide  an additional output parameter if user wants to see failures.
 * @return              true if all items were hidden false if at least one was not found.
 */
bool hide_item( Pokemon_links& dlx,
                const std::vector<Type_encoding>& to_hide,
                std::vector<Type_encoding>& failed_to_hide );

/**
 * @brief hide_items_except  hides all items NOT included in the specified set. In place, O(NlgK)
 *                         N is the number of Items and K is the number of items to keep.
 * @param dlx              the Pokemon_links object we alter in place.
 * @param to_keep           the items that remain the world as items we must cover.
 */
void hide_items_except( Pokemon_links& dlx, const std::set<Type_encoding>& to_keep );

/**
 * @brief num_hid_items  returns the number of items we are currently hiding from the world. O(1).
 * @param dlx          the Pokemon_links object we examine.
 * @return             the int number of hidden items.
 */
uint64_t num_hid_items( const Pokemon_links& dlx );

/**
 * @brief peek_hid_item  hidden items act like a stack that must be unhidden Last-in-First-out.
 *                     check the most recently hid item without altering the stack. Throws an
 *                     exception is the stack is empty. O(1).
 * @param dlx          the Pokemon_links object we examine.
 * @return             the most recently hidden item.
 */
Type_encoding peek_hid_item( const Pokemon_links& dlx );

/**
 * @brief pop_hid_item  pop the most recently hidden item from the stack altering the stack. Throws
 *                    an exception if attempt is made to pop from empty stack. O(1).
 * @param dlx         the Pokemon_links object we alter.
 */
void pop_hid_item( Pokemon_links& dlx );

/**
 * @brief hid_items_empty  reports if the stack of hidden items is empty.
 * @param dlx            the Pokemon_links object we examine.
 * @return               true if empty false if not.
 */
bool hid_items_empty( const Pokemon_links& dlx );

/**
 * @brief hid_items  view the currently hidden stack as a vector. The last item is the first out.
 * @param dlx       the Pokemon_links object we alter.
 * @return          the vector of items in the order we hid them. Last is first out if popped.
 */
std::vector<Type_encoding> hid_items( const Pokemon_links& dlx );

/**
 * @brief reset_items  restores the items in the world to their original state. O(H), H hidden items.
 * @param dlx         the Pokemon_links object we alter.
 */
void reset_items( Pokemon_links& dlx );

/**
 * @brief hide_option  hides an option from the dancing links so that it no longer available to
 *                    cover items. This is a different type of hiding than in Algorithm X. Here,
 *                    the number of options available to every item covered by this option is
 *                    reduced by one and all items in this option remain in the world. They must
 *                    still be covered through other options to solve the problem. Hide options
 *                    behaves as a stack and they must be unhidden in Last-in-First-out order to
 *                    restore the world to its original state. In place, O(lgN + I) where N is the
 *                    number of options and I is the number of Items in an option.
 * @param dlx         the Pokemon_links object we alter.
 * @param to_hide      the option we must hide from the world.
 * @return            true if option was hidden false if it was hidden or could not be found.
 */
bool hide_option( Pokemon_links& dlx, Type_encoding to_hide );

/**
 * @brief hide_option  hides all options specified in the vector from the world. Uses the same
 *                    process as hide_option() for each option making an O(HlgNI) where H is the
 *                    number of options to hide, N is the number of options and I is the number of
 *                    items in an option. In practice, I is often small in sparse links.
 * @param dlx         the Pokemon_links object we alter.
 * @param to_hide      the options we must hide from the world.
 * @return            true if all options were hidden false if at least one was not.
 */
bool hide_option( Pokemon_links& dlx, const std::vector<Type_encoding>& to_hide );

/**
 * @brief hide_option    hides all options specified in the vector from the world. Uses the same
 *                      process as hide_option() for each option making an O(HlgNI) where H is the
 *                      number of options to hide, N is the number of options and I is the number of
 *                      items in an option. In practice, I is often small in sparse links.
 * @param dlx           the Pokemon_links object we alter.
 * @param to_hide        the options we must hide from the world.
 * @param failed_to_hide  an additional output showing the options that could not be found.
 * @return              true if all options were hidden false if at least one failed.
 */
bool hide_option( Pokemon_links& dlx,
                  const std::vector<Type_encoding>& to_hide,
                  std::vector<Type_encoding>& failed_to_hide );

/**
 * @brief hide_options_except  hides all options NOT specified in the given set. In place O(NlgKI)
 *                           where N is the number of options, K the number of options to keep, and
 *                           I the number of Items covered by each option. In practice I is small in
 *                           sparse Pokemon_links grids.
 * @param dlx                the Pokemon_links object we alter.
 * @param to_keep             the options we will keep available to choose from for the problem.
 */
void hide_options_except( Pokemon_links& dlx, const std::set<Type_encoding>& to_keep );

/**
 * @brief num_hid_options  num options currently hidden in the stack. The last is first out. O(1).
 * @param dlx            the Pokemon_links object we examine.
 * @return               the int representing number of hidden options in the stack.
 */
uint64_t num_hid_options( const Pokemon_links& dlx );

/**
 * @brief peek_hid_option  hidden items act like a stack that must be unhidden Last-in-First-out.
 *                       check the most recently hid item without altering the stack. O(1).
 * @param dlx            the Pokemon_links object we examine.
 * @return               the most recently hidden option.
 */
Type_encoding peek_hid_option( const Pokemon_links& dlx );

/**
 * @brief pop_hid_option  pop the most recently hidden item from stack altering the stack. O(1).
 * @param dlx           the Pokemon_links object we alter.
 */
void pop_hid_option( Pokemon_links& dlx );

/**
 * @brief hid_options_empty  reports if the stack of hidden options is empty.
 * @param dlx              the Pokemon_links object we examine.
 * @return                 true if empty false if not.
 */
bool hid_options_empty( const Pokemon_links& dlx );

/**
 * @brief hid_options  view the currently hidden stack as a vector. The last item is first-out.
 * @param dlx         the Pokemon_links object we alter.
 * @return            the vector of options in order we hid them. Last is first out if popped.
 */
std::vector<Type_encoding> hid_options( const Pokemon_links& dlx );

/**
 * @brief reset_options  restore the options in the world to original state. O(H), H hidden items.
 * @param dlx           the pokemon_links object we alter.
 */
void reset_options( Pokemon_links& dlx );

/**
 * @brief reset_all  completely restore the items and options to original state. O(I + (PC)) where
 *                  I is the number of hidden items, P is the number of hidden options and C is the
 *                  number of items covered by each option.
 * @param dlx       the Pokemon_links object we alter.
 */
void reset_all( Pokemon_links& dlx );

} // namespace Dancing_links

#endif // DANCING_LINKS_HH
