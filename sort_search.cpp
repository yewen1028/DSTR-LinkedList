#include <iostream>
#include <iomanip>
#include <chrono>
#include "sort_search.hpp"
using namespace std;

//Step6: Sorting experiment
double ageOf(const Patient& p) {
    return p.age;
}

double stayOf(const Patient& p) {
    return p.lengthOfStay;
}

double costOf(const Patient& p) {
    return p.medicalCost;
}

bool byStayAsc(const Patient& a, const Patient& b) {
    return a.lengthOfStay < b.lengthOfStay;
}

bool byIdAsc(const Patient& a, const Patient& b) {
    return a.patientID < b.patientID;
}

SortStats measureSort(PatientList& list, PatientLess less, const string& label, bool useInsertion) {
    SortStats s;
    s.label = label;

    chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
    if (useInsertion)
        list.insertionSort(less);
    else
        list.sort(less);
    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();

    s.comparisons = g_sortComparisons;
    s.timeMs = chrono::duration<double, milli>(end - start).count();
    s.memoryBytes = list.size() * sizeof(Node);
    return s;
}

void printSortStatsTable(const SortStats stats[], int count) {
    cout << endl;
    cout << "Step 6 - Sorting performance (singly linked list)" << endl;
    cout << left << setw(24) << "Algorithm/SortKey";
    cout << right << setw(14) << "Comparisons";
    cout << right << setw(12) << "Time(ms)";
    cout << right << setw(16) << "Est.MemBytes" << endl;
    cout << string(66, '-') << endl;
    for (int i = 0; i < count; i++) {
        cout << left << setw(24) << stats[i].label;
        cout << right << setw(14) << stats[i].comparisons;
        cout << fixed << setprecision(4);
        cout << right << setw(12) << stats[i].timeMs;
        cout << right << setw(16) << stats[i].memoryBytes << endl;
    }
}

//Step7: Searching experiment
bool SearchCriteria::matches(const Patient& p) const {
    if (useAgeRange && (p.age < minAge || p.age > maxAge))
        return false;
    if (useCareType && p.careType != careType)
        return false;
    if (useStayOver && !(p.lengthOfStay > minLengthOfStay))
        return false;
    return true;
}

SearchStats linearSearch(const PatientList& list, const SearchCriteria& crit, PatientList& results) {
    SearchStats s;
    s.label = "Linear (unsorted)";
    s.comparisons = 0;
    s.matches = 0;

    chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
    Node* temp = list.head();
    while (temp != nullptr) {
        s.comparisons++;
        if (crit.matches(temp->data)) {
            results.append(temp->data);
            s.matches++;
        }
        temp = temp->next;
    }
    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();

    s.timeMs = chrono::duration<double, milli>(end - start).count();
    return s;
}

Node* binarySearchByKey(const PatientList& sortedList, double (*keyOf)(const Patient&), double target, SearchStats& statsOut) {
    statsOut.label = "Binary (sorted)";
    statsOut.comparisons = 0;
    statsOut.matches = 0;

    chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
    Node* found = nullptr;
    int low = 0;
    int high = sortedList.size() - 1;

    while (low <= high && found == nullptr) {
        int mid = (low + high) / 2;

        Node* temp = sortedList.head();
        for (int i = 0; i < mid; i++) {
            temp = temp->next;
            statsOut.comparisons++;
        }

        statsOut.comparisons++;
        double key = keyOf(temp->data);
        if (key == target)
            found = temp;
        else if (key < target)
            low = mid + 1;
        else
            high = mid - 1;
    }
    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();

    statsOut.timeMs = chrono::duration<double, milli>(end - start).count();
    if (found != nullptr)
        statsOut.matches = 1;
    return found;
}

void printSearchStatsTable(const SearchStats stats[], int count) {
    cout << endl;
    cout << "Step 7 - Search performance (singly linked list)" << endl;
    cout << left << setw(22) << "SearchType";
    cout << right << setw(14) << "Comparisons";
    cout << right << setw(12) << "Time(ms)";
    cout << right << setw(10) << "Matches" << endl;
    cout << string(58, '-') << endl;
    for (int i = 0; i < count; i++) {
        cout << left << setw(22) << stats[i].label;
        cout << right << setw(14) << stats[i].comparisons;
        cout << fixed << setprecision(4);
        cout << right << setw(12) << stats[i].timeMs;
        cout << right << setw(10) << stats[i].matches << endl;
    }
}
