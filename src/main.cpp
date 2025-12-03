#include <iostream>
#include <string>
#include "CampusCompass.h"

using namespace std;

int main() {
    CampusCompass compass;
    compass.ParseCSV("data/edges.csv", "data/classes.csv");

    string line;
    getline(cin, line);
    int no_of_lines = 0;
    try {
        no_of_lines = stoi(line);
    } catch (const exception& e) {
        return 0;
    }

    for (int i = 0; i < no_of_lines; i++) {
        getline(cin, line);
        if (!line.empty()) {
            compass.ParseCommand(line);
        }
    }
    return 0;
}