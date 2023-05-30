#include "StupidTable.hpp"
#include <string>
#include <iostream>
#include <map>
#include <chrono>
#include <random>
using namespace std;

int main(){

    StupidCompTable<string,int> shapes{};
    shapes.set("squares") = 5;
    shapes.set("trianges") = 2;
    shapes.set("circles") = 4;
    pair<string,int> count_of_squares = *(shapes.find("squares"));
    StupidCompTable<string,int>::iterator count_of_cubes = shapes.find("cubes");

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

    roman_itr++;
    roman_itr++;

    cout << (*roman_itr).first << " = " << (*roman_itr).second << endl;

    roman_itr++;
    roman_itr++;

    cout << (*roman_itr).first << " = " << (*roman_itr).second << endl;
    
    roman_itr--;

    cout << (*roman_itr).first << " = " << (*roman_itr).second << endl;

    roman_itr--;
    roman_itr--;

    cout << (*roman_itr).first << " = " << (*roman_itr).second << endl;

    random_device randint{};

    auto my_start = chrono::high_resolution_clock::now();
    StupidCompTable<int,int> my_table{};
    for (int count = 0; count != 100000; count++){
        my_table.set(randint()) = randint();
    }
    auto my_end = chrono::high_resolution_clock::now();
    cout << "My time: " << (my_end-my_start) / 1us << endl;

    auto std_start = chrono::high_resolution_clock::now();
    map<int,int> std_table{};
    for (int count = 0; count != 100000; count++){
        std_table[randint()] = randint();
    }
    auto std_end = chrono::high_resolution_clock::now();
    cout << "Standard library time: " << (std_end-std_start) / 1us << endl;



    return 0;
}


