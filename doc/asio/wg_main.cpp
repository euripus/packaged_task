// https://github.com/MikhailGorobets/HTTP
#define _WIN32_WINNT 0x0A00
#pragma warning(disable : 4996)

#include <iostream>
#include <istream>
#include <ostream>
#include <string>

#define OPENSSL_NO_SSL2
#include <boost/asio.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

auto const HOST           = "api.worldoftanks.ru";
auto const PORT           = "443";
auto const APPLICATION_ID = "5ca102cca5e97d839d92df4aa85c95e1";

using tcp      = boost::asio::ip::tcp;
namespace ssl  = boost::asio::ssl;
namespace http = boost::beast::http;

class HTTPSClient
{
public:
    using Stream = ssl::stream<boost::asio::ip::tcp::socket>;

public:
    HTTPSClient(std::string const & cert) :
        m_Resolver {m_IOService}, m_SSLContext {ssl::context::tlsv12_client}
    {
        m_SSLContext.load_verify_file(cert);
    }

    auto Connect(std::string const & host, std::string const & port) -> std::shared_ptr<Stream>
    {
        auto stream = std::make_shared<Stream>(m_IOService, m_SSLContext);
        SSL_set_tlsext_host_name(stream->native_handle(), host.c_str());
        boost::asio::connect(stream->next_layer(), m_Resolver.resolve(host, port));
        stream->handshake(ssl::stream_base::client);
        return stream;
    }

private:
    boost::asio::io_service m_IOService;
    tcp::resolver           m_Resolver;
    ssl::context            m_SSLContext;
};

namespace Wargaming
{
class Context;

class Command
{
public:
    friend class Context;
    using Param       = std::map<std::string, std::string>;
    using CommandName = std::string;
    using JSON        = boost::property_tree::ptree;

public:
    Command(CommandName const & cmdName) : m_Command {cmdName} {}
    auto AppendParam(std::string const & key, std::string const & value) -> Command &;
    auto Execute(Context & cmdContext) -> JSON;

private:
    CommandName m_Command;
    Param       m_Params;
};

class Context
{
public:
    friend class Command;
    using Token = std::string;
    using JSON  = boost::property_tree::ptree;

public:
    Context(Token const & id, HTTPSClient::Stream & stream) : m_ID {id}, m_Stream {stream} {}
    auto ExecuteCommand(Command & cmd) -> JSON;

private:
    Token                 m_ID;
    HTTPSClient::Stream & m_Stream;
};

auto Command::AppendParam(std::string const & key, std::string const & value) -> Command &
{
    m_Params.insert({key, value});
    return *this;
}

auto Command::Execute(Context & cmdContext) -> JSON
{
    std::stringstream ostreamRequest;
    ostreamRequest << m_Command << "/?";
    ostreamRequest << "application_id"
                   << "=" << cmdContext.m_ID << "&";
    for(auto const & [key, value] : m_Params)
        ostreamRequest << key << "=" << value << "&";

    http::request<http::string_body> request {http::verb::get, ostreamRequest.str(), 11};
    request.set(http::field::host, HOST);
    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    http::write(cmdContext.m_Stream, request);

    boost::beast::flat_buffer         buffer;
    http::response<http::string_body> response;
    http::read(cmdContext.m_Stream, buffer, response);

    std::stringstream           ostreamResponse(response.body());
    boost::property_tree::ptree propertyTree;
    boost::property_tree::read_json(ostreamResponse, propertyTree);

    if(propertyTree.get<std::string>("status") != "ok")
    {
        std::string error = "ERROR:\n";
        for(auto const & [key, value] : propertyTree.get_child("error"))
            error.append(key).append(": ").append(value.get<std::string>("")).append("\n");

        throw std::runtime_error(error);
    }
    return propertyTree;
}

auto Context::ExecuteCommand(Command & cmd) -> JSON
{
    return cmd.Execute(*this);
}

}   // namespace Wargaming

int main(int argc, char * argv[])
{
    try
    {
        HTTPSClient client {"client.pem"};
        auto        socket     = client.Connect(HOST, PORT);
        auto        cmdContext = Wargaming::Context {APPLICATION_ID, *socket};

        auto cmdList  = Wargaming::Command {"/wot/account/list"};
        auto cmdInfo  = Wargaming::Command {"/wot/account/info"};
        auto cmdTanks = Wargaming::Command {"/wot/account/tanks"};

        auto const accountID = cmdContext.ExecuteCommand(cmdList.AppendParam("search", "Straik"))
                                   .get_child("data")
                                   .front()
                                   .second.get<std::string>("account_id");
        auto const accountInfo = cmdContext.ExecuteCommand(cmdInfo.AppendParam("account_id", accountID))
                                     .get_child("data")
                                     .get_child(accountID);
        auto const accountTanks = cmdContext.ExecuteCommand(cmdTanks.AppendParam("account_id", accountID))
                                      .get_child("data")
                                      .get_child(accountID);

        std::stringstream ss;
        boost::property_tree::write_json(ss, accountInfo);
        boost::property_tree::write_json(ss, accountTanks);
        std::cout << ss.str() << std::endl;
    }
    catch(std::exception const & e)
    {
        std::cerr << e.what() << std::endl;
    }
}
