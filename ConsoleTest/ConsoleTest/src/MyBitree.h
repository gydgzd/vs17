#pragma once
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
        return 0;
    cout << bt->value << "  " << endl;
    preTraversal(bt->lchild);
    preTraversal(bt->rchild);
}

template<class T>
inline int MyBitree<T>::midTraversal(MyBitree * bt)
{
    if (bt == nullptr)
        return 0;
    midTraversal(bt->lchild);
    cout << bt->value << "  " << endl;
    midTraversal(bt->rchild);
}

template<class T>
inline int MyBitree<T>::postTraversal(MyBitree * bt)
{
    if (bt == nullptr)
        return 0;
    postTraversal(bt->lchild);
    postTraversal(bt->rchild);
    cout << bt->value << "  " << endl;
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

