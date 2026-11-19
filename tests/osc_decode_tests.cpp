#include <bbb/osc.hpp>
#include <iostream>

struct local_receiver : bbb::osc::receiver {
    bool check_datagrams() {
        std::array<char, bbb::udp::buf_size> buffer{};
        binded_port = 0;
        asio::ip::udp::endpoint peer(asio::ip::address_v4::loopback(), 1234);
        receive({}, buffer, 0, peer);
        bbb::osc::message parsed;
        if(queued_messages.receive(parsed)) return false;
        OSCPP::Client::Packet writer(buffer.data(), buffer.size());
        writer.openMessage("/level", 1).int32(12).closeMessage();
        receive({}, buffer, writer.size(), peer);
        return queued_messages.receive(parsed) && parsed.address == "/level" &&
            parsed.size() == 1 && parsed[0].num.i == 12 && parsed.received_port == 1234;
    }
};

int main() {
    using namespace bbb::osc;
    alignas(8) char storage[256]{};
    OSCPP::Client::Packet writer(storage, sizeof(storage));
    constexpr std::uint64_t time = 0xf123456789abcdefULL;
    writer.openBundle(1).openMessage("/mixed", 7)
        .boolean(true).NIL().impulse().character('c').symbol("front")
        .timetag(time).int32(42).closeMessage().closeBundle();
    OSCPP::Server::Packet packet(storage, writer.size());
    int delivered{0};
    bool correct{false};
    auto check = [&](const message &m) {
        ++delivered;
        correct = m.address == "/mixed" && m.size() == 7 &&
            m[0].tag == Tag::True && m[1].tag == Tag::NIL && m[2].tag == Tag::IMPULSE &&
            m[3].num.c == 'c' && m[4].str == "front" && m[5].num.ul == time && m[6].num.i == 42;
    };
    // Exercise both sides of the recursion limit using the same single, valid bundle.
    detail::read_packet(packet, check, detail::max_bundle_depth);
    if(delivered != 0) return 1;
    detail::read_packet(packet, check, detail::max_bundle_depth - 1);
    if(delivered != 1 || !correct) return 2;

    // Isolated argument streams: reject an unsupported tag instead of guessing its width.
    message unused;
    OSCPP::ReadStream empty(storage, 0);
    if(detail::read_arguments({OSCPP::ReadStream("?", 1), empty}, unused)) return 3;
    try {
        detail::read_arguments({OSCPP::ReadStream("i", 1), empty}, unused);
        return 4;
    } catch(const OSCPP::UnderrunError &) {}
    if(!local_receiver().check_datagrams()) return 5;
    std::cout << "OSC decode checks passed\n";
}
