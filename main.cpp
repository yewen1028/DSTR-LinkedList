#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

#include "dataset.hpp"
#include "patient_list.hpp"
#include "report.hpp"
#include "sort_search.hpp"

using namespace std;

const int DATASET_COUNT = 3;
const int LIST_COUNT    = DATASET_COUNT + 1;

static bool readLine(const string& prompt, string& out) {
    cout << prompt;
    if (!getline(cin, out)) return false;
    trim(out);
    return true;
}

static int readInt(const string& prompt, int lo, int hi, int onEOF) {
    string line;
    while (readLine(prompt, line)) {
        stringstream ss(line);
        int value = 0;
        char extra = 0;
        if ((ss >> value) && !(ss >> extra) && value >= lo && value <= hi) {
            return value;
        }
        cout << "  Invalid input. Enter a whole number from "
                  << lo << " to " << hi << ".\n";
    }
    return onEOF;
}

// Wipe the console so each menu and result starts on a fresh screen.
// Also stops the VS Code terminal from leaving stale text from earlier output.
static void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

static void pause() {
    string ignored;
    cout << "\n(press Enter to return to the menu) ";
    getline(cin, ignored);
}

static bool loadAll(PatientList lists[], const string files[]) {
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

static int pickList(const string labels[]) {
    cout << "Select data source\n";
    for (int i = 0; i < DATASET_COUNT; ++i) {
        cout << "  " << (i + 1) << ". " << labels[i] << "\n";
    }
    cout << "  4. " << labels[DATASET_COUNT] << "\n"
              << "  0. Back\n";

    const int choice = readInt("Choice: ", 0, LIST_COUNT, 0);
    return choice - 1;
}

static void fullReport(const PatientList lists[], const Analysis stats[],
                       const string labels[], const string shortLabels[]) {
    printLegend();

    cout << "\n=== SAMPLE RECORDS (" << labels[0] << ", first 10) ===\n";
    printSampleRows(lists[0], 10);

    cout << "\n=== AGE GROUP ANALYSIS (top care type, total & average cost) ===\n";
    for (int i = 0; i < LIST_COUNT; ++i) printAgeGroupTable(stats[i], labels[i]);

    cout << "\n=== CARE TYPE ANALYSIS (total cost per care type) ===\n";
    for (int i = 0; i < LIST_COUNT; ++i) printCareTypeTable(stats[i], labels[i]);

    cout << "\n=== CROSS-DATASET COMPARISON ===\n";
    printDatasetComparison(stats, labels, DATASET_COUNT, stats[DATASET_COUNT]);
    printCrossMatrix(stats, shortLabels, DATASET_COUNT, stats[DATASET_COUNT], true);
    printCrossMatrix(stats, shortLabels, DATASET_COUNT, stats[DATASET_COUNT], false);
}

//Step6: Sorting experiment
static void sortExperiment(PatientList lists[], const string labels[]) {
    cout << "\n=== SORTING EXPERIMENT ===" << endl;
    for (int i = 0; i < LIST_COUNT; ++i) {
        PatientLess rules[3] = { byAgeAsc, byStayAsc, byCostDesc };
        string names[3] = { "Age", "LengthOfStay", "TotalCost" };
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

        lists[i].sort(byIdAsc);   // leave list in original order for the other menu options
    }
}

//Step7: Searching experiment
static void searchExperiment(PatientList lists[], const string labels[]) {
    cout << "\n=== SEARCHING EXPERIMENT ===" << endl;
    // Senior emergency search: age group 61-100 + care type Emergency
    SearchCriteria seniorEmergency;
    seniorEmergency.useAgeRange = true;
    seniorEmergency.minAge = 61;
    seniorEmergency.maxAge = 100;
    seniorEmergency.useCareType = true;
    seniorEmergency.careType = "Emergency";

    // Long stay search: visit duration threshold > 24 hours
    SearchCriteria longStay;
    longStay.useStayOver = true;
    longStay.minLengthOfStay = 24;

    for (int i = 0; i < LIST_COUNT; ++i) {
        cout << "\n------------------------------------------------------------\n"
                  << labels[i] << " (" << lists[i].size() << " records)\n"
                  << "------------------------------------------------------------" << endl;

        // Senior emergency search: linear search on unsorted (original order), then on data sorted by age
        PatientList emergencyMatches, emergencyMatchesSorted;
        SearchStats emergencyStats[3];

        lists[i].sort(byIdAsc);
        emergencyStats[0] = linearSearch(lists[i], seniorEmergency, emergencyMatches);

        lists[i].sort(byAgeAsc);
        emergencyStats[1] = rangeSearchSortedByAge(lists[i], seniorEmergency, emergencyMatchesSorted);
        Node* hit = binarySearchByKey(lists[i], ageOf, 65.0, emergencyStats[2]);

        cout << "\nSenior Emergency Patients: Age 61-100 + Care Type = Emergency" << endl;
        printSearchStatsTable(emergencyStats, 3);
        cout << "\nMatching records (linear search result):" << endl;
        printSampleRows(emergencyMatches, 10);
        if (hit != nullptr)
            cout << "Binary search found a patient aged 65: " << hit->data.patientID << endl;
        else
            cout << "Binary search: no patient aged exactly 65 in this dataset" << endl;

        // Long stay search: visit duration threshold
        PatientList longStayMatches;
        SearchStats longStayStats[2];

        lists[i].sort(byIdAsc);
        longStayStats[0] = linearSearch(lists[i], longStay, longStayMatches);

        lists[i].sort(byStayAsc);
        const int first = binaryFirstGreater(lists[i], stayOf, 24.0, longStayStats[1]);

        cout << "\nLong Stay Patients: Visit Duration > 24 hours" << endl;
        printSearchStatsTable(longStayStats, 2);
        cout << "\nMatching records (linear search result):" << endl;
        printSampleRows(longStayMatches, 10);
        cout << "Binary search boundary index = " << first << " (" << longStayStats[1].matches
                  << " records from this index onwards match)" << endl;

        lists[i].sort(byIdAsc);   // leave list in original order for the other menu options
    }
}

int main() {
    const string FILES[DATASET_COUNT] = {
        "dataset/dataset1 facility_a.csv",
        "dataset/dataset2 facility_b.csv",
        "dataset/dataset3_facility_c.csv"
    };
    const string LABELS[LIST_COUNT] = {
        "Dataset 1 - Facility A",
        "Dataset 2 - Facility B",
        "Dataset 3 - Facility C",
        "ALL DATASETS COMBINED"
    };
    const string SHORT_LABELS[DATASET_COUNT] = {
        "Dataset 1", "Dataset 2", "Dataset 3"
    };

    PatientList lists[LIST_COUNT];

    if (!loadAll(lists, FILES)) {
        cerr << "Loading failed - check that the dataset folder sits next "
                     "to the executable.\n";
        return 1;
    }

    Analysis stats[LIST_COUNT];
    for (int i = 0; i < LIST_COUNT; ++i) analyse(lists[i], stats[i]);

    bool running = true;
    while (running) {
        clearScreen();
        cout << "============================================================\n"
                  << "  Singly Linked List Menu\n"
                  << "============================================================\n"
                  << "  1. Age group legend\n"
                  << "  2. Browse records\n"
                  << "  3. Age group analysis\n"
                  << "  4. Care type analysis\n"
                  << "  5. Cross-dataset comparison\n"
                  << "  6. Full report (everything above)\n"
                  << "  7. Sorting experiment\n"
                  << "  8. Searching experiment\n"
                  << "  0. Exit\n";

        const int choice = readInt("Enter a Choice: ", 0, 8, 0);
        clearScreen();

        switch (choice) {
            case 1:
                printLegend();
                pause();
                break;

            case 2: {
                const int which = pickList(LABELS);
                if (which < 0) break;
                const int rows = readInt("How many records to display (1-50): ", 1, 50, 10);
                clearScreen();
                cout << "\n" << LABELS[which] << "  ("
                          << lists[which].size() << " records)\n";
                printSampleRows(lists[which], rows);
                pause();
                break;
            }

            case 3: {
                const int which = pickList(LABELS);
                if (which < 0) break;
                clearScreen();
                printAgeGroupTable(stats[which], LABELS[which]);
                pause();
                break;
            }

            case 4: {
                const int which = pickList(LABELS);
                if (which < 0) break;
                clearScreen();
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
                sortExperiment(lists, LABELS);
                pause();
                break;

            case 8:
                searchExperiment(lists, LABELS);
                pause();
                break;

            case 0:
            default:
                running = false;
                break;
        }
    }

    cout << "\nExiting.\n";
    return 0;
}
