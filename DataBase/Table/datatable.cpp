#include "datatable.h"
//#include <unistd.h>

const std::size_t SELECTION_CPY_SIZE = 5000;

DataTable::DataTable(const std::string &dir, const std::string &table_file, const std::size_t &mem_alloc_mb): records(nullptr, mem_alloc_mb * 1024 * 1024), record_size_bytes(0)
{
    std::string file = table_file;
    add_extension(file);
    table_name = file;

    if(!open_table(dir, file))
        throw ("Error: Failed To Load Table!");
}

DataTable::DataTable(const std::string &dir, const std::string &table_name, const std::vector<std::string> &columns, const std::size_t &mem_alloc_mb): records(nullptr, mem_alloc_mb * 1024 * 1024), column_count(columns.size()), record_size_bytes(0)
{
    std::string file = table_name;
    add_extension(file);
    this->table_name = file;

    if(!create_table(dir, file, columns))
        throw ("Error: Failed To Create Table!");
}

DataTable::DataTable(const std::string &dir, const std::string &table_name, DataTable* other): column_count(other->column_count), records(nullptr, other->records.MAX_DATA_COUNT * sizeof(Record))
{
    this->table_name = table_name;
    add_extension(this->table_name);

    std::cout << "HEREWEGOOOOOOO" << std::endl;

    if(!create_table(dir, this->table_name, other->columns))
        throw ("Error Failed To Create Table!");

    other->resetBuffer();
    std::streampos write_pos = rec_start;

    std::cout << "Did we crash yet?\n";

    do
    {
        write_pos = records.writeChunk(other->getBuffer()->getBlock(), sizeof(Record) * other->getBuffer()->size(), write_pos);
    } while(other->getBuffer()->readBlock());

    std::cout << "WOA WAO WAO\n";
    record_size_bytes = other->record_size_bytes;

    rebuildIndexedTrees();
}

DataTable::~DataTable()
{
    for(IndexTree* it_ptr : index_tables)
        delete it_ptr;
}

bool DataTable::open_table(const std::string &dir, const std::string &file_name)
{
    std::string file_w_dir = dir + "/" + file_name;

    try { open_file_RW(table, file_w_dir.c_str()); }
    catch (const char* msg) { return false; }

    table.seekg(0, std::ios_base::beg);
    table.read(reinterpret_cast<char*>(&column_count), sizeof(column_count));
    char str[Record::MAX_STR_LENGTH];

    for(int i = 0; i < column_count; i++)
    {
        table.seekg(sizeof(column_count) + i*sizeof(char)*Record::MAX_STR_LENGTH, std::ios_base::beg);
        table.read(str, sizeof(str));

        std::string temp_str(str);
        column_indices.insert(std::pair<std::string, int>(temp_str, i));
        columns.push_back(temp_str);

        std::string name = file_name.substr(0, file_name.size() - std::strlen(record_extension));
        temp_str = dir + "/" + name + "_" + temp_str + "_index_table" + index_extension;

        index_tables.push_back(new IndexTree(temp_str));

        if(!(index_tables.at(i)->is_open()))
            return false;

        std::cout << " IndexedTree Loaded: " << temp_str << std::endl;
    }
    std::cout << std::endl;

    rec_start = table.tellg();

    record_size_bytes = g_goto_end(table) - rec_start;
    table.seekg(rec_start, std::ios_base::beg);

    records.setFile(&table);
    records.readBlock(rec_start);

    return true;
}

bool DataTable::create_table(const std::string &dir, const std::string &file_name, const std::vector<std::string> &columns)
{
    std::string file_w_dir = dir + "/" + file_name;

    try
    {
        clear_open_file_W(table, file_w_dir.c_str());
        table.close();
        open_file_RW(table, file_w_dir.c_str());
    } catch(const char* msg) { return false; }

    table.seekp(0, std::ios_base::beg);
    table.write(reinterpret_cast<char*>(&column_count), sizeof(std::size_t));
    table.flush();

    char cstr[Record::MAX_STR_LENGTH];

    for(int i = 0; i < column_count; i++)
    {
        std::string str(columns.at(i));
        if(str.length() > Record::MAX_STR_LENGTH)
            std::strcpy(cstr, str.substr(0, Record::MAX_STR_LENGTH-1).c_str());
        else
            std::strcpy(cstr, str.c_str());

        table.write(cstr, sizeof(cstr));
        table.flush();

        column_indices.insert(std::pair<std::string, int>(str, i));
        this->columns.push_back(str);

        std::string name = file_name.substr(0, file_name.size() - std::strlen(record_extension));
        str = dir + "/" + name + "_" + str + "_index_table" + index_extension;

        index_tables.push_back(new IndexTree(str, true));

        if(!(index_tables.at(i)->is_open()))
            return false;

        std::cout << " IndexedTree Created: " << str << std::endl;
    }
    std::cout << std::endl;

    rec_start = table.tellp();
    records.setFile(&table);
    records.readBlock(rec_start);
    record_size_bytes = 0;

    return true;
}

void DataTable::add_extension(std::string &table_name)
{
    if(table_name.rfind(record_extension) == std::string::npos)
        table_name += record_extension;
}

bool DataTable::InsertRecord(const std::vector<std::string> &rec)
{
    if(rec.size() != column_count)
        return false;

    table.seekp(0, std::ios_base::end);
    Record rcrd(table.tellp(), rec);
    record_size_bytes += sizeof(rcrd);

    rcrd.save(table);
    for(int i = 0; i < column_count; i++)
        index_tables.at(i)->insert(IndexPair(rcrd[i].c_str(), rcrd.getUID()));

    return true;
}

Record DataTable::GetRecord(const std::streampos &UID)
{
    Record temp(UID);
    temp.load(table);
    return temp;
}

void DataTable::OverwriteRecord(Record rec)
{
    rec.save(table);
}

std::ostream& operator<<(std::ostream &out,  const DataTable &table)
{
    int cell_width = 16;
    for(int i = 0; i < (table.columns.size()+1)*(cell_width+1); i++)
        out << "=";
    out << "=" << std::endl;

    out << "| UID            ";

    for(std::vector<std::string>::const_iterator it = table.columns.begin(); it != table.columns.end(); ++it)
    {
        out << "| " << *it;
        for(int i = 0; i < cell_width - (1+(*it).length()); i++)
            out << " ";
    }
    out << "|\n";

    for(int i = 0; i < (table.columns.size()+1)*(cell_width+1); i++)
        out << "=";
    out << "=" << std::endl;

    int rec_count = table.records.size();
    for(int i = 0; i < rec_count; i++)
    {
        std::vector<std::string> rec = table.records.at(i).getList();
        rec.insert(rec.begin(), std::to_string(table.records.at(i).getUID()/2));
        table.print_row(out, rec, cell_width);
    }

    return out;
}

std::ostream& DataTable::print_row(std::ostream &out, const std::vector<std::string> &values, int &cell_size)
{
    for(std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it)
    {
        out << "| " << *it;
        for(int i = 0; i < cell_size - (1+(*it).length()); i++)
            out << " ";
    }
    out << "|\n";

    for(int j = 0; j < values.size(); j++)
    {
        out << '|';
        for(int i = 0; i < cell_size; i++)
            out << '-';
    }
    out << "|\n";

    return out;
}

DataTable* DataTable::makeSelection(std::string temp_dir, std::vector<std::string> cols, std::vector<std::string> conditions)
{
    if(cols.size() == 0)
        throw ("Error: No Columns Selected!");

    std::string sort_by = cols.at(0);
    std::vector<int> copy_cols;

    for(int i = 0; i < cols.size(); i++)
    {
        std::map<std::string, int>::iterator it = column_indices.find(cols.at(i));

        if(it != column_indices.end())
            copy_cols.push_back(it->second);
        else
            throw ("Error: Invalid Column Selection!");
    }


    std::map<std::string, int>::iterator it = column_indices.find(sort_by);
    DataTable* temp_table = new DataTable(temp_dir, "temp", cols, 2);
    IndexTree* i_tree = index_tables.at(it->second);

    Record *rcr_chunk = new Record[SELECTION_CPY_SIZE];
    int rcr_count = 0;

    std::streampos write_pos = temp_table->getStartingByte();
    std::streampos rec_uid = write_pos;

    int rcr_total = 0;
    //std::vector<IndexTree*> indexed_trees = temp_table->getIndexedTrees();

    for(IndexTree::iterator it = i_tree->begin(); it != i_tree->end(); ++it)
    {
        for(int i = 0; i < (*it).numb_used; i++)
        {
            std::vector<std::string> rec = (this->GetRecord((*it).records[i])).getList();
            std::vector<std::string> new_rec;

            for(std::vector<int>::iterator itr = copy_cols.begin(); itr != copy_cols.end(); ++itr)
                new_rec.push_back(rec.at(*itr));

            Record record(rec_uid, new_rec);

            //for(int i = 0; i < new_rec.size(); i++)
                //indexed_trees.at(i)->insert(IndexPair(new_rec.at(i).c_str(), rec_uid));

            rec_uid += sizeof(Record);

            rcr_chunk[rcr_count++] = record;

            if(rcr_count == SELECTION_CPY_SIZE)
            {
                std::cout << "=========================================================" << std::endl;
                for(int j = 0; j < rcr_count; j++)
                    std::cout << j << "| " << rcr_chunk[j] << std::endl;
                std::cout << std::endl;

                rcr_total += rcr_count;

                write_pos = temp_table->getBuffer()->writeChunk(rcr_chunk, sizeof(Record)*rcr_count, write_pos);
                temp_table->record_size_bytes += sizeof(Record)*rcr_count;
                rcr_count = 0;
            }

        }
    }

    if(rcr_count)
    {
        std::cout << "=========================================================" << std::endl;
        for(int j = 0; j < rcr_count; j++)
            std::cout << j << "| " << rcr_chunk[j] << std::endl;
        std::cout << std::endl;

        rcr_total += rcr_count;

        write_pos = temp_table->getBuffer()->writeChunk(rcr_chunk, sizeof(Record)*rcr_count, write_pos);
        temp_table->record_size_bytes += sizeof(Record)*rcr_count;
        rcr_count = 0;
    }

    std::cout << "TOTAL: " << rcr_total << std::endl;

    return temp_table;

}

void DataTable::rebuildIndexedTrees()
{
    for(std::vector<IndexTree*>::iterator it = index_tables.begin(); it != index_tables.end(); ++it)
        (*it)->clear();

    resetBuffer();

    std::cout << "Indexed Trees Cleared!\nDataBuffer Reset!\n\n";

    int chunk_count = 0;

    do
    {
        chunk_count++;
        std::cout << "Looping Through Record Chunk: " << chunk_count << std::endl;
        for(int i = 0; i <records.size(); i++)
        {
            std::cout << "Grabbing Record " << i << " of " << records.size() << std::endl;
            Record rcrd = records[i];

            std::cout << " Getting Record's List " << std::endl;

            std::vector<std::string> rec = rcrd.getList();//records[i].getList();

            for(int j = 0; j < rec.size(); j++)
                std::cout << rec[j] << " ";
            std::cout << " | Successfully Grabbed | ";

            for(int j = 0; j < rec.size(); j++)
            {
                //std::cout << "Inserting Col " << j << " Of Row " << i << std::endl;
                //std::cout << "index_tables size: " << index_tables.size() << std::endl;
                index_tables.at(j)->insert(IndexPair(rec[j].c_str(), rcrd.getUID()));
            }

            std::cout << "Done!" << std::endl;
        }
    } while(records.readBlock());

    std::cout << "Done" << std::endl;
}
