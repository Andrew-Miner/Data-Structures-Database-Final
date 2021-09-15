#ifndef BPLUSTREE_H
#define BPLUSTREE_H
#include "balanced_plus_tree_functions.h"
#include <vector>


template <typename T>
class BPlusTree
{
public:
    BPlusTree();
    BPlusTree(const BPlusTree<T>& other);
    BPlusTree& operator=(const BPlusTree<T>& other);
    ~BPlusTree();

    void insert(const T& item);
    T* find(const T& item);
    std::vector<T> getRange(const T& low_end, const T& high_end);
    void clear();

    friend std::ostream& operator<<(std::ostream &out, const BPlusTree<T>& tree)
    {
        bpt::print_tree(tree.root, out, 0);
        return out;
    }

private:
    bpt::b_plus_node<T>* root;
};

template <typename T>
BPlusTree<T>::BPlusTree()
{
    root = new bpt::leaf_node<T>();
}

template <typename T>
BPlusTree<T>::BPlusTree(const BPlusTree<T>& other)
{
    root = bpt::copy(other.root);
}

template <typename T>
BPlusTree<T>& BPlusTree<T>::operator=(const BPlusTree<T>& other)
{
    root = bpt::copy(other.root);
    return *this;
}

template <typename T>
BPlusTree<T>::~BPlusTree()
{
    bpt::clear_tree(root);
    if(root->getType() == bpt::t_internal)
        delete reinterpret_cast<bpt::internal_node<T>*>(root);
    else
        delete reinterpret_cast<bpt::leaf_node<T>*>(root);
}

template <typename T>
void BPlusTree<T>::insert(const T& item)
{
    bpt::insert(root, item);
}

template <typename T>
T* BPlusTree<T>::find(const T& item)
{
    return bpt::search(root, item);
}

template <typename T>
std::vector<T> BPlusTree<T>::getRange(const T& low_end, const T& high_end)
{
    int pos, end_pos;
    bpt::leaf_node<T> *node_walker = bpt::find_node(root, low_end, pos),
                      *final_node = bpt::find_node(root, high_end, end_pos);

    if(!node_walker)
        return std::vector<T>();

    std::vector<T> result;

    while(node_walker && node_walker != final_node)
    {
        for(int i = pos; i < node_walker->data_count; i++)
            result.push_back(node_walker->data[i]);
        pos = 0;
        node_walker = node_walker->next_leaf;
    }

    if(node_walker)
    {
        for(int i = pos; i < end_pos; i++)
            result.push_back(node_walker->data[i]);
    }

    return result;
}

template <typename T>
void BPlusTree<T>::clear()
{
    bpt::clear_tree(root);
    delete root;
    root = new bpt::leaf_node<T>();
}

#endif // BPLUSTREE_H
