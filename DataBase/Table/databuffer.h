#ifndef DATABUFFER_H
#define DATABUFFER_H
#include <fstream>
#include <iostream>


template<typename data_type>
class DataBuffer
{
public:
    static const bool DIR_FORWARD = 1;
    static const bool DIR_BACKWARD = 0;
    static const int GIGABYTE = 1073741824;
    const int MAX_DATA_COUNT;

    DataBuffer(int max_memeory_usage_bytes = GIGABYTE/1024);
    DataBuffer(std::fstream* file, int max_memeory_usage_bytes = GIGABYTE/1024);
    DataBuffer(std::fstream* file, std::streampos pos, int max_memory_usage_bytes);
    DataBuffer(std::fstream* file, std::streampos start, std::streampos end, int max_memory_usage_bytes);

    DataBuffer(const DataBuffer& other);
    DataBuffer& operator=(const DataBuffer& other);
    ~DataBuffer();

    bool readBlock(std::streampos pos = -1, std::streampos end = -1);
    std::streampos writeBlock(std::streampos pos = -1);
    std::streampos writeChunk(data_type *chunk, std::size_t chunk_size, std::streampos pos);

    data_type* const getBlock() { return objects; }
    void invertDirection() { read_dir = !read_dir; }
    bool getDirection() { return read_dir; }

    int size() const { return obj_count; }
    data_type& operator[](int index);
    data_type at(int index) const { return objects[index]; }

    void setReadLimit(std::streampos limit) { read_limit = limit; }
    std::streampos getReadLimit() { return read_limit; }

    bool isValid() { return file; }

    void setFile(std::fstream* file);

    std::streampos getPosition() { return last_block; }
    std::streampos getNextPosition() { return next_block; }

private:
    std::fstream* file;
    std::streampos last_block, next_block, read_limit;
    bool read_dir, hit_end;

    data_type* objects;
    int obj_count;

    std::streampos read_forward(std::streampos start, std::streampos end);
    std::streampos read_backward(std::streampos start, std::streampos end);
};

template <typename data_type>
DataBuffer<data_type>::DataBuffer(int max_memeory_usage_bytes)
: MAX_DATA_COUNT(max_memeory_usage_bytes/sizeof(data_type)), file(nullptr), read_dir(DIR_FORWARD), read_limit(-1), last_block(-1), next_block(-1)
{
    objects = new data_type[MAX_DATA_COUNT];
}

template <typename data_type>
DataBuffer<data_type>::DataBuffer(std::fstream* file, int max_memory_usage_bytes)
: MAX_DATA_COUNT(max_memory_usage_bytes/sizeof(data_type)), file(file), read_dir(DIR_FORWARD), read_limit(-1), last_block(-1), next_block(-1)
{
    objects = new data_type[MAX_DATA_COUNT];

    if(file)
        next_block = file->tellg();

    //readBlock();
}

template <typename data_type>
DataBuffer<data_type>::DataBuffer(std::fstream* file, std::streampos pos, int max_memory_usage_bytes)
: MAX_DATA_COUNT(max_memory_usage_bytes/sizeof(data_type)), read_dir(DIR_FORWARD), read_limit(-1), last_block(-1), next_block(pos)
{
    this->file = file;
    objects = new data_type[MAX_DATA_COUNT];
    //readBlock();
}

template <typename data_type>
DataBuffer<data_type>::DataBuffer(std::fstream* file, std::streampos pos, std::streampos end, int max_memory_usage_bytes)
: MAX_DATA_COUNT(max_memory_usage_bytes/sizeof(data_type)), read_dir(DIR_FORWARD), read_limit(end), last_block(-1), next_block(pos)
{
    this->file = file;
    objects = new data_type[MAX_DATA_COUNT];
    //readBlock();
}

template <typename data_type>
DataBuffer<data_type>::DataBuffer(const DataBuffer& other)
: MAX_DATA_COUNT(other.MAX_DATA_COUNT), file(other.file), read_dir(other.read_dir), read_limit(other.read_limit),
  last_block(other.last_block), next_block(other.next_block), obj_count(other.obj_count)
{
    delete [] objects;
    objects = new data_type[MAX_DATA_COUNT];

    for(int i = 0; i < obj_count; i++)
        objects[i] = objects[i];
}

template <typename data_type>
void DataBuffer<data_type>::setFile(std::fstream* file)
{
    this->file = file;

    if(next_block == -1)
        next_block = file->tellg();

    //readBlock();
}

template <typename data_type>
DataBuffer<data_type>& DataBuffer<data_type>::operator=(const DataBuffer& other)
{
    MAX_DATA_COUNT = other.MAX_DATA_COUNT;
    file = other.file;
    last_block = other.last_block;
    next_block = other.next_block;
    read_dir = other.read_dir;


    delete [] objects;
    objects = new data_type[MAX_DATA_COUNT];
    obj_count = other.obj_count;

    for(int i = 0; i < obj_count; i++)
        objects[i] = objects[i];

    return *this;
}

template <typename data_type>
DataBuffer<data_type>::~DataBuffer()
{
    delete [] objects;
}

template <typename data_type>
bool DataBuffer<data_type>::readBlock(std::streampos pos, std::streampos end)
{
    if(!file)
        return false;

    if(end != -1 && end != read_limit)
        read_limit = end;

    if(pos == -1)
        pos = next_block;

    std::streampos cur_pos;

    if(read_dir == DIR_FORWARD)
    {
        cur_pos = read_forward(pos, read_limit);

        if(cur_pos == -1)
            return false;

        last_block = pos;
        next_block = cur_pos;
    }
    else
    {
        cur_pos = read_backward(pos, read_limit);

        if(cur_pos == -1)
            return false;

        last_block = cur_pos;
        next_block = cur_pos;
    }

    if(obj_count == 0)
        return false;

    return true;
}

template <typename data_type>
std::streampos DataBuffer<data_type>::writeBlock(std::streampos pos)
{
    if(!file)
        return -1;

    if(!obj_count)
        return -1;

    if(pos == -1)
        pos = last_block;

    if(file->tellp() != pos)
        file->seekp(pos, std::ios_base::beg);

    file->write(reinterpret_cast<char*>(objects), obj_count);
    file->flush();

    if(!(file->good()))
    {
        file->clear();
        file->seekg(0, std::ios_base::beg);
    }

    return file->tellp();
}

template <typename data_type>
data_type& DataBuffer<data_type>::operator[](int index)
{
    return objects[index];
}

template <typename data_type>
std::streampos DataBuffer<data_type>::read_forward(std::streampos start, std::streampos end)
{

    std::streampos read_size = sizeof(data_type)*MAX_DATA_COUNT;

    if(file->tellg() != start)
        file->seekg(start, std::ios_base::beg);

    if(end != -1)
    {
        if(start > end)
            return -1;

        if(start + read_size > end)
            read_size = ( (read_limit-start) / sizeof(data_type) ) * sizeof(data_type);
    }

    file->read(reinterpret_cast<char*>(objects), read_size);
    obj_count = file->gcount()/sizeof(data_type);

    if(!(file->good()))
        file->clear();

    return file->tellg();
}

template <typename data_type>
std::streampos DataBuffer<data_type>::read_backward(std::streampos start, std::streampos end)
{
    std::streampos read_size = sizeof(data_type)*MAX_DATA_COUNT;

    if(end != -1)
    {
        if(start < end)
            return -1;

        if(start - read_size < end)
            read_size = ( (start-end) / sizeof(data_type) ) * sizeof(data_type);
    }

    std::streampos real_start = start - read_size;

    if(real_start < 0)
    {
        read_size += real_start;
        real_start = 0;
    }

    if(file->tellg() != real_start)
        file->seekg(real_start, std::ios_base::beg);

    file->read(reinterpret_cast<char*>(objects), read_size);
    obj_count = file->gcount()/sizeof(data_type);

    if(!(file->good()))
        file->clear();

    return real_start;
}

template <typename data_type>
std::streampos DataBuffer<data_type>::writeChunk(data_type *chunk, std::size_t chunk_size, std::streampos pos)
{
    if(!file)
        return -1;

    if(chunk_size == 0)
        return -1;

    if(file->tellp() != pos)
        file->seekp(pos, std::ios_base::beg);

    file->write(reinterpret_cast<char*>(chunk), chunk_size);
    file->flush();
    return file->tellp();
}

#endif // DATABUFFER_H
