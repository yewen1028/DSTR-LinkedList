// ============================================================================
//  patient_list.hpp - singly linked list of Patient records (requirement 3b/3c)
// ============================================================================
#ifndef PATIENT_LIST_H
#define PATIENT_LIST_H

#include <string>
#include "dataset.hpp"

// Singly linked list node.
struct Node {
    Patient data;
    Node*   next;

    explicit Node(const Patient& p) : data(p), next(nullptr) {}
};

// Comparator type used by sort(). Returns true when a must come before b.
typedef bool (*PatientLess)(const Patient& a, const Patient& b);

// ----------------------------------------------------------------------------
//  Singly linked list with a tail pointer.
//
//  Tail pointer keeps append at O(1); without it, loading 200 records would be
//  O(n^2). Ownership: the list owns every node and frees them in the
//  destructor.
//
//  Operations provided for requirement 3c:
//    - append / prepend : insertion, both O(1)
//    - findByID         : searching,  O(n)
//    - sort             : merge sort, O(n log n), comparator swappable
//    - head()           : traversal entry point for every analysis pass
// ----------------------------------------------------------------------------
class PatientList {
public:
    PatientList() : head_(nullptr), tail_(nullptr), size_(0) {}

    ~PatientList() { clear(); }

    // No copying - a shallow copy would double-free the nodes.
    PatientList(const PatientList&)            = delete;
    PatientList& operator=(const PatientList&) = delete;

    // Insert at the end. O(1).
    void append(const Patient& p);

    // Insert at the front. O(1) - no shifting, only a pointer rewrite.
    void prepend(const Patient& p);

    // Linear search by patient ID. O(n). Returns nullptr when not found.
    Node* findByID(const std::string& id) const;

    // Merge sort. O(n log n) time, O(log n) stack. Chosen over quicksort or
    // insertion sort because it needs no random access - it only relinks
    // nodes, which is exactly what a singly linked list is good at.
    void sort(PatientLess less);

    void clear();

    Node* head() const { return head_; }
    int   size() const { return size_; }
    bool  empty() const { return head_ == nullptr; }

private:
    // Splits the chain in half using the slow/fast pointer technique and
    // returns the head of the second half.
    static Node* split(Node* start);

    // Merges two already-sorted chains into one.
    static Node* merge(Node* a, Node* b, PatientLess less);

    static Node* mergeSort(Node* start, PatientLess less);

    Node* head_;
    Node* tail_;
    int   size_;
};

#endif  // PATIENT_LIST_H
