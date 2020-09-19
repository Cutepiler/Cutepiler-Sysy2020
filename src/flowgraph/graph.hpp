#pragma once

#include <vector>

template <class TV, class TE>
struct Edge;

template <class TV, class TE>
struct Vertex {
    TV value;
    std::vector<Edge<TV, TE> *> in, out;
    Vertex(const TV &value) : value(value) {}
};

template <class TV, class TE>
struct Edge {
    TE value;
    Vertex<TV, TE> *from, *to;
    Edge(const TE &value, Vertex<TV, TE> *from, Vertex<TV, TE> *to)
        : value(value), from(from), to(to) {}
};

template <class TV, class TE>
struct Graph {
    std::set<Vertex<TV, TE> *> vertices;
    std::set<Edge<TV, TE> *> edges;
};