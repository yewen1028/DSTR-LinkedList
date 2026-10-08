#include "patient_list.hpp"

long g_sortComparisons = 0;
long g_sortMoves = 0;

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

void PatientList::sort(PatientLess less) {
    g_sortComparisons = 0;
    g_sortMoves = 0;
    head_ = mergeSort(head_, less);
    Node* cur = head_;
    while (cur != nullptr && cur->next != nullptr) cur = cur->next;
    tail_ = cur;
}

//Step6: Sorting experiment - insertion sort
void PatientList::insertionSort(PatientLess less) {
    g_sortComparisons = 0;
    g_sortMoves = 0;
    Node* sorted = nullptr;
    Node* current = head_;

    while (current != nullptr) {
        Node* nextNode = current->next;
        if (sorted != nullptr) g_sortComparisons++;

        if (sorted == nullptr || less(current->data, sorted->data)) {
            current->next = sorted;
            sorted = current;
        } else {
            Node* temp = sorted;
            while (temp->next != nullptr) {
                g_sortComparisons++;
                if (less(current->data, temp->next->data))
                    break;
                temp = temp->next;
            }
            current->next = temp->next;
            temp->next = current;
        }
        g_sortMoves += 2;   // two pointer relinks per insertion
        current = nextNode;
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
        ++g_sortMoves;
    }
    tail->next = (a != nullptr) ? a : b;
    ++g_sortMoves;
    return stub.next;
}

Node* PatientList::mergeSort(Node* start, PatientLess less) {
    if (start == nullptr || start->next == nullptr) return start;
    Node* second = split(start);
    return merge(mergeSort(start, less), mergeSort(second, less), less);
}
