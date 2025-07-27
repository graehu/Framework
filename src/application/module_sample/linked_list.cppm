export module linked_list;

export template<typename T>
class LinkedList {
private:
    struct Node
    {
        T data;
        Node* next;
        Node(const T& value) : data(value), next(nullptr) {}
    };
    Node* head;
public:
    T* get_head();
};