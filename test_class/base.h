#pragma once
class Base {
public:
	virtual int get();
	void set(int n);
private:
	int memb;

protected:
	int level;
};

class Base1 : public Base{
public:
	virtual int get();
	virtual void set(int n);
private:
	int b1;

protected:
	int c1;
};

class Base2 : public Base {
public:
	virtual int get();
	virtual void set(int n);
	virtual int print();
private:
	int b2;

protected:
	int c2;
};