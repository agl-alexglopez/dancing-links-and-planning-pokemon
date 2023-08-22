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
#include <array>
#include <functional>
#include <exception>
#include <map>
#include <set>
#include <string_view>
#include <iostream>
#include <utility>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

namespace {


constexpr int GEN_1 = 1;
constexpr int GEN_2 = 2;
constexpr int GEN_3 = 3;
constexpr int GEN_4 = 4;
constexpr int GEN_5 = 5;
constexpr int GEN_6 = 6;
constexpr int GEN_7 = 7;
constexpr int GEN_8 = 8;
constexpr int GEN_9 = 9;
constexpr int MAX_GEN_COMMENT_LEN = 4;
constexpr std::string_view JSON_ALL_MAPS_FILE = "Data/json/all-maps.json";
constexpr std::string_view GYM_ATTACKS_KEY = "attack";
constexpr std::string_view GYM_DEFENSE_KEY = "defense";

// There is no 0th generation so we will make it easier to select the right file by leaving 0 "".
constexpr std::array<std::string_view,10> GENERATION_JSON_FILES = {
    "",
    "Data/json/gen-1-types.json",
    "Data/json/gen-2-types.json",
    "Data/json/gen-3-types.json",
    "Data/json/gen-4-types.json",
    "Data/json/gen-5-types.json",
    "Data/json/gen-6-types.json",
    "Data/json/gen-7-types.json",
    "Data/json/gen-8-types.json",
    "Data/json/gen-9-types.json",
};

// Might as well use QStrings if I am parsing with them in the first place.
const std::array<std::pair<std::string_view, Dx::Multiplier>,6> DAMAGE_MULTIPLIERS = { {
    {"immune", Dx::IMMUNE},
    {"quarter", Dx::FRAC14},
    {"half", Dx::FRAC12},
    {"normal", Dx::NORMAL},
    {"double", Dx::DOUBLE},
    {"quad", Dx::QUADRU},
} };

const Dx::Multiplier& get_multiplier( const std::string& key ) {
    for ( const auto& mult : DAMAGE_MULTIPLIERS ) {
        if ( mult.first == key ) {
            return mult.second;
        }
    }
    throw std::logic_error( "Out of bounds. Key not found. ");
}

void printGenerationError(const std::exception& ex) {
    std::cerr << "Found this: " << ex.what();
    std::cerr << "Could not choose the correct generation from first line of file.\n";
    std::cerr << "Comment first line as follows. Any other comment can start on the next line\n";
    std::cerr << "# 1\n";
    std::cerr << "# Above, I want to load in this map as Generation One. Choose 1-9" << std::endl;
}

void getQJsonObject(QJsonObject& jsonObj, std::string_view pathToJson) {
    QFile jsonFile(QString::fromStdString( std::string( pathToJson ) ) );
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

void setResistances(std::map<Dx::TypeEncoding,std::set<Dx::Resistance>>& result, const Dx::TypeEncoding& newType,
                     const QJsonObject& multipliers) {
    for (const QString& multiplier : multipliers.keys()) {
        Dx::Multiplier multiplierTag = get_multiplier( multiplier.toStdString() );
        QJsonArray  typesInMultiplier = multipliers[multiplier].toArray();
        for (QJsonValueConstRef t : typesInMultiplier) {
            const std::string& resistanceType = QString(t.toString()).toStdString();
            result[newType].insert({Dx::TypeEncoding(resistanceType), multiplierTag});
        }
    }
}

std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> fromJsonToMap(int generation) {
    QJsonObject jsonTypes;
    getQJsonObject(jsonTypes, GENERATION_JSON_FILES.at( generation ));
    std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> result = {};
    for (const QString& type : jsonTypes.keys()) {
        const std::string& newType = type.toStdString();
        Dx::TypeEncoding encoded(newType);
        result.insert({encoded, {}});
        setResistances(result, encoded, jsonTypes[type].toObject());
    }
    return result;
}

std::map<Dx::TypeEncoding,std::set<Dx::Resistance>> loadGenerationFromJson(std::istream& source) {
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

std::set<Dx::TypeEncoding>
loadSelectedGymsDefenses(const std::string& selectedMap, const std::set<std::string>& selectedGyms) {
    QJsonObject mapData;
    getQJsonObject(mapData, JSON_ALL_MAPS_FILE);
    std::set<Dx::TypeEncoding> result = {};

    QString map = QString::fromStdString(selectedMap);
    QJsonObject gymKeys = mapData[map].toObject();

    for (const QString& gym : gymKeys.keys()) {
        if (!selectedGyms.count(gym.toStdString())) {
            continue;
        }
        QJsonArray gymDefenseTypes
                = gymKeys[gym][QString::fromStdString( std::string( GYM_DEFENSE_KEY ) )].toArray();
        for (const QJsonValueConstRef& type : gymDefenseTypes) {
            const std::string& stdVersion = QString(type.toString()).toStdString();
            result.insert(Dx::TypeEncoding(stdVersion));
        }
    }
    // This will be a much smaller map.
    return result;
}

std::set<Dx::TypeEncoding>
loadSelectedGymsAttacks(const std::string& selectedMap, const std::set<std::string>& selectedGyms) {
    QJsonObject mapData;
    getQJsonObject(mapData, JSON_ALL_MAPS_FILE);
    std::set<Dx::TypeEncoding> result = {};
    QString map = QString::fromStdString(selectedMap);
    QJsonObject gymKeys = mapData[map].toObject();

    for (const QString& gym : gymKeys.keys()) {
        if (!selectedGyms.count(gym.toStdString())) {
            continue;
        }
        QJsonArray gymAttackTypes
                = gymKeys[gym][QString::fromStdString( std::string( GYM_ATTACKS_KEY ) )].toArray();
        for (const QJsonValueConstRef& type : gymAttackTypes) {
            const std::string& stdVersion = QString(type.toString()).toStdString();
            result.insert(Dx::TypeEncoding(stdVersion));
        }
    }
    // Return a simple set rather than altering every type's resistances in a large map.
    return result;
}
