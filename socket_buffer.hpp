#include <iostream>
#include <iterator>
#include <memory>
#include <unistd.h>

class read_buffer {
    int fd;

    using buf_t = std::array<char, 1000>;
    std::shared_ptr<buf_t> buffer {std::make_shared<buf_t>()};
    buf_t::const_iterator end_it  {std::begin(*buffer)};
    buf_t::const_iterator it      {end_it};

public:
    read_buffer(int fd_) : fd{fd_} {}
    read_buffer(const std::string &s) : fd{0} {
        std::copy(std::cbegin(s), std::cend(s), std::begin(*buffer));
        end_it = std::cbegin(*buffer) + s.size();
    }

    char operator*() const { return *it; }

    char consume() { return *(it++); }

    bool at_end() {
        if (it != end_it) { return false; }

        std::cout << "[buffer] empty. reading...\n";
        const ssize_t n {read(fd, buffer->data(), buffer->size())};
        if (n <= 0) {
            std::cout << "[buffer] connection closed {" << n << "}\n";
            return true;
        }

        it = std::begin(*buffer);
        end_it = it + n;

        std::cout << "[buffer] received " << n << " bytes: {{{";
        print_buffer();
        std::cout << "}}}\n";

        return false;
    }

    void print_buffer() const {
        std::copy(it, end_it, std::ostream_iterator<char>{std::cout});
    }

    read_buffer& next() {
        ++it;
        return *this;
    }
};
