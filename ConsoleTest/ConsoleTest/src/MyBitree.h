#pragma once
template<class T>

class MyBitree
{
public:
    MyBitree();
    virtual ~MyBitree();

    int init(MyBitree * bt, T array[], int n);
private:
    
    int insertOne(MyBitree *bt, T value);
    int deleteOne();
    int modifyOne();
    void printAll(MyBitree *);

private: 
    T value;
    MyBitree *lchild;
    MyBitree *rchild;

    MyBitree *root;
};

template<class T>
MyBitree<T>::MyBitree()
{
    root = NULL;
    lchild = nullptr;
    rchild = NULL;
}

template<class T>
MyBitree<T>::~MyBitree()
{

}

template<class T>
int MyBitree<T>::init(MyBitree * bt, T array[], int n)
{
 

    return 0;
}

template<class T>
int MyBitree<T>::insertOne(MyBitree *bt, T value)
{
    if (bt == nullptr)
    {
        bt->value = value;
    }
    while ()
        return 0;
}
