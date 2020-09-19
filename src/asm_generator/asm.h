#pragma once

#include <memory>
#include <string>

#include "../tac/tac.h"

using std::string;

struct Line;
using LinePtr = std::shared_ptr<Line>;
struct Line {
    string content;
    LinePtr pred, succ;
    int length;

    Line(const string &content, int length) : content(content), length(length) {}

    void insert(LinePtr s) {
        auto ss = succ;
        s->pred = ss->pred;
        s->succ = ss;
        succ = ss->pred = s;
    }

    void insert(const Line &line) { insert(std::make_shared<Line>(line)); }

    void insert(const string &content, int length) {
        insert(std::make_shared<Line>(content, length));
    }

    void insert(LinePtr head, LinePtr tail) {
        head->pred->succ = tail->succ;
        tail->succ->pred = head->pred;
        auto s = succ, p = succ->pred;
        p->succ = head;
        head->pred = p;
        s->pred = tail;
        tail->succ = s;
    }

    void remove() {
        auto p = pred, s = succ;
        pred = succ = nullptr;
        p->succ = s;
        s->pred = p;
    }
};

struct Lines {
    LinePtr head, tail;

    Lines() {
        head = std::make_shared<Line>("", 0);
        tail = std::make_shared<Line>("", 0);
        head->succ = tail;
        tail->pred = head;
    }

    bool empty() const { return head->succ == tail; }

    void push_back(LinePtr line) { tail->pred->insert(line); }
    void push_front(LinePtr line) { head->insert(line); }
    void push_back(Lines lines) {
        if (lines.empty()) return;
        tail->pred->insert(lines.head->succ, lines.tail->pred);
    }
    void push_front(Lines lines) {
        if (lines.empty()) return;
        head->insert(lines.head->succ, lines.tail->pred);
    }

    static int line_number(LinePtr line) {
        assert(line->content != "");
        int counter = 0;
        while (line->content != "") {
            line = line->pred;
            ++counter;
        }
        return counter;
    }
};

struct AsmProg {
    Lines prog;
    AsmProg(const TacProg &tacProg);
};

std::ostream &operator<<(std::ostream &os, const AsmProg &prog);