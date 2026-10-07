#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

#include "dataset.hpp"
#include "patient_list.hpp"
#include "report.hpp"
#include "sort_search.hpp"

const int DATASET_COUNT = 3;
const int LIST_COUNT    = DATASET_COUNT + 1;

static bool readLine(const std::string& prompt, std::string& out) {
    std::cout << prompt;
    if (!std::getline(std::cin, out)) return false;
    trim(out);
    return true;
}

static int readInt(const std::string& prompt, int lo, int hi, int onEOF) {
    std::string line;
    while (readLine(prompt, line)) {
        std::stringstream ss(line);
        int value = 0;
        char extra = 0;
        if ((ss >> value) && !(ss >> extra) && value >= lo && value <= hi) {
            return value;
        }
        std::cout << "  Invalid input. Enter a whole number from "
                  << lo << " to " << hi << ".\n";
    }
    return onEOF;
}

static void pause() {
    std::string ignored;
    std::cout << "\n(press Enter to return to the menu) ";
    std::getline(std::cin, ignored);
}

static bool loadAll(PatientList lists[], const std::string files[]) {
    for (int i = 0; i < DATASET_COUNT; ++i) {
        int skipped = 0;
        const int n = loadCSV(files[i], lists[i], skipped);
        if (n < 0) return false;

        for (Node* cur = lists[i].head(); cur != nullptr; cur = cur->next) {
            lists[DATASET_COUNT].append(cur->data);
        }
    }
    return true;
}

static int pickList(const std::string labels[]) {
    std::cout << "\nSelect data source\n";
    for (int i = 0; i < DATASET_COUNT; ++i) {
        std::cout << "  " << (i + 1) << ". " << labels[i] << "\n";
    }
    std::cout << "  4. " << labels[DATASET_COUNT] << "\n"
              << "  0. Back\n";

    const int choice = readInt("Choice: ", 0, LIST_COUNT, 0);
    return choice - 1;
}

static void fullReport(const PatientList lists[], const Analysis stats[],
                       const std::string labels[], const std::string shortLabels[]) {
    printLegend();

    std::cout << "\n=== SAMPLE RECORDS (" << labels[0] << ", first 10) ===\n";
    printSampleRows(lists[0], 10);

    std::cout << "\n=== AGE GROUP ANALYSIS (top care type, total & average cost) ===\n";
    for (int i = 0; i < LIST_COUNT; ++i) printAgeGroupTable(stats[i], labels[i]);

    std::cout << "\n=== CARE TYPE ANALYSIS (total cost per care type) ===\n";
    for (int i = 0; i < LIST_COUNT; ++i) printCareTypeTable(stats[i], labels[i]);

    std::cout << "\n=== CROSS-DATASET COMPARISON ===\n";
    printDatasetComparison(stats, labels, DATASET_COUNT, stats[DATASET_COUNT]);
    printCrossMatrix(stats, shortLabels, DATASET_COUNT, stats[DATASET_COUNT], true);
    printCrossMatrix(stats, shortLabels, DATASET_COUNT, stats[DATASET_COUNT], false);
}

//Step6 and Step7: Sorting and searching experiment
static void sortSearchExperiment(PatientList lists[], const std::string labels[]) {
    //Step6: Sorting experiment
    std::cout << "\n=== SORTING EXPERIMENT ===" << std::endl;
    for (int i = 0; i < LIST_COUNT; ++i) {
        PatientLess rules[3] = { byAgeAsc, byStayAsc, byCostDesc };
        std::string names[3] = { "Age", "LengthOfStay", "TotalCost" };
        SortStats stats[6];

        // Reset to original (ID) order before every run so each sort starts
        // from the same input as the array version.
        for (int k = 0; k < 3; ++k) {
            lists[i].sort(byIdAsc);
            stats[k] = measureSort(lists[i], rules[k], "Merge / " + names[k], false);
            lists[i].sort(byIdAsc);
            stats[k + 3] = measureSort(lists[i], rules[k], "Insertion / " + names[k], true);
        }
        printSortStatsTable(stats, 6, labels[i], lists[i].size());
    }

    //Step7: Searching experiment
    std::cout << "\n=== SEARCHING EXPERIMENT ===" << std::endl;
    SearchCriteria critA;
    critA.useAgeRange = true;
    critA.minAge = 61;
    critA.maxAge = 100;
    critA.useCareType = true;
    critA.careType = "Emergency";

    SearchCriteria critB;
    critB.useStayOver = true;
    critB.minLengthOfStay = 24;

    for (int i = 0; i < LIST_COUNT; ++i) {
        std::cout << "\n------------------------------------------------------------\n"
                  << labels[i] << " (" << lists[i].size() << " records)\n"
                  << "------------------------------------------------------------" << std::endl;

        // Query A: linear search on unsorted (original order), then on data sorted by age
        PatientList matchesA, matchesSortedA;
        SearchStats a[3];

        lists[i].sort(byIdAsc);
        a[0] = linearSearch(lists[i], critA, matchesA);

        lists[i].sort(byAgeAsc);
        a[1] = rangeSearchSortedByAge(lists[i], critA, matchesSortedA);
        Node* hit = binarySearchByKey(lists[i], ageOf, 65.0, a[2]);

        std::cout << "\nQuery A: Age 61-100 + Care Type = Emergency" << std::endl;
        printSearchStatsTable(a, 3);
        std::cout << "\nMatching records (linear search result):" << std::endl;
        printSampleRows(matchesA, 10);
        if (hit != nullptr)
            std::cout << "Binary search found a patient aged 65: " << hit->data.patientID << std::endl;
        else
            std::cout << "Binary search: no patient aged exactly 65 in this dataset" << std::endl;

        // Query B: visit duration threshold
        PatientList matchesB;
        SearchStats b[2];

        lists[i].sort(byIdAsc);
        b[0] = linearSearch(lists[i], critB, matchesB);

        lists[i].sort(byStayAsc);
        const int first = binaryFirstGreater(lists[i], stayOf, 24.0, b[1]);

        std::cout << "\nQuery B: Length of Stay > 24 hours" << std::endl;
        printSearchStatsTable(b, 2);
        std::cout << "\nMatching records (linear search result):" << std::endl;
        printSampleRows(matchesB, 10);
        std::cout << "Binary search boundary index = " << first << " (" << b[1].matches
                  << " records from this index onwards match)" << std::endl;

        lists[i].sort(byIdAsc);   // leave list in original order for the other menu options
    }
}

int main() {
    const std::string FILES[DATASET_COUNT] = {
        "dataset/dataset1 facility_a.csv",
        "dataset/dataset2 facility_b.csv",
        "dataset/dataset3_facility_c.csv"
    };
    const std::string LABELS[LIST_COUNT] = {
        "Dataset 1 - Facility A",
        "Dataset 2 - Facility B",
        "Dataset 3 - Facility C",
        "ALL DATASETS COMBINED"
    };
    const std::string SHORT_LABELS[DATASET_COUNT] = {
        "Dataset 1", "Dataset 2", "Dataset 3"
    };

    PatientList lists[LIST_COUNT];

    if (!loadAll(lists, FILES)) {
        std::cerr << "Loading failed - check that the dataset folder sits next "
                     "to the executable.\n";
        return 1;
    }

    Analysis stats[LIST_COUNT];
    for (int i = 0; i < LIST_COUNT; ++i) analyse(lists[i], stats[i]);

    bool running = true;
    while (running) {
        std::cout << "\n============================================================\n"
                  << "  Singly Linked List Menu\n"
                  << "============================================================\n"
                  << "  1. Age group legend\n"
                  << "  2. Browse records\n"
                  << "  3. Age group analysis\n"
                  << "  4. Care type analysis\n"
                  << "  5. Cross-dataset comparison\n"
                  << "  6. Full report (everything above)\n"
                  << "  7. Sorting and searching experiment\n"
                  << "  0. Exit\n";

        const int choice = readInt("Enter a Choice: ", 0, 7, 0);

        switch (choice) {
            case 1:
                printLegend();
                pause();
                break;

            case 2: {
                const int which = pickList(LABELS);
                if (which < 0) break;
                const int rows = readInt("How many records to display (1-50): ", 1, 50, 10);
                std::cout << "\n" << LABELS[which] << "  ("
                          << lists[which].size() << " records)\n";
                printSampleRows(lists[which], rows);
                pause();
                break;
            }

            case 3: {
                const int which = pickList(LABELS);
                if (which < 0) break;
                printAgeGroupTable(stats[which], LABELS[which]);
                pause();
                break;
            }

            case 4: {
                const int which = pickList(LABELS);
                if (which < 0) break;
                printCareTypeTable(stats[which], LABELS[which]);
                pause();
                break;
            }

            case 5:
                printDatasetComparison(stats, LABELS, DATASET_COUNT, stats[DATASET_COUNT]);
                printCrossMatrix(stats, SHORT_LABELS, DATASET_COUNT, stats[DATASET_COUNT], true);
                printCrossMatrix(stats, SHORT_LABELS, DATASET_COUNT, stats[DATASET_COUNT], false);
                pause();
                break;

            case 6:
                fullReport(lists, stats, LABELS, SHORT_LABELS);
                pause();
                break;

            case 7:
                sortSearchExperiment(lists, LABELS);
                pause();
                break;

            case 0:
            default:
                running = false;
                break;
        }
    }

    std::cout << "\nExiting.\n";
    return 0;
}
