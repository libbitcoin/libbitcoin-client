#include <iostream>
#include <sstream>
#include <string>
#include "read_line.hpp"

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

    bool done_;

    zmq::context_t context_;
    read_line terminal_;
};

cli::~cli()
{
}

cli::cli()
  : done_(false), terminal_(context_)
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
        auto pollitem = terminal_.pollitem();
        zmq::poll(&pollitem, 1, -1);
        command();
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
}

int main()
{
    cli c;
    return c.run();
}
