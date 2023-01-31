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
 * File: PokemonLinks.h
 * --------------------------
 * This class defines a PokemonLinks object that can be used to solve the Pokemon Type Coverage
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
#ifndef POKEMONLINKS_H
#define POKEMONLINKS_H
#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <set>
#include <utility>
#include <vector>
#include "GUI/SimpleTest.h"
#include "Utilities/Resistance.h"
#include "Utilities/RankedSet.h"


namespace DancingLinks {

/* See bottom of file for the free functions that accompany this class in DancingLinks namespace. */

class PokemonLinks {

public:

    // The user is asking us for the defensive team they should build or attacks they need.
    enum CoverageType {
        DEFENSE,
        ATTACK
    };

    /**
     * @brief PokemonLinks            this constructor builds the necessary internal data structures
     *                                to run the exact cover via dancing links algorithm. We need
     *                                to build differently based on attack or defense.
     * @param typeInteractions        map of pokemon types and their resistances to attack types.
     * @param requestedCoverSolution  ATTACK or DEFENSE. Build a team or choose attack types.
     */
    explicit PokemonLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions,
                          const CoverageType requestedCoverSolution);

    /**
     * @brief PokemonLinks      this alternative constructor is helpful when choosing a defensive
     *                          team based on a subset of attack types. For example, we could build
     *                          defenses against the attack types present at specific gyms.
     * @param typeInteractions  the map of types and their defenses for a given generation.
     * @param attackTypes       the subset of attacks we must cover with choices of Pokemon teams.
     */
    explicit PokemonLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions,
                          const std::set<std::string>& attackTypes);

    /**
     * @brief getExactCoverages  solves the exact cover problem on the initialized PokemonLinks
     *                           object with a specified limit on the number of options we can
     *                           choose to acheive an exact cover. The returned value is a set of
     *                           RankedSets because RankedSets have a natural ordering according to
     *                           their rank value. std::set orders elements in ascending order so if
     *                           you solve a DEFENSE cover, the best solutions will have the lowest
     *                           rank in the set. If you solve the ATTACK cover, the best solutions
     *                           will have the highest rank in the set.
     * @param dlx                the initialized PokemonLinks object. ATTACK or DEFENSE.
     * @param choiceLimit        the user defined limit on number of options we can use to solve.
     * @return                   all exact cover solutions found with the limit.
     */
    std::set<RankedSet<std::string>> getExactCoverages(int choiceLimit);

    /**
     * @brief getOverlappingCoverages  solves the overlapping cover problem and returns a solution
     *                                 that is organized in its set with the same principles as the
     *                                 exact cover solution. However, we allow options we choose to
     *                                 cover our items to overlap. This means two options may cover
     *                                 some of the same items. However, if chosen, an option will
     *                                 always provide value by covering an option that was not
     *                                 previously covered, even if there is some overlap with other
     *                                 options.
     * @param choiceLimit              the user defined limit on number of options we use to solve.
     * @return                         all overlapping cover solutions found with the limit.
     */
    std::set<RankedSet<std::string>> getOverlappingCoverages(int choiceLimit);

    /**
     * @brief hideRequestedItem  hiding a requested item can occur in place and be undone later.
     *                           This item will no longer need coverage in a cover problem. If the
     *                           item is not in the PokemonLinks object, it is unchanged. O(lgN)
     * @param toHide             the item we wish to cover.
     */
    bool hideRequestedItem(const std::string& toHide);

    /**
     * @brief hideRequestedItem  hides all items in the vector if successful. You cannot find an
     *                           item if it is hidden or does not exist.
     * @param toHide             the vector of items to hide.
     * @return                   true if all items are hidden false if at least one fails.
     */
    bool hideRequestedItem(const std::vector<std::string>& toHide);

    /**
     * @brief hideRequestedItem  hides all items in the vector if successful. You cannot find an
     *                           item if it is hidden or does not exist.
     * @param toHide             the vector of items to hide.
     * @param failedToHide       output of the items we failed to hide.
     * @return                   true if all items are hidden false if at least one fails.
     */
    bool hideRequestedItem(const std::vector<std::string>& toHide,
                           std::vector<std::string>& failedToHide);

    /**
     * @brief hideAllItemsExcept  hides all items EXCEPT those specified in the toKeep set. These
     *                            can all be unhidden later. Warning, if an item cannot be found in
     *                            the toKeep set, it will be hidden. O(NlgK) where N is the number
     *                            of items and K is the number of items to keep.
     * @param toKeep              the set of items we wish to keep for future cover problems.
     */
    void hideAllItemsExcept(const std::set<std::string>& toKeep);

    /**
     * @brief hasItem  searches for the requested item in the PokemonLinks items. A hidden item
     *                 cannot be found by this membership test. O(lgN) N is all items.
     * @param item     the string item we search for.
     * @return         true if we found it false if it is hidden or absent.
     */
    bool hasItem(const std::string& item) const;

    /**
     * @brief peekHidItem  peek the most recently hidden item from the stack. Throws an exception
     *                     if the stack is empty and there are no hidden items.
     * @return             the string copy of the most recent hidden item.
     */
    std::string peekHidItem() const;

    /**
     * @brief popHidItem  pops the most recently hidden item from the stack restoring it as an
     *                    item in the dancing links array. Throws an exception if the stack is
     *                    empty and there are no hidden items.
     */
    void popHidItem();

    /**
     * @brief hidItemsEmpty  reports whether there are currently hidden items.
     * @return               true if the stack of hidden items is emtpy false if not.
     */
    bool hidItemsEmpty() const;

    /**
     * @brief getHidItems  reports a vector representation of the stack of hidden items. The end
     *                     of the vector is the top of the stack.
     * @return             the vector of all hidden items.
     */
    std::vector<std::string> getHidItems() const;

    /**
     * @brief getNumHidItems  reports the number of hidden items in O(1).
     * @return                the int num of hidden items.
     */
    int getNumHidItems() const;

    /**
     * @brief resetItems  unhides all items as they were upon object construction. O(H) Hidden.
     */
    void resetItems();

    /**
     * @brief hideRequestedOption  hides the option so it cannot be used to cover items. If the
     *                             option is not present or option has already been covered the
     *                             options remain unchanged. O(lgN + C) where N is the number of
     *                             options and C is the items covered by this option.
     * @param toHide               the option to hide.
     */
    bool hideRequestedOption(const std::string& toHide);

    /**
     * @brief hideRequestedOption  hides all requested options from the links. If an option is not
     *                             found or already hidden we move to the next requested option.
     *                             O(HlgNC) where H is the number of items to hide, N is the
     *                             number of options, and C is the number of items covered by each
     *                             option. In practice C is small because links are sparse.
     * @param toHide               the vector of options we must hide.
     * @return                     true if all items are hidden false if at least one fails.
     */
    bool hideRequestedOption(const std::vector<std::string>& toHide);

    /**
     * @brief hideRequestedOption  hides all requested options. If any hide operations fail the
     *                             function continues and can report failures in the output param.
     * @param toHide               options requested to be hidden.
     * @param failedToCover        any options we failed to hide.
     * @return                     true if all options were hidden false if at least one failed.
     */
    bool hideRequestedOption(const std::vector<std::string>& toHide,
                             std::vector<std::string>& failedToCover);

    /**
     * @brief hideAllOptionsExcept  hides all options EXCEPT those specified in the keep set. By
     *                              default options that are not found are hidden so take care when
     *                              forming the set.
     * @param toKeep                the options we wish to keep for future cover problems.
     */
    void hideAllOptionsExcept(const std::set<std::string>& toKeep);

    /**
     * @brief hasOption  determines if an option is present and not hidden. Hidden options cannot
     *                   be found by this membership test. O(lgN) where N is all options.
     * @param option     the string option we search for.
     * @return           true if the item is present and not hidden, false if not.
     */
    bool hasOption(const std::string& option) const;

    /**
     * @brief peekHidOption  peeks the most recently hidden option. Throws if no hidden items.
     * @return               the string of the most recently hidden option.
     */
    std::string peekHidOption() const;

    /**
     * @brief popHidOption  pops the most recently hidden option from the stack restoring it into
     *                      the dancing links array as an option for cover problems. O(I) where
     *                      I is the number of items covered by this option.
     */
    void popHidOption();

    /**
     * @brief hidOptionsEmpty  reports if we are currently hiding any options. O(1).
     * @return                 true if there are no hidden options false if not.
     */
    bool hidOptionsEmpty() const;

    /**
     * @brief getHidOptions  view a vector representation of the hidden option stack.
     * @return               the vector of hidden options. End of the vector is the top of stack.
     */
    std::vector<std::string> getHidOptions() const;

    /**
     * @brief getNumHidOptions  reports the number of currently hidden options in the stack. O(1)
     * @return                     the int representation of the stack size.
     */
    int getNumHidOptions() const;

    /**
     * @brief resetOptions  returns all options into the dancing links grid to be available as
     *                      options for a cover problem. O(HI) where H is the number of hidden
     *                      options and I is the number of items covered by each option.
     */
    void resetOptions();

    /**
     * @brief resetItemsOptions  returns all options and items to their original state and the
     *                           PokemonLinks object returns to the state it was in when
     *                           constructed. O(I) + O(PC) where I is the number of Items, P is the
     *                           number of oPtions and C is the number of items covered by each
     *                           option.
     */
    void resetItemsOptions();

    /**
     * @brief reachedOutputLimit  for usability of Pokemon Planning application I cut off output at
     *                            a set limit because sets can exceed the size of available memory
     *                            in some cases. Use this function if you want to know if the last
     *                            query to dancing links reached the limit and you are missing
     *                            all possible sets.
     * @return                    true if the last query reached the limit, false if not.
     */
    bool reachedOutputLimit() const;

    /**
     * @brief getItems  gets the items we try to cover in the constructed attack/defense links
     * @return          a vector of strings representing these items in O(n) time.
     */
    std::vector<std::string> getItems() const;

    /**
     * @brief getNumItems  will tell you how many items you must cover in the given cover problem.
     * @return             an int representing the number of items.
     */
    int getNumItems() const;

    /**
     * @brief getOptions  gets the options we use to cover items in the attack/defense links.
     * @return            a vector of strings representing the options in O(n) time.
     */
    std::vector<std::string> getOptions() const;

    /**
     * @brief getNumOptions  tells you how many choices present to combine in order to cover all items.
     * @return               an int representing number of options.
     */
    int getNumOptions() const;

    /**
     * @brief getLinksType  returns the what the current grid is set to for a cover problem.
     * @return              ATTACK or DEFENSE depending on the cover request upon construction.
     */
    CoverageType getLinksType() const;


private:


    /* * * * * * * * * * *   Dancing Links Internals and Implementation   * * * * * * * * * * * * */


    // This type is entered into our dancing links array for the in place recursive algorithm.
    struct pokeLink {
        int topOrLen;
        int up;
        int down;
        Resistance::Multiplier multiplier; // x0.0, x0.25, x0.5, x1.0, x2, or x4 damage multipliers.
        int tag;                           // We use this to efficiently generate overlapping sets.
    };

    // This type, in a seperate vector, controls the base case of our recursion.
    struct typeName {
        std::string name;
        int left;
        int right;
    };

    // Simple, a string and an int! I use it twice when I need two pieces of info at once.
    struct strNum {
        std::string str;
        int num;
    };

    /* These data structures contain the core logic of Algorithm X via dancing links. For more
     * detailed information, see the tests in the implementation. These help acheive in place
     * recursion. We can also play around with more advanced in place techniques like hiding options
     * and items at the users request and restoring them later in place. Finally, because the option
     * table and item table are sorted lexographically we can find any option or item in O(lgN). No
     * auxillary maps are needed.
     */
    std::vector<strNum> optionTable_;       // How we know the name of the option we chose.
    std::vector<typeName> itemTable_;       // How we know the names of our items.
    std::vector<pokeLink> links_;           // The links that dance!
    std::vector<int> hiddenItems_;          // Treat as stack with user hidden Items.
    std::vector<int> hiddenOptions_;        // Treat as stack with user hidden Options.
    std::size_t maxOutput_;                 // Cutoff our solution generation for GUI usability.
    bool hitLimit_;                         // How we report to a user that we cutoff more solutions
    int numItems_;                          // What needs to be covered.
    int numOptions_;                        // Options we can choose from to cover items.
    CoverageType requestedCoverSolution_;   // The user is asking for ATTACK or DEFENSE

    static const int HIDDEN = -1;

    /**
     * @brief fillExactCoverages  fills the output parameters with every exact cover that can be
     *                            determined for defending against attack types or attacking
     *                            defensive types. Exact covers use options to cover every item
     *                            exactly once.
     * @param coverages           the output parameter that serves as the final solution.
     * @param coverage            the successfully coverages we find while the links dance.
     * @param depthLimit          size of a pokemon team or the number of attacks a team can have.
     */
    void fillExactCoverages(std::set<RankedSet<std::string>>& coverages,
                            RankedSet<std::string>& coverage,
                            int depthLimit);

    /**
     * @brief fillOverlappingCoverages  fills the output parameter with every overlapping cover that
     *                                  can be determined for defending against attack types or
     *                                  attacking defensive types. Overlapping covers use any
     *                                  number of options within their depth limit to cover all
     *                                  items. Two options covering some overlapping items is
     *                                  acceptable. This is slower and I have no way to not generate
     *                                  duplicates other than using a set. Better ideas wanted.
     * @param coverages                 the output parameter as our final solution if found.
     * @param coverage                  the helper set that fills the output parameter.
     * @param depthTag                  a tag used to signify the recursive depth. Used internally.
     */
    void fillOverlappingCoverages(std::set<RankedSet<std::string>>& coverages,
                                  RankedSet<std::string>& coverage,
                                  int depthTag);

    /**
     * @brief chooseItem  choose an item to cover that appears the least across all options. If an
     *                    item becomes inaccessible over the course of recursion I signify this by
     *                    returning 0. That branch should fail at that point.
     * @return            the index in the lookup table and headers of links_ of the item to cover.
     */
    int chooseItem() const;

    /**
     * @brief coverType      perform an exact cover as described by Donald Knuth, eliminating the
     *                       option we have chosen, covering all associated items, and eliminating
     *                       all other options that include those covered items.
     * @param indexInOption  the index in the array we use to start covering and eliminating links.
     * @return               every option we choose contributes to the strength of the RankedSet
     *                       it becomes a part of. Return the strength contribution to the set
     *                       and the name of the option we chose.
     */
    strNum coverType(int indexInOption);

    /**
     * @brief uncoverType    undoes the work of the exact cover operation returning the option,
     *                       the items it covered, and all other options that include the items we
     *                       covered back into the links.
     * @param indexInOption  the work will be undone for the same option if given same index.
     */
    void uncoverType(int indexInOption);

    /**
     * @brief hideOptions    takes the options containing the option we chose out of the links. Do
     *                       this in order to cover every item exactly once and not overlap. This
     *                       is the vertical traversal of the links.
     * @param indexInOption  the index we start at in a given option.
     */
    void hideOptions(int indexInOption);

    /**
     * @brief unhideOptions  undoes the work done by the hideOptions operation, returning the other
     *                       options containing covered items in an option back into the links.
     * @param indexInOption  the work will be undone for the same option if given same index.
     */
    void unhideOptions(int indexInOption);

    /**
     * @brief overlappingCoverType  performs a loose or "overlapping" cover of items in a dancing
     *                              links algorithm. We allow other options that cover items already
     *                              covered to stay accessible in the links leading to many more
     *                              solutions being found as multiple options can cover some of the
     *                              same items.
     * @param indexInOption         the index in array used to start covering and eliminating links.
     * @param depthTag              to perform this type of coverage I use a depth tag to know which
     *                              items have already been covered in an option and which still
     *                              need coverage.
     * @return                      the score our option contributes to its RankedSet and name.
     */
    strNum overlappingCoverType(int indexInOption, int depthTag);

    /**
     * @brief overlappingUncoverType  undoes the work of the loos cover operation. It uncovers items
     *                                that were covered by an option at the same level of recursion
     *                                in which they were covered, using the depth tags to note
     *                                levels.
     * @param indexInOption           the same index as cover operation will uncover same items.
     */
    void overlappingUncoverType(int indexInOption);

    /**
     * @brief findItemIndex  performs binary search on the sorted item array to find its index in
     *                       the links array as the column header.
     * @param item           the string item we search for depending on ATTACK or DEFENSE.
     * @return               the index in the item lookup table. This is same as header in links.
     */
    int findItemIndex(const std::string& item) const;

    /**
     * @brief findItemIndex  performs binary search on the sorted option array to find its index in
     *                       the links array as the row spacer.
     * @param item           the string item we search for depending on ATTACK or DEFENSE.
     * @return               the index in the item option table. This is same as spacer in links.
     */
    int findOptionIndex(const std::string& option) const;

    /**
     * @brief hideItem     hiding an item in the links means we simply tag its column header with a
     *                     special value that tells our algorithms to ignore items. O(1).
     * @param headerIndex  the index in the column header of the links that dance.
     */
    void hideItem(int headerIndex);

    /**
     * @brief unhideItem   unhiding items means we reset tag to indicate is back in the world. O(1).
     * @param headerIndex  the index of the column header for the dancing links array.
     */
    void unhideItem(int headerIndex);

    /**
     * @brief hideOption  hiding an option involves splicing it out of the up-down linked list. We
     *                    remove all items in this option from the world so the option is hidden.
     * @param rowIndex    the spacer row index in the row within the dancing links array.
     */
    void hideOption(int rowIndex);

    /**
     * @brief unhideOption  unhiding an option undoes the splicing operation. Undoing an option
     *                      must be done in last in first out order. User is expected to manage
     *                      hidden options in a stack.
     * @param rowIndex      the spacer row index in the row within the dancing links array.
     */
    void unhideOption(int rowIndex);


    /* * * * * * * * * * *   Dancing Links Instantiation and Building     * * * * * * * * * * * * */


    /**
     * @brief buildDefenseLinks  defensive links have all typings for a generation as options and
     *                           all single attack types as items. We will build these links along
     *                           with auxillary vectors that help control recursion and record the
     *                           names of the items and options.
     * @param typeInteractions   the map of interactions and resistances between types in a gen.
     */
    void buildDefenseLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions);

    /**
     * @brief buildAttackLinks  attack links have all single attack types for a generation as
     *                          options and all possible Pokemon typings as items in the links.
     * @param typeInteractions  the map of interactions and resistances between types in a gen.
     */
    void buildAttackLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions);

    /**
     * @brief initializeColumns  helper to build the options in our links and the appearances of the
     *                           items across these options.
     * @param typeInteractions   the map of interactions and resistances between types in a gen.
     * @param columnBuilder      the helper data structure to build the columns.
     * @param requestedCoverage  requested coverage to know which multipliers to pay attention to.
     */
    void initializeColumns(const std::map<std::string,std::set<Resistance>>& typeInteractions,
                           std::unordered_map<std::string,int>& columnBuilder,
                           CoverageType requestedCoverage);


    /* * * * * * * * * * *    Operators for Test Harness Functionality    * * * * * * * * * * * * */


    /* I test dancing links implementations internally rather than with unit tests because the
     * vectors are easy to examine. To use the Stanford Test Harness I need to provide overloaded
     * operators for equality testing and printing output on failure. These friend functions
     * are strictly for the ALLOW_TEST_ACCESS() call at the end of this class and shouldn't be
     * useful or used by anything else that uses this class. This algorithm doesn't need to allow
     * users to have access to these internals.
     */
    friend bool operator==(const pokeLink& lhs, const pokeLink& rhs);
    friend bool operator!=(const pokeLink& lhs, const pokeLink& rhs);
    friend std::ostream& operator<<(std::ostream& os, const pokeLink& link);
    friend bool operator==(const typeName& lhs, const typeName& rhs);
    friend bool operator!=(const typeName& lhs, const typeName& rhs);
    friend std::ostream& operator<<(std::ostream& os, const typeName& type);
    friend bool operator==(const strNum& lhs, const strNum& rhs);
    friend bool operator!=(const strNum& lhs, const strNum& rhs);
    friend std::ostream& operator<<(std::ostream& os, const strNum& nN);
    friend std::ostream& operator<<(std::ostream& os, const std::vector<strNum>& nN);
    friend bool operator==(const std::vector<strNum>& lhs, const std::vector<strNum>& rhs);
    friend bool operator!=(const std::vector<strNum>& lhs, const std::vector<strNum>& rhs);
    friend bool operator==(const std::vector<pokeLink>& lhs, const std::vector<pokeLink>& rhs);
    friend bool operator!=(const std::vector<pokeLink>& lhs, const std::vector<pokeLink>& rhs);
    friend bool operator==(const std::vector<typeName>& lhs, const std::vector<typeName>& rhs);
    friend bool operator!=(const std::vector<typeName>& lhs, const std::vector<typeName>& rhs);
    friend std::ostream& operator<<(std::ostream& os, const std::vector<pokeLink>& links);
    friend std::ostream& operator<<(std::ostream&os, const std::vector<typeName>& items);
    ALLOW_TEST_ACCESS();
}; // class PokemonLinks



/* * * * * * * * * * *     Free Functions for Client to Use with Class    * * * * * * * * * * * * */


/**
 * @brief solveExactCover  an exact type coverage is one in which every "option" we choose
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
std::set<RankedSet<std::string>> solveExactCover(PokemonLinks& dlx, int choiceLimit);

/**
 * @brief solveOverlappingCover an overlapping coverage is when we cover every "item"
 *                              with our choices of "options." It is allowable for two
 *                              options cover the same item twice, the goal is to cover
 *                              the items with any allowable choices. The scoring scheme
 *                              for generated sets is the same as described in the
 *                              exact cover version of the problem. Currently, this
 *                              solution is slow because it generates duplicate
 *                              RankedSet solutions. I filter out duplicates by using a
 *                              set. I have not yet found a way to prevent generating
 *                              duplicate solutions with the Dancing Links method.
 * @return                      the set of Ranked Sets that form all overlapping covers.
 */
std::set<RankedSet<std::string>> solveOverlappingCover(PokemonLinks& dlx, int choiceLimit);

/**
 * @brief hasMaxSolutions  exact and overlapping cover solutions are limited at a large number
 *                         for usability if entered into a GUI application. If we have
 *                         generated our maximum output the class will report this fact. This
 *                         means there may have been more solutions but they weren't generated.
 * @param dlx              the PokemonLinks object used to solve exact cover problems.
 * @return                 true if we reached the max limit false if not O(1).
 */
bool hasMaxSolutions(const PokemonLinks& dlx);

/**
 * @brief items  reports the items in the cover problem as a vector of strings. May be attack types
 *               or defense types depending on the requested cover solution on constructing class.
 * @param dlx    the PokemonLinks object that is instantiated with a requested cover solution.
 * @return       the vector of strings that are our items we must cover in the problem.
 */
std::vector<std::string> items(const PokemonLinks& dlx);

/**
 * @brief numItems  the number of items that need cover in the current PokemonLinks object.
 * @param dlx       the PokemonLinks object that uses options to cover items.
 * @return          the int number of items to cover.
 */
int numItems(const PokemonLinks& dlx);

/**
 * @brief hasItem  returns true if an item is contained the object. An item that is hidden CANNOT
 *                 be found by a membership test, it is hidden. O(lgN) where N is items.
 * @param dlx      the PokemonLinks object we examine.
 * @param item     the item we are trying to find.
 * @return         true if the item is present and not hidden false if not.
 */
bool hasItem(const PokemonLinks& dlx, const std::string& item);

/**
 * @brief options  reports the options available to us to cover our items in the cover problem.
 * @param dlx      the PokemonLinks object that is instantiated with a requested cover solution.
 * @return         the vector of strings that are our options we can use to cover the items.
 */
std::vector<std::string> options(const PokemonLinks& dlx);

/**
 * @brief numOptions  the number of options we can choose from to cover the total items.
 * @param dlx         the PokemonLinks object that uses options to cover items.
 * @return            the int number of options we can choose from.
 */
int numOptions(const PokemonLinks& dlx);

/**
 * @brief hasOption  returns true if an option is contained in the object. An option that is hidden
 *                   CANNOT be found by a membership test. O(lgN) where N is options.
 * @param dlx        the PokemonLinks object we examine.
 * @param option     the option we are searching for.
 * @return           true if found and not hidden false if not.
 */
bool hasOption(const PokemonLinks& dlx, const std::string& option);

/**
 * @brief coverageType  the coverage type the PokemonLinks is set to.
 * @param dlx           the PokemonLinks we have instantiated.
 * @return              ATTACK or DEFENSE, depeding on the request when building the class.
 */
PokemonLinks::CoverageType coverageType(const PokemonLinks& dlx);

/**
 * @brief hideItem  removes an item from the dancing links so that it no longer exists in any cover
 *                  problem. This is different than covering an item in Algorithm X. Here, the user
 *                  is requesting that only this item disappear. All other options that contain this
 *                  item retain their other items and remain in the world. We can restore this item
 *                  into the world later if desired. Interact with hidden items as you would a
 *                  stack. We must unhide the top of the stack of hidden items first. In place,
 *                  O(lgN) guarantee.
 * @param dlx       the PokemonLinks object we alter in place.
 * @param toHide    the string item representing the item to hide depending on ATTACK or DEFENSE.
 * @return          true if the item was hidden false if it was already hidden or not found.
 */
bool hideItem(PokemonLinks& dlx, const std::string& toHide);

/**
 * @brief hideItem  hides all items specified from the vector as above. In place, O(NlgN) guarantee.
 * @param dlx       the PokemonLinks object we alter in place.
 * @param toHide    the string item representing the item to hide depending on ATTACK or DEFENSE.
 * @return          true if all items were hidden false if at least one was not.
 */
bool hideItem(PokemonLinks& dlx, const std::vector<std::string>& toHide);

/**
 * @brief hideItem      hides items specified from the vector as above. In place, O(NlgN) guarantee.
 * @param dlx           the PokemonLinks object we alter in place.
 * @param toHide        the string representing the item to hide depending on ATTACK or DEFENSE.
 * @param failedToHide  an additional output parameter if user wants to see failures.
 * @return              true if all items were hidden false if at least one was not found.
 */
bool hideItem(PokemonLinks& dlx, const std::vector<std::string>& toHide,
                                 std::vector<std::string>& failedToHide);

/**
 * @brief hideItemsExcept  hides all items NOT included in the specified set. In place, O(NlgK)
 *                         N is the number of Items and K is the number of items to keep.
 * @param dlx              the PokemonLinks object we alter in place.
 * @param toKeep           the items that remain the world as items we must cover.
 */
void hideItemsExcept(PokemonLinks& dlx, const std::set<std::string>& toKeep);

/**
 * @brief numHidItems  returns the number of items we are currently hiding from the world. O(1).
 * @param dlx          the PokemonLinks object we examine.
 * @return             the int number of hidden items.
 */
int numHidItems(const PokemonLinks& dlx);

/**
 * @brief peekHidItem  hidden items act like a stack that must be unhidden Last-in-First-out.
 *                     check the most recently hid item without altering the stack. Throws an
 *                     exception is the stack is empty. O(1).
 * @param dlx          the PokemonLinks object we examine.
 * @return             the most recently hidden item.
 */
std::string peekHidItem(const PokemonLinks& dlx);

/**
 * @brief popHidItem  pop the most recently hidden item from the stack altering the stack. Throws
 *                    an exception if attempt is made to pop from empty stack. O(1).
 * @param dlx         the PokemonLinks object we alter.
 */
void popHidItem(PokemonLinks& dlx);

/**
 * @brief hidItemsEmpty  reports if the stack of hidden items is empty.
 * @param dlx            the PokemonLinks object we examine.
 * @return               true if empty false if not.
 */
bool hidItemsEmpty(const PokemonLinks& dlx);

/**
 * @brief hidItems  view the currently hidden stack as a vector. The last item is the first out.
 * @param dlx       the PokemonLinks object we alter.
 * @return          the vector of items in the order we hid them. Last is first out if popped.
 */
std::vector<std::string> hidItems(const PokemonLinks& dlx);

/**
 * @brief resetItems  restores the items in the world to their original state. O(H), H hidden items.
 * @param dlx         the PokemonLinks object we alter.
 */
void resetItems(PokemonLinks& dlx);

/**
 * @brief hideOption  hides an option from the dancing links so that it no longer available to
 *                    cover items. This is a different type of hiding than in Algorithm X. Here,
 *                    the number of options available to every item covered by this option is
 *                    reduced by one and all items in this option remain in the world. They must
 *                    still be covered through other options to solve the problem. Hide options
 *                    behaves as a stack and they must be unhidden in Last-in-First-out order to
 *                    restore the world to its original state. In place, O(lgN + I) where N is the
 *                    number of options and I is the number of Items in an option.
 * @param dlx         the PokemonLinks object we alter.
 * @param toHide      the option we must hide from the world.
 * @return            true if option was hidden false if it was hidden or could not be found.
 */
bool hideOption(PokemonLinks& dlx, const std::string& toHide);

/**
 * @brief hideOption  hides all options specified in the vector from the world. Uses the same
 *                    process as hideOption() for each option making an O(HlgNI) where H is the
 *                    number of options to hide, N is the number of options and I is the number of
 *                    items in an option. In practice, I is often small in sparse links.
 * @param dlx         the PokemonLinks object we alter.
 * @param toHide      the options we must hide from the world.
 * @return            true if all options were hidden false if at least one was not.
 */
bool hideOption(PokemonLinks& dlx, const std::vector<std::string>& toHide);

/**
 * @brief hideOption    hides all options specified in the vector from the world. Uses the same
 *                      process as hideOption() for each option making an O(HlgNI) where H is the
 *                      number of options to hide, N is the number of options and I is the number of
 *                      items in an option. In practice, I is often small in sparse links.
 * @param dlx           the PokemonLinks object we alter.
 * @param toHide        the options we must hide from the world.
 * @param failedToHide  an additional output showing the options that could not be found.
 * @return              true if all options were hidden false if at least one failed.
 */
bool hideOption(PokemonLinks& dlx, const std::vector<std::string>& toHide,
                                   std::vector<std::string>& failedToHide);

/**
 * @brief hideOptionsExcept  hides all options NOT specified in the given set. In place O(NlgKI)
 *                           where N is the number of options, K the number of options to keep, and
 *                           I the number of Items covered by each option. In practice I is small in
 *                           sparse PokemonLinks grids.
 * @param dlx                the PokemonLinks object we alter.
 * @param toKeep             the options we will keep available to choose from for the problem.
 */
void hideOptionsExcept(PokemonLinks& dlx, const std::set<std::string>& toKeep);

/**
 * @brief numHidOptions  num options currently hidden in the stack. The last is first out. O(1).
 * @param dlx            the PokemonLinks object we examine.
 * @return               the int representing number of hidden options in the stack.
 */
int numHidOptions(const PokemonLinks& dlx);

/**
 * @brief peekHidOption  hidden items act like a stack that must be unhidden Last-in-First-out.
 *                       check the most recently hid item without altering the stack. O(1).
 * @param dlx            the PokemonLinks object we examine.
 * @return               the most recently hidden option.
 */
std::string peekHidOption(const PokemonLinks& dlx);

/**
 * @brief popHidOption  pop the most recently hidden item from stack altering the stack. O(1).
 * @param dlx           the PokemonLinks object we alter.
 */
void popHidOption(PokemonLinks& dlx);

/**
 * @brief hidOptionsEmpty  reports if the stack of hidden options is empty.
 * @param dlx              the PokemonLinks object we examine.
 * @return                 true if empty false if not.
 */
bool hidOptionsEmpty(const PokemonLinks& dlx);

/**
 * @brief hidOptions  view the currently hidden stack as a vector. The last item is first-out.
 * @param dlx         the PokemonLinks object we alter.
 * @return            the vector of options in order we hid them. Last is first out if popped.
 */
std::vector<std::string> hidOptions(const PokemonLinks& dlx);

/**
 * @brief resetOptions  restore the options in the world to original state. O(H), H hidden items.
 * @param dlx           the PokemonLinks object we alter.
 */
void resetOptions(PokemonLinks& dlx);

/**
 * @brief resetAll  completely restore the items and options to original state. O(I + (PC)) where
 *                  I is the number of hidden items, P is the number of hidden options and C is the
 *                  number of items covered by each option.
 * @param dlx       the PokemonLinks object we alter.
 */
void resetAll(PokemonLinks& dlx);


} // namespace DancingLinks

#endif // POKEMONLINKS_H
