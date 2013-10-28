#ifndef Corrade_Interconnect_StateMachine_h
#define Corrade_Interconnect_StateMachine_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
              Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include "Interconnect/Emitter.h"

namespace Corrade { namespace Interconnect {

/**
@brief Transition between states

See @ref StateMachine for more information.
*/
template<class State, class Input> class StateTransition {
    template<std::size_t, std::size_t, class, class> friend class StateMachine;

    public:
        /** @brief Constructor */
        constexpr /*implicit*/ StateTransition(State from, Input input, State to): from(from), input(input), to(to) {}

    private:
        State from;
        Input input;
        State to;
};

/**
@brief State machine

Exploits the Interconnect library for a simple state machine. Information about
state transitions is broadcasted through signals. The machine is meant to be
defined and connected at compile time.

@section Interconnect-StateMachine-usage Basic usage

Define two enums for states and inputs. The enums should have consecutive
values starting from `0`.
@code
enum class State: std::uint8_t {
    Ready,
    Printing,
    Finished
};

enum class Input: std::uint8_t {
    Operate,
    RemoveDocument
};
@endcode

Then `typedef` the state machine consisting of these two enums, count
of all states and count of all inputs:
@code
typdef Interconnect::StateMachine<3, 2, State, Input> Printer;
@endcode

Now add your state transitions, for each transition first original state, then
input, and then state after transition. Everything else is implicitly a no-op.
@code
Printer p;
p.addTransitions({
    {State::Ready,      Input::Operate,         State::Printing},
    {State::Printing,   Input::Operate,         State::Finished},
    {State::Finished,   Input::RemoveDocument,  State::Ready}
});
@endcode

Lastly connect transition signals to desired slots, so you can react to state
changes:
@code
Interconnect::connect(p, &Printer::entered<State::Ready>, []() { Utility::Debug() << "Printer is ready."; });
Interconnect::connect(p, &Printer::entered<State::Finished>, []() { Utility::Debug() << "Print finished. Please remove the document."; });
Interconnect::connect(p, &Printer::entered<State::Printing>, []() { Utility::Debug() << "Starting the print..."; });
Interconnect::connect(p, &Printer::exited<State::Printing>, []() { Utility::Debug() << "Finishing the print..."; });
@endcode

Stepping the machine will print the following output:
@code
p.step(Input::Operate);
p.step(Input::Operate);
p.step(Input::Remove);
@endcode

    Starting the print...
    Finishing the print...
    Print finished. Please remove the document.
    Printer is ready.

*/
template<std::size_t states, std::size_t inputs, class State, class Input> class StateMachine: public Emitter {
    public:
        enum: std::size_t {
            StateCount = states, /**< @brief Count of states in the machine */
            InputCount = inputs, /**< @brief Count of inputs for the machine */
        };

        /**
         * @brief Constructor
         *
         * All states are initially no-op (i.e., given state will not be
         * changed to anything else for any input).
         */
        explicit StateMachine();

        /**
         * @brief Current state
         *
         * Initial state is the one corresponding to `0` (i.e., usually the
         * first).
         */
        State current() const { return _current; }

        /**
         * @brief Add transitions to the list
         *
         * Expects that all states and inputs have
         */
        void addTransitions(std::initializer_list<StateTransition<State, Input>> transitions);

        /**
         * @brief Step the machine
         * @return Reference to self (for method chaining)
         *
         * Switches current state based on the @p input. If the new state is
         * different from previous one, emits @ref exited() with the old state
         * and then @ref entered() with the new one.
         */
        StateMachine<states, inputs, State, Input>& step(Input input);

        /**
         * @brief The machine entered given state
         *
         * Emitted when machine goes to given @p state from different one,
         * right after corresponding @ref exited() signal.
         * @see @ref step()
         */
        template<State state> Signal entered() {
            return emit(&StateMachine::entered<state>);
        }

        /**
         * @brief The machine exited given state
         *
         * Emitted when machine leaves given @p state for different one. The
         * corresponding @ref entered() signal is emitted after this one.
         * @see @ref step()
         */
        template<State state> Signal exited() {
            return emit(&StateMachine::exited<state>);
        }

    private:
        State& at(State current, Input input) {
            return _transitions[std::size_t(current)*inputs+std::size_t(input)];
        }

        const State& at(State current, Input input) const {
            return _transitions[std::size_t(current)*inputs+std::size_t(input)];
        }

        template<std::size_t current> Signal enteredInternal(State wanted, std::integral_constant<std::size_t, current>) {
            return State(current-1) == wanted ? entered<State(current-1)>() : enteredInternal(wanted, std::integral_constant<std::size_t, current-1>{});
        }

        Signal enteredInternal(State, std::integral_constant<std::size_t, 0>) {
            CORRADE_ASSERT_UNREACHABLE();
        }

        template<std::size_t current> Signal exitedInternal(State wanted, std::integral_constant<std::size_t, current>) {
            return State(current-1) == wanted ? exited<State(current-1)>() : exitedInternal(wanted, std::integral_constant<std::size_t, current - 1>{});
        }

        Signal exitedInternal(State, std::integral_constant<std::size_t, 0>) {
            CORRADE_ASSERT_UNREACHABLE();
        }

        State _transitions[states*inputs];
        State _current;
};

template<std::size_t states, std::size_t inputs, class State, class Input> StateMachine<states, inputs, State, Input>::StateMachine(): _transitions{}, _current{} {
    /* Make input in all states a no-op */
    for(std::size_t i = 0; i != states; ++i)
        for(std::size_t j = 0; j != inputs; ++j)
            at(State(i), Input(j)) = State(i);
}

template<std::size_t states, std::size_t inputs, class State, class Input> void StateMachine<states, inputs, State, Input>::addTransitions(const std::initializer_list<StateTransition<State, Input>> transitions) {
    for(const auto transition: transitions) {
        CORRADE_ASSERT(std::size_t(transition.from) < states && std::size_t(transition.input) < inputs && std::size_t(transition.to) < states, "Interconnect::StateMachine: out-of-bounds state, from:" << std::size_t(transition.from) << "input:" << std::size_t(transition.input) << "to:" << std::size_t(transition.to), );
        at(transition.from, transition.input) = transition.to;
    }
}

template<std::size_t states, std::size_t inputs, class State, class Input> StateMachine<states, inputs, State, Input>& StateMachine<states, inputs, State, Input>::step(Input input) {
    const State next = at(_current, input);

    if(next != _current) {
        exitedInternal(_current, std::integral_constant<std::size_t, states>{});
        enteredInternal(_current = next, std::integral_constant<std::size_t, states>{});
    }

    return *this;
}

}}

#endif