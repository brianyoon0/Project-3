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

//helper function to trim
static string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, (last - first + 1));
}

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
                break;
            }
        }
    }
    //toggle v->u
    if (adj.count(v)) {
        for (auto &e : adj.at(v)) {
            if (e.to == u) {
                e.open = !e.open;
                break;
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
                return "open";
            } else {
                return "closed";
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

unordered_map<int, int> Graph::dijkstraWithParents(
    int src, unordered_map<int, int> &parent) const {
    unordered_map<int, int> dist;

    for (const auto &p : adj) {
        dist[p.first] = INF;
    }
    if (!adj.count(src)) {
        return dist;
    }
    dist[src] = 0;
    parent.clear();
    parent[src] = -1; // root has no parent

    using NodePair = pair<int, int>;
    priority_queue<NodePair, vector<NodePair>, greater<NodePair>> pq;
    pq.push({0, src});
    while (!pq.empty()) {
        auto [d, u] = pq.top();
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
                parent[v] = u;
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

int Graph::mstCostOnNodes(const unordered_set<int> &nodes) const {
    if (nodes.empty()) {
        return 0;
    }
    //prim's algorithm on the subgraph
    unordered_set<int> visited;
    int totalCost = 0;

    //start from any node in the set
    int start = *nodes.begin();
    visited.insert(start);

    using EdgePair = pair<int, int>;
    priority_queue<EdgePair, vector<EdgePair>, greater<EdgePair>> pq;

    //helper lambda to push edges from the given node
    auto pushEdges = [&](int u) {
        auto it = adj.find(u);
        if (it == adj.end()) return;
        for (const auto &e : it->second) {
            if (!e.open) continue;
            int v = e.to;
            if (!nodes.count(v)) continue;
            if (visited.count(v)) continue;
            pq.push({e.time, v});
        }
    };
    pushEdges(start);
    while (!pq.empty() && visited.size() < nodes.size()) {
        auto [w, v] = pq.top();
        pq.pop();
        if (visited.count(v)) continue;
        visited.insert(v);
        totalCost += w;
        pushEdges(v);
    }
    if (visited.size() != nodes.size()) {
        return -1;
    }
    return totalCost;
}







CampusCompass::CampusCompass() = default;

int CampusCompass::parseTimetoMinutes(const string &timeStr) const {
    if (timeStr.empty()) return -1;

    string trimmed = trim(timeStr);
    if (trimmed.empty()) return -1;
    int h = 0;
    int m = 0;
    char colon;
    stringstream ss(trimmed);
    ss >> h >> colon >> m;
    if (!ss || colon !=':') {
        return -1;
    }
    if (h <  0 || h > 23 || m < 0 || m > 59) {
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

        getline(ss, name1, ',');
        getline(ss, name2, ',');
        if (!getline(ss, sTime, ',')) continue;

        sId1 = trim(sId1);
        sId2 = trim(sId2);
        sTime = trim(sTime);

        if (sId1.empty() || sId2.empty() || sTime.empty())continue;

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
    getline(in, line);
    while (getline(in, line)) {
        if (line.empty()) continue;


        stringstream ss(line);
        string code, sLoc, sStart, sEnd;

        if (!getline(ss, code, ',')) continue;
        if (!getline(ss, sLoc, ',')) continue;
        if (!getline(ss, sStart, ',')) continue;
        getline(ss, sEnd, ',');

        code = trim(code);
        sLoc = trim(sLoc);
        sStart = trim(sStart);
        sEnd = trim(sEnd);

        if (code.empty() || sLoc.empty() || sStart.empty() || sEnd.empty()) continue;

        ClassInfo info;
        try {
            info.locationId = stoi(sLoc);
            info.startMinutes = parseTimetoMinutes(sStart);
            info.endMinutes = parseTimetoMinutes(sEnd);

            // Validate times
            if (info.startMinutes == -1 || info.endMinutes == -1 || info.startMinutes >= info.endMinutes) {
                continue; // Skip invalid time entries
            }

        classesByCode[code] = info;
        } catch (const exception& e) {
            cerr << "Error parsing line: " << line << endl;
            continue;
        }
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
        if (!isalpha(static_cast<unsigned char>(code[i])) || !isupper(code[i])) return false;
    }
    for (int i = 3; i < 7; i++) {
        if (!isdigit(static_cast<unsigned char>(code[i]))) return false;
    }
    return true;
}


bool CampusCompass::ParseCommand(const string &command) {
    string trimmedCmd = trim(command);
    if (trimmedCmd.empty()) {
        cout << "unsuccessful\n";
        return false;
    }

    string cmdWord;
    stringstream ss(trimmedCmd);
    ss >> cmdWord;

    if (!ss) {
        cout << "unsuccessful\n";
        return false;
    }
    if (cmdWord == "insert") {
        return handleInsert(trimmedCmd);
    } else if (cmdWord == "remove") {
        return handleRemove(trimmedCmd);
    } else if (cmdWord == "removeClass") {
        return handleRemoveClass(trimmedCmd);
    } else if (cmdWord == "dropClass") {
        return handleDropClass(trimmedCmd);
    } else if (cmdWord == "replaceClass") {
        return handleReplaceClass(trimmedCmd);
    } else if (cmdWord == "toggleEdgesClosure") {
        return handleToggleEdgesClosure(trimmedCmd);
    } else if (cmdWord == "checkEdgeStatus") {
        return handleCheckEdgeStatus(trimmedCmd);
    } else if (cmdWord == "isConnected") {
        return handleIsConnected(trimmedCmd);
    } else if (cmdWord == "printShortestEdges") {
        return handlePrintShortestEdges(trimmedCmd);
    } else if (cmdWord == "printStudentZone") {
        return handlePrintStudentZone(trimmedCmd);
    } else if (cmdWord == "verifySchedule") {
        return handleVerifySchedule(trimmedCmd);
    }

    cout << "unsuccessful" << endl;
    return false;
}

bool CampusCompass::handleInsert(const string &line) {

    //find the name
    size_t firstQuote = line.find('"');
    if (firstQuote == string::npos) {
        cout << "unsuccessful" << endl;
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
        cout << "unsuccessful\n";
        return false;
    }

    vector<string> codes;
    string code;
    while (ss >> code) {
        code = trim(code);
        if (!code.empty()) {
            codes.push_back(code);
        }
    }
    //checks
    if (n < 1 || n > 6) {
        cout << "unsuccessful\n";
        return false;
    }
    if (n != static_cast<int>(codes.size())) {
        cout << "unsuccessful" << endl;
        return false;
    }
    //make sure its valid
    if (!isValidName(name) || !isValidUFID(ufid)) {
        cout << "unsuccessful\n" ;
        return false;
    }
    if (studentsById.count(ufid)) {
        cout << "unsuccessful\n" ;
        return false;
    }
    for (const string &c: codes) {
        if (!isValidClassCode(c)) {
            cout << "unsuccessful\n" ;
            return false;
        }
        if (!classesByCode.count(c)) {
            cout << "unsuccessful\n";
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
        cout << "unsuccessful" << endl ;
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
    for (const string &ufid : toErase) {
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

    ufid = trim(ufid);
    oldCode = trim(oldCode);
    newCode = trim(newCode);

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
    cout << "successful"<< endl;
    return true;
}

bool CampusCompass::handleToggleEdgesClosure(const string &line) {
    string cmd;
    int N;
    stringstream ss(line);
    ss >> cmd >> N;
    if (!ss || N <= 0) {
        cout << "unsuccessful" << endl;
        return false;
    }
    vector<int> ids;
    int id;

    while (ss >> id) {
        ids.push_back(id);
    }

    if (static_cast<int>(ids.size()) != 2* N) {
        cout << "unsuccessful" << endl;
        return false;
    }
    //toggle each edge
    for (int i = 0; i < 2* N; i += 2) {
        int u = ids[i];
        int v = ids[i + 1];

        graph.toggleEdge(u,v);
    }
    cout << "successful" << endl;
    return true;
}

bool CampusCompass::handleCheckEdgeStatus(const string &line) {
    string cmd;
    int x, y;
    stringstream ss(line);
    ss >> cmd >> x >> y;

    if (!ss) {
        cout << "unsuccessful" << endl;
        return false;
    }
    string status = graph.getEdgeStatus(x, y);

    cout << status << endl;
    return true;
}

bool CampusCompass::handleIsConnected(const string &line) {

    string cmd;
    int start, goal;
    stringstream ss(line);
    ss >> cmd >> start >> goal;

    if (!ss) {
        cout << "unsuccessful" << endl;
        return false;
    }
    bool ok = graph.isConnected(start, goal);

    if (ok) {
        cout << "successful" << endl;
    } else {
        cout << "unsuccessful" << endl;
    }
    return ok;
}

bool CampusCompass::handlePrintShortestEdges(const string &line) {
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
        cout << "unsuccessful\n" ;
        return false;
    }
    Student &s = it->second;

    //dijkstra
    int startLocation = s.residenceLocation;
    unordered_map<int, int> dist = graph.dijkstra(startLocation);

    vector<pair<string, int>> results;

    for (const string &code : s.classCodes) {
        auto classIt = classesByCode.find(code);
        if (classIt == classesByCode.end()) {
            results.push_back({code, -1});
            continue;
        }

        int loc = classIt->second.locationId;

        int time = -1;
        auto dIt = dist.find(loc);
        if (dIt != dist.end() && dIt->second != INF) {
            time = dIt->second;
        }
        results.push_back({code, time});
    }
    sort(results.begin(), results.end());

    cout << "Name: " << s.name << '\n';
    for (const auto &p : results) {
        cout << p.first << " | Total Time: " << p.second << endl;
    }
    return true;
}

bool CampusCompass::handlePrintStudentZone(const string &line) {
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
    Student &s = it->second;
    int startLoc = s.residenceLocation;
    unordered_map<int, int> parent;
    unordered_map<int, int> dist = graph.dijkstraWithParents(startLoc, parent);

    unordered_set<int> zoneNodes;
    zoneNodes.insert(startLoc);

    for (const string &code : s.classCodes) {
        auto classIt = classesByCode.find(code);
        if (classIt == classesByCode.end()) {
            continue;
        }
        int dest = classIt->second.locationId;

        auto dIt = dist.find(dest);
        if (dIt == dist.end() || dIt->second == INF) {
            continue;
        }
        int cur = dest;
        while (cur != -1 && cur != startLoc) {
            zoneNodes.insert(cur);
            auto pIt = parent.find(cur);
            if (pIt == parent.end()) {
                break;
            }
            cur = pIt->second;
        }
        zoneNodes.insert(startLoc);
    }
    if (zoneNodes.size() < 2) {
        cout << "Student Zone Cost For " << s.name << ": 0" << endl;
        return true;
    }

    int cost = graph.mstCostOnNodes(zoneNodes);
    cout << "Student Zone Cost For " << s.name << ": " << cost << endl;
    return true;
}

bool CampusCompass::handleVerifySchedule(const string &line) {
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
    Student &s = it->second;

    if (s.classCodes.size() < 2) {
        cout << "unsuccessful" << endl;
        return false;
    }

    struct ClassSlot {
        string code;
        int locationId;
        int start;
        int end;
    };
    vector<ClassSlot> slots;
    slots.reserve(s.classCodes.size());
    for (const string &code : s.classCodes) {
        auto ci = classesByCode.find(code);
        if (ci == classesByCode.end()) {
            continue;
        }
        ClassSlot slot;
        slot.code = code;
        slot.locationId = ci->second.locationId;
        slot.start = ci->second.startMinutes;
        slot.end = ci->second.endMinutes;

        slots.push_back(slot);
    }
//after this they have less than 2 valid classes its not real
    if (slots.size() < 2) {
        cout << "unsuccessful" << endl;
        return false;
    }

    sort(slots.begin(), slots.end(),
        [](const ClassSlot &a, const ClassSlot &b) {
            return a.start < b.start;
        });
    cout << "Schedule Check for " << s.name << endl;

    for (size_t i = 0; i + 1 < slots.size(); i++) {
        const ClassSlot &c1 = slots[i];
        const ClassSlot &c2 = slots[i + 1];

        int gap = c2.start - c1.end;

        string result;
        if (gap < 0) {
            result = "Overlapping of times";
        } else {
            unordered_map<int, int> dist = graph.dijkstra(c1.locationId);
            int travel = -1;

            auto dIt = dist.find(c2.locationId);
            if (dIt != dist.end() && dIt->second != INF) {
                travel = dIt->second;
            }
            if (travel == -1 || travel > gap) {
                result = "Can't make it";
            } else {
                result = "Can make it";
            }
        }
        cout << c1.code << " - " << c2.code << " \"" << result << "\"" << endl;
    }
    return true;
}

