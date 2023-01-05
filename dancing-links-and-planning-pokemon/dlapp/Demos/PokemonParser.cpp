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
 * Author: Alex Lopez
 * File: PokemonParser.cpp
 * -----------------------
 * This file handles getting Pokemon Data from JSON files and turning it into C++ std compliant
 * data forms like maps and sets. This implementation is heavily dependent on QT libraries for
 * parsing the JSON. I chose to turn the data into C++ std forms to use with the program, even
 * though it would have been possible to just read the data from the files in directly during the
 * program runtime, in case QT becomes deprecated or I want to move this to a CLI implementation.
 * Then, I just need to rewrite the parser that gets me data from JSON files rather than rewrite
 * the whole program.
 *
 * To be more thorough I could seperate out each generation to its own .json file and just directly
 * parse the correct generation information to a map. Right now, I am filtering out types from
 * a map that contains the most recent generation typing information if I want an older generation.
 */
#include "PokemonParser.h"
#include "MapParser.h"
#include <functional>
#include <map>
#include <set>
#include <iostream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

namespace {

    const int GEN_ONE = 1;
    const int GEN_2 = 2;
    const int GEN_6 = 6;
    const int GEN_8 = 8;
    const int GEN_9 = 9;
    const int MAX_GEN_COMMENT_LEN = 4;
    const QString JSON_ALL_TYPES_FILE = "res/json/all-types.json";
    const QString JSON_ALL_MAPS_FILE = "res/json/all-maps.json";
    const QString GYM_ATTACKS_KEY = "attack";
    const QString GYM_DEFENSE_KEY = "defense";

    const std::set<std::string> ADDED_GEN_9 = {"Bug-Dark","Fire-Grass","Poison-Steel",
                                               "Electric-Fighting","Normal-Poison","Fighting-Ground",
                                               "Fairy-Fighting","Ghost-Normal","Electric-Rock",
                                               "Electric-Grass","Electric-Psychic"};

    const std::set<std::string> ADDED_GEN_6_TO_8 = {"Fairy","Fighting-Ghost","Electric-Normal",
                                                    "Fire-Normal","Fire-Water","Fighting-Ice",
                                                    "Bug-Ice","Poison-Rock","Ghost-Grass",
                                                    "Fire-Poison","Poison-Psychic",
                                                    "Electric-Poison","Dragon-Grass","Dark-Normal",
                                                    "Fighting-Flying"};

    const std::set<std::string> ADDED_GEN_2_TO_5 = {"Dark","Steel","Dragon-Normal","Dragon-Fire",
                                                    "Dragon-Water","Dragon-Electric",
                                                    "Dragon-Ice","Dragon-Fighting","Dragon-Poison",
                                                    "Dragon-Ground","Dragon-Psychic","Dragon-Rock",
                                                    "Dragon-Ghost","Dark-Dragon","Dragon-Steel",
                                                    "Bug-Rock","Electric-Water","Fighting-Fire",
                                                    "Bug-Electric","Fire-Psychic","Fighting-Grass",
                                                    "Grass-Ice","Grass-Ground","Bug-Ghost",
                                                    "Grass-Normal","Grass-Rock","Fire-Ground",
                                                    "Grass-Water","Fighting-Normal",
                                                    "Electric-Fire","Electric-Ground","Bug-Psychic",
                                                    "Ghost-Psychic","Ice-Rock","Normal-Water",
                                                    "Normal-Psychic","Fire-Ice","Flying-Psychic",
                                                    "Fire-Rock","Bug-Water","Bug-Fighting",
                                                    "Ground-Ice","Ghost-Ice","Ghost-Water",
                                                    "Ghost-Ground","Flying-Grass","Electric-Ghost",
                                                    "Bug-Ground","Flying-Ground","Fighting-Rock",
                                                    "Ground-Psychic","Flying-Ghost","Fire-Ghost",
                                                    "Bug-Fire","Psychic-Rock","Electric-Ice"};
    const std::string DUAL_TYPE_DELIM = "-";

    // Might as well use QStrings if I am parsing with them in the first place.
    const std::map<QString, Resistance::Multiplier> DAMAGE_MULTIPLIERS = {
        {"immune", Resistance::IMMUNE},
        {"quarter", Resistance::FRAC14},
        {"half", Resistance::FRAC12},
        {"normal", Resistance::NORMAL},
        {"double", Resistance::DOUBLE},
        {"quad", Resistance::QUADRU},
    };


    void getQJsonObject(QJsonObject& jsonObj, const QString& pathToJson) {
        QFile jsonFile(pathToJson);
        if (!jsonFile.open(QIODevice::ReadOnly)) {
            std::cerr << "Could not open json file." << std::endl;
            jsonFile.close();
            std::abort();
        }
        QByteArray bytes = jsonFile.readAll();
        jsonFile.close();
        QJsonParseError jsonError;
        QJsonDocument qtJsonDoc = QJsonDocument::fromJson(bytes, &jsonError);
        if (jsonError.error != QJsonParseError::NoError) {
            std::cerr << "Error parsing JSON to QDocument." << std::endl;
            std::abort();
        }
        if (!qtJsonDoc.isObject()) {
            std::cerr << "Error identifying JSON as object at highest level." << std::endl;
            std::abort();
        }
        jsonObj = qtJsonDoc.object();
        if (jsonObj.empty()) {
            std::cerr << "No Data in QJsonObject." << std::endl;
            std::abort();
        }
    }

    // Add more filtering if you ever come back to this after Gen 9. For now this is most recent.
    bool isGenNine(std::string& type) {
        (void) type;
        return true;
    }

    bool isGenOneType(const std::string& type) {
        if (ADDED_GEN_9.count(type) || ADDED_GEN_2_TO_5.count(type)
                || ADDED_GEN_6_TO_8.count(type)) {
            return false;
        }
        std::size_t typeDelim = type.find_first_of(DUAL_TYPE_DELIM);
        if (typeDelim != std::string::npos) {
            std::string firstType = type.substr(0, typeDelim);
            std::string secondType = type.substr(typeDelim + 1);
            if (ADDED_GEN_2_TO_5.count(firstType) || ADDED_GEN_6_TO_8.count(firstType)
                    || ADDED_GEN_2_TO_5.count(secondType) || ADDED_GEN_6_TO_8.count(secondType)) {
                return false;
            }
        }
        return true;
    }

    bool isGenTwoToFive(std::string& type) {
        if (ADDED_GEN_6_TO_8.count(type) || ADDED_GEN_9.count(type)) {
            return false;
        }
        std::size_t typeDelim = type.find_first_of(DUAL_TYPE_DELIM);
        if (typeDelim != std::string::npos) {
            std::string firstType = type.substr(0, typeDelim);
            std::string secondType = type.substr(typeDelim + 1);
            if (ADDED_GEN_6_TO_8.count(firstType) || ADDED_GEN_6_TO_8.count(secondType)
                    || ADDED_GEN_9.count(firstType) || ADDED_GEN_9.count(secondType)) {
                return false;
            }
        }
        return true;
    }

    bool isGenSixToEight(std::string& type) {
        if (ADDED_GEN_9.count(type)) {
            return false;
        }
        std::size_t typeDelim = type.find_first_of(DUAL_TYPE_DELIM);
        if (typeDelim != std::string::npos) {
            std::string firstType = type.substr(0, typeDelim);
            std::string secondType = type.substr(typeDelim + 1);
            if (ADDED_GEN_9.count(firstType) || ADDED_GEN_9.count(secondType)) {
                return false;
            }
        }
        return true;
    }

    void addResistancesForType(std::map<std::string,std::set<Resistance>>& result,
                               const std::string& newType,
                               const QJsonObject& damageMultipliers,
                               const std::function<bool(std::string&)>& filterFunc) {
        for (const QString& mult : damageMultipliers.keys()) {
            Resistance::Multiplier multiplierTag = DAMAGE_MULTIPLIERS.at(mult);
            QJsonArray typesInMultipliers = damageMultipliers[mult].toArray();
            for (const QJsonValueConstRef& t : typesInMultipliers) {
                std::string type = QString(t.toString()).toStdString();
                if (filterFunc(type)) {
                    result[newType].insert({type, multiplierTag});
                }
            }
        }
    }

    std::map<std::string,std::set<Resistance>>
    filterPokemonByGeneration(const std::function<bool(std::string&)>& filterFunc) {
        QJsonObject pokemonData;
        getQJsonObject(pokemonData, JSON_ALL_TYPES_FILE);
        std::map<std::string,std::set<Resistance>> result = {};
        for (const QString& type : pokemonData.keys()) {
            std::string newType = type.toStdString();
            if (filterFunc(newType)) {
                result.insert({newType, {}});
                addResistancesForType(result, newType, pokemonData[type].toObject(), filterFunc);
            }
        }
        return result;
    }

    std::map<std::string,std::set<Resistance>> setTypeInteractions(std::istream& source) {
        // We need to check the first line of the pokemon map file for the generation info.
        std::string line;
        std::getline(source, line);
        std::string afterHashtag = line.substr(1, line.length() - 1);
        int generation = std::stoi(afterHashtag);
        if (line.length() > MAX_GEN_COMMENT_LEN || generation >= GEN_9) {
            return filterPokemonByGeneration(&isGenNine);
        } else if (generation == GEN_ONE) {
            return filterPokemonByGeneration(&isGenOneType);
        } else if (generation < GEN_6) {
            return filterPokemonByGeneration(&isGenTwoToFive);
        } else if (generation <= GEN_8){
            return filterPokemonByGeneration(&isGenSixToEight);
        } else {
            std::cerr << "Could not pick a Pokemon Generation to load." << std::endl;
            return {};
        }
    }
}

PokemonTest loadPokemonGeneration(std::istream& source) {
    PokemonTest generation;
    generation.typeInteractions = setTypeInteractions(source);
    generation.pokemonGenerationMap = loadDisaster(source);
    return generation;
}

std::map<std::string,std::set<Resistance>>
loadSelectedGymsDefense(const std::map<std::string,std::set<Resistance>>& currentGenInteractions,
                        const std::string& selectedMap,
                        const std::set<std::string>& selectedGyms) {
    QJsonObject mapData;
    getQJsonObject(mapData, JSON_ALL_MAPS_FILE);
    std::map<std::string,std::set<Resistance>> result = {};

    QString map = QString::fromStdString(selectedMap);
    QJsonObject gymKeys = mapData[map].toObject();

    for (const QString& gym : gymKeys.keys()) {
        if (selectedGyms.count(gym.toStdString())) {
            QJsonArray gymDefenseTypes = gymKeys[gym][GYM_DEFENSE_KEY].toArray();

            for (const QJsonValueConstRef& type : gymDefenseTypes) {
                std::string stdVersion = QString(type.toString()).toStdString();
                result[stdVersion] = currentGenInteractions.at(stdVersion);
            }
        }
    }
    // This will be a much smaller map.
    return result;
}

std::set<std::string> loadSelectedGymsAttacks(const std::string& selectedMap,
                                              const std::set<std::string>& selectedGyms) {
    QJsonObject mapData;
    getQJsonObject(mapData, JSON_ALL_MAPS_FILE);
    std::set<std::string> result = {};
    QString map = QString::fromStdString(selectedMap);
    QJsonObject gymKeys = mapData[map].toObject();

    for (const QString& gym : gymKeys.keys()) {
        if (selectedGyms.count(gym.toStdString())) {
            QJsonArray gymAttackTypes = gymKeys[gym][GYM_ATTACKS_KEY].toArray();

            for (const QJsonValueConstRef& type : gymAttackTypes) {
                std::string stdVersion = QString(type.toString()).toStdString();
                result.insert(stdVersion);
            }
        }
    }
    // Return a simple set rather than altering every type's resistances in a large map.
    return result;
}
