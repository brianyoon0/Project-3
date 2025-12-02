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
    int locationID;
    int startMinutes;
    int endMinutes;
};

struct Student {
    string name;
    string ufidl;
    int residenceLocation;
    vector<string> classCodes;
};

class Graph {
public:
    void addEdge(int u, int v, int time);
    void toggleEdge(int u, int v);
    string getEdgeStatus(int u, int v) const;
    bool isConnected(int start, int goal) const;

    unordered_map<int, int> dijkstra(int src) const;

private:
    unordered_map<int, vector<Edge>> adj;
};


class CampusCompass {
private:
    Graph graph;
    unordered_map<string, ClassInfo> classByCode;
    unordered_map<string, Student> studentByID;
    //CSV loading helpers
    bool loadEdges(const string &edges_filepath);
    bool loadClasses(const string &classes_filepath);
    int parseTimetoMinutes(const string &timesStr) const;

public:
    // Think about what helper functions you will need in the algorithm
    CampusCompass(); // constructor
    bool ParseCSV(const string &edges_filepath, const string &classes_filepath);
    bool ParseCommand(const string &command);
};
