// ============================================================================
//  Healthcare Patient Records - SINGLY LINKED LIST IMPLEMENTATION
//  ---------------------------------------------------------------------------
//  Scope of this program (strictly singly linked list, no arrays used for
//  record storage, no STL containers):
//
//    3a/3b  Load Dataset 1, 2 and 3 into singly linked lists
//    3c     Structure supports searching (findByID), sorting (merge sort with
//           a swappable comparator) and analysis traversals
//    4a     Age groups: 0-17, 18-25, 26-45, 46-60, 61-100
//    4b     Per age group: most preferred Care Type + total medical cost
//    4c     Per age group: average medical cost per patient
//    5a     Total medical billing cost per dataset
//    5b     Total medical cost grouped by Care Type
//    5c     Expenditure and visit durations compared across datasets and
//           age groups
//    5d     All results printed as text-based console tables
//
//  Cost model (as specified):
//      Cost = LengthOfStay x BaseCostPerHour x DaysVisitsPerYear
//
//  ---------------------------------------------------------------------------
//  Source layout
//    patient_list.hpp / .cpp   Node + PatientList - the data structure
//    dataset.hpp / .cpp        Patient record, age bands, cost formula,
//                              care-type registry, CSV loading
//    report.hpp / .cpp         Analysis gather + console tables
//    main.cpp                  this file - orchestration only
//
//  Build:
//    g++ -std=c++11 -Wall -Wextra -O2 -static -o main.exe ^
//        main.cpp dataset.cpp patient_list.cpp report.cpp
// ============================================================================

#include <iostream>
#include <iomanip>
#include <string>

#include "dataset.hpp"
#include "patient_list.hpp"
#include "report.hpp"

int main() {
    const int DATASET_COUNT = 3;
    const std::string FILES[DATASET_COUNT] = {
        "dataset/dataset1 facility_a.csv",
        "dataset/dataset2 facility_b.csv",
        "dataset/dataset3_facility_c.csv"
    };
    const std::string LABELS[DATASET_COUNT] = {
        "Dataset 1 - Facility A",
        "Dataset 2 - Facility B",
        "Dataset 3 - Facility C"
    };
    // Narrow variants for the matrix headers, where each column is 16 wide.
    const std::string SHORT_LABELS[DATASET_COUNT] = {
        "Dataset 1", "Dataset 2", "Dataset 3"
    };

    PatientList dataset[DATASET_COUNT];  // one linked list per dataset
    PatientList combined;                // all three merged

    std::cout << "###  SINGLY LINKED LIST IMPLEMENTATION  ###\n\n";
    std::cout << "=== 1. DATA LOADING ===\n";
    for (int i = 0; i < DATASET_COUNT; ++i) {
        int skipped = 0;
        const int n = loadCSV(FILES[i], dataset[i], skipped);
        if (n < 0) return 1;

        std::cout << std::left << std::setw(24) << LABELS[i]
                  << "loaded " << n << " records";
        if (skipped > 0) std::cout << ", skipped " << skipped << " malformed row(s)";
        std::cout << "   [" << FILES[i] << "]\n";

        // Merge into the combined list (each list owns its own nodes).
        for (Node* cur = dataset[i].head(); cur != nullptr; cur = cur->next) {
            combined.append(cur->data);
        }
    }
    std::cout << std::left << std::setw(24) << "TOTAL"
              << combined.size() << " records across " << DATASET_COUNT
              << " datasets, " << g_careTypes.count() << " distinct care types\n";

    printLegend();

    std::cout << "\n=== 2. SAMPLE RECORDS (Dataset 1, first 10) ===\n";
    printSampleRows(dataset[0], 10);

    // One analysis pass per dataset, plus one for the combined list.
    Analysis stats[DATASET_COUNT];
    Analysis all;
    for (int i = 0; i < DATASET_COUNT; ++i) analyse(dataset[i], stats[i]);
    analyse(combined, all);

    std::cout << "\n=== 3. AGE GROUP ANALYSIS (top care type, total & average cost) ===\n";
    for (int i = 0; i < DATASET_COUNT; ++i) printAgeGroupTable(stats[i], LABELS[i]);
    printAgeGroupTable(all, "ALL DATASETS COMBINED");

    std::cout << "\n=== 4. CARE TYPE ANALYSIS (total cost per care type) ===\n";
    for (int i = 0; i < DATASET_COUNT; ++i) printCareTypeTable(stats[i], LABELS[i]);
    printCareTypeTable(all, "ALL DATASETS COMBINED");

    std::cout << "\n=== 5. CROSS-DATASET COMPARISON ===\n";
    printDatasetComparison(stats, LABELS, DATASET_COUNT, all);
    printCrossMatrix(stats, SHORT_LABELS, DATASET_COUNT, all, true);   // expenditure
    printCrossMatrix(stats, SHORT_LABELS, DATASET_COUNT, all, false);  // visit duration

    // ---- Requirement 3c: searching and sorting on the linked list ----------
    std::cout << "\n=== 6. SEARCH & SORT DEMONSTRATION ===\n";
    const std::string probe[3] = { "PT1002", "PT2003", "PT9999" };
    for (int i = 0; i < 3; ++i) {
        Node* hit = combined.findByID(probe[i]);
        if (hit == nullptr) {
            std::cout << "search " << probe[i] << ": not found\n";
        } else {
            const Patient& p = hit->data;
            std::cout << std::fixed << std::setprecision(2)
                      << "search " << p.patientID << ": age " << p.age
                      << " (" << GROUP_LABEL[p.ageGroup] << "), " << p.careType
                      << ", cost RM " << p.medicalCost << "\n";
        }
    }

    combined.sort(byCostDesc);
    std::cout << "\nTop 5 most expensive patients (merge sort by cost, descending)\n";
    printSampleRows(combined, 5);

    combined.sort(byAgeAsc);
    std::cout << "\nYoungest 5 patients (merge sort by age, ascending)\n";
    printSampleRows(combined, 5);

    return 0;   // destructors free every node
}
