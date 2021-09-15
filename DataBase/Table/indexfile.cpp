#include "indexfile.h"
#include "../AssistFunctions/file_assist_functions.h"
#include <iostream>

IndexFile::IndexFile(const std::string& file_name): buffer(nullptr, 1024 * 1024 * 1024)
{
    open_file(file_name);
    buffer.setFile(&index_file);
    build_file_index();
}

IndexFile::~IndexFile()
{
    index_file.close();
}

void IndexFile::insertIndex(const std::string& value, int key)
{
    if(page_index.size())
    {
        std::map<char, std::pair<int, page_header>>::iterator it = page_index.find(value[0]);

        if(it != page_index.end())
        {
            IndexRecord* record = getIndex(value);

            if(record == nullptr)
                insert_new_index(value, key);
            else if(record->push_back(key))
                buffer.writeBlock();
            else
                std::cout << "[IndexFile] Error: Need More Key Space In Index \"" << value << "\"" << std::endl;
            return;
        }
    }

    insert_new_page(value[0]);
    insert_new_index(value, key);
}

void IndexFile::removeIndex(const std::string& value, int key)
{

}

DataBuffer<IndexRecord>* IndexFile::getPage(char page)
{
    std::map<char, std::pair<int, page_header>>::iterator it = page_index.find(page);

    if(it == page_index.end())
        return nullptr;

    buffer.readBlock((*it).first, getPageEnd((*it).second.first, (*it).second.second));
    return &buffer;
}

IndexRecord* IndexFile::getIndex(const std::string& index)
{
    std::map<char, std::pair<int, page_header>>::iterator it = page_index.find(index[0]);

    if(it != page_index.end())
    {
        if(index[0] != current_page || index < buffer[0])
            buffer.readBlock((*it).first, getPageEnd((*it).second.first, (*it).second.second));

        current_page = index[0];
        int record = searchBuffer(0, buffer.size()-1, index);

        while(record == -1 && buffer.readBlock())
            record = searchBuffer(0, buffer.size()-1, index);

        if(record == -1)
            return nullptr;
        return &buffer[record];
    }
    return nullptr;
}

DataBuffer<IndexRecord>& IndexFile::getAllIndices()
{
    DataBuffer<IndexRecord> buf(0);
    return buf;
}

void IndexFile::build_file_index()
{
    //if(length() == 0)
    //return;

    page_header current_header;
    int current_offset = 0;

    if(!index_file.good())
        index_file.clear();

    index_file.seekg(0, std::ios_base::beg);
    index_file.read(reinterpret_cast<char*>(&current_header), sizeof(page_header));

    while(!index_file.eof())
    {
        page_index.insert(std::pair<char, std::pair<int, page_header>>(current_header.page, std::pair<int, page_header>(current_offset+sizeof(page_header), current_header)));

        // Current POS + ( numbOfIndices * (sizeof each index) )
        current_offset = getPageEnd(current_offset+sizeof(page_header), current_header);
        index_file.seekg(current_offset, std::ios_base::beg);

        index_file.read(reinterpret_cast<char*>(&current_header), sizeof(page_header));
    }

    if(!index_file.good())
        index_file.clear();

    index_file.seekg(0, std::ios_base::beg);
}

std::streampos IndexFile::getPageEnd(std::streampos page_pos, page_header header)
{
    return page_pos + static_cast<std::streampos>(header.index_count*(header.key_limit*sizeof(int) + sizeof(index_info)));
}

void IndexFile::open_file(std::string file_name)
{
    try
    {
        open_file_RW(index_file, file_name.c_str());
    }
    catch (const char* msg)
    {
        try {

            open_file_W(index_file, file_name.c_str());
            index_file.close();
            open_file_RW(index_file, file_name.c_str());

        } catch(const char* msg) {
            throw("[IndexFile] Failed To Create New File!");
        }
    }
}

int IndexFile::searchBuffer(int start, int end, const std::string& value)
{
    if(start > end)
        return -1;

    int mid = start + (end - start)/2;

    if(value == buffer[mid])
        return mid;

    if(value < buffer[mid])
        return searchBuffer(start, mid, value);
    else
        return searchBuffer(mid, end, value);
}

bool IndexFile::insert_new_index(const std::string& value, int key)
{
    std::cout << "Insert New Index\n";
    std::map<char, std::pair<int, page_header>>::iterator it = page_index.find(value[0]);

    if(it == page_index.end())
        return false;

    std::cout << "Found Page\n";

    std::streampos pg_end = getPageEnd((*it).second.first, (*it).second.second);
    buffer.readBlock((*it).second.first, pg_end);

    std::cout << "Read Page\n";

    while(value > buffer[buffer.size()-1] && buffer.getNextPosition() > pg_end)
        buffer.readBlock();

    std::cout << "Found Page Section\n";

    int pi = potential_index(0, buffer.size()-1, value);

    std::cout << "Found Potential Index\n";

    std::streampos readLimit = buffer.getPosition() + static_cast<std::streampos>(pi*sizeof(IndexRecord));

    std::cout << "Found Read Limit: " << readLimit << std::endl;

    if(buffer.getDirection() != buffer.DIR_BACKWARD)
        buffer.invertDirection();

    std::cout << "Read Direction Inverted\n";

    if(buffer.readBlock(length(), readLimit))
    {
        buffer.writeBlock(buffer.getPosition() + static_cast<std::streampos>(sizeof(IndexRecord)));

        while(buffer.readBlock())
            buffer.writeBlock(buffer.getPosition() + static_cast<std::streampos>(sizeof(IndexRecord)));
    }

    std::cout << "Pushed File Back\n";

    IndexRecord new_rec(value);
    new_rec.push_back(key);
    new_rec.writeData(index_file, readLimit);

    std::cout << "Wrote New Record Index\n";

    (*it).second.second.index_count++;
    index_file.seekp((*it).first-sizeof(page_header), std::ios_base::beg);
    index_file.write(reinterpret_cast<char*>(&((*it).second.second)), sizeof(page_header));
    std::cout << "Updated Page Header\n";

    buffer.invertDirection();
    std::cout << "Inverted Read Direction\n";

    return true;
}

void IndexFile::insert_new_page(char page)
{
    std::cout << "Insert New Page:" << sizeof(page_header) << " " << index_file.tellp() << " " << index_file.tellg() << "\n";

    index_file.seekp(0, std::ios_base::end);
    page_header new_header;
    new_header.page = page;
    new_header.key_limit = 32;
    new_header.index_count = 0;

    index_file.write(reinterpret_cast<char*>(&new_header), sizeof(page_header));
    index_file.flush();

    page_index.insert(std::pair<char, std::pair<int, page_header>>(page, std::pair<int, page_header>(index_file.tellp(), new_header)));
    std::cout << "Pages: " << page_index.size() << "\n";
}

std::streampos IndexFile::length()
{
    index_file.seekg(0, std::ios_base::end);
    std::streampos file_length = index_file.tellg();
    index_file.seekg(0, std::ios_base::beg);

    if(file_length == -1)
        return 0;

    return file_length;
}

int IndexFile::potential_index(int start, int end, const std::string& value)
{
    if(start > end)
        return -1;

    int mid = start + (end - start)/2;

    if(value == buffer[mid])
        return mid;

    if(value < buffer[mid])
    {
        if(mid > 0 && value > buffer[mid-1])
            return mid;
        else if(start > mid)
            return mid;

        return potential_index(start, mid, value);
    }
    else
    {
        if(mid < buffer.size()-1 && value < buffer[mid+1])
            return mid+1;
        else if(start > mid)
            return mid+1;

        return potential_index(mid, end, value);
    }
}
