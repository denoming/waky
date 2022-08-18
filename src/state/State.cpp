#include "State.hpp"

#include "StateContext.hpp"

State::State(StateContext& context)
    : _context{context}
{
}

const StateContext&
State::context() const
{
    return _context;
}

StateContext&
State::context()
{
    return _context;
}