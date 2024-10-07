#pragma once

#include <any>
#include <asio.hpp>
#include <map>

class Connection {

  public:
    Connection(asio::ip::tcp::socket sock)
        : sock_{std::move(sock)}, sock_addr_{std::format("{}:{}", sock_.remote_endpoint().address().to_string(),
                                                         sock_.remote_endpoint().port())} {}
    ~Connection() = default;

  public:
    auto sock() -> asio::ip::tcp::socket & { return sock_; }

    auto address() -> const std::string & { return sock_addr_; }

    auto ext_data() -> std::map<std::string, std::any> & { return ext_datas_; }

  private:
    asio::ip::tcp::socket sock_;
    std::string sock_addr_;
    mutable std::map<std::string, std::any> ext_datas_; // 所有额外的数据
};