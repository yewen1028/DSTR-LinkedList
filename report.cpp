#include <iostream>
#include <iomanip>
#include <sstream>

#include "report.hpp"

using namespace std;

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

void rule(int width) {
    cout << string(static_cast<size_t>(width), '-') << "\n";
}

void printLegend() {
    cout << "\nAge group legend\n";
    rule(60);
    for (int g = 0; g < GROUP_COUNT; ++g) {
        cout << left << setw(22) << GROUP_LABEL[g]
                  << GROUP_FULL[g] << "\n";
    }
}

void printRowHeader() {
    cout << left
              << setw(10) << "PatientID"
              << setw(5)  << "Age"
              << setw(21) << "AgeGroup"
              << setw(18) << "CareType"
              << right
              << setw(7)  << "Hours"
              << setw(9)  << "Rate"
              << setw(8)  << "Visits"
              << setw(14) << "Cost(RM)" << "\n";
    rule(92);
}

void printPatientRow(const Patient& p) {
    cout << left
              << setw(10) << p.patientID
              << setw(5)  << p.age
              << setw(21) << GROUP_LABEL[p.ageGroup]
              << setw(18) << p.careType
              << right << fixed << setprecision(2)
              << setw(7)  << p.lengthOfStay
              << setw(9)  << p.baseCostPerHour
              << setw(8)  << p.daysVisitsPerYear
              << setw(14) << p.medicalCost << "\n";
}

void printSampleRows(const PatientList& list, int n) {
    printRowHeader();

    int i = 0;
    for (Node* cur = list.head(); cur != nullptr && i < n; cur = cur->next, ++i) {
        printPatientRow(cur->data);
    }
    if (list.size() > n) {
        cout << "... (" << (list.size() - n) << " more records)\n";
    }
}

void printAgeGroupTable(const Analysis& a, const string& title) {
    cout << "\n" << title << "  -  by age group   (" << a.patients
              << " patients)\n";
    cout << left  << setw(21) << "AgeGroup"
              << right << setw(9)  << "Patients"
              << setw(8)  << "Share"
              << setw(8)  << "AvgAge"
              << "  " << left << setw(24) << "TopCareType"
              << right << setw(9)  << "AvgStay"
              << setw(16) << "TotalCost(RM)"
              << setw(14) << "AvgCost(RM)" << "\n";
    rule(109);

    for (int g = 0; g < GROUP_COUNT; ++g) {
        const int n = a.groupPatients[g];
        const int top = a.topCareType(g);

        string topText = "-";
        if (top >= 0) {
            stringstream ss;
            ss << g_careTypes.name(top) << " (" << a.groupCare[g][top] << ")";
            topText = ss.str();
        }

        cout << left  << setw(21) << GROUP_LABEL[g]
                  << right << setw(9) << n
                  << fixed << setprecision(1)
                  << setw(7) << (a.patients > 0 ? 100.0 * n / a.patients : 0.0) << "%"
                  << setw(8) << (n > 0 ? (double)a.groupAge[g] / n : 0.0)
                  << "  " << left << setw(24) << topText
                  << right << setprecision(1)
                  << setw(9) << (n > 0 ? (double)a.groupStay[g] / n : 0.0)
                  << setprecision(2)
                  << setw(16) << a.groupCost[g]
                  << setw(14) << a.avgGroupCost(g) << "\n";
    }
    rule(109);
    cout << left << setw(21) << "TOTAL"
              << right << setw(9) << a.patients
              << fixed << setprecision(2)
              << setw(67) << a.totalCost
              << setw(14) << (a.patients > 0 ? a.totalCost / a.patients : 0.0)
              << "\n";
}

void printCareTypeTable(const Analysis& a, const string& title) {
    cout << "\n" << title << "  -  by care type\n";
    cout << left  << setw(18) << "CareType"
              << right << setw(9)  << "Patients"
              << setw(9)  << "AvgStay"
              << setw(16) << "TotalCost(RM)"
              << setw(14) << "AvgCost(RM)"
              << setw(11) << "ShareCost" << "\n";
    rule(77);

    for (int c = 0; c < g_careTypes.count(); ++c) {
        if (a.carePatients[c] == 0) continue;
        const int n = a.carePatients[c];
        cout << left  << setw(18) << g_careTypes.name(c)
                  << right << setw(9) << n
                  << fixed << setprecision(1)
                  << setw(9) << (double)a.careStay[c] / n
                  << setprecision(2)
                  << setw(16) << a.careCost[c]
                  << setw(14) << a.careCost[c] / n
                  << setprecision(1)
                  << setw(10) << (a.totalCost > 0.0 ? 100.0 * a.careCost[c] / a.totalCost : 0.0)
                  << "%" << "\n";
    }
    rule(77);
    cout << left << setw(18) << "TOTAL"
              << right << setw(9) << a.patients
              << fixed << setprecision(1)
              << setw(9) << (a.patients > 0 ? (double)a.totalStay / a.patients : 0.0)
              << setprecision(2)
              << setw(16) << a.totalCost
              << setw(14) << (a.patients > 0 ? a.totalCost / a.patients : 0.0)
              << setprecision(1) << setw(10) << 100.0 << "%\n";
}

void printDatasetComparison(const Analysis a[], const string labels[],
                            int count, const Analysis& combined) {
    cout << "\nDataset comparison  -  expenditure and visit duration\n";
    cout << left  << setw(26) << "Dataset"
              << right << setw(9)  << "Patients"
              << setw(8)  << "AvgAge"
              << setw(10) << "AvgStay"
              << setw(11) << "AvgVisits"
              << setw(16) << "TotalCost(RM)"
              << setw(14) << "AvgCost(RM)" << "\n";
    rule(94);

    for (int i = 0; i < count; ++i) {
        const Analysis& d = a[i];
        cout << left  << setw(26) << labels[i]
                  << right << setw(9) << d.patients
                  << fixed << setprecision(1)
                  << setw(8)  << (d.patients > 0 ? (double)d.totalAge / d.patients : 0.0)
                  << setw(10) << (d.patients > 0 ? (double)d.totalStay / d.patients : 0.0)
                  << setw(11) << (d.patients > 0 ? (double)d.totalVisits / d.patients : 0.0)
                  << setprecision(2)
                  << setw(16) << d.totalCost
                  << setw(14) << (d.patients > 0 ? d.totalCost / d.patients : 0.0) << "\n";
    }
    rule(94);
    cout << left  << setw(26) << "ALL DATASETS"
              << right << setw(9) << combined.patients
              << fixed << setprecision(1)
              << setw(8)  << (double)combined.totalAge / combined.patients
              << setw(10) << (double)combined.totalStay / combined.patients
              << setw(11) << (double)combined.totalVisits / combined.patients
              << setprecision(2)
              << setw(16) << combined.totalCost
              << setw(14) << combined.totalCost / combined.patients << "\n";
}

void printCrossMatrix(const Analysis a[], const string labels[], int count,
                      const Analysis& combined, bool costMode) {
    cout << "\n" << (costMode ? "Total medical cost (RM) by age group x dataset"
                                   : "Average length of stay (hours) by age group x dataset")
              << "\n";
    cout << left << setw(21) << "AgeGroup";
    for (int i = 0; i < count; ++i) cout << right << setw(16) << labels[i];
    cout << right << setw(16) << "ALL" << "\n";
    rule(21 + 16 * (count + 1));

    for (int g = 0; g < GROUP_COUNT; ++g) {
        cout << left << setw(21) << GROUP_LABEL[g] << right;
        for (int i = 0; i < count; ++i) {
            const int n = a[i].groupPatients[g];
            if (costMode) {
                cout << fixed << setprecision(2)
                          << setw(16) << a[i].groupCost[g];
            } else if (n > 0) {
                cout << fixed << setprecision(1)
                          << setw(16) << (double)a[i].groupStay[g] / n;
            } else {
                cout << setw(16) << "-";
            }
        }
        const int cn = combined.groupPatients[g];
        if (costMode) {
            cout << fixed << setprecision(2)
                      << setw(16) << combined.groupCost[g] << "\n";
        } else if (cn > 0) {
            cout << fixed << setprecision(1)
                      << setw(16) << (double)combined.groupStay[g] / cn << "\n";
        } else {
            cout << setw(16) << "-" << "\n";
        }
    }
    rule(21 + 16 * (count + 1));

    cout << left << setw(21) << (costMode ? "TOTAL" : "OVERALL AVG")
              << right;
    for (int i = 0; i < count; ++i) {
        if (costMode) {
            cout << fixed << setprecision(2) << setw(16) << a[i].totalCost;
        } else {
            cout << fixed << setprecision(1)
                      << setw(16) << (a[i].patients > 0 ? (double)a[i].totalStay / a[i].patients : 0.0);
        }
    }
    if (costMode) {
        cout << fixed << setprecision(2) << setw(16) << combined.totalCost << "\n";
    } else {
        cout << fixed << setprecision(1)
                  << setw(16) << (double)combined.totalStay / combined.patients << "\n";
    }
}
