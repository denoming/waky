#pragma once

class StateContext;

class State {
public:
    explicit State(StateContext& context);

    virtual ~State() = default;

    [[nodiscard]] const StateContext&
    context() const;

    [[nodiscard]] StateContext&
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
