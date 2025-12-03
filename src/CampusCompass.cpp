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
        return "DNE";
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
        info.locationId = stoi(sLoc);
        info.startMinutes = parseTimetoMinutes(sStart);
        info.endMinutes = parseTimetoMinutes(sEnd);

        classesByCode[code] = info;
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
    if (command.empty()) {
        cout << "unsuccessful" << endl;
        return false;
    }

    string cmdWord;
    stringstream ss(command);
    ss >> cmdWord;

    if (!ss) {
        cout << "unsuccessful" << endl;
        return false;
    }
    if (cmdWord == "insert") {
        return handleInsert(command);
    } else if (cmdWord == "remove") {
        return handleRemove(command);
    } else if (cmdWord == "removeClass") {
        return handleRemoveClass(command);
    } else if (cmdWord == "dropClass") {
        return handleDropClass(command);
    } else if (cmdWord == "replaceClass") {
        return handleReplaceClass(command);
    }

    cout << "unsuccessful" << endl;
    return false;
}

bool CampusCompass::handleInsert(const string &line) {

    //find the name
    size_t firstQuote = line.find('"');
    if (firstQuote == string::npos) {
        cout << "unsuccessful" << endl;
        return false;
    }
    size_t secondQuote = line.find('"', firstQuote + 1);
    if (secondQuote == string::npos) {
        cout << "unsuccessful" << endl;
        return false;
    }

    string name = line.substr(firstQuote + 1, secondQuote - firstQuote - 1);

    //parse the rest
    string rest = line.substr(secondQuote + 1);
    stringstream ss(rest);

    string ufid;
    int residence = 0;
    int n = 0;

    if (!(ss >> ufid >> residence >> n)) {
        cout << "unsuccessful" << endl;
        return false;
    }

    vector<string> codes;
    string code;
    while (ss >> code) {
        codes.push_back(code);
    }

    //checks
    if (n != static_cast<int>(codes.size())) {
        cout << "unsuccessful" << endl;
        return false;
    }

    if (n < 1 || n > 6) {
        cout << "unsuccessful" << endl;
        return false;
    }
    //make sure its valid
    if (!isValidName(name) || !isValidUFID(ufid)) {
        cout << "unsuccessful" << endl;
        return false;
    }
    if (studentsById.count(ufid)) {
        cout << "unsuccessful" << endl;
        return false;
    }
    for (const string &c: codes) {
        if (!isValidClassCode(c)) {
            cout << "unsuccessful" << endl;
            return false;
        }
        if (!classesByCode.count(c)) {
            cout << "unsuccessful" << endl;
            return false;
        }
    }

    //if everything is valid create and store the students data
    Student s;
    s.name = name;
    s.ufid = ufid;
    s.residenceLocation = residence;
    s.classCodes = codes;
    studentsById[ufid] = s;
    cout << "successful" << endl;
    return true;
}

bool CampusCompass::handleRemove(const string &line) {
    string cmd, ufid;
    stringstream ss(line);
    ss >> cmd >> ufid;

    if (!ss || ufid.empty()) {
        cout << "unsuccessful" << endl;
        return false;
    }
    if (!isValidUFID(ufid)) {
        cout << "unsuccessful" << endl;
        return false;
    }
    auto it = studentsById.find(ufid);
    if (it == studentsById.end()) {
        cout << "unsuccessful" << endl;
        return false;
    }
    studentsById.erase(it);
    cout << "successful" << endl;
    return true;
}

bool CampusCompass::handleRemoveClass(const string &line) {
    string cmd, code;
    stringstream ss(line);
    ss >> cmd >> code;

    if (!ss || code.empty()) {
        cout << "unsuccessful" << endl;
        return false;
    }
    if (!isValidClassCode(code)) {
        cout << "unsuccessful" << endl;
        return false;
    }
    int affectedStudents = 0;
    vector<string> toErase;

    for (auto &entry : studentsById) {
        string ufid = entry.first;
        Student &s = entry.second;

        bool hadThisClass = false;
        vector<string> newList;
        newList.reserve(s.classCodes.size());

        for (const string &c : s.classCodes) {
            if (c == code) {
                hadThisClass = true;
            } else {
                newList.push_back(c);
            }
        }
        if (hadThisClass) {
            affectedStudents++;
            s.classCodes.swap(newList);
            if (s.classCodes.empty()) {
                toErase.push_back(ufid);
            }
        }
    }
    for (const string &ufid :toErase) {
        studentsById.erase(ufid);
    }
    cout << affectedStudents << endl;
    return true;
}

bool CampusCompass::handleDropClass(const string &line) {
    string cmd, ufid, code;
    stringstream ss(line);
    ss >> cmd >> ufid >> code;
//parsing check
    if (!ss || ufid.empty() || code.empty()) {
        cout << "unsuccessful" << endl;
        return false;
    }
    //validate ufid format and the class format
    if (!isValidUFID(ufid) || !isValidClassCode(code)) {
        cout << "unsuccessful" << endl;
        return false;
    }//class must exist in the classes.csv
    if (!classesByCode.count(code)) {
        cout << "unsuccessful" << endl;
        return false;
    }

    //student must exist
    auto it = studentsById.find(ufid);
    if (it == studentsById.end()) {
        cout << "unsuccessful" << endl;
        return false;
    }

    Student &s = it->second;
//students must actually have the class
    bool found = false;
    vector<string> newList;
    newList.reserve(s.classCodes.size());
    for (const string &c : s.classCodes) {
        if (c == code) {
            found = true;
        } else {
            newList.push_back(c);
        }
    }
    if (!found) {
        cout << "unsuccessful" << endl;
        return false;
    }
    s.classCodes.swap(newList);

    if (s.classCodes.empty()) {
        studentsById.erase(it);
    }
    cout << "successful" << endl;
    return true;
}

bool CampusCompass::handleReplaceClass(const string &line) {
    string cmd, ufid, oldCode, newCode;
    stringstream ss(line);
    ss >> cmd >> ufid >> oldCode >> newCode;

    if (!ss || ufid.empty() || oldCode.empty() || newCode.empty()) {
        cout << "unsuccessful" << endl;
        return false;
    }

    //validate uf id and class code format
    if (!isValidUFID(ufid) || !isValidClassCode(oldCode) || !isValidClassCode(newCode)) {
        cout << "unsuccessful" << endl;
        return false;
    }

    //student must exist
    auto it = studentsById.find(ufid);
    if (it == studentsById.end()) {
        cout << "unsuccessful" << endl;
        return false;
    }

    Student &s = it->second;

    //student must have the old code
    auto posOld = find(s.classCodes.begin(), s.classCodes.end(), oldCode);
    if (posOld == s.classCodes.end()) {
        cout << "unsuccessful" << endl;
        return false;
    }
    if (find(s.classCodes.begin(), s.classCodes.end(), newCode) != s.classCodes.end()) {
        cout << "unsuccessful" << endl;
        return false;
    }

    //newCode must exist in the classes.csv now
    if (!classesByCode.count(newCode)) {
        cout << "unsuccessful" << endl;
        return false;
    }
    * posOld = newCode;
    cout << "successful" << endl;
    return true;
}
