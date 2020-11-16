#include "stdafx.h"
#include <iostream>
#include <string>
#include <tuple>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>
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
    friend std::ostream& operator<<(std::ostream& os, const Book& e)
    {
        os << e.ID << " " << e.name << " " << e.author << std::endl;
        return os;
    }
};
// method 1
typedef multi_index_container< Book, indexed_by<
    ordered_unique<      member<Book, int, &Book::ID> >,
    ordered_non_unique<  member<Book, string, &Book::name> >,
    ordered_non_unique < member<Book, string, &Book::author> >
    > >BookContainer;

typedef BookContainer::nth_index<0>::type Id_Index;
typedef BookContainer::nth_index<1>::type Name_Index;
typedef BookContainer::nth_index<2>::type Author_Index;
// method 2 - tag
struct book_ID{};
struct book_Name {};
struct book_Author {};
struct book_id_name {};

typedef ordered_unique<     tag<book_ID>, BOOST_MULTI_INDEX_MEMBER(Book, int, ID) > idx_id;
typedef ordered_non_unique< tag<book_Name>, BOOST_MULTI_INDEX_MEMBER(Book, string, name) > idx_name;
typedef ordered_non_unique< tag<book_Author>, BOOST_MULTI_INDEX_MEMBER(Book, string, author) > idx_author;
typedef ordered_non_unique< tag<book_id_name>, composite_key<Book, member<Book, int, &Book::ID>, member<Book, string, &Book::name> >> idx_id_name;
typedef multi_index_container< Book, indexed_by<
    idx_id,
    idx_name,
    idx_author,
    idx_id_name
> >BookTagContainer;

// tmplate function 
template<typename Tag, typename MultiIndexContainer>
void print_out_by(const MultiIndexContainer& s)
{
    // obtain a reference to the index tagged by Tag
    const typename boost::multi_index::index<MultiIndexContainer, Tag>::type& i = get<Tag>(s);
    typedef typename MultiIndexContainer::value_type value_type;
    // dump the elements of the index to cout
    std::copy(i.begin(), i.end(), std::ostream_iterator<value_type>(std::cout));  // 二进制“<<”: 没有找到接受“const _Ty”类型的右操作数的运算符(或没有可接受的转换) 

}
void testMultiIndex()
{
    // method 1
    cout << "method 1" << endl;
    BookContainer mybooks;
    mybooks.insert(mybooks.begin(), Book(0, "C++ Premium", "jim"));
    mybooks.insert(mybooks.begin(), Book(1, "Java Premium", "catier"));
    mybooks.insert(mybooks.begin(), Book(2, "PHP Premium", "tiffy"));
    mybooks.insert(mybooks.end(),   Book(3, "Hello C", "tiffy"));

    Id_Index& id_idx = mybooks.get<0>();         // std::get<>()
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
    Author_Index& author_idx = mybooks.get<2>();
    cout << "order by author:" << endl;
    copy(author_idx.begin(), author_idx.end(), ostream_iterator<Book>(cout));
    cout << endl;
    // search
    auto iter = id_idx.find(2);
    if (iter != id_idx.end())
    {
        cout << "Found elememnt: " << iter->ID << "  " << iter->name << "  " << iter->author << endl;
    }
    // method 2 - index using tag
    cout << "method 2" << endl;
    BookTagContainer  tagBook;
    tagBook.insert(tagBook.begin(), Book(0, "C++ Premium", "jim"));
    tagBook.insert(tagBook.begin(), Book(1, "Java Premium", "catier"));
    tagBook.insert(tagBook.begin(), Book(2, "PHP Premium", "tiffy"));
    tagBook.insert(tagBook.begin(), Book(3, "C++ Premium", "tiffy"));
    tagBook.insert(tagBook.end(),   Book(4, "Hello C", "tiffy"));

    print_out_by<book_ID>(tagBook);
    print_out_by<book_Name>(tagBook);
    
    BookTagContainer::index<book_Name>::type& tag_book = tagBook.get<book_Name>();
    BookTagContainer::index<book_Name>::type::iterator iter_id = tag_book.find("C++ Premium");
    if(iter_id != tag_book.end())
        cout << "Found elememnt: " << iter_id->ID << "  " << iter_id->name << "  " << iter_id->author << endl;
    //
    BookTagContainer::index<book_Name>::type::iterator iter_up = tag_book.upper_bound("C++ Premium");
    BookTagContainer::index<book_Name>::type::iterator iter_low = tag_book.lower_bound("C++ Premium");
    while (iter_low != iter_up)
    {
        std::cout << iter_low->ID << "  " << iter_low->name << "  " << iter_low->author << endl;
        iter_low++;
    }
    //
    BookTagContainer::index<book_id_name>::type& tag_id_name = tagBook.get<book_id_name>();
    auto iterp = tag_id_name.equal_range(boost::tuple<int, string>(3, "C++ Premium"));
   /* while (iterp.first!= iterp.second)
    {
        std::cout << iterp.first->ID << "  " << iterp.first->name << "  " << iterp.first->author << endl;
        iterp.first++;
    }*/
    copy(iterp.first, iterp.second, ostream_iterator<Book>(cout));
    cout << endl;
    // find 

}