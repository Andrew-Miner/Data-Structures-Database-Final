#include "commandparser.h"
#include "../AssistFunctions/array2D_helper_functions.h"
#include <algorithm>
#include <iostream>

CommandParser::CommandParser(std::string command)
{
    // Build String:Index map
    build_string_indices();

    // Allcoate/Build state_array and
    // State related maps
    build_state_array();

    // Parse string command into tokens and insert
    // them into t_command
    if(command.length())
        getTokens(command);
}

PTree CommandParser::parseCommand()
{
    // Check if tokens is empty
    // if so return empty parse tree
    if(!t_command.size())
        return PTree();

    // Get Index Of First Command and corresponding
    // parse tree identifiers
    int i_cmd = getCmdIndex(t_command.at(0));
    std::vector<std::string> ids = get_identifiers(i_cmd);

    // Check if identifier vector is not empty
    // If it is empty it means your begining command
    // is invalid and we return an empty PTree()
    if(ids.size())
    {
        // Build PTree with i_cmd as type, using
        // command identifiers
        PTree cmd(i_cmd, ids);
        int row = 0;

        // Begin Looping Through Command Tokens in order to parse command
        for(std::vector<std::string>::iterator it = t_command.begin(); it != t_command.end(); ++it)
        {
            // Get Column Index t_command token represents
            int col = getCmdIndex(*it);

            // Get State_Array row value
            int next_row = state_array[row][col];

            // Debug Print
            //std::cout << "ROW: " << row << " Col: " << *it << "|" << col << " Next Row: " << next_row << std::endl;

            // If t_command token is valid
            if(next_row != -1)
            {
                if(row_insertion_identifier.find(row) != row_insertion_identifier.end())
                {
                    if(find(rows_maintain_order.begin(), rows_maintain_order.end(), row) != rows_maintain_order.end())
                        cmd.InsertInOrder(row_insertion_identifier[row], *it);
                    else
                        cmd.Insert(row_insertion_identifier[row], *it);
                }
                row = next_row;
            }
            // Else if current row is a success row, return valid parse tree
            else if(find(success_rows.begin(), success_rows.end(), row) != success_rows.end())
                return cmd;
            // Else return empty parse tree
            else return PTree();
        }

        // If Parsing Has Finished, Check if ending row is a success row
        if(find(success_rows.begin(), success_rows.end(), row) != success_rows.end())
            return cmd;
    }

    return PTree();
}

CommandParser::~CommandParser()
{
    if(state_array)
    {
        for(int i = 0; i < max_rows; i++)
            delete[] state_array[i];
        delete[] state_array;
    }
}

void CommandParser::setCommand(const std::string& command)
{
    this->command = command;
    getTokens(this->command);
}

std::string CommandParser::getCommand()
{
    return this->command;
}

void CommandParser::build_state_array()
{
    // 5, where, 7
    // 6, where, 7
    // 7, rand, 8
    // 8, == - !=, 9
    // 9, rand, 10
    // 10, And, 7

    // Current State Array Configuration:
    /*=====================================================================================================================================================================================================
     *  | Select | Random Str | * | , | From | Where | And | Create | Table | Load | Exit | Insert | Into | Values | Next | Reset | Goto | Close | Rebuild | == | < | <= | > | >= | != | Save | Temp | As |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     * 0|    1   |     -1     |-1 |-1 |  -1  |  -1   | -1  |   11   |   -1  |  15  |  17  |   18   |  -1  |   -1   |  17  |  17   |  23  |  15   |   15    | -1 |-1 | -1 |-1 | -1 | -1 |  24  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     * 1|   -1   |      2     | 3 |-1 |  -1  |  -1   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     * 2|   -1   |     -1     |-1 | 1 |   4  |  -1   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     * 3|   -1   |     -1     |-1 |-1 |   4  |  -1   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     * 4|   -1   |      5     | 6 |-1 |  -1  |  -1   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     * 5|   -1   |     -1     |-1 | 4 |  -1  |   7   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     * 6|   -1   |     -1     |-1 |-1 |  -1  |   7   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     * 7|   -1   |      8     |-1 |-1 |  -1  |   -1  | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     * 8|   -1   |     -1     |-1 |-1 |  -1  |   -1  | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    |  9 | 9 |  9 | 9 |  9 |  9 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     * 9|   -1   |     10     |-1 |-1 |  -1  |   -1  | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     *10|   -1   |     -1     |-1 |-1 |  -1  |   -1  |  7  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     *11|   -1   |     -1     |-1 |-1 |  -1  |  -1   | -1  |   -1   |   12  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     *12|   -1   |     13     |-1 |-1 |  -1  |  -1   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     *13|   -1   |     14     |-1 |-1 |  -1  |  -1   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     *14|   -1   |     -1     |-1 |13 |  -1  |  -1   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     *15|   -1   |     -1     |-1 |-1 |  -1  |  -1   | -1  |   -1   |   16  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     *16|   -1   |     17     |-1 |-1 |  -1  |  -1   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     *17|   -1   |     -1     |-1 |-1 |  -1  |  -1   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     *18|   -1   |     -1     |-1 |-1 |  -1  |  -1   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  19  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     *19|   -1   |     20     |-1 |-1 |  -1  |  -1   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     *20|   -1   |     -1     |-1 |-1 |  -1  |  -1   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   21   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     *21|   -1   |     22     |-1 |-1 |  -1  |  -1   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     *22|   -1   |     -1     |-1 | 21|  -1  |  -1   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     *23|   -1   |     17     |-1 | -1|  -1  |  -1   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     *24|   -1   |     -1     |-1 | -1|  -1  |  -1   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  25  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     *25|   -1   |     -1     |-1 | -1|  -1  |  -1   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | 26 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     *26|   -1   |     17     |-1 | -1|  -1  |  -1   | -1  |   -1   |   -1  |  -1  |  -1  |   -1   |  -1  |   -1   |  -1  |  -1   |  -1  |  -1   |   -1    | -1 |-1 | -1 |-1 | -1 | -1 |  -1  |  -1  | -1 |
     * -|--------|------------|---|---|------|-------|-----|--------|-------|------|------|--------|------|--------|------|-------|------|-------|---------|----|---|----|---|----|----|------|------|----|
     */


    max_rows = DEFAULT_MAX_ROWS;
    max_cols = DEFAULT_MAX_COLS;

    allocate_array(state_array, max_rows, max_cols);
    initialize_array(state_array, max_rows, max_cols, -1);

    // Select State Array
    // Success States: Row 5, Row 6
    // Add To Command List: Row 0 | CMD_ID
    // Add To Column List: Row 1 | COL_ID
    // Add To Table List: Row 4 | TABLE_ID
    fill_cell(state_array, 0, i_select, 1);
    fill_cell(state_array, 1, i_rand_str, 2);
    fill_cell(state_array, 1, i_star, 3);
    fill_cell(state_array, 2, i_comma, 1);
    fill_cell(state_array, 2, i_from, 4);
    fill_cell(state_array, 3, i_from, 4);
    fill_cell(state_array, 4, i_rand_str, 5);
    fill_cell(state_array, 4, i_star, 6);
    fill_cell(state_array, 5, i_comma, 4);
    fill_cell(state_array, 5, i_where, 7);
    fill_cell(state_array, 6, i_where, 7);
    fill_cell(state_array, 7, i_rand_str, 8);

    fill_cell(state_array, 8, i_less_than, 9);
    fill_cell(state_array, 8, i_less_equal, 9);
    fill_cell(state_array, 8, i_greater_than, 9);
    fill_cell(state_array, 8, i_greater_equal, 9);
    fill_cell(state_array, 8, i_equal, 9);
    fill_cell(state_array, 8, i_not_equal, 9);
    fill_cell(state_array, 9, i_rand_str, 10);
    fill_cell(state_array, 10, i_and, 7);

    success_rows.push_back(5);
    success_rows.push_back(6);
    success_rows.push_back(10);
    rows_maintain_order.push_back(1);
    rows_maintain_order.push_back(7);
    rows_maintain_order.push_back(8);
    rows_maintain_order.push_back(9);
    row_insertion_identifier.insert(std::pair<int, std::string>(0, CMD_ID));
    row_insertion_identifier.insert(std::pair<int, std::string>(1, COL_ID));
    row_insertion_identifier.insert(std::pair<int, std::string>(4, TABLE_ID));
    row_insertion_identifier.insert(std::pair<int, std::string>(7, CONDITIONAL_ID));
    row_insertion_identifier.insert(std::pair<int, std::string>(8, CONDITIONAL_ID));
    row_insertion_identifier.insert(std::pair<int, std::string>(9, CONDITIONAL_ID));

    // Create State Array
    // Success States: Row 14
    // Add To Command List: Row 0 | CMD_ID
    // Add To Column List: Row 13 | COL_ID
    // Add To Table List: Row 12  | TABLE_ID
    fill_cell(state_array, 0, i_create, 11);
    fill_cell(state_array, 11, i_table, 12);
    fill_cell(state_array, 12, i_rand_str, 13);
    fill_cell(state_array, 13, i_rand_str, 14);
    fill_cell(state_array, 14, i_comma, 13);

    success_rows.push_back(14);
    rows_maintain_order.push_back(13);
    row_insertion_identifier.insert(std::pair<int, std::string>(13, COL_ID));
    row_insertion_identifier.insert(std::pair<int, std::string>(12, TABLE_ID));

    // Load State Array
    // Success States: Row 17
    // Add To Command List: Row 0 | CMD_ID
    // Add To Table List: Row 16  | TABLE_ID
    fill_cell(state_array, 0, i_load, 15);
    fill_cell(state_array, 15, i_table, 16);
    fill_cell(state_array, 16, i_rand_str, 17);

    success_rows.push_back(17);
    row_insertion_identifier.insert(std::pair<int, std::string>(16, TABLE_ID));

    // Rebuild State Array
    // Success States: Row 17
    // Add To Command List: Row 0 | CMD_ID
    // Add To Table List: Row 16  | TABLE_ID
    fill_cell(state_array, 0, i_rebuild, 15);

    // Close State Array
    // Success States: Row 17
    fill_cell(state_array, 0, i_close, 15);

    // Exit State Array
    // Success States: Row 17
    fill_cell(state_array, 0, i_exit, 17);

    // Next State Array
    // Success States: Row 17
    fill_cell(state_array, 0, i_next, 17);

    // Reset State Array
    // Success States: Row 17
    fill_cell(state_array, 0, i_reset, 17);

    // Goto State Array
    // Success States: Row 17
    fill_cell(state_array, 0, i_goto, 23);
    fill_cell(state_array, 23, i_rand_str, 17);

    row_insertion_identifier.insert(std::pair<int, std::string>(23, CONDITIONAL_ID));

    // Insert State Array
    // Success States: Row 22
    // Add To Command List: Row 0 | CMD_ID
    // Add To Table List: Row 19  | TABLE_ID
    // Add To Column List: Row 21 | COL_ID
    fill_cell(state_array, 0, i_insert, 18);
    fill_cell(state_array, 18, i_into, 19);
    fill_cell(state_array, 19, i_rand_str, 20);
    fill_cell(state_array, 20, i_values, 21);
    fill_cell(state_array, 21, i_rand_str, 22);
    fill_cell(state_array, 22, i_comma, 21);

    success_rows.push_back(22);
    rows_maintain_order.push_back(21);
    row_insertion_identifier.insert(std::pair<int, std::string>(19, TABLE_ID));
    row_insertion_identifier.insert(std::pair<int, std::string>(21, COL_ID));

    // Save Temp State Array
    // Success States: Row 17
    // Add To Command List: Row 0 | CMD_ID
    // Add To Table List: Row 26  | TABLE_ID
    fill_cell(state_array, 0, i_save, 24);
    fill_cell(state_array, 24, i_temp, 25);
    fill_cell(state_array, 25, i_as, 26);
    fill_cell(state_array, 26, i_rand_str, 17);

    row_insertion_identifier.insert(std::pair<int, std::string>(26, TABLE_ID));
}

void CommandParser::build_string_indices()
{
    string_indices.insert(std::pair<std::string, cmd_indices>(CMD_SELECT, i_select));
    string_indices.insert(std::pair<std::string, cmd_indices>("from", i_from));
    string_indices.insert(std::pair<std::string, cmd_indices>("where", i_where));
    string_indices.insert(std::pair<std::string, cmd_indices>(",", i_comma));
    string_indices.insert(std::pair<std::string, cmd_indices>("and", i_and));
    string_indices.insert(std::pair<std::string, cmd_indices>("*", i_star));
    string_indices.insert(std::pair<std::string, cmd_indices>(CMD_CREATE, i_create));
    string_indices.insert(std::pair<std::string, cmd_indices>("table", i_table));
    string_indices.insert(std::pair<std::string, cmd_indices>(CMD_LOAD, i_load));
    string_indices.insert(std::pair<std::string, cmd_indices>(CMD_EXIT, i_exit));
    string_indices.insert(std::pair<std::string, cmd_indices>(CMD_INSERT, i_insert));
    string_indices.insert(std::pair<std::string, cmd_indices>("into", i_into));
    string_indices.insert(std::pair<std::string, cmd_indices>("values", i_values));
    string_indices.insert(std::pair<std::string, cmd_indices>(CMD_NEXT, i_next));
    string_indices.insert(std::pair<std::string, cmd_indices>(CMD_RESET, i_reset));
    string_indices.insert(std::pair<std::string, cmd_indices>(CMD_GOTO, i_goto));
    string_indices.insert(std::pair<std::string, cmd_indices>(CMD_CLOSE, i_close));
    string_indices.insert(std::pair<std::string, cmd_indices>(CMD_REBUILD, i_rebuild));

    string_indices.insert(std::pair<std::string, cmd_indices>("<", i_less_than));
    string_indices.insert(std::pair<std::string, cmd_indices>("<=", i_less_equal));
    string_indices.insert(std::pair<std::string, cmd_indices>("==", i_equal));
    string_indices.insert(std::pair<std::string, cmd_indices>("!=", i_not_equal));
    string_indices.insert(std::pair<std::string, cmd_indices>(">", i_greater_than));
    string_indices.insert(std::pair<std::string, cmd_indices>(">=", i_greater_equal));

    string_indices.insert(std::pair<std::string, cmd_indices>(CMD_SAVE, i_save));
    string_indices.insert(std::pair<std::string, cmd_indices>("temp", i_temp));
    string_indices.insert(std::pair<std::string, cmd_indices>("as", i_as));
}

void CommandParser::getTokens(const std::string& command)
{
    this->command = command;
    t_command.clear();
    str_tokenizer.setBlock(this->command);

    while(str_tokenizer.hasNext())
    {
        Token token = str_tokenizer.getNextToken();
        if(token.getType() != TYPE_SPACE)
        {
            std::string str = token.getToken();
            std::transform(str.begin(), str.end(), str.begin(), std::tolower);
            t_command.push_back(str);
        }
    }
}

int CommandParser::getCmdIndex(const std::string& command)
{
    std::map<std::string, cmd_indices>::iterator i_cmd = string_indices.find(command);

    if(i_cmd != string_indices.end())
        return (*i_cmd).second;

    return i_rand_str;
}

std::vector<std::string> CommandParser::get_identifiers(int cmd)
{
    std::vector<std::string> returnMe;

    switch(cmd)
    {
    case i_select:
        returnMe.push_back(CMD_ID);
        returnMe.push_back(COL_ID);
        returnMe.push_back(TABLE_ID);
        returnMe.push_back(CONDITIONAL_ID);
        return returnMe;
    case i_insert:
    case i_create:
        returnMe.push_back(CMD_ID);
        returnMe.push_back(COL_ID);
        returnMe.push_back(TABLE_ID);
        return returnMe;
    case i_close:
    case i_load:
    case i_rebuild:
    case i_save:
        returnMe.push_back(CMD_ID);
        returnMe.push_back(TABLE_ID);
        return returnMe;
    case i_reset:
    case i_next:
    case i_exit:
        returnMe.push_back(CMD_ID);
        return returnMe;
    case i_goto:
        returnMe.push_back(CMD_ID);
        returnMe.push_back(CONDITIONAL_ID);
        return returnMe;
    default:
        return returnMe;
    }
}
