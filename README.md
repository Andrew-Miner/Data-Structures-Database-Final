# Data-Structures-DBMS-Final
This project was created for the final of my Data Structures class. It is incomplete but went above and beyond the requirments of the class. The project implements a Database Management System that accepts SQL-like queries or commands. A Database consists of one or more Table objects stored on disk. A Table is made up rows and columns of data.

There are 3 main components to this project. The [`Command Parser`](https://github.com/Andrew-Miner/Data-Structures-Database-Final/tree/main/DataBase/CommandParser) which parses and interprets database queries, the [`Tables`](https://github.com/Andrew-Miner/Data-Structures-Database-Final/tree/main/DataBase/Table) which organize and store data on disk, and the [`Database class`](https://github.com/Andrew-Miner/Data-Structures-Database-Final/blob/main/DataBase/database.h) which accepts parsed queries and retrieves data from or stores data to Tables.

## Tables
The Tables are the most interesting part of this project. Tables are what actually hold the data in the database. From an abstract point of view, Tables consist of rows and columns. Each row is a record or entry in the database and each column describes the kind of data held in a record.

On disk, tables work a little differently. They consist of an [`IndexFile`](https://github.com/Andrew-Miner/Data-Structures-Database-Final/blob/main/DataBase/Table/indexfile.h) and a set of [`IndexTrees (which are really just B+Trees on disk)`](https://github.com/Andrew-Miner/Data-Structures-Database-Final/blob/main/DataBase/Table/BalancedTree/D%2BTree/disk_bplustree.h). The IndexFile is just a binary file full of records and each records' unique id. IndexTrees on the other hand represents data as a B+Tree data structure stored in a binary file. There is an IndexTree for every column in the Table. These trees hold the unique id of every record and keep them sorted by the column they represent. They allow for the table to be instantaneously sorted by any of its columns. 

## For an indepth explanation of this project please watch this short video presentation:

### Part 1:
[![Watch the video](https://img.youtube.com/vi/BWTfk_l8-Jo/maxresdefault.jpg)](https://youtu.be/BWTfk_l8-Jo)

### Part 2:
[![Watch the video](https://img.youtube.com/vi/r7BcYf-MEo0/maxresdefault.jpg)](https://youtu.be/r7BcYf-MEo0)
