#include "file_assist_functions.h"

void open_file_RW(std::fstream& file, const char filename[])
{
    file.open(filename, std::fstream::in | std::fstream::out| std::fstream::binary );

    if (file.fail())
        throw("Error file failed to open file!");
}

void clear_open_file_W(std::fstream& file, const char filename[])
{
    file.open(filename, std::fstream::trunc | std::fstream::out | std::fstream::binary );

    if (file.fail())
        throw("Error file failed to open!");
}

void open_file_W(std::fstream& file, const char filename[])
{
    file.open(filename, std::fstream::out| std::fstream::binary );

    if (file.fail())
        throw("Error file failed to open!");
}

std::streampos g_goto_end(std::fstream &file)
{
    file.seekg(0, std::ios_base::end);
    return file.tellg();
}

std::streampos p_goto_end(std::fstream &file)
{
    file.seekp(0, std::ios_base::end);
    return file.tellp();
}
