// ============================================================================
//  dataset.hpp - the Patient record, the business rules behind requirements
//                4a/4b/4c, the care-type registry, and CSV loading (3a/3b)
// ============================================================================
#ifndef DATASET_HPP
#define DATASET_HPP

#include <string>

class PatientList;   // forward declaration - loadCSV only needs a reference

// ----------------------------------------------------------------------------
//  The record
// ----------------------------------------------------------------------------

// One row of a facility CSV.
// Columns: PatientID,Age,CareType,LengthOfStay,BaseCostPerHour,DaysVisitsPerYear
struct Patient {
    std::string patientID;         // e.g. "PT1001"
    int         age;               // years
    std::string careType;          // Emergency / Inpatient / Outpatient / ...
    int         lengthOfStay;      // hours per visit
    double      baseCostPerHour;   // RM per hour, set by the facility
    int         daysVisitsPerYear; // number of visits in a year

    // Derived fields - filled once at load time by categoriseAndBill().
    int    ageGroup;               // index 0..4, see GROUP_LABEL below
    double medicalCost;            // LOS x BaseCostPerHour x DaysVisitsPerYear

    Patient()
        : age(0), lengthOfStay(0), baseCostPerHour(0.0), daysVisitsPerYear(0),
          ageGroup(0), medicalCost(0.0) {}
};

// ----------------------------------------------------------------------------
//  Age groups (requirement 4a)
// ----------------------------------------------------------------------------
const int GROUP_COUNT = 5;

// Upper bound of each band; the last band absorbs anything above 100 as well,
// so no record can fall outside the classification.
extern const int GROUP_UPPER[GROUP_COUNT];

// Short labels for table columns.
extern const std::string GROUP_LABEL[GROUP_COUNT];

// Full names as written in the specification, printed once as a legend.
extern const std::string GROUP_FULL[GROUP_COUNT];

// Returns 0..4, matching GROUP_LABEL / GROUP_FULL.
int ageGroupIndex(int age);

// ----------------------------------------------------------------------------
//  Cost model
//      Cost = LengthOfStay x BaseCostPerHour x DaysVisitsPerYear
// ----------------------------------------------------------------------------
double computeMedicalCost(const Patient& p);

// Fills the derived fields of one record.
void categoriseAndBill(Patient& p);

// ----------------------------------------------------------------------------
//  Care-type registry.
//
//  Care types are not known until the files are read, and the three datasets
//  do not offer the same ones. This registry assigns every distinct care type
//  a stable index the moment it is first seen, so all datasets can be tallied
//  and compared against the same column order. Backed by a fixed-size table
//  because the number of care types is small and bounded.
// ----------------------------------------------------------------------------
const int MAX_CARE_TYPES = 16;

class CareTypeRegistry {
public:
    CareTypeRegistry() : count_(0) {}

    // Returns the index of `name`, registering it on first sight.
    // Returns -1 only if the table is full.
    int indexOf(const std::string& name);

    const std::string& name(int i) const { return names_[i]; }
    int count() const { return count_; }

private:
    std::string names_[MAX_CARE_TYPES];
    int         count_;
};

// One registry shared by every dataset keeps the care-type columns aligned.
// Defined in dataset.cpp.
extern CareTypeRegistry g_careTypes;

// ----------------------------------------------------------------------------
//  CSV loading (requirement 3a / 3b)
// ----------------------------------------------------------------------------

// Removes a trailing '\r' left behind by CRLF files, plus surrounding spaces.
void trim(std::string& s);

// Splits one CSV line into exactly `expected` fields.
// Returns false when the column count does not match (malformed row).
bool splitCSV(const std::string& line, std::string out[], int expected);

// Loads one facility CSV into `list`. Existing contents are kept (append).
// Returns the number of records loaded; -1 when the file cannot be opened.
int loadCSV(const std::string& path, PatientList& list, int& skippedRows);

#endif  // DATASET_HPP
