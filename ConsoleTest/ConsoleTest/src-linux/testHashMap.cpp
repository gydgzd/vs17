/*
 * testHashMap.cpp
 *
 *  Created on: Dec 17, 2018
 *      Author: gyd
 */

#include <unordered_map>
using namespace std;
int test_unorderedMap()
{
    unordered_map<string, int> mymap;
    mymap.insert(pair<string, int>("Lisa", 28));
 //   mymap.insert(pair<string, int>("Anna", 23));
 //   mymap.insert(pair<string, int>("Nick", 18));
  //  mymap.insert(pair<string, int>("Nick", 18));
    std::unordered_map<string, int>::iterator mapiterator;
    for (mapiterator = mymap.begin(); mapiterator != mymap.end(); mapiterator++)
        printf("%s, %d \n", mapiterator->first.c_str(), mapiterator->second);
    printf("Num %lu/%lu,  used mem %lu\n", mymap.size(), mymap.max_size(), sizeof(mymap));

    unordered_multimap<int, int> mymtmap_num;
    mymtmap_num.insert(mymtmap_num.begin(),pair<int, int>(5, 2));
    mymtmap_num.insert(mymtmap_num.begin(),pair<int, int>(4, 2));
    mymtmap_num.insert(mymtmap_num.begin(), pair<int, int>(3, 2));
    mymtmap_num.insert(mymtmap_num.begin(), pair<int, int>(1, 2));
    mymtmap_num.insert(mymtmap_num.begin(), pair<int, int>(2, 2));
    mymtmap_num.insert(mymtmap_num.begin(), pair<int, int>(3, 2));
    mymtmap_num.insert(mymtmap_num.begin(), pair<int, int>(4, 2));
    mymtmap_num.insert(mymtmap_num.begin(), pair<int, int>(5, 2));
    mymtmap_num.insert(mymtmap_num.begin(), pair<int, int>(6, 2));
    mymtmap_num.insert(mymtmap_num.begin(), pair<int, int>(7, 2));
    std::unordered_multimap<int, int>::iterator mtmap_num_iter;
    for (mtmap_num_iter = mymtmap_num.begin(); mtmap_num_iter != mymtmap_num.end(); mtmap_num_iter++)
    {
        printf("%d, %d \n", mtmap_num_iter->first, mtmap_num_iter->second);
    }
    return 0;
}


