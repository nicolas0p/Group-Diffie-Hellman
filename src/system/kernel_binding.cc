// EPOS Kernel Binding

#include <framework/main.h>
#include <framework/agent.h>

// Framework class attributes
__BEGIN_SYS

IPC::Observed IPC::_observed;

Agent::Member Agent::_handlers[] = {&Agent::handle_thread,
                                    &Agent::handle_task,
                                    &Agent::handle_active,
                                    &Agent::handle_address_space,
                                    &Agent::handle_segment,
                                    &Agent::handle_mutex,
                                    &Agent::handle_semaphore,
                                    &Agent::handle_condition,
                                    &Agent::handle_clock,
                                    &Agent::handle_alarm,
                                    &Agent::handle_chronometer,
                                    &Agent::handle_ipc,
                                    &Agent::handle_utility
};

__END_SYS

__USING_SYS;
extern "C" { void _exec(void * m) { reinterpret_cast<Agent *>(m)->exec(); } }
