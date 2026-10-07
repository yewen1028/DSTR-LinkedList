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
    s.moves = g_sortMoves;
    s.timeMs = chrono::duration<double, milli>(end - start).count();
    s.dataBytes = static_cast<long>(list.size()) * static_cast<long>(sizeof(Node));
    // Both sorts relink existing nodes in place: no copy buffer like the array merge sort.
    // Merge sort only needs one dummy stub node; insertion sort needs 3 pointers.
    s.auxBytes = useInsertion ? static_cast<long>(3 * sizeof(Node*)) : static_cast<long>(sizeof(Node));
    return s;
}

void printSortStatsTable(const SortStats stats[], int count, const string& title, int n) {
    cout << "\n" << title << " (" << n << " records)" << endl;
    cout << left << setw(24) << "Algorithm / Sort Key";
    cout << right << setw(13) << "Comparisons";
    cout << right << setw(10) << "Moves";
    cout << right << setw(12) << "Time(ms)";
    cout << right << setw(12) << "Data(B)";
    cout << right << setw(13) << "ExtraMem(B)" << endl;
    cout << string(84, '-') << endl;
    for (int i = 0; i < count; i++) {
        cout << left << setw(24) << stats[i].label;
        cout << right << setw(13) << stats[i].comparisons;
        cout << right << setw(10) << stats[i].moves;
        cout << fixed << setprecision(4);
        cout << right << setw(12) << stats[i].timeMs;
        cout << right << setw(12) << stats[i].dataBytes;
        cout << right << setw(13) << stats[i].auxBytes << endl;
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

// A singly linked list has no index: reaching position `index` means walking from head.
static Node* nodeAt(const PatientList& list, int index, long& hops) {
    Node* cur = list.head();
    for (int i = 0; i < index && cur != nullptr; i++) {
        cur = cur->next;
        hops++;
    }
    return cur;
}

SearchStats linearSearch(const PatientList& list, const SearchCriteria& crit, PatientList& results) {
    SearchStats s;
    s.label = "Linear (unsorted)";
    s.comparisons = 0;
    s.nodeHops = 0;   // sequential scan, same cost as array i++
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
    s.extraBytes = static_cast<long>(s.matches) * static_cast<long>(sizeof(Node));
    return s;
}

// Returns the index of the first node whose key is greater than threshold.
int binaryFirstGreater(const PatientList& sortedList, double (*keyOf)(const Patient&), double threshold, SearchStats& statsOut) {
    statsOut.label = "Binary (boundary)";
    statsOut.comparisons = 0;
    statsOut.nodeHops = 0;
    statsOut.extraBytes = 0;

    const int n = sortedList.size();
    chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
    int low = 0, high = n;
    while (low < high) {
        int mid = low + (high - low) / 2;
        Node* midNode = nodeAt(sortedList, mid, statsOut.nodeHops);
        statsOut.comparisons++;
        if (keyOf(midNode->data) > threshold) high = mid;
        else                                  low = mid + 1;
    }
    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();

    statsOut.timeMs = chrono::duration<double, milli>(end - start).count();
    statsOut.matches = n - low;
    return low;
}

Node* binarySearchByKey(const PatientList& sortedList, double (*keyOf)(const Patient&), double target, SearchStats& statsOut) {
    statsOut.label = "Binary (exact)";
    statsOut.comparisons = 0;
    statsOut.nodeHops = 0;
    statsOut.matches = 0;
    statsOut.extraBytes = 0;

    chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
    Node* found = nullptr;
    int low = 0;
    int high = sortedList.size() - 1;

    while (low <= high && found == nullptr) {
        int mid = low + (high - low) / 2;
        Node* temp = nodeAt(sortedList, mid, statsOut.nodeHops);

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

// Binary search for the first age >= minAge, then scan forward until age > maxAge.
SearchStats rangeSearchSortedByAge(const PatientList& sortedByAge, const SearchCriteria& crit, PatientList& results) {
    SearchStats s;
    s.label = "Binary+scan (sorted)";
    s.matches = 0;

    chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

    SearchStats bound;
    int first = binaryFirstGreater(sortedByAge, ageOf, crit.minAge - 1, bound);
    s.comparisons = bound.comparisons;
    s.nodeHops = bound.nodeHops;

    for (Node* cur = nodeAt(sortedByAge, first, s.nodeHops); cur != nullptr; cur = cur->next) {
        s.comparisons++;
        if (cur->data.age > crit.maxAge) break;
        if (crit.matches(cur->data)) {
            results.append(cur->data);
            s.matches++;
        }
    }
    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();

    s.timeMs = chrono::duration<double, milli>(end - start).count();
    s.extraBytes = static_cast<long>(s.matches) * static_cast<long>(sizeof(Node));
    return s;
}

void printSearchStatsTable(const SearchStats stats[], int count) {
    cout << left << setw(24) << "Search Type";
    cout << right << setw(13) << "Comparisons";
    cout << right << setw(12) << "Time(ms)";
    cout << right << setw(10) << "Matches";
    cout << right << setw(14) << "ExtraMem(B)";
    cout << right << setw(11) << "NodeHops" << endl;
    cout << string(84, '-') << endl;
    for (int i = 0; i < count; i++) {
        cout << left << setw(24) << stats[i].label;
        cout << right << setw(13) << stats[i].comparisons;
        cout << fixed << setprecision(5);
        cout << right << setw(12) << stats[i].timeMs;
        cout << right << setw(10) << stats[i].matches;
        cout << right << setw(14) << stats[i].extraBytes;
        cout << right << setw(11) << stats[i].nodeHops << endl;
    }
}
