#include <iostream>
#include <sstream>
#include <string>
#include <zmq.hpp>
#include "read_line.hpp"

/**
 * A dynamically-allocated structure holding the resources needed for a
 * connection to a bitcoin server.
 */
class connection
{
public:
    connection(zmq::context_t& context, const std::string& server)
      : socket(context, ZMQ_DEALER)
    {
        // Do not wait for unsent messages when shutting down:
        int linger = 0;
        socket.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
        socket.connect(server.c_str());
    }

    zmq::socket_t socket;
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

    bool check_connection();
    bool read_string(std::stringstream& args, std::string& out,
        const std::string& error_message);

    bool done_;

    zmq::context_t context_;
    read_line terminal_;
    connection *connection_;
};

cli::~cli()
{
    delete connection_;
}

cli::cli()
  : done_(false), terminal_(context_), connection_(nullptr)
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
        std::vector<zmq_pollitem_t> items;
        items.reserve(2);
        items.push_back(terminal_.pollitem());
        if (connection_)
            items.push_back(zmq_pollitem_t
                {connection_->socket, 0, ZMQ_POLLIN});
        zmq::poll(items.data(), items.size(), -1);

        if (items[0].revents)
            command();
        if (connection_ && items[1].revents)
            ; // Server
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
    else
        std::cout << "unknown command " << command << std::endl;

    // Display another prompt, if needed:
    if (!done_)
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
}

void cli::cmd_connect(std::stringstream& args)
{
    std::string server;
    if (!read_string(args, server, "error: no server given"))
        return;
    std::cout << "connecting to " << server << std::endl;

    delete connection_;
    connection_ = nullptr;
    try
    {
        connection_ = new connection(context_, server);
    }
    catch (zmq::error_t)
    {
        std::cout << "error: failed to connect" << std::endl;
    }
}

void cli::cmd_disconnect(std::stringstream& args)
{
    if (!check_connection())
        return;
    delete connection_;
    connection_ = nullptr;
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

int main()
{
    cli c;
    return c.run();
}
