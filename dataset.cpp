#include <iostream>
#include <fstream>
#include <sstream>

#include "dataset.hpp"
#include "patient_list.hpp"

const int GROUP_UPPER[GROUP_COUNT] = { 17, 25, 45, 60, 100 };

const std::string GROUP_LABEL[GROUP_COUNT] = {
    "0-17 Pediatrics",
    "18-25 Young Adult",
    "26-45 Working Early",
    "46-60 Working Late",
    "61-100 Senior"
};

const std::string GROUP_FULL[GROUP_COUNT] = {
    "Pediatrics & Adolescents",
    "Young Adults / University Students",
    "Working Adults (Early Career)",
    "Working Adults (Late Career)",
    "Senior Citizens / Geriatric Care"
};

CareTypeRegistry g_careTypes;

int ageGroupIndex(int age) {
    for (int g = 0; g < GROUP_COUNT - 1; ++g) {
        if (age <= GROUP_UPPER[g]) return g;
    }
    return GROUP_COUNT - 1;
}

double computeMedicalCost(const Patient& p) {
    return p.lengthOfStay * p.baseCostPerHour * p.daysVisitsPerYear;
}

void categoriseAndBill(Patient& p) {
    p.ageGroup    = ageGroupIndex(p.age);
    p.medicalCost = computeMedicalCost(p);
}

int CareTypeRegistry::indexOf(const std::string& name) {
    for (int i = 0; i < count_; ++i) {
        if (names_[i] == name) return i;
    }
    if (count_ >= MAX_CARE_TYPES) return -1;
    names_[count_] = name;
    return count_++;
}

void trim(std::string& s) {
    size_t b = 0;
    size_t e = s.size();
    while (b < e && (s[b] == ' ' || s[b] == '\t')) ++b;
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' ||
                     s[e - 1] == '\r' || s[e - 1] == '\n')) --e;
    s = s.substr(b, e - b);
}

bool splitCSV(const std::string& line, std::string out[], int expected) {
    std::stringstream ss(line);
    std::string field;
    int i = 0;

    while (i < expected && std::getline(ss, field, ',')) {
        trim(field);
        out[i++] = field;
    }
    if (i != expected) return false;
    return !std::getline(ss, field);
}

int loadCSV(const std::string& path, PatientList& list, int& skippedRows) {
    std::ifstream file(path.c_str());
    if (!file.is_open()) {
        std::cerr << "ERROR: cannot open \"" << path << "\"\n";
        return -1;
    }

    const int COLS = 6;
    std::string line;
    std::string field[COLS];
    int loaded = 0;
    skippedRows = 0;

    if (!std::getline(file, line)) {
        std::cerr << "ERROR: \"" << path << "\" is empty\n";
        return -1;
    }

    while (std::getline(file, line)) {
        trim(line);
        if (line.empty()) continue;

        if (!splitCSV(line, field, COLS)) {
            ++skippedRows;
            continue;
        }

        Patient p;
        p.patientID = field[0];
        p.careType  = field[2];
        try {
            p.age               = std::stoi(field[1]);
            p.lengthOfStay      = std::stoi(field[3]);
            p.baseCostPerHour   = std::stod(field[4]);
            p.daysVisitsPerYear = std::stoi(field[5]);
        } catch (...) {
            ++skippedRows;
            continue;
        }

        if (p.patientID.empty() || p.age < 0 || p.lengthOfStay <= 0 ||
            p.baseCostPerHour <= 0.0 || p.daysVisitsPerYear <= 0) {
            ++skippedRows;
            continue;
        }

        g_careTypes.indexOf(p.careType);
        categoriseAndBill(p);
        list.append(p);
        ++loaded;
    }

    file.close();
    return loaded;
}
