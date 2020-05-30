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

/**
 * Contains definitions necessary for creating and checking languages against a DFA.
 */
namespace dfa
{
/**
 * Represents a DFA State.
 *
 * If the associated DFA WAS NOT created from an NFA, this structure is of cardinality one.
 * If the associated DFA WAS created from an NFA, this structure is of cardinality greater than or equal to one.
 */
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

/**
 * Custom hasher for a DFA State.
 */
struct StateHasher
{
  std::size_t operator()(const State& state) const;
};

/**
 * Represents an unordered set of States.
 */
using StateSet = std::unordered_set<State, StateHasher>;

/**
 * Represents an unordered map of States to the given class.
 */
template <typename T>
using StateMap = std::unordered_map<State, T, StateHasher>;

class Dfa
{
 public:
  /**
   * Represents an element of a language.
   *
   * The special string "epsilon" is recognized as the empty string.
   */
  using Symbol = std::string;

  /**
   * The set of Symbols that are recognized by the DFA.
   */
  using Alphabet = std::unordered_set<Symbol>;

  /**
   * Represents all transitions from a State to other States, with Symbols as the key.
   */
  using Transitions = std::unordered_map<Symbol, State>;

  using Language = std::string;

  using Json = nlohmann::json;

  /**
   * Defines whether a given language was accepted by the DFA.
   */
  enum Acceptance
  {
    /**
     * The language is accepted.
     */
    ACCEPTS,
    /**
     * The language was rejected.
     */
    REJECTS,
    /**
     * The language contained a Symbol that is not part of this DFA's Alphabet.
     */
    INVALID_ALPHABET,
    /**
     * The language was not accepted because there was no transition from one of the input Symbols to another State.
     */
    NO_TRANSITION
  };

  /**
   * Constructs a DFA from the input DFA file.
   *
   * If the input is an NFA, it will be converted to a DFA automatically.
   * @param dfa_file_contents DFA file contents as a string
   * @see https://github.com/aokellermann/dfa for file format
   */
  explicit Dfa(const std::string& dfa_file_contents);

  /**
   * Constructs a DFA from the input JSON file.
   *
   * If the input is an NFA, it will be converted to a DFA automatically.
   * @param dfa_file_contents JSON file contents
   * @see https://github.com/aokellermann/dfa for file format
   */
  explicit Dfa(const Json& dfa_file_contents);

  /**
   * Determines whether the input language is accepted by the DFA.
   * @param input the input Language
   * @param verbose prints debug information to stdout if true
   * @return Acceptance of input Language
   */
  Acceptance AcceptsString(const Language& input, bool verbose = false) const;

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