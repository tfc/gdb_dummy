#include <array>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>

#include <attoparsecpp/parser.hpp>

#include "socket_buffer.hpp"
#include "socket.hpp"

// TODO: Encapsulate all this into a class/struct

static int global_fd {0};

static std::vector<uint8_t> mem (0x1000, 0);

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
    //std::cout << "-> " << s << '\n';
    send(global_fd, s.data(), s.size(), 0);
}

static void send_msg(const std::string &s) { send_msg_raw(gdb_chksum_msg(s)); }

static void send_ack(bool ack_for_valid_packet = true) {
    send_msg_raw(ack_for_valid_packet ? "+" : "-");
}

namespace {
using namespace apl;

namespace foo {
template <typename P1, typename P2>
static auto operator|(P1 p1, P2 p2) { return choice(p1, p2); }

template <typename P1, typename P2>
static auto operator>>(P1 p1, P2 p2) { return prefixed(p1, p2); }

template <typename P1, typename P2>
static auto operator<<(P1 p1, P2 p2) { return postfixed(p2, p1); }

static auto operator"" _charP (char c) { return oneOf(c); }
}

static parser<std::string> checksum_parser(buffer_pos &pos) {
  using namespace foo;
  if (const auto payload_str {('$'_charP >> many(noneOf('#')) << '#'_charP)(pos)}) {
      const auto ref_chksum {base_integer<uint8_t>(16, 2)(pos)};
      if (ref_chksum == gdb_checksum(*payload_str)) { return payload_str; }
  }
  return {};
}

static parser<bool> gdb_ok_msg(buffer_pos &pos) {
  using namespace foo;
  return map('+'_charP, [] (auto) {
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
  using namespace foo;
  return ('q'_charP >>
             ('T'_charP >> (
               ignore_symbols("fV") | simple_answer_on("Status", "T0")
             ))
           | simple_answer_on("Supported", "PacketSize=2000")
           | simple_answer_on("Attached", "1")
         )(pos);
}

static parser<bool> get_register(buffer_pos &pos) {
  using namespace foo;
  return map('p'_charP >> integer,
      [](const auto &x) {
        std::cout << "GET REGISTER " << x << '\n';
        send_msg("08ab0000");
        return true;
      })(pos);
}

static std::optional<std::string> read_memory_at(size_t offset, size_t bytes) {
  if (offset + bytes < mem.size()) {
    std::ostringstream ss;
    for (size_t i{offset}; i < offset + bytes; ++i) {
      ss << std::hex << std::setfill('0') << std::setw(2) << unsigned(mem[i]);
    }
    return ss.str();
  }
  return {};
}

static parser<bool> read_memory(buffer_pos &pos) {
  using namespace foo;
  return map(tuple_of('m'_charP >> base_integer(16),
                      ','_charP >> base_integer(16)),
      [] (const auto &x) {
        const auto [offset, bytes] = x;
        if (const auto retmem {read_memory_at(offset, bytes)}) {
          send_msg(*retmem);
        } else {
          send_msg("E00");
        }
        return true;
      })(pos);
}

static uint8_t fix_hex(uint8_t d) {
  // TODO: rename `hexnumber` parser to `hex_digit`
  // and then reimplement `hexnumber` to use `hex_digit` and perform
  // the char to unsigned conversion.
  if ('a' <= d && d <= 'f') { return d - 'a' + 10; }
  return d - '0';
}

static parser<uint8_t> byte(buffer_pos &pos) {
  return map(tuple_of(hexnumber, hexnumber),
    [] (const auto &t) -> uint8_t {
      const auto [hi, lo] = t;
      return (fix_hex(hi) << 4) | fix_hex(lo);
    })(pos);
}

static parser<bool> write_memory(buffer_pos &pos) {
  // TODO: A more "monadic version" that works like:
  // do
  //     address   <- prefixed('M'_charP, base_integer(16))
  //     num_bytes <- prefixed(','_charP, base_integer(16))
  //     content   <- prefixed(':'_charP, manyV(byte, num_bytes))
  //     return (address, num_bytes, content)
  //
  // would be awesome here because then we would have only a single allocation
  // for the write's actual content instead of a growing vector.
  using namespace foo;
  return map(tuple_of('M'_charP >> base_integer(16),
                      ','_charP >> base_integer(16),
                      ':'_charP >> manyV(byte)),
      [] (const auto &x) {
        const auto &[offset, bytes, content_bytes] = x;

        if (offset + bytes < mem.size()) {
          std::copy(std::begin(content_bytes), std::end(content_bytes),
                    std::begin(mem) + offset);
          send_msg("OK");
        } else {
          send_msg("E00");
        }

        return true;
      })(pos);
}

static parser<bool> parse_gdb_message(buffer_pos &pos) {
  using namespace foo;
  return (simple_answer_on("?", "S05")
        | simple_answer_on("!", "OK")
        | simple_answer_on("g", "xxxxxxxx00000000xxxxxxxx00000000")
        | get_register
        | read_memory
        | write_memory
        | q_messages
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
            //std::cout << "<- [" << *chk_ret << "]\n";
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
