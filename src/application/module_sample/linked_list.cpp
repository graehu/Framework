module linked_list;

template <typename T> T* LinkedList<T>::get_head() { return &head.data; }
template <typename T> void LinkedList<T>::set_head(T in_head) { head = in_head; }

template class LinkedList<int>;
