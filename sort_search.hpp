#ifndef SORT_SEARCH_HPP
#define SORT_SEARCH_HPP

#include <string>

#include "dataset.hpp"
#include "patient_list.hpp"

//Step6: Sorting experiment

double ageOf(const Patient& p);
double stayOf(const Patient& p);
double costOf(const Patient& p);

struct SortStats {
    std::string label;
    long comparisons;
    long moves;        // relink pointer
    double timeMs;
    long dataBytes;    // size of all nodes in list
    long auxBytes;     // extra memory
};

SortStats measureSort(PatientList& list, PatientLess less, const std::string& label, bool useInsertion = false);

bool byStayAsc(const Patient& a, const Patient& b);
bool byIdAsc(const Patient& a, const Patient& b);

void printSortStatsTable(const SortStats stats[], int count, const std::string& title, int n);

//Step7: Searching experiment

struct SearchCriteria {
    bool useAgeRange;
    int minAge, maxAge;
    bool useCareType;
    std::string careType;
    bool useStayOver;
    int minLengthOfStay;

    SearchCriteria()
        : useAgeRange(false), minAge(0), maxAge(0), useCareType(false), useStayOver(false), minLengthOfStay(0) {}

    bool matches(const Patient& p) const;
};

struct SearchStats {
    std::string label;
    long comparisons;  // key comparisons only, same meaning as the array version
    long nodeHops;     // next-pointer steps needed to reach nodes (array: 0, direct indexing)
    double timeMs;
    int matches;
    long extraBytes;
};

SearchStats linearSearch(const PatientList& list, const SearchCriteria& crit, PatientList& results);

int binaryFirstGreater(const PatientList& sortedList, double (*keyOf)(const Patient&), double threshold, SearchStats& statsOut);

Node* binarySearchByKey(const PatientList& sortedList, double (*keyOf)(const Patient&), double target, SearchStats& statsOut);

SearchStats rangeSearchSortedByAge(const PatientList& sortedByAge, const SearchCriteria& crit, PatientList& results);

void printSearchStatsTable(const SearchStats stats[], int count);

#endif
