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
    double timeMs;
    long memoryBytes;
};

SortStats measureSort(PatientList& list, PatientLess less, const std::string& label, bool useInsertion = false);

bool byStayAsc(const Patient& a, const Patient& b);
bool byIdAsc(const Patient& a, const Patient& b);

void printSortStatsTable(const SortStats stats[], int count);

//Step7: Searching experiment

struct SearchCriteria {
    bool useAgeRange;
    int minAge, maxAge;
    bool useCareType;
    std::string careType;
    bool useStayOver;
    int minLengthOfStay;

    SearchCriteria()
        : useAgeRange(false), minAge(0), maxAge(), useCareType(false), useStayOver(false), minLengthOfStay(0) {}
    
    bool matches(const Patient& p) const;
};

struct SearchStats {
    std::string label;
    long comparisons;
    double timeMs;
    int matches;
};

SearchStats linearSearch(const PatientList& list, const SearchCriteria& crit, PatientList& results);

Node* binarySearchByKey(const PatientList& sortedList, double (*keyOf)(const Patient&), double target, SearchStats& statsOut);

void printSearchStatsTable(const SearchStats stats[], int count);

#endif