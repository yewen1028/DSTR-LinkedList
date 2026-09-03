// ============================================================================
//  report.cpp - see report.hpp
// ============================================================================
#include <iostream>
#include <iomanip>
#include <sstream>

#include "report.hpp"

// ----------------------------------------------------------------------------
//  Analysis
// ----------------------------------------------------------------------------

void Analysis::reset() {
    for (int g = 0; g < GROUP_COUNT; ++g) {
        groupPatients[g] = 0;
        groupCost[g]     = 0.0;
        groupStay[g]     = 0;
        groupVisits[g]   = 0;
        groupAge[g]      = 0;
        for (int c = 0; c < MAX_CARE_TYPES; ++c) groupCare[g][c] = 0;
    }
    for (int c = 0; c < MAX_CARE_TYPES; ++c) {
        carePatients[c] = 0;
        careCost[c]     = 0.0;
        careStay[c]     = 0;
    }
    patients = 0;
    totalCost = 0.0;
    totalStay = totalVisits = totalAge = 0;
}

int Analysis::topCareType(int g) const {
    int best = -1;
    for (int c = 0; c < g_careTypes.count(); ++c) {
        if (groupCare[g][c] > 0 &&
            (best < 0 || groupCare[g][c] > groupCare[g][best])) {
            best = c;
        }
    }
    return best;
}

void analyse(const PatientList& list, Analysis& out) {
    out.reset();
    for (Node* cur = list.head(); cur != nullptr; cur = cur->next) {
        const Patient& p = cur->data;
        const int g = p.ageGroup;
        const int c = g_careTypes.indexOf(p.careType);

        out.groupPatients[g] += 1;
        out.groupCost[g]     += p.medicalCost;
        out.groupStay[g]     += p.lengthOfStay;
        out.groupVisits[g]   += p.daysVisitsPerYear;
        out.groupAge[g]      += p.age;
        if (c >= 0) {
            out.groupCare[g][c] += 1;
            out.carePatients[c] += 1;
            out.careCost[c]     += p.medicalCost;
            out.careStay[c]     += p.lengthOfStay;
        }

        out.patients    += 1;
        out.totalCost   += p.medicalCost;
        out.totalStay   += p.lengthOfStay;
        out.totalVisits += p.daysVisitsPerYear;
        out.totalAge    += p.age;
    }
}

bool byCostDesc(const Patient& a, const Patient& b) { return a.medicalCost > b.medicalCost; }
bool byAgeAsc  (const Patient& a, const Patient& b) { return a.age < b.age; }

// ----------------------------------------------------------------------------
//  Console tables
// ----------------------------------------------------------------------------

void rule(int width) {
    std::cout << std::string(static_cast<size_t>(width), '-') << "\n";
}

void printLegend() {
    std::cout << "\nAge group legend\n";
    rule(60);
    for (int g = 0; g < GROUP_COUNT; ++g) {
        std::cout << std::left << std::setw(22) << GROUP_LABEL[g]
                  << GROUP_FULL[g] << "\n";
    }
}

void printSampleRows(const PatientList& list, int n) {
    std::cout << std::left
              << std::setw(10) << "PatientID"
              << std::setw(5)  << "Age"
              << std::setw(21) << "AgeGroup"
              << std::setw(18) << "CareType"
              << std::right
              << std::setw(7)  << "Hours"
              << std::setw(9)  << "Rate"
              << std::setw(8)  << "Visits"
              << std::setw(14) << "Cost(RM)" << "\n";
    rule(92);

    int i = 0;
    for (Node* cur = list.head(); cur != nullptr && i < n; cur = cur->next, ++i) {
        const Patient& p = cur->data;
        std::cout << std::left
                  << std::setw(10) << p.patientID
                  << std::setw(5)  << p.age
                  << std::setw(21) << GROUP_LABEL[p.ageGroup]
                  << std::setw(18) << p.careType
                  << std::right << std::fixed << std::setprecision(2)
                  << std::setw(7)  << p.lengthOfStay
                  << std::setw(9)  << p.baseCostPerHour
                  << std::setw(8)  << p.daysVisitsPerYear
                  << std::setw(14) << p.medicalCost << "\n";
    }
    if (list.size() > n) {
        std::cout << "... (" << (list.size() - n) << " more records)\n";
    }
}

void printAgeGroupTable(const Analysis& a, const std::string& title) {
    std::cout << "\n" << title << "  -  by age group   (" << a.patients
              << " patients)\n";
    std::cout << std::left  << std::setw(21) << "AgeGroup"
              << std::right << std::setw(9)  << "Patients"
              << std::setw(8)  << "Share"
              << std::setw(8)  << "AvgAge"
              << "  " << std::left << std::setw(24) << "TopCareType"
              << std::right << std::setw(9)  << "AvgStay"
              << std::setw(16) << "TotalCost(RM)"
              << std::setw(14) << "AvgCost(RM)" << "\n";
    rule(109);

    for (int g = 0; g < GROUP_COUNT; ++g) {
        const int n = a.groupPatients[g];
        const int top = a.topCareType(g);

        std::string topText = "-";
        if (top >= 0) {
            std::stringstream ss;
            ss << g_careTypes.name(top) << " (" << a.groupCare[g][top] << ")";
            topText = ss.str();
        }

        std::cout << std::left  << std::setw(21) << GROUP_LABEL[g]
                  << std::right << std::setw(9) << n
                  << std::fixed << std::setprecision(1)
                  << std::setw(7) << (a.patients > 0 ? 100.0 * n / a.patients : 0.0) << "%"
                  << std::setw(8) << (n > 0 ? (double)a.groupAge[g] / n : 0.0)
                  << "  " << std::left << std::setw(24) << topText
                  << std::right << std::setprecision(1)
                  << std::setw(9) << (n > 0 ? (double)a.groupStay[g] / n : 0.0)
                  << std::setprecision(2)
                  << std::setw(16) << a.groupCost[g]
                  << std::setw(14) << a.avgGroupCost(g) << "\n";
    }
    rule(109);
    std::cout << std::left << std::setw(21) << "TOTAL"
              << std::right << std::setw(9) << a.patients
              << std::fixed << std::setprecision(2)
              << std::setw(67) << a.totalCost
              << std::setw(14) << (a.patients > 0 ? a.totalCost / a.patients : 0.0)
              << "\n";
}

void printCareTypeTable(const Analysis& a, const std::string& title) {
    std::cout << "\n" << title << "  -  by care type\n";
    std::cout << std::left  << std::setw(18) << "CareType"
              << std::right << std::setw(9)  << "Patients"
              << std::setw(9)  << "AvgStay"
              << std::setw(16) << "TotalCost(RM)"
              << std::setw(14) << "AvgCost(RM)"
              << std::setw(11) << "ShareCost" << "\n";
    rule(77);

    for (int c = 0; c < g_careTypes.count(); ++c) {
        if (a.carePatients[c] == 0) continue;   // not offered by this dataset
        const int n = a.carePatients[c];
        std::cout << std::left  << std::setw(18) << g_careTypes.name(c)
                  << std::right << std::setw(9) << n
                  << std::fixed << std::setprecision(1)
                  << std::setw(9) << (double)a.careStay[c] / n
                  << std::setprecision(2)
                  << std::setw(16) << a.careCost[c]
                  << std::setw(14) << a.careCost[c] / n
                  << std::setprecision(1)
                  << std::setw(10) << (a.totalCost > 0.0 ? 100.0 * a.careCost[c] / a.totalCost : 0.0)
                  << "%" << "\n";
    }
    rule(77);
    std::cout << std::left << std::setw(18) << "TOTAL"
              << std::right << std::setw(9) << a.patients
              << std::fixed << std::setprecision(1)
              << std::setw(9) << (a.patients > 0 ? (double)a.totalStay / a.patients : 0.0)
              << std::setprecision(2)
              << std::setw(16) << a.totalCost
              << std::setw(14) << (a.patients > 0 ? a.totalCost / a.patients : 0.0)
              << std::setprecision(1) << std::setw(10) << 100.0 << "%\n";
}

void printDatasetComparison(const Analysis a[], const std::string labels[],
                            int count, const Analysis& combined) {
    std::cout << "\nDataset comparison  -  expenditure and visit duration\n";
    std::cout << std::left  << std::setw(26) << "Dataset"
              << std::right << std::setw(9)  << "Patients"
              << std::setw(8)  << "AvgAge"
              << std::setw(10) << "AvgStay"
              << std::setw(11) << "AvgVisits"
              << std::setw(16) << "TotalCost(RM)"
              << std::setw(14) << "AvgCost(RM)" << "\n";
    rule(94);

    for (int i = 0; i < count; ++i) {
        const Analysis& d = a[i];
        std::cout << std::left  << std::setw(26) << labels[i]
                  << std::right << std::setw(9) << d.patients
                  << std::fixed << std::setprecision(1)
                  << std::setw(8)  << (d.patients > 0 ? (double)d.totalAge / d.patients : 0.0)
                  << std::setw(10) << (d.patients > 0 ? (double)d.totalStay / d.patients : 0.0)
                  << std::setw(11) << (d.patients > 0 ? (double)d.totalVisits / d.patients : 0.0)
                  << std::setprecision(2)
                  << std::setw(16) << d.totalCost
                  << std::setw(14) << (d.patients > 0 ? d.totalCost / d.patients : 0.0) << "\n";
    }
    rule(94);
    std::cout << std::left  << std::setw(26) << "ALL DATASETS"
              << std::right << std::setw(9) << combined.patients
              << std::fixed << std::setprecision(1)
              << std::setw(8)  << (double)combined.totalAge / combined.patients
              << std::setw(10) << (double)combined.totalStay / combined.patients
              << std::setw(11) << (double)combined.totalVisits / combined.patients
              << std::setprecision(2)
              << std::setw(16) << combined.totalCost
              << std::setw(14) << combined.totalCost / combined.patients << "\n";
}

void printCrossMatrix(const Analysis a[], const std::string labels[], int count,
                      const Analysis& combined, bool costMode) {
    std::cout << "\n" << (costMode ? "Total medical cost (RM) by age group x dataset"
                                   : "Average length of stay (hours) by age group x dataset")
              << "\n";
    std::cout << std::left << std::setw(21) << "AgeGroup";
    for (int i = 0; i < count; ++i) std::cout << std::right << std::setw(16) << labels[i];
    std::cout << std::right << std::setw(16) << "ALL" << "\n";
    rule(21 + 16 * (count + 1));

    for (int g = 0; g < GROUP_COUNT; ++g) {
        std::cout << std::left << std::setw(21) << GROUP_LABEL[g] << std::right;
        for (int i = 0; i < count; ++i) {
            const int n = a[i].groupPatients[g];
            if (costMode) {
                std::cout << std::fixed << std::setprecision(2)
                          << std::setw(16) << a[i].groupCost[g];
            } else if (n > 0) {
                std::cout << std::fixed << std::setprecision(1)
                          << std::setw(16) << (double)a[i].groupStay[g] / n;
            } else {
                std::cout << std::setw(16) << "-";
            }
        }
        const int cn = combined.groupPatients[g];
        if (costMode) {
            std::cout << std::fixed << std::setprecision(2)
                      << std::setw(16) << combined.groupCost[g] << "\n";
        } else if (cn > 0) {
            std::cout << std::fixed << std::setprecision(1)
                      << std::setw(16) << (double)combined.groupStay[g] / cn << "\n";
        } else {
            std::cout << std::setw(16) << "-" << "\n";
        }
    }
    rule(21 + 16 * (count + 1));

    std::cout << std::left << std::setw(21) << (costMode ? "TOTAL" : "OVERALL AVG")
              << std::right;
    for (int i = 0; i < count; ++i) {
        if (costMode) {
            std::cout << std::fixed << std::setprecision(2) << std::setw(16) << a[i].totalCost;
        } else {
            std::cout << std::fixed << std::setprecision(1)
                      << std::setw(16) << (a[i].patients > 0 ? (double)a[i].totalStay / a[i].patients : 0.0);
        }
    }
    if (costMode) {
        std::cout << std::fixed << std::setprecision(2) << std::setw(16) << combined.totalCost << "\n";
    } else {
        std::cout << std::fixed << std::setprecision(1)
                  << std::setw(16) << (double)combined.totalStay / combined.patients << "\n";
    }
}
