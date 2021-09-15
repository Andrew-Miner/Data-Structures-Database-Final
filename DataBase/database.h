#ifndef DATABASE_H
#define DATABASE_H
#include <map>
#include "Table/datatable.h"
#include "CommandParser/commandparser.h"
# include <sys/stat.h>

class DataBase
{
public:
    const std::string DEFAULT_DIR = "DataBase";
    const std::string TABLE_DIR = "/Tables";
    const std::string TEMP_DIR = "/Temp";

    // Default Constructor
    DataBase();

    // Destructor
    ~DataBase();

    // Runs Query Application
    void run();

    /* ===========================================================
     * Name: transact
     * Function: Proccesses Commands in the form of strings
     * Pre: none
     * Post: If query is valid it's command is executed
     * Parameters: Query - command to be executed
     *             display_logs - determines if logs are displayed
     */
    void transact(const std::string &query, bool display_logs = true);

private:
    const char* db_prefix = "[DataBase] ";
    const char* cmd_prefix = "[Command] ";
    const char* line = "========================================================================================================================\n";

    bool exit_process, print_log;
    std::map<std::string, DataTable*> open_tables;
    CommandParser cparser;

    /* ======================================================
     * Name: executeCMD
     * Function: Execute DataBase Commands
     * Pre: cmd is valid command
     * Post: If cmd is valid it's command is executed
     * Parameters: cmd - Parse Tree of command to be executed
     */
    bool executeCMD(PTree &cmd);

    /* ======================================================
     * Name: create_table
     * Function: Create New Table
     * Pre: cmd is valid create command
     * Post: A New Table Is Created and Added To The Database
     * Parameters: cmd - Parse Tree of command to be executed
     */
    void create_table(PTree &cmd);

    /* ======================================================
     * Name: load_table
     * Function: Load Table From Disk
     * Pre: cmd is valid load command
     * Post: The Requested Table Is Loaded Into Memory
     * Parameters: cmd - Parse Tree of command to be executed
     */
    void load_table(PTree &cmd);

    /* ======================================================
     * Name: close_table
     * Function: Remove Table From Memory
     * Pre: cmd is valid close command
     * Post: The Requested Table Is Unloaded Into Memory
     * Parameters: cmd - Parse Tree of command to be executed
     */
    void close_table(PTree &cmd);

    /* ======================================================
     * Name: select_table
     * Function: Select and Display requested table
     * Pre: cmd is valid select command
     * Post: If a whole table is selected it's displayed.
     *       If peices of a table are selected those records
     *       are stored into a temp table and dispalyed.
     * Parameters: cmd - Parse Tree of command to be executed
     */
    void select_table(PTree &cmd);

    /* ======================================================
     * Name: insert_table
     * Function: Insert record into requested table
     * Pre: cmd is valid insert command
     * Post: Inserts record contained within cmd at the
     *       end of the requested table
     * Parameters: cmd - Parse Tree of command to be executed
     */
    void insert_table(PTree &cmd);

    /* ======================================================
     * Name: rebuild_table
     * Function: Rebuild A Table's IndexedTrees
     * Pre: cmd is valid rebuild command
     * Post: Erases and Reconstructs The Table's IndexedTrees
     * Parameters: cmd - Parse Tree of command to be executed
     * TODO: Implement a delete record function and have this
     *       function do all of the actual deleting when called
     */
    void rebuild_table(PTree &cmd);

    /* ======================================================
     * Name: display_table
     * Function: Display's A Table To The CMD Window
     * Pre: table is a valid loaded DataTable
     * Post: Table Is Displayed To Screen And The User Is
     *       Requested For Input
     * Parameters: table - DataTable to be displayed
     *             is_temp - bool determining if table is temp
     */
    void display_table(DataTable &table, bool is_temp = false);

};

#endif // DATABASE_H
