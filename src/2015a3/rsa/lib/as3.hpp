#pragma once

#include <winsock.h>
#include <string>
using std::string;
#include <iostream>
using std::cout; using std::endl; using std::cin; using std::ostream;
#include <vector>
using std::vector;


namespace as3
{
    template<typename Printable>
    auto println(Printable const& printable) -> ostream&{ return cout << printable << endl; }

    const int WSVERS = MAKEWORD(2, 0);

    class Socket
    {
    public:
        explicit Socket(SOCKET s)
            : socket_{ s }
        {}
        Socket(int address_family, int type, int protocol)
            : socket_{ ::socket(address_family, type, protocol) }
        {}

        auto get() const ->  SOCKET { return socket_; }
        auto is_failed() const -> bool { return socket_ < 0; }
        ~Socket(){ ::closesocket(socket_); }

        Socket(Socket const&) = delete;
        Socket(Socket &&) = delete;
        Socket& operator=(Socket const&) = delete;
        Socket& operator=(Socket &&) = delete;
    private:
        const SOCKET socket_;
    };

    auto inline setup_win_sock_api(WORD version_required) -> WSADATA
    {
        WSADATA wsadata;
        if (WSAStartup(WSVERS, &wsadata) != 0)
        {
            WSACleanup();
            println("WSAStartup failed");
            exit(1);
        }
        return wsadata;
    }

    class Receive
    {
    public:
        Receive() : is_normal_{ true }{}
        auto operator()(Socket const& sock) -> string
        {
            auto received = string();
            for (auto ch = char(0); true; /* */) //receive char by char, end on an LF, ignore CR's
            {
                if (0 >= recv(sock.get(), &ch, 1, 0)) { is_normal_ = false; break; }
                if (ch == '\n') break; else if (ch == '\r') continue; else received.push_back(ch);
            }
            return received;
        }
        auto is_normal() const -> bool { return is_normal_; }
    private:
        bool is_normal_;
    };

    auto inline send(SOCKET sock, string const& send_buffer) -> void
    {
        auto bytes_sent = ::send(sock, send_buffer.c_str(), send_buffer.size(), 0);
        if (bytes_sent < 0) { println("send failed"); exit(1); }
    }

    namespace rsa
    {
        template<typename Integer>
        auto repeat_square(Integer x, Integer e, Integer n) -> Integer //(x ^ e) % n.
        {
            auto y = Integer{ 1 };
            while (e > 0)
            {
                if ((e % 2) == 0){ x = (x*x) % n; e /= 2; }
                else{ y = (x*y) % n; --e; }
            }
            return y; 
        }

        struct TriKey
        {
            int const n, e, d;
        };

        struct BinKey
        {
            int const n, e_or_d;
            auto calculate(int m) const -> int
            { 
                return repeat_square(m, e_or_d, n); 
            }
            auto to_str() const -> string
            { 
                return{ char(n), char(e_or_d) }; 
            }
        };

        class RsaKeyList
        {
        public:
            template<typename Iterator>
            RsaKeyList(Iterator first, Iterator last)
                : list_(first, last), curr_{ list_.cbegin() }
            {}

            auto next() const -> void
            {
                if (++curr_ == list_.cend()) curr_ = list_.cbegin();
            };

            auto current_public_key() const -> BinKey
            {
                return{ curr_->n, curr_->e };
            }

            auto current_private_key() const -> BinKey
            {
                return{ curr_->n, curr_->d };
            }

            auto data() const -> vector<TriKey> const&
            {
                return list_;
            }

        private:
            vector<TriKey> const list_;
            mutable vector<TriKey>::const_iterator curr_;
        };

        auto read_char_as_int(char ch) -> int
        {
            return ch < 0 ? 256 + ch : ch;
        }
    }
}//namespace