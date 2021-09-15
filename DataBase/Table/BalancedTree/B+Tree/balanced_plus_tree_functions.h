#ifndef BALANCED_PLUS_TREE_FUNCTIONS_H
#define BALANCED_PLUS_TREE_FUNCTIONS_H
#include "../array_assist_functions.h"
#include <iomanip>
#include <iostream>

namespace bpt
{
    // ================================ Constants =================================
    const std::size_t EXTRA_INDENTATION = 4;

    enum node_types
    {
        t_default,
        t_internal,
        t_leaf
    };


    // ============================= Node Definitions =============================
    template <typename T>
    struct b_plus_node
    {
        virtual node_types getType() { return t_default; }
    };

    template <typename T>
    struct internal_node: public b_plus_node<T>
    {
        static const std::size_t MINIMUM = 1;
        static const std::size_t MAXIMUM = MINIMUM*2;

        std::size_t data_count;
        T data[MAXIMUM+1];

        std::size_t child_count;
        b_plus_node<T>* children[MAXIMUM+2];

        internal_node():data_count(0), child_count(0) {}
        node_types getType() { return t_internal; }

    };

    template <typename T>
    struct leaf_node: public b_plus_node<T>
    {
        static const std::size_t MINIMUM = 2;
        static const std::size_t MAXIMUM = 20;//MINIMUM*2;

        std::size_t data_count;
        T data[MAXIMUM+1];

        leaf_node<T>* next_leaf;

        leaf_node():data_count(0), next_leaf(nullptr) {}
        node_types getType() { return t_leaf; }
    };


    // ========================== Functions Declarations ==========================
    template <typename T>
    bool insert(b_plus_node<T>* &node, const T &item);

    template <typename T>
    bool loose_insert(b_plus_node<T>* &node, const T &item);

    template <typename T>
    void print_tree(b_plus_node<T>* const &node, std::ostream &out, std::size_t indent);

    template <typename T>
    b_plus_node<T>* copy(b_plus_node<T>* const &copy_me);

    template <typename T>
    void clear_tree(b_plus_node<T>* clear_me);

    template <typename T>
    T search(b_plus_node<T>* const &root, const T &item);

    template <typename T>
    leaf_node<T>* find_node(b_plus_node<T>* const &root, const T &item, int &pos_found);


    // ========================== Functions Definitions ==========================
    template <typename T>
    bool insert(b_plus_node<T>* &node, const T &item)
    {
        if(!loose_insert(node, item))
            return false;

        switch(node->getType())
        {
        case t_internal:
            if(reinterpret_cast<internal_node<T>*>(node)->data_count > internal_node<T>::MAXIMUM)
            {
                internal_node<T>* sub_root = new internal_node<T>();

                array_assist::copy(reinterpret_cast<internal_node<T>*>(node)->data, reinterpret_cast<internal_node<T>*>(node)->data_count, sub_root->data);
                sub_root->data_count = reinterpret_cast<internal_node<T>*>(node)->data_count;
                reinterpret_cast<internal_node<T>*>(node)->data_count = 0;

                array_assist::copy(reinterpret_cast<internal_node<T>*>(node)->children, reinterpret_cast<internal_node<T>*>(node)->child_count, sub_root->children);
                sub_root->child_count = reinterpret_cast<internal_node<T>*>(node)->child_count;
                reinterpret_cast<internal_node<T>*>(node)->child_count = 1;
                reinterpret_cast<internal_node<T>*>(node)->children[0] = sub_root;

                fix_excess(node, 0);
            }
            break;
        case t_leaf:
            if(reinterpret_cast<leaf_node<T>*>(node)->data_count > leaf_node<T>::MAXIMUM)
            {
                leaf_node<T>* right_child = new leaf_node<T>();

                array_assist::copy(reinterpret_cast<leaf_node<T>*>(node)->data+(reinterpret_cast<internal_node<T>*>(node)->data_count/2), reinterpret_cast<internal_node<T>*>(node)->data_count - reinterpret_cast<internal_node<T>*>(node)->data_count/2, right_child->data);
                right_child->data_count = reinterpret_cast<internal_node<T>*>(node)->data_count - (reinterpret_cast<internal_node<T>*>(node)->data_count/2);
                reinterpret_cast<internal_node<T>*>(node)->data_count -= reinterpret_cast<internal_node<T>*>(node)->data_count/2;

                reinterpret_cast<leaf_node<T>*>(node)->next_leaf = right_child;

                internal_node<T>* new_root = new internal_node<T>();
                new_root->data[0] = right_child->data[0];
                new_root->data_count = 1;

                new_root->children[0] = node;
                new_root->children[1] = right_child;
                new_root->child_count = 2;

                node = new_root;
            }
            break;
        default:
            return false;
        }

        return true;
    }

    template <typename T>
    bool loose_insert(b_plus_node<T>* &node, const T &item)
    {
        //if(i < reinterpret_cast<internal_node<T>*>(node)->data_count && !(item < reinterpret_cast<internal_node<T>*>(node)->data[i]))
            //return false;

        if(node->getType() == t_leaf)
        {
            leaf_node<T>* l_node = reinterpret_cast<leaf_node<T>*>(node);
            std::size_t i = (l_node->data_count == 0) ? 0 : array_assist::get_binary_sorted_index(l_node->data, item, 0, l_node->data_count-1);
            array_assist::insert(l_node->data, l_node->data_count, i, item);
            return true;
        }

        internal_node<T>* i_node = reinterpret_cast<internal_node<T>*>(node);

        std::size_t i = (i_node->data_count == 0) ? 0 : array_assist::get_binary_sorted_index(i_node->data, item, 0, i_node->data_count-1);
        bool temp = loose_insert(reinterpret_cast<internal_node<T>*>(node)->children[i], item);

        std::size_t data_max, data_count;
        if(i_node->children[i]->getType() == t_internal)
        {
            data_max = internal_node<T>::MAXIMUM;
            data_count = reinterpret_cast<internal_node<T>*>(i_node->children[i])->data_count;
        }
        else
        {
            data_max = leaf_node<T>::MAXIMUM;
            data_count = reinterpret_cast<leaf_node<T>*>(i_node->children[i])->data_count;
        }

        if(data_count > data_max)
            fix_excess(node, i);

        return temp;
    }

    template <typename T>
    void fix_excess(b_plus_node<T>* &node, std::size_t i)
    {

        if(node->getType() == t_internal)
        {
            internal_node<T>* i_node = reinterpret_cast<internal_node<T>*>(node);

            if(i_node->children[i]->getType() == t_internal)
            {
                internal_node<T>* sub_node = reinterpret_cast<internal_node<T>*>(i_node->children[i]);
                internal_node<T>* new_sub = new internal_node<T>();
                std::size_t pos = sub_node->data_count/2 + 1;

                array_assist::split(sub_node->children, sub_node->child_count, pos, new_sub->children, new_sub->child_count);
                array_assist::split(sub_node->data, sub_node->data_count, pos, new_sub->data, new_sub->data_count);

                T middle = sub_node->data[--sub_node->data_count];
                std::size_t j = (i_node->data_count == 0) ? 0 : array_assist::get_binary_sorted_index(i_node->data, middle, 0, i_node->data_count-1);

                array_assist::insert(i_node->data, i_node->data_count, j, middle);
                array_assist::insert(i_node->children, i_node->child_count, j+1, reinterpret_cast<b_plus_node<T>*>(new_sub));
            }
            else if(i_node->children[i]->getType() == t_leaf)
            {
                leaf_node<T>* sub_node = reinterpret_cast<leaf_node<T>*>(i_node->children[i]);
                leaf_node<T>* new_sub = new leaf_node<T>();
                new_sub->next_leaf = sub_node->next_leaf;
                sub_node->next_leaf = new_sub;
                std::size_t pos = sub_node->data_count/2;

                array_assist::split(sub_node->data, sub_node->data_count, pos, new_sub->data, new_sub->data_count);

                T middle = new_sub->data[0];
                std::size_t j = (i_node->data_count == 0) ? 0 : array_assist::get_binary_sorted_index(i_node->data, middle, 0, i_node->data_count-1);

                array_assist::insert(i_node->data, i_node->data_count, j, middle);
                array_assist::insert(i_node->children, i_node->child_count, j+1, reinterpret_cast<b_plus_node<T>*>(new_sub));

            }
        }
    }

    template <typename T>
    void print_tree(b_plus_node<T>* const &node, std::ostream &out, std::size_t indent)
    {
        out << std::setw(indent) << "";

        if(node->getType() == t_internal)
        {
            internal_node<T>* i_node = reinterpret_cast<internal_node<T>*>(node);

            for(std::size_t i = 0; i < i_node->data_count; i++)
                out << i_node->data[i] << " ";
            out << std::endl;

            for(std::size_t i = 0; i < i_node->child_count; i++)
                print_tree(i_node->children[i], out, indent + EXTRA_INDENTATION);
        }
        else if(node->getType() == t_leaf)
        {
            leaf_node<T>* l_node = reinterpret_cast<leaf_node<T>*>(node);

            out << l_node << ": ";

            for(std::size_t i = 0; i < l_node->data_count; i++)
                out << l_node->data[i] << " ";
            out << "-> " << l_node->next_leaf << std::endl;
        }
    }

    // Doesn't clear the root
    template <typename T>
    void clear_tree(b_plus_node<T>* clear_me)
    {
        if(clear_me->getType() == t_leaf)
            return;

        internal_node<T>* i_node = reinterpret_cast<internal_node<T>*>(clear_me);

        for(int i = 0; i < i_node->child_count; i++)
        {
            clear_tree(i_node->children[i]);
            delete i_node->children[i];
        }
    }

    template <typename T>
    b_plus_node<T>* copy(b_plus_node<T>* const &copy_me)
    {
        if(copy_me->getType() == t_leaf)
        {
            leaf_node<T>* node_copy = new leaf_node<T>(),
                          l_node = reinterpret_cast<leaf_node<T>*>(copy_me);

            array_assist::copy(l_node.data, l_node.data_count, node_copy->data);
            node_copy->data_count = l_node.data;

            return node_copy;
        }
        else if(copy_me->getType() == t_internal)
        {
            internal_node<T>* node_copy = new internal_node<T>(),
                              i_node = reinterpret_cast<internal_node<T>*>(copy_me);

            array_assist::copy(i_node.data, i_node.data_count, node_copy->data);
            node_copy->data_count = i_node.data_count;

            for(int i = 0; i < i_node.child_count; i++)
                node_copy->children[i] = copy(i_node.children[i]);
            node_copy->child_count = i_node.child_count;

            return node_copy;
        }

        return nullptr;
    }

    template <typename T>
    T* search(b_plus_node<T>* const &root, const T &item)
    {
        if(root->getType() == t_leaf)
        {
            leaf_node<T>* l_root = reinterpret_cast<leaf_node<T>*>(root);
            int i = array_assist::binary_search(l_root->data, item, 0, l_root->data_count-1);

            if(i != -1)
                return &(l_root->data[i]);
        }
        else if(root->getType() == t_internal)
        {
            internal_node<T>* i_root = reinterpret_cast<internal_node<T>*>(root);

            if(i_root->data_count == 0)
                return nullptr;

            int i = array_assist::get_binary_sorted_index(i_root->data, item, 0, i_root->data_count-1);

            return search(i_root->children[i], item);
        }

        return nullptr;
    }

    template <typename T>
    leaf_node<T>* find_node(b_plus_node<T>* const &root, const T &item, int &pos_found)
    {
        if(root->getType() == t_leaf)
        {
            leaf_node<T>* l_root = reinterpret_cast<leaf_node<T>*>(root);
            int i = array_assist::binary_search(l_root->data, item, 0, l_root->data_count-1);

            pos_found = i;

            if(i != -1)
                return l_root;
        }
        else if(root->getType() == t_internal)
        {
            internal_node<T>* i_root = reinterpret_cast<internal_node<T>*>(root);

            if(i_root->data_count == 0)
                return nullptr;

            int i = array_assist::get_binary_sorted_index(i_root->data, item, 0, i_root->data_count-1);

            return find_node(i_root->children[i], item, pos_found);
        }

        return nullptr;
    }
}
#endif // BALANCED_PLUS_TREE_FUNCTIONS_H
