#ifndef DATASET_HPP
#define DATASET_HPP

#include <string>

class PatientList;

struct Patient {
    std::string patientID;
    int         age;
    std::string careType;
    int         lengthOfStay;
    double      baseCostPerHour;
    int         daysVisitsPerYear;

    int    ageGroup;
    double medicalCost;

    Patient()
        : age(0), lengthOfStay(0), baseCostPerHour(0.0), daysVisitsPerYear(0),
          ageGroup(0), medicalCost(0.0) {}
};

const int GROUP_COUNT = 5;

extern const int GROUP_UPPER[GROUP_COUNT];

extern const std::string GROUP_LABEL[GROUP_COUNT];

extern const std::string GROUP_FULL[GROUP_COUNT];

int ageGroupIndex(int age);

double computeMedicalCost(const Patient& p);

void categoriseAndBill(Patient& p);

const int MAX_CARE_TYPES = 16;

class CareTypeRegistry {
public:
    CareTypeRegistry() : count_(0) {}

    int indexOf(const std::string& name);

    const std::string& name(int i) const { return names_[i]; }
    int count() const { return count_; }

private:
    std::string names_[MAX_CARE_TYPES];
    int         count_;
};

extern CareTypeRegistry g_careTypes;

void trim(std::string& s);

bool splitCSV(const std::string& line, std::string out[], int expected);

int loadCSV(const std::string& path, PatientList& list, int& skippedRows);

#endif
