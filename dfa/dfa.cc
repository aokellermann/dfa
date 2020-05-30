/**
 * @file dfa.cc
 * @author Antony Kellermann
 * @copyright 2020 Antony Kellermann
 */

#include "dfa.h"

#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace dfa
{
namespace
{
const Dfa::Symbol kEpsilon = "epsilon";
}  // namespace

std::ostream& operator<<(std::ostream& os, const StateID& state)
{
  if (state.size() == 1)
  {
    os << *state.begin();
  }
  else
  {
    os << '{';

    std::size_t i = 0;
    for (const auto& s : state)
    {
      os << s;
      if (i != state.size() - 1)
      {
        os << ", ";
      }
      else
      {
        os << '}';
      }

      ++i;
    }
  }
  return os;
}

std::size_t StateIDHasher::operator()(const StateID& state) const
{
  // I was getting wayyy too many collisions using boost::hash_combine, so this is a workaround.
  // This is probably really slow...

  std::vector<std::string> strings(state.begin(), state.end());
  std::sort(strings.begin(), strings.end());
  std::string str;
  for (auto& s : strings)
  {
    str.append(std::move(s));
  }

  return std::hash<std::string>()(str);
}

Dfa::Dfa(const std::string& dfa_file_contents)
{
  std::istringstream sstr(dfa_file_contents);

  const std::string states_str = "states: ";
  const std::string alphabet_str = "alphabet: ";
  const std::string start_state_str = "startstate: ";
  const std::string final_state_str = "finalstate: ";
  const std::string transition_str = "transition: ";
  for (std::string line; std::getline(sstr, line);)
  {
    const std::size_t first_space_idx = line.find(' ');
    if (first_space_idx == std::string::npos)
    {
      // Parsing failure
      throw std::runtime_error("Parsing error: could not find first space after colon");
    }

    const auto tokens_begin_idx = first_space_idx + 1;
    std::istringstream tokens_sstr(line.substr(tokens_begin_idx));

    const auto tokens_begin = std::istream_iterator<std::string>(tokens_sstr);
    const auto tokens_end = std::istream_iterator<std::string>();
    std::vector<std::string> tokens(tokens_begin, tokens_end);

    if (tokens.empty())
    {
      throw std::runtime_error("Parsing error: no tokens");
    }

    const auto section_str = line.substr(0, tokens_begin_idx);
    if (section_str == states_str)
    {
      for (const auto& token : tokens)
      {
        states_.insert({token});
      }
    }
    else if (section_str == alphabet_str)
    {
      alphabet_.insert(std::make_move_iterator(tokens.begin()), std::make_move_iterator(tokens.end()));
    }
    else if (section_str == start_state_str)
    {
      if (!tokens.empty())
      {
        start_state_.emplace(std::move(tokens[0]));
      }
    }
    else if (section_str == final_state_str)
    {
      for (const auto& token : tokens)
      {
        final_states_.insert({token});
      }
    }
    else if (section_str == transition_str)
    {
      if (tokens.size() == 3)
      {
        transitions_[StateID{tokens[0]}][tokens[1]].emplace(tokens[2]);
      }
    }
    else
    {
      throw std::runtime_error("Parsing error: invalid section");
    }
  }

  ExpandNfaIfNeeded();
}

Dfa::Dfa(const Dfa::Json& dfa_file_contents)
{
  try
  {
    for (const auto& element : dfa_file_contents.items())
    {
      if (element.key() == "states")
      {
        for (const auto& j : element.value())
        {
          states_.insert(StateID(j.get<std::string>()));
        }
      }
      else if (element.key() == "alphabet")
      {
        alphabet_.insert(element.value().begin(), element.value().end());
      }
      else if (element.key() == "transitions")
      {
        const auto& arr = element.value();
        for (const auto& tr : arr)
        {
          transitions_[StateID{tr["s1"]}][tr["symbol"]].emplace(tr["s2"]);
        }
      }
      else if (element.key() == "start_state")
      {
        start_state_.emplace(element.value());
      }
      else if (element.key() == "final_states")
      {
        for (const auto& j : element.value())
        {
          final_states_.insert(StateID(j.get<std::string>()));
        }
      }
    }
  }
  catch (const std::exception& e)
  {
    throw std::runtime_error(std::string("Failed to parse JSON: ") + e.what());
  }

  ExpandNfaIfNeeded();
}

Dfa::Acceptance Dfa::AcceptsString(const std::string& input, bool verbose)
{
  StateID current_state_id = start_state_;
  if (verbose)
  {
    std::cout << "Starting State: " << current_state_id << std::endl;
  }

  Symbol current_symbol;
  for (const auto& c : input)
  {
    current_symbol = c;
    if (alphabet_.find(current_symbol) == alphabet_.end())
    {
      return INVALID_ALPHABET;
    }

    const auto current_state_transitions = transitions_.find(current_state_id);
    if (current_state_transitions == transitions_.end())
    {
      return NO_TRANSITION;
    }

    const auto current_state_transition_for_symbol = current_state_transitions->second.find(current_symbol);
    if (current_state_transition_for_symbol == current_state_transitions->second.end())
    {
      return NO_TRANSITION;
    }

    if (verbose)
    {
      std::cout << "Current State: " << current_state_id << " Symbol: " << current_symbol
                << " -> New State: " << current_state_transition_for_symbol->second << std::endl;
    }

    current_state_id = current_state_transition_for_symbol->second;
  }

  return final_states_.find(current_state_id) != final_states_.end() ? ACCEPTS : REJECTS;
}

void Dfa::AggregateEpsilonClosure(StateID& total_state, const StateID& current_state) const
{
  const auto current_state_transitions = transitions_.find(current_state);
  if (current_state_transitions != transitions_.end())
  {
    const auto current_state_epsilon_transitions = current_state_transitions->second.find(kEpsilon);
    if (current_state_epsilon_transitions != current_state_transitions->second.end())
    {
      for (const auto& epsilon_transition_state : current_state_epsilon_transitions->second)
      {
        // If state has not already been added to total_states, insert it and recurse on that state.
        if (total_state.find(epsilon_transition_state) == total_state.end())
        {
          total_state.insert(epsilon_transition_state);
          AggregateEpsilonClosure(total_state, {epsilon_transition_state});
        }
      }
    }
  }
}

void Dfa::AggregateTransitions(StateIDMap<Transitions>& all_transitions, const StateID& current_state) const
{
  // Check if there is at least one transition for the current state.
  bool at_least_once_transition = false;
  for (const auto& st : current_state)
  {
    if (transitions_.find({st}) != transitions_.end())
    {
      at_least_once_transition = true;
      break;
    }
  }

  if (!at_least_once_transition)
  {
    return;
  }

  // Get epsilon closure before finding reachable states.
  StateID aggregated_current_state = current_state;
  for (const auto& state : current_state)
  {
    AggregateEpsilonClosure(aggregated_current_state, {state});
  }

  // If current_state hasn't been aggregated yet, do so.
  if (all_transitions.find(aggregated_current_state) == all_transitions.end())
  {
    // Get transitions for all reachable states.
    Transitions aggregated_current_state_transitions;
    for (const auto& reachable_state : aggregated_current_state)
    {
      auto reachable_state_transitions_pair = transitions_.find({reachable_state});
      if (reachable_state_transitions_pair != transitions_.end())
      {
        for (const auto& [symbol, reachable_state_transitions] : reachable_state_transitions_pair->second)
        {
          if (symbol != kEpsilon)
          {
            // Aggregate epsilon closure for each reachable transition.
            auto reachable_states_with_epsilon_closure = reachable_state_transitions;
            for (const auto& state : reachable_state_transitions)
            {
              AggregateEpsilonClosure(reachable_states_with_epsilon_closure, {state});
            }

            aggregated_current_state_transitions[symbol].insert(reachable_states_with_epsilon_closure.begin(),
                                                                reachable_states_with_epsilon_closure.end());
          }
        }
      }
    }

    // Add transitions for current state to all_transitions so they aren't duplicated when we recurse.
    all_transitions.emplace(aggregated_current_state, aggregated_current_state_transitions);

    // Get set of unique reachable states.
    StateIDSet reachable_states;
    for (const auto& [symbol, states] : aggregated_current_state_transitions)
    {
      reachable_states.emplace(states);
    }

    // Recurse on reachable states
    for (const auto& state : reachable_states)
    {
      AggregateTransitions(all_transitions, state);
    }
  }
}

void Dfa::ExpandNfaIfNeeded()
{
  bool is_nfa = false;

  for (const auto& [_, transitions] : transitions_)
  {
    for (const auto& [symbol, transition] : transitions)
    {
      if (symbol == kEpsilon || transition.size() > 1)
      {
        is_nfa = true;
        break;
      }
    }
    if (is_nfa)
    {
      break;
    }
  }

  if (!is_nfa)
  {
    return;
  }

  StateIDMap<Transitions> all_transitions;

  // First, find all states reachable by epsilon closure from the start state.
  StateID epsilon_reachable_state{start_state_};
  AggregateEpsilonClosure(epsilon_reachable_state, start_state_);

  // Recursively find all states reachable by reading input from the start state.
  AggregateTransitions(all_transitions, start_state_);
  transitions_ = std::move(all_transitions);

  // Update members.
  if (epsilon_reachable_state != start_state_)
  {
    start_state_ = std::move(epsilon_reachable_state);
  }

  states_.clear();
  for (const auto& transition : transitions_)
  {
    states_.insert(transition.first);
  }

  StateIDSet final_states;
  for (const auto& state : states_)
  {
    for (const auto& final_state : final_states_)
    {
      if (state.find(*final_state.begin()) != state.end())
      {
        final_states.insert(state);
      }
    }
  }

  final_states_ = std::move(final_states);
}
}  // namespace dfa