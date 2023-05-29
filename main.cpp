#include "StupidTable.hpp"
#include <string>
#include <iostream>
#include <map>

using namespace std;

int main(){

    StupidCompTable<string,int> shapes{};
    shapes.set("squares") = 5;
    shapes.set("trianges") = 2;
    shapes.set("circles") = 4;
    pair<string,int> count_of_squares = *(get<pair<string,int>*>(shapes.find("squares")));
    variant<pair<string,int>* ,nullptr_t> count_of_cubes = shapes.find("cubes");

    shapes.set("pyramids") = 9;


    StupidCompTable<int,char> roman_numerals{};
    roman_numerals.set(1) = 'I';
    roman_numerals.set(5) = 'V';
    roman_numerals.set(10) = 'X';

    StupidCompTable<int,char> more_roman_numerals{};
    more_roman_numerals.set(50) = 'L';
    more_roman_numerals.set(100) = 'C';
    more_roman_numerals.set(500) = 'D';

    cout << boolalpha << (roman_numerals == more_roman_numerals) << endl;

    more_roman_numerals.set(1) = 'I';
    more_roman_numerals.set(5) = 'V';
    more_roman_numerals.set(10) = 'X';
    roman_numerals.set(50) = 'L';
    roman_numerals.set(100) = 'C';
    roman_numerals.set(500) = 'D';

    cout << boolalpha << (roman_numerals == more_roman_numerals) << endl;

    cout << boolalpha << (roman_numerals >= more_roman_numerals) << endl;

    StupidCompTable<string,int> compcheck1{};
    StupidCompTable<string,int> compcheck2{};

    compcheck1.set("a") = 2;
    compcheck1.set("d") = 10;

    compcheck2.set("a") = 2;
    compcheck2.set("d") = 5;

    cout << boolalpha << (compcheck1 < compcheck2) << (compcheck1 > compcheck2) << endl;

    auto roman_itr = roman_numerals.front();

    return 0;
}


