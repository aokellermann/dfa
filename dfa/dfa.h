/**
 * @file dfa.h
 * @author Antony Kellermann
 * @copyright 2020 Antony Kellermann
 */

#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace dfa
{
class Dfa
{
 public:
  using StateID = std::string;
  using Symbol = std::string;
  using Transitions = std::unordered_map<Symbol, StateID>;

  explicit Dfa(const std::string& dfa_file_contents);

  bool AcceptsString(const std::string& input);

  constexpr const std::unordered_set<StateID>& GetStates() const noexcept { return states_; }

  constexpr const std::unordered_set<Symbol>& GetAlphabet() const noexcept { return alphabet_; }

  constexpr const std::unordered_map<StateID, Transitions>& GetTransitions() const noexcept { return transitions_; }

  constexpr const StateID& GetStartState() const noexcept { return start_state_; }

  constexpr const std::unordered_set<StateID>& GetFinalStates() const noexcept { return final_states_; }

 private:
  /**
   * Q: all possible states.
   */
  std::unordered_set<StateID> states_;

  /**
   * Sigma: input symbols.
   */
  std::unordered_set<Symbol> alphabet_;

  /**
   * Delta: Q x Sigma -> Q.
   */
  std::unordered_map<StateID, Transitions> transitions_;

  /**
   * q0: element of Q.
   */
  StateID start_state_;

  /**
   * F: subset of Q.
   */
  std::unordered_set<StateID> final_states_;
};

}  // namespace dfa