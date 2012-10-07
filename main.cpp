//
// T-1000
// John Richards <jrichards@barracuda.com>
//

#include <cstdlib>
#include <iostream>

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

#include "xmlrpc_methods.hpp"

const int DEFAULT_PORT = 5150;

int
main(int argc, char* argv[])
{
    int port = DEFAULT_PORT;

    if (argc > 1) {
        port = atoi(argv[1]);
    }

    xmlrpc_c::registry registry;
    xmlrpc_c::methodPtr const ping_method(new PingMethod);
    xmlrpc_c::methodPtr const init_game_method(new InitGameMethod);
    xmlrpc_c::methodPtr const get_bid_method(new GetBidMethod);
    xmlrpc_c::methodPtr const make_choice_method(new MakeChoiceMethod);
    xmlrpc_c::methodPtr const move_result_method(new MoveResultMethod);
    xmlrpc_c::methodPtr const game_result_method(new GameResultMethod);

    registry.addMethod("ping", ping_method);
    registry.addMethod("init_game", init_game_method);
    registry.addMethod("get_bid", get_bid_method);
    registry.addMethod("make_choice", make_choice_method);
    registry.addMethod("move_result", move_result_method);
    registry.addMethod("game_result", game_result_method);

    xmlrpc_c::serverAbyss server(xmlrpc_c::serverAbyss::constrOpt().registryP(&registry).portNumber(port));

    std::cout << "T-1000: Starting on " << port << ". Kill all robots." << std::endl;
    server.run();

    return 0;
}
