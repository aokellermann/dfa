/**
 * @file dfa.h
 * @author Antony Kellermann
 * @copyright 2020 Antony Kellermann
 */

#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace dfa
{
struct State : std::unordered_set<std::string>
{
  using std::unordered_set<std::string>::unordered_set;

  inline explicit State(std::string id) { emplace(std::move(id)); }

  friend std::ostream& operator<<(std::ostream& os, const State& state);

  inline bool operator==(const State& other)
  {
    const auto* b = dynamic_cast<const std::unordered_set<std::string>*>(&other);
    return b != nullptr && *this == *b;
  }
};

struct StateHasher
{
  std::size_t operator()(const State& state) const;
};

using StateSet = std::unordered_set<State, StateHasher>;

template <typename T>
using StateMap = std::unordered_map<State, T, StateHasher>;

class Dfa
{
 public:
  using Symbol = std::string;
  using Alphabet = std::unordered_set<Symbol>;
  using Transitions = std::unordered_map<Symbol, State>;

  using Json = nlohmann::json;

  enum Acceptance
  {
    ACCEPTS,
    REJECTS,
    INVALID_ALPHABET,
    NO_TRANSITION
  };

  explicit Dfa(const std::string& dfa_file_contents);

  explicit Dfa(const Json& dfa_file_contents);

  Acceptance AcceptsString(const std::string& input, bool verbose = false) const;

  constexpr const StateSet& GetStates() const noexcept { return states_; }

  constexpr const Alphabet& GetAlphabet() const noexcept { return alphabet_; }

  constexpr const StateMap<Transitions>& GetTransitions() const noexcept { return transitions_; }

  constexpr const State& GetStartState() const noexcept { return start_state_; }

  constexpr const StateSet& GetFinalStates() const noexcept { return final_states_; }

 private:
  void ExpandNfaIfNeeded();

  void AggregateEpsilonClosure(State& total_state, const State& current_state) const;

  void AggregateTransitions(StateMap<Transitions>& all_transitions, const State& current_state) const;

  /**
   * Q: all possible states.
   */
  StateSet states_;

  /**
   * Sigma: input symbols.
   */
  Alphabet alphabet_;

  /**
   * Delta: Q x Sigma -> Q.
   */
  StateMap<Transitions> transitions_;

  /**
   * q0: element of Q.
   */
  State start_state_;

  /**
   * F: subset of Q.
   */
  StateSet final_states_;
};

}  // namespace dfa