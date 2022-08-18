#include "State.hpp"

#include "Context.hpp"

State::State(Context& context)
    : _context{context}
{
}

const Context&
State::context() const
{
    return _context;
}

Context&
State::context()
{
    return _context;
}