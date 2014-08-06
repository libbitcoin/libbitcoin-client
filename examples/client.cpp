#include <iostream>
#include <sstream>
#include <string>
#include "read_line.hpp"
#include <bitcoin/client.hpp>

static void on_unknown(const std::string& command)
{
    std::cout << "unknown message:" << command << std::endl;
}

static void on_update(const bc::payment_address& address,
    size_t height, const bc::hash_digest& blk_hash,
    const bc::transaction_type&)
{
    std::cout << "update:" << address.encoded() << std::endl;
}

/**
 * A dynamically-allocated structure holding the resources needed for a
 * connection to a bitcoin server.
 */
class connection
{
public:
    connection(zmq::context_t& context)
      : socket(context), codec(socket, on_update, on_unknown)
    {
    }

    libbitcoin::client::zeromq_socket socket;
    libbitcoin::client::obelisk_codec codec;
};

/**
 * Command-line interface for talking to the oblelisk server.
 */
class cli
{
public:
    ~cli();
    cli();

    int run();

private:
    void command();
    void cmd_exit();
    void cmd_help();
    void cmd_connect(std::stringstream& args);
    void cmd_disconnect(std::stringstream& args);
    void cmd_height(std::stringstream& args);
    void cmd_history(std::stringstream& args);

    bc::client::obelisk_codec::error_handler error_handler();
    void request_done();
    bool check_connection();
    bool read_string(std::stringstream& args, std::string& out,
        const std::string& error_message);
    bool read_address(std::stringstream& args, bc::payment_address& out);

    bool done_;
    bool pending_request_;

    zmq::context_t context_;
    read_line terminal_;
    connection *connection_;
};

cli::~cli()
{
    delete connection_;
}

cli::cli()
  : done_(false), pending_request_(false),
    terminal_(context_), connection_(nullptr)
{
}

/**
 * The main loop for the example application. This loop can be woken up
 * by either events from the network or by input from the terminal.
 */
int cli::run()
{
    std::cout << "type \"help\" for instructions" << std::endl;
    terminal_.show_prompt();

    while (!done_)
    {
        int delay = -1;
        std::vector<zmq_pollitem_t> items;
        items.reserve(2);
        items.push_back(terminal_.pollitem());
        if (connection_)
        {
            items.push_back(connection_->socket.pollitem());
            auto next_wakeup = connection_->codec.wakeup();
            if (next_wakeup.count())
                delay = next_wakeup.count();
        }
        zmq::poll(items.data(), items.size(), delay);

        if (items[0].revents)
            command();
        if (connection_ && items[1].revents)
            connection_->socket.forward(connection_->codec);
    }
    return 0;
}

/**
 * Reads a command from the terminal thread, and processes it appropriately.
 */
void cli::command()
{
    std::stringstream reader(terminal_.get_line());
    std::string command;
    reader >> command;

    if (command == "exit")              cmd_exit();
    else if (command == "help")         cmd_help();
    else if (command == "connect")      cmd_connect(reader);
    else if (command == "disconnect")   cmd_disconnect(reader);
    else if (command == "height")       cmd_height(reader);
    else if (command == "history")      cmd_history(reader);
    else
        std::cout << "unknown command " << command << std::endl;

    // Display another prompt, if needed:
    if (!done_ && !pending_request_)
        terminal_.show_prompt();
}

void cli::cmd_exit()
{
    done_ = true;
}

void cli::cmd_help()
{
    std::cout << "commands:" << std::endl;
    std::cout << "  exit              - leave the program" << std::endl;
    std::cout << "  help              - this menu" << std::endl;
    std::cout << "  connect <server>  - connect to a server" << std::endl;
    std::cout << "  disconnect        - disconnect from the server" << std::endl;
    std::cout << "  height            - get last block height" << std::endl;
    std::cout << "  history <address> - get an address' history" << std::endl;
}

void cli::cmd_connect(std::stringstream& args)
{
    std::string server;
    if (!read_string(args, server, "error: no server given"))
        return;
    std::cout << "connecting to " << server << std::endl;

    delete connection_;
    connection_ = new connection(context_);
    if (!connection_->socket.connect(server))
    {
        std::cout << "error: failed to connect" << std::endl;
        delete connection_;
        connection_ = nullptr;
    }
}

void cli::cmd_disconnect(std::stringstream& args)
{
    if (!check_connection())
        return;
    delete connection_;
    connection_ = nullptr;
}

void cli::cmd_height(std::stringstream& args)
{
    if (!check_connection())
        return;

    auto handler = [this](size_t height)
    {
        std::cout << "height: " << height << std::endl;
        request_done();
    };
    pending_request_ = true;
    connection_->codec.fetch_last_height(error_handler(), handler);
}

void cli::cmd_history(std::stringstream& args)
{
    if (!check_connection())
        return;
    bc::payment_address address;
    if (!read_address(args, address))
        return;

    auto handler = [this](const bc::blockchain::history_list& history)
    {
        for (auto row: history)
            std::cout << row.value << std::endl;
        request_done();
    };
    pending_request_ = true;
    connection_->codec.address_fetch_history(error_handler(),
        handler, address);
}

/**
 * Obtains a simple error-handling functor object.
 */
bc::client::obelisk_codec::error_handler cli::error_handler()
{
    return [this](const std::error_code& ec)
    {
        std::cout << "error: " << ec.message() << std::endl;
        request_done();
    };
}

/**
 * Call this to display a new prompt once a request has finished.
 */
void cli::request_done()
{
    pending_request_ = false;
    terminal_.show_prompt();
}

/**
 * Verifies that a connection exists, and prints an error message otherwise.
 */
bool cli::check_connection()
{
    if (!connection_)
    {
        std::cout << "error: no connection" << std::endl;
        return false;
    }
    return true;
}

/**
 * Parses a string argument out of the command line,
 * or prints an error message if there is none.
 */
bool cli::read_string(std::stringstream& args, std::string& out,
    const std::string& error_message)
{
    args >> out;
    if (!out.size())
    {
        std::cout << error_message << std::endl;
        return false;
    }
    return true;
}

/**
 * Reads a bitcoin address from the command-line, or prints an error if
 * the address is missing or invalid.
 */
bool cli::read_address(std::stringstream& args, bc::payment_address& out)
{
    std::string address;
    if (!read_string(args, address, "error: no address given"))
        return false;
    if (!out.set_encoded(address))
    {
        std::cout << "error: invalid address " << address << std::endl;
        return false;
    }
    return true;
}

int main()
{
    cli c;
    return c.run();
}
