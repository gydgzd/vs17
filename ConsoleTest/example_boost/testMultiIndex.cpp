#include "stdafx.h"
#include <iostream>
#include <string>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>

using namespace std;
using namespace boost;
using namespace boost::multi_index;

struct Book
{
    int ID;
    string name;
    string author;
    Book(int _ID, string _name, string _author)
    {
        ID = _ID;
        name = _name;
        author = _author;
    }
};

typedef multi_index_container< Book, indexed_by<
    ordered_unique<      member<Book, int, &Book::ID> >,
    ordered_non_unique<  member<Book, string, &Book::name> >,
    ordered_non_unique < member<Book, string, &Book::author> >
    > >BookContainer;

typedef BookContainer::nth_index<0>::type Id_Index;
typedef BookContainer::nth_index<1>::type Name_Index;
typedef BookContainer::nth_index<2>::type Author_Index;

void testMultiIndex()
{
    BookContainer mybooks;
    mybooks.insert(mybooks.begin(), Book(0, "C++ Premium", "jim"));
    mybooks.insert(mybooks.begin(), Book(1, "Java Premium", "catier"));
    mybooks.insert(mybooks.begin(), Book(2, "PHP Premium", "tiffy"));
    mybooks.insert(mybooks.end(), Book(3, "Hello C", "tiffy"));

    Id_Index& id_idx = mybooks.get<0>();
    for (auto iter = id_idx.begin(); iter != id_idx.end(); iter++)
    {
        cout << iter->ID << "  " << iter->name << "  " << iter->author << endl;
    }
    cout << endl;
    Name_Index& name_idx = mybooks.get<1>();
    for (auto iter = name_idx.begin(); iter != name_idx.end(); iter++)
    {
        cout << iter->ID << "  " << iter->name << "  " << iter->author << endl;
    }
}