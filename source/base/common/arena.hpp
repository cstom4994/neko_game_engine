#pragma once

#include "base/common/array.hpp"
#include "base/common/base.hpp"
#include "base/common/mem.hpp"
#include "base/common/string.hpp"

namespace Neko {

struct ArenaNode {
    ArenaNode* next;
    u64 capacity;
    u64 allocd;
    u64 prev;
    u8 buf[1];
};

static u64 align_forward(u64 p, u32 align) {
    if ((p & (align - 1)) != 0) {
        p += align - (p & (align - 1));
    }
    return p;
}

static ArenaNode* arena_block_make(u64 capacity) {
    u64 page = 4096 - offsetof(ArenaNode, buf);
    if (capacity < page) {
        capacity = page;
    }

    ArenaNode* a = (ArenaNode*)mem_alloc(offsetof(ArenaNode, buf[capacity]));
    a->next = nullptr;
    a->allocd = 0;
    a->capacity = capacity;
    return a;
}

struct Arena {
    ArenaNode* head;

    inline void trash() {
        ArenaNode* a = head;
        while (a != nullptr) {
            ArenaNode* rm = a;
            a = a->next;
            mem_free(rm);
        }
    }

    inline void* bump(u64 size) {
        if (head == nullptr) {
            head = arena_block_make(size);
        }

        u64 next = 0;
        do {
            next = align_forward(head->allocd, 16);
            if (next + size <= head->capacity) {
                break;
            }

            ArenaNode* block = arena_block_make(size);
            block->next = head;

            head = block;
        } while (true);

        void* ptr = &head->buf[next];
        head->allocd = next + size;
        head->prev = next;
        return ptr;
    }

    inline void* rebump(void* ptr, u64 old, u64 size) {
        if (head == nullptr || ptr == nullptr || old == 0) {
            return bump(size);
        }

        if (&head->buf[head->prev] == ptr) {
            u64 resize = head->prev + size;
            if (resize <= head->capacity) {
                head->allocd = resize;
                return ptr;
            }
        }

        void* new_ptr = bump(size);

        u64 copy = old < size ? old : size;
        memmove(new_ptr, ptr, copy);

        return new_ptr;
    }

    inline String bump_string(String s) {
        if (s.len > 0) {
            char* cstr = (char*)bump(s.len + 1);
            memcpy(cstr, s.data, s.len);
            cstr[s.len] = '\0';
            return {cstr, s.len};
        } else {
            return {};
        }
    }
};

template <typename T>
struct Slice {
    T* data = nullptr;
    u64 len = 0;

    Slice() = default;
    explicit Slice(Array<T> arr) : data(arr.data), len(arr.len) {}

    T& operator[](size_t i) {
        assert(i >= 0 && i < len);
        return data[i];
    }

    const T& operator[](size_t i) const {
        assert(i >= 0 && i < len);
        return data[i];
    }

    void resize(u64 n) {
        T* buf = (T*)mem_alloc(sizeof(T) * n);
        memcpy(buf, data, sizeof(T) * len);
        mem_free(data);
        data = buf;
        len = n;
    }

    void resize(Arena* arena, u64 n) {
        T* buf = (T*)arena->rebump(data, sizeof(T) * len, sizeof(T) * n);
        data = buf;
        len = n;
    }

    T* begin() { return data; }
    T* end() { return &data[len]; }
    const T* begin() const { return data; }
    const T* end() const { return &data[len]; }
};

template <typename T>
class LinkedList {
public:
    struct link_t {
        friend class LinkedList;
        link_t() : _val(), _pnext(nullptr), _pprev(nullptr) {}
        bool rlast(void) const { return (_pprev == nullptr ? true : false); }
        bool last(void) const { return (_pnext == nullptr ? true : false); }

    public:
        T _val;

    private:
        link_t* _pnext;
        link_t* _pprev;
    };

    struct iterator_t {
        iterator_t() : plink(nullptr), skipnext(false) {}

        link_t* plink;
        bool skipnext;
    };

public:
    inline LinkedList(void);
    inline LinkedList(LinkedList& src);
    inline ~LinkedList(void);

    inline link_t* add(T element);
    inline link_t* radd(T element);
    inline link_t* insert_before(link_t* link, T element);
    inline link_t* insert_after(link_t* link, T element);
    inline void remove(const link_t* link);
    inline bool remove(const T element);
    inline link_t* get_link(void);
    inline T& get(void);
    inline void begin(void);
    inline void begin(link_t* link);
    inline void rbegin(void);
    inline bool end(void) const;
    inline void next(void);
    inline void prev(void);
    inline void clear(void);
    inline bool empty(void) const;
    inline Uint32 size(void) const;
    inline void push_iterator(void);
    inline void pop_iterator(void);

    inline LinkedList<T>& operator=(const LinkedList<T>& src);

private:
    link_t* m_pLinkHead;
    link_t* m_pLinkTail;
    iterator_t m_iterator;
    Uint32 m_numLinks;
    CArray<iterator_t> m_pushedIteratorsArray;
};

template <typename T>
inline LinkedList<T>::LinkedList() : m_pLinkHead(nullptr), m_pLinkTail(nullptr), m_numLinks(0) {}

template <typename T>
inline LinkedList<T>::LinkedList(LinkedList& src) : m_pLinkHead(nullptr), m_pLinkTail(nullptr), m_numLinks(0) {
    src.begin();
    while (!src.end()) {
        add(src.get());
        src.next();
    }

    if (!m_pushedIteratorsArray.empty()) m_pushedIteratorsArray.clear();
}

template <typename T>
inline LinkedList<T>::~LinkedList() {
    clear();
}

template <typename T>
inline typename LinkedList<T>::link_t* LinkedList<T>::add(const T element) {
    typename LinkedList<T>::link_t* pNew = new typename LinkedList<T>::link_t;

    pNew->_pnext = m_pLinkHead;
    pNew->_val = element;

    if (m_pLinkHead) m_pLinkHead->_pprev = pNew;

    if (!m_pLinkTail) m_pLinkTail = pNew;

    m_pLinkHead = pNew;
    m_numLinks++;

    return pNew;
}

template <typename T>
inline typename LinkedList<T>::link_t* LinkedList<T>::radd(const T element) {
    typename LinkedList<T>::link_t* pNew = new typename LinkedList<T>::link_t;

    pNew->_pprev = m_pLinkTail;
    pNew->_val = element;

    if (m_pLinkTail) m_pLinkTail->_pnext = pNew;

    if (!m_pLinkHead) m_pLinkHead = pNew;

    m_pLinkTail = pNew;
    m_numLinks++;

    return pNew;
}

template <typename T>
inline typename LinkedList<T>::link_t* LinkedList<T>::insert_before(typename LinkedList<T>::link_t* link, const T element) {
    typename LinkedList<T>::link_t* pNew = new typename LinkedList<T>::link_t;

    if (link->_pprev) {
        link->_pprev->_pnext = pNew;
        pNew->_pprev = link->_pprev;
    } else
        m_pLinkHead = pNew;

    link->_pprev = pNew;
    pNew->_pnext = link;

    pNew->_val = element;
    m_numLinks++;

    return pNew;
}

template <typename T>
inline typename LinkedList<T>::link_t* LinkedList<T>::insert_after(typename LinkedList<T>::link_t* link, const T element) {
    typename LinkedList<T>::link_t* pNew = new typename LinkedList<T>::link_t;

    if (link->_pnext) {
        link->_pnext->_pprev = pNew;
        pNew->_pnext = link->_pnext;
    } else
        m_pLinkTail = pNew;

    link->_pnext = pNew;
    pNew->_pprev = link;

    pNew->_val = element;
    m_numLinks++;

    return pNew;
}

template <typename T>
inline void LinkedList<T>::remove(typename const LinkedList<T>::link_t* link) {
    if (link == m_iterator.plink) {
        m_iterator.plink = m_iterator.plink->_pnext;
        m_iterator.skipnext = true;
    }

    for (Uint32 i = 0; i < m_pushedIteratorsArray.size(); i++) {
        if (m_pushedIteratorsArray[i].plink == link) {
            m_pushedIteratorsArray[i].plink = m_pushedIteratorsArray[i].plink->_pnext;
            m_pushedIteratorsArray[i].skipnext = true;
        }
    }

    if (!link->_pprev)
        m_pLinkHead = link->_pnext;
    else
        link->_pprev->_pnext = link->_pnext;

    if (link->_pnext) link->_pnext->_pprev = link->_pprev;

    if (m_pLinkTail == link) m_pLinkTail = link->_pprev;

    delete link;
    m_numLinks--;
}

template <typename T>
inline bool LinkedList<T>::remove(const T element) {
    if (!m_pLinkHead) return false;

    typename LinkedList<T>::link_t* pnext = m_pLinkHead;
    while (pnext) {
        if (pnext->_val == element) {
            remove(pnext);
            return true;
        }

        pnext = pnext->_pnext;
    }

    return false;
}

template <typename T>
inline typename LinkedList<T>::link_t* LinkedList<T>::get_link(void) {
    return m_iterator.plink;
}

template <typename T>
inline typename T& LinkedList<T>::get(void) {
    return m_iterator.plink->_val;
}

template <typename T>
inline void LinkedList<T>::begin(void) {
    m_iterator.plink = m_pLinkHead;
    m_iterator.skipnext = false;
}

template <typename T>
inline void LinkedList<T>::begin(link_t* link) {
    m_iterator.plink = link;
    m_iterator.skipnext = false;
}

template <typename T>
inline void LinkedList<T>::rbegin(void) {
    m_iterator.plink = m_pLinkTail;
    m_iterator.skipnext = false;
}

template <typename T>
inline bool LinkedList<T>::end(void) const {
    return (!m_iterator.plink ? true : false);
}

template <typename T>
inline void LinkedList<T>::next(void) {
    if (!m_iterator.plink) return;

    if (m_iterator.skipnext) {
        m_iterator.skipnext = false;
        return;
    }

    m_iterator.plink = m_iterator.plink->_pnext;
}

template <typename T>
inline void LinkedList<T>::prev(void) {
    if (!m_iterator.plink) return;

    if (m_iterator.skipnext) {
        m_iterator.skipnext = false;
        return;
    }

    m_iterator.plink = m_iterator.plink->_pprev;
}

template <typename T>
inline void LinkedList<T>::clear(void) {
    if (!m_pLinkHead) return;

    typename LinkedList<T>::link_t* pnext = m_pLinkHead;
    while (pnext) {
        typename LinkedList<T>::link_t* pfree = pnext;
        pnext = pfree->_pnext;

        remove(pfree);
    }

    m_numLinks = 0;

    if (!m_pushedIteratorsArray.empty()) m_pushedIteratorsArray.clear();
}

template <typename T>
inline bool LinkedList<T>::empty(void) const {
    return (!m_pLinkHead) ? true : false;
}

template <typename T>
inline Uint32 LinkedList<T>::size(void) const {
    return m_numLinks;
}

template <typename T>
inline void LinkedList<T>::push_iterator(void) {
    m_pushedIteratorsArray.push_back(m_iterator);
}

template <typename T>
inline void LinkedList<T>::pop_iterator(void) {
    assert(!m_pushedIteratorsArray.empty());

    const Int32 index = m_pushedIteratorsArray.size() - 1;
    m_iterator = m_pushedIteratorsArray[index];

    m_pushedIteratorsArray.resize(m_pushedIteratorsArray.size() - 1);
}

template <typename T>
inline LinkedList<T>& LinkedList<T>::operator=(const LinkedList<T>& src) {
    if (!empty()) clear();

    LinkedList<T>::iterator_t it;
    it.plink = src.m_pLinkHead;
    while (it.plink) {
        add(it.plink->_val);
        it.plink = it.plink->_pnext;
    }

    if (!m_pushedIteratorsArray.empty()) m_pushedIteratorsArray.clear();

    return *this;
}

}  // namespace Neko
