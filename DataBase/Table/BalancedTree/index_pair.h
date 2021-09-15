#ifndef MAP_PAIR_H
#define MAP_PAIR_H
#include "array_assist_functions.h"
#include "../record.h"
#include <cstring>

class IndexPair
{
public:
    static const std::size_t MAX_RECORDS = 32;
    char key[Record::MAX_STR_LENGTH];
    std::size_t numb_used;
    std::streampos records[MAX_RECORDS];

    IndexPair():numb_used(0) { key[0] = '\0'; }
    IndexPair(const char key[]):numb_used(0) { std::strcpy(this->key, key); }
    IndexPair(const char key[], std::streampos record)
    {
        std::strcpy(this->key, key);
        records[0] = record;
        numb_used = 1;
    }

    bool operator+=(const IndexPair& other)
    {

        int i_start = numb_used;
        if(i_start + other.numb_used <= MAX_RECORDS)
        {
            for(int i = i_start; (i < MAX_RECORDS && i-i_start < other.numb_used); i++)
            {
                records[i] = other.records[i-i_start];
                numb_used++;
            }
            return true;
        }
        return false;
    }

    bool push_back(std::streampos pos)
    {
        if(numb_used < MAX_RECORDS)
        {
            records[numb_used++] = pos;
            return true;
        }
        return false;
    }

    std::streampos operator[](int index) const { return records[index]; }
    bool operator==(const IndexPair &other) const { return std::strcmp(key, other.key) == 0; }
    bool operator!=(const IndexPair &other) const { return std::strcmp(key, other.key) != 0; }
    bool operator<(const IndexPair &other) const { return std::strcmp(key, other.key) < 0; }
    bool operator<=(const IndexPair &other) const { return std::strcmp(key, other.key) <= 0; }
    bool operator>(const IndexPair &other) const { return std::strcmp(key, other.key) > 0; }
    bool operator>=(const IndexPair &other) const { return std::strcmp(key, other.key) >= 0; }

    friend std::ostream& operator<<(std::ostream& out, IndexPair printMe)
    {
        out << '[' << printMe.key << "|" << printMe.numb_used << ']';
    }
};

template <typename T>
class AddIndices
{
public:
    bool operator()(T &existing_item, const T &new_item)
    {
        if(existing_item += new_item)
            return false;
        else
            return true;
    }
};

#endif // MAP_PAIR_H
