#ifndef INDEXFILE_H
#define INDEXFILE_H
#include "indexrecord.h"
#include "databuffer.h"
#include <map>

class IndexFile
{
public:
    struct page_header
    {
        char page;
        int index_count;
        int key_limit;
    };

    IndexFile(const std::string& file_name);
    ~IndexFile();

    void insertIndex(const std::string& value, int key);
    void removeIndex(const std::string& value, int key);
    IndexRecord* getIndex(const std::string& index);

    DataBuffer<IndexRecord>* getPage(char page);
    DataBuffer<IndexRecord>& getAllIndices();

    DataBuffer<IndexRecord>& getBuffer() { return buffer; }

    std::streampos length();
    int getPageCount() { return page_index.size(); }

private:
    std::map<char, std::pair<int, page_header>> page_index;
    std::fstream index_file;

    char current_page;
    DataBuffer<IndexRecord> buffer;

    void open_file(std::string file_name);
    void build_file_index();
    std::streampos getPageEnd(std::streampos page_pos, page_header header);
    int searchBuffer(int start, int end, const std::string& value);

    bool insert_new_index(const std::string& value, int key);
    void insert_new_page(char page);

    int potential_index(int start, int end, const std::string& value);
};

#endif // INDEXFILE_H
