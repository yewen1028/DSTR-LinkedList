// ============================================================================
//  patient_list.cpp - see patient_list.hpp
// ============================================================================
#include "patient_list.hpp"

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
    head_ = mergeSort(head_, less);
    Node* cur = head_;
    while (cur != nullptr && cur->next != nullptr) cur = cur->next;
    tail_ = cur;
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
    Node    stub(dummy);   // dummy head, avoids special-casing the first link
    Node*   tail = &stub;

    while (a != nullptr && b != nullptr) {
        if (less(b->data, a->data)) {
            tail->next = b;
            b = b->next;
        } else {                    // stable: ties keep a before b
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
