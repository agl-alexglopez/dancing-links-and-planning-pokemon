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
 * Problem with Donald Knuth's Algorithm X via Dancing Links. For detailed documentation read the
 * comment in the DancingLinks.h free function file for this class. Also the README.md for the
 * repository has a detailed writeup with images.
 */
#ifndef POKEMONLINKS_H
#define POKEMONLINKS_H
#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <string_view>
#include <set>
#include <utility>
#include <vector>
#include "GUI/SimpleTest.h"
#include "RankedSet.h"
#include "Resistance.h"
#include "TypeEncoding.h"


namespace DancingLinks {


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
     *                                to build differently based on attack or defense. It is
     *                                important that the data is passed in with a map because we
     *                                need our dancing links items and options to be built and setup
     *                                in lexicographic order for some additional functionality and
     *                                runtime guarantees.
     * @param typeInteractions        map of pokemon types and their resistances to attack types.
     * @param requestedCoverSolution  ATTACK or DEFENSE. Build a team or choose attack types.
     */
    explicit PokemonLinks(const std::map<TypeEncoding,std::set<Resistance>>& typeInteractions,
                           const CoverageType requestedCoverSolution);

    /**
     * @brief PokemonLinks      this alternative constructor is helpful when choosing a defensive
     *                          team based on a subset of attack types. For example, we could build
     *                          defenses against the attack types present at specific gyms. It is
     *                          important that the data is passed in with a map and set because we
     *                          need our dancing links items and options to be built and setup
     *                          in lexicographic order for some additional functionality and
     *                          runtime guarantees.
     * @param typeInteractions  the map of types and their defenses for a given generation.
     * @param attackTypes       the subset of attacks we must cover with choices of Pokemon teams.
     */
    explicit PokemonLinks(const std::map<TypeEncoding,std::set<Resistance>>& typeInteractions,
                           const std::set<TypeEncoding>& attackTypes);


    /* * * * * * * * * * * *  See DancingLinks.h for Documented Free Functions  * * * * * * * * * */


    std::set<RankedSet<TypeEncoding>> getExactCoverages(int choiceLimit);

    std::set<RankedSet<TypeEncoding>> getOverlappingCoverages(int choiceLimit);

    bool hideRequestedItem(TypeEncoding toHide);

    bool hideRequestedItem(const std::vector<TypeEncoding>& toHide);

    bool hideRequestedItem(const std::vector<TypeEncoding>& toHide,
                             std::vector<TypeEncoding>& failedToHide);

    void hideAllItemsExcept(const std::set<TypeEncoding>& toKeep);

    bool hasItem(TypeEncoding item) const;

    TypeEncoding peekHidItem() const;

    void popHidItem();

    bool hidItemsEmpty() const;

    std::vector<TypeEncoding> getHidItems() const;

    int getNumHidItems() const;

    void resetItems();

    bool hideRequestedOption(TypeEncoding toHide);

    bool hideRequestedOption(const std::vector<TypeEncoding>& toHide);

    bool hideRequestedOption(const std::vector<TypeEncoding>& toHide,
                               std::vector<TypeEncoding>& failedToCover);

    void hideAllOptionsExcept(const std::set<TypeEncoding>& toKeep);

    bool hasOption(TypeEncoding option) const;

    TypeEncoding peekHidOption() const;

    void popHidOption();

    bool hidOptionsEmpty() const;

    std::vector<TypeEncoding> getHidOptions() const;

    int getNumHidOptions() const;

    void resetOptions();

    void resetItemsOptions();

    bool reachedOutputLimit() const;

    std::vector<TypeEncoding> getItems() const;

    int getNumItems() const;

    std::vector<TypeEncoding> getOptions() const;

    int getNumOptions() const;

    CoverageType getLinksType() const;


private:


    /* * * * * * * * * * *   Dancing Links Internals and Implementation   * * * * * * * * * * * * */


    // This type is entered into our dancing links array for the in place recursive algorithm.
    struct pokeLink {
        int topOrLen;
        int up;
        int down;
        Multiplier multiplier; // x0.0, x0.25, x0.5, x1.0, x2, or x4 damage multipliers.
        int tag;                // We use this to efficiently generate overlapping sets.
    };

    // This type, in a seperate vector, controls the base case of our recursion.
    struct typeName {
        TypeEncoding name;
        int left;
        int right;
    };

    struct encodingAndNum {
        TypeEncoding name;
        int num;
    };


    /* These data structures contain the core logic of Algorithm X via dancing links. For more
     * detailed information, see the tests in the implementation. These help acheive in place
     * recursion. We can also play around with more advanced in place techniques like hiding options
     * and items at the users request and restoring them later in place. Finally, because the option
     * table and item table are sorted lexographically we can find any option or item in O(lgN). No
     * auxillary maps are needed.
     */
    std::vector<encodingAndNum> optionTable_;  // How we know the name of the option we chose.
    std::vector<typeName> itemTable_;          // How we know the names of our items.
    std::vector<pokeLink> links_;              // The links that dance!
    std::vector<int> hiddenItems_;             // Treat as stack with user hidden Items.
    std::vector<int> hiddenOptions_;           // Treat as stack with user hidden Options.
    std::size_t maxOutput_;                    // Cutoff our solution generation for GUI usability.
    bool hitLimit_;                            // How we report to a user that we cutoff more solutions
    int numItems_;                             // What needs to be covered.
    int numOptions_;                           // Options we can choose from to cover items.
    CoverageType requestedCoverSolution_;      // The user is asking for ATTACK or DEFENSE

    static constexpr int HIDDEN = -1;

    /**
     * @brief fillExactCoverages  fills the output parameters with every exact cover that can be
     *                            determined for defending against attack types or attacking
     *                            defensive types. Exact covers use options to cover every item
     *                            exactly once.
     * @param coverages           the output parameter that serves as the final solution.
     * @param coverage            the successfully coverages we find while the links dance.
     * @param depthLimit          size of a pokemon team or the number of attacks a team can have.
     */
    void fillExactCoverages(std::set<RankedSet<TypeEncoding>>& coverages,
                              RankedSet<TypeEncoding>& coverage,
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
    void fillOverlappingCoverages(std::set<RankedSet<TypeEncoding>>& coverages,
                                    RankedSet<TypeEncoding>& coverage,
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
    encodingAndNum coverType(int indexInOption);

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
    encodingAndNum overlappingCoverType(int indexInOption, int depthTag);

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
     * @param item           the type item we search for depending on ATTACK or DEFENSE.
     * @return               the index in the item lookup table. This is same as header in links.
     */
    int findItemIndex(TypeEncoding item) const;

    /**
     * @brief findItemIndex  performs binary search on the sorted option array to find its index in
     *                       the links array as the row spacer.
     * @param item           the type item we search for depending on ATTACK or DEFENSE.
     * @return               the index in the item option table. This is same as spacer in links.
     */
    int findOptionIndex(TypeEncoding option) const;

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
    void buildDefenseLinks(const std::map<TypeEncoding,std::set<Resistance>>& typeInteractions);

    /**
     * @brief buildAttackLinks  attack links have all single attack types for a generation as
     *                          options and all possible Pokemon typings as items in the links.
     * @param typeInteractions  the map of interactions and resistances between types in a gen.
     */
    void buildAttackLinks(const std::map<TypeEncoding,std::set<Resistance>>& typeInteractions);

    /**
     * @brief initializeColumns  helper to build the options in our links and the appearances of the
     *                           items across these options.
     * @param typeInteractions   the map of interactions and resistances between types in a gen.
     * @param columnBuilder      the helper data structure to build the columns.
     * @param requestedCoverage  requested coverage to know which multipliers to pay attention to.
     */
    void initializeColumns(const std::map<TypeEncoding,std::set<Resistance>>& typeInteractions,
                             std::unordered_map<TypeEncoding,int>& columnBuilder,
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
    friend bool operator==(const encodingAndNum& lhs, const encodingAndNum& rhs);
    friend bool operator!=(const encodingAndNum& lhs, const encodingAndNum& rhs);
    friend std::ostream& operator<<(std::ostream& os, const encodingAndNum& nN);
    friend std::ostream& operator<<(std::ostream& os, const std::vector<encodingAndNum>& nN);
    friend bool operator==(const std::vector<encodingAndNum>& lhs, const std::vector<encodingAndNum>& rhs);
    friend bool operator!=(const std::vector<encodingAndNum>& lhs, const std::vector<encodingAndNum>& rhs);
    friend bool operator==(const std::vector<pokeLink>& lhs, const std::vector<pokeLink>& rhs);
    friend bool operator!=(const std::vector<pokeLink>& lhs, const std::vector<pokeLink>& rhs);
    friend bool operator==(const std::vector<typeName>& lhs, const std::vector<typeName>& rhs);
    friend bool operator!=(const std::vector<typeName>& lhs, const std::vector<typeName>& rhs);
    friend std::ostream& operator<<(std::ostream& os, const std::vector<pokeLink>& links);
    friend std::ostream& operator<<(std::ostream&os, const std::vector<typeName>& items);
    friend std::ostream& operator<<(std::ostream& os, const std::vector<encodingAndNum>& vec);
    ALLOW_TEST_ACCESS();

}; // class PokemonLinks

} // namespace DancingLinks


#endif // POKEMONLINKS_H
