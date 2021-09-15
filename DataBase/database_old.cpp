#include "database.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <algorithm>

#ifdef __APPLE__
    #define IS_WIN false
    #define mkFolder(dir) mkdir(dir, 777)
#else
    #define IS_WIN true
    #define mkFolder(dir) mkdir(dir);
#endif

const int PERMISSION_MODE = 0777;


DataBase::DataBase(): exit_process(false)
{
    if(IS_WIN)
    {
        int code;
        bool slp = false;

        std::cout << line;

        if(!(code = mkFolder(DEFAULT_DIR.c_str())))
            std::cout << " " << db_prefix << "Default Directory Created!" << std::endl;

        slp = (code == -1 && !slp) ? false : true;

        if(!(code = mkFolder((DEFAULT_DIR + TABLE_DIR).c_str())))
            std::cout << " " << db_prefix << "Table Directory Created!" << std::endl;

        slp = (code == -1 && !slp) ? false : true;

        if(!(code = mkFolder((DEFAULT_DIR + TEMP_DIR).c_str())))
            std::cout << " " << db_prefix << "Temp Table Directory Created!" << std::endl;

        slp = (code == -1 && !slp) ? false : true;

        if(slp)
            sleep(5);
    }
}

DataBase::~DataBase()
{
    for(std::map<std::string, DataBuffer*>::iterator it = open_tables.begin(); it != open_tables.end(); ++it)
        delete *it;
}

void DataBase::run()
{

    while(!exit_process)
    {
        //std::system("CLS");
        if (system("CLS"));
            //system("clear");

        std::cout << line << cmd_prefix << "\b: ";

        std::string args;
        std::getline(std::cin, args);
        fflush(stdin);

        std::cout << std::endl;

        cparser.setCommand(args);
        PTree cmd = cparser.parseCommand();

        if(cmd.Empty())
            std::cout << cmd_prefix << "Error: Invalid Command!\n\n";
        else
            executeCMD(cmd);

        std::cout << "Press Any Button To Continue... " << std::endl;
        std::cin.ignore();
        fflush(stdin);
    }
}

bool DataBase::executeCMD(PTree &cmd)
{
    std::string cmd_str = cmd.getList(CMD_ID).at(0);

    if(cmd_str == CMD_SELECT)
        return false;//select_table(cmd);
    else if(cmd_str == CMD_CREATE)
    {
        try { create_table(cmd); }
        catch(const char* msg) { std::cout << std::endl << db_prefix << msg << std::endl << std::endl; }
        return true;
    }
    else if(cmd_str == CMD_INSERT)
    {
        std::cout << cmd << std::endl;
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
    std::string dir = DEFAULT_DIR + TABLE_DIR + "/" + pair.first;

    if(mkdir(dir.c_str()))
        throw ("Error: Failed To Create Table Directory!");

    try
    {
        pair.second = new DataTable(dir, pair.first, cmd.getList(COL_ID));
        open_tables.insert(pair);
    }
    catch (const char* msg) { throw msg; }
}

void DataBase::load_table(PTree &cmd)
{
    std::pair<std::string, DataTable*> pair;
    pair.first = cmd.getList(TABLE_ID).at(0);
    std::string dir = DEFAULT_DIR + TABLE_DIR + "/" + pair.first;

    std::cout << " Table: " << pair.first << std::endl;
    std::cout << " Directory: " << dir << std::endl << std::endl;

    try
    {
        pair.second = new DataTable(dir, pair.first);
        open_tables.insert(pair);
    }
    catch (const char* msg) { throw msg; }
}

bool DataBase::select_table(PTree &cmd)
{

}

void DataBase::insert_table(PTree &cmd)
{
    std::map<std::string, DataTable*>::iterator table = open_tables.find(cmd.getList(TABLE_ID).at(0));

    if(table == open_tables.end())
        throw ("Error: Table Not Loaded!");

    std::vector<std::string> record = cmd.getList(COL_ID);

    if(!(table->second->InsertRecord(record)))
        throw ("Error: Incorrect Number Of Values!");

    std::cout << " Inserted Record: ";

    for(std::vector<std::string>::iterator it = record.begin(); it != record.end(); ++it)
        std::cout << '[' << *it << "] ";
    std::cout << std::endl;
}

