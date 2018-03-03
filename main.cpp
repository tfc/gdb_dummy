#include <iostream>
#include <array>

#include "socket_buffer.hpp"
using str_pos = read_buffer;
#define __USE_OWN_STRPOS_IMPL__
#include <attoparsecpp/parser.hpp>

#include "socket.hpp"

static int global_fd {0};

namespace {
using namespace apl;

template <typename P, typename F>
static auto trans_parse(P p, F f) {
    return [p, f] (str_pos &pos) -> parser<typename std::result_of<F(parser_payload_type<P>)>::type> {
        if (auto ret {p(pos)}) {
            return {f(*ret)};
        }
        return {};
    };
}

static parser<bool> gdb_ok_msg(str_pos &pos) {
    return trans_parse(oneOf('+'), [] (auto) { std::cout << "<- OK\n"; return true; })(pos);
}

static parser<bool> break_msg(str_pos &pos) {
    return trans_parse(choice(const_string("swbreak"), const_string("hwbreak")),
        [] (const std::string &brk) {
            std::cout << "<- break msg: " << brk << '\n';
            return true;
        })(pos);
}

static parser<bool> v_messages(str_pos &pos) {
     return trans_parse(prefixed(oneOf('v'),
                              choice(const_string("fork-events"),
                                     const_string("ContSupported"))),
        [] (const std::string &msg) {
            std::cout << "<- v-prefix msg: " << msg << '\n';
            return true;
        })(pos);
}

static parser<bool> q_messages(str_pos &pos) {
    auto reloc = trans_parse(const_string("RelocInsn"),
            [] (const std::string &s) {
                std::cout << "<- qRelocInsn foo\n";
                return true;
            });
    auto supp = trans_parse(prefixed(const_string("Supported:"), many(noneOf('+'))),
            [] (const std::string &s) {
                std::cout << "<- supported: " << s << '\n';
                std::cout << "-> $#00 (empty qSupported answer)\n";

                const std::string empty {"$#00"};
                send(global_fd, empty.data(), empty.size(), 0);
                return true;
            });

     return prefixed(oneOf('q'), choice(reloc, supp))(pos);
}

static parser<bool> random_foo(str_pos &pos) {
    return trans_parse(choice(const_string("fork-events"),
                           const_string("exec-events"),
                           const_string("QThreadEvents"),
                           const_string("no-resumed")),
        [] (const std::string &brk) {
            std::cout << "<- whatever that means, it parses OK: " << brk << '\n';
            return true;
        })(pos);
}

static parser<bool> xml_registers(str_pos &pos) {
    return trans_parse(prefixed(const_string("xmlRegisters="),
                            many(noneOf('#', ';'))),
        [] (const std::string &register_support) {
            std::cout << "<- REGISTER support: " << register_support << '\n';
            return true;
        })(pos);
}

static parser<bool> retransmit_plz(str_pos &pos) {
    return trans_parse(oneOf('-'), [] (auto) {
            std::cout << "<- client wants retransmit Plz!\n";
            return true;
        })(pos);
}

static parser<bool> client_ack(str_pos &pos) {
    return trans_parse(oneOf('+'), [] (auto) {
            std::cout << "<- ACK\n";
            return true;
        })(pos);
}

static parser<bool> parse_gdb_command(str_pos &pos) {
    return choice(q_messages,
                  break_msg,
                  v_messages,
                  xml_registers,
                  random_foo
           )(pos);
}

static parser<std::vector<bool>> parse_gdb_message(str_pos &pos) {
    return choice(trans_parse(client_ack, [](bool b){ return std::vector<bool>{b}; }),
                  trans_parse(retransmit_plz, [](bool b){ return std::vector<bool>{b}; }),
                  clasped(oneOf('$'),
                          prefixed(oneOf('#'), base_integer(16, 2)),
                          sep_by1(parse_gdb_command, const_string("+;")))
            )(pos);
}

}

int main()
{
    std::cout << "Listening on port 1234. Open a GDB instance and enter "
                 "\"target remote tcp:localhost:1234\"\n";

    wait_for_connection(1234, [] (int socket_fd) {
        str_pos read_pos (socket_fd);
        if (auto init_ok {gdb_ok_msg(read_pos)}) {
            std::cout << "[main] client sent OK\n";
        } else {
            std::cout << "[main] connection failed early\n";
            return;
        }

        global_fd = socket_fd;
        while (auto ret {parse_gdb_message(read_pos)}) {
            std::cout << "[main] Read successful. Sending ACK.\n";
            // Ok, just answering "+" at this point seems to be not right.
            // i guess the client wants some more answers on his queries.
            send(socket_fd, "+", 1, 0);
        }
        std::cout << "[main] Read error\n";
    });
}
