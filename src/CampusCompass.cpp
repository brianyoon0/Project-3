#include "CampusCompass.h"

#include<fstream>
#include<sstream>
#include<iostream>
#include <queue>
#include<limits>
#include<cctype>
#include<algorithm>
#include <string>

using namespace std;

static const int INF = numeric_limits<int>::max();

void Graph::addEdge(int u, int v, int time) {
    adj[u].push_back({v, time, true});
    adj[v].push_back({u,time, true});
}

void Graph::toggleEdge(int u, int v) {
    //toggle u->v
    if (adj.count(u)) {
        for (auto &e : adj.at(u)) {
            if (e.to == v) {
                e.open = !e.open;
            }
        }
    }
    //toggle v->u
    if (adj.count(v)) {
        for (auto &e : adj.at(v)) {
            if (e.to == u) {
                e.open = !e.open;
            }
        }
    }
}


string Graph::getEdgeStatus(int u, int v) const {
    auto it = adj.find(u);
    if (it == adj.end()) {
        return "Does not exist";
    }

    for (const auto &e : it->second) {
        if (e.to == v) {
            if (e.open) {
                return "Open";
            } else {
                return "Closed";
            }
        }
    }
        return "DNE";
    }

bool Graph::isConnected(int start, int goal) const {
    if (start == goal) {
        return true;
    }
    if (!adj.count(start) || !adj.count(goal)) {
        return false;
    }
    unordered_set<int> visited;
    queue<int> q;
    q.push(start);
    visited.insert(start);

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        auto it = adj.find(u);
        if (it == adj.end()) {
            continue;
        }
        for (const auto &e : it->second) {
            if (!e.open) continue;
            int v = e.to;
            if (!visited.count(v)) {
                if (v == goal) return true;
                visited.insert(v);
                q.push(v);
            }
        }
    }
    return false;
}

unordered_map<int, int> Graph::dijkstra(int src) const {
    unordered_map<int, int> dist;

    for (const auto &p : adj) {
        dist[p.first] = INF;
    }
    if (!adj.count(src)) {
        return dist;
    }
    dist[src] = 0;
//distance, node
    using NodePair = pair<int, int>;
    priority_queue<NodePair, vector<NodePair>, std::greater<NodePair>> pq;
    pq.push({0, src});

    while (!pq.empty()) {
        auto[d, u] = pq.top();
        pq.pop();
        if (d > dist[u]) continue;
        auto it = adj.find(u);
        if (it == adj.end()) continue;

        for (const auto &e : it->second) {
            if (!e.open) continue;
            int v = e.to;
            int w = e.time;

            if (dist[u] != INF && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

CampusCompass::CampusCompass() = default;

int CampusCompass::parseTimetoMinutes(const string &timeStr) const {
    if (timeStr.empty()) return -1;

    int h = 0;
    int m = 0;
    char colon;
    stringstream ss(timeStr);
    ss >> h >> colon >> m;
    if (!ss || colon !=':') {
        return -1;
    }
    return h * 60 + m;
}
//load edges.csv
bool CampusCompass::loadEdges(const string &edges_filepath) {
    ifstream in(edges_filepath);
    if (!in.is_open()) {
        cerr << "Failed to open edges file: " << edges_filepath << endl;
        return false;
    }
    string line;
    if (!getline(in, line)) {
        return false;
    }
    while (getline(in, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string sId1, sId2, name1, name2, sTime;

        if (!getline(ss, sId1, ',')) continue;
        if (!getline(ss, sId2, ',')) continue;
        if (!getline(ss, name1, ',')) continue;
        if (!getline(ss, name2, ',')) continue;
        if (!getline(ss, sTime, ',')) continue;

        int id1 = stoi(sId1);
        int id2 = stoi(sId2);
        int t = stoi(sTime);

        graph.addEdge(id1, id2, t);
    }
    return true;
}

bool CampusCompass::loadClasses(const string &classes_filepath) {
    ifstream in(classes_filepath);
    if (!in.is_open()) {
        cerr << "Failed to open classes file: " << classes_filepath << endl;
        return false;
    }
    string line;
    if (!getline(in, line)) {
        return false;
    }
    while (getline(in, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string code, sLoc, sStart, sEnd;

        if (!getline(ss, code, ',')) continue;
        if (!getline(ss, sLoc, ',')) continue;
        if (!getline(ss, sStart, ',')) continue;
        if (!getline(ss, sEnd, ',')) continue;

        ClassInfo info;
        info.locationID = stoi(sLoc);
        info.startMinutes = parseTimetoMinutes(sStart);
        info.endMinutes = parseTimetoMinutes(sEnd);

        classByCode[code] = info;
    }
    return true;
}

bool CampusCompass::ParseCSV(const string &edges_filepath, const string &classes_filepath) {
    bool ok1 = loadEdges(edges_filepath);
    bool ok2 = loadClasses(classes_filepath);
    return ok1 && ok2;
}

//validation helpers
bool CampusCompass::isValidUFID(const string &id) const {
    if (id.size() !=8) return false;
    for (char c : id) {
        if (!isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

bool CampusCompass::isValidName(const string &name) const {
    if (name.empty()) return false;
    for (char c : name) {
        if (!isalpha(static_cast<unsigned char>(c)) && c != ' ') return false;
    }
    return true;
}

bool CampusCompass::isValidClassCode(const string &code) const {
    if (code.size() != 7) return false;
    for (int i = 0 ; i < 3; i++) {
        if (!(code[i] >= 'A' && code[i] <= 'Z')) return false;
    }
    for (int i = 3; i < 7; i++) {
        if (!isdigit(static_cast<unsigned char>(code[i]))) return false;
    }
    return true;
}


bool CampusCompass::ParseCommand(const string &command) {
    // do whatever regex you need to parse validity
    // hint: return a boolean for validation when testing. For example:
    (void)command;
    return false;
}
