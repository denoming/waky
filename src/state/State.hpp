#pragma once

class StateContext;

class State {
public:
    State(StateContext& context);

    virtual ~State() = default;

    const StateContext&
    context() const;

    StateContext&
    context();

    virtual void
    enterState()
        = 0;

    virtual void
    run()
        = 0;

    virtual void
    exitState()
        = 0;

private:
    StateContext& _context;
};
