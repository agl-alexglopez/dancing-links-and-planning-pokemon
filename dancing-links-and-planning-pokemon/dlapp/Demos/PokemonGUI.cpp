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
 * Author: Keith Schwarz with Modifications by Alex Lopez
 * File: PokemonGUI
 * ----------------
 * This file contains a mix of implementations. First, I take the Graph drawing logic written by
 * Keith Schwarz and Stanford Course Staff and apply it to Pokemon maps. I have marked with comments
 * the sections of the code that I did not write. I also modified the implementation to use the
 * STL whenever possible, especially for containers over the Stanford C++ Library. I am gradually
 * trying to move this project away from the Stanford Libraries and may even want to see if I can
 * rewrite the graphics for this specific application from scratch as an exercise. To use the GUI
 * you can solve the map for an entire generation by pressing the solver buttons. If instead you
 * would like to solve for specific gyms do so by selecting them with the G1-E4 buttons. Clear all
 * selections with the CL button.
 */
#include "GUI/MiniGUI.h"
#include "PokemonParser.h"
#include "PokemonLinks.h"
#include "Utilities/Resistance.h"
#include "Utilities/RankedSet.h"
#include <fstream>
#include <memory>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <queue>
#include <vector>
#include <regex>
#include <string>
#include <set>
#include "filelib.h"
#include "strlib.h"

namespace Dx = DancingLinks;
namespace {


/* * * * * * * * *    Start of Adapted Graph Drawing Algorithm by Keith Schwarz   * * * * * * * * */


/* File constants. */
const std::string kProblemSuffix = ".dst";
const std::string kBasePath = "res/dst/";
/* Background color. */
const std::string kBackgroundColor  = "#000000";

/* Colors to use when drawing the network. */
struct CityColors {
    std::string borderColor;
    std::string fillColor;
    MiniGUI::Font font;
};

enum CityState { // "The state the city is in," not "Singapore." :-)
    UNCOVERED,
    COVERED_DIRECTLY
};

enum MapDrawSelection {
    FULL_GENERATION=0,
    SELECTED_GYMS
};

/* Colors to use when drawing cities. */
const std::vector<CityColors> kColorOptions = {
    { "#101010", "#202020", MiniGUI::Font(MiniGUI::FontFamily::MONOSPACE, MiniGUI::FontStyle::BOLD, 12, "#A0A0A0") },   // Uncovered
    { "#806030", "#FFB000", MiniGUI::Font(MiniGUI::FontFamily::MONOSPACE, MiniGUI::FontStyle::BOLD, 12, "#000000") },   // Directly covered
};

const std::vector<std::string> BUTTON_TOGGLE_COLORS = {
    // NOT_SELECTED
    "#000000",
    // SELECTED
    "#FF0000"
};

/* Colors to use to draw the roads. */
const std::string kDarkRoadColor = "#505050";
const std::string kLightRoadColor = "#FFFFFF";

/* Line thicknesses. */
const double kRoadWidth = 3;
const double kCityWidth = 1.5;

/* Font to use for city labels. */
const std::string kLabelFont = "Monospace-BOLD-12";

/* Radius of a city */
const double kCityRadius = 25;

/* Buffer space around the window. */
const double kBufferSpace = 60;

/* Lower bound on the width or height of the data range, used for
 * collinear points.
 */
const double kLogicalPadding = 1e-5;

/* Max length of a string in a label. */
const std::string::size_type kMaxLength = 3;

/* Geometry information for drawing the network. */
struct Geometry {
    /* Range of X and Y values in the data set, used for
     * scaling everything.
     */
    double minDataX, minDataY, maxDataX, maxDataY;

    /* Range of X and Y values to use when drawing everything. */
    double minDrawX, minDrawY, maxDrawX, maxDrawY;
};

/* Given a data set, fills in the min and max X and Y values
 * encountered in that set.
 */
void computeDataBounds(const MapTest& network, Geometry& geo) {
    geo.minDataX = geo.minDataY = std::numeric_limits<double>::infinity();
    geo.maxDataX = geo.maxDataY = -std::numeric_limits<double>::infinity();

    for (const auto& cityName: network.cityLocations) {
        geo.minDataX = std::min(geo.minDataX, network.cityLocations.at(cityName.first).x);
        geo.minDataY = std::min(geo.minDataY, network.cityLocations.at(cityName.first).y);

        geo.maxDataX = std::max(geo.maxDataX, network.cityLocations.at(cityName.first).x);
        geo.maxDataY = std::max(geo.maxDataY, network.cityLocations.at(cityName.first).y);
    }

    /* Pad the boundaries. This accounts for the edge case where one set of bounds is
     * degenerate.
     */
    geo.minDataX -= kLogicalPadding;
    geo.minDataY -= kLogicalPadding;
    geo.maxDataX += kLogicalPadding;
    geo.maxDataY += kLogicalPadding;
}

/* Once we have the data bounds, we can compute the graphics bounds,
 * which will try to take maximum advantage of the width and height
 * that we have available to us.
 */
void computeGraphicsBounds(GWindow& window, Geometry& geo) {
    /* Get the aspect ratio of the window. */
    double winWidth  = window.getCanvasWidth()  - 2 * kBufferSpace;
    double winHeight = window.getCanvasHeight() - 2 * kBufferSpace;
    double winAspect = winWidth / winHeight;

    /* Get the aspect ratio of the data set. */
    double dataAspect = (geo.maxDataX - geo.minDataX) / (geo.maxDataY - geo.minDataY);

    double dataWidth, dataHeight;

    /* If the data aspect ratio exceeds the window aspect ratio,
     * the limiting factor in the display is going to be the
     * width. Therefore, we'll use that to determine our effective
     * width and height.
     */
    if (dataAspect >= winAspect) {
        dataWidth = winWidth;
        dataHeight = dataWidth / dataAspect;
    } else {
        dataHeight = winHeight;
        dataWidth = dataAspect * dataHeight;
    }

    /* Now, go center that in the window. */
    geo.minDrawX = (winWidth  -  dataWidth) / 2.0 + kBufferSpace;
    geo.minDrawY = (winHeight - dataHeight) / 2.0 + kBufferSpace;

    geo.maxDrawX = geo.minDrawX + dataWidth;
    geo.maxDrawY = geo.minDrawY + dataHeight;
}

/* Given the road network, determines its geometry. */
Geometry geometryFor(GWindow& window, const MapTest& network) {
    Geometry result;
    computeDataBounds(network, result);
    computeGraphicsBounds(window, result);
    return result;
}

/* Converts a coordinate in logical space into a coordinate in
 * physical space.
 */
GPoint logicalToPhysical(const GPoint& pt, const Geometry& geo) {
    double x = ((pt.x - geo.minDataX) / (geo.maxDataX - geo.minDataX)) * (geo.maxDrawX - geo.minDrawX) + geo.minDrawX;
    double y = ((pt.y - geo.minDataY) / (geo.maxDataY - geo.minDataY)) * (geo.maxDrawY - geo.minDrawY) + geo.minDrawY;

    return { x, y };
}

/* Draws all the roads in the network, highlighting ones that
 * are adjacent to lit cities.
 */
void drawRoads(GWindow& window,
                const Geometry& geo,
                const MapTest& network,
                const MapDrawSelection userSelection) {
    /* For efficiency's sake, just create one line. */
    GLine toDraw;
    toDraw.setLineWidth(kRoadWidth);

    for (const auto& source: network.network) {
        for (const std::string& dest: network.network.at(source.first)) {
            /* Selected roads draw in the bright color; deselected
             * roads draw in a the dark color.
             */
            toDraw.setColor(userSelection == FULL_GENERATION ? kLightRoadColor : kDarkRoadColor);

            /* Draw the line, remembering that the coordinates are in
             * logical rather than physical space.
             */
            auto src = logicalToPhysical(network.cityLocations.at(source.first), geo);
            auto dst = logicalToPhysical(network.cityLocations.at(dest), geo);
            toDraw.setStartPoint(src.x, src.y);
            toDraw.setEndPoint(dst.x, dst.y);

            window.draw(toDraw);
        }
    }
}

/* Returns a shortened name for the given city name. We use the first
 * three letters of the name if it's a single word and otherwise use
 * its initials.
 */
std::string shorthandFor(const std::string& name) {
    auto components = stringSplit(name, " ");
    if (components.size() == 0) {
        error("It shouldn't be possible for there to be no components of the city name.");
        return "";
    } else if (components.size() == 1) {
        if (components[0].length() < kMaxLength) return components[0];
        else return components[0].substr(0, 3);
    } else {
        /* Use initials. */
        std::string result;
        for (size_t i = 0; result.length() < kMaxLength && i < components.size(); i++) {
            /* Skip empty components, which might exist if there are consecutive spaces in
             * the name
             */
            if (!components[i].empty()) {
                result += components[i][0];
            }
        }
        return result;
    }
}

/* Draws all the cities, highlighting the ones that are in the
 * selected set.
 */
void drawCities(GWindow& window,
                 const Geometry& geo,
                 const MapTest& network,
                 const std::set<std::string>& selected) {

    /* For simplicity, just make a single oval. */
    GOval oval(0, 0, 2 * kCityRadius, 2 * kCityRadius);
    oval.setLineWidth(kCityWidth);
    oval.setFilled(true);

    for (const auto& city: network.network) {
        /* Figure out the center of the city on the screen. */
        auto center = logicalToPhysical(network.cityLocations.at(city.first), geo);

        /* See what state the city is in with regards to coverage. */
        CityState state = selected.count(city.first) ? COVERED_DIRECTLY : UNCOVERED;

        /* There's no way to draw a filled circle with a boundary as one call. */
        oval.setColor(kColorOptions[state].borderColor);
        oval.setFillColor(kColorOptions[state].fillColor);
        window.draw(oval,
                    center.x - kCityRadius,
                    center.y - kCityRadius);

        /* Set the label text and color. */
        auto render = TextRender::construct(shorthandFor(city.first), {
                                                center.x - kCityRadius,
                                                center.y - kCityRadius,
                                                2 * kCityRadius,
                                                2 * kCityRadius
                                            }, kColorOptions[state].font);
        render->alignCenterHorizontally();
        render->alignCenterVertically();
        render->draw(window);
    }
}

void visualizeNetwork(GWindow& window,
                      const PokemonTest& network,
                      const std::set<std::string>& selected,
                      const MapDrawSelection userSelection) {
    clearDisplay(window, kBackgroundColor);

    /* Edge case: Don't draw if the window is too small. */
    if (window.getCanvasWidth()  <= 2 * kBufferSpace ||
        window.getCanvasHeight() <= 2 * kBufferSpace) {
        return;
    }

    /* There's a weird edge case where if there are no cities,
     * the window geometry can't be calculated properly. Therefore,
     * we're going skip all this logic if there's nothing to draw.
     */
    if (!network.genMap.network.empty()) {
        Geometry geo = geometryFor(window, network.genMap);

        /* Draw the roads under the cities to avoid weird graphics
         * artifacts.
         */
        drawRoads(window, geo, network.genMap, userSelection);
        drawCities(window, geo, network.genMap, selected);
    }
}

std::vector<std::string> sampleProblems(const std::string& basePath) {
    std::vector<std::string> result;
    for (const auto& file: listDirectory(basePath)) {
        if (endsWith(file, kProblemSuffix)) {
            result.push_back(file);
        }
    }
    return result;
}


/* * * * * * * * *     End of Adapted Graph Drawing Algorithm by Keith Schwarz    * * * * * * * * */



/* You could alter the rules of pokemon here. The default team size is 6 Pokemon that you could
 * choose for defensive types and they can each learn 4 attack moves. However, there is only
 * 15-18 attack types in this game so there is effectively no limit on attack choices. So if
 * one day they add more attack types, the second limit for attack may become more relevant.
 */
const int POKEMON_TEAM_SIZE = 6;
const int POKEMON_TEAM_ATTACK_SLOTS = 24;
const int GYM_BUTTON_ROW_START = 6;
const int GYM_BUTTON_COL_START = 0;
const std::string CLEAR_SELECTIONS = "CL";
const std::string GBUTTON_STR = "GButton";

class PokemonGUI: public ProblemHandler {
public:
    PokemonGUI(GWindow& window);

    void actionPerformed(GObservable* source) override;
    void changeOccurredIn(GObservable* source) override;

protected:
    void repaint() override;

private:

    enum DlxRequest {
        EXACT,
        OVERLAPPING
    };

    enum ButtonToggle {
        NOT_SELECTED=0,
        SELECTED,
    };

    /* Dropdown of all the problems to choose from. */
    Temporary<GComboBox> mapDropdown;
    Temporary<GColorConsole> solutionsDisplay;
    const double DISPLAY_WIDTH = 900.0;

    /* Button to trigger the solver. */
    Temporary<GContainer> solutionControls;
    GButton* exactDefenseButton;
    GButton* exactAttackButton;
    GButton* overlappingDefenseButton;
    GButton* overlappingAttackButton;

    Temporary<GContainer> gymControls;
    std::unique_ptr<std::map<std::string,std::unique_ptr<GButton>>> gymButtons;

    /* Current network and solution. */
    PokemonTest generation;
    std::set<std::string> selectedGyms;
    std::set<std::string> allGyms;
    MapDrawSelection selectionDrawStyle;

    /* It can be costly in some generations to build and destroy larger PokemonLinks so we will
     * leave the full links solver in place for any given generations lifetime in the GUI. If
     * we are asked to solve for a smaller subset of gyms we will create a local PokemonLinks
     * object to solve that problem and leave this one intact until we switch maps.
     */
    std::unique_ptr<Dx::PokemonLinks> defenseDLX;
    std::unique_ptr<Dx::PokemonLinks> attackDLX;
    std::unique_ptr<std::set<RankedSet<std::string>>> allSolutions;

    /* Loads the world with the given name. */
    void loadWorld(const std::string& filename);

    void toggleSelectedGym(GButton& button);
    void toggleAllGyms(const ButtonToggle& buttonState);
    void clearSelections();
    void resetAllCoverages(Dx::PokemonLinks& dlxSolver, const DlxRequest& req, int depthLimit);
    void solveDefense(const DlxRequest& exactOrOverlapping);
    void solveAttack(const DlxRequest& exactOrOverlapping);
    void printDefenseSolution(const std::set<RankedSet<std::string>>& solution);
    void printAttackSolution(const std::set<RankedSet<std::string>>& solution);
    void printDefenseMessage();
    void printAttackMessage();
};

PokemonGUI::PokemonGUI(GWindow& window) : ProblemHandler(window) {
    exactDefenseButton  = new GButton("Exact Defense Coverage");
    exactDefenseButton->setTooltip("Which teams resist all attack types exactly once?");
    exactAttackButton  = new GButton("Exact Attack Coverage");
    exactAttackButton->setTooltip("Which attack types are effective against every defensive type exactly once?");
    overlappingDefenseButton  = new GButton("Loose Defense Coverage");
    overlappingDefenseButton->setTooltip("Which teams resist all attack types?");
    overlappingAttackButton  = new GButton("Loose Attack Coverage");
    overlappingAttackButton->setTooltip("Which attack types are effective against every defensive type?");

    gymControls = make_temporary<GContainer>(window, "WEST", GContainer::LAYOUT_GRID);
    gymButtons.reset(new std::map<std::string, std::unique_ptr<GButton>>);

    solutionControls = make_temporary<GContainer>(window, "WEST", GContainer::LAYOUT_GRID);
    solutionControls->addToGrid(exactDefenseButton, 0, 0);
    solutionControls->addToGrid(exactAttackButton, 1, 0);
    solutionControls->addToGrid(overlappingDefenseButton, 2, 0);
    solutionControls->addToGrid(overlappingAttackButton, 3, 0);
    solutionControls->setEnabled(false);
    GComboBox* choices = new GComboBox();
    for (const std::string& file: sampleProblems(kBasePath)) {
        choices->addItem(file);
    }
    choices->setEditable(false);
    mapDropdown = Temporary<GComboBox>(choices, window, "WEST");
    solutionsDisplay = Temporary<GColorConsole>(new GColorConsole(), window, "SOUTH");
    solutionsDisplay->setWidth(DISPLAY_WIDTH);
    solutionsDisplay->setStyle("black", GColorConsole::BOLD, FontSize{11});
    loadWorld(choices->getSelectedItem());
}

void PokemonGUI::changeOccurredIn(GObservable* source) {
    if (source == mapDropdown) {
        loadWorld(mapDropdown->getSelectedItem());
    }
}

void PokemonGUI::toggleSelectedGym(GButton& button) {
    std::string gymName = button.getText();
    if (selectedGyms.count(gymName)) {
        selectedGyms.erase(gymName);
        button.setForeground(BUTTON_TOGGLE_COLORS[NOT_SELECTED]);
    } else {
        selectedGyms.insert(gymName);
        button.setForeground(BUTTON_TOGGLE_COLORS[SELECTED]);
    }
    selectionDrawStyle = SELECTED_GYMS;
    requestRepaint();
}

void PokemonGUI::toggleAllGyms(const ButtonToggle& buttonState) {
    for (const auto& button : *gymButtons) {
        button.second->setForeground(BUTTON_TOGGLE_COLORS[buttonState]);
    }
}

void PokemonGUI::clearSelections() {
    selectedGyms.clear();
    allSolutions.reset();
    solutionsDisplay->clearDisplay();
    solutionsDisplay->flush();
    toggleAllGyms(NOT_SELECTED);
    selectionDrawStyle = SELECTED_GYMS;
    requestRepaint();
}

void PokemonGUI::actionPerformed(GObservable* source) {
    if (source == exactDefenseButton) {
        solveDefense(EXACT);
    } else if (source == exactAttackButton) {
        solveAttack(EXACT);
    } else if (source == overlappingDefenseButton) {
        solveDefense(OVERLAPPING);
    } else if (source == overlappingAttackButton) {
        solveAttack(OVERLAPPING);
    } else if (source->getType() == GBUTTON_STR){
        std::string gymName = dynamic_cast<GButton*>(source)->getText();
        if (gymName == CLEAR_SELECTIONS) {
            clearSelections();
        } else {
            toggleSelectedGym(*((*gymButtons)[gymName]));
        }
    }
}

void PokemonGUI::repaint() {
    if (selectionDrawStyle == FULL_GENERATION) {
        visualizeNetwork(window(), generation, allGyms, FULL_GENERATION);
    } else {
        visualizeNetwork(window(), generation, selectedGyms, SELECTED_GYMS);
    }
}

void PokemonGUI::loadWorld(const std::string& filename) {
    std::ifstream input(kBasePath + filename);
    if (!input) error("Cannot open file.");
    generation = loadPokemonGeneration(input);

    // The clear() method doesn't seem to clear the grid _/(*_*)\_. Need to remove each button.
    for (const auto& b : *gymButtons) {
        gymControls->remove(*b.second);
    }

    gymButtons.reset(new std::map<std::string, std::unique_ptr<GButton>>);

    for (const auto& s : generation.genMap.network) {
        allGyms.insert(s.first);
        gymButtons->insert({s.first, std::unique_ptr<GButton>(new GButton(s.first))});
    }
    gymButtons->insert({CLEAR_SELECTIONS, std::unique_ptr<GButton>(new GButton(CLEAR_SELECTIONS))});

    /* Create buttons for the gyms in a 2 by N grid where N is determined by how many gyms exist.
     * Different games have different numbers of gyms so we don't know until we build.
     *
     *      CL G1
     *      G2 G3
     *      G4 G5
     *      G6 G7
     *      G8 E4
     */
    int row = GYM_BUTTON_ROW_START;
    int col = GYM_BUTTON_COL_START;
    for (const auto& button : *gymButtons) {
        gymControls->addToGrid(*button.second, row, col);
        if ((++col %= 2) == 0) {
            row++;
        }
    }

    selectionDrawStyle = SELECTED_GYMS;

    defenseDLX.reset(new Dx::PokemonLinks(generation.interactions, Dx::PokemonLinks::DEFENSE));
    attackDLX.reset(new Dx::PokemonLinks(generation.interactions, Dx::PokemonLinks::ATTACK));
    allSolutions.reset();
    selectedGyms.clear();
    solutionsDisplay->clearDisplay();
    solutionsDisplay->flush();
    solutionControls->setEnabled(true);
    gymControls->setEnabled(true);
    toggleAllGyms(NOT_SELECTED);
    requestRepaint();
}

void PokemonGUI::printDefenseMessage() {
    (*solutionsDisplay) << "Defending against the following ";
    (*solutionsDisplay) << Dx::numItems(*defenseDLX);
    (*solutionsDisplay) << " attack types with "
                         << Dx::numOptions(*defenseDLX)
                         << " defense options:\n\n| ";
    for (const auto& g : Dx::items(*defenseDLX)) {
        (*solutionsDisplay) << g << " | ";
    }
    (*solutionsDisplay) << "\n" << std::endl;
}

void PokemonGUI::printAttackSolution(const std::set<RankedSet<std::string>>& solution) {
    *solutionsDisplay << "Found " << solution.size()
                       << " attack configurations SCORE | TYPES |. Higher score is better.\n";
    std::string maximumOutputExceeded = "\n";
    if (Dx::hasMaxSolutions(*attackDLX)) {
        maximumOutputExceeded = "...exceeded maximum output, stopping at "
                                + std::to_string(solution.size()) + ".\n\n";
    }
    *solutionsDisplay << maximumOutputExceeded;
    for (auto it = solution.rbegin(); it != solution.rend(); it++) {
        *solutionsDisplay << it->rank() << " | ";
        for (const std::string& type : *it) {
            *solutionsDisplay << type << " | ";
        }
        *solutionsDisplay << "\n";
    }
    *solutionsDisplay << maximumOutputExceeded << std::endl;
}

void PokemonGUI::printAttackMessage() {
    (*solutionsDisplay) << "Attacking the following "
                         << Dx::numItems(*attackDLX)
                         << " defensive types with "
                         << Dx::numOptions(*attackDLX)
                         << " attack options:\n\n| ";
    for (const auto& type : Dx::items(*attackDLX)) {
        (*solutionsDisplay) << type << " | ";
    }
    (*solutionsDisplay) << "\n" << std::endl;
}

void PokemonGUI::printDefenseSolution(const std::set<RankedSet<std::string>>& solution) {
    *solutionsDisplay << "Found " << solution.size()
                       << " Pokemon teams SCORE | TEAM |. Lower score is better.\n";

    std::string maximumOutputExceeded = "\n";
    if (Dx::hasMaxSolutions(*defenseDLX)) {
        maximumOutputExceeded = "...exceeded maximum output, stopping at "
                                + std::to_string(solution.size()) + ".\n\n";
    }
    *solutionsDisplay << maximumOutputExceeded;
    for (const RankedSet<std::string>& cov : solution) {
        *solutionsDisplay << cov.rank() << " | ";
        for (const std::string& type : cov) {
            *solutionsDisplay << type << " | ";
        }
        *solutionsDisplay << "\n";
    }
    *solutionsDisplay << maximumOutputExceeded << std::endl;
}

void PokemonGUI::resetAllCoverages(Dx::PokemonLinks& dlxSolver,
                                   const DlxRequest& req,
                                   int depthLimit) {
    if (req == EXACT) {
        allSolutions.reset(
            new std::set<RankedSet<std::string>>(Dx::solveExactCover(dlxSolver, depthLimit))
        );
    } else {
        allSolutions.reset(
            new std::set<RankedSet<std::string>>(Dx::solveOverlappingCover(dlxSolver, depthLimit))
        );
    }
}

void PokemonGUI::solveDefense(const DlxRequest& req) {
    allSolutions.reset();
    solutionsDisplay->clearDisplay();
    solutionsDisplay->flush();
    solutionControls->setEnabled(false);
    gymControls->setEnabled(false);
    mapDropdown->setEnabled(false);

    if (!selectedGyms.empty()) {
        selectionDrawStyle = SELECTED_GYMS;

        std::set<std::string>
        gymAttackTypes = loadSelectedGymsAttacks(mapDropdown->getSelectedItem(), selectedGyms);
        DancingLinks::hideItemsExcept(*defenseDLX, gymAttackTypes);

        printDefenseMessage();
        resetAllCoverages(*defenseDLX, req, POKEMON_TEAM_SIZE);
        printDefenseSolution(*allSolutions);
    } else {
        selectionDrawStyle = FULL_GENERATION;
        printDefenseMessage();
        resetAllCoverages(*defenseDLX, req, POKEMON_TEAM_SIZE);
        printDefenseSolution(*allSolutions);
    }

    /* Enable controls. */
    solutionControls->setEnabled(true);
    mapDropdown->setEnabled(true);
    gymControls->setEnabled(true);
    solutionsDisplay->scrollToTop();
    /* The PokemonLinks data structure can restore itself in place, unhiding items. */
    Dx::resetItems(*defenseDLX);

    requestRepaint();
}

void PokemonGUI::solveAttack(const DlxRequest& req) {
    allSolutions.reset();
    solutionsDisplay->clearDisplay();
    solutionsDisplay->flush();
    solutionControls->setEnabled(false);
    mapDropdown->setEnabled(false);
    gymControls->setEnabled(false);

    if (!selectedGyms.empty()) {
        selectionDrawStyle = SELECTED_GYMS;
        std::set<std::string>
        gymDefenseTypes = loadSelectedGymsDefenses(mapDropdown->getSelectedItem(),
                                                   selectedGyms);
        Dx::hideItemsExcept(*attackDLX, gymDefenseTypes);
        printAttackMessage();
        resetAllCoverages(*attackDLX, req, POKEMON_TEAM_ATTACK_SLOTS);
        printAttackSolution(*allSolutions);

    } else {
        selectionDrawStyle = FULL_GENERATION;
        printAttackMessage();
        resetAllCoverages(*attackDLX, req, POKEMON_TEAM_ATTACK_SLOTS);
        printAttackSolution(*allSolutions);
    }

    /* Enable controls. */
    solutionControls->setEnabled(true);
    mapDropdown->setEnabled(true);
    gymControls->setEnabled(true);
    solutionsDisplay->scrollToTop();
    Dx::resetItems(*attackDLX);

    requestRepaint();
}

} // namespace

GRAPHICS_HANDLER("Pokemon Planning", GWindow& window) {
    return std::make_shared<PokemonGUI>(window);
}
