#ifndef FILE_ASSIST_FUNCTIONS_H
#define FILE_ASSIST_FUNCTIONS_H
#include <fstream>
#include <iostream>

// ============================ Declarations =============================
void open_file_RW(std::fstream& file, const char filename[]);

void clear_open_file_W(std::fstream& file, const char filename[]);

void open_file_W(std::fstream& file, const char filename[]);

std::streampos g_goto_end(std::fstream &file);
std::streampos p_goto_end(std::fstream &file);

#endif // FILE_ASSIST_FUNCTIONS_H
