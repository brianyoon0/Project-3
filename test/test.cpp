#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <iostream>
#include <string>
#include "CampusCompass.h"

// helper to run multiple commands and capture the printed output
string runCommands(const vector<string> &cmds) {
    CampusCompass c;
    c.ParseCSV("../data/edges.csv", "../data/classes.csv");

    std::streambuf *oldCout = std::cout.rdbuf();
    std::ostringstream out;
    std::cout.rdbuf(out.rdbuf());

    for (auto &cmd : cmds) {
        c.ParseCommand(cmd);
    }

    std::cout.rdbuf(oldCout);
    return out.str();
}


// ---------------------------------------------------------------------------
// 1) FIVE INCORRECT COMMANDS
// ---------------------------------------------------------------------------
TEST_CASE("Incorrect Commands", "[incorrect]") {
    vector<string> cmds = {
        R"(insert "A11y" 45679999 1 1 COP3530)",    // invalid name
        R"(insert "Alex" 123 1 1 COP3530)",         // UFID too short
        R"(insert "Alex" 12345678 1 1 BAD0000)",    // invalid class code format
        R"(remove BADUFID)",                        // invalid UFID
        R"(dropClass 99999999 COP3530)"             // student doesn't exist
    };

    string out = runCommands(cmds);

    // each must print "unsuccessful"
    int count = 0;
    string line;
    stringstream ss(out);
    while (getline(ss, line)) {
        REQUIRE(line == "unsuccessful");
        count++;
    }
    REQUIRE(count == 5);
}


// ---------------------------------------------------------------------------
// 2) THREE EDGE CASES
// ---------------------------------------------------------------------------
TEST_CASE("Edge Cases", "[edgecases]") {
    vector<string> cmds = {
        R"(insert "Brian" 11112222 1 1 COP3502)",
        R"(remove 99999999)",             // removing nonexistent student
        R"(dropClass 11112222 MAC2311)",  // dropping class student doesn't have
        R"(replaceClass 11112222 COP3502 COP3530)"  // COP3530 exists but student has only 3502
    };

    string out = runCommands(cmds);

    // expected:
    // successful
    // unsuccessful
    // unsuccessful
    // unsuccessful

    vector<string> expected = {
        "successful",
        "unsuccessful",
        "unsuccessful",
        "unsuccessful"
    };

    stringstream ss(out);
    string line;
    int i = 0;

    while (getline(ss, line)) {
        REQUIRE(line == expected[i]);
        i++;
    }
    REQUIRE(i == 4);
}


// ---------------------------------------------------------------------------
// 3) Test dropClass, removeClass, remove, replaceClass
// ---------------------------------------------------------------------------
TEST_CASE("dropClass / removeClass / remove / replaceClass", "[mutations]") {

    vector<string> cmds = {
        R"(insert "Brandon" 22223333 1 2 COP3502 MAC2311)",
        R"(dropClass 22223333 COP3502)",
        R"(replaceClass 22223333 MAC2311 COP3503)",
        R"(insert "Alice" 33334444 1 1 COP3503)",
        R"(removeClass COP3503)",   // removes from both students
        R"(remove 22223333)"        // remove Brandon entirely
    };

    string out = runCommands(cmds);

    // Expected output:
    // successful            (insert)
    // successful            (dropClass)
    // successful            (replaceClass)
    // successful            (insert Alice)
    // 2                     (removeClass COP3503)
    // successful            (remove Brandon)

    vector<string> expected = {
        "successful",
        "successful",
        "successful",
        "successful",
        "2",
        "successful"
    };

    stringstream ss(out);
    string line;
    int i = 0;
    while (getline(ss, line)) {
        REQUIRE(line == expected[i]);
        i++;
    }
    REQUIRE(i == 6);
}


// ---------------------------------------------------------------------------
// 4) printShortestEdges reachable → toggle → unreachable
// ---------------------------------------------------------------------------
// Using discovered *critical edge* (6,15) from dataset.
TEST_CASE("Shortest edges reachable then unreachable", "[paths]") {

    vector<string> cmds = {
        R"(insert "Test" 99998888 1 1 COP3502)",  // class COP3502 at location 23
        // Initially reachable

        R"(printShortestEdges 99998888)",

        // Turn off critical edge (6,15)
        R"(toggleEdgesClosure 1 6 15)",

        // Now unreachable
        R"(printShortestEdges 99998888)"
    };

    string out = runCommands(cmds);

    stringstream ss(out);
    vector<string> lines;
    string line;

    while (getline(ss, line))
        lines.push_back(line);

    // Expected:
    // successful
    // Name: Test
    // COP3502 | Total Time: <some non-negative number>
    // successful
    // Name: Test
    // COP3502 | Total Time: -1

    REQUIRE(lines[0] == "successful");     // insert

    REQUIRE(lines[1] == "Name: Test");
    // Time should be >= 0 before edge removal
    REQUIRE(lines[2].substr(0,20) == "COP3502 | Total Time:");
    int firstTime = stoi(lines[2].substr(lines[2].find_last_of(' ')+1));
    REQUIRE(firstTime >= 0);

    REQUIRE(lines[3] == "successful");     // toggle

    REQUIRE(lines[4] == "Name: Test");
    REQUIRE(lines[5] == "COP3502 | Total Time: -1");
}
