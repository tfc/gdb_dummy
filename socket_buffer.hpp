#include <iostream>
#include <iterator>
#include <memory>
#include <unistd.h>

#include <attoparsecpp/parser.hpp>

class socket_pos : public apl::buffer_pos {
    int fd;

    using buf_t = std::array<char, 1000>;
    std::shared_ptr<buf_t> buffer {std::make_shared<buf_t>()};
    buf_t::const_iterator end_it  {std::begin(*buffer)};
    buf_t::const_iterator it      {end_it};

public:
    socket_pos(int fd_) : fd{fd_} {}
    socket_pos(const std::string &s) : fd{0} {
        std::copy(std::cbegin(s), std::cend(s), std::begin(*buffer));
        end_it = std::cbegin(*buffer) + s.size();
    }

    virtual char operator*() const override { return *it; }

    virtual buffer_pos& operator++() override {
        ++it;
        return *this;
    }

    virtual bool at_end() override {
        if (it != end_it) { return false; }
        std::fill(std::begin(*buffer), std::end(*buffer), 0);

        //std::cout << "[buffer] reading... ";
        const ssize_t n {read(fd, buffer->data(), buffer->size())};
        if (n <= 0) {
            //std::cout << "connection closed (" << n << ")\n";
            return true;
        }

        it = std::begin(*buffer);
        end_it = it + n;

        print_buffer();
        std::cout << " " << n << " B\n";

        return false;
    }

    void print_buffer() const {
        std::cout << '[';
        std::copy(it, end_it, std::ostream_iterator<char>{std::cout});
        std::cout << ']';
    }
};
