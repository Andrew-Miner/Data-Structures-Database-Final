 #ifndef DATATABLE_H
#define DATATABLE_H
#include "record.h"
#include <map>
#include "../Table/BalancedTree/D+Tree/disk_bplustree.h"
#include "databuffer.h"
#include "../Table/BalancedTree/index_pair.h"

typedef DPlusTree<IndexPair, AddIndices<IndexPair>> IndexTree;

class DataTable
{
public:
    /* ============================================================
     * Name: Load Constructor
     * Function: Loads Table In Directory dir named table_file
     * Pre: none
     * Post: Constructs DataTable Object from table_file table
     * Parameters: dir - Directory Containing Table File
     *             table_file - name of table
     *             mem_alloc_mb - max memory DataTables buffer uses
     */
    DataTable(const std::string &dir, const std::string &table_file, const std::size_t &mem_alloc_mb = 1);

    /* ============================================================
     * Name: Create Constructor
     * Function: Creates Table In Directory dir named table_file
     * Pre: none
     * Post: Constructs New DataTable Object
     * Parameters: dir - Directory To Store table_file in
     *             table_file - name of table
     *             columns - table column names
     *             mem_alloc_mb - max memory DataTables buffer uses
     */
    DataTable(const std::string &dir, const std::string &table_name, const std::vector<std::string> &columns,  const std::size_t &mem_alloc_mb = 1);

    /* ============================================================
     * Name: Sudo-Copy Constructor
     * Function: Copies DataTable other to directory dir in table
     *           named table_name and creates DataTable Object
     * Pre: Other points to valid DataTable
     * Post: Constructs DataTable Object
     * Parameters: dir - Directory To Save Table File
     *             table_file - name of table
     *             other - DataTable to copy
     * TODO: FIX THIS FUNCTION. Currently it has an index out of
     *       bounds error.
     */
    DataTable(const std::string &dir, const std::string &table_name, DataTable* other);

    // Destructor
    ~DataTable();

    /* ======================================================
     * Name: Insert Record
     * Function: Insert Record rec
     * Pre: rec is the same size as column_count
     * Post: Inserts record rec
     * Parameters: rec - vector containing record strings in
     *                   the same order as the table columns
     */
    bool InsertRecord(const std::vector<std::string> &rec);

    /* ======================================================
     * Name: Get Record
     * Function: Get Record From Table
     * Pre: UID is within Table file's range
     * Post: Returns Record located at UID
     * Parameters: UID - position with table file of record
     */
    Record GetRecord(const std::streampos &UID);

    /* =========================================================
     * Name: Overwrite Record
     * Function: Save Record To Table Base On Record's unique_id
     * Pre: Record rec is located within table
     * Post: Record rec is rewritten to file
     * Parameters: rec - Record Obj Contained Within DataTable
     */
    void OverwriteRecord(Record rec);

    /* ======================================================
     * Name: Get Buffer
     * Function: Get DataTable's DataBuffer object
     * Pre: None
     * Post: Returns Pointer To DataTable's DataBuffer
     * Parameters: None
     */
    DataBuffer<Record>* getBuffer() { return &records; }

    /* ======================================================
     * Name: Reset Buffer
     * Function: Resets DataTable's DataBuffer
     * Pre: none
     * Post: DataTable's DataBuffer is set to the start of
     *       the table
     * Parameters: None
     */
    void resetBuffer() { records.readBlock(rec_start); }

    /* ======================================================
     * Name: Get Record Size
     * Function: Returns Total Size of records in bytes
     */
    std::size_t getRecordSize() const { return record_size_bytes; }

    /* ======================================================
     * Name: Get Starting Byte
     * Function: Returns POS in table that records start
     */
    std::streampos getStartingByte() { return rec_start; }


    /* ======================================================
     * Name: Get Column Count
     * Function: Returns Count of total columns
     */
    std::size_t getColumnCount() { return column_count; }

    /* ======================================================
     * Name: Get Columns
     * Function: Returns Vector Of Column Names
     */
    std::vector<std::string> getColumns() { return columns; }

    /* ======================================================
     * Name: Get Indexed Trees
     * Function: Returns Vector Of IndexedTree Pointers
     */
    std::vector<IndexTree*> getIndexedTrees() { return index_tables; }

    /* ======================================================
     * Name: Rebuild Indexed Trees
     * Function: Clear IndexTrees and reinsert all records
     */
    void rebuildIndexedTrees();

    /* ============================================================
     * Name: Make Selection
     * Function: Creates/Returns A Temporary Table Out Of Selection
     * Pre: cols are contained within Table
     * Post: Returns Pointer To Newly Allocated DataTable
     * Parameters: temp_dir - directory to create temporary table
     *             cols - columns selected
     *             conditions - selection conditions
     * NOTE: Conditions are NOT implemented
     * TODO: Implement Conditional Selections
     */
    DataTable* makeSelection(std::string temp_dir, std::vector<std::string> cols, std::vector<std::string> conditions);

    /* =======================================================
     * Name: Operator <<
     * Function: Prints Currently Loaded Records In DataBuffer
     *           to ostream out
     */
    friend std::ostream& operator<<(std::ostream &out,  const DataTable &table);

private:
    const char* record_extension = ".rcr";
    const char* index_extension = ".dptr";

    std::size_t record_size_bytes;
    std::size_t column_count;
    std::streampos rec_start;

    std::string table_name;
    std::vector<std::string> columns;
    std::map<std::string, int> column_indices;
    std::vector<IndexTree*> index_tables;

    std::fstream table;
    DataBuffer<Record> records;

    /* ============================================================
     * Name: Open Table
     * Function: Loads Table From Directory dir named file_name
     * Pre: dir exists and DataTable named file_name exists
     * Post: Loads Columns and rec_start and index_tables
     * Parameters: dir - Directory Containing Table File
     *             file_name - name of table
     */
    bool open_table(const std::string &dir, const std::string &file_name);

    /* ============================================================
     * Name: Create Table
     * Function: Creates Table in Directory dir named file_name
     * Pre: Directory dir exists and columns is not empty
     * Post: Creates Table File and IndexedTree files
     * Parameters: dir - Directory To Create Table File In
     *             file_name - name of table
     *             columns - name of table columns
     */
    bool create_table(const std::string &dir, const std::string &file_name, const std::vector<std::string> &columns);

    /* =======================================================
     * Name: Add Extension
     * Function: Adds extension .rcr to table_name if it
     *           it doesn't already have it
     */
    void add_extension(std::string &table_name);

    /* ========================================================
     * Name: Print Row
     * Function: Prints Values To ostream out with a table cell
     *           size of cell_size
     */
    static std::ostream& print_row(std::ostream &out, const std::vector<std::string> &values, int &cell_size);
};

#endif // DATATABLE_H
