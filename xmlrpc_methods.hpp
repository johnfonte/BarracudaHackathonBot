//
// T-1000
// John Richards <jrichards@barracuda.com>
//

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

class PingMethod : public xmlrpc_c::method {
public:
    PingMethod();
    ~PingMethod();

    void execute(const xmlrpc_c::paramList& paramList, xmlrpc_c::value* const retval);
};

// ------------------------------------------------------------------------

class InitGameMethod : public xmlrpc_c::method {
public:
    InitGameMethod();
    ~InitGameMethod();

    void execute(const xmlrpc_c::paramList& paramList, xmlrpc_c::value* const retval);
};

// ------------------------------------------------------------------------

class GetBidMethod : public xmlrpc_c::method {
public:
    GetBidMethod();
    ~GetBidMethod();

    void execute(const xmlrpc_c::paramList& paramList, xmlrpc_c::value* const retval);
};

// ------------------------------------------------------------------------

class MakeChoiceMethod : public xmlrpc_c::method {
public:
    MakeChoiceMethod();
    ~MakeChoiceMethod();

    void execute(const xmlrpc_c::paramList& paramList, xmlrpc_c::value* const retval);
};

// ------------------------------------------------------------------------

class MoveResultMethod : public xmlrpc_c::method {
public:
    MoveResultMethod();
    ~MoveResultMethod();

    void execute(const xmlrpc_c::paramList& paramList, xmlrpc_c::value* const retval);
};

// ------------------------------------------------------------------------

class GameResultMethod : public xmlrpc_c::method {
public:
    GameResultMethod();
    ~GameResultMethod();

    void execute(const xmlrpc_c::paramList& paramList, xmlrpc_c::value* const retval);
};