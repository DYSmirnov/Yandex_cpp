#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <list>
#include <numeric>
#include <random>

using namespace std;

template <typename It>
void PrintRange(It range_begin, It range_end) {
    for (auto it = range_begin; it != range_end; ++it) {
        cout << *it << " "s;
    }
    cout << endl;
}

template <typename Type>
class Stack {
public:
    void Push(const Type& element) {
        elements_.push_back(element);
    }
    void Pop() {
        elements_.pop_back();
    }
    const Type& Peek() const {
        return elements_.back();
    }
    Type& Peek() {
        return elements_.back();
    }
    void Print() const {
        PrintRange(begin(elements_), end(elements_));
    }
    uint64_t Size() const {
        return static_cast<uint64_t>(elements_.size());
    }
    bool IsEmpty() const {
        return elements_.empty();
    }

private:
    vector<Type> elements_;
};

#include <stack>

using namespace std;

template <typename Type>
class Queue {
public:
    void Push(const Type& element) {
        // �������� ����������
    }
    void Pop() {
        // �������� ����������
    }
    Type& Front() {
        // �������� ����������
    }
    uint64_t Size() const {
        // �������� ����������
    }
    bool IsEmpty() const {
        // �������� ����������
    }

private:
    stack<Type> stack1_;
    stack<Type> stack2_;
};

int main() {
    setlocale(LC_ALL, "Russian");
    Queue<int> queue;
    vector<int> values(5);
    // ��������� ������ ��� ������������ �������
    iota(values.begin(), values.end(), 1);
    // ������������ ��������
    std::random_device rd;
    std::mt19937 g(rd());
    cout << endl;
    PrintRange(values.begin(), values.end());
    cout << "��������� �������"s << endl;
    // ��������� ������� � ������� ������� � ������ �������
    for (int i = 0; i < 5; ++i) {
        queue.Push(values[i]);
        cout << "����������� ������� "s << values[i] << endl;
        cout << "������ ������� ������� "s << queue.Front() << endl;
    }
    cout << "�������� �������� �� �������"s << endl;
    // ������� ������� � ������ ������� � ����������� �������� �� ������
    while (!queue.IsEmpty()) {
        // ������� ����� ��������� ��������� �������, � ����� �����������,
        // ��� ��� �������� Front �� ������ ������� �� ����������
        cout << "����� �������� ������� "s << queue.Front() << endl;
        queue.Pop();
    }
    return 0;
}
