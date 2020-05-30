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
struct StateID : std::unordered_set<std::string>
{
  using std::unordered_set<std::string>::unordered_set;

  inline explicit StateID(std::string id) { emplace(std::move(id)); }

  friend std::ostream& operator<<(std::ostream& os, const StateID& state);

  inline bool operator==(const StateID& other)
  {
    const auto* b = dynamic_cast<const std::unordered_set<std::string>*>(&other);
    return b != nullptr && *this == *b;
  }
};

struct StateIDHasher
{
  std::size_t operator()(const StateID& state) const;
};

using StateIDSet = std::unordered_set<StateID, StateIDHasher>;

template <typename T>
using StateIDMap = std::unordered_map<StateID, T, StateIDHasher>;

class Dfa
{
 public:
  using Symbol = std::string;
  using Alphabet = std::unordered_set<Symbol>;
  using Transitions = std::unordered_map<Symbol, StateID>;

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

  Acceptance AcceptsString(const std::string& input, bool verbose = false);

  constexpr const StateIDSet& GetStates() const noexcept { return states_; }

  constexpr const Alphabet& GetAlphabet() const noexcept { return alphabet_; }

  constexpr const StateIDMap<Transitions>& GetTransitions() const noexcept { return transitions_; }

  constexpr const StateID& GetStartState() const noexcept { return start_state_; }

  constexpr const StateIDSet& GetFinalStates() const noexcept { return final_states_; }

 private:
  void ExpandNfaIfNeeded();

  void AggregateEpsilonClosure(StateID& total_state, const StateID& current_state) const;

  void AggregateTransitions(StateIDMap<Transitions>& all_transitions, const StateID& current_state) const;

  /**
   * Q: all possible states.
   */
  StateIDSet states_;

  /**
   * Sigma: input symbols.
   */
  Alphabet alphabet_;

  /**
   * Delta: Q x Sigma -> Q.
   */
  StateIDMap<Transitions> transitions_;

  /**
   * q0: element of Q.
   */
  StateID start_state_;

  /**
   * F: subset of Q.
   */
  StateIDSet final_states_;
};

}  // namespace dfa