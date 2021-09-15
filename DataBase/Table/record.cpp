#include "record.h"
#include <cstring>
#include <iomanip>

Record::Record(std::streampos uid): unique_id(uid), is_deleted(false)
{
    for(int col = 0; col < MAX_COLUMN_SIZE; col++)
        for(int c = 0; c < MAX_STR_LENGTH; c++)
            this->record[col][c] = '\0';
}

Record::Record(std::streampos uid, std::vector<std::string> data): unique_id(uid), is_deleted(false)
{
    for(int col = 0; col < MAX_COLUMN_SIZE; col++)
        for(int c = 0; c < MAX_STR_LENGTH; c++)
            this->record[col][c] = '\0';

    int col = 0;
    for(std::string str : data)
    {
        if(str.length() > MAX_STR_LENGTH)
            std::strcpy(record[col++], str.substr(0, MAX_STR_LENGTH-1).c_str());
        else
            std::strcpy(record[col++], str.c_str());

        if(col > MAX_COLUMN_SIZE)
            return;
    }
}

Record::Record(std::streampos uid, char list[][MAX_STR_LENGTH], int str_count): unique_id(uid), is_deleted(false)
{
    for(int col = 0; col < MAX_COLUMN_SIZE; col++)
        for(int c = 0; c < MAX_STR_LENGTH; c++)
            this->record[col][c] = '\0';

    if(str_count > MAX_COLUMN_SIZE)
        str_count = MAX_COLUMN_SIZE;

    int col;
    for(col = 0; col < str_count; col++)
        std::strcpy(record[col], list[col]);
}

void Record::removeValue(int rec_number)
{
    record[rec_number][0] = '\0';
}

void Record::setValue(int rec_number, const std::string& value)
{
    std::strcpy(record[rec_number], value.c_str());
}

std::string Record::getValue(int rec_number) const
{
    return std::string(record[rec_number]);
}

std::vector<std::string> Record::getList() const
{
    std::vector<std::string> list;

    for(int col = 0; col < MAX_COLUMN_SIZE; col++)
    {
        if(record[col][0] != '\0')
            list.push_back(std::string(record[col]));
    }

    return list;
}

std::vector<std::string> Record::getList(int length) const
{
    std::vector<std::string> list;

    for(int col = 0; col < length; col++)
        list.push_back(std::string(record[col]));

    return list;
}

std::streampos Record::load(std::fstream& file_in, std::streampos UID)
{
    file_in.seekg(UID, std::ios_base::beg);
    file_in.read(reinterpret_cast<char*>(this), sizeof(Record));
    return UID;
}

std::streampos Record::save(std::fstream& file_out)
{
    file_out.seekp(unique_id, std::ios_base::beg);
    //file_out.seekp(0, std::ios_base::end);
    file_out.write(reinterpret_cast<char*>(this), sizeof(Record));
    file_out.flush();
    return file_out.tellp();
}

std::string Record::operator[](int index) const
{
    return std::string(record[index]);
}

std::ostream& operator<<(std::ostream& out, const Record& record)
{
    std::string temp = "UID:";
    temp += std::to_string(record.unique_id) + " isDeleted: " + std::to_string(record.is_deleted) + " Data: ";
    out << temp;

    std::vector<std::string> list = record.getList();

    if(list.size())
    {
        out << "[0]" << list.at(0) << std::endl;

        for(int i = 1; i < list.size(); i++)
            out << std::setw(temp.size()+1) << "[" << i << "]" << list.at(i) << std::endl;
    }
    return out;
}
