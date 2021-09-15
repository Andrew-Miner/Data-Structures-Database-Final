#ifndef DISK_BPLUSTREE_H
#define DISK_BPLUSTREE_H
#include "disk_bplus_functions.h"
#include <vector>

template <typename T, class RepInsert = dbpt::DefaultRepInsert<T>>
class DPlusTree
{
public:
    class iterator
    {
    public:
        friend class DPlusTree;
        iterator():file_ptr(nullptr), cur_pos(0), leaf_pos(0), leaf(nullptr) { }
        iterator(std::fstream *file, std::streampos leaf_pos);
        iterator(std::fstream *file, std::streampos leaf_pos, std::size_t cur_pos);
        iterator(const typename DPlusTree<T, RepInsert>::iterator &other);
        ~iterator();

        iterator& operator=(const iterator &other);

        iterator operator++(int);
        iterator& operator++();

        T operator*();

        bool operator!=(const typename DPlusTree<T, RepInsert>::iterator& itr);
        bool operator==(const typename DPlusTree<T, RepInsert>::iterator& lhs);

    private:
        std::streampos leaf_pos;
        std::size_t cur_pos;
        dbpt::leaf_node<T>* leaf;
        std::fstream *file_ptr;

    };

    /* ============================================================
     * Name: Default Constructor
     * Function: Contructs An Invalid/Uninitialized DPlusTree
     */
    DPlusTree():file(nullptr) {}

    /* ============================================================
     * Name: Normal Constructor
     * Function: Loads/Creates DPlusTree named file_name
     * Pre: file_name includes the directory path and name of tree
     * Post: Constructs DPlusTree File/Object
     * Parameters: file_name - Directory + Tree Name
     *             clear_existing_file - If set file_name will be
     *                                   overwritten
     */
    DPlusTree(const std::string file_name, bool clear_existing_file = false);

    /* ============================================================
     * Name: Copy Constructor - NOT IMPLEMENTED
     * TODO: Implement this function
     */
    DPlusTree(const DPlusTree<T, RepInsert>& other);

    /* ============================================================
     * Name: Equals Operator - NOT IMPLEMENTED
     * TODO: Implement this function
     */
    DPlusTree& operator=(const DPlusTree<T, RepInsert>& other);

    /* ============================================================
     * Name: Destructor
     * Function: Closes DPlusTree Binary File
     */
    ~DPlusTree();

    /* ============================================================
     * Name: Insert
     * Function: Insert Item Into DPlusTree
     * Pre: none
     * Post: If Item Is Unique It Is Insert. Else RepInsert Policy
     *       is followed.
     * Parameters: item - item to insert
     */
    void insert(const T& item);

    /* ============================================================
     * Name: Find
     * Function: Find Item And Return Iterator To It
     * Pre: none
     * Post: iterator either points to item or is DplusTree.end()
     * Parameters: item - item to locate
     */
    typename DPlusTree<T, RepInsert>::iterator find(const T& item);

    /* ============================================================
     * Name: Find - Depricated
     * Function: Returns True if item is found and sets found_item
     *           to item
     * Pre: none
     * Post: found_item is set to item if it was found
     * Parameters: item - item to locate
     */
    bool find(const T& item, T &found_item);

    /* ============================================================
     * Name: getRange - NOT IMPLEMENTED
     * Function: Returns a vector of items within range
     * Pre: none
     * Post: If no items were found returns empty vector, Else
     *       returns items in range
     * Parameters: low_end - low end of range
     *             high_end - high end of range
     * TODO: Implement This Function. It Should Be Easy Utilizing
     *       Iterators!
     */
    std::vector<T> getRange(const T& low_end, const T& high_end);

    /* ============================================================
     * Name: Clear
     * Function: Clears The DPlusTree on disk
     */
    void clear();

    /* ============================================================
     * Name: Is Open
     * Function: Returns True if file is open, false otherwise.
     */
    bool is_open() { return file; }


    /* ============================================================
     * Name: Begin
     * Function: Returns Iterator Pointing To Start Of Tree
     */
    typename DPlusTree<T, RepInsert>::iterator begin() { return DPlusTree<T, RepInsert>::iterator(file, list_start); }

    /* ============================================================
     * Name: Begin
     * Function: Returns Iterator Pointing To End Of Tree
     */
    typename DPlusTree<T, RepInsert>::iterator end() { return DPlusTree<T, RepInsert>::iterator(); }

    /* ============================================================
     * Name: Operator <<
     * Function: Prints Tree To ostream out
     */
    friend std::ostream& operator<<(std::ostream &out, const DPlusTree<T, RepInsert>& tree)
    {
        if(tree.file)
            dbpt::print_tree<T>(*(tree.file), tree.root, out, 0);
        else
            out << "DPlusTree Failed To Load From File!";
        return out;
    }

private:
    std::streampos root, list_start;
    std::fstream* file;
    std::string file_name;

    /* ============================================================
     * Name: Open File
     * Function: Opens File file_name
     * Pre: file_name contains directory and name of existing file
     * Post: File file_name is loaded
     * Parameters: file_name - Directory + file_name
     */
    bool open_file(const std::string& file_name);

    /* ============================================================
     * Name: Create File
     * Function: Creates New File file_name
     * Pre: file_name contains directory and name of file
     * Post: File file_name is created
     * Parameters: file_name - Directory + file_name
     */
    bool create_file(const std::string& file_name);

    /* ============================================================
     * Name: Create New File
     * Function: Creates New File file_name And Overwrites Any
     *           Previously Existing File file_name
     * Pre: file_name contains directory and name of file
     * Post: File file_name is created/cleared
     * Parameters: file_name - Directory + file_name
     */
    bool create_new_file(const std::string& file_name);

    /* ============================================================
     * Name: Load Tree
     * Function: Load DPlusTree from file
     */
    void load_tree();

    /* ============================================================
     * Name: Create Tree
     * Function: Create DPlusTree file/object
     */
    void create_tree();
};

// ======================== Iterator Functions ============================

template <typename T, class RepInsert>
DPlusTree<T, RepInsert>::iterator::iterator(std::fstream *file, std::streampos leaf_pos): cur_pos(0)
{
    file_ptr = file;
    this->leaf_pos = leaf_pos;

    leaf = new dbpt::leaf_node<T>();
    dbpt::loadLeafNode(leaf, *file_ptr, this->leaf_pos);
}

template <typename T, class RepInsert>
DPlusTree<T, RepInsert>::iterator::iterator(std::fstream *file, std::streampos leaf_pos, std::size_t cur_pos):  file_ptr(file), leaf_pos(leaf_pos), cur_pos(cur_pos)
{
    leaf = new dbpt::leaf_node<T>();
    dbpt::loadLeafNode(leaf, *file_ptr, this->leaf_pos);
}

template <typename T, class RepInsert>
DPlusTree<T, RepInsert>::iterator::iterator(const typename DPlusTree<T, RepInsert>::iterator &other)
{
    file_ptr = other.file_ptr;

    if(!leaf && other.leaf)
        leaf = new dbpt::leaf_node<T>();

    if(other.leaf)
        *leaf = *(other.leaf);
    else
        leaf = nullptr;

    cur_pos = other.cur_pos;
    leaf_pos = other.leaf_pos;
}

template <typename T, class RepInsert>
DPlusTree<T, RepInsert>::iterator::~iterator()
{
    if(leaf)
        delete leaf;
}

template <typename T, class RepInsert>
typename DPlusTree<T, RepInsert>::iterator& DPlusTree<T, RepInsert>::iterator::operator=(const iterator &other)
{
    if(!leaf && other.leaf)
        leaf = new dbpt::leaf_node<T>();

    if(other.leaf)
        *leaf = *(other.leaf);
    else
        leaf = nullptr;

    cur_pos = other.cur_pos;

    return *this;
}

template <typename T, class RepInsert>
T DPlusTree<T, RepInsert>::iterator::operator*()
{
    if(!leaf)
        return T();

    return leaf->data[cur_pos];
}

template <typename T, class RepInsert>
bool DPlusTree<T, RepInsert>::iterator::operator!=(const DPlusTree<T, RepInsert>::iterator& itr)
{
    return !(*this == itr);
}

template <typename T, class RepInsert>
bool DPlusTree<T, RepInsert>::iterator::operator==(const DPlusTree<T, RepInsert>::iterator& lhs)
{
    return (leaf_pos == lhs.leaf_pos && cur_pos == lhs.cur_pos);
}

template <typename T, class RepInsert>
typename DPlusTree<T, RepInsert>::iterator DPlusTree<T, RepInsert>::iterator::operator++(int)
{
    DPlusTree<T, RepInsert>::iterator temp(*this);
    operator++();
    return temp;
}

template <typename T, class RepInsert>
typename DPlusTree<T, RepInsert>::iterator& DPlusTree<T, RepInsert>::iterator::operator++()
{
    if(leaf)
    {
        if(cur_pos < leaf->data_count)
            cur_pos++;

        if(cur_pos == leaf->data_count)
        {
            leaf_pos = leaf->next_leaf;
            if(leaf_pos != 0)
                dbpt::loadLeafNode(leaf, *file_ptr, leaf_pos);
            else
            {
                delete leaf;
                leaf = nullptr;
            }
            cur_pos = 0;
        }
    }
    return *this;
}


// ======================== DPlusTree Functions ============================

template <typename T, class RepInsert>
DPlusTree<T, RepInsert>::DPlusTree(const std::string file_name, bool clear_existing_file)
{
    file = new std::fstream();
    this->file_name = file_name;

    if(clear_existing_file)
    {
        if(!create_new_file(file_name))
        {
            delete file;
            file = nullptr;
            return;
        }
    }
    else if(!open_file(file_name) && !create_file(file_name))
    {
        delete file;
        file = nullptr;
        return;
    }

    if(g_goto_end(*file) > 0)
        load_tree();
    else
        create_tree();
}

template <typename T, class RepInsert>
DPlusTree<T, RepInsert>::~DPlusTree()
{
    file->close();
    delete file;
}

template <typename T, class RepInsert>
void DPlusTree<T, RepInsert>::clear()
{
    file->close();
    create_new_file(file_name);
    create_tree();
}

template <typename T, class RepInsert>
void DPlusTree<T, RepInsert>::insert(const T& item)
{
    if(!file)
        return;

    //std::cout << "DPlusTree Insertion: " << item << std::endl;

    RepInsert Rep;
    dbpt::insert(*file, root, item, Rep);
}

template <typename T, class RepInsert>
bool DPlusTree<T, RepInsert>::open_file(const std::string& file_name)
{
    if(file == nullptr)
        return false;
    try
    {
        open_file_RW(*file, file_name.c_str());
        return true;
    }
    catch (const char* msg) { }

    return false;
}

template <typename T, class RepInsert>
bool DPlusTree<T, RepInsert>::create_file(const std::string& file_name)
{
    if(file == nullptr)
        return false;

    try
    {
        open_file_W(*file, file_name.c_str());
        file->close();
        open_file_RW(*file, file_name.c_str());
        return true;

    } catch(const char* msg) { }

    return false;
}

template <typename T, class RepInsert>
bool DPlusTree<T, RepInsert>::create_new_file(const std::string& file_name)
{
    if(file == nullptr)
        return false;

    try
    {
        clear_open_file_W(*file, file_name.c_str());
        file->close();
        open_file_RW(*file, file_name.c_str());
        return true;

    } catch(const char* msg) { }

    return false;
}

template <typename T, class RepInsert>
void DPlusTree<T, RepInsert>::load_tree()
{
    file->seekg(0, std::ios_base::beg);
    file->read(reinterpret_cast<char*>(&root), sizeof(std::streampos));
    file->read(reinterpret_cast<char*>(&list_start), sizeof(std::streampos));
}

template <typename T, class RepInsert>
void DPlusTree<T, RepInsert>::create_tree()
{
    dbpt::update_root<T>(*file, root);
    dbpt::update_leaf_start<T>(*file, list_start);

    std::pair<std::streampos, dbpt::leaf_node<T>*> root_pair = dbpt::createLeafNode<T>(*file);

    root = root_pair.first;
    list_start = root;

    dbpt::update_root<T>(*file, root);
    dbpt::update_leaf_start<T>(*file, list_start);
    delete root_pair.second;
}

template <typename T, class RepInsert>
bool DPlusTree<T, RepInsert>::find(const T& item, T &found_item)
{
    if(!file)
        return false;

    int pos;
    std::streampos leaf_pos;
    dbpt::leaf_node<T>* found = dbpt::find_node(*file, root, item, pos, leaf_pos);

    if(found)
    {
        found_item = found->data[pos];
        delete found;
        return true;
    }

    return false;
}
\
template <typename T, class RepInsert>
typename DPlusTree<T, RepInsert>::iterator DPlusTree<T, RepInsert>::find(const T& item)
{
    if(!file)
         return this->end();

    int pos;
    std::streampos leaf_pos;

    if(dbpt::find_node(*file, root, item, pos, leaf_pos))
        return DPlusTree<T, RepInsert>::iterator(file, leaf_pos, pos);

    return this->end();
}


#endif // DISK_BPLUSTREE_H
