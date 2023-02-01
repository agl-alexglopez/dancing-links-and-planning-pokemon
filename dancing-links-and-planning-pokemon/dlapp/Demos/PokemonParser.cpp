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
#include <exception>
#include <map>
#include <set>
#include <iostream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

namespace {

const int GEN_1 = 1;
const int GEN_2 = 2;
const int GEN_3 = 3;
const int GEN_4 = 4;
const int GEN_5 = 5;
const int GEN_6 = 6;
const int GEN_7 = 7;
const int GEN_8 = 8;
const int GEN_9 = 9;
const int MAX_GEN_COMMENT_LEN = 4;
const QString JSON_ALL_MAPS_FILE = "res/json/all-maps.json";
const QString GYM_ATTACKS_KEY = "attack";
const QString GYM_DEFENSE_KEY = "defense";

const std::vector<QString> GENERATION_JSON_FILES = {
    "",
    "res/json/gen-1-types.json",
    "res/json/gen-2-types.json",
    "res/json/gen-3-types.json",
    "res/json/gen-4-types.json",
    "res/json/gen-5-types.json",
    "res/json/gen-6-types.json",
    "res/json/gen-7-types.json",
    "res/json/gen-8-types.json",
    "res/json/gen-9-types.json",
};

// Might as well use QStrings if I am parsing with them in the first place.
const std::map<QString, Resistance::Multiplier> DAMAGE_MULTIPLIERS = {
    {"immune", Resistance::IMMUNE},
    {"quarter", Resistance::FRAC14},
    {"half", Resistance::FRAC12},
    {"normal", Resistance::NORMAL},
    {"double", Resistance::DOUBLE},
    {"quad", Resistance::QUADRU},
};

void printGenerationError(const std::exception& ex) {
    std::cerr << "Found this: " << ex.what();
    std::cerr << "Could not choose the correct generation from first line of file.\n";
    std::cerr << "Comment first line as follows. Any other comment can start on the next line\n";
    std::cerr << "# 1\n";
    std::cerr << "# Above, I want to load in this map as Generation One. Choose 1-9" << std::endl;
}

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

void setResistances(std::map<std::string,std::set<Resistance>>& result,
                     const std::string& newType,
                     const QJsonObject& multipliers) {
    for (const QString& multiplier : multipliers.keys()) {
        Resistance::Multiplier multiplierTag = DAMAGE_MULTIPLIERS.at(multiplier);
        QJsonArray  typesInMultiplier = multipliers[multiplier].toArray();
        for (QJsonValueConstRef t : typesInMultiplier) {
            std::string resistanceType = QString(t.toString()).toStdString();
            result[newType].insert({resistanceType, multiplierTag});
        }
    }
}

std::map<std::string,std::set<Resistance>> fromJsonToMap(int generation) {
    QJsonObject jsonTypes;
    getQJsonObject(jsonTypes, GENERATION_JSON_FILES[generation]);
    std::map<std::string,std::set<Resistance>> result = {};
    for (const QString& type : jsonTypes.keys()) {
        std::string newType = type.toStdString();
        result.insert({newType, {}});
        setResistances(result, newType, jsonTypes[type].toObject());
    }
    return result;
}

std::map<std::string,std::set<Resistance>> loadGenerationFromJson(std::istream& source) {
    std::string line;
    std::getline(source, line);
    std::string afterHashtag = line.substr(1, line.length() - 1);
    try {
        int generation = std::stoi(afterHashtag);
        return fromJsonToMap(generation);
    } catch (const std::out_of_range &oor) {
        printGenerationError(oor);
        std::abort();
    } catch (const std::invalid_argument &ia) {
        printGenerationError(ia);
        std::abort();
    }
}


} // namespace


PokemonTest loadPokemonGeneration(std::istream& source) {
    PokemonTest generation;
    generation.interactions = loadGenerationFromJson(source);
    generation.genMap = loadDisaster(source);
    return generation;
}

std::set<std::string> loadSelectedGymsDefenses(const std::string& selectedMap,
                                                 const std::set<std::string>& selectedGyms) {
    QJsonObject mapData;
    getQJsonObject(mapData, JSON_ALL_MAPS_FILE);
    std::set<std::string> result = {};

    QString map = QString::fromStdString(selectedMap);
    QJsonObject gymKeys = mapData[map].toObject();

    for (const QString& gym : gymKeys.keys()) {
        if (selectedGyms.count(gym.toStdString())) {
            QJsonArray gymDefenseTypes = gymKeys[gym][GYM_DEFENSE_KEY].toArray();

            for (const QJsonValueConstRef& type : gymDefenseTypes) {
                std::string stdVersion = QString(type.toString()).toStdString();
                result.insert(stdVersion);
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
