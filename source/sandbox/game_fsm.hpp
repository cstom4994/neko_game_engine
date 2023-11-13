
#ifndef GAME_FSM_HPP
#define GAME_FSM_HPP

#include <functional>
#include <set>
#include <stack>
#include <tuple>
#include <vector>

template <typename T>
using fsm_pi = std::function<bool(T)>;  // fsm_pi for predicate

template <typename T>
fsm_pi<T> is(T a) {
    return [a](T b) { return b == a; };
}

template <typename T>
fsm_pi<T> no(T a) {
    return [a](T b) { return b != a; };
}

template <typename T>
fsm_pi<T> no(std::set<T> a) {
    return [a](T b) { return a.find(b) == a.end(); };
}

template <typename T>
fsm_pi<T> is(std::set<T> a) {
    return [a](T b) { return a.find(b) != a.end(); };
}

template <typename T>
fsm_pi<T> no(fsm_pi<T> inner) {
    return [inner](T x) { return !inner(x); };
}

template <typename T>
fsm_pi<T> any() {
    return [](T b) { return true; };
}

template <typename M, typename S>
class neko_fsm;

template <typename M, typename S>
neko_fsm<M, S> &operator<<(neko_fsm<M, S> &, M);

// @ https://en.wikipedia.org/wiki/Finite-state_machine#Mathematical_model
// M 是输入字母表 有限非空类型的符号
// S 是有限非空类型的状态
template <typename M, typename S>
class neko_fsm {
public:
    // 状态转换函数类型
    using delta = std::function<S(S, M)>;           // δ
    using gamma = std::function<neko_fsm *(S, M)>;  // γ

    neko_fsm(S initial, fsm_pi<S> is_terminal) : current(initial), is_terminal(is_terminal) {
        machine_stack = new std::stack<neko_fsm *>();
        machine_stack->push(this);
    }

#pragma region Simple transition registration

    delta give(S value) {  // S -> (S⨯M -> S)
        return [value](S, M) { return value; };
    }

    void on(M in, S from, S to) { return on(is(in), is(from), give(to)); }

    void on(std::set<M> in, S from, S to) { return on(is(in), is(from), give(to)); }

    void on(fsm_pi<M> in, S from, S to) { return on(in, is(from), give(to)); }

    void on(M in, fsm_pi<S> from, S to) { return on(is(in), from, give(to)); }

    void on(std::set<M> in, fsm_pi<S> from, S to) { return on(is(in), from, give(to)); }

    void on(fsm_pi<M> in, fsm_pi<S> from, S to) { return on(in, from, give(to)); }

    void on(M in, S from, delta t) { return on(is(in), is(from), t); }

    void on(std::set<M> in, S from, delta t) { return on(is(in), is(from), t); }

    void on(fsm_pi<M> in, S from, delta t) { return on(in, is(from), t); }

    void on(M in, fsm_pi<S> from, delta t) { return on(is(in), from, t); }

    void on(std::set<M> in, fsm_pi<S> from, delta t) { return on(is(in), from, t); }

    void on(fsm_pi<M> in, fsm_pi<S> from, delta t) { transitions.push_back(std::make_tuple(in, from, t)); }

#pragma endregion

#pragma region Submachine transition registration

    void on(M in, S from, gamma t) { return on(is(in), is(from), t); }

    void on(M in, fsm_pi<S> from, gamma t) { return on(is(in), from, t); }

    void on(fsm_pi<M> in, fsm_pi<S> from, gamma t) { submachines.push_back(std::make_tuple(in, from, t)); }

#pragma endregion

    friend neko_fsm &operator<< <>(neko_fsm &, M);

    S state() const { return current; }

private:
    S current;
    fsm_pi<S> is_terminal;
    std::vector<std::tuple<fsm_pi<M>, fsm_pi<S>, delta>> transitions;
    std::vector<std::tuple<fsm_pi<M>, fsm_pi<S>, gamma>> submachines;
    std::stack<neko_fsm *> *machine_stack;
};

// 处理输入并转换到新状态
template <typename M, typename S>
neko_fsm<M, S> &operator<<(neko_fsm<M, S> &m, M input) {
    bool found = false;
    // 处理上层转换
    S new_state;
    for (auto tup : m.transitions) {
        auto input_pred = std::get<0>(tup);
        auto state_pred = std::get<1>(tup);
        if (state_pred(m.current) && input_pred(input)) {
            new_state = std::get<2>(tup)(m.current, input);
            found = true;
        }
    }
    if (found) {
        m.current = new_state;
        if (m.is_terminal(m.current)) {
            m.machine_stack->pop();
            return *m.machine_stack->top();
        }
        return m;
    }
    // 处理子机转换
    neko_fsm<M, S> *sub;
    for (auto tup : m.submachines) {
        auto input_pred = std::get<0>(tup);
        auto state_pred = std::get<1>(tup);
        if (state_pred(m.current) && input_pred(input)) {
            sub = std::get<2>(tup)(m.current, input);
            found = true;
        }
    }
    if (found) {
        m.machine_stack->push(sub);
        sub->machine_stack = m.machine_stack;
        return *m.machine_stack->top();
    }
    throw "Invalid input";
}

template <typename M, typename S>
neko_fsm<M, S> &operator<<=(neko_fsm<M, S> &m, M input) {
    m = m << input;
    return m;
}

#endif