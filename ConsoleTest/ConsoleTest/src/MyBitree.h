#pragma once
#include <queue>
#include <list>
using namespace std;
template<typename T>
class MyBitree
{
public:
    MyBitree();
    MyBitree(T& value);           // explicit
    virtual ~MyBitree();

    virtual int init(MyBitree * bt, T array[], int n);
    virtual int insertOne(MyBitree *bt, T value);      // insert by order
    virtual int insertFull(MyBitree *bt, T value);     // insert one next to one ,like a full tree

    int preTraversal(MyBitree * bt);
    int midTraversal(MyBitree * bt);
    int postTraversal(MyBitree * bt);
    int breadthTraversal(MyBitree * bt);
    
    int deleteTree(MyBitree *bt);                     // delete root node bt 
    int deleteTree();                                 // don't delete root node
private:

    
    int modifyOne();
    void printAll(MyBitree *bt);

private: 
    T m_value;
    MyBitree *m_lchild;
    MyBitree *m_rchild;

};

template<class T>
MyBitree<T>::MyBitree()
{
    m_lchild = nullptr;
    m_rchild = nullptr;
    cout << "Construct MyBitree" << endl;
}

template<class T>
MyBitree<T>::MyBitree(T& _value)
{
    m_value = _value;
    m_lchild = nullptr;
    m_rchild = nullptr;
    cout << "Construct MyBitree" << endl;
}

template<class T>
MyBitree<T>::~MyBitree()
{
    std::queue<T> tmp;
    deleteTree(this->m_lchild);
    deleteTree(this->m_rchild);
    cout << "~MyBitree()" << endl;
}

/*
*init a binary tree with an array, n is the index of element in array
*/
template<class T>
int MyBitree<T>::init(MyBitree * bt, T array[], int n)
{
    if (bt == nullptr || array == nullptr || n == 0)
        return 0;
    bt->m_value = array[0];
    std::queue<MyBitree *> tmp;
    tmp.emplace(bt);
    for (int idx = 1; idx < n; idx++)
    {
        // method 1 
        //insertOne(bt, array[idx]);

        // method 2
        insertFull(bt, array[idx]);

    }
    return 0;
}

template<class T>
int MyBitree<T>::insertOne(MyBitree *bt, T value)
{
    if (bt == nullptr)
        return 0;
    if (value < bt->m_value)
    {
        if (bt->m_lchild == nullptr)
        {
            bt->m_lchild = new MyBitree();
            bt->m_lchild->m_value = value;
        }
        else
        {
            insertOne(bt->m_lchild, value);
        }
    }
    else
    {
        if (bt->m_rchild == nullptr)
        {
            bt->m_rchild = new MyBitree();
            bt->m_rchild->m_value = value;
        }
        else
        {
            insertOne(bt->m_rchild, value);
        }
    }
    return 0;
}
// insert a node in breadthTraversal order
template<class T>
int MyBitree<T>::insertFull(MyBitree *bt, T value)
{
    if (bt == nullptr)
        return 0;
    std::queue<MyBitree *> tmpQ;
    tmpQ.push(bt);
    MyBitree * pos = nullptr;
    MyBitree * node = new MyBitree(value);
    while (!tmpQ.empty())
    {
        pos = tmpQ.front();
        if (pos->m_lchild == nullptr)
        {
            pos->m_lchild = node;
            tmpQ.push(pos->m_lchild);
            break;
        }else
            tmpQ.push(pos->m_lchild);
        if (pos->m_rchild == nullptr)
        {
            pos->m_rchild = node;
            tmpQ.push(pos->m_rchild);
            break;
        }
        else
        {
            tmpQ.push(pos->m_rchild);
        }
        tmpQ.pop();
    }

    return 0;
}


template<class T>
inline int MyBitree<T>::preTraversal(MyBitree * bt)
{
    if (bt == nullptr)
        return -1;
   // method1 recursive
 /*  
    cout << bt->m_value << "  " ;
    preTraversal(bt->m_lchild);
    preTraversal(bt->m_rchild);
*/ 
    // method2 non-recursive
 /**/
    std::stack<MyBitree *> tmp;
    MyBitree *pos = bt;
    while (pos != nullptr || !tmp.empty())
    {
        cout << pos->m_value;
        if (pos->m_lchild == nullptr && pos->m_rchild == nullptr && tmp.empty())
            break;
        if(pos->m_rchild != nullptr)
            tmp.emplace(pos->m_rchild);
        if (pos->m_lchild != nullptr)   // go to left 
            pos = pos->m_lchild;
        else if(!tmp.empty())           // until there is no left, go back to right
        {
            pos = tmp.top();
            tmp.pop();
        }
    }
    
    return 0;
}

template<class T>
inline int MyBitree<T>::midTraversal(MyBitree * bt)
{
    if (bt == nullptr)
        return -1;
    midTraversal(bt->m_lchild);
    cout << bt->m_value << "  " ;
    midTraversal(bt->m_rchild);
    return 0;
}

template<class T>
inline int MyBitree<T>::postTraversal(MyBitree * bt)
{
    if (bt == nullptr)
        return -1;
    postTraversal(bt->m_lchild);
    postTraversal(bt->m_rchild);
    cout << bt->m_value << "  " ;
    return 0;
}

template<class T>
inline int MyBitree<T>::breadthTraversal(MyBitree * bt)
{
    if (bt == nullptr)
        return -1;
    std::queue<MyBitree*> tmp;
    tmp.push(bt);
    while (!tmp.empty())
    {
        cout << tmp.front()->m_value << "  ";
        if (tmp.front()->m_lchild != nullptr)
            tmp.push(tmp.front()->m_lchild);
        if (tmp.front()->m_rchild != nullptr)
            tmp.push(tmp.front()->m_rchild);
        tmp.pop();
    }
    return 0;
}

//delete a tree whose root is constructed by new
template<typename T>
inline int MyBitree<T>::deleteTree(MyBitree * bt)
{   
    if (bt == nullptr || (bt->m_lchild == nullptr && bt->m_rchild == nullptr))
        return 0;
    std::list<MyBitree *> tmpList;
    tmpList.push_back(bt);
    auto iterList = tmpList.begin();
    while (true)
    {
        if ((*iterList)->m_lchild)
        {
            tmpList.insert(tmpList.end(), (*iterList)->m_lchild);
        }
        if ((*iterList)->m_rchild)
        {
            tmpList.insert(tmpList.end(), (*iterList)->m_rchild);
        }
        iterList++;
        if (iterList == tmpList.end())
            break;
    }
    auto riterList = tmpList.rbegin();
    while (riterList != tmpList.rend())
    {
        if ((*riterList)->m_lchild)
            (*riterList)->m_lchild = nullptr;
        if ((*riterList)->m_rchild)
            (*riterList)->m_rchild = nullptr;
        delete *riterList++;
    }
    return 0;
}

//delete a tree whose root is constructed without new
template<typename T>
inline int MyBitree<T>::deleteTree()
{
    if (this == nullptr || (this->m_lchild == nullptr && this->m_rchild == nullptr))
        return 0;
    std::list<MyBitree *> tmpList;
    tmpList.push_back(this);
    auto iterList = tmpList.begin();
    while (true)
    {
        if ((*iterList)->m_lchild)
        {
            tmpList.insert(tmpList.end(), (*iterList)->m_lchild);
        }
        if ((*iterList)->m_rchild)
        {
            tmpList.insert(tmpList.end(), (*iterList)->m_rchild);
        }
        iterList++;
        if (iterList == tmpList.end())
            break;
    }
    tmpList.erase(tmpList.begin());       // de not delete root
    auto riterList = tmpList.rbegin();
    while (riterList != tmpList.rend())
    {
        if ((*riterList)->m_lchild)
            (*riterList)->m_lchild = nullptr;
        if ((*riterList)->m_rchild)
            (*riterList)->m_rchild = nullptr;
        delete *riterList++;
    }
    this->m_lchild = nullptr;
    this->m_rchild = nullptr;
    return 0;
}

template<typename T>
class MyAVLTree : public MyBitree<typename T>
{
public:
    MyAVLTree()  { cout << "Construct MyAVLTree" << endl; };
    ~MyAVLTree() { cout << "~MyAVLTree" << endl; };

    virtual int init(MyBitree * bt, T array[], int n);
    virtual int insertOne(MyBitree *bt, T value);      // insert by order
    virtual int insertFull(MyBitree *bt, T value);     // insert one next to one ,like a full tree
protected:

private:
};
