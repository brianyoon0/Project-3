// #include <catch2/catch_test_macros.hpp>
// #include <iostream>
// #include <sstream>
// #include "CampusCompass.h"
//
// using namespace std;
//
// // helper function to run commands and capture output
// string runCommands(const vector<string>& commands) {
//     stringstream output;
//     streambuf* oldCoutBuffer = cout.rdbuf(output.rdbuf());
//
//     CampusCompass compass;
//     compass.ParseCSV("data/edges.csv", "data/classes.csv");
//
//     for (const auto& cmd : commands) {
//         compass.ParseCommand(cmd);
//     }
//
//     cout.rdbuf(oldCoutBuffer);
//     return output.str();
// }
//
// TEST_CASE("Test 1: Five incorrect commands", "incorrect commands") {
//     CampusCompass compass;
//     compass.ParseCSV("data/edges.csv", "data/classes.csv");
//
//     SECTION("UFID is too short (7 digits instead of 8)") {
//         stringstream output;
//         streambuf* oldCoutBuffer = cout.rdbuf(output.rdbuf());
//
//         compass.ParseCommand("insert \"Brian Yoon\" 8404513 1 1 COP3502");
//
//         cout.rdbuf(oldCoutBuffer);
//         REQUIRE(output.str() == "unsuccessful\n");
//     }
//
//
//     SECTION("UFID contains letters") {
//         stringstream output;
//         streambuf* oldCoutBuffer = cout.rdbuf(output.rdbuf());
//         compass.ParseCommand("insert \"Brian Yoon\" abcdefgh 1 1 COP3502");
//
//         cout.rdbuf(oldCoutBuffer);
//         REQUIRE(output.str() == "unsuccessful\n");
//     }
//
//     SECTION("Name contains special characters") {
//         stringstream output;
//         streambuf* oldCoutBuffer = cout.rdbuf(output.rdbuf());
//
//         compass.ParseCommand("insert \"Brian!Yoon\" 84045135 1 1 COP3502");
//
//         cout.rdbuf(oldCoutBuffer);
//         REQUIRE(output.str() == "unsuccessful\n");
//     }
//
//     SECTION("Invalid class code format") {
//         stringstream output;
//         streambuf* oldCoutBuffer = cout.rdbuf(output.rdbuf());
//
//         compass.ParseCommand("insert \"Brian Yoon\" 84045135 1 1 COP35");
//
//         cout.rdbuf(oldCoutBuffer);
//         REQUIRE(output.str() == "unsuccessful\n");
//     }
//
//     SECTION("Non-existent class") {
//         stringstream output;
//         streambuf* oldCoutBuffer = cout.rdbuf(output.rdbuf());
//
//         compass.ParseCommand("insert \"Brian Yoon\" 84045135 1 1 BAK1011");
//
//         cout.rdbuf(oldCoutBuffer);
//         REQUIRE(output.str() == "unsuccessful\n");
//     }
// }
//
// TEST_CASE("Test 2: Three edge cases", "edge cases") {
//     CampusCompass compass;
//     compass.ParseCSV("data/edges.csv", "data/classes.csv");
//
//     SECTION("Remove a student that doesn't exist") {
//         stringstream output;
//         streambuf* oldCoutBuffer = cout.rdbuf(output.rdbuf());
//         compass.ParseCommand("remove 00000000");
//
//         cout.rdbuf(oldCoutBuffer);
//         REQUIRE(output.str() == "unsuccessful\n");
//     }
//
//     SECTION("Drop a class from non-existent student") {
//         stringstream output;
//         streambuf* oldCoutBuffer = cout.rdbuf(output.rdbuf());
//
//         compass.ParseCommand("dropClass 00000000 COP3502");
//
//         cout.rdbuf(oldCoutBuffer);
//         REQUIRE(output.str() == "unsuccessful\n");
//     }
//
//
//     SECTION("Replace class with non-existent student") {
//         stringstream output;
//         streambuf* oldCoutBuffer = cout.rdbuf(output.rdbuf());
//
//         compass.ParseCommand("replaceClass 00000000 COP3502 CDA3101");
//
//         cout.rdbuf(oldCoutBuffer);
//         REQUIRE(output.str() == "unsuccessful\n");
//     }
// }
//
// TEST_CASE("Test 3: Student management commands", "student management") {
//     vector<string> commands = {
//         "insert \"Brian Yoon\" 00000001 1 2 COP3502 CDA3101",
//         "insert \"bRIAN yOON\" 00000002 1 2 COP3502 MAC2311",
//         "insert \"Brian Yooon\" 00000003 1 1 COP3502",
//         "dropClass 00000001 COP3502",
//         "removeClass CDA3101",
//         "replaceClass 00000002 MAC2311 COP3503",
//         "remove 00000003"
//     };
//
//     string output = runCommands(commands);
//
//     REQUIRE(output.find("unsuccessful") == string::npos);
// }
//
// TEST_CASE("Test 4: printShortestEdges with edge toggling", "[edge-toggling]") {
//     vector<string> commands = {
//         "insert \"Brian Yoon\" 00000001 1 2 COP3502 CDA3101",
//         "printShortestEdges 00000001",
//         "toggleEdgesClosure 2 1 2 2 4",
//         "printShortestEdges 00000001",
//         "toggleEdgesClosure 2 1 2 2 4",
//         "printShortestEdges 00000001"
//     };
//
//     string output = runCommands(commands);
//
//     // verify the output contains expected patterns
//     REQUIRE(output.find("Name: Brian Yoon") != string::npos);
//     REQUIRE(output.find("COP3502") != string::npos);
//     REQUIRE(output.find("CDA3101") != string::npos);
//     REQUIRE(output.find("successful") != string::npos);
// }
//
// TEST_CASE("Example CampusCompass Output Test", "end to end") {
//     string input = R"(6
// insert "Student A" 00000001 1 1 COP3502
// insert "Student B" 00000002 1 1 COP3502
// insert "Student C" 00000003 1 2 COP3502 MAC2311
// dropClass 00000001 COP3502
// remove 00000001
// removeClass COP3502
// )";
//
//     // parse the input into commands
//     stringstream ss(input);
//     string line;
//     getline(ss, line); // skip first line (6)
//
//     vector<string> commands;
//     while (getline(ss, line)) {
//         if (!line.empty()) {
//             commands.push_back(line);
//         }
//     }
//
//     string actualOutput = runCommands(commands);
//
//     // expected output based on the test
//     string expectedOutput = "successful\nsuccessful\nsuccessful\nsuccessful\nunsuccessful\n2\n";
//
//     REQUIRE(actualOutput == expectedOutput);
// }