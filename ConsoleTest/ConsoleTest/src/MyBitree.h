#pragma once
#include <queue>

template<class T>
class MyBitree
{
public:
    MyBitree();
    virtual ~MyBitree();

    int init(MyBitree * bt, T array[], int n);
    int preTraversal(MyBitree * bt);
    int midTraversal(MyBitree * bt);
    int postTraversal(MyBitree * bt);
    int breadthTraversal(MyBitree * bt);
private:
    
    int insertOne(MyBitree *bt, T value);
    int deleteOne();
    int modifyOne();
    void printAll(MyBitree *);

private: 
    T value;
    MyBitree *lchild;
    MyBitree *rchild;

};

template<class T>
MyBitree<T>::MyBitree()
{
    lchild = nullptr;
    rchild = nullptr;
}

template<class T>
MyBitree<T>::~MyBitree()
{

}

/*
init a binary tree with an array, n is the number of element in array
*/
template<class T>
int MyBitree<T>::init(MyBitree * bt, T array[], int n)
{
    if (bt == nullptr || array == nullptr || n == 0)
        return 0;
    bt->value = array[0];
    for (int idx = 1; idx < n; idx++)
    {
        insertOne(bt, array[idx]);
    }
    return 0;
}

template<class T>
inline int MyBitree<T>::preTraversal(MyBitree * bt)
{
    if (bt == nullptr)
        return -1;
   // method1 recursive
 /**/   
    cout << bt->value << "  " ;
    preTraversal(bt->lchild);
    preTraversal(bt->rchild);

    // method2 non-recursive
 /*   std::stack<MyBitree *> tmp;
    MyBitree *pos = bt;
    while (pos != nullptr || !tmp.empty())
    {
        cout << pos->value;
        if (pos->lchild == nullptr && pos->rchild == nullptr && tmp.empty())
            break;
        if(pos->rchild != nullptr)
            tmp.emplace(pos->rchild);
        if (pos->lchild != nullptr)   // go to left 
            pos = pos->lchild;
        else if(!tmp.empty())                         // until there is no left, go back to right
        {
            pos = tmp.top();
            tmp.pop();
        }
    }
    */
    return 0;
}

template<class T>
inline int MyBitree<T>::midTraversal(MyBitree * bt)
{
    if (bt == nullptr)
        return -1;
    midTraversal(bt->lchild);
    cout << bt->value << "  " ;
    midTraversal(bt->rchild);
    return 0;
}

template<class T>
inline int MyBitree<T>::postTraversal(MyBitree * bt)
{
    if (bt == nullptr)
        return -1;
    postTraversal(bt->lchild);
    postTraversal(bt->rchild);
    cout << bt->value << "  " ;
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
        cout << tmp.front()->value << "  ";
        if (tmp.front()->lchild != nullptr)
            tmp.push(tmp.front()->lchild);
        if (tmp.front()->rchild != nullptr)
            tmp.push(tmp.front()->rchild);
        tmp.pop();
    }
    return 0;
}

template<class T>
int MyBitree<T>::insertOne(MyBitree *bt, T value)
{
    if (bt == nullptr)
        return 0;
    if (value < bt->value)
    {
        if (bt->lchild == nullptr)
        {
            bt->lchild = new MyBitree();
            bt->lchild->value = value;
        }
        else
        {
            insertOne(bt->lchild, value);
        }
    }
    else
    {
        if (bt->rchild == nullptr)
        {
            bt->rchild = new MyBitree();
            bt->rchild->value = value;
        }
        else
        {
            insertOne(bt->rchild, value);
        }
    }
    return 0;
}

