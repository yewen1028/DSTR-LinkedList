#include "patient_list.hpp"

long g_sortComparisons = 0;

void PatientList::append(const Patient& p) {
    Node* n = new Node(p);
    if (tail_ == nullptr) {
        head_ = tail_ = n;
    } else {
        tail_->next = n;
        tail_ = n;
    }
    ++size_;
}

void PatientList::prepend(const Patient& p) {
    Node* n = new Node(p);
    n->next = head_;
    head_ = n;
    if (tail_ == nullptr) tail_ = n;
    ++size_;
}

Node* PatientList::findByID(const std::string& id) const {
    for (Node* cur = head_; cur != nullptr; cur = cur->next) {
        if (cur->data.patientID == id) return cur;
    }
    return nullptr;
}

void PatientList::sort(PatientLess less) {
    g_sortComparisons = 0;
    head_ = mergeSort(head_, less);
    Node* cur = head_;
    while (cur != nullptr && cur->next != nullptr) cur = cur->next;
    tail_ = cur;
}

// Insertion sort for linked list
void PatientList::insertionSort(PatientLess less) {
    g_sortComparisons = 0;
    Node* sorted = nullptr;
    Node* cur = head_;
    while (cur != nullptr) {
        Node* nxt = cur->next;
        if (sorted == nullptr) {
            ++g_sortComparisons;
            sorted = cur;
        } else {
            ++g_sortComparisons;
            if (less(cur->data, sorted->data)) {
                cur->next = sorted;
                sorted = cur;
            } else {
                Node* p = sorted;
                while (p->next != nullptr) {
                    ++g_sortComparisons;
                    if (less(cur->data, p->next->data)) break;
                    p = p->next;
                }
            cur->next = p->next;
            p->next = cur;
        }
        }
        cur = nxt;
    }
    head_ = sorted;
    Node* last = head_;
    while (last != nullptr && last->next != nullptr) last = last->next;
    tail_ = last;
}

void PatientList::clear() {
    Node* cur = head_;
    while (cur != nullptr) {
        Node* nxt = cur->next;
        delete cur;
        cur = nxt;
    }
    head_ = tail_ = nullptr;
    size_ = 0;
}

Node* PatientList::split(Node* start) {
    Node* slow = start;
    Node* fast = start->next;
    while (fast != nullptr && fast->next != nullptr) {
        slow = slow->next;
        fast = fast->next->next;
    }
    Node* second = slow->next;
    slow->next = nullptr;
    return second;
}

Node* PatientList::merge(Node* a, Node* b, PatientLess less) {
    Patient dummy;
    Node    stub(dummy);
    Node*   tail = &stub;

    while (a != nullptr && b != nullptr) {
        ++g_sortComparisons;
        if (less(b->data, a->data)) {
            tail->next = b;
            b = b->next;
        } else {
            tail->next = a;
            a = a->next;
        }
        tail = tail->next;
    }
    tail->next = (a != nullptr) ? a : b;
    return stub.next;
}

Node* PatientList::mergeSort(Node* start, PatientLess less) {
    if (start == nullptr || start->next == nullptr) return start;
    Node* second = split(start);
    return merge(mergeSort(start, less), mergeSort(second, less), less);
}
