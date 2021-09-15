#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include "DataBase/database.h"

/*--------------------------------
 * Program Name: DataBase
 * Author: Andrew Miner
 * Purpose: SQL - like Database
 * that stores data to disk using
 * the DPlusTree structure and a
 * DataBuffer object
 * Notes: Half-Finished. No
 * conditionals and some bugs.
 */

using namespace std;

// Run DataBase
void test_database();

// Fill table_name with 3 random string values
// starting with the letter z in order to keep
// valid data sorted above the randomly inserted junk
void fill_table(DataBase& db, const string& table_name);

int main(int argc, char* argv[])
{
    test_database();
    return 1;
}

void test_database()
{
    DataBase data;
    //fill_table(data, "big_test");
    data.run();
}


void fill_table(DataBase& db, const string& table_name)
{
    srand(time(0));
    db.transact("load table " + table_name);

    for (int i = 0; i < 1000; i++)
    {
        string cmd = "insert into " + table_name + " values z" + to_string(rand() % 1000000000) + ", z" + to_string(rand() % 1000000000) + ", " + to_string(rand() % 100 + 1);
        db.transact(cmd, false);
    }
}
