
module linked_list;

template <typename T>
T* LinkedList<T>::get_head()
{
   return &head.data;
}    
