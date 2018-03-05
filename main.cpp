#include <array>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>

#include <attoparsecpp/parser.hpp>

#include "socket_buffer.hpp"
#include "socket.hpp"

static int global_fd {0};

static uint8_t gdb_checksum(const std::string &s) {
    return std::accumulate(std::begin(s), std::end(s), static_cast<uint8_t>(0));
}

static std::string gdb_chksum_msg(const std::string &s) {
    std::ostringstream ss;
    ss << '$' << s << '#'
       << std::hex << std::setfill('0') << std::setw(2)
       << size_t(gdb_checksum(s));
    return ss.str();
}

static void send_msg_raw(const std::string &s) {
    std::cout << "-> " << s << '\n';
    send(global_fd, s.data(), s.size(), 0);
}

static void send_msg(const std::string &s) { send_msg_raw(gdb_chksum_msg(s)); }

static void send_ack(bool ack_for_valid_packet = true) {
    send_msg_raw(ack_for_valid_packet ? "+" : "-");
}

namespace {
using namespace apl;

static parser<std::string> checksum_parser(buffer_pos &pos) {
    if (const auto payload_str {clasped(oneOf('$'), oneOf('#'),
                                        many(noneOf('#'))
                                    )(pos)}) {
        const auto ref_chksum {base_integer<uint8_t>(16, 2)(pos)};
        if (ref_chksum == gdb_checksum(*payload_str)) { return payload_str; }
    }
    return {};
}

static parser<bool> gdb_ok_msg(buffer_pos &pos) {
    return map(oneOf('+'), [] (auto) {
        std::cout << "<- + (ACK)\n";
        return true;
    })(pos);
}

template <typename ... Ts>
static auto ignore_symbols(const Ts& ... ts) {
    return [ts...] (buffer_pos &pos) -> parser<bool> {
        return map(choice(const_string(ts)...),
            [](const auto &x) {
                std::cout << "<- " << x << " (whatever that means)\n";
                send_msg("");
                return true;
            })(pos);
    };
}

static auto simple_answer_on(const std::string &symbol,
                             const std::string &answer) {
    return [&symbol, &answer] (buffer_pos &pos) {
        return map(const_string(symbol),
                [&answer](const auto &x) {
                    std::cout << "<- " << x << '\n';
                    send_msg(answer);
                    return true;
                })(pos);
    };
}

static parser<bool> q_messages(buffer_pos &pos) {
    return prefixed(oneOf('q'),
                    choice(prefixed(oneOf('T'),
                                    choice(
                                        ignore_symbols("fV"),
                                        simple_answer_on("Status", "T0")
                                    )),
                           simple_answer_on("Supported", "PacketSize=2000"),
                           simple_answer_on("Attached", "1")
                          )
            )(pos);
}

static parser<bool> parse_gdb_message(buffer_pos &pos) {
    return choice(simple_answer_on("?", "S05"),
                  simple_answer_on("!", "OK"),
                  q_messages
            )(pos);
}

}

int main()
{
    std::cout << "Listening on port 1234. Open a GDB instance and enter "
                 "\"target remote tcp:localhost:1234\"\n";

    wait_for_connection(1234, [] (int socket_fd) {
        global_fd = socket_fd;
        socket_pos read_pos {socket_fd};

        if (auto ret {gdb_ok_msg(read_pos)}) {
            send_ack();
        } else {
            return;
        }

        while (auto chk_ret {checksum_parser(read_pos)}) {
            std::cout << "<- [" << *chk_ret << "]\n";
            send_ack();
            str_pos payload_pos {*chk_ret};
            if (auto ret {parse_gdb_message(payload_pos)}) {
                //std::cout << "[main] Read successful.\n";
                gdb_ok_msg(read_pos);
            } else {
                std::cout << "<- " << *chk_ret << " (UNKNOWN)\n";
                send_msg("");
                gdb_ok_msg(read_pos);
            }
        }
        std::cout << "[main] Read error\n";
    });
}
