#include "connection.h"
#include "log.h"
#include "req_handles.h"

auto recv(std::shared_ptr<Connection> conn) -> void {
    try {
        while (true) {
            auto pack_meta = Pack{};
            asio::read(conn->sock(), asio::buffer(&pack_meta, sizeof(pack_meta)));

            auto r_pack = create_pack_with_meta(pack_meta);
            if (r_pack == nullptr) {
                Log::error(std::format("recv from {}, pack invalid", conn->address()));
                continue;
            }

            asio::read(conn->sock(), asio::buffer(r_pack->data, r_pack->data_size));

            if (req_handles.contains(r_pack->api)) {
                if (auto s_pack = req_handles.at(r_pack->api)(conn, r_pack); s_pack != nullptr) {
                    Log::debug(std::format("send to {} api = {}, state = {}, data_size = {}", conn->address(),
                                           api_to_string(s_pack->api), s_pack->state, s_pack->data_size));
                    asio::write(conn->sock(), asio::const_buffer(s_pack.get(), sizeof(Pack) + s_pack->data_size));
                }
            }
        }
    } catch (asio::system_error &e) {
        Log::error(std::format("recv error from {}", conn->address(), e.what()));
        req_handles.at(Api::LOGOUT)(conn, nullptr);
        conn->sock().close();
    } catch (std::exception &e) {
        Log::error(std::format("error {}", conn->address(), e.what()));
    }
}

auto accept() -> asio::awaitable<void> {
    auto acceptor =
        asio::ip::tcp::acceptor{co_await asio::this_coro::executor, asio::ip::tcp::endpoint{asio::ip::tcp::v4(), 8080}};
    Log::info(std::format("server run in {}:{}", acceptor.local_endpoint().address().to_string(),
                          acceptor.local_endpoint().port()));
    while (true) {
        auto sock = co_await acceptor.async_accept(asio::use_awaitable);
        Log::info(std::format("new connect from {}:{}", sock.remote_endpoint().address().to_string(),
                              sock.remote_endpoint().port()));

        std::thread{[conn = std::make_shared<Connection>(std::move(sock))] { recv(conn); }}.detach();
    }
}

auto main() -> int {
    try {
        Log::init("server");
        Log::setLevel(Log::Level::DEBUG);
        auto context = asio::io_context{};
        asio::co_spawn(context, accept(), asio::detached);
        context.run();
    } catch (std::exception &e) {
        Log::error(e.what());
    }

    return 0;
}