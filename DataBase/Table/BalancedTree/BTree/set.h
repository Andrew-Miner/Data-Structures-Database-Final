#ifndef SET_H
#define SET_H
#include <iostream>
#include <iomanip>
#include "../array_assist_functions.h"

template <typename T>
class Set
{
public:
    Set();
    Set(const Set<T>& source);
    ~Set() { clear(); }

    Set operator=(const Set<T> &source);

    void clear();
    bool insert(const T &item);
    std::size_t erase(const T& item);
    std::size_t contains(const T& item) const;
    bool empty() const { return data_count; }

    friend std::ostream& operator<<(std::ostream &out, const Set<T>& set)
    {
        set.print(out, 0);
        return out;
    }



private:
    static const std::size_t MINIMUM = 1;
    static const std::size_t MAXIMUM = MINIMUM*2;
    static const std::size_t EXTRA_INDENTATION = 4;

    std::size_t data_count;
    T data[MAXIMUM+1];

    std::size_t child_count;
    Set* subset[MAXIMUM+2];

    bool is_leaf() const { return child_count == 0; }
    bool loose_insert(const T &item);
    bool loose_erase(const T &item);
    void remove_biggest(T &removed_item);

    void fix_excess(std::size_t i);
    void fix_shortage(std::size_t i);

    void print(std::ostream &out, std::size_t indent) const;
    void clear(Set<T>* clear_me);
    Set<T>* copy(const Set<T>* copy_me);
};

template <typename T>
Set<T>::Set()
{
    data_count = 0;
    child_count = 0;
}

template <typename T>
Set<T>::Set(const Set<T>& source)
{
    array_assist::copy(source.data, source.data_count, data);
    data_count = source.data_count;

    for(int i = 0; i < source.child_count; i++)
        subset[i] = copy(source.subset[i]);
    child_count = source.child_count;
}

template <typename T>
Set<T> Set<T>::operator=(const Set<T> &source)
{
    array_assist::copy(source.data, source.data_count, data);
    data_count = source.data_count;

    for(int i = 0; i < source.child_count; i++)
        subset[i] = copy(source.subset[i]);
    child_count = source.child_count;

    return *this;
}

template <typename T>
Set<T>* Set<T>::copy(const Set<T>* copy_me)
{
    Set<T>* set_copy = new Set<T>();

    array_assist::copy(copy_me->data, copy_me->data_count, set_copy->data);
    set_copy->data_count = copy_me->data_count;

    for(int i = 0; i < copy_me->child_count; i++)
        set_copy->subset[i] = copy(copy_me->subset[i]);
    set_copy->child_count = copy_me->child_count;

    return set_copy;
}

template <typename T>
void Set<T>::clear()
{
    clear(this);
    child_count = 0;
    data_count = 0;
}

template <typename T>
void Set<T>::clear(Set<T>* clear_me)
{
    if(clear_me->is_leaf())
        return;

    for(int i = 0; i < clear_me->child_count; i++)
    {
        clear(clear_me->subset[i]);
        delete clear_me->subset[i];
    }

}

template <typename T>
bool Set<T>::insert(const T &item)
{
    if(!loose_insert(item))
        return false;
    if(data_count > MAXIMUM)
    {
        Set<T>* sub_root = new Set<T>();

        array_assist::copy(data, data_count, sub_root->data);
        sub_root->data_count = data_count;
        data_count = 0;

        array_assist::copy(subset, child_count, sub_root->subset);
        sub_root->child_count = child_count;
        child_count = 1;
        subset[0] = sub_root;

        fix_excess(0);
    }
    return true;
}

template <typename T>
std::size_t Set<T>::erase(const T& item)
{
    if(!loose_erase(item))
        return false;
    if(data_count == 0 && child_count == 1)
    {
        Set<T>* child = subset[0];

        array_assist::copy(child->data, child->data_count, data);
        data_count = child->data_count;

        array_assist::copy(child->subset, child->child_count, subset);
        child_count = child->child_count;

        child->child_count = 0;
        delete child;
    }
    return true;
}

template <typename T>
std::size_t Set<T>::contains(const T& item) const
{
    std::size_t i = array_assist::get_sorted_index(data, data_count, item);

    if(i < data_count && !(item < data))
        return 1;
    if(is_leaf())
        return 0;
    return subset[i]->count(item);
}

template <typename T>
bool Set<T>::loose_insert(const T &item)
{
    std::size_t i = array_assist::get_sorted_index(data, data_count, item);

    if(i < data_count && !(item < data[i]))
        return false;

    if(is_leaf())
    {
        array_assist::insert(data, data_count, i, item);
        return true;
    }

    bool temp = subset[i]->loose_insert(item);

    if(subset[i]->data_count > MAXIMUM)
        fix_excess(i);
    return temp;
}

template <typename T>
bool Set<T>::loose_erase(const T &item)
{
    std::size_t i = array_assist::get_sorted_index(data, data_count, item);

    if(child_count == 0 && i == data_count)
        return false;

    if(child_count == 0 && i < data_count && !(item < data[i]))
    {
        array_assist::remove(data, data_count, i);
        return true;
    }

    if(child_count && !(i < data_count && !(item < data[i])))
    {
       bool temp = subset[i]->loose_erase(item);
       if(subset[i]->data_count < MINIMUM)
           fix_shortage(i);
       return temp;
    }

    if(child_count && i < data_count && !(item < data[i]))
    {
        subset[i]->remove_biggest(data[i]);
        if(subset[i]->data_count < MINIMUM)
            fix_shortage(i);
        return true;
    }
}

template <typename T>
void Set<T>::remove_biggest(T &removed_item)
{
    if(is_leaf())
        removed_item = data[--data_count];
    else
    {
        subset[child_count-1]->remove_biggest(removed_item);
        if(subset[child_count-1]->data_count < MINIMUM)
            fix_shortage(child_count-1);
    }
}

template <typename T>
void Set<T>::fix_excess(std::size_t i)
{
    Set<T>* new_sub = new Set<T>();
    std::size_t pos = subset[i]->data_count/2 + 1;

    array_assist::split(subset[i]->subset, subset[i]->child_count, pos, new_sub->subset, new_sub->child_count);
    array_assist::split(subset[i]->data, subset[i]->data_count, pos, new_sub->data, new_sub->data_count);

    T middle = subset[i]->data[--(subset[i]->data_count)];
    std::size_t j = array_assist::get_sorted_index(data, data_count, middle);

    array_assist::insert(data, data_count, j, middle);
    array_assist::insert(subset, child_count, j+1, new_sub);
}

template <typename T>
void Set<T>::fix_shortage(std::size_t i)
{
    if(i && subset[i-1]->data_count > MINIMUM) // Case 1: Transfer an extra entry from subset[i-1]
    {
        array_assist::insert(subset[i]->data, subset[i]->data_count, 0, data[i-1]);
        data[i-1] = subset[i-1]->data[--(subset[i-1]->data_count)];

        if(!(subset[i-1]->is_leaf()))
            array_assist::insert(subset[i]->subset, subset[i]->child_count, 0, subset[i-1]->subset[--(subset[i-1]->child_count)]);

    }
    else if(i+1 < child_count && subset[i+1]->data_count > MINIMUM) // Case 2: Transfer an extra entry from subset[i+1]
    {
        array_assist::insert(subset[i]->data, subset[i]->data_count, subset[i]->data_count, data[i]);
        data[i] = subset[i+1]->data[0];
        array_assist::remove(subset[i+1]->data, subset[i+1]->data_count, 0);

        if(!(subset[i+1]->is_leaf()))
        {
            array_assist::insert(subset[i]->subset, subset[i]->child_count, subset[i]->child_count, subset[i+1]->subset[0]);
            array_assist::remove(subset[i+1]->subset, subset[i+1]->child_count, 0);
        }
    }
    else if(i) // Case 3: Combine subset[i] with subset[i-1]
    {
        array_assist::insert(subset[i-1]->data, subset[i-1]->data_count, subset[i-1]->data_count, data[i-1]);
        array_assist::remove(data, data_count, i-1);

        std::size_t iterations = subset[i]->data_count;
        for(std::size_t j = 0; j < iterations; j++)
        {
            array_assist::insert(subset[i-1]->data, subset[i-1]->data_count, subset[i-1]->data_count, subset[i]->data[0]);
            array_assist::remove(subset[i]->data, subset[i]->data_count, 0);
        }

        iterations = subset[i]->child_count;
        for(std::size_t j = 0; j < iterations; j++)
        {
            array_assist::insert(subset[i-1]->subset, subset[i-1]->child_count, subset[i-1]->child_count, subset[i]->subset[0]);
            array_assist::remove(subset[i]->subset, subset[i]->child_count, 0);
        }

        delete subset[i];
        array_assist::remove(subset, child_count, i);
    }
    else if(i+1 < child_count && i < data_count) // Case 4: Combine subset[i] with subset[i+1]
    {
        array_assist::insert(subset[i+1]->data, subset[i+1]->data_count, 0, data[i]);
        array_assist::remove(data, data_count, i);

        std::size_t iterations = subset[i]->data_count;
        for(std::size_t j = 0; j < iterations; j++)
        {
            array_assist::insert(subset[i+1]->data, subset[i+1]->data_count, 0, subset[i]->data[subset[i]->data_count-1]);
            array_assist::remove(subset[i]->data, subset[i]->data_count, subset[i]->data_count-1);
        }

        iterations = subset[i]->child_count;
        for(std::size_t j = 0; j < iterations; j++)
        {
            array_assist::insert(subset[i+1]->subset, subset[i+1]->child_count, 0, subset[i]->subset[subset[i]->child_count-1]);
            array_assist::remove(subset[i]->subset, subset[i]->child_count, subset[i]->child_count-1);
        }

        delete subset[i];
        array_assist::remove(subset, child_count, i);
    }


}

template <typename T>
void Set<T>::print(std::ostream &out, std::size_t indent) const
{
    out << std::setw(indent) << "";

    for(std::size_t i = 0; i < data_count; i++)
        out << data[i] << " ";
    out << std::endl;

    for(std::size_t i = 0; i < child_count; i++)
        subset[i]->print(out, indent+EXTRA_INDENTATION);
}

#endif // SET_H
