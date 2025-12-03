#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
using namespace std;

struct Edge {
    int to;
    int time;
    bool open;
};

struct ClassInfo {
    int locationId;
    int startMinutes;
    int endMinutes;
};

struct Student {
    string name;
    string ufid;
    int residenceLocation;
    vector<string> classCodes;
};

class Graph {
public:
    void addEdge(int u, int v, int time);
    void toggleEdge(int u, int v);
    string getEdgeStatus(int u, int v) const;
    bool isConnected(int start, int goal) const;

    //dijkstra shortest path from src, using only open edges
    unordered_map<int, int> dijkstra(int src) const;

private:
    unordered_map<int, vector<Edge>> adj;
};


class CampusCompass {
private:
    Graph graph;
    //class code into info
    unordered_map<string, ClassInfo> classesByCode;
    //UFID into student
    unordered_map<string, Student> studentsById;

    //CSV loading helpers
    bool loadEdges(const string &edges_filepath);
    bool loadClasses(const string &classes_filepath);
    int parseTimetoMinutes(const string &timeStr) const;
//validation
    bool isValidUFID(const string &id) const;
    bool isValidName(const string &name) const;
    bool isValidClassCode(const string &code) const;
    //handlers
    bool handleInsert(const string &line);
    bool handleRemove(const string &line);
    bool handleRemoveClass(const string &line);
    bool handleDropClass(const string &line);
    bool handleReplaceClass(const string &line);
    bool handleToggleEdgesClosure(const string &line);
    bool handleCheckEdgeStatus(const string &line);
    bool handleIsConnected(const string &line);
    bool handlePrintShortestEdges(const string &line);
public:
    // Think about what helper functions you will need in the algorithm
    CampusCompass(); // constructor
    bool ParseCSV(const string &edges_filepath, const string &classes_filepath);
    bool ParseCommand(const string &command);
};
