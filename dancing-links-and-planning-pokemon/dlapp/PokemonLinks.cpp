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
 * the .h file and README.md in this repository.
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

bool hasItem(const PokemonLinks& dlx, const TypeEncoding& item) {
    return dlx.hasItem(item);
}

int numOptions(const PokemonLinks& dlx) {
    return dlx.getNumOptions();
}

bool hasOption(const PokemonLinks& dlx, const TypeEncoding& option) {
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

bool hideItem(PokemonLinks& dlx, const TypeEncoding& toHide) {
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

bool hideOption(PokemonLinks& dlx, const TypeEncoding& toHide) {
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


/* * * * * * * * * * * * * * * * *      TypeEncoding Utilities          * * * * * * * * * * * * * */


namespace {

// Half a byte is a nibble (aka nybl, nyble, nybble, half-byte...)
const uint32_t NYBL_SHIFT = 4;
const uint32_t NYBL_MASK = 0xf;
const uint32_t TYPE_MAX = 0x30000;
const size_t TYPE_TABLE_SIZE = 18;

// Lexographically organized table. 0th index is 0th bit in encoding. Goes up to 17th bit index.
std::string TYPE_ENCODING_TABLE[TYPE_TABLE_SIZE] = {
    "Bug","Dark","Dragon","Electric","Fairy","Fighting","Fire","Flying","Ghost","Grass",
    "Ground","Ice","Normal","Poison","Psychic","Rock","Steel","Water"
};

size_t binsearchBitIndex(std::string_view type) {
    for (size_t remain = TYPE_TABLE_SIZE, base = 0; remain; remain >>= 1) {
        size_t index = base + (remain >> 1);
        std::string_view found = TYPE_ENCODING_TABLE[index];
        if (found == type) {
            return index;
        }
        if (type > found) {
            base = index + 1;
            remain--;
        }
    }
    return 0;
}

} // namespace


TypeEncoding::TypeEncoding(std::string_view type) {
    if (type == "") {
        this->encoding_ = 0;
        return;
    }
    size_t delim = type.find('-');
    this->encoding_ = 1 << binsearchBitIndex(type.substr(0, delim));
    if (!this->encoding_ || delim == std::string::npos) {
        return;
    }
    size_t secondBitIndex = binsearchBitIndex(type.substr(delim + 1));
    if (!secondBitIndex) {
        this->encoding_ = 0;
        return;
    }
    this->encoding_ |= (1 << secondBitIndex);
}

/* Shifting a nyble at a time is a good balance. The second loop will have at most 3 shifts.
 * This could cut down the shifts to get to the bit we want significantly. Consider how many
 * shifts to get the indices of the following bits with normal single shifts and then with
 * nyble shifts. Nybl shifts are the size of a nybl, 4 bits.
 *
 *      Bug-Steel = 0x10001 = 0b10000 0000 0000 0001
 *
 * With single shifts it would take 16 bit shifts to get to the second type. With nyble shifts
 * we can get that bit in 4 shifts. At worst we have three additional shifts after a nybl shift.
 */
std::pair<std::string_view,std::string_view> TypeEncoding::to_pair() const {
    if (!this->encoding_) {
        return {};
    }
    TypeEncoding thisCopy = *this;
    int tableIndex = 0;
    while (thisCopy.encoding_) {
        if (!(thisCopy.encoding_ & NYBL_MASK)) {
            thisCopy.encoding_ >>= NYBL_SHIFT;
            tableIndex += NYBL_SHIFT;
            continue;
        }
        for (; !(thisCopy.encoding_ & 1); tableIndex++, thisCopy.encoding_ >>= 1) {
        }
        break;
    }
    std::string_view decoded = TYPE_ENCODING_TABLE[tableIndex];
    if (thisCopy.encoding_ == 1) {
        return {decoded, {}};
    }
    thisCopy.encoding_ >>= 1;
    tableIndex++;
    while (thisCopy.encoding_) {
        if (!(thisCopy.encoding_ & NYBL_MASK)) {
            thisCopy.encoding_ >>= NYBL_SHIFT;
            tableIndex += NYBL_SHIFT;
            continue;
        }
        for (; !(thisCopy.encoding_ & 1); tableIndex++, thisCopy.encoding_ >>= 1) {
        }
        break;
    }
    return {decoded, TYPE_ENCODING_TABLE[tableIndex]};
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
    int head = 0;
    for (int cur = itemTable_[0].right; cur != head; cur = itemTable_[cur].right) {
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

PokemonLinks::encodingAndNum PokemonLinks::overlappingCoverType(int indexInOption, int depthTag) {
    int i = indexInOption;
    encodingAndNum result = {};
    do {
        int top = links_[i].topOrLen;
        if (top <= 0) {
            i = links_[i].up;
            result.name = optionTable_[std::abs(links_[i - 1].topOrLen)].name;
        } else {
            /* Overlapping cover is much simpler at the cost of generating a tremendous number of
             * solutions. We only need to know which items and options are covered at which
             * recursive levels because we are more relaxed about leaving options available after
             * items in those options have been covered by other options.
             */

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

bool PokemonLinks::hideRequestedItem(const TypeEncoding& toHide) {
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

bool PokemonLinks::hasItem(const TypeEncoding& item) const {
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

bool PokemonLinks::hideRequestedOption(const TypeEncoding& toHide) {
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
    // We are not interested in the last node in the vector.
    for (int i = itemTable_.size(); i < links_.size() - 1; i = links_[i].down + 1) {
        if (links_[i].tag != HIDDEN
                && !toKeep.count(optionTable_[std::abs(links_[i].topOrLen)].name)) {
            hiddenOptions_.push_back(i);
            hideOption(i);
        }
    }
}

bool PokemonLinks::hasOption(const TypeEncoding& option) const {
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

int PokemonLinks::findItemIndex(const TypeEncoding& item) const {
    if (item.encoding_) {
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
    }
    // We know zero holds no value in the itemTable_ and this can double as a falsey value.
    return 0;
}

int PokemonLinks::findOptionIndex(const TypeEncoding& option) const {
    if (option.encoding_) {
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
    }
    // We know zero holds no value in the optionTable_ and this can double as a falsey value.
    return 0;
}


/* * * * * * * * * * * * * * * * *   Constructors and Links Build       * * * * * * * * * * * * * */


PokemonLinks::PokemonLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions,
                            const CoverageType requestedCoverSolution)
    : optionTable_(),
      itemTable_(),
      links_(),
      hiddenItems_(),
      hiddenOptions_(),
      maxOutput_(100000),
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

PokemonLinks::PokemonLinks(const std::map<std::string,std::set<Resistance>>& typeInteractions,
                            const std::set<std::string>& attackTypes)
    : optionTable_(),
      itemTable_(),
      links_(),
      hiddenItems_(),
      hiddenOptions_(),
      maxOutput_(100000),
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

        std::map<std::string,std::set<Resistance>> modifiedInteractions = {};
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

void PokemonLinks::buildDefenseLinks(const std::map<std::string,std::set<Resistance>>&
                                      typeInteractions) {
    // We always must gather all attack types available in this query
    std::set<std::string> generationTypes = {};
    for (const Resistance& res : typeInteractions.begin()->second) {
        generationTypes.insert(res.type());
    }

    std::map<TypeEncoding,int> columnBuilder = {};
    optionTable_.push_back({TypeEncoding(""),0});
    itemTable_.push_back({TypeEncoding(""), 0, 1});
    links_.push_back({0, 0, 0, Resistance::EMPTY_, 0});
    int index = 1;
    for (const std::string& type : generationTypes) {
        TypeEncoding typeCode(type);

        columnBuilder[typeCode] = index;

        itemTable_.push_back({typeCode, index - 1, index + 1});
        itemTable_[0].left++;

        links_.push_back({0, index, index, Resistance::EMPTY_,0});

        numItems_++;
        index++;
    }
    itemTable_[itemTable_.size() - 1].right = 0;

    initializeColumns(typeInteractions, columnBuilder, requestedCoverSolution_);
}

void PokemonLinks::initializeColumns(const std::map<std::string,std::set<Resistance>>&
                                       typeInteractions,
                                       std::map<TypeEncoding,int>& columnBuilder,
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
                          Resistance::EMPTY_,
                          0});
        optionTable_.push_back({TypeEncoding(type.first),currentLinksIndex});

        for (const Resistance& singleType : type.second) {

            /* Important consideration for this algorithm. I am only interested in damage
             * resistances better than normal. So "covered" for a pokemon team means you found at
             * most 6 Pokemon that give you some level of resistance to all types in the game and
             * no pokemon on your team overlap by resisting the same types. You could have Pokemon
             * with x0.0, x0.25, or x0.5 resistances, but no higher. Maybe we could lessen criteria?
             * Also, just flip this condition for the ATTACK version. We want damage better than
             * Normal, meaining x2 or x4.
             */

            if ((requestedCoverage == DEFENSE ? singleType.multiplier() < Resistance::NORMAL :
                                                Resistance::NORMAL < singleType.multiplier())) {
                currentLinksIndex++;
                links_[typeTitle].down++;
                setSize++;

                TypeEncoding sType(singleType.type());
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
                      Resistance::EMPTY_,
                      0});
}

void PokemonLinks::buildAttackLinks(const std::map<std::string,std::set<Resistance>>&
                                        typeInteractions) {
    optionTable_.push_back({TypeEncoding(""),0});
    itemTable_.push_back({TypeEncoding(""), 0, 1});
    links_.push_back({0, 0, 0, Resistance::EMPTY_,0});
    int index = 1;

    /* An inverted map has the attack types as the keys and the damage they do to defensive types
     * as the set of Resistances. Once this is built just use the same builder function for cols.
     */

    std::map<std::string,std::set<Resistance>> invertedMap = {};
    std::map<TypeEncoding,int> columnBuilder = {};
    for (const auto& interaction : typeInteractions) {
        TypeEncoding defenseType(interaction.first);
        columnBuilder[defenseType] = index;
        itemTable_.push_back({defenseType, index - 1, index + 1});
        itemTable_[0].left++;
        links_.push_back({0, index, index, Resistance::EMPTY_,0});
        numItems_++;
        index++;
        for (const Resistance& attack : interaction.second) {
            invertedMap[attack.type()].insert({interaction.first, attack.multiplier()});
        }
    }
    itemTable_[itemTable_.size() - 1].right = 0;
    initializeColumns(invertedMap, columnBuilder, requestedCoverSolution_);
}


/* * * * * * * * * * * * * * * *   Test Cases Below this Point    * * * * * * * * * * * * * * * * */


/* Implementation ends here and the following are just the operators for debugging and the tests
 * that use those operators. The namespace DancingLinks will end after the implementation of these
 * friend overloaded operators and the tests will begin.
 */

std::ostream& operator<<(std::ostream& out, const TypeEncoding& tp) {
    std::pair<std::string_view,std::string_view> toPrint = tp.to_pair();
    out << toPrint.first;
    if (toPrint.second != "") {
        out << "-" << toPrint.second;
    }
    return out;
}
std::ostream& operator<<(std::ostream& os,
                         const std::set<RankedSet<TypeEncoding>>& solution) {
    for (const auto& i : solution) {
        os << i;
    }
    return os << std::endl;
}
std::ostream& operator<<(std::ostream& os, const std::vector<TypeEncoding>& types) {
    for (const auto& t : types) {
        os << t << ",";
    }
    return os << std::endl;
}
std::ostream& operator<<(std::ostream& os, const std::set<TypeEncoding>& types) {
    for (const auto& t : types) {
        os << t << ",";
    }
    return os << std::endl;
}
bool operator==(const PokemonLinks::pokeLink& lhs, const PokemonLinks::pokeLink& rhs) {
    return lhs.topOrLen == rhs.topOrLen && lhs.up == rhs.up
            && lhs.down == rhs.down && lhs.multiplier == rhs.multiplier
             && lhs.tag == rhs.tag;
}
bool operator!=(const PokemonLinks::pokeLink& lhs, const PokemonLinks::pokeLink& rhs) {
    return !(lhs == rhs);
}
std::ostream& operator<<(std::ostream& os, const PokemonLinks::pokeLink& link) {
    return os << "{" << link.topOrLen
              << ", " << link.up << ", " << link.down << ", " << link.multiplier << "},";
}
bool operator==(const PokemonLinks::typeName& lhs, const PokemonLinks::typeName& rhs) {
    return lhs.name == rhs.name && lhs.left == rhs.left && lhs.right == rhs.right;
}
bool operator!=(const PokemonLinks::typeName& lhs, const PokemonLinks::typeName& rhs) {
    return !(lhs == rhs);
}
std::ostream& operator<<(std::ostream& os, const PokemonLinks::typeName& type) {
    return os << "{ name: " << type.name
              << ", left: " << type.left << ", right: " << type.right << " }";
}
bool operator==(const std::vector<PokemonLinks::pokeLink>& lhs,
                const std::vector<PokemonLinks::pokeLink>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (int i = 0; i < lhs.size(); i++) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}
bool operator!=(const std::vector<PokemonLinks::pokeLink>& lhs,
                const std::vector<PokemonLinks::pokeLink>& rhs) {
    return !(lhs == rhs);
}
bool operator==(const std::vector<PokemonLinks::typeName>& lhs,
                const std::vector<PokemonLinks::typeName>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (int i = 0; i < lhs.size(); i++) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}
bool operator!=(const std::vector<PokemonLinks::typeName>& lhs,
                const std::vector<PokemonLinks::typeName>& rhs) {
    return !(lhs == rhs);
}
std::ostream& operator<<(std::ostream& os, const std::vector<PokemonLinks::pokeLink>& links) {
    os << "DLX ARRAY" << std::endl;
    for (int index = 0; index < links.size(); index++) {
        PokemonLinks::pokeLink item = links[index];
        if (item.topOrLen < 0) {
            os << "\n";
        }
        os << "{" << item.topOrLen << ","
           << item.up << "," << item.down << "," << item.multiplier << "," << item.tag << "},";
    }
    os << std::endl;
    return os;
}
std::ostream& operator<<(std::ostream&os,
                                const std::vector<PokemonLinks::typeName>& items) {
    os << "LOOKUP TABLE" << std::endl;
    for (const auto& item : items) {
        os << "{\"" << item.name << "\"," << item.left << "," << item.right << "},\n";
    }
    os << std::endl;
    return os;
}
bool operator==(const PokemonLinks::encodingAndNum& lhs, const PokemonLinks::encodingAndNum& rhs) {
    return lhs.name == rhs.name && lhs.name == rhs.name;
}
bool operator!=(const PokemonLinks::encodingAndNum& lhs, const PokemonLinks::encodingAndNum& rhs) {
    return !(lhs == rhs);
}
std::ostream& operator<<(std::ostream& os, const PokemonLinks::encodingAndNum& nN) {
    return os << "{\"" << nN.name << "\"," << nN.name << "}";
}
bool operator==(const std::vector<PokemonLinks::encodingAndNum>& lhs,
                const std::vector<PokemonLinks::encodingAndNum>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (int i = 0; i < lhs.size(); i++) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}
bool operator!=(const std::vector<PokemonLinks::encodingAndNum>& lhs,
                const std::vector<PokemonLinks::encodingAndNum>& rhs) {
    return !(lhs == rhs);
}
std::ostream& operator<<(std::ostream& os, const std::vector<PokemonLinks::encodingAndNum>& nN) {
    for (const auto& i : nN) {
        os << i;
    }
    return os << std::endl;
}


} // namespace DancingLinks


/* * * * * * * * * * * * * * *      Tests Below This Point      * * * * * * * * * * * * * * * * * */


/* These type names completely crowd out our test cases when I construct the dlx grids in the test
 * making them hard to read. They stretch too far horizontally so I am creating these name codes
 * here that should only be used in this translation unit for readablity and convenience when
 * building tests. Refer here if terminology in the tests is confusing. Also, hovering over the
 * code in a test case in QT should show you the full constexpr for Resistances.
 */

namespace Dx = DancingLinks;
namespace {

constexpr Resistance::Multiplier EM = Resistance::EMPTY_;
constexpr Resistance::Multiplier IM = Resistance::IMMUNE;
constexpr Resistance::Multiplier F4 = Resistance::FRAC14;
constexpr Resistance::Multiplier F2 = Resistance::FRAC12;
constexpr Resistance::Multiplier NM = Resistance::NORMAL;
constexpr Resistance::Multiplier DB = Resistance::DOUBLE;
constexpr Resistance::Multiplier QD = Resistance::QUADRU;

} // namespace


/* * * * * * * *  Test the Type Encoding We Use To Represent Pokemon Types in Bits  * * * * * * * */

STUDENT_TEST("Easiest type encoding lexographically is Bug.") {
    uint32_t hexType = 0x1;
    std::string_view strType = "Bug";
    Dx::TypeEncoding code(strType);
    EXPECT_EQUAL(hexType, code.encoding_);
    EXPECT_EQUAL(strType, code.to_pair().first);
    std::string_view empty{};
    EXPECT_EQUAL(empty, code.to_pair().second);
}

STUDENT_TEST("Test the next simplist dual type Bug-Dark") {
    uint32_t hexType = 0x3;
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
    const uint32_t ENCODE_END = 0x40000;
    for (uint32_t bit1 = 1, type1 = 0;
            bit1 < ENCODE_END && type1 < Dx::TYPE_TABLE_SIZE;
                bit1 <<= 1, type1++) {

        std::string checkSingleType(Dx::TYPE_ENCODING_TABLE[type1]);
        Dx::TypeEncoding singleTypeEncoding(checkSingleType);
        EXPECT_EQUAL(singleTypeEncoding.encoding_, bit1);
        EXPECT_EQUAL(singleTypeEncoding.to_pair().first, checkSingleType);
        EXPECT_EQUAL(singleTypeEncoding.to_pair().second, {});

        for (uint32_t bit2 = bit1 << 1, type2 = type1 + 1;
                bit2 < ENCODE_END && type2 < Dx::TYPE_TABLE_SIZE;
                    bit2 <<= 1, type2++) {

            std::string checkDualType = checkSingleType + "-" + Dx::TYPE_ENCODING_TABLE[type2];
            Dx::TypeEncoding dualTypeEncoding(checkDualType);
            EXPECT_EQUAL(dualTypeEncoding.encoding_, bit1 | bit2);
            std::ostringstream captureType;
            captureType << dualTypeEncoding;
            std::string dualTypeString = captureType.str();
            EXPECT_EQUAL(dualTypeString, checkDualType);
        }
    }
}


/* * * * * * * * * * * * * * * * * *   Defense Links Init   * * * * * * * * * * * * * * * * * * * */


STUDENT_TEST("Initialize small defensive links") {
    /*
     *
     *          Fire   Normal    Water   <-Attack
     *  Ghost          x0.0              <-Defense
     *  Water   x0.5             x0.5
     *
     */
    const std::map<std::string,std::set<Resistance>> types {
        {"Ghost", {{"Fire",NM},{"Normal",IM},{"Water",NM}}},
        {"Water", {{"Fire",F2},{"Normal",NM},{"Water",F2}}},
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
    const std::map<std::string,std::set<Resistance>> types {
        {"Dragon", {{"Normal",NM},{"Fire",F2},{"Water",F2},{"Electric",F2},{"Grass",F2},{"Ice",DB}}},
        {"Electric", {{"Normal",NM},{"Fire",NM},{"Water",NM},{"Electric",F2},{"Grass",NM},{"Ice",NM}}},
        {"Ghost", {{"Normal",IM},{"Fire",NM},{"Water",NM},{"Electric",NM},{"Grass",NM},{"Ice",NM}}},
        {"Ice", {{"Normal",NM},{"Fire",NM},{"Water",NM},{"Electric",NM},{"Grass",NM},{"Ice",F2}}},
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
    const std::map<std::string,std::set<Resistance>> types {
        {"Dragon", {{"Normal",NM},{"Fire",F2},{"Water",F2},{"Electric",F2},{"Grass",F2},{"Ice",DB}}},
        {"Electric", {{"Normal",NM},{"Fire",NM},{"Water",NM},{"Electric",F2},{"Grass",NM},{"Ice",NM}}},
        {"Ghost", {{"Normal",IM},{"Fire",NM},{"Water",NM},{"Electric",NM},{"Grass",NM},{"Ice",NM}}},
        {"Ice", {{"Normal",NM},{"Fire",NM},{"Water",NM},{"Electric",NM},{"Grass",NM},{"Ice",F2}}},
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
    const std::map<std::string,std::set<Resistance>> types {
        {"Electric", {{"Electric",F2},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",NM}}},
        {"Fire", {{"Electric",F2},{"Fire",NM},{"Grass",F2},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Grass", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Ice", {{"Electric",NM},{"Fire",NM},{"Grass",NM},{"Ice",F2},{"Normal",NM},{"Water",F2}}},
        {"Normal", {{"Electric",F2},{"Fire",NM},{"Grass",NM},{"Ice",NM},{"Normal",F2},{"Water",NM}}},
        {"Water", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
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
    const std::map<std::string,std::set<Resistance>> types {
        {"Electric", {{"Electric",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",NM}}},
        {"Ghost", {{"Electric",NM},{"Grass",NM},{"Ice",NM},{"Normal",IM},{"Water",NM}}},
        {"Ground", {{"Electric",IM},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",NM}}},
        {"Ice", {{"Electric",NM},{"Grass",NM},{"Ice",F2},{"Normal",NM},{"Water",NM}}},
        {"Poison", {{"Electric",NM},{"Grass",F2},{"Ice",NM},{"Normal",NM},{"Water",NM}}},
        {"Water", {{"Electric",NM},{"Grass",DB},{"Ice",F2},{"Normal",NM},{"Water",F2}}},
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
    const std::map<std::string,std::set<Resistance>> types = {
        /* In reality maps will have every type present in every key. But I know the internals
         * of my implementation and will just enter all types for the first key to make entering
         * the rest of the test cases easier.
         */
        {"Bug-Ghost",{{"Electric",NM},{"Fire",NM},{"Grass",F2},{"Ice",NM},{"Normal",IM},{"Water",NM}}},
        {"Electric-Grass",{{"Electric",F4},{"Grass",F2},{"Water",F2}}},
        {"Fire-Flying",{{"Fire",F2},{"Grass",F4}}},
        {"Ground-Water",{{"Electric",IM},{"Fire",F2}}},
        {"Ice-Psychic",{{"Ice",F2}}},
        {"Ice-Water",{{"Ice",F4},{"Water",F2}}},
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
    const std::map<std::string,std::set<Resistance>> types {
        {"Ground-Rock", {{"Electric",IM},{"Fire",NM},{"Water",QD}}},
        {"Ground-Grass", {{"Electric",IM},{"Fire",DB},{"Water",NM}}},
        {"Fire-Flying", {{"Electric",DB},{"Fire",F2},{"Water",DB}}},
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
    const std::map<std::string,std::set<Resistance>> types = {
        {"Electric", {{"Ground",DB}}},
        {"Fire", {{"Ground",DB}}},
        {"Grass", {{"Ice",DB},{"Poison",DB}}},
        {"Ice", {{"Fighting",DB}}},
        {"Normal", {{"Fighting",DB}}},
        {"Water", {{"Grass",DB}}},
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
    const std::map<std::string,std::set<Resistance>> types = {
        {"Bug-Ghost",{{"Electric",NM},{"Fire",DB},{"Grass",F2},{"Ice",NM},{"Normal",IM},{"Water",NM}}},
        {"Electric-Grass",{{"Electric",F4},{"Fire",DB},{"Grass",F2},{"Ice",F2},{"Normal",NM},{"Water",F2}}},
        {"Fire-Flying",{{"Electric",DB},{"Fire",F2},{"Grass",F4},{"Ice",F2},{"Normal",NM},{"Water",DB}}},
        {"Ground-Water",{{"Electric",IM},{"Fire",F2},{"Grass",QD},{"Ice",NM},{"Normal",NM},{"Water",NM}}},
        {"Ice-Psychic",{{"Electric",NM},{"Fire",DB},{"Grass",NM},{"Ice",F2},{"Normal",NM},{"Water",NM}}},
        {"Ice-Water",{{"Electric",DB},{"Fire",NM},{"Grass",DB},{"Ice",F2},{"Normal",NM},{"Water",F2}}},
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
    const std::map<std::string,std::set<Resistance>> types {
        {"Electric", {{"Electric",F2},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",NM}}},
        {"Fire", {{"Electric",F2},{"Fire",NM},{"Grass",F2},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Grass", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Ice", {{"Electric",NM},{"Fire",NM},{"Grass",NM},{"Ice",F2},{"Normal",NM},{"Water",F2}}},
        {"Normal", {{"Electric",F2},{"Fire",NM},{"Grass",NM},{"Ice",NM},{"Normal",F2},{"Water",NM}}},
        {"Water", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
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
    const std::map<std::string,std::set<Resistance>> types {
        {"Electric", {{"Electric",F2},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",NM}}},
        {"Fire", {{"Electric",F2},{"Fire",NM},{"Grass",F2},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Grass", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Ice", {{"Electric",NM},{"Fire",NM},{"Grass",NM},{"Ice",F2},{"Normal",NM},{"Water",F2}}},
        {"Normal", {{"Electric",F2},{"Fire",NM},{"Grass",NM},{"Ice",NM},{"Normal",F2},{"Water",NM}}},
        {"Water", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
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
    const std::map<std::string,std::set<Resistance>> types = {
        /* In reality maps will have every type present in every key. But I know the internals
         * of my implementation and will just enter all types for the first key to make entering
         * the rest of the test cases easier.
         */
        {"Bug-Ghost",{{"Electric",NM},{"Fire",NM},{"Grass",F2},{"Ice",NM},{"Normal",IM},{"Water",NM}}},
        {"Electric-Grass",{{"Electric",F4},{"Grass",F2},{"Water",F2}}},
        {"Fire-Flying",{{"Fire",F2},{"Grass",F4}}},
        {"Ground-Water",{{"Electric",IM},{"Fire",F2}}},
        {"Ice-Psychic",{{"Ice",F2}}},
        {"Ice-Water",{{"Ice",F4},{"Water",F2}}},
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
    const std::map<std::string,std::set<Resistance>> types {
        {"Electric", {{"Electric",F2},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",NM}}},
        {"Fire", {{"Electric",F2},{"Fire",NM},{"Grass",F2},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Grass", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Ice", {{"Electric",NM},{"Fire",NM},{"Grass",NM},{"Ice",F2},{"Normal",NM},{"Water",F2}}},
        {"Normal", {{"Electric",F2},{"Fire",NM},{"Grass",NM},{"Ice",NM},{"Normal",F2},{"Water",NM}}},
        {"Water", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
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
    const std::map<std::string,std::set<Resistance>> types {
        {"Electric", {{"Electric",F2},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",NM}}},
        {"Fire", {{"Electric",F2},{"Fire",NM},{"Grass",F2},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Grass", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Ice", {{"Electric",NM},{"Fire",NM},{"Grass",NM},{"Ice",F2},{"Normal",NM},{"Water",F2}}},
        {"Normal", {{"Electric",F2},{"Fire",NM},{"Grass",NM},{"Ice",NM},{"Normal",F2},{"Water",NM}}},
        {"Water", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
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
    const std::map<std::string,std::set<Resistance>> types {
        {"Electric", {{"Electric",F2},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",NM}}},
        {"Fire", {{"Electric",F2},{"Fire",NM},{"Grass",F2},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Grass", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Ice", {{"Electric",NM},{"Fire",NM},{"Grass",NM},{"Ice",F2},{"Normal",NM},{"Water",F2}}},
        {"Normal", {{"Electric",F2},{"Fire",NM},{"Grass",NM},{"Ice",NM},{"Normal",F2},{"Water",NM}}},
        {"Water", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
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
    const std::map<std::string,std::set<Resistance>> types {
        {"Electric", {{"Electric",F2},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",NM}}},
        {"Fire", {{"Electric",F2},{"Fire",NM},{"Grass",F2},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Grass", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Ice", {{"Electric",NM},{"Fire",NM},{"Grass",NM},{"Ice",F2},{"Normal",NM},{"Water",F2}}},
        {"Normal", {{"Electric",F2},{"Fire",NM},{"Grass",NM},{"Ice",NM},{"Normal",F2},{"Water",NM}}},
        {"Water", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
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
    const std::map<std::string,std::set<Resistance>> types {
        {"Electric", {{"Electric",F2},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",NM}}},
        {"Fire", {{"Electric",F2},{"Fire",NM},{"Grass",F2},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Grass", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Ice", {{"Electric",NM},{"Fire",NM},{"Grass",NM},{"Ice",F2},{"Normal",NM},{"Water",F2}}},
        {"Normal", {{"Electric",F2},{"Fire",NM},{"Grass",NM},{"Ice",NM},{"Normal",F2},{"Water",NM}}},
        {"Water", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
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
    const std::map<std::string,std::set<Resistance>> types {
        {"Electric", {{"Electric",F2},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",NM}}},
        {"Fire", {{"Electric",F2},{"Fire",NM},{"Grass",F2},{"Ice",NM},{"Normal",NM},{"Water",DB}}},
        {"Grass", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Ice", {{"Electric",NM},{"Fire",NM},{"Grass",NM},{"Ice",F2},{"Normal",NM},{"Water",F2}}},
        {"Normal", {{"Electric",F2},{"Fire",NM},{"Grass",NM},{"Ice",NM},{"Normal",F2},{"Water",NM}}},
        {"Water", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
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
    const std::map<std::string,std::set<Resistance>> types {
        {"Electric", {{"Electric",F2},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",NM}}},
        {"Fire", {{"Electric",F2},{"Fire",NM},{"Grass",F2},{"Ice",NM},{"Normal",NM},{"Water",DB}}},
        {"Grass", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Ice", {{"Electric",NM},{"Fire",NM},{"Grass",NM},{"Ice",F2},{"Normal",NM},{"Water",F2}}},
        {"Normal", {{"Electric",F2},{"Fire",NM},{"Grass",NM},{"Ice",NM},{"Normal",F2},{"Water",NM}}},
        {"Water", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
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
    const std::map<std::string,std::set<Resistance>> types {
        {"Electric", {{"Electric",F2},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",NM}}},
        {"Fire", {{"Electric",F2},{"Fire",NM},{"Grass",F2},{"Ice",NM},{"Normal",NM},{"Water",DB}}},
        {"Grass", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Ice", {{"Electric",NM},{"Fire",NM},{"Grass",NM},{"Ice",F2},{"Normal",NM},{"Water",F2}}},
        {"Normal", {{"Electric",F2},{"Fire",NM},{"Grass",NM},{"Ice",NM},{"Normal",F2},{"Water",NM}}},
        {"Water", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
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
    const std::map<std::string,std::set<Resistance>> types {
        {"Electric", {{"Electric",F2},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",NM}}},
        {"Fire", {{"Electric",F2},{"Fire",NM},{"Grass",F2},{"Ice",NM},{"Normal",NM},{"Water",DB}}},
        {"Grass", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
        {"Ice", {{"Electric",NM},{"Fire",NM},{"Grass",NM},{"Ice",F2},{"Normal",NM},{"Water",F2}}},
        {"Normal", {{"Electric",F2},{"Fire",NM},{"Grass",NM},{"Ice",NM},{"Normal",F2},{"Water",NM}}},
        {"Water", {{"Electric",NM},{"Fire",F2},{"Grass",NM},{"Ice",NM},{"Normal",NM},{"Water",F2}}},
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
