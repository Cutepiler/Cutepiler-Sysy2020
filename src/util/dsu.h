#include <vector> 
#include <algorithm> 
#include <cassert> 

class DSU {
private: 
    std::vector<int> parent; 
public:
    /* indices from 0 to @siz */
    DSU(int siz) 
    { 
        parent.resize(siz); 
        std::fill(parent.begin(), parent.end(), -1);    
    }
    int getRep(int id)
    {
        assert(0 <= id && id < parent.size());
        return parent[id] == -1 ? 
               id : 
               (parent[id] = getRep(parent[id]));
    }
    void link(int id, int tar)
    {
        parent[id] = tar; 
    }
    void unionSet(int u, int v)
    {
        link(u, v); 
    }
    bool sameSet(int u, int v)
    {
        return getRep(u) == getRep(v); 
    }
}; 
