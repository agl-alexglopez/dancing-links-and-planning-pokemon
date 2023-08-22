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
#include <climits>
#include <cmath>
#include <cstdint>
#include <stdexcept>


namespace Dancing_links {


/* * * * * * * * * * * * *       Convenience Callers for Encapsulation      * * * * * * * * * * * */


std::set<Ranked_set<Type_encoding>> solveExactCover(Pokemon_links& dlx, int8_t choiceLimit) {
    return dlx.getExactCoverages(choiceLimit);
}

std::set<Ranked_set<Type_encoding>> solveOverlappingCover(Pokemon_links& dlx, int8_t choiceLimit) {
    return dlx.getOverlappingCoverages(choiceLimit);
}

bool hasMaxSolutions(const Pokemon_links& dlx) {
    return dlx.reachedOutputLimit();
}

uint64_t numItems(const Pokemon_links& dlx) {
    return dlx.getNumItems();
}

bool hasItem(const Pokemon_links& dlx, Type_encoding item) {
    return dlx.hasItem(item);
}

uint64_t numOptions(const Pokemon_links& dlx) {
    return dlx.getNumOptions();
}

bool hasOption(const Pokemon_links& dlx, Type_encoding option) {
    return dlx.hasOption(option);
}

Pokemon_links::Coverage_type coverageType(const Pokemon_links& dlx) {
    return dlx.getLinksType();
}

std::vector<Type_encoding> items(const Pokemon_links& dlx) {
    return dlx.getItems();
}

std::vector<Type_encoding> options(const Pokemon_links& dlx) {
    return dlx.getOptions();
}

bool hideItem(Pokemon_links& dlx, Type_encoding toHide) {
    return dlx.hideRequestedItem(toHide);
}

bool hideItem(Pokemon_links& dlx, const std::vector<Type_encoding>& toHide) {
    return dlx.hideRequestedItem(toHide);
}

bool hideItem(Pokemon_links& dlx, const std::vector<Type_encoding>& toHide,
               std::vector<Type_encoding>& failedToHide) {
    return dlx.hideRequestedItem(toHide, failedToHide);
}

void hideItemsExcept(Pokemon_links& dlx, const std::set<Type_encoding>& toKeep) {
    dlx.hideAllItemsExcept(toKeep);
}

uint64_t numHidItems(const Pokemon_links& dlx) {
    return dlx.getNumHidItems();
}

Type_encoding peekHidItem(const Pokemon_links& dlx) {
    return dlx.peekHidItem();
}

void popHidItem(Pokemon_links& dlx) {
    dlx.popHidItem();
}

bool hidItemsEmpty(const Pokemon_links& dlx) {
    return dlx.hidItemsEmpty();
}

std::vector<Type_encoding> hidItems(const Pokemon_links& dlx) {
    return dlx.getHidItems();
}

void resetItems(Pokemon_links& dlx) {
    dlx.resetItems();
}

bool hideOption(Pokemon_links& dlx, Type_encoding toHide) {
    return dlx.hideRequestedOption(toHide);
}

bool hideOption(Pokemon_links& dlx, const std::vector<Type_encoding>& toHide) {
    return dlx.hideRequestedOption(toHide);
}

bool hideOption(Pokemon_links& dlx, const std::vector<Type_encoding>& toHide,
                 std::vector<Type_encoding>& failedToHide) {
    return dlx.hideRequestedOption(toHide, failedToHide);
}

void hideOptionsExcept(Pokemon_links& dlx, const std::set<Type_encoding>& toKeep) {
    dlx.hideAllOptionsExcept(toKeep);
}

uint64_t numHidOptions(const Pokemon_links& dlx) {
    return dlx.getNumHidOptions();
}

Type_encoding peekHidOption(const Pokemon_links& dlx) {
    return dlx.peekHidOption();
}

void popHidOption(Pokemon_links& dlx) {
    dlx.popHidOption();
}

bool hidOptionsEmpty(const Pokemon_links& dlx) {
    return dlx.hidOptionsEmpty();
}

std::vector<Type_encoding> hidOptions(const Pokemon_links& dlx) {
    return dlx.getHidOptions();
}

void resetOptions(Pokemon_links& dlx) {
    dlx.resetOptions();
}

void resetAll(Pokemon_links& dlx) {
    dlx.resetItemsOptions();
}


/* * * * * * * * * * * * * * * *    Algorithm X via Dancing Links   * * * * * * * * * * * * * * * */


std::set<Ranked_set<Type_encoding>> Pokemon_links::getExactCoverages(int8_t choiceLimit) {
    std::set<Ranked_set<Type_encoding>> coverages = {};
    Ranked_set<Type_encoding> coverage {};
    hitLimit_ = false;
    fillExactCoverages(coverages, coverage, choiceLimit);
    return coverages;
}

void Pokemon_links::fillExactCoverages(std::set<Ranked_set<Type_encoding>>& coverages, // NOLINT
                                       Ranked_set<Type_encoding>& coverage,
                                       int8_t depthLimit) {
    if (itemTable_[0].right == 0 && depthLimit >= 0) {
        coverages.insert(coverage);
        return;
    }
    // Depth limit is either the size of a Pokemon Team or the number of attack slots on a team.
    if (depthLimit <= 0) {
        return;
    }
    uint64_t itemToCover = chooseItem();
    // An item has become inaccessible due to our chosen options so far, undo.
    if (!itemToCover) {
        return;
    }
    for (uint64_t cur = links_[itemToCover].down; cur != itemToCover; cur = links_[cur].down) {
        Encoding_score score = coverType(cur);
        coverage.insert(score.score, score.name);

        fillExactCoverages(coverages, coverage, static_cast<int8_t>(depthLimit - 1));

        /* It is possible for these algorithms to produce many many sets. To make the Pokemon
         * Planner GUI more usable I cut off recursion if we are generating too many sets.
         */
        if (coverages.size() == maxOutput_) {
            hitLimit_ = true;
            uncoverType(cur);
            return;
        }
        coverage.erase(score.score, score.name);
        uncoverType(cur);
    }
}

Pokemon_links::Encoding_score Pokemon_links::coverType(uint64_t indexInOption) {
    Encoding_score result = {};
    uint64_t i = indexInOption;
    do {
        int top = links_[i].topOrLen;
        /* This is the next spacer node for the next option. We now know how to find the title of
         * our current option if we go back to the start of the chosen option and go left.
         */
        if (top <= 0) {
            i = links_[i].up;
            result.name = optionTable_[std::abs(links_[i - 1].topOrLen)].name;
            continue;
        }
        if (!links_[top].tag) {
            Type_name cur = itemTable_[top];
            itemTable_[cur.left].right = cur.right;
            itemTable_[cur.right].left = cur.left;
            hideOptions(i);
            /* If there is a better way to score the teams or attack schemes we build here would
             * be the place to change it. I just give points based on how good the resistance or
             * attack strength is. Immunity is better than quarter is better than half damage if
             * we are building defense. Quad is better than double damage if we are building
             * attack types. Points only change by increments of one.
             */
            result.score += links_[i].multiplier;
        }
        i++;
    } while (i != indexInOption);
    return result;
}

void Pokemon_links::uncoverType(uint64_t indexInOption) {
    // Go left first so the in place link restoration of the doubly linked lookup table works.
    uint64_t i = --indexInOption;
    do {
        int top = links_[i].topOrLen;
        if (top <= 0) {
            i = links_[i].down;
            continue;
        }
        if (!links_[top].tag) {
            Type_name cur = itemTable_[top];
            itemTable_[cur.left].right = top;
            itemTable_[cur.right].left = top;
            unhideOptions(i);
        }
        i--;
    } while (i != indexInOption);
}

/* The hide/unhide technique is what makes exact cover so much more restrictive and fast at
 * shrinking the problem. Notice how aggressively it eliminates the appearances of items across
 * other options. When compared to Overlapping Coverage, Exact Coverage answers a different
 * question but also shrinks the problem much more quickly.
 */

void Pokemon_links::hideOptions(uint64_t indexInOption) {
    for (uint64_t row = links_[indexInOption].down; row != indexInOption; row = links_[row].down) {
        if (static_cast<int>(row) == links_[indexInOption].topOrLen) {
            continue;
        }
        for (uint64_t col = row + 1; col != row;) {
            int top = links_[col].topOrLen;
            if (top <= 0) {
                col = links_[col].up;
                continue;
            }
            Poke_link cur = links_[col];
            links_[cur.up].down = cur.down;
            links_[cur.down].up = cur.up;
            links_[top].topOrLen--;
            col++;
        }
    }
}

void Pokemon_links::unhideOptions(uint64_t indexInOption) {
    for (uint64_t row = links_[indexInOption].up; row != indexInOption; row = links_[row].up) {
        if (static_cast<int>(row) == links_[indexInOption].topOrLen) {
            continue;
        }
        for (uint64_t col = row - 1; col != row;) {
            int top = links_[col].topOrLen;
            if (top <= 0) {
                col = links_[col].down;
                continue;
            }
            Poke_link cur = links_[col];
            links_[cur.up].down = col;
            links_[cur.down].up = col;
            links_[top].topOrLen++;
            col--;
        }
    }
}


/* * * * * * * * * * * *  Shared Choosing Heuristic for Both Techniques * * * * * * * * * * * * * */


uint64_t Pokemon_links::chooseItem() const {
    int32_t min = INT32_MAX;
    uint64_t chosenIndex = 0;
    for (uint64_t cur = itemTable_[0].right; cur != 0; cur = itemTable_[cur].right) {
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


std::set<Ranked_set<Type_encoding>>
Pokemon_links::getOverlappingCoverages(int8_t choiceLimit) {
    std::set<Ranked_set<Type_encoding>> coverages = {};
    Ranked_set<Type_encoding> coverage = {};
    hitLimit_ = false;
    fillOverlappingCoverages(coverages, coverage, choiceLimit);
    return coverages;
}

void Pokemon_links::fillOverlappingCoverages(std::set<Ranked_set<Type_encoding>>& coverages, // NOLINT
                                             Ranked_set<Type_encoding>& coverage,
                                             int8_t depthTag) {
    if (itemTable_[0].right == 0 && depthTag >= 0) {
        coverages.insert(coverage);
        return;
    }
    if (depthTag <= 0) {
        return;
    }
    // In certain generations certain types have no weaknesses so we might return 0 here.
    uint64_t itemToCover = chooseItem();
    if (!itemToCover) {
        return;
    }

    for (uint64_t cur = links_[itemToCover].down; cur != itemToCover; cur = links_[cur].down) {
        Encoding_score score = overlappingCoverType({cur, depthTag});
        coverage.insert(score.score, score.name);

        fillOverlappingCoverages(coverages, coverage, static_cast<int8_t>(depthTag - 1));

        /* It is possible for these algorithms to produce many many sets. To make the Pokemon
         * Planner GUI more usable I cut off recursion if we are generating too many sets.
         */
        if (coverages.size() == maxOutput_) {
            hitLimit_ = true;
            overlappingUncoverType(cur);
            return;
        }
        coverage.erase(score.score, score.name);
        overlappingUncoverType(cur);
    }
}

/* Overlapping cover is much simpler at the cost of generating a tremendous number of solutions. We
 * only need to know which items and options are covered at which recursive levels because we are
 * more relaxed about leaving options available after items in those options have been covered by
 * other options.
 */

Pokemon_links::Encoding_score Pokemon_links::overlappingCoverType(Pokemon_links::Cover_tag tag) {
    uint64_t i = tag.index;
    Encoding_score result = {};
    do {
        int top = links_[i].topOrLen;
        if (top <= 0) {
            i = links_[i].up;
            result.name = optionTable_[std::abs(links_[i - 1].topOrLen)].name;
            continue;
        }
        if (!links_[top].tag) {
            links_[top].tag = tag.tag;
            itemTable_[itemTable_[top].left].right = itemTable_[top].right;
            itemTable_[itemTable_[top].right].left = itemTable_[top].left;
            result.score += links_[i].multiplier;
        }
        links_[top].tag == hidden_ ? i++ : links_[i++].tag = tag.tag;
    } while (i != tag.index);

    return result;
}

void Pokemon_links::overlappingUncoverType(uint64_t indexInOption) {
    uint64_t i = --indexInOption;
    do {
        int top = links_[i].topOrLen;
        if (top < 0) {
            i = links_[i].down;
            continue;
        }
        if (links_[top].tag == links_[i].tag) {
            links_[top].tag = 0;
            itemTable_[itemTable_[top].left].right = top;
            itemTable_[itemTable_[top].right].left = top;
        }
        links_[top].tag == hidden_ ? i-- : links_[i--].tag = 0;
    } while (i != indexInOption);
}


/* * * * * * * * * * * * * * * * *        Utility Functions             * * * * * * * * * * * * * */


bool Pokemon_links::reachedOutputLimit() const {
    return hitLimit_;
}

uint64_t Pokemon_links::getNumItems() const {
    return numItems_;
}

uint64_t Pokemon_links::getNumOptions() const {
    return numOptions_;
}

Pokemon_links::Coverage_type Pokemon_links::getLinksType() const {
    return requestedCoverSolution_;
}

std::vector<Type_encoding> Pokemon_links::getItems() const {
    std::vector<Type_encoding> result = {};
    for (uint64_t i = itemTable_[0].right; i != 0; i = itemTable_[i].right) {
        result.push_back(itemTable_[i].name);
    }
    return result;
}

std::vector<Type_encoding> Pokemon_links::getHidItems() const {
    std::vector<Type_encoding>result = {};
    result.reserve(hiddenItems_.size());
    for (const auto& i : hiddenItems_) {
        result.push_back(itemTable_[i].name);
    }
    return result;
}

std::vector<Type_encoding> Pokemon_links::getOptions() const {
    std::vector<Type_encoding> result = {};
    // Hop from row title to row title, skip hidden options. Skip bookend node that is placeholder.
    for (uint64_t i = itemTable_.size(); i < links_.size() - 1; i = links_[i].down + 1) {
        if (links_[i].tag != hidden_) {
            result.push_back(optionTable_[i].name);
        }
    }
    return result;
}

std::vector<Type_encoding> Pokemon_links::getHidOptions() const {
    std::vector<Type_encoding> result = {};
    result.reserve(hiddenOptions_.size());
    for (const auto& i : hiddenOptions_) {
        result.push_back(optionTable_[std::abs(links_[i].topOrLen)].name);
    }
    return result;
}

bool Pokemon_links::hideRequestedItem(Type_encoding toHide) {
    uint64_t lookupIndex = findItemIndex(toHide);
    // Can't find or this item has already been hidden.
    if (lookupIndex && links_[lookupIndex].tag != hidden_) {
        hiddenItems_.push_back(lookupIndex);
        hideItem(lookupIndex);
        return true;
    }
    return false;
}

bool Pokemon_links::hideRequestedItem(const std::vector<Type_encoding>& toHide) {
    bool result = true;
    for (const auto& t : toHide) {
        if (!hideRequestedItem(t)) {
            result = false;
        }
    }
    return result;
}

bool Pokemon_links::hideRequestedItem(const std::vector<Type_encoding>& toHide,
                                       std::vector<Type_encoding>& failedToHide) {
    bool result = true;
    for (const auto& t : toHide) {
        if (!hideRequestedItem(t)) {
            result = false;
            failedToHide.push_back(t);
        }
    }
    return result;
}

void Pokemon_links::hideAllItemsExcept(const std::set<Type_encoding>& toKeep) {
    for (uint64_t i = itemTable_[0].right; i != 0; i = itemTable_[i].right) {
        if (!toKeep.contains(itemTable_[i].name)) {
            hiddenItems_.push_back(i);
            hideItem(i);
        }
    }

}

bool Pokemon_links::hasItem(Type_encoding item) const {
    uint64_t found = findItemIndex(item);
    return found && links_[found].tag != hidden_;
}

void Pokemon_links::popHidItem() {
    if (!hiddenItems_.empty()) {
        unhideItem(hiddenItems_.back());
        hiddenItems_.pop_back();
    } else {
        std::cout << "No hidden items. Stack is empty." << std::endl;
        throw;
    }
}

Type_encoding Pokemon_links::peekHidItem() const {
    if (!hiddenItems_.empty()) {
        return itemTable_[hiddenItems_.back()].name;
    }
    std::cout << "No hidden items. Stack is empty." << std::endl;
    throw;
}

bool Pokemon_links::hidItemsEmpty() const {
    return hiddenItems_.empty();
}

uint64_t Pokemon_links::getNumHidItems() const {
    return hiddenItems_.size();
}

void Pokemon_links::resetItems() {
    while (!hiddenItems_.empty()) {
        unhideItem(hiddenItems_.back());
        hiddenItems_.pop_back();
    }
}

bool Pokemon_links::hideRequestedOption(Type_encoding toHide) {
    uint64_t lookupIndex = findOptionIndex(toHide);
    // Couldn't find or this option has already been hidden.
    if (lookupIndex && links_[lookupIndex].tag != hidden_) {
        hiddenOptions_.push_back(lookupIndex);
        hideOption(lookupIndex);
        return true;
    }
    return false;
}

bool Pokemon_links::hideRequestedOption(const std::vector<Type_encoding>& toHide) {
    bool result = true;
    for (const auto& h : toHide) {
        if (!hideRequestedOption(h)) {
            result = false;
        }
    }
    return result;
}

bool Pokemon_links::hideRequestedOption(const std::vector<Type_encoding>& toHide,
                                        std::vector<Type_encoding>& failedToHide) {
    bool result = true;
    for (const auto& h : toHide) {
        if (!hideRequestedOption(h)) {
            failedToHide.push_back(h);
            result = false;
        }
    }
    return result;
}

void Pokemon_links::hideAllOptionsExcept(const std::set<Type_encoding>& toKeep) {
    // We start i at the index of the first option spacer. This is after the column headers.
    for (uint64_t i = itemTable_.size(); i < links_.size() - 1; i = links_[i].down + 1) {
        if (links_[i].tag != hidden_
                && !toKeep.contains(optionTable_[std::abs(links_[i].topOrLen)].name)) {
            hiddenOptions_.push_back(i);
            hideOption(i);
        }
    }
}

bool Pokemon_links::hasOption(Type_encoding option) const {
    uint64_t found = findOptionIndex(option);
    return found && links_[found].tag != hidden_;
}

void Pokemon_links::popHidOption() {
    if (!hiddenOptions_.empty()) {
        unhideOption(hiddenOptions_.back());
        hiddenOptions_.pop_back();
    } else {
        std::cout << "No hidden items. Stack is empty." << std::endl;
        throw;
    }
}

Type_encoding Pokemon_links::peekHidOption() const {
    if (!hiddenOptions_.empty()) {
        // Row spacer tiles in the links hold their name as a negative index in the optionTable_
        return optionTable_[std::abs(links_[hiddenOptions_.back()].topOrLen)].name;
    }
    return Type_encoding("");
}

bool Pokemon_links::hidOptionsEmpty() const {
    return hiddenOptions_.empty();
}

uint64_t Pokemon_links::getNumHidOptions() const {
    return hiddenOptions_.size();
}

void Pokemon_links::resetOptions() {
    while (!hiddenOptions_.empty()) {
        unhideOption(hiddenOptions_.back());
        hiddenOptions_.pop_back();
    }
}

void Pokemon_links::resetItemsOptions() {
    resetItems();
    resetOptions();
}

void Pokemon_links::hideItem(uint64_t headerIndex) {
    Type_name curItem = itemTable_[headerIndex];
    itemTable_[curItem.left].right = curItem.right;
    itemTable_[curItem.right].left = curItem.left;
    links_[headerIndex].tag = hidden_;
    numItems_--;
}

void Pokemon_links::unhideItem(uint64_t headerIndex) {
    Type_name curItem = itemTable_[headerIndex];
    itemTable_[curItem.left].right = headerIndex;
    itemTable_[curItem.right].left = headerIndex;
    links_[headerIndex].tag = 0;
    numItems_++;
}

void Pokemon_links::hideOption(uint64_t rowIndex) {
    links_[rowIndex].tag = hidden_;
    for (uint64_t i = rowIndex + 1; links_[i].topOrLen > 0; i++) {
        Poke_link cur = links_[i];
        links_[cur.up].down = cur.down;
        links_[cur.down].up = cur.up;
        links_[cur.topOrLen].topOrLen--;
    }
    numOptions_--;
}

void Pokemon_links::unhideOption(uint64_t rowIndex) {
    links_[rowIndex].tag = 0;
    for (uint64_t i = rowIndex + 1; links_[i].topOrLen > 0; i++) {
        Poke_link cur = links_[i];
        links_[cur.up].down = i;
        links_[cur.down].up = i;
        links_[cur.topOrLen].topOrLen++;
    }
    numOptions_++;
}

uint64_t Pokemon_links::findItemIndex(Type_encoding item) const {
    for (uint64_t nremain = itemTable_.size(), base = 0; nremain != 0; nremain >>= 1) {
        uint64_t curIndex = base + (nremain >> 1);
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

uint64_t Pokemon_links::findOptionIndex(Type_encoding option) const {
    for (uint64_t nremain = optionTable_.size(), base = 0; nremain != 0; nremain >>= 1) {
        uint64_t curIndex = base + (nremain >> 1);
        if (optionTable_[curIndex].name == option) {
            // This is the index corresponding to the spacer node for an option in the links.
            return optionTable_[curIndex].index;
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


Pokemon_links::Pokemon_links(const std::map<Type_encoding,std::set<Resistance>>& typeInteractions,
                            const Coverage_type requestedCoverSolution)
    : requestedCoverSolution_(requestedCoverSolution) {
    if (requestedCoverSolution == defense) {
        buildDefenseLinks(typeInteractions);
    } else if (requestedCoverSolution == attack){
        buildAttackLinks(typeInteractions);
    } else {
        std::cerr << "Invalid requested cover solution. Choose ATTACK or DEFENSE." << std::endl;
        std::abort();
    }
}

Pokemon_links::Pokemon_links(const std::map<Type_encoding,std::set<Resistance>>& typeInteractions,
                            const std::set<Type_encoding>& attackTypes)
    : requestedCoverSolution_(defense) {
    if (attackTypes.empty()) {
        buildDefenseLinks(typeInteractions);
    } else {

        /* If we want altered attack types to defend against, it is more efficient and explicit
         * to pass in their own set then eliminate them from the Generation map by making a
         * smaller copy.
         */

        std::map<Type_encoding,std::set<Resistance>> modifiedInteractions = {};
        for (const auto& type : typeInteractions) {
            modifiedInteractions[type.first] = {};
            for (const Resistance& t : type.second) {
                if (attackTypes.contains(t.type())) {
                    modifiedInteractions[type.first].insert(t);
                }
            }
        }
        buildDefenseLinks(modifiedInteractions);
    }
}

void Pokemon_links::buildDefenseLinks(const std::map<Type_encoding,std::set<Resistance>>&
                                      typeInteractions) {
    // We always must gather all attack types available in this query
    std::set<Type_encoding> generationTypes = {};
    for (const Resistance& res : typeInteractions.begin()->second) {
        generationTypes.insert(res.type());
    }

    std::unordered_map<Type_encoding,uint64_t> columnBuilder = {};
    optionTable_.push_back({Type_encoding(""),0});
    itemTable_.push_back({Type_encoding(""), 0, 1});
    links_.push_back({0, 0, 0, emp, 0});
    uint64_t index = 1;
    for (const Type_encoding& type : generationTypes) {

        columnBuilder[type] = index;

        itemTable_.push_back({type, index - 1, index + 1});
        itemTable_[0].left++;

        links_.push_back({0, index, index, emp,0});

        numItems_++;
        index++;
    }
    itemTable_[itemTable_.size() - 1].right = 0;

    initializeColumns(typeInteractions, columnBuilder, requestedCoverSolution_);
}

void Pokemon_links::initializeColumns(const std::map<Type_encoding,std::set<Resistance>>&
                                       typeInteractions,
                                       std::unordered_map<Type_encoding,uint64_t>& columnBuilder,
                                       Coverage_type requestedCoverage) {
    uint64_t previousSetSize = links_.size();
    uint64_t currentLinksIndex = links_.size();
    int32_t typeLookupIndex = 1;
    for (const auto& type : typeInteractions) {

        uint64_t typeTitle = currentLinksIndex;
        int setSize = 0;
        // We will lookup our defense options in a seperate array with an O(1) index.
        links_.push_back({-typeLookupIndex,
                          currentLinksIndex - previousSetSize,
                          currentLinksIndex,
                          emp,
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

            if ((requestedCoverage == defense ? singleType.multiplier() < nrm :
                                                nrm < singleType.multiplier())) {
                currentLinksIndex++;
                links_[typeTitle].down++;
                setSize++;

                Type_encoding sType = singleType.type();
                links_[links_[columnBuilder[sType]].down].topOrLen++;

                // A single item in a circular doubly linked list points to itself.
                links_.push_back({static_cast<int>(links_[columnBuilder[sType]].down),
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
                      UINT64_MAX,
                      emp,
                      0});
}

void Pokemon_links::buildAttackLinks(const std::map<Type_encoding,std::set<Resistance>>&
                                        typeInteractions) {
    optionTable_.push_back({Type_encoding(""),0});
    itemTable_.push_back({Type_encoding(""), 0, 1});
    links_.push_back({0, 0, 0, emp,0});
    uint64_t index = 1;

    /* An inverted map has the attack types as the keys and the damage they do to defensive types
     * as the set of Resistances. Once this is built just use the same builder function for cols.
     */

    std::map<Type_encoding,std::set<Resistance>> invertedMap = {};
    std::unordered_map<Type_encoding,uint64_t> columnBuilder = {};
    for (const auto& interaction : typeInteractions) {
        columnBuilder[interaction.first] = index;
        itemTable_.push_back({interaction.first, index - 1, index + 1});
        itemTable_[0].left++;
        links_.push_back({0, index, index, emp,0});
        numItems_++;
        index++;
        for (const Resistance& attack : interaction.second) {
            invertedMap[attack.type()].insert({interaction.first, attack.multiplier()});
        }
    }
    itemTable_[itemTable_.size() - 1].right = 0;
    initializeColumns(invertedMap, columnBuilder, requestedCoverSolution_);
}


} // namespace Dancing_links
