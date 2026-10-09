#ifndef PATIENT_LIST_H
#define PATIENT_LIST_H

#include <string>
#include "dataset.hpp"

struct Node {
    Patient data;
    Node*   next;

    explicit Node(const Patient& p) : data(p), next(nullptr) {}
};

typedef bool (*PatientLess)(const Patient& a, const Patient& b);

extern long g_sortComparisons;
extern long g_sortMoves;

class PatientList {
public:
    PatientList() : head_(nullptr), tail_(nullptr), size_(0) {}

    ~PatientList() { clear(); }

    PatientList(const PatientList&)            = delete;
    PatientList& operator=(const PatientList&) = delete;

    void append(const Patient& p);

    void sort(PatientLess less);
    void insertionSort(PatientLess less);

    void clear();

    Node* head() const { return head_; }
    int   size() const { return size_; }

private:
    static Node* split(Node* start);

    static Node* merge(Node* a, Node* b, PatientLess less);

    static Node* mergeSort(Node* start, PatientLess less);

    Node* head_;
    Node* tail_;
    int   size_;
};

#endif
