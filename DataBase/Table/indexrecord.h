#ifndef INDEXRECORD_H
#define INDEXRECORD_H
#include <vector>
#include <fstream>
#include <cString>

static const int MAX_STR_LENGTH = 64;

struct index_info
{
    char str[MAX_STR_LENGTH];
    int key_max;
    int key_count;

};

class IndexRecord
{
public:

    IndexRecord();
    IndexRecord(std::string str);
    IndexRecord(std::fstream& file, std::streampos pos);
    IndexRecord(std::fstream &file);

    IndexRecord(const IndexRecord& other);
    IndexRecord& operator=(const IndexRecord& other);
    ~IndexRecord();

    std::streampos readData(std::fstream &file);
    std::streampos readData(std::fstream& file, std::streampos pos);

    std::streampos writeData(std::fstream& file);
    std::streampos writeData(std::fstream& file, std::streampos pos);

    bool  push_back(int key);
    int at(int index);
    void remove(int index);

    index_info& getInfo() { return info; }

    friend bool operator==(const std::string& str, const IndexRecord& rec) { return std::strcmp(rec.info.str, str.c_str()) == 0; }
    friend bool operator<(const std::string& str, const IndexRecord& rec) { return std::strcmp(rec.info.str, str.c_str()) < 0; }
    friend bool operator>(const std::string& str, const IndexRecord& rec) { return std::strcmp(rec.info.str, str.c_str()) > 0; }

private:
    index_info info;
    int keys[32];

    void readInfo(std::fstream& file);
    void writeInfo(std::fstream& file);

    void readKeys(std::fstream& file);
    void writeKeys(std::fstream& file);
};

#endif // INDEXRECORD_H
