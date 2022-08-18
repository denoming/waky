#pragma once

class Context;

class State {
public:
    State(Context& context);

    const Context&
    context() const;

    Context&
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
    Context& _context;
};
