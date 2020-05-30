/**
 * @file main.cc
 * @author Antony Kellermann
 * @copyright 2020 Antony Kellermann
 */

#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

#include "dfa/dfa.h"

namespace fs = std::filesystem;
using Json = nlohmann::json;

int main(int argc, char** argv)
{
  bool verbose = false;
  fs::path dfa_file_path;

  for (;;)
  {
    switch (getopt(argc, argv, "vd:h"))  // note the colon (:) to indicate that 'b' has a parameter and is not a switch
    {
      case 'v':
        verbose = true;
        continue;

      case 'd':
        dfa_file_path = optarg;
        continue;

      case 'h':
      default:
        std::cout << "-h\n\tprint usage\n-d <dfafile>\n\tDFA definition file\n-v\n\t verbose mode; display machine "
                     "definition, transitions, etc."
                  << std::endl;
        return 0;

      case -1:
        break;
    }
    break;
  }

  if (dfa_file_path.empty())
  {
    std::cout << "No DFA file path specified." << std::endl;
    return 1;
  }

  if (!fs::exists(dfa_file_path))
  {
    std::cout << "Specified DFA file path doesn't exist." << std::endl;
    return 1;
  }

  bool is_dfa_file = dfa_file_path.extension() == ".dfa";
  bool is_json_file = dfa_file_path.extension() == ".json";

  if (!is_dfa_file && !is_json_file)
  {
    std::cout << "Only .dfa and .json files are valid." << std::endl;
    return 1;
  }

  std::string input;
  try
  {
    std::ifstream fstream(dfa_file_path);
    std::stringstream sstr;
    sstr << fstream.rdbuf();

    input = sstr.str();
  }
  catch (std::exception& e)
  {
    std::cout << "Failed to read file: " << e.what() << std::endl;
    return 1;
  }

  if (input.empty())
  {
    std::cout << "Input file empty." << std::endl;
    return 1;
  }

  Json j_input;
  if (is_json_file)
  {
    try
    {
      j_input = Json::parse(input);
    }
    catch (std::exception& e)
    {
      std::cout << "Failed to parse JSON: " << e.what() << std::endl;
      return 1;
    }
  }

  std::unique_ptr<dfa::Dfa> dfa;
  try
  {
    dfa = std::make_unique<dfa::Dfa>(is_json_file ? dfa::Dfa(j_input) : dfa::Dfa(input));
  }
  catch (std::exception& e)
  {
    std::cout << "Failed to parse input file: " << e.what() << std::endl;
    return 1;
  }

  if (verbose)
  {
    std::cout << "---BEGIN DFA DEFINITION---" << std::endl;

    std::cout << "States:" << std::endl << '\t';
    for (const auto& state : dfa->GetStates())
    {
      std::cout << state << ' ';
    }

    std::cout << std::endl;

    std::cout << "Alphabet:" << std::endl << '\t';
    for (const auto& symbol : dfa->GetAlphabet())
    {
      std::cout << symbol << ' ';
    }

    std::cout << std::endl;

    std::cout << "Start State:" << std::endl << '\t';
    std::cout << dfa->GetStartState();

    std::cout << std::endl;

    std::cout << "Final States:" << std::endl << '\t';
    for (const auto& state : dfa->GetFinalStates())
    {
      std::cout << state << ' ';
    }

    std::cout << std::endl;

    std::cout << "Transitions:" << std::endl;
    for (const auto& [state, transitions] : dfa->GetTransitions())
    {
      std::cout << state << std::endl;
      for (const auto& [symbol, transition_state] : transitions)
      {
        std::cout << '\t' << symbol << " -> " << transition_state << std::endl;
      }
    }
  }

  std::string language;
  while (std::getline(std::cin, language) && !language.empty())
  {
    std::cout << language << " -> ";
    switch (dfa->AcceptsString(language, verbose))
    {
      case dfa::Dfa::ACCEPTS:
        std::cout << "ACCEPT";
        break;
      case dfa::Dfa::REJECTS:
        std::cout << "NOT ACCEPT";
        break;
      case dfa::Dfa::INVALID_ALPHABET:
        std::cout << "INVALID ALPHABET";
        break;
      case dfa::Dfa::NO_TRANSITION:
        std::cout << "NO TRANSITION";
        break;
      default:
        break;
    }

    std::cout << std::endl;
  }

  return 0;
}