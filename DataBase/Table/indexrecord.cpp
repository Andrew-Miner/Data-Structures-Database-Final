#include "indexrecord.h"
#include <cstring>

IndexRecord::IndexRecord()
{
    info.str[0] = '\0';
    info.key_count = 0;
    info.key_max = 32;
}

IndexRecord::IndexRecord(std::string str)
{
    for(int i = 0; i < MAX_STR_LENGTH; i++)
        info.str[i] = 'X';

    if(str.length() >= MAX_STR_LENGTH)
        std::strcpy(info.str, str.substr(0, MAX_STR_LENGTH-1).c_str());
    else
        std::strcpy(info.str, str.c_str());

    info.key_max = 32;
    info.key_count = 0;
}

IndexRecord::IndexRecord(std::fstream& file, std::streampos pos)
{
    info.key_max = 0;
    readData(file, pos);
}

IndexRecord::IndexRecord(std::fstream &file)
{
    info.key_max = 0;
    readData(file);
}

IndexRecord::IndexRecord(const IndexRecord& other)
{
    std::strcpy(info.str, other.info.str);
    info.key_count = other.info.key_count;
    info.key_max = other.info.key_max;

    for(int  i = 0; i < info.key_count; i++)
        keys[i] = other.keys[i];
}

IndexRecord& IndexRecord::operator=(const IndexRecord& other)
{
    std::strcpy(info.str, other.info.str);
    info.key_count = other.info.key_count;
    info.key_max = other.info.key_max;

    for(int  i = 0; i < info.key_count; i++)
        keys[i] = other.keys[i];

    return *this;
}

IndexRecord::~IndexRecord()
{

}

bool IndexRecord::push_back(int key)
{
    if(info.key_count == info.key_max)
        return false;

    keys[info.key_count++] = key;
    return true;
}

int IndexRecord::at(int index)
{
    if(index < info.key_count)
        return keys[index];
    else
        return -1;
}

void IndexRecord::remove(int index)
{
    if(index >= info.key_count)
        return;

    for(int i = index; i < info.key_count-1; i++)
        keys[i] = keys[i+1];

    info.key_count--;
}

std::streampos IndexRecord::readData(std::fstream &file)
{
    std::streampos pos = file.tellg();
    readInfo(file);
    readKeys(file);
    return pos;
}

std::streampos IndexRecord::readData(std::fstream& file, std::streampos pos)
{
    file.seekg(pos, std::ios_base::beg);
    return readData(file);
}

std::streampos IndexRecord::writeData(std::fstream& file)
{
    std::streampos pos = file.tellp();
    writeInfo(file);
    writeKeys(file);
    return pos;
}

std::streampos IndexRecord::writeData(std::fstream& file, std::streampos pos)
{
    file.seekp(pos, std::ios_base::beg);
    return writeData(file);
}

void IndexRecord::readInfo(std::fstream& file)
{
    file.read(reinterpret_cast<char*>(&info), sizeof(info));
}

void IndexRecord::writeInfo(std::fstream& file)
{
    file.write(reinterpret_cast<char*>(&info), sizeof(info));
}

void IndexRecord::readKeys(std::fstream& file)
{
    file.read(reinterpret_cast<char*>(keys), sizeof(int)*info.key_max);
}

void IndexRecord::writeKeys(std::fstream& file)
{
    file.write(reinterpret_cast<char*>(keys), sizeof(int)*info.key_max);
}
