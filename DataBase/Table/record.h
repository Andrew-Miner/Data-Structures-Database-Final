#ifndef RECORD_H
#define RECORD_H
#include <vector>
#include <string>
#include <fstream>

class Record
{
public:
    static const int MAX_STR_LENGTH = 64;
    static const int MAX_COLUMN_SIZE = 16;

    /* ======================================================
     * Name: Default Constructor
     * Function: Create Record Object
     * Pre: none
     * Post: Record Object With UID created
     * Parameters: uid - unique record id
     */
    Record(std::streampos uid = 0);

    /* ======================================================
     * Name: Constructor 1
     * Function: Create Record Object
     * Pre: none
     * Post: Record Object With UID and Data created
     * Parameters: uid - unique record id
     *             data - vector containing strings to be
     *                    saved in record
     */
    Record(std::streampos uid, std::vector<std::string> data);

    /* ======================================================
     * Name: Constructor 2
     * Function: Create Record Object
     * Pre: none
     * Post: Record Object With UID and Data created
     * Parameters: uid - unique record id
     *             list - array containing cstrings to be
     *                    saved in record
     *             str_count - count of cstrings in list
     */
    Record(std::streampos uid, char list[][MAX_STR_LENGTH], int str_count);

    /* ======================================================
     * Name: Remove Value
     * Function: Remove Value From cstring list located at
     *           rec_number
     * Pre: rec_number >= 0 && < MAX_COLUMN_SIZE
     * Post: CString located at rec_number is removed
     * Parameters: rec_number - index cstring is located
     *                          within record array
     */
    void removeValue(int rec_number);

    /* =======================================================
     * Name: Set  Value
     * Function: Insert value into list at index rec_number
     * Pre: rec_number >= 0 && < MAX_COLUMN_SIZE
     * Post: value is inserted into record at index rec_number
     * Parameters: rec_number - index to insert into
     *             value - string to insert as cstring
     */
    void setValue(int rec_number, const std::string& value);

    /* ======================================================
     * Name: Get Value
     * Function: Get String From Record Located At rec_number
     * Pre: rec_number >= 0 && < MAX_COLUMN_SIZE
     * Post: Returns String located at records[rec_number]
     * Parameters: rec_number - index cstring is located at
     *                          within record array
     */
    std::string getValue(int rec_number) const;

    /* ======================================================
     * Name: Get List
     * Function: Get List Of All Strings In Record Object
     * Pre: none
     * Post: Vector of strings located In Record Object
     *       Returned
     * Parameters: none
     */
    std::vector<std::string> getList() const;

    /* ======================================================
     * Name: Get List
     * Function: Get List Of first length Strings In
     *           Record Object
     * Pre: length > 0 && <= MAX_COLUMN_SIZE
     * Post: Vector of strings located In Record Object
     *       Returned
     * Parameters: length - amount of strings to insert
     */
    std::vector<std::string> getList(int length) const;

    bool operator==(const Record& other) { return unique_id == other.unique_id; }

    /* ======================================================
     * Name: Is Deleted
     * Function: Check If Record Is Marked Deleted
     * Pre: none
     * Post: Returns bool determining if record is deleted
     * Parameters: none
     */
    bool isDelted() { return is_deleted; }

    /* ======================================================
     * Name: Delete
     * Function: Marks Record as not deleted
     * Pre: none
     * Post: Record is marked not deleted
     * Parameters: none
     */
    void Delete() { is_deleted = true; }

    /* ======================================================
     * Name: unDelete
     * Function: Marks Record as deleted
     * Pre: none
     * Post: Record is marked deleted
     * Parameters: none
     */
    void unDelete() { is_deleted = false; }

    /* ======================================================
     * Name: Get UID
     * Function: Get Record's Unique ID
     * Pre: none
     * Post: Record UID is returned
     * Parameters: none
     */
    std::streampos getUID() { return unique_id; }

    /* ======================================================
     * Name: Set UID
     * Function: Set Record's Unique ID
     * Pre: UID must be the record's streampos within the
     *      Record File
     * Post: Record UID is returned
     * Parameters: none
     */
    void setUID(const std::streampos& UID) { unique_id = UID; }

    /* ======================================================
     * Name: Load
     * Function: Loads Record From file_in at pos UID
     * Pre: UID must fall within file_in's length
     * Post: Returns Position In file_in Record is located at
     * Parameters: file_in - file to load record from
     *             UID - file pos to load record from
     */
    std::streampos load(std::fstream& file_in, std::streampos UID);

    /* ======================================================
     * Name: Load
     * Function: Loads Record From file_in at pos unique_id
     * Pre: none
     * Post: Returns Position In file_in Record is located at
     * Parameters: file_in - file to load record from
     */
    std::streampos load(std::fstream& file_in) { return load(file_in, unique_id); }

    /* ======================================================
     * Name: Save
     * Function: Stores Record in file_out at pos unique_id
     * Pre: none
     * Post: Returns Position In file_in Record is located at
     * Parameters: file_out - file to save record to
     */
    std::streampos save(std::fstream& file_out);

    /* ======================================================
     * Name: Operator []
     * Function: Get Copy Of Record's CString as String
     * Pre: none
     * Post: Returns CString located at index within record
     * Parameters: index - location within record
     */
    std::string operator[](int index) const;

    /* ======================================================
     * Name: Operator <<
     * Function: Print Record To ostream
     * Pre: none
     * Post: Record is printed to out
     * Parameters: out = valid ostream object
     *             record - valid Record object
     */
    friend std::ostream& operator<<(std::ostream& out, const Record& record);

private:
    bool is_deleted;
    std::streampos unique_id;
    char record[MAX_COLUMN_SIZE][MAX_STR_LENGTH];
};

#endif // RECORD_H
