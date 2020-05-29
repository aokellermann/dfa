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
      states_.insert(std::make_move_iterator(tokens.begin()), std::make_move_iterator(tokens.end()));
    }
    else if (section_str == alphabet_str)
    {
      alphabet_.insert(std::make_move_iterator(tokens.begin()), std::make_move_iterator(tokens.end()));
    }
    else if (section_str == start_state_str)
    {
      if (!tokens.empty())
      {
        start_state_ = std::move(tokens[0]);
      }
    }
    else if (section_str == final_state_str)
    {
      final_states_.insert(std::make_move_iterator(tokens.begin()), std::make_move_iterator(tokens.end()));
    }
    else if (section_str == transition_str)
    {
      if (tokens.size() == 3)
      {
        transitions_[tokens[0]][tokens[1]] = tokens[2];
      }
    }
    else
    {
      throw std::runtime_error("Parsing error: invalid section");
    }
  }
}

Dfa::Dfa(const Dfa::Json& dfa_file_contents)
{
  try
  {
    for (const auto& element : dfa_file_contents.items())
    {
      if (element.key() == "states")
      {
        states_.insert(element.value().begin(), element.value().end());
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
          transitions_[tr["s1"]][tr["symbol"]] = tr["s2"];
        }
      }
      else if (element.key() == "start_state")
      {
        start_state_ = element.value();
      }
      else if (element.key() == "final_states")
      {
        final_states_.insert(element.value().begin(), element.value().end());
      }
    }
  }
  catch (const std::exception& e)
  {
    throw std::runtime_error(std::string("Failed to parse JSON: ") + e.what());
  }
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
}  // namespace dfa