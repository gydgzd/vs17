#include <stdio.h>
#include <iostream>
using namespace std;
class car
{
public:
	
	void getSize();
	void getColor();
	friend void getCar();
private:
	static int length;
	int width;
	int color;
};
int car::length = 1;
void car::getSize()
{


}
void car::getColor()
{

}
void getCar()
{
	cout << car::length << endl;
}