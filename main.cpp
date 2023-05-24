#include "StupidTable.hpp"
#include <string>

using namespace std;

int main(){
    StupidCompTable<string,int> shapes{};
    shapes.set("squares") = 5;
    shapes.set("trianges") = 2;
    shapes.set("circles") = 4;
    pair<string,int> count_of_squares = *(get<pair<string,int>*>(shapes.find("squares")));
    variant<pair<string,int>* ,nullptr_t> count_of_cubes = shapes.find("cubes");

    shapes.set("pyramids") = 9;

    return 0;
}


