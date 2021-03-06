//
// Copyright (c) 2013-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <beast/core.hpp>
#include <beast/websocket.hpp>
#include <beast/websocket/ssl.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

int main()
{
    // A helper for reporting errors
    auto const fail =
        [](std::string what, beast::error_code ec)
        {
            std::cerr << what << ": " << ec.message() << std::endl;
            std::cerr.flush();
            return EXIT_FAILURE;
        };

    boost::system::error_code ec;

    // Set up an asio socket to connect to a remote host
    boost::asio::io_service ios;
    boost::asio::ip::tcp::resolver r{ios};
    boost::asio::ip::tcp::socket sock{ios};

    // Look up the domain name
    std::string const host = "echo.websocket.org";
    auto const lookup = r.resolve({host, "https"}, ec);
    if(ec)
        return fail("resolve", ec);

    // Make the connection on the IP address we get from a lookup
    boost::asio::connect(sock, lookup, ec);
    if(ec)
        return fail("connect", ec);

    // Wrap the now-connected socket in an SSL stream
    using stream_type = boost::asio::ssl::stream<boost::asio::ip::tcp::socket&>;
    boost::asio::ssl::context ctx{boost::asio::ssl::context::sslv23};
    stream_type stream{sock, ctx};
    stream.set_verify_mode(boost::asio::ssl::verify_none);

    // Perform SSL handshaking
    stream.handshake(boost::asio::ssl::stream_base::client, ec);
    if(ec)
        return fail("ssl handshake", ec);

    // Now wrap the handshaked SSL stream in a websocket stream
    beast::websocket::stream<stream_type&> ws{stream};

    // Perform the websocket handshake
    ws.handshake(host, "/", ec);
    if(ec)
        return fail("handshake", ec);

    // Send a message
    ws.write(boost::asio::buffer("Hello, world!"), ec);
    if(ec)
        return fail("write", ec);

    // This buffer will hold the incoming message
    beast::multi_buffer b;

    // Read the message into our buffer
    ws.read(b, ec);
    if(ec)
        return fail("read", ec);

    // Send a "close" frame to the other end, this is a websocket thing
    ws.close(beast::websocket::close_code::normal, ec);
    if(ec)
        return fail("close", ec);

    // The buffers() function helps print a ConstBufferSequence
    std::cout << beast::buffers(b.data()) << std::endl;

    // WebSocket says that to close a connection you have
    // to keep reading messages until you receive a close frame.
    // Beast delivers the close frame as an error from read.
    //
    beast::drain_buffer drain; // Throws everything away efficiently
    for(;;)
    {
        // Keep reading messages...
        ws.read(drain, ec);

        // ...until we get the special error code
        if(ec == beast::websocket::error::closed)
            break;

        // Some other error occurred, report it and exit.
        if(ec)
            return fail("close", ec);
    }

    return EXIT_SUCCESS;
}
