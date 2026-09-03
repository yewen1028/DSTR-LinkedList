// ============================================================================
//  report.hpp - the single-pass analysis gather (requirements 4b, 4c, 5a, 5b,
//               5c) and the text-based console tables (requirement 5d)
// ============================================================================
#ifndef REPORT_HPP
#define REPORT_HPP

#include <string>

#include "dataset.hpp"
#include "patient_list.hpp"

// ----------------------------------------------------------------------------
//  Analysis
// ----------------------------------------------------------------------------

// Every figure the reports need, collected in ONE traversal of the list.
struct Analysis {
    // Per age group (requirement 4b / 4c)
    int    groupPatients[GROUP_COUNT];
    double groupCost[GROUP_COUNT];
    long   groupStay[GROUP_COUNT];      // summed hours
    long   groupVisits[GROUP_COUNT];
    long   groupAge[GROUP_COUNT];
    int    groupCare[GROUP_COUNT][MAX_CARE_TYPES];  // care-type tally per group

    // Per care type (requirement 5b)
    int    carePatients[MAX_CARE_TYPES];
    double careCost[MAX_CARE_TYPES];
    long   careStay[MAX_CARE_TYPES];

    // Whole dataset (requirement 5a)
    int    patients;
    double totalCost;
    long   totalStay;
    long   totalVisits;
    long   totalAge;

    Analysis() { reset(); }

    void reset();

    // Requirement 4c - average medical cost per patient in an age group.
    double avgGroupCost(int g) const {
        return groupPatients[g] > 0 ? groupCost[g] / groupPatients[g] : 0.0;
    }

    // Requirement 4b - most preferred / requested care type in an age group.
    // Ties are resolved by registration order, which is deterministic across
    // runs because the datasets are read in a fixed sequence.
    int topCareType(int g) const;
};

// Single O(n) traversal of the linked list.
void analyse(const PatientList& list, Analysis& out);

// Comparators for PatientList::sort (requirement 3c).
bool byCostDesc(const Patient& a, const Patient& b);
bool byAgeAsc  (const Patient& a, const Patient& b);

// ----------------------------------------------------------------------------
//  Console tables (requirement 5d)
// ----------------------------------------------------------------------------

void rule(int width);

void printLegend();

void printSampleRows(const PatientList& list, int n);

// Requirements 4b + 4c: per age group, patient count, most preferred care
// type, total medical cost and average cost per patient.
void printAgeGroupTable(const Analysis& a, const std::string& title);

// Requirement 5b: total medical cost grouped by care type.
void printCareTypeTable(const Analysis& a, const std::string& title);

// Requirement 5a + 5c: dataset-level totals side by side.
void printDatasetComparison(const Analysis a[], const std::string labels[],
                            int count, const Analysis& combined);

// Requirement 5c: age group x dataset matrix. Called twice - once for money,
// once for time - so both dimensions of the comparison are visible.
// `labels` here are the SHORT dataset names - the full ones do not fit a
// 16-character numeric column.
void printCrossMatrix(const Analysis a[], const std::string labels[], int count,
                      const Analysis& combined, bool costMode);

#endif  // REPORT_HPP
