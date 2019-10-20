/*
 * Copyright 2014-present IVK JSC. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UPMQ_BROKER_SELECTORTOKEN_H
#define UPMQ_BROKER_SELECTORTOKEN_H

#include <iosfwd>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace upmq {
namespace broker {
namespace storage {

enum class TokenType {
  T_EOS,
  T_NULL,
  T_TRUE,
  T_FALSE,
  T_NOT,
  T_AND,
  T_OR,
  T_IN,
  T_IS,
  T_BETWEEN,
  T_LIKE,
  T_ESCAPE,
  T_IDENTIFIER,
  T_STRING,
  T_NUMERIC_EXACT,
  T_NUMERIC_APPROX,
  T_LPAREN,
  T_RPAREN,
  T_COMMA,
  T_PLUS,
  T_MINUS,
  T_MULT,
  T_DIV,
  T_EQUAL,
  T_NEQ,
  T_LESS,
  T_GRT,
  T_LSEQ,
  T_GREQ
};

struct Token {
  TokenType type = TokenType::T_EOS;
  std::string val;
  std::string::const_iterator tokenStart;

  Token() = default;

  Token(TokenType t, std::string v) : type(t), val(std::move(v)) {}

  Token(TokenType t, const std::string::const_iterator &s, std::string v) : type(t), val(std::move(v)), tokenStart(s) {}

  Token(TokenType t, const std::string::const_iterator &s, const char *v) : type(t), val(v), tokenStart(s) {}

  Token(TokenType t, const std::string::const_iterator &s, const std::string::const_iterator &e) : type(t), val(std::string(s, e)), tokenStart(s) {}

  bool operator==(const Token &r) const { return (type == TokenType::T_EOS && r.type == TokenType::T_EOS) || (type == r.type && val == r.val); }
};

std::ostream &operator<<(std::ostream &os, const Token &t);

class TokenException : public std::range_error {
 public:
  explicit TokenException(const std::string &);
};

bool tokenise(std::string::const_iterator &s, std::string::const_iterator &e, Token &tok);

class Tokeniser {
  std::vector<Token> tokens;
  size_t tokp;

  std::string::const_iterator inStart;
  std::string::const_iterator inp;
  std::string::const_iterator inEnd;

 public:
  Tokeniser(const std::string::const_iterator &s, const std::string::const_iterator &e);
  void returnTokens(size_t n = 1);
  const Token &nextToken();
  std::string remaining();
};
}  // namespace storage
}  // namespace broker
}  // namespace upmq

#endif
