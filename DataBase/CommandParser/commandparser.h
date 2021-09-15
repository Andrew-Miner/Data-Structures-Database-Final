#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H
#include "ParseTree/parsetree.h"
#include "STokenizer/stokenizer.h"
#include <map>
#include <string>

// Constants Used To Identify Tokens and Build
// the parse tree
const std::string CMD_ID = "command";
const std::string COL_ID = "columns";
const std::string TABLE_ID = "tables";

const std::string CONDITIONAL_ID = "conditionals";

// The ID Const are never used
// and intended for conditional implementation
const std::string LST_ID = "<";
const std::string LSTE_ID = "<=";
const std::string EQ_ID = "==";
const std::string NEQ_ID = "!=";
const std::string GRT_ID = ">";
const std::string GRTE_ID = ">=";

const std::string CMD_SELECT = "select";
const std::string CMD_CREATE = "create";
const std::string CMD_INSERT = "insert";
const std::string CMD_DROP = "drop";
const std::string CMD_LOAD = "load";
const std::string CMD_EXIT = "exit";
const std::string CMD_NEXT = "next";
const std::string CMD_RESET = "reset";
const std::string CMD_GOTO = "goto";
const std::string CMD_CLOSE = "close";
const std::string CMD_REBUILD = "rebuild";
const std::string CMD_SAVE = "save";

// Enum representing column indices
// in the state_array
enum cmd_indices
{
    i_select = 0,
    i_from,
    i_where,
    i_comma,
    i_and,
    i_star,
    i_rand_str,
    i_create,
    i_table,
    i_load,
    i_exit,
    i_insert,
    i_into,
    i_values,
    i_next,
    i_reset,
    i_goto,
    i_close,
    i_rebuild,
    i_less_than,
    i_less_equal,
    i_equal,
    i_not_equal,
    i_greater_than,
    i_greater_equal,
    i_save,
    i_as,
    i_temp,
    enum_end
};

class CommandParser
{
public:

    // Defualt Constructor/String Constructor
    CommandParser(std::string = std::string());

    ~CommandParser();

    // Function that actually does the parsing
    PTree parseCommand();

    // Function That Sets the command to be parsed
    void setCommand(const std::string& command);

    // Function that returns the command
    std::string getCommand();


private:

    const static int DEFAULT_MAX_ROWS = 30;
    const static int DEFAULT_MAX_COLS = 30;

    /* Description:
     *      Map used to convert string tokens into
     *      state_array column indices
     *
     * Pair:
     *      string - valid column string
     *      cmd_indices - integer from enum defined
     *                    above that represents a
     *                    column index in state_array
     */
    std::map<std::string, cmd_indices> string_indices;

    /* Description:
     *      Map that holds the indices of state_array
     *      rows used to add tokens to the parse tree
     *
     * Pair:
     *      int - state_array row
     *      string - identifier used to insert
     *               into the parse tree
     */
    std::map<int, std::string> row_insertion_identifier;

    // Vector used to identify success rows
    std::vector<int> success_rows;
    std::vector<int> rows_maintain_order;

    // State Array Pointer
    int** state_array;
    int max_rows, max_cols;

    // Vector Contained Tokens Built From Command String
    std::vector<std::string> t_command;
    STokenizer str_tokenizer;

    // Actual Command
    std::string command;

    // Function Used To Allocate/Initialize State Array
    // NOTE: In order to add more commands you must edit
    //       this function.
    void build_state_array();

    // Function used to initialize string_indices map
    // NOTE: In order to add more commands you must edit
    //       this function.
    void build_string_indices();

    // Function Used To Tokenize command into t_command
    // NOTE: In order to add more commands you must edit
    //       this function definition.
    void getTokens(const std::string& command);

    // Function Used To Get State Array Column Indices From String
    int getCmdIndex(const std::string& command);

    // Function Used To Get A List Of Identifiers From a Command
    // that are needed to intialize the parse tree
    std::vector<std::string> get_identifiers(int cmd);

};

#endif // COMMANDPARSER_H
