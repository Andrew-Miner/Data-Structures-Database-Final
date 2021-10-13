#include "database.h"
#include <stdlib.h>
//#include <unistd.h>
#include <sys/stat.h>
#include <algorithm>
#include <thread>
#include <chrono>
#include <filesystem>

// Constant Used To Determine How
// Much Memory in Megabytes Is Used
// By Each Table. Currently Set To 2mb
const std::size_t MEM_ALLOC = 2;

DataBase::DataBase(): exit_process(false), print_log(true)
{
     //Windows Create Directory Code
     //Commented out for Mac compatability

        int code;
        bool slp = false;
//        std::cout << line;

//        if(!(code = mkdir(DEFAULT_DIR.c_str())))
//            std::cout << " " << db_prefix << "Default Directory Created!" << std::endl;

//        slp = (code == -1 && !slp) ? false : true;

//        if(!(code = mkdir((DEFAULT_DIR + TABLE_DIR).c_str())))
//            std::cout << " " << db_prefix << "Table Directory Created!" << std::endl;

//        slp = (code == -1 && !slp) ? false : true;

//        if(!(code = mkdir((DEFAULT_DIR + TEMP_DIR).c_str())))
//            std::cout << " " << db_prefix << "Temp Table Directory Created!" << std::endl;

//        slp = (code == -1 && !slp) ? false : true;

        if(slp)
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

DataBase::~DataBase()
{
    // Delete All Open Tables
    for(std::map<std::string, DataTable*>::iterator it = open_tables.begin(); it != open_tables.end(); ++it)
        delete (*it).second;
}

void DataBase::run()
{

    while(!exit_process)
    {
        // Horribly inefficient clear cmd window
        // functions. CLS is for windows, and if it failed
        // clear is called for Mac
        if (system("CLS"))
            system("clear");

        std::cout << "Current: " << std::filesystem::current_path() << std::endl;
        // If Tables Are Open Print Them To The CMD Window
        if(open_tables.size())
        {
            std::cout << line;
            std::cout << std::setw(std::strlen(line)/2 + std::strlen(db_prefix)/2) << db_prefix << std::endl;

            std::string msg = "Open Tables";
            std::cout << std::setw(std::strlen(line)/2 + msg.length()/2) << msg << std::endl << std::endl;

            for(std::map<std::string, DataTable*>::iterator it = open_tables.begin(); it != open_tables.end(); ++it)
            {
                std::string tbl = "[" + it->first + "]";

                std::vector<std::string> cols = it->second->getColumns();
                for(std::string col : cols)
                    tbl += " - " + col;

                tbl += " | Total Records: " + std::to_string(it->second->getRecordSize()/sizeof(Record)) + " | " + std::to_string((it->second->getRecordSize()/1024)/1024) + " mb";

                std::cout << std::setw(std::strlen(line)/2 + tbl.length()/2) << tbl << std::endl;
            }
        }

        std::cout << line << cmd_prefix << "\b: ";

        std::string args;
        std::getline(std::cin, args);
        fflush(stdin);

        std::cout << std::endl;

        // Parse/Execute Command
        transact(args);

        std::cout << "Press <Return> To Continue... " << std::endl;
        std::cin.ignore();
        fflush(stdin);
    }
}

void DataBase::transact(const std::string &query, bool display_logs)
{
    // Parse Command Into ParseTree
    cparser.setCommand(query);
    std::cout << "Parsing Query: " << query << std::endl;
    PTree cmd = cparser.parseCommand();
    std::cout << "Parse Finished" << std::endl;

    // Establish w/e or not to print logs
    if(!display_logs)
        print_log = false;

    // If query is an invalid command the PTree
    // Returns empty
    if(cmd.Empty())
        std::cout << cmd_prefix << "Error: Invalid Command!\n\n";
    else
        executeCMD(cmd);

    // Reset Print Log setting
    print_log = true;
}

bool DataBase::executeCMD(PTree &cmd)
{
    std::string cmd_str = cmd.getList(CMD_ID).at(0);
    std::cout << "Executing Command: " << cmd_str << std::endl;
    if(cmd_str == CMD_SELECT)
    {
        try { select_table(cmd); }
        catch(const char* msg) { std::cout << std::endl << db_prefix << msg << std::endl << std::endl; }
        return true;
    }
    else if(cmd_str == CMD_CREATE)
    {
        try { create_table(cmd); }
        catch(const char* msg) { std::cout << std::endl << db_prefix << msg << std::endl << std::endl; }
        return true;
    }
    else if(cmd_str == CMD_INSERT)
    {
        try { insert_table(cmd); }
        catch(const char* msg) { std::cout << std::endl << db_prefix << msg << std::endl << std::endl; }
        return true;
    }
    else if(cmd_str == CMD_LOAD)
    {
        try { load_table(cmd); }
        catch(const char* msg) { std::cout << std::endl << db_prefix << msg << std::endl << std::endl; }
        return true;
    }
    else if(cmd_str == CMD_CLOSE)
    {
        try { close_table(cmd); }
        catch(const char* msg) { std::cout << std::endl << db_prefix << msg << std::endl << std::endl; }
        return true;
    }
    else if(cmd_str == CMD_REBUILD)
    {
        try { rebuild_table(cmd); }
        catch(const char* msg) { std::cout << std::endl << db_prefix << msg << std::endl << std::endl; }
        return true;
    }
    else
        exit_process = (cmd_str == CMD_EXIT);

    if(exit_process)
        return true;

    return false;
}

void DataBase::create_table(PTree &cmd)
{
    std::pair<std::string, DataTable*> pair;
    pair.first = cmd.getList(TABLE_ID).at(0);

    // Commented Code Used For Folder Creation In Windows
    // Commented For Mac Compatability
    std::string dir = DEFAULT_DIR + TABLE_DIR + "/" + pair.first;

    // Commented For Mac Compatability
    //if(mkdir(dir.c_str()))
        //throw ("Error: Failed To Create Table Directory!");

    try
    {
        pair.second = new DataTable(dir, pair.first, cmd.getList(COL_ID), MEM_ALLOC);
        open_tables.insert(pair);
    }
    catch (const char* msg) { throw msg; }
}

void DataBase::load_table(PTree &cmd)
{
    std::pair<std::string, DataTable*> pair;
    pair.first = cmd.getList(TABLE_ID).at(0);

    if(open_tables.find(pair.first) != open_tables.end())
        throw ("Error: Table Already Loaded!");

    // Commented Code Used For Folder Creation In Windows
    // Commented For Mac Compatability
    std::string dir = DEFAULT_DIR + TABLE_DIR + "/" + pair.first;


    if(print_log)
    {
        std::cout << " Table: " << pair.first << std::endl;
        std::cout << " Directory: " << dir << std::endl << std::endl;
    }

    try
    {
        pair.second = new DataTable(dir, pair.first, MEM_ALLOC);
        open_tables.insert(pair);
    }
    catch (const char* msg) { throw msg; }
}

void DataBase::close_table(PTree &cmd)
{
    std::string tbl_name = cmd.getList(TABLE_ID).at(0);
    std::map<std::string, DataTable*>::iterator tbl_ptr = open_tables.find(tbl_name);

    if(tbl_ptr == open_tables.end())
        throw ("Error: Table Not Loaded!");

    delete tbl_ptr->second;
    open_tables.erase(tbl_ptr);

    if(print_log)
        std::cout << std::endl << " Closed Table: " << tbl_name << std::endl << std::endl;
}

void DataBase::rebuild_table(PTree &cmd)
{
    std::string table_name = cmd.getList(TABLE_ID).at(0);

    std::map<std::string, DataTable*>::iterator it = open_tables.find(table_name);

    if(it == open_tables.end())
        throw ("Error: Table Not Loaded!");

    if(print_log)
        std::cout << " Rebuilding Table: " << table_name << std::endl << std::endl;

    try
    {
        it->second->rebuildIndexedTrees();
    }
    catch (const char* msg) { throw msg; }
}

void DataBase::select_table(PTree &cmd)
{
    std::map<std::string, DataTable*>::iterator table = open_tables.find(cmd.getList(TABLE_ID).at(0));

    if(table == open_tables.end())
        throw ("Error: Table Not Loaded!");

    std::vector<std::string> cols = cmd.getList(COL_ID);
    std::vector<std::string> conditions = cmd.getList(CONDITIONAL_ID);

    // Debug Code For Non-implemented Conditionals
    if(conditions.size())
    {
        std::cout << "Conditions: ";
        for(std::string str : conditions)
            std::cout << str << " ";
        std::cout << std::endl << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    if(cols.at(0) == "*")
        display_table(*(table->second));
    else
    {
        DataTable* selection = table->second->makeSelection(DEFAULT_DIR + TEMP_DIR, cols, conditions);
        display_table(*selection, true);
        delete selection;
    }
}

void DataBase::display_table(DataTable &table, bool is_temp)
{
    // ResetBuffer Resets the block of records currently loaded
    // Within DataTable to the begining of the table
    table.resetBuffer();

    // NOTE: skipConditional is used here to repeat the loop
    //       while skipping the While Conditional Statement
    //       in order to avoid incrementing the DataBuffer.
    bool skipConditional = false;
    do
    {
        skipConditional = false;

        if (system("CLS"))
            system("clear");

        std::cout << std::endl << std::endl << std::endl << table << std::endl;

        std::cout << " Start Position: " << table.getStartingByte() << std::endl;
        std::cout << " File Position: " << table.getBuffer()->getPosition() << std::endl;
        std::cout << " Max Buffer Size: " << table.getBuffer()->MAX_DATA_COUNT << std::endl;
        std::cout << " Cur Buffer Size: " << table.getBuffer()->size() << std::endl;
        std::cout << " Size Of Record: " << sizeof(Record) << std::endl << std::endl;

        std::cout << " Page " << (table.getBuffer()->getPosition() - table.getStartingByte())/(table.getBuffer()->MAX_DATA_COUNT * sizeof(Record)) + 1
                  << "/" << (table.getRecordSize()/(table.getBuffer()->MAX_DATA_COUNT * sizeof(Record))) + ((table.getRecordSize() % (table.getBuffer()->MAX_DATA_COUNT * sizeof(Record)) == 0) ? 0 : 1) << std::endl;

        // Issue with the total records displayed counted || second line
        std::cout << " Currently Displaying Records #" << (table.getBuffer()->getPosition() - table.getStartingByte())/sizeof(Record)+1
                  << " - #" << (table.getBuffer()->getNextPosition() - table.getStartingByte())/sizeof(Record) << std::endl << std::endl;

        std::cout << " [Next]          - Print Next Chunk" <<
        std::endl << " [Goto] <Page #> - Goto Specified Page" << std::endl;
        std::cout << " [Reset]         - Return To Top Of Table\n" << std::endl;

        if(is_temp)
            std::cout << " [Save Temp As] <Table Name> - Save Temporary Selection To Database" << std::endl << std::endl;

        std::cout << " [Exit] - Exit Table Display" << std::endl << std::endl;
        std::cout << cmd_prefix << "\b: ";

        std::string args;
        std::getline(std::cin, args);
        fflush(stdin);

        cparser.setCommand(args);
        PTree ans = cparser.parseCommand();

        if(ans.Empty())
            continue;

        std::string answer = ans.getList(CMD_ID).at(0);

        if(answer == CMD_RESET)
        {
            table.resetBuffer();
            skipConditional = true;
        }
        else if(answer == CMD_GOTO)
        {
            table.getBuffer()->readBlock(table.getBuffer()->size() * sizeof(Record) * (std::stoi(ans.getList(CONDITIONAL_ID).at(0)) - 1) + table.getStartingByte());
            skipConditional = true;
        }
        else if(is_temp && answer == CMD_SAVE)
        {
            if(ans.getList(TABLE_ID).size() == 0)
                skipConditional = true;

            std::cout << std::endl << " Saving Temporary Selection To Table: " << ans.getList(TABLE_ID).at(0) << std::endl << std::endl;

            open_tables.insert(std::pair<std::string, DataTable*>(ans.getList(TABLE_ID).at(0), new DataTable((DEFAULT_DIR + TABLE_DIR).c_str(), ans.getList(TABLE_ID).at(0), &table)));
        }
        else if(answer == CMD_EXIT)
            break;
        else if(answer != CMD_NEXT)
            skipConditional = true;

    } while(skipConditional || table.getBuffer()->readBlock());
}

void DataBase::insert_table(PTree &cmd)
{
    std::map<std::string, DataTable*>::iterator table = open_tables.find(cmd.getList(TABLE_ID).at(0));

    if(table == open_tables.end())
        throw ("Error: Table Not Loaded!");

    std::vector<std::string> record = cmd.getList(COL_ID);

    if(!(table->second->InsertRecord(record)))
        throw ("Error: Incorrect Number Of Values!");

    if(print_log)
    {
        std::cout << " Inserted Record: ";

        for(std::vector<std::string>::iterator it = record.begin(); it != record.end(); ++it)
            std::cout << '[' << *it << "] ";
        std::cout << std::endl << std::endl;
    }
}

