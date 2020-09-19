#include <algorithm>
#include <set>

using std::set;

template<class T>
set<T> union_set(const set<T> &A, const set<T> &B) {
    set<T> result = A;
    for (auto ele : B) result.insert(ele);
    return std::move(result);
}

template<class T>
set<T> operator+(const set<T> &A, const set<T> &B) {
    return union_set(A, B);
}

template<class T>
set<T> &operator+=(set<T> &A, const set<T> &B) {
    for (auto ele : B) A.insert(ele);
    return A;
}

template<class T>
set<T> intersection_set(const set<T> &A, const set<T> &B) {
    set<T> result;
    for (auto ele : A)
        if (B.find(ele) != B.end()) result.insert(ele);
    return std::move(result);
}
