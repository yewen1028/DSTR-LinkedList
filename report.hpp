#ifndef REPORT_HPP
#define REPORT_HPP

#include <string>

#include "dataset.hpp"
#include "patient_list.hpp"

struct Analysis {
    int    groupPatients[GROUP_COUNT];
    double groupCost[GROUP_COUNT];
    long   groupStay[GROUP_COUNT];
    long   groupVisits[GROUP_COUNT];
    long   groupAge[GROUP_COUNT];
    int    groupCare[GROUP_COUNT][MAX_CARE_TYPES];

    int    carePatients[MAX_CARE_TYPES];
    double careCost[MAX_CARE_TYPES];
    long   careStay[MAX_CARE_TYPES];

    int    patients;
    double totalCost;
    long   totalStay;
    long   totalVisits;
    long   totalAge;

    Analysis() { reset(); }

    void reset();

    double avgGroupCost(int g) const {
        return groupPatients[g] > 0 ? groupCost[g] / groupPatients[g] : 0.0;
    }

    int topCareType(int g) const;
};

void analyse(const PatientList& list, Analysis& out);

bool byCostDesc(const Patient& a, const Patient& b);
bool byAgeAsc  (const Patient& a, const Patient& b);

void rule(int width);

void printLegend();

void printRowHeader();
void printPatientRow(const Patient& p);

void printSampleRows(const PatientList& list, int n);

void printAgeGroupTable(const Analysis& a, const std::string& title);

void printCareTypeTable(const Analysis& a, const std::string& title);

void printDatasetComparison(const Analysis a[], const std::string labels[],
                            int count, const Analysis& combined);

void printCrossMatrix(const Analysis a[], const std::string labels[], int count,
                      const Analysis& combined, bool costMode);

#endif
