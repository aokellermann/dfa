/**
 * @file dfa_test.cc
 * @author Antony Kellermann
 * @copyright 2020 Antony Kellermann
 */

#include "dfa/dfa.h"

#include <gtest/gtest.h>

#include <memory>
#include <vector>

struct DfaTransition
{
  std::string s1;
  std::string symbol;
  std::string s2;
};

struct NfaTransition
{
  dfa::StateID s1;
  dfa::Dfa::Symbol symbol;
  dfa::StateID s2;
};

TEST(DFA, ParseDFA)
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
  EXPECT_NE(states.find({"q1"}), states.end());
  EXPECT_NE(states.find({"q2"}), states.end());
  EXPECT_NE(states.find({"q3"}), states.end());

  const auto& alphabet = dfa.GetAlphabet();
  EXPECT_NE(alphabet.find("0"), alphabet.end());
  EXPECT_NE(alphabet.find("1"), alphabet.end());

  const auto& start_state = dfa.GetStartState();
  const auto& final_states = dfa.GetFinalStates();
  EXPECT_EQ(start_state, dfa::StateID{"q1"});
  EXPECT_NE(final_states.find({"q2"}), final_states.end());

  const auto& transitions = dfa.GetTransitions();

  const std::vector<DfaTransition> expected_transitions = {
      {"q1", "0", "q1"}, {"q1", "1", "q2"}, {"q2", "0", "q3"}, {"q2", "1", "q2"}, {"q3", "0", "q2"}, {"q3", "1", "q2"},
  };

  for (const auto& transition : expected_transitions)
  {
    const auto iter_1 = transitions.find({transition.s1});
    EXPECT_NE(iter_1, transitions.end());

    if (iter_1 != transitions.end())
    {
      const auto iter_2 = iter_1->second.find(transition.symbol);
      EXPECT_NE(iter_2, iter_1->second.end());
      if (iter_2 != iter_1->second.end())
      {
        EXPECT_EQ(iter_2->second, dfa::StateID{transition.s2});
      }
    }
  }
}

TEST(DFA, ParseJSON)
{
  const std::string dfa_file_contents =
      "{\n"
      "  \"states\": [\n"
      "    \"q1\",\n"
      "    \"q2\",\n"
      "    \"q3\"\n"
      "  ],\n"
      "  \"alphabet\": [\n"
      "    \"0\",\n"
      "    \"1\"\n"
      "  ],\n"
      "  \"start_state\": \"q1\",\n"
      "  \"final_states\": [\n"
      "    \"q2\"\n"
      "  ],\n"
      "  \"transitions\": [\n"
      "    {\n"
      "      \"s1\": \"q1\",\n"
      "      \"symbol\": \"0\",\n"
      "      \"s2\": \"q1\"\n"
      "    },\n"
      "    {\n"
      "      \"s1\": \"q1\",\n"
      "      \"symbol\": \"1\",\n"
      "      \"s2\": \"q2\"\n"
      "    },\n"
      "    {\n"
      "      \"s1\": \"q2\",\n"
      "      \"symbol\": \"0\",\n"
      "      \"s2\": \"q3\"\n"
      "    },\n"
      "    {\n"
      "      \"s1\": \"q2\",\n"
      "      \"symbol\": \"1\",\n"
      "      \"s2\": \"q2\"\n"
      "    },\n"
      "    {\n"
      "      \"s1\": \"q3\",\n"
      "      \"symbol\": \"0\",\n"
      "      \"s2\": \"q2\"\n"
      "    },\n"
      "    {\n"
      "      \"s1\": \"q3\",\n"
      "      \"symbol\": \"1\",\n"
      "      \"s2\": \"q2\"\n"
      "    }\n"
      "  ]\n"
      "}";

  nlohmann::json json = nlohmann::json::parse(dfa_file_contents);
  dfa::Dfa dfa(json);

  const auto& states = dfa.GetStates();
  EXPECT_NE(states.find({"q1"}), states.end());
  EXPECT_NE(states.find({"q2"}), states.end());
  EXPECT_NE(states.find({"q3"}), states.end());

  const auto& alphabet = dfa.GetAlphabet();
  EXPECT_NE(alphabet.find({"0"}), alphabet.end());
  EXPECT_NE(alphabet.find("1"), alphabet.end());

  const auto& start_state = dfa.GetStartState();
  const auto& final_states = dfa.GetFinalStates();
  EXPECT_EQ(start_state, dfa::StateID{"q1"});
  EXPECT_NE(final_states.find({"q2"}), final_states.end());

  const auto& transitions = dfa.GetTransitions();

  const std::vector<DfaTransition> expected_transitions = {
      {"q1", "0", "q1"}, {"q1", "1", "q2"}, {"q2", "0", "q3"}, {"q2", "1", "q2"}, {"q3", "0", "q2"}, {"q3", "1", "q2"},
  };

  for (const auto& transition : expected_transitions)
  {
    const auto iter_1 = transitions.find({transition.s1});
    EXPECT_NE(iter_1, transitions.end());

    if (iter_1 != transitions.end())
    {
      const auto iter_2 = iter_1->second.find(transition.symbol);
      EXPECT_NE(iter_2, iter_1->second.end());
      if (iter_2 != iter_1->second.end())
      {
        EXPECT_EQ(iter_2->second, dfa::StateID{transition.s2});
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

TEST(NFA, ConvertToDFA)
{
  const std::string dfa_file_contents =
      "states: q0 q1 q2 q3\n"
      "alphabet: a b\n"
      "startstate: q0\n"
      "finalstate: q0\n"
      "transition: q0 epsilon q1\n"
      "transition: q1 a q1\n"
      "transition: q1 a q2\n"
      "transition: q1 b q2\n"
      "transition: q2 a q0\n"
      "transition: q2 a q2\n"
      "transition: q2 b q3\n"
      "transition: q3 b q1";

  dfa::Dfa dfa(dfa_file_contents);

  const auto& states = dfa.GetStates();
  EXPECT_NE(states.find({"q1"}), states.end());
  EXPECT_NE(states.find({"q2"}), states.end());
  EXPECT_NE(states.find({"q3"}), states.end());
  EXPECT_NE(states.find({"q0", "q1"}), states.end());
  EXPECT_NE(states.find({"q1", "q2"}), states.end());
  EXPECT_NE(states.find({"q1", "q3"}), states.end());
  EXPECT_NE(states.find({"q2", "q3"}), states.end());
  EXPECT_NE(states.find({"q0", "q1", "q2"}), states.end());

  const auto& alphabet = dfa.GetAlphabet();
  EXPECT_NE(alphabet.find("a"), alphabet.end());
  EXPECT_NE(alphabet.find("b"), alphabet.end());
  EXPECT_EQ(alphabet.size(), 2);

  const auto& start_state = dfa.GetStartState();
  const auto& final_states = dfa.GetFinalStates();
  EXPECT_EQ(start_state, dfa::StateID({"q0", "q1"}));
  EXPECT_NE(final_states.find(dfa::StateID{"q0", "q1"}), final_states.end());
  EXPECT_NE(final_states.find(dfa::StateID{"q0", "q1", "q2"}), final_states.end());
  EXPECT_EQ(final_states.size(), 2);

  const auto& transitions = dfa.GetTransitions();

  const std::vector<NfaTransition> expected_transitions = {
      {{"q1"}, "a", {"q1", "q2"}},
      {{"q1"}, "b", {"q2"}},

      {{"q2"}, "a", {"q0", "q1", "q2"}},
      {{"q2"}, "b", {"q3"}},

      {{"q3"}, "b", {"q1"}},

      {{"q0", "q1"}, "a", {"q1", "q2"}},
      {{"q0", "q1"}, "b", {"q2"}},

      {{"q1", "q2"}, "a", {"q0", "q1", "q2"}},
      {{"q1", "q2"}, "b", {"q2", "q3"}},

      {{"q1", "q3"}, "a", {"q1", "q2"}},
      {{"q1", "q3"}, "b", {"q1", "q2"}},

      {{"q2", "q3"}, "a", {"q0", "q1", "q2"}},
      {{"q2", "q3"}, "b", {"q1", "q3"}},

      {{"q0", "q1", "q2"}, "a", {"q0", "q1", "q2"}},
      {{"q0", "q1", "q2"}, "b", {"q2", "q3"}},
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

TEST(NFA, AcceptedInputs)
{
  const std::string dfa_file_contents =
      "states: q0 q1 q2 q3\n"
      "alphabet: a b\n"
      "startstate: q0\n"
      "finalstate: q0\n"
      "transition: q0 epsilon q1\n"
      "transition: q1 a q1\n"
      "transition: q1 a q2\n"
      "transition: q1 b q2\n"
      "transition: q2 a q0\n"
      "transition: q2 a q2\n"
      "transition: q2 b q3\n"
      "transition: q3 b q1";

  dfa::Dfa dfa(dfa_file_contents);

  EXPECT_EQ(dfa.AcceptsString("epsilon"), dfa::Dfa::Acceptance::ACCEPTS);
  EXPECT_EQ(dfa.AcceptsString("aba"), dfa::Dfa::Acceptance::ACCEPTS);
  EXPECT_EQ(dfa.AcceptsString("ba"), dfa::Dfa::Acceptance::ACCEPTS);
  EXPECT_EQ(dfa.AcceptsString("abbaba"), dfa::Dfa::Acceptance::ACCEPTS);
  EXPECT_EQ(dfa.AcceptsString("aa"), dfa::Dfa::Acceptance::ACCEPTS);
}

TEST(NFA, RejectedInputs)
{
  const std::string dfa_file_contents =
      "states: q0 q1 q2 q3\n"
      "alphabet: a b\n"
      "startstate: q0\n"
      "finalstate: q0\n"
      "transition: q0 epsilon q1\n"
      "transition: q1 a q1\n"
      "transition: q1 a q2\n"
      "transition: q1 b q2\n"
      "transition: q2 a q0\n"
      "transition: q2 a q2\n"
      "transition: q2 b q3\n"
      "transition: q3 b q1";

  dfa::Dfa dfa(dfa_file_contents);

  EXPECT_EQ(dfa.AcceptsString("a"), dfa::Dfa::Acceptance::REJECTS);
  EXPECT_EQ(dfa.AcceptsString("b"), dfa::Dfa::Acceptance::REJECTS);
  EXPECT_EQ(dfa.AcceptsString("abb"), dfa::Dfa::Acceptance::REJECTS);
  EXPECT_EQ(dfa.AcceptsString("babba"), dfa::Dfa::Acceptance::REJECTS);
}

TEST(NFA, InvalidAlphabet)
{
  const std::string dfa_file_contents =
      "states: q0 q1 q2 q3\n"
      "alphabet: a b\n"
      "startstate: q0\n"
      "finalstate: q0\n"
      "transition: q0 epsilon q1\n"
      "transition: q1 a q1\n"
      "transition: q1 a q2\n"
      "transition: q1 b q2\n"
      "transition: q2 a q0\n"
      "transition: q2 a q2\n"
      "transition: q2 b q3\n"
      "transition: q3 b q1";

  dfa::Dfa dfa(dfa_file_contents);

  EXPECT_EQ(dfa.AcceptsString("1ababb"), dfa::Dfa::Acceptance::INVALID_ALPHABET);
  EXPECT_EQ(dfa.AcceptsString("ababb2"), dfa::Dfa::Acceptance::INVALID_ALPHABET);
  EXPECT_EQ(dfa.AcceptsString("abb3bba"), dfa::Dfa::Acceptance::INVALID_ALPHABET);
  EXPECT_EQ(dfa.AcceptsString("abbacb"), dfa::Dfa::Acceptance::INVALID_ALPHABET);
  EXPECT_EQ(dfa.AcceptsString("a-bbab"), dfa::Dfa::Acceptance::INVALID_ALPHABET);
}

TEST(NFA, NoTransition)
{
  const std::string dfa_file_contents =
      "states: q0 q1 q2 q3\n"
      "alphabet: a b\n"
      "startstate: q0\n"
      "finalstate: q0\n"
      "transition: q0 epsilon q1\n"
      "transition: q1 a q1\n"
      "transition: q1 a q2\n"
      "transition: q1 b q2\n"
      "transition: q2 a q0\n"
      "transition: q2 a q2\n"
      "transition: q2 b q3\n"
      "transition: q3 b q1";

  dfa::Dfa dfa(dfa_file_contents);

  EXPECT_EQ(dfa.AcceptsString("bba"), dfa::Dfa::Acceptance::NO_TRANSITION);
  EXPECT_EQ(dfa.AcceptsString("bbab"), dfa::Dfa::Acceptance::NO_TRANSITION);
  EXPECT_EQ(dfa.AcceptsString("bbaa"), dfa::Dfa::Acceptance::NO_TRANSITION);
}

TEST(Hasher, NoCollisions)
{
  dfa::StateID s1{"q0", "q1", "q2"};
  dfa::StateID s2{"q0", "q2", "q1"};
  dfa::StateID s3{"q1", "q0", "q2"};
  dfa::StateID s4{"q1", "q2", "q0"};
  dfa::StateID s5{"q2", "q0", "q1"};
  dfa::StateID s6{"q2", "q1", "q0"};

  dfa::StateIDSet set{s1, s2, s3, s4, s5, s6};
  EXPECT_EQ(set.size(), 1);
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
