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

namespace {


    /* * * * * * *    Start of Adapted Graph Drawing Algorithm by Keith Schwarz   * * * * * * * * */


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

    typedef enum MapDrawSelection {
        FULL_GENERATION=0,
        SELECTED_GYMS
    }MapDrawSelection;

    /* Colors to use when drawing cities. */
    const std::vector<CityColors> kColorOptions = {
        { "#101010", "#202020", MiniGUI::Font(MiniGUI::FontFamily::MONOSPACE, MiniGUI::FontStyle::BOLD, 12, "#A0A0A0") },   // Uncovered
        { "#806030", "#FFB000", MiniGUI::Font(MiniGUI::FontFamily::MONOSPACE, MiniGUI::FontStyle::BOLD, 12, "#000000") },   // Directly covered
    };

    const std::string GYM_1_STR = "G1";
    const std::string GYM_2_STR = "G2";
    const std::string GYM_3_STR = "G3";
    const std::string GYM_4_STR = "G4";
    const std::string GYM_5_STR = "G5";
    const std::string GYM_6_STR = "G6";
    const std::string GYM_7_STR = "G7";
    const std::string GYM_8_STR = "G8";
    const std::string ELT_4_STR = "E4";

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
        if (!network.pokemonGenerationMap.network.empty()) {
            Geometry geo = geometryFor(window, network.pokemonGenerationMap);

            /* Draw the roads under the cities to avoid weird graphics
             * artifacts.
             */
            drawRoads(window, geo, network.pokemonGenerationMap, userSelection);
            drawCities(window, geo, network.pokemonGenerationMap, selected);
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


    /* * * * * * *     End of Adapted Graph Drawing Algorithm by Keith Schwarz    * * * * * * * * */


    /* You could alter the rules of pokemon here. The default team size is 6 Pokemon that you could
     * choose for defensive types and they can each learn 4 attack moves. However, there is only
     * 15-18 attack types in this game so there is effectively no limit on attack choices. So if
     * one day they add more attack types, the second limit for attack may become more relevant.
     */
    namespace {
        const int POKEMON_TEAM_SIZE = 6;
        const int POKEMON_TEAM_ATTACK_SLOTS = 24;
    }

    class PokemonGUI: public ProblemHandler {
    public:
        PokemonGUI(GWindow& window);

        void actionPerformed(GObservable* source) override;
        void changeOccurredIn(GObservable* source) override;

    protected:
        void repaint() override;

    private:

        typedef enum CoverageRequested {
            EXACT,
            OVERLAPPING
        }CoverageRequested;

        typedef enum ButtonToggle {
            NOT_SELECTED=0,
            SELECTED,
        }ButtonToggle;

        /* Dropdown of all the problems to choose from. */
        Temporary<GComboBox> mProblems;
        Temporary<GColorConsole> mSolutionsDisplay;
        const double DISPLAY_WIDTH = 900.0;

        /* Button to trigger the solver. */
        Temporary<GContainer> controls;
        GButton* exactDefenseButton;
        GButton* exactAttackButton;
        GButton* overlappingDefenseButton;
        GButton* overlappingAttackButton;

        Temporary<GContainer> gymControls;
        /* I would rather use checkboxes but am having trouble hooking them to event listeners. */
        GButton* gym1;
        GButton* gym2;
        GButton* gym3;
        GButton* gym4;
        GButton* gym5;
        GButton* gym6;
        GButton* gym7;
        GButton* gym8;
        GButton* elite4;
        GButton* clearChoices;

        /* Current network and solution. */
        PokemonTest mGeneration;
        std::set<std::string> mSelected;
        std::set<std::string> mAllSelected;
        std::set<std::string> mAllGenerationAttackTypes;
        MapDrawSelection mUserSelection;

        std::unique_ptr<std::set<RankedSet<std::string>>> mAllCoverages;

        /* Loads the world with the given name. */
        void loadWorld(const std::string& filename);

        void toggleSelectedGym(GButton*& button);
        void toggleAllGyms(const ButtonToggle& buttonState);
        void clearSelections();
        void solveDefense(const CoverageRequested& exactOrOverlapping);
        void solveAttack(const CoverageRequested& exactOrOverlapping);
        void printDefenseSolution(bool hitLimit, const std::set<RankedSet<std::string>>& solution);
        void printAttackSolution(bool hitLimit, const std::set<RankedSet<std::string>>& solution);
        void printDefenseMessage(const std::set<std::string>& attacksToPrint,int numDefenseOptions);
        void printAttackMessage(const std::map<std::string,std::set<Resistance>>& defenseToPrint,
                                int numAttackOptions);
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

        gym1 = new GButton(GYM_1_STR);
        gym2 = new GButton(GYM_2_STR);
        gym3 = new GButton(GYM_3_STR);
        gym4 = new GButton(GYM_4_STR);
        gym5 = new GButton(GYM_5_STR);
        gym6 = new GButton(GYM_6_STR);
        gym7 = new GButton(GYM_7_STR);
        gym8 = new GButton(GYM_8_STR);
        elite4 = new GButton(ELT_4_STR);
        clearChoices = new GButton("CL");
        gymControls = make_temporary<GContainer>(window, "WEST", GContainer::LAYOUT_GRID);
        gymControls->setTooltip("Leave gym selections blank to solve for all types in a generation.");
        gymControls->addToGrid(gym1, 6, 0);
        gymControls->addToGrid(gym2, 6, 1);
        gymControls->addToGrid(gym3, 7, 0);
        gymControls->addToGrid(gym4, 7, 1);
        gymControls->addToGrid(gym5, 8, 0);
        gymControls->addToGrid(gym6, 8, 1);
        gymControls->addToGrid(gym7, 9, 0);
        gymControls->addToGrid(gym8, 9, 1);
        gymControls->addToGrid(elite4, 10, 0);
        gymControls->addToGrid(clearChoices, 10, 1);

        gymControls->setEnabled(false);

        controls = make_temporary<GContainer>(window, "WEST", GContainer::LAYOUT_GRID);
        controls->addToGrid(exactDefenseButton, 0, 0);
        controls->addToGrid(exactAttackButton, 1, 0);
        controls->addToGrid(overlappingDefenseButton, 2, 0);
        controls->addToGrid(overlappingAttackButton, 3, 0);
        controls->setEnabled(false);
        GComboBox* choices = new GComboBox();
        for (const std::string& file: sampleProblems(kBasePath)) {
            choices->addItem(file);
        }
        choices->setEditable(false);
        mProblems = Temporary<GComboBox>(choices, window, "WEST");

        mSolutionsDisplay = Temporary<GColorConsole>(new GColorConsole(), window, "SOUTH");
        mSolutionsDisplay->setWidth(DISPLAY_WIDTH);
        mSolutionsDisplay->setStyle("black", GColorConsole::BOLD, FontSize{11});

        loadWorld(choices->getSelectedItem());
    }

    void PokemonGUI::changeOccurredIn(GObservable* source) {
        if (source == mProblems) {
            loadWorld(mProblems->getSelectedItem());
        }
    }

    void PokemonGUI::toggleSelectedGym(GButton*& button) {
        std::string gymName = button->getText();
        if (mSelected.count(gymName)) {
            mSelected.erase(gymName);
            button->setForeground(BUTTON_TOGGLE_COLORS[NOT_SELECTED]);
        } else {
            mSelected.insert(gymName);
            button->setForeground(BUTTON_TOGGLE_COLORS[SELECTED]);
        }
        mUserSelection = SELECTED_GYMS;
        requestRepaint();
    }

    void PokemonGUI::toggleAllGyms(const ButtonToggle& buttonState) {
        gym1->setForeground(BUTTON_TOGGLE_COLORS[buttonState]);
        gym2->setForeground(BUTTON_TOGGLE_COLORS[buttonState]);
        gym3->setForeground(BUTTON_TOGGLE_COLORS[buttonState]);
        gym4->setForeground(BUTTON_TOGGLE_COLORS[buttonState]);
        gym5->setForeground(BUTTON_TOGGLE_COLORS[buttonState]);
        gym6->setForeground(BUTTON_TOGGLE_COLORS[buttonState]);
        gym7->setForeground(BUTTON_TOGGLE_COLORS[buttonState]);
        gym8->setForeground(BUTTON_TOGGLE_COLORS[buttonState]);
        elite4->setForeground(BUTTON_TOGGLE_COLORS[buttonState]);
    }

    void PokemonGUI::clearSelections() {
        mSelected.clear();
        mAllCoverages.reset();
        mSolutionsDisplay->clearDisplay();
        mSolutionsDisplay->flush();
        toggleAllGyms(NOT_SELECTED);
        mUserSelection = SELECTED_GYMS;
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
        } else if (source == gym1) {
            toggleSelectedGym(gym1);
        } else if (source == gym2) {
            toggleSelectedGym(gym2);
        } else if (source == gym3) {
            toggleSelectedGym(gym3);
        } else if (source == gym4) {
            toggleSelectedGym(gym4);
        } else if (source == gym5) {
            toggleSelectedGym(gym5);
        } else if (source == gym6) {
            toggleSelectedGym(gym6);
        } else if (source == gym7) {
            toggleSelectedGym(gym7);
        } else if (source == gym8) {
            toggleSelectedGym(gym8);
        } else if (source == elite4) {
            toggleSelectedGym(elite4);
        } else if (source == clearChoices) {
            clearSelections();
        }
    }

    void PokemonGUI::repaint() {
        if (mUserSelection == FULL_GENERATION) {
            visualizeNetwork(window(), mGeneration, mAllSelected, FULL_GENERATION);
        } else {
            visualizeNetwork(window(), mGeneration, mSelected, SELECTED_GYMS);
        }
    }

    void PokemonGUI::loadWorld(const std::string& filename) {
        std::ifstream input(kBasePath + filename);
        if (!input) error("Cannot open file.");

        mGeneration = loadPokemonGeneration(input);
        for (const auto& s : mGeneration.pokemonGenerationMap.network) {
            mAllSelected.insert(s.first);
        }
        for (const auto& attack : mGeneration.typeInteractions.begin()->second) {
            mAllGenerationAttackTypes.insert(attack.type());
        }
        mUserSelection = SELECTED_GYMS;


        mAllCoverages.reset();
        mSelected.clear();
        mSolutionsDisplay->clearDisplay();
        mSolutionsDisplay->flush();
        controls->setEnabled(true);
        gymControls->setEnabled(true);
        toggleAllGyms(NOT_SELECTED);
        requestRepaint();
    }

    void PokemonGUI::printDefenseMessage(const std::set<std::string>& attacksToPrint,
                                         int numDefenseOptions) {
        (*mSolutionsDisplay) << "Defending against the following ";
        if (attacksToPrint.empty()) {
            (*mSolutionsDisplay) << mAllGenerationAttackTypes.size();
        } else {
            (*mSolutionsDisplay) << attacksToPrint.size();
        }
        (*mSolutionsDisplay) << " attack types with " << numDefenseOptions
                             << " defense options:\n\n| ";
        for (const auto& g : attacksToPrint) {
            (*mSolutionsDisplay) << g << " | ";
        }
        (*mSolutionsDisplay) << "\n" << std::endl;
    }

    void PokemonGUI::printAttackSolution(bool hitLimit,
                                         const std::set<RankedSet<std::string>>& solution) {
        *mSolutionsDisplay << "Found " << solution.size()
                           << " attack configurations SCORE | TYPES |. Higher score is better.\n";
        std::string maximumOutputExceeded = "\n";
        if (hitLimit) {
            maximumOutputExceeded = "...exceeded maximum output, stopping at "
                                    + std::to_string(solution.size()) + ".\n\n";
        }
        *mSolutionsDisplay << maximumOutputExceeded;
        for (auto it = solution.rbegin(); it != solution.rend(); it++) {
            *mSolutionsDisplay << it->rank() << " | ";
            for (const std::string& type : *it) {
                *mSolutionsDisplay << type << " | ";
            }
            *mSolutionsDisplay << "\n";
        }
        *mSolutionsDisplay << maximumOutputExceeded << std::endl;
    }

    void PokemonGUI::printAttackMessage(const std::map<std::string,
                                        std::set<Resistance>>& defenseToPrint,
                                        int numAttackOptions) {
        (*mSolutionsDisplay) << "Attacking the following " << defenseToPrint.size()
                             << " defensive types with " << numAttackOptions
                             << " attack options:\n\n| ";
        for (const auto& type : defenseToPrint) {
            (*mSolutionsDisplay) << type.first << " | ";
        }
        (*mSolutionsDisplay) << "\n" << std::endl;
    }

    void PokemonGUI::printDefenseSolution(bool hitLimit,
                                          const std::set<RankedSet<std::string>>& solution) {
        *mSolutionsDisplay << "Found " << solution.size()
                           << " Pokemon teams SCORE | TEAM |. Lower score is better.\n";

        std::string maximumOutputExceeded = "\n";
        if (hitLimit) {
            maximumOutputExceeded = "...exceeded maximum output, stopping at "
                                    + std::to_string(solution.size()) + ".\n\n";
        }
        *mSolutionsDisplay << maximumOutputExceeded;
        for (const RankedSet<std::string>& cov : solution) {
            *mSolutionsDisplay << cov.rank() << " | ";
            for (const std::string& type : cov) {
                *mSolutionsDisplay << type << " | ";
            }
            *mSolutionsDisplay << "\n";
        }
        *mSolutionsDisplay << maximumOutputExceeded << std::endl;
    }

    void PokemonGUI::solveDefense(const CoverageRequested& exactOrOverlapping) {
        mAllCoverages.reset();
        mSolutionsDisplay->clearDisplay();
        mSolutionsDisplay->flush();
        controls->setEnabled(false);
        gymControls->setEnabled(false);
        mProblems->setEnabled(false);


        std::set<std::string>* attackTypes = &mAllGenerationAttackTypes;
        std::set<std::string> gymAttackTypes = {};
        if (!mSelected.empty()) {
            mUserSelection = SELECTED_GYMS;
            gymAttackTypes = loadSelectedGymsAttacks(mProblems->getSelectedItem(), mSelected);
            attackTypes = &gymAttackTypes;
        } else {
            mUserSelection = FULL_GENERATION;
        }

        // If gymAttackTypes is empty the constructor just builds the full generation of pokemon.
        PokemonLinks dlx(mGeneration.typeInteractions, *attackTypes);

        printDefenseMessage(*attackTypes, dlx.numOptions());

        std::set<RankedSet<std::string>> solution = {};

        if (exactOrOverlapping == EXACT) {
            solution = dlx.getExactTypeCoverages(POKEMON_TEAM_SIZE);
        } else {
            solution = dlx.getOverlappingTypeCoverages(POKEMON_TEAM_SIZE);
        }

        mAllCoverages.reset(new std::set<RankedSet<std::string>>(solution));

        printDefenseSolution(dlx.reachedOutputLimit(), *mAllCoverages);


        /* Enable controls. */
        controls->setEnabled(true);
        mProblems->setEnabled(true);
        gymControls->setEnabled(true);
        mSolutionsDisplay->scrollToTop();

        requestRepaint();
    }

    void PokemonGUI::solveAttack(const CoverageRequested& req) {
        mAllCoverages.reset();
        mSolutionsDisplay->clearDisplay();
        mSolutionsDisplay->flush();
        controls->setEnabled(false);
        mProblems->setEnabled(false);
        gymControls->setEnabled(false);


        /* We are not sure if the user wants solution for map or selected gyms yet. We will point
         * to whatever they have asked for instead of preemptively creating copies of large maps.
         */
        std::map<std::string,std::set<Resistance>>* genToUse = &mGeneration.typeInteractions;
        std::map<std::string,std::set<Resistance>> modifiedGeneration = {};

        if (!mSelected.empty()) {
            mUserSelection = SELECTED_GYMS;
            modifiedGeneration = loadSelectedGymsDefense(mGeneration.typeInteractions,
                                                         mProblems->getSelectedItem(),
                                                         mSelected);
            genToUse = &modifiedGeneration;
        } else {
            mUserSelection = FULL_GENERATION;
        }

        std::set<RankedSet<std::string>> solution = {};
        PokemonLinks dlx(*genToUse, PokemonLinks::ATTACK);

        printAttackMessage(*genToUse, dlx.numOptions());

        req == EXACT ? solution = dlx.getExactTypeCoverages(POKEMON_TEAM_ATTACK_SLOTS) :
                       solution = dlx.getOverlappingTypeCoverages(POKEMON_TEAM_ATTACK_SLOTS);


        mAllCoverages.reset(new std::set<RankedSet<std::string>>(solution));

        printAttackSolution(dlx.reachedOutputLimit(), *mAllCoverages);

        /* Enable controls. */
        controls->setEnabled(true);
        mProblems->setEnabled(true);
        gymControls->setEnabled(true);
        mSolutionsDisplay->scrollToTop();

        requestRepaint();
    }

}

GRAPHICS_HANDLER("Pokemon Planning", GWindow& window) {
    return std::make_shared<PokemonGUI>(window);
}
