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
 * File: PokemonLinks.cpp
 * ----------------------
 * Contained in this file is my implementation of Algorithm X via dancing links as outlined by
 * Donald Knuth. The exact cover implementation is a faithful representation of the algorithm that
 * Knuth describes in the context of C++ and the Pokemon Type Coverage Problem. The Overlapping
 * Coverage implementation is a variation on exact cover that I use to generate coverage that allows
 * multiple options to cover some of the same items more than once. For a more detailed writeup see
 * the DancingLinks.h file and README.md in this repository.
 */
#include "PokemonLinks.h"
#include <stdexcept>
#include <limits.h>
#include <cmath>


namespace DancingLinks {


/* * * * * * * * * * * * *       Convenience Callers for Encapsulation      * * * * * * * * * * * */


std::set<RankedSet<TypeEncoding>> solveExactCover(PokemonLinks& dlx, int choiceLimit) {
    return dlx.getExactCoverages(choiceLimit);
}

std::set<RankedSet<TypeEncoding>> solveOverlappingCover(PokemonLinks& dlx, int choiceLimit) {
    return dlx.getOverlappingCoverages(choiceLimit);
}

bool hasMaxSolutions(const PokemonLinks& dlx) {
    return dlx.reachedOutputLimit();
}

int numItems(const PokemonLinks& dlx) {
    return dlx.getNumItems();
}

bool hasItem(const PokemonLinks& dlx, TypeEncoding item) {
    return dlx.hasItem(item);
}

int numOptions(const PokemonLinks& dlx) {
    return dlx.getNumOptions();
}

bool hasOption(const PokemonLinks& dlx, TypeEncoding option) {
    return dlx.hasOption(option);
}

PokemonLinks::CoverageType coverageType(const PokemonLinks& dlx) {
    return dlx.getLinksType();
}

std::vector<TypeEncoding> items(const PokemonLinks& dlx) {
    return dlx.getItems();
}

std::vector<TypeEncoding> options(const PokemonLinks& dlx) {
    return dlx.getOptions();
}

bool hideItem(PokemonLinks& dlx, TypeEncoding toHide) {
    return dlx.hideRequestedItem(toHide);
}

bool hideItem(PokemonLinks& dlx, const std::vector<TypeEncoding>& toHide) {
    return dlx.hideRequestedItem(toHide);
}

bool hideItem(PokemonLinks& dlx, const std::vector<TypeEncoding>& toHide,
               std::vector<TypeEncoding>& failedToHide) {
    return dlx.hideRequestedItem(toHide, failedToHide);
}

void hideItemsExcept(PokemonLinks& dlx, const std::set<TypeEncoding>& toKeep) {
    dlx.hideAllItemsExcept(toKeep);
}

int numHidItems(const PokemonLinks& dlx) {
    return dlx.getNumHidItems();
}

TypeEncoding peekHidItem(const PokemonLinks& dlx) {
    return dlx.peekHidItem();
}

void popHidItem(PokemonLinks& dlx) {
    dlx.popHidItem();
}

bool hidItemsEmpty(const PokemonLinks& dlx) {
    return dlx.hidItemsEmpty();
}

std::vector<TypeEncoding> hidItems(const PokemonLinks& dlx) {
    return dlx.getHidItems();
}

void resetItems(PokemonLinks& dlx) {
    dlx.resetItems();
}

bool hideOption(PokemonLinks& dlx, TypeEncoding toHide) {
    return dlx.hideRequestedOption(toHide);
}

bool hideOption(PokemonLinks& dlx, const std::vector<TypeEncoding>& toHide) {
    return dlx.hideRequestedOption(toHide);
}

bool hideOption(PokemonLinks& dlx, const std::vector<TypeEncoding>& toHide,
                 std::vector<TypeEncoding>& failedToHide) {
    return dlx.hideRequestedOption(toHide, failedToHide);
}

void hideOptionsExcept(PokemonLinks& dlx, const std::set<TypeEncoding>& toKeep) {
    dlx.hideAllOptionsExcept(toKeep);
}

int numHidOptions(const PokemonLinks& dlx) {
    return dlx.getNumHidOptions();
}

TypeEncoding peekHidOption(const PokemonLinks& dlx) {
    return dlx.peekHidOption();
}

void popHidOption(PokemonLinks& dlx) {
    dlx.popHidOption();
}

bool hidOptionsEmpty(const PokemonLinks& dlx) {
    return dlx.hidOptionsEmpty();
}

std::vector<TypeEncoding> hidOptions(const PokemonLinks& dlx) {
    return dlx.getHidOptions();
}

void resetOptions(PokemonLinks& dlx) {
    dlx.resetOptions();
}

void resetAll(PokemonLinks& dlx) {
    dlx.resetItemsOptions();
}


/* * * * * * * * * * * * * * * *    Algorithm X via Dancing Links   * * * * * * * * * * * * * * * */


std::set<RankedSet<TypeEncoding>> PokemonLinks::getExactCoverages(int choiceLimit) {
    std::set<RankedSet<TypeEncoding>> coverages = {};
    RankedSet<TypeEncoding> coverage = {};
    hitLimit_ = false;
    fillExactCoverages(coverages, coverage, choiceLimit);
    return coverages;
}

void PokemonLinks::fillExactCoverages(std::set<RankedSet<TypeEncoding>>& coverages,
                                        RankedSet<TypeEncoding>& coverage,
                                        int depthLimit) {
    if (itemTable_[0].right == 0 && depthLimit >= 0) {
        coverages.insert(coverage);
        return;
    }
    // Depth limit is either the size of a Pokemon Team or the number of attack slots on a team.
    if (depthLimit <= 0) {
        return;
    }
    int itemToCover = chooseItem();
    // An item has become inaccessible due to our chosen options so far, undo.
    if (!itemToCover) {
        return;
    }
    for (int cur = links_[itemToCover].down; cur != itemToCover; cur = links_[cur].down) {
        encodingAndNum score = coverType(cur);
        coverage.insert(score.num, score.name);

        fillExactCoverages(coverages, coverage, depthLimit - 1);

        /* It is possible for these algorithms to produce many many sets. To make the Pokemon
         * Planner GUI more usable I cut off recursion if we are generating too many sets.
         */
        if (coverages.size() == maxOutput_) {
            hitLimit_ = true;
            uncoverType(cur);
            return;
        }
        coverage.erase(score.num, score.name);
        uncoverType(cur);
    }
}

PokemonLinks::encodingAndNum PokemonLinks::coverType(int indexInOption) {
    encodingAndNum result = {};
    int i = indexInOption;
    do {
        int top = links_[i].topOrLen;
        /* This is the next spacer node for the next option. We now know how to find the title of
         * our current option if we go back to the start of the chosen option and go left.
         */
        if (top <= 0) {
            i = links_[i].up;
            result.name = optionTable_[std::abs(links_[i - 1].topOrLen)].name;
        } else {
            if (!links_[top].tag) {
                typeName cur = itemTable_[top];
                itemTable_[cur.left].right = cur.right;
                itemTable_[cur.right].left = cur.left;
                hideOptions(i);
                /* If there is a better way to score the teams or attack schemes we build here would
                 * be the place to change it. I just give points based on how good the resistance or
                 * attack strength is. Immunity is better than quarter is better than half damage if
                 * we are building defense. Quad is better than double damage if we are building
                 * attack types. Points only change by increments of one.
                 */
                result.num += links_[i].multiplier;
            }
            i++;
        }
    } while (i != indexInOption);
    return result;
}

void PokemonLinks::uncoverType(int indexInOption) {
    // Go left first so the in place link restoration of the doubly linked lookup table works.
    int i = --indexInOption;
    do {
        int top = links_[i].topOrLen;
        if (top <= 0) {
            i = links_[i].down;
        } else {
            if (!links_[top].tag) {
                typeName cur = itemTable_[top];
                itemTable_[cur.left].right = top;
                itemTable_[cur.right].left = top;
                unhideOptions(i);
            }
            i--;
        }
    } while (i != indexInOption);
}

/* The hide/unhide technique is what makes exact cover so much more restrictive and fast at
 * shrinking the problem. Notice how aggressively it eliminates the appearances of items across
 * other options. When compared to Overlapping Coverage, Exact Coverage answers a different
 * question but also shrinks the problem much more quickly.
 */

void PokemonLinks::hideOptions(int indexInOption) {
    for (int row = links_[indexInOption].down; row != indexInOption; row = links_[row].down) {
        if (row == links_[indexInOption].topOrLen) {
            continue;
        }
        for (int col = row + 1; col != row;) {
            int top = links_[col].topOrLen;
            if (top <= 0) {
                col = links_[col].up;
            } else {
                pokeLink cur = links_[col];
                links_[cur.up].down = cur.down;
                links_[cur.down].up = cur.up;
                links_[top].topOrLen--;
                col++;
            }
        }
    }
}

void PokemonLinks::unhideOptions(int indexInOption) {
    for (int row = links_[indexInOption].up; row != indexInOption; row = links_[row].up) {
        if (row == links_[indexInOption].topOrLen) {
            continue;
        }
        for (int col = row - 1; col != row;) {
            int top = links_[col].topOrLen;
            if (top <= 0) {
                col = links_[col].down;
            } else {
                pokeLink cur = links_[col];
                links_[cur.up].down = col;
                links_[cur.down].up = col;
                links_[top].topOrLen++;
                col--;
            }
        }
    }
}


/* * * * * * * * * * * *  Shared Choosing Heuristic for Both Techniques * * * * * * * * * * * * * */


int PokemonLinks::chooseItem() const {
    int min = INT_MAX;
    int chosenIndex = 0;
    for (int cur = itemTable_[0].right; cur != 0; cur = itemTable_[cur].right) {
        // No way to reach this item. Bad past choices or impossible to solve.
        if (links_[cur].topOrLen <= 0) {
            return 0;
        }
        if (links_[cur].topOrLen < min) {
            chosenIndex = cur;
            min = links_[cur].topOrLen;
        }
    }
    return chosenIndex;
}


/* * * * * * * * * * * *   Overlapping Coverage via Dancing Links   * * * * * * * * * * * * * * * */


std::set<RankedSet<TypeEncoding>>
PokemonLinks::getOverlappingCoverages(int choiceLimit) {
    std::set<RankedSet<TypeEncoding>> coverages = {};
    RankedSet<TypeEncoding> coverage = {};
    hitLimit_ = false;
    fillOverlappingCoverages(coverages, coverage, choiceLimit);
    return coverages;
}

void PokemonLinks::fillOverlappingCoverages(std::set<RankedSet<TypeEncoding>>& coverages,
                                              RankedSet<TypeEncoding>& coverage,
                                                 int depthTag) {
    if (itemTable_[0].right == 0 && depthTag >= 0) {
        coverages.insert(coverage);
        return;
    }
    if (depthTag <= 0) {
        return;
    }
    // In certain generations certain types have no weaknesses so we might return 0 here.
    int itemToCover = chooseItem();
    if (!itemToCover) {
        return;
    }

    for (int cur = links_[itemToCover].down; cur != itemToCover; cur = links_[cur].down) {
        encodingAndNum score = overlappingCoverType(cur, depthTag);
        coverage.insert(score.num, score.name);

        fillOverlappingCoverages(coverages, coverage, depthTag - 1);

        /* It is possible for these algorithms to produce many many sets. To make the Pokemon
         * Planner GUI more usable I cut off recursion if we are generating too many sets.
         */
        if (coverages.size() == maxOutput_) {
            hitLimit_ = true;
            overlappingUncoverType(cur);
            return;
        }
        coverage.erase(score.num, score.name);
        overlappingUncoverType(cur);
    }
}

/* Overlapping cover is much simpler at the cost of generating a tremendous number of solutions. We
 * only need to know which items and options are covered at which recursive levels because we are
 * more relaxed about leaving options available after items in those options have been covered by
 * other options.
 */

PokemonLinks::encodingAndNum PokemonLinks::overlappingCoverType(int indexInOption, int depthTag) {
    int i = indexInOption;
    encodingAndNum result = {};
    do {
        int top = links_[i].topOrLen;
        if (top <= 0) {
            i = links_[i].up;
            result.name = optionTable_[std::abs(links_[i - 1].topOrLen)].name;
        } else {
            if (!links_[top].tag) {
                links_[top].tag = depthTag;
                itemTable_[itemTable_[top].left].right = itemTable_[top].right;
                itemTable_[itemTable_[top].right].left = itemTable_[top].left;
                result.num += links_[i].multiplier;
            }
            links_[top].tag == HIDDEN ? i++ : links_[i++].tag = depthTag;
        }
    } while (i != indexInOption);

    return result;
}

void PokemonLinks::overlappingUncoverType(int indexInOption) {
    int i = --indexInOption;
    do {
        int top = links_[i].topOrLen;
        if (top < 0) {
            i = links_[i].down;
        } else {
            if (links_[top].tag == links_[i].tag) {
                links_[top].tag = 0;
                itemTable_[itemTable_[top].left].right = top;
                itemTable_[itemTable_[top].right].left = top;
            }
            links_[top].tag == HIDDEN ? i-- : links_[i--].tag = 0;
        }
    } while (i != indexInOption);
}


/* * * * * * * * * * * * * * * * *        Utility Functions             * * * * * * * * * * * * * */


bool PokemonLinks::reachedOutputLimit() const {
    return hitLimit_;
}

int PokemonLinks::getNumItems() const {
    return numItems_;
}

int PokemonLinks::getNumOptions() const {
    return numOptions_;
}

PokemonLinks::CoverageType PokemonLinks::getLinksType() const {
    return requestedCoverSolution_;
}

std::vector<TypeEncoding> PokemonLinks::getItems() const {
    std::vector<TypeEncoding> result = {};
    for (int i = itemTable_[0].right; i != 0; i = itemTable_[i].right) {
        result.push_back(itemTable_[i].name);
    }
    return result;
}

std::vector<TypeEncoding> PokemonLinks::getHidItems() const {
    std::vector<TypeEncoding>result = {};
    for (const auto& i : hiddenItems_) {
        result.push_back(itemTable_[i].name);
    }
    return result;
}

std::vector<TypeEncoding> PokemonLinks::getOptions() const {
    std::vector<TypeEncoding> result = {};
    // Hop from row title to row title, skip hidden options. Skip bookend node that is placeholder.
    for (int i = itemTable_.size(); i < links_.size() - 1; i = links_[i].down + 1) {
        if (links_[i].tag != HIDDEN) {
            result.push_back(optionTable_[i].name);
        }
    }
    return result;
}

std::vector<TypeEncoding> PokemonLinks::getHidOptions() const {
    std::vector<TypeEncoding> result = {};
    for (const auto& i : hiddenOptions_) {
        result.push_back(optionTable_[std::abs(links_[i].topOrLen)].name);
    }
    return result;
}

bool PokemonLinks::hideRequestedItem(TypeEncoding toHide) {
    int lookupIndex = findItemIndex(toHide);
    // Can't find or this item has already been hidden.
    if (lookupIndex && links_[lookupIndex].tag != HIDDEN) {
        hiddenItems_.push_back(lookupIndex);
        hideItem(lookupIndex);
        return true;
    }
    return false;
}

bool PokemonLinks::hideRequestedItem(const std::vector<TypeEncoding>& toHide) {
    bool result = true;
    for (const auto& t : toHide) {
        if (!hideRequestedItem(t)) {
            result = false;
        }
    }
    return result;
}

bool PokemonLinks::hideRequestedItem(const std::vector<TypeEncoding>& toHide,
                                       std::vector<TypeEncoding>& failedToHide) {
    bool result = true;
    for (const auto& t : toHide) {
        if (!hideRequestedItem(t)) {
            result = false;
            failedToHide.push_back(t);
        }
    }
    return result;
}

void PokemonLinks::hideAllItemsExcept(const std::set<TypeEncoding>& toKeep) {
    for (int i = itemTable_[0].right; i != 0; i = itemTable_[i].right) {
        if (!toKeep.count(itemTable_[i].name)) {
            hiddenItems_.push_back(i);
            hideItem(i);
        }
    }

}

bool PokemonLinks::hasItem(TypeEncoding item) const {
    int found = findItemIndex(item);
    return found && links_[found].tag != HIDDEN;
}

void PokemonLinks::popHidItem() {
    if (!hiddenItems_.empty()) {
        unhideItem(hiddenItems_.back());
        hiddenItems_.pop_back();
    } else {
        std::cout << "No hidden items. Stack is empty." << std::endl;
        throw;
    }
}

TypeEncoding PokemonLinks::peekHidItem() const {
    if (hiddenItems_.size()) {
        return itemTable_[hiddenItems_.back()].name;
    }
    std::cout << "No hidden items. Stack is empty." << std::endl;
    throw;
}

bool PokemonLinks::hidItemsEmpty() const {
    return hiddenItems_.empty();
}

int PokemonLinks::getNumHidItems() const {
    return hiddenItems_.size();
}

void PokemonLinks::resetItems() {
    while (!hiddenItems_.empty()) {
        unhideItem(hiddenItems_.back());
        hiddenItems_.pop_back();
    }
}

bool PokemonLinks::hideRequestedOption(TypeEncoding toHide) {
    int lookupIndex = findOptionIndex(toHide);
    // Couldn't find or this option has already been hidden.
    if (lookupIndex && links_[lookupIndex].tag != HIDDEN) {
        hiddenOptions_.push_back(lookupIndex);
        hideOption(lookupIndex);
        return true;
    }
    return false;
}

bool PokemonLinks::hideRequestedOption(const std::vector<TypeEncoding>& toHide) {
    bool result = true;
    for (const auto& h : toHide) {
        if (!hideRequestedOption(h)) {
            result = false;
        }
    }
    return result;
}

bool PokemonLinks::hideRequestedOption(const std::vector<TypeEncoding>& toHide,
                                         std::vector<TypeEncoding>& failedToHide) {
    bool result = true;
    for (const auto& h : toHide) {
        if (!hideRequestedOption(h)) {
            failedToHide.push_back(h);
            result = false;
        }
    }
    return result;
}

void PokemonLinks::hideAllOptionsExcept(const std::set<TypeEncoding>& toKeep) {
    // We start i at the index of the first option spacer. This is after the column headers.
    for (int i = itemTable_.size(); i < links_.size() - 1; i = links_[i].down + 1) {
        if (links_[i].tag != HIDDEN
                && !toKeep.count(optionTable_[std::abs(links_[i].topOrLen)].name)) {
            hiddenOptions_.push_back(i);
            hideOption(i);
        }
    }
}

bool PokemonLinks::hasOption(TypeEncoding option) const {
    int found = findOptionIndex(option);
    return found && links_[found].tag != HIDDEN;
}

void PokemonLinks::popHidOption() {
    if (!hiddenOptions_.empty()) {
        unhideOption(hiddenOptions_.back());
        hiddenOptions_.pop_back();
    } else {
        std::cout << "No hidden items. Stack is empty." << std::endl;
        throw;
    }
}

TypeEncoding PokemonLinks::peekHidOption() const {
    if (!hiddenOptions_.empty()) {
        // Row spacer tiles in the links hold their name as a negative index in the optionTable_
        return optionTable_[std::abs(links_[hiddenOptions_.back()].topOrLen)].name;
    }
    return TypeEncoding("");
}

bool PokemonLinks::hidOptionsEmpty() const {
    return hiddenOptions_.empty();
}

int PokemonLinks::getNumHidOptions() const {
    return hiddenOptions_.size();
}

void PokemonLinks::resetOptions() {
    while (!hiddenOptions_.empty()) {
        unhideOption(hiddenOptions_.back());
        hiddenOptions_.pop_back();
    }
}

void PokemonLinks::resetItemsOptions() {
    resetItems();
    resetOptions();
}

void PokemonLinks::hideItem(int headerIndex) {
    typeName curItem = itemTable_[headerIndex];
    itemTable_[curItem.left].right = curItem.right;
    itemTable_[curItem.right].left = curItem.left;
    links_[headerIndex].tag = HIDDEN;
    numItems_--;
}

void PokemonLinks::unhideItem(int headerIndex) {
    typeName curItem = itemTable_[headerIndex];
    itemTable_[curItem.left].right = headerIndex;
    itemTable_[curItem.right].left = headerIndex;
    links_[headerIndex].tag = 0;
    numItems_++;
}

void PokemonLinks::hideOption(int rowIndex) {
    links_[rowIndex].tag = HIDDEN;
    for (int i = rowIndex + 1; links_[i].topOrLen > 0; i++) {
        pokeLink cur = links_[i];
        links_[cur.up].down = cur.down;
        links_[cur.down].up = cur.up;
        links_[cur.topOrLen].topOrLen--;
    }
    numOptions_--;
}

void PokemonLinks::unhideOption(int rowIndex) {
    links_[rowIndex].tag = 0;
    for (int i = rowIndex + 1; links_[i].topOrLen > 0; i++) {
        pokeLink cur = links_[i];
        links_[cur.up].down = i;
        links_[cur.down].up = i;
        links_[cur.topOrLen].topOrLen++;
    }
    numOptions_++;
}

int PokemonLinks::findItemIndex(TypeEncoding item) const {
    for (size_t nremain = itemTable_.size(), base = 0; nremain != 0; nremain >>= 1) {
        int curIndex = base + (nremain >> 1);
        if (itemTable_[curIndex].name == item) {
            // This is the index where we can find the header for this items column.
            return curIndex;
        }
        if (item > itemTable_[curIndex].name) {
            base = curIndex + 1;
            nremain--;
        }
    }
    // We know zero holds no value in the itemTable_ and this can double as a falsey value.
    return 0;
}

int PokemonLinks::findOptionIndex(TypeEncoding option) const {
    for (size_t nremain = optionTable_.size(), base = 0; nremain != 0; nremain >>= 1) {
        int curIndex = base + (nremain >> 1);
        if (optionTable_[curIndex].name == option) {
            // This is the index corresponding to the spacer node for an option in the links.
            return optionTable_[curIndex].num;
        }
        if (option > optionTable_[curIndex].name) {
            base = curIndex + 1;
            nremain--;
        }
    }
    // We know zero holds no value in the optionTable_ and this can double as a falsey value.
    return 0;
}


/* * * * * * * * * * * * * * * * *   Constructors and Links Build       * * * * * * * * * * * * * */


PokemonLinks::PokemonLinks(const std::map<TypeEncoding,std::set<Resistance>>& typeInteractions,
                            const CoverageType requestedCoverSolution)
    : optionTable_(),
      itemTable_(),
      links_(),
      hiddenItems_(),
      hiddenOptions_(),
      maxOutput_(200000),
      hitLimit_(false),
      numItems_(0),
      numOptions_(0),
      requestedCoverSolution_(requestedCoverSolution) {

    if (requestedCoverSolution == DEFENSE) {
        buildDefenseLinks(typeInteractions);
    } else if (requestedCoverSolution == ATTACK){
        buildAttackLinks(typeInteractions);
    } else {
        std::cerr << "Invalid requested cover solution. Choose ATTACK or DEFENSE." << std::endl;
        std::abort();
    }
}

PokemonLinks::PokemonLinks(const std::map<TypeEncoding,std::set<Resistance>>& typeInteractions,
                            const std::set<TypeEncoding>& attackTypes)
    : optionTable_(),
      itemTable_(),
      links_(),
      hiddenItems_(),
      hiddenOptions_(),
      maxOutput_(200000),
      hitLimit_(false),
      numItems_(0),
      numOptions_(0),
      requestedCoverSolution_(DEFENSE) {

    if (attackTypes.empty()) {
        buildDefenseLinks(typeInteractions);
    } else {

        /* If we want altered attack types to defend against, it is more efficient and explicit
         * to pass in their own set then eliminate them from the Generation map by making a
         * smaller copy.
         */

        std::map<TypeEncoding,std::set<Resistance>> modifiedInteractions = {};
        for (const auto& type : typeInteractions) {
            modifiedInteractions[type.first] = {};
            for (const Resistance& t : type.second) {
                if (attackTypes.count(t.type())) {
                    modifiedInteractions[type.first].insert(t);
                }
            }
        }
        buildDefenseLinks(modifiedInteractions);
    }
}

void PokemonLinks::buildDefenseLinks(const std::map<TypeEncoding,std::set<Resistance>>&
                                      typeInteractions) {
    // We always must gather all attack types available in this query
    std::set<TypeEncoding> generationTypes = {};
    for (const Resistance& res : typeInteractions.begin()->second) {
        generationTypes.insert(res.type());
    }

    std::unordered_map<TypeEncoding,int> columnBuilder = {};
    optionTable_.push_back({TypeEncoding(""),0});
    itemTable_.push_back({TypeEncoding(""), 0, 1});
    links_.push_back({0, 0, 0, EMPTY_, 0});
    int index = 1;
    for (const TypeEncoding& type : generationTypes) {

        columnBuilder[type] = index;

        itemTable_.push_back({type, index - 1, index + 1});
        itemTable_[0].left++;

        links_.push_back({0, index, index, EMPTY_,0});

        numItems_++;
        index++;
    }
    itemTable_[itemTable_.size() - 1].right = 0;

    initializeColumns(typeInteractions, columnBuilder, requestedCoverSolution_);
}

void PokemonLinks::initializeColumns(const std::map<TypeEncoding,std::set<Resistance>>&
                                       typeInteractions,
                                       std::unordered_map<TypeEncoding,int>& columnBuilder,
                                       CoverageType requestedCoverage) {
    int previousSetSize = links_.size();
    int currentLinksIndex = links_.size();
    int typeLookupIndex = 1;
    for (const auto& type : typeInteractions) {

        int typeTitle = currentLinksIndex;
        int setSize = 0;
        // We will lookup our defense options in a seperate array with an O(1) index.
        links_.push_back({-typeLookupIndex,
                          currentLinksIndex - previousSetSize,
                          currentLinksIndex,
                          EMPTY_,
                          0});
        optionTable_.push_back({type.first,currentLinksIndex});

        for (const Resistance& singleType : type.second) {

            /* Important consideration for this algorithm. I am only interested in damage
             * resistances better than normal. So "covered" for a pokemon team means you found at
             * most 6 Pokemon that give you some level of resistance to all types in the game and
             * no pokemon on your team overlap by resisting the same types. You could have Pokemon
             * with x0.0, x0.25, or x0.5 resistances, but no higher. Maybe we could lessen criteria?
             * Also, just flip this condition for the ATTACK version. We want damage better than
             * Normal, meaining x2 or x4.
             */

            if ((requestedCoverage == DEFENSE ? singleType.multiplier() < NORMAL :
                                                NORMAL < singleType.multiplier())) {
                currentLinksIndex++;
                links_[typeTitle].down++;
                setSize++;

                TypeEncoding sType = singleType.type();
                links_[links_[columnBuilder[sType]].down].topOrLen++;

                // A single item in a circular doubly linked list points to itself.
                links_.push_back({links_[columnBuilder[sType]].down,
                                  currentLinksIndex,
                                  currentLinksIndex,
                                  singleType.multiplier(),
                                  0});

                // This is the adjustment to the column header's up field for a given item.
                links_[links_[columnBuilder[sType]].down].up = currentLinksIndex;
                // The current node is the new tail in a vertical circular linked list for an item.
                links_[currentLinksIndex].up = columnBuilder[sType];
                links_[currentLinksIndex].down = links_[columnBuilder[sType]].down;
                // Update the old tail to reflect the new addition of an item in its option.
                links_[columnBuilder[sType]].down = currentLinksIndex;
                // Similar to a previous/current coding pattern but in an above/below column.
                columnBuilder[sType] = currentLinksIndex;
            }
        }
        typeLookupIndex++;
        currentLinksIndex++;
        numOptions_++;
        previousSetSize = setSize;
    }
    links_.push_back({INT_MIN,
                      currentLinksIndex - previousSetSize,
                      INT_MIN,
                      EMPTY_,
                      0});
}

void PokemonLinks::buildAttackLinks(const std::map<TypeEncoding,std::set<Resistance>>&
                                        typeInteractions) {
    optionTable_.push_back({TypeEncoding(""),0});
    itemTable_.push_back({TypeEncoding(""), 0, 1});
    links_.push_back({0, 0, 0, EMPTY_,0});
    int index = 1;

    /* An inverted map has the attack types as the keys and the damage they do to defensive types
     * as the set of Resistances. Once this is built just use the same builder function for cols.
     */

    std::map<TypeEncoding,std::set<Resistance>> invertedMap = {};
    std::unordered_map<TypeEncoding,int> columnBuilder = {};
    for (const auto& interaction : typeInteractions) {
        columnBuilder[interaction.first] = index;
        itemTable_.push_back({interaction.first, index - 1, index + 1});
        itemTable_[0].left++;
        links_.push_back({0, index, index, EMPTY_,0});
        numItems_++;
        index++;
        for (const Resistance& attack : interaction.second) {
            invertedMap[attack.type()].insert({interaction.first, attack.multiplier()});
        }
    }
    itemTable_[itemTable_.size() - 1].right = 0;
    initializeColumns(invertedMap, columnBuilder, requestedCoverSolution_);
}


} // namespace DancingLinks
