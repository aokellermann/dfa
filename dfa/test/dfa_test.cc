/**
 * @file dfa_test.cc
 * @author Antony Kellermann
 * @copyright 2020 Antony Kellermann
 */

#include "dfa/dfa.h"

#include <gtest/gtest.h>

#include <fstream>
#include <memory>
#include <string>
#include <vector>

struct Transition
{
  std::string s1;
  std::string symbol;
  std::string s2;
};

TEST(DFA, ParseFile)
{
  const std::string dfa_file_contents =
      "states: q1 q2 q3\n"
      "alphabet: 0 1\n"
      "startstate: q1\n"
      "finalstate: q2\n"
      "transition: q1 0 q1\n"
      "transition: q1 1 q2\n"
      "transition: q2 0 q3\n"
      "transition: q2 1 q2\n"
      "transition: q3 0 q2\n"
      "transition: q3 1 q2";

  dfa::Dfa dfa(dfa_file_contents);

  const auto& states = dfa.GetStates();
  EXPECT_NE(states.find("q1"), states.end());
  EXPECT_NE(states.find("q2"), states.end());
  EXPECT_NE(states.find("q3"), states.end());

  const auto& alphabet = dfa.GetAlphabet();
  EXPECT_NE(alphabet.find("0"), alphabet.end());
  EXPECT_NE(alphabet.find("1"), alphabet.end());

  const auto& start_state = dfa.GetStartState();
  const auto& final_states = dfa.GetFinalStates();
  EXPECT_EQ(start_state, "q1");
  EXPECT_NE(final_states.find("q2"), final_states.end());

  const auto& transitions = dfa.GetTransitions();

  const std::vector<Transition> expected_transitions = {
      {"q1", "0", "q1"}, {"q1", "1", "q2"}, {"q2", "0", "q3"}, {"q2", "1", "q2"}, {"q3", "0", "q2"}, {"q3", "1", "q2"},
  };

  for (const auto& transition : expected_transitions)
  {
    const auto iter_1 = transitions.find(transition.s1);
    EXPECT_NE(iter_1, transitions.end());

    if (iter_1 != transitions.end())
    {
      const auto iter_2 = iter_1->second.find(transition.symbol);
      EXPECT_NE(iter_2, iter_1->second.end());
      if (iter_2 != iter_1->second.end())
      {
        EXPECT_EQ(iter_2->second, transition.s2);
      }
    }
  }
}

TEST(DFA, AcceptedInputs)
{
  const std::string dfa_file_contents =
      "states: q1 q2 q3\n"
      "alphabet: 0 1\n"
      "startstate: q1\n"
      "finalstate: q2\n"
      "transition: q1 0 q1\n"
      "transition: q1 1 q2\n"
      "transition: q2 0 q3\n"
      "transition: q2 1 q2\n"
      "transition: q3 0 q2\n"
      "transition: q3 1 q2";

  dfa::Dfa dfa(dfa_file_contents);

  EXPECT_EQ(dfa.AcceptsString("11111"), dfa::Dfa::Acceptance::ACCEPTS);
  EXPECT_EQ(dfa.AcceptsString("00100"), dfa::Dfa::Acceptance::ACCEPTS);
  EXPECT_EQ(dfa.AcceptsString("11100"), dfa::Dfa::Acceptance::ACCEPTS);
  EXPECT_EQ(dfa.AcceptsString("110011"), dfa::Dfa::Acceptance::ACCEPTS);
  EXPECT_EQ(dfa.AcceptsString("001001"), dfa::Dfa::Acceptance::ACCEPTS);
  EXPECT_EQ(dfa.AcceptsString("0010001"), dfa::Dfa::Acceptance::ACCEPTS);
}

TEST(DFA, RejectedInputs)
{
  const std::string dfa_file_contents =
      "states: q1 q2 q3\n"
      "alphabet: 0 1\n"
      "startstate: q1\n"
      "finalstate: q2\n"
      "transition: q1 0 q1\n"
      "transition: q1 1 q2\n"
      "transition: q2 0 q3\n"
      "transition: q2 1 q2\n"
      "transition: q3 0 q2\n"
      "transition: q3 1 q2";

  dfa::Dfa dfa(dfa_file_contents);

  EXPECT_EQ(dfa.AcceptsString("00000"), dfa::Dfa::Acceptance::REJECTS);
  EXPECT_EQ(dfa.AcceptsString("01010"), dfa::Dfa::Acceptance::REJECTS);
  EXPECT_EQ(dfa.AcceptsString("001000"), dfa::Dfa::Acceptance::REJECTS);
}

TEST(DFA, InvalidAlphabet)
{
  const std::string dfa_file_contents =
      "states: q1 q2 q3\n"
      "alphabet: 0 1\n"
      "startstate: q1\n"
      "finalstate: q2\n"
      "transition: q1 0 q1\n"
      "transition: q1 1 q2\n"
      "transition: q2 0 q3\n"
      "transition: q2 1 q2\n"
      "transition: q3 0 q2\n"
      "transition: q3 1 q2";

  dfa::Dfa dfa(dfa_file_contents);

  EXPECT_EQ(dfa.AcceptsString("a11111"), dfa::Dfa::Acceptance::INVALID_ALPHABET);
  EXPECT_EQ(dfa.AcceptsString("00100b"), dfa::Dfa::Acceptance::INVALID_ALPHABET);
  EXPECT_EQ(dfa.AcceptsString("111c00"), dfa::Dfa::Acceptance::INVALID_ALPHABET);
  EXPECT_EQ(dfa.AcceptsString("111020"), dfa::Dfa::Acceptance::INVALID_ALPHABET);
  EXPECT_EQ(dfa.AcceptsString("1-11c00"), dfa::Dfa::Acceptance::INVALID_ALPHABET);
}

int main(int argc, char** argv)
{
  for (int i = 1; i < argc; ++i)
  {
    if (std::string(argv[i]) == "-h")
    {
      return 0;
    }
  }

  for (int i = 1; i < argc; ++i)
  {
    if (std::string(argv[i]) == "-d" && i + 1 < argc)
    {
      std::ifstream stream(argv[i + 1]);
      std::stringstream sstr;
      sstr << stream.rdbuf();

      dfa::Dfa dfa(sstr.str());
      // return 0;
    }
  }

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
