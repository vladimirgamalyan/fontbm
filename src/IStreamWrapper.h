#pragma once

class IStreamWrapper {
public:
    typedef char Ch;
    IStreamWrapper(std::istream& is) : is_(is) {
    }
    Ch Peek() const { // 1
        int c = is_.peek();
        return c == std::char_traits<char>::eof() ? '\0' : (Ch)c;
    }
    Ch Take() { // 2
        int c = is_.get();
        return c == std::char_traits<char>::eof() ? '\0' : (Ch)c;
    }
    size_t Tell() const { return (size_t)is_.tellg(); } // 3
    Ch* PutBegin() { assert(false); return 0; }
    void Put(Ch) { assert(false); }
    void Flush() { assert(false); }
    size_t PutEnd(Ch*) { assert(false); return 0; }
private:
    IStreamWrapper(const IStreamWrapper&);
    IStreamWrapper& operator=(const IStreamWrapper&);
    std::istream& is_;
};
