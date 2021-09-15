#ifndef BPLUS_DISK_FUNCTIONS_H
#define BPLUS_DISK_FUNCTIONS_H

#include "../array_assist_functions.h"
#include "../../../AssistFunctions/file_assist_functions.h"
#include <iomanip>
#include <iostream>
#include <fstream>

namespace dbpt
{

    // ================================ Constants =================================
    const std::size_t EXTRA_INDENTATION = 4;

    enum node_types
    {
        t_default,
        t_internal,
        t_leaf
    };

    // ================================ Functors =================================
    template <typename T>
    class DefaultRepInsert
    {
    public:
        DefaultRepInsert() {}
        bool operator()(T &existing_item, const T &new_item)
        {
            return true;
        }
    };

    // ============================= Node Definitions =============================
    template <typename T>
    struct b_plus_node
    {
        bool isLeaf;
        b_plus_node(bool leaf = false):isLeaf(leaf) {}
    };

    template <typename T>
    struct internal_node: public b_plus_node<T>
    {
        static const std::size_t MINIMUM = 1;
        static const std::size_t MAXIMUM = MINIMUM*2;

        std::size_t data_count;
        T data[MAXIMUM+1];

        std::size_t child_count;
        std::streampos children[MAXIMUM+2];

        internal_node():b_plus_node<T>(false), data_count(0), child_count(0) {}
        internal_node(std::fstream &file, std::streampos pos):b_plus_node<T>(true)
        {
            loadInternalNode(this, file, pos);
        }

        node_types getType() { return t_internal; }
    };

    template <typename T>
    struct leaf_node: public b_plus_node<T>
    {
        static const std::size_t MINIMUM = 2;

        // This number determines the size of each leaf
        static const std::size_t MAXIMUM = 20;

        std::size_t data_count;
        T data[MAXIMUM+1];

        std::streampos next_leaf;

        leaf_node():b_plus_node<T>(true), data_count(0), next_leaf(0) {}
        leaf_node(std::fstream &file, std::streampos pos):b_plus_node<T>(true), next_leaf(0), data_count(0)
        {
            loadLeafNode(this,file, pos);
        }


        node_types getType() { return t_leaf; }
    };

    // ========================== Function Declarations ==========================
    template <typename T, class RepInsert>
    bool insert(std::fstream &tree_file, std::streampos &node, const T &item, RepInsert &Rep);

    template <typename T, class RepInsert>
    bool loose_insert(std::fstream &tree_file, std::streampos &node, const T &item, RepInsert &Rep);

    template <typename T>
    void fix_excess(std::fstream &file, b_plus_node<T>* &parent_node, b_plus_node<T>* child_node);

    template <typename T>
    void print_tree(std::streampos &node, std::ostream &out, std::size_t indent);

    template <typename T>
    void clear_tree(std::streampos clear_me);

    template <typename T>
    T search(std::streampos const &root, const T &item);

    template <typename T>
    leaf_node<T>* find_node(std::fstream &file, std::streampos &root_pos, const T &item, int &pos_found, std::streampos &leaf_pos);

    // ========================== File Function Declarations ==========================
    template <typename T>
    bool is_leaf(std::fstream &file, std::streampos pos);

    template <typename T>
    void loadInternalNode(internal_node<T>* &node, std::fstream &file, std::streampos pos);

    template <typename T>
    void storeInternalNode(internal_node<T>* node, std::fstream &file, std::streampos pos);

    template <typename T>
    void loadLeafNode(leaf_node<T>* &node, std::fstream &file, std::streampos pos);

    template <typename T>
    void storeLeafNode(leaf_node<T>* node, std::fstream &file, std::streampos pos);

    template <typename T>
    b_plus_node<T>* loadNode(std::fstream &file, std::streampos pos);

    template <typename T>
    std::pair<std::streampos, leaf_node<T>*> createLeafNode(std::fstream &file);

    template <typename T>
    std::pair<std::streampos, internal_node<T>*> createInternalNode(std::fstream &file);

    template <typename T>
    void update_root(std::fstream &file, std::streampos &new_root);

    template <typename T>
    void update_leaf_start(std::fstream &file, std::streampos &leaf_start);

    // ========================== File Function Definitions ==========================
    template <typename T>
    b_plus_node<T>* loadNode(std::fstream &file, std::streampos pos)
    {
        b_plus_node<T>* mem_node;

        if (is_leaf<T>(file, pos))
        {
            leaf_node<T>* leaf = new leaf_node<T>();
            loadLeafNode<T>(leaf, file, pos);
            mem_node = reinterpret_cast<b_plus_node<T>*>(leaf);
        }
        else
        {
            internal_node<T>* internal = new internal_node<T>();
            loadInternalNode<T>(internal, file, pos);
            mem_node = reinterpret_cast<b_plus_node<T>*>(internal);
        }

        return mem_node;
    }

    template <typename T>
    bool is_leaf(std::fstream &file, std::streampos pos)
    {
        bool returnMe;
        file.seekg(pos, std::ios_base::beg);
        file.read(reinterpret_cast<char*>(&returnMe), sizeof(bool));
        return returnMe;
    }

    template <typename T>
    std::pair<std::streampos, leaf_node<T>*> createLeafNode(std::fstream &file)
    {
        std::pair<std::streampos, leaf_node<T>*> pair;
        pair.second = new leaf_node<T>();
        pair.first = p_goto_end(file);

        file.write(reinterpret_cast<char*>(pair.second), sizeof(leaf_node<T>));
        file.flush();
        return pair;
    }

    template <typename T>
    std::pair<std::streampos, internal_node<T>*> createInternalNode(std::fstream &file)
    {
        std::pair<std::streampos, internal_node<T>*> pair;
        pair.second = new internal_node<T>();
        pair.first = p_goto_end(file);

        file.write(reinterpret_cast<char*>(pair.second), sizeof(internal_node<T>));
        file.flush();
        return pair;
    }

    template <typename T>
    void loadInternalNode(internal_node<T>* &node, std::fstream &file, std::streampos pos)
    {
        file.seekg(pos, std::ios_base::beg);
        file.read(reinterpret_cast<char*>(node), sizeof(internal_node<T>));
    }

    template <typename T>
    void storeInternalNode(internal_node<T>* node, std::fstream &file, std::streampos pos)
    {
        file.seekp(pos, std::ios_base::beg);
        file.write(reinterpret_cast<char*>(node), sizeof(internal_node<T>));
        file.flush();
    }

    template <typename T>
    void loadLeafNode(leaf_node<T>* &node, std::fstream &file, std::streampos pos)
    {
        file.seekg(pos, std::ios_base::beg);
        file.read(reinterpret_cast<char*>(node), sizeof(leaf_node<T>));
    }

    template <typename T>
    void storeLeafNode(leaf_node<T>* node, std::fstream &file, std::streampos pos)
    {
        file.seekp(pos, std::ios_base::beg);
        file.write(reinterpret_cast<char*>(node), sizeof(leaf_node<T>));
        file.flush();
    }

    template <typename T>
    void update_root(std::fstream &file, std::streampos &new_root)
    {
        file.seekp(0, std::ios_base::beg);
        file.write(reinterpret_cast<char*>(&new_root), sizeof(std::streampos));
        file.flush();
    }

    template <typename T>
    void update_leaf_start(std::fstream &file, std::streampos &leaf_start)
    {
        file.seekp(sizeof(std::streampos), std::ios_base::beg);
        file.write(reinterpret_cast<char*>(&leaf_start), sizeof(std::streampos));
        file.flush();
    }

    // ========================== Function Definitions ==========================
    template <typename T, class RepInsert>
    bool insert(std::fstream &tree_file, std::streampos &node, const T &item, RepInsert &Rep)
    {

        if(!loose_insert(tree_file, node, item, Rep))
            return false;

        b_plus_node<T>* mem_node = loadNode<T>(tree_file, node);

        if(mem_node->isLeaf)
        {
            leaf_node<T>* l_node = reinterpret_cast<leaf_node<T>*>(mem_node);

            if(l_node->data_count > leaf_node<T>::MAXIMUM)
            {
                std::pair<std::streampos, leaf_node<T>*> right_child = createLeafNode<T>(tree_file);

                array_assist::copy(l_node->data+(l_node->data_count/2), l_node->data_count - l_node->data_count/2, right_child.second->data);
                right_child.second->data_count = l_node->data_count - (l_node->data_count/2);
                l_node->data_count -= l_node->data_count/2;

                l_node->next_leaf = right_child.first;

                std::pair<std::streampos, internal_node<T>*> new_root = createInternalNode<T>(tree_file);
                new_root.second->data[0] = right_child.second->data[0];
                new_root.second->data_count = 1;

                new_root.second->children[0] = node;
                new_root.second->children[1] = right_child.first;
                new_root.second->child_count = 2;

                storeInternalNode(new_root.second, tree_file, new_root.first);
                storeLeafNode(right_child.second, tree_file, right_child.first);
                storeLeafNode(l_node, tree_file, node);

                node = new_root.first;
                update_root<T>(tree_file, node);

                delete right_child.second;
                delete new_root.second;
            }
        }
        else
        {
            internal_node<T>* i_node = reinterpret_cast<internal_node<T>*>(mem_node);

            if(i_node->data_count > internal_node<T>::MAXIMUM)
            {
                std::pair<std::streampos, internal_node<T>*> sub_root = createInternalNode<T>(tree_file);

                array_assist::copy(i_node->data, i_node->data_count, sub_root.second->data);
                sub_root.second->data_count = i_node->data_count;
                i_node->data_count = 0;

                array_assist::copy(i_node->children, i_node->child_count, sub_root.second->children);
                sub_root.second->child_count = i_node->child_count;
                i_node->child_count = 1;
                i_node->children[0] = sub_root.first;

                fix_excess<T>(tree_file, mem_node, reinterpret_cast<b_plus_node<T>*>(sub_root.second));

                storeInternalNode(i_node, tree_file, node);
                storeInternalNode(sub_root.second, tree_file, sub_root.first);

                delete sub_root.second;
            }
        }

        delete mem_node;
        return true;
    }

    template <typename T, class RepInsert>
    bool loose_insert(std::fstream &tree_file, std::streampos &node, const T &item, RepInsert &rep_policy)
    {
        b_plus_node<T>* mem_node = loadNode<T>(tree_file, node);
        if(mem_node->isLeaf)
        {
            leaf_node<T>* l_node = reinterpret_cast<leaf_node<T>*>(mem_node);
            std::size_t i = (l_node->data_count == 0) ? 0 : array_assist::get_binary_sorted_index(l_node->data, item, 0, l_node->data_count-1);

            // If item is already in the Tree
            if(i < l_node->data_count && !(item < l_node->data[i]))
            {
                // If Functor Returns True (RepetitionInsert = true)
                // This if will fail and the item will be inserted again
                // If Functor Return False, l_node->data[i] can be manipulated
                // And Item Wont Be Inserted Again
                if(!rep_policy(l_node->data[i], item))
                {
                    storeLeafNode(l_node, tree_file, node);
                    delete mem_node;
                    return false;
                }
            }

            array_assist::insert(l_node->data, l_node->data_count, i, item);
            storeLeafNode(l_node, tree_file, node);
            delete mem_node;
            return true;
        }

        internal_node<T>* i_node = reinterpret_cast<internal_node<T>*>(mem_node);

        std::size_t i = (i_node->data_count == 0) ? 0 : array_assist::get_binary_sorted_index(i_node->data, item, 0, i_node->data_count-1);
        bool temp = loose_insert(tree_file, i_node->children[i], item, rep_policy);

        b_plus_node<T>* mem_child = loadNode<T>(tree_file, i_node->children[i]);
        std::size_t data_max, data_count;

        if(mem_child->isLeaf)
        {
            data_max = leaf_node<T>::MAXIMUM;
            data_count = reinterpret_cast<leaf_node<T>*>(mem_child)->data_count;
        }
        else
        {
            data_max = internal_node<T>::MAXIMUM;
            data_count = reinterpret_cast<internal_node<T>*>(mem_child)->data_count;
        }

        if(data_count > data_max)
            fix_excess(tree_file, mem_node, mem_child);

        storeInternalNode(i_node, tree_file, node);

        if(mem_child->isLeaf)
            storeLeafNode(reinterpret_cast<leaf_node<T>*>(mem_child), tree_file, i_node->children[i]);
        else
            storeInternalNode(reinterpret_cast<internal_node<T>*>(mem_child), tree_file, i_node->children[i]);

        delete mem_node;
        delete mem_child;

        return temp;
    }

    template <typename T>
    void fix_excess(std::fstream &tree_file, b_plus_node<T>* &parent_node, b_plus_node<T>* child_node)
    {

        if(!(parent_node->isLeaf))
        {
            internal_node<T>* i_node = reinterpret_cast<internal_node<T>*>(parent_node);

            if(!(child_node->isLeaf))
            {
                internal_node<T>* sub_node = reinterpret_cast<internal_node<T>*>(child_node);
                std::pair<std::streampos, internal_node<T>*> new_sub = createInternalNode<T>(tree_file);
                std::size_t pos = sub_node->data_count/2 + 1;

                array_assist::split(sub_node->children, sub_node->child_count, pos, new_sub.second->children, new_sub.second->child_count);
                array_assist::split(sub_node->data, sub_node->data_count, pos, new_sub.second->data, new_sub.second->data_count);

                T middle = sub_node->data[--sub_node->data_count];
                std::size_t j = (i_node->data_count == 0) ? 0 : array_assist::get_binary_sorted_index(i_node->data, middle, 0, i_node->data_count-1);

                array_assist::insert(i_node->data, i_node->data_count, j, middle);
                array_assist::insert(i_node->children, i_node->child_count, j+1, new_sub.first);

                storeInternalNode(new_sub.second, tree_file, new_sub.first);
            }
            else
            {
                leaf_node<T>* sub_node = reinterpret_cast<leaf_node<T>*>(child_node);
                std::pair<std::streampos, leaf_node<T>*> new_sub = createLeafNode<T>(tree_file);
                new_sub.second->next_leaf = sub_node->next_leaf;
                sub_node->next_leaf = new_sub.first;
                std::size_t pos = sub_node->data_count/2;

                array_assist::split(sub_node->data, sub_node->data_count, pos, new_sub.second->data, new_sub.second->data_count);

                T middle = new_sub.second->data[0];
                std::size_t j = (i_node->data_count == 0) ? 0 : array_assist::get_binary_sorted_index(i_node->data, middle, 0, i_node->data_count-1);

                array_assist::insert(i_node->data, i_node->data_count, j, middle);
                array_assist::insert(i_node->children, i_node->child_count, j+1, new_sub.first);

                storeLeafNode(new_sub.second, tree_file, new_sub.first);
            }
        }
    }

    template <typename T>
    void print_tree(std::fstream &file, const std::streampos &node_pos, std::ostream &out, std::size_t indent)
    {
        out << std::setw(indent) << "";

        b_plus_node<T>* node = loadNode<T>(file, node_pos);

        if(!(node->isLeaf))
        {
            internal_node<T>* i_node = reinterpret_cast<internal_node<T>*>(node);

            for(std::size_t i = 0; i < i_node->data_count; i++)
                out << i_node->data[i] << " ";
            out << std::endl;

            std::streampos *children = new std::streampos[i_node->child_count];
            array_assist::copy(i_node->children, i_node->child_count, children);
            std::size_t count = i_node->child_count;

            delete node;
            node = nullptr;

            for(std::size_t i = 0; i < count; i++)
                print_tree<T>(file, children[i], out, indent + EXTRA_INDENTATION);
            delete [] children;
        }
        else if(node->isLeaf)
        {
            leaf_node<T>* l_node = reinterpret_cast<leaf_node<T>*>(node);

            out << node_pos << ": ";

            for(std::size_t i = 0; i < l_node->data_count; i++)
                out << l_node->data[i] << " ";
            out << "-> " << l_node->next_leaf << std::endl;
        }

        if(node)
            delete node;
    }

    template <typename T>
    T* search(std::fstream &file, std::streampos &root_pos, const T &item)
    {
        b_plus_node<T>* root = loadNode<T>(file, root_pos);

        if(root->isLeaf)
        {
            leaf_node<T>* l_root = reinterpret_cast<leaf_node<T>*>(root);
            int i = array_assist::binary_search(l_root->data, item, 0, l_root->data_count-1);

            if(i != -1)
            {
                T* return_t = new T(l_root->data[i]);
                delete root;
                return return_t;
            }
        }
        else
        {
            internal_node<T>* i_root = reinterpret_cast<internal_node<T>*>(root);

            if(i_root->data_count == 0)
            {
                delete root;
                return nullptr;
            }

            int i = array_assist::get_binary_sorted_index(i_root->data, item, 0, i_root->data_count-1);

            std::streampos i_pos = i_root->children[i];
            delete root;

            return search(file, i_pos, item);
        }

        delete root;
        return nullptr;
    }

    template <typename T>
    leaf_node<T>* find_node(std::fstream &file, std::streampos &root_pos, const T &item, int &pos_found, std::streampos &leaf_pos)
    {
        b_plus_node<T>* root = loadNode<T>(file, root_pos);

        if(root->isLeaf)
        {
            leaf_node<T>* l_root = reinterpret_cast<leaf_node<T>*>(root);
            int i = array_assist::binary_search(l_root->data, item, 0, l_root->data_count-1);

            pos_found = i;

            if(i != -1)
            {
                leaf_pos = root_pos;
                return l_root;
            }

            leaf_pos = -1;
        }
        else
        {
            internal_node<T>* i_root = reinterpret_cast<internal_node<T>*>(root);

            if(i_root->data_count == 0)
            {
                delete root;
                return nullptr;
            }

            int i = array_assist::get_binary_sorted_index(i_root->data, item, 0, i_root->data_count-1);

            std::streampos child_pos = i_root->children[i];
            delete root;

            return find_node(file, child_pos, item, pos_found, leaf_pos);
        }

        delete root;
        return nullptr;
    }


    // =========================== NON- Implemented B+Tree Functions ===========================
    //    // Doesn't clear the root
    //    template <typename T>
    //    void clear_tree(b_plus_node<T>* clear_me)
    //    {
    //        if(clear_me->getType() == t_leaf)
    //            return;

    //        internal_node<T>* i_node = reinterpret_cast<internal_node<T>*>(clear_me);

    //        for(int i = 0; i < i_node->child_count; i++)
    //        {
    //            clear_tree(i_node->children[i]);
    //            delete i_node->children[i];
    //        }
    //    }

    //    template <typename T>
    //    b_plus_node<T>* copy(b_plus_node<T>* const &copy_me)
    //    {
    //        if(copy_me->getType() == t_leaf)
    //        {
    //            leaf_node<T>* node_copy = new leaf_node<T>(),
    //                          l_node = reinterpret_cast<leaf_node<T>*>(copy_me);

    //            array_assist::copy(l_node.data, l_node.data_count, node_copy->data);
    //            node_copy->data_count = l_node.data;

    //            return node_copy;
    //        }
    //        else if(copy_me->getType() == t_internal)
    //        {
    //            internal_node<T>* node_copy = new internal_node<T>(),
    //                              i_node = reinterpret_cast<internal_node<T>*>(copy_me);

    //            array_assist::copy(i_node.data, i_node.data_count, node_copy->data);
    //            node_copy->data_count = i_node.data_count;

    //            for(int i = 0; i < i_node.child_count; i++)
    //                node_copy->children[i] = copy(i_node.children[i]);
    //            node_copy->child_count = i_node.child_count;

    //            return node_copy;
    //        }

    //        return nullptr;
    //    }
}

#endif // BPLUS_DISK_FUNCTIONS_H
