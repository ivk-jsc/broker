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

#include "SelectorExpression.h"

#include "Selector.h"
#include "SelectorRegex.h"
#include "SelectorToken.h"
#include "SelectorValue.h"

#include <cerrno>
#include <cstdlib>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
/*
 * Syntax for JMS style selector expressions (informal):
 * This is a mixture of regular expression and EBNF formalism
 *
 * The top level term is SelectExpression
 *
 * // Lexical elements
 *
 * Alpha ::= [a-zA-Z]
 * Digit ::= [0-9]
 * HexDigit ::= [0-9a-fA-F]
 * OctDigit ::= [0-7]
 * BinDigit ::= [0-1]
 *
 * IdentifierInitial ::= Alpha | "_" | "$"
 * IdentifierPart ::= IdentifierInitial | Digit | "."
 * Identifier ::= IdentifierInitial IdentifierPart*
 * Constraint : Identifier NOT IN ("NULL", "TRUE", "FALSE", "NOT", "AND", "OR",
 * "BETWEEN", "LIKE", "IN", "IS") // Case
 * insensitive
 *
 * LiteralString ::= ("'" [^']* "'")+ // Repeats to cope with embedded single
 * quote
 *
 * // LiteralExactNumeric is a little simplified as it also allows underscores
 * ("_") as internal seperators and suffix
 * "l" or "L"
 * LiteralExactNumeric ::= "0x" HexDigit+ | "0X" HexDigit+ | "0b" BinDigit+ |
 * "0B" BinDigit+ | "0" OctDigit* | Digit+
 *
 * // LiteralApproxNumeric is a little simplified as it also allows suffix "d",
 * "D", "f", "F"
 * Exponent ::= ('+'|'-')? LiteralExactNumeric
 * LiteralApproxNumeric ::= ( Digit "." Digit* ( "E" Exponent )? ) |
 *                          ( "." Digit+ ( "E" Exponent )? ) |
 *                          ( Digit+ "E" Exponent )
 * LiteralBool ::= "TRUE" | "FALSE"
 *
 * Literal ::= LiteralBool | LiteralString | LiteralApproxNumeric |
 * LiteralExactNumeric
 *
 * EqOps ::= "=" | "<>"
 * ComparisonOps ::= EqOps | ">" | ">=" | "<" | "<="
 * AddOps ::= "+" | "-"
 * MultiplyOps ::= "*" | "/"
 *
 * // Expression Syntax
 *
 * SelectExpression ::= OrExpression? // An empty expression is equivalent to
 * "true"
 *
 * OrExpression ::= AndExpression  ( "OR" AndExpression )*
 *
 * AndExpression :: = ComparisonExpression ( "AND" ComparisonExpression )*
 *
 * ComparisonExpression ::= AddExpression "IS" "NOT"? "NULL" |
 *                          AddExpression "NOT"? "LIKE" LiteralString [ "ESCAPE"
 * LiteralString ] |
 *                          AddExpression "NOT"? "BETWEEN" AddExpression "AND"
 * AddExpression |
 *                          AddExpression "NOT"? "IN" "(" PrimaryExpression (","
 * PrimaryExpression)* ")" |
 *                          AddExpression ComparisonOps AddExpression |
 *                          "NOT" ComparisonExpression |
 *                          AddExpression
 *
 * AddExpression :: = MultiplyExpression (  AddOps MultiplyExpression )*
 *
 * MultiplyExpression :: = UnaryArithExpression ( MultiplyOps
 * UnaryArithExpression )*
 *
 * UnaryArithExpression ::= "-" LiteralExactNumeric |  // This is a special case
 * to simplify negative ints
 *                          AddOps AddExpression |
 *                          "(" OrExpression ")" |
 *                          PrimaryExpression
 *
 * PrimaryExpression :: = Identifier |
 *                        Literal
 */

#include <cstdlib>
#include <fake_cpp14.h>

using std::ostream;
using std::string;

namespace upmq {
namespace broker {
namespace storage {

class Expression {
 public:
  virtual ~Expression() noexcept {}
  virtual void repr(std::ostream &) const = 0;
  virtual Value eval(const SelectorEnv &) const = 0;

  virtual BoolOrNone eval_bool(const SelectorEnv &env) const {
    Value v = eval(env);
    if (v.type == Value::T_BOOL) {
      return BoolOrNone(v.b);
    } else {
      return BN_UNKNOWN;
    }
  }
};

class BoolExpression : public Expression {
 public:
  ~BoolExpression() noexcept override {}
  void repr(std::ostream &) const override = 0;
  BoolOrNone eval_bool(const SelectorEnv &) const override = 0;

  Value eval(const SelectorEnv &env) const override { return Value(eval_bool(env)); }
};

// Operators

class ComparisonOperator {
 public:
  virtual ~ComparisonOperator() = default;
  virtual void repr(ostream &) const = 0;
  virtual BoolOrNone eval(Expression &, Expression &, const SelectorEnv &) const = 0;
};

class UnaryBooleanOperator {
 public:
  virtual ~UnaryBooleanOperator() = default;
  virtual void repr(ostream &) const = 0;
  virtual BoolOrNone eval(Expression &, const SelectorEnv &) const = 0;
};

class ArithmeticOperator {
 public:
  virtual ~ArithmeticOperator() = default;
  virtual void repr(ostream &) const = 0;
  virtual Value eval(Expression &, Expression &, const SelectorEnv &) const = 0;
};

class UnaryArithmeticOperator {
 public:
  virtual ~UnaryArithmeticOperator() = default;
  virtual void repr(ostream &) const = 0;
  virtual Value eval(Expression &, const SelectorEnv &) const = 0;
};

////////////////////////////////////////////////////

// Convenience outputters

ostream &operator<<(ostream &os, const Expression &e) {
  e.repr(os);
  return os;
}

ostream &operator<<(ostream &os, const ComparisonOperator &e) {
  e.repr(os);
  return os;
}

ostream &operator<<(ostream &os, const UnaryBooleanOperator &e) {
  e.repr(os);
  return os;
}

ostream &operator<<(ostream &os, const ArithmeticOperator &e) {
  e.repr(os);
  return os;
}

ostream &operator<<(ostream &os, const UnaryArithmeticOperator &e) {
  e.repr(os);
  return os;
}

// Boolean Expression types...

class ComparisonExpression : public BoolExpression {
  ComparisonOperator *op;
  std::unique_ptr<Expression> e1;
  std::unique_ptr<Expression> e2;

 public:
  ComparisonExpression(ComparisonOperator *o, Expression *e, Expression *e_) : op(o), e1(e), e2(e_) {}

  void repr(ostream &os) const override { os << "(" << *e1 << *op << *e2 << ")"; }

  BoolOrNone eval_bool(const SelectorEnv &env) const override { return op->eval(*e1, *e2, env); }
};

class OrExpression : public BoolExpression {
  std::unique_ptr<Expression> e1;
  std::unique_ptr<Expression> e2;

 public:
  OrExpression(Expression *e, Expression *e_) : e1(e), e2(e_) {}

  void repr(ostream &os) const override { os << "(" << *e1 << " OR " << *e2 << ")"; }

  BoolOrNone eval_bool(const SelectorEnv &env) const override {
    BoolOrNone bn1(e1->eval_bool(env));
    if (bn1 == BN_TRUE) {
      return BN_TRUE;
    }
    BoolOrNone bn2(e2->eval_bool(env));
    if (bn2 == BN_TRUE) {
      return BN_TRUE;
    }
    if (bn1 == BN_FALSE && bn2 == BN_FALSE) {
      return BN_FALSE;
    } else {
      return BN_UNKNOWN;
    }
  }
};

class AndExpression : public BoolExpression {
  std::unique_ptr<Expression> e1;
  std::unique_ptr<Expression> e2;

 public:
  AndExpression(Expression *e, Expression *e_) : e1(e), e2(e_) {}

  void repr(ostream &os) const override { os << "(" << *e1 << " AND " << *e2 << ")"; }

  BoolOrNone eval_bool(const SelectorEnv &env) const override {
    BoolOrNone bn1(e1->eval_bool(env));
    if (bn1 == BN_FALSE) {
      return BN_FALSE;
    }
    BoolOrNone bn2(e2->eval_bool(env));
    if (bn2 == BN_FALSE) {
      return BN_FALSE;
    }
    if (bn1 == BN_TRUE && bn2 == BN_TRUE) {
      return BN_TRUE;
    } else {
      return BN_UNKNOWN;
    }
  }
};

class UnaryBooleanExpression : public BoolExpression {
  UnaryBooleanOperator *op;
  std::unique_ptr<Expression> e1;

 public:
  UnaryBooleanExpression(UnaryBooleanOperator *o, Expression *e) : op(o), e1(e) {}

  void repr(ostream &os) const override { os << *op << "(" << *e1 << ")"; }

  BoolOrNone eval_bool(const SelectorEnv &env) const override { return op->eval(*e1, env); }
};

class LikeExpression : public BoolExpression {
  std::unique_ptr<Expression> e;
  string reString;
  upmq::broker::SelectorRegex regexBuffer;

  static string toRegex(const string &s, const string &escape) {
    string regex("^");
    if (escape.size() > 1) {
      throw std::logic_error("Internal error");
    }
    char e = 0;
    if (escape.size() == 1) {
      e = escape[0];
    }
    // Translate % -> .*, _ -> ., . ->\. *->\*
    bool doEscape = false;
    for (const char &i : s) {
      if (e != 0 && i == e) {
        doEscape = true;
        continue;
      }
      switch (i) {
        case '%':
          if (doEscape) {
            regex += i;
          } else {
            regex += ".*";
          }
          break;
        case '_':
          if (doEscape) {
            regex += i;
          } else {
            regex += ".";
          }
          break;
        case ']':
          regex += "[]]";
          break;
        case '-':
          regex += "[-]";
          break;
        // Don't add any more cases here: these are sufficient,
        // adding more might turn on inadvertent matching
        case '\\':
        case '^':
        case '$':
        case '.':
        case '*':
        case '[':
          regex += "\\";
        // Fallthrough
        default:
          regex += i;
          break;
      }
      doEscape = false;
    }
    regex += "$";
    return regex;
  }

 public:
  LikeExpression(Expression *e_, const string &like, const string &escape = "") : e(e_), reString(toRegex(like, escape)), regexBuffer(reString) {}
  ~LikeExpression() noexcept override {}

  void repr(ostream &os) const override { os << *e << " REGEX_MATCH '" << reString << "'"; }

  BoolOrNone eval_bool(const SelectorEnv &env) const override {
    Value v(e->eval(env));
    if (v.type != Value::T_STRING) {
      return BN_UNKNOWN;
    }
    return BoolOrNone(upmq::broker::regex_match(*v.s, regexBuffer));
  }
};

class BetweenExpression : public BoolExpression {
  std::unique_ptr<Expression> e;
  std::unique_ptr<Expression> l;
  std::unique_ptr<Expression> u;

 public:
  BetweenExpression(Expression *e_, Expression *l_, Expression *u_) : e(e_), l(l_), u(u_) {}

  void repr(ostream &os) const override { os << *e << " BETWEEN " << *l << " AND " << *u; }

  BoolOrNone eval_bool(const SelectorEnv &env) const override {
    Value ve(e->eval(env));
    Value vl(l->eval(env));
    Value vu(u->eval(env));
    if (unknown(ve) || unknown(vl) || unknown(vu)) {
      return BN_UNKNOWN;
    }
    return BoolOrNone(ve >= vl && ve <= vu);
  }
};

class InExpression : public BoolExpression {
  std::unique_ptr<Expression> e;
  std::vector<std::shared_ptr<Expression>> l;

 public:
  InExpression(Expression *e_, std::vector<std::shared_ptr<Expression>> &l_) : e(e_) { l.swap(l_); }

  void repr(ostream &os) const override {
    os << *e << " IN (";
    for (std::size_t i = 0; i < l.size(); ++i) {
      os << l[i] << ((i < (l.size() - 1)) ? ", " : ")");  //-V658
    }
  }

  BoolOrNone eval_bool(const SelectorEnv &env) const override {
    Value ve(e->eval(env));
    if (unknown(ve)) {
      return BN_UNKNOWN;
    }
    BoolOrNone r = BN_FALSE;
    for (const auto &i : l) {
      Value li(i->eval(env));
      if (unknown(li)) {
        r = BN_UNKNOWN;
        continue;
      }
      if (ve == li) {
        return BN_TRUE;
      }
    }
    return r;
  }
};

class NotInExpression : public BoolExpression {
  std::unique_ptr<Expression> e;
  std::vector<std::shared_ptr<Expression>> l;

 public:
  NotInExpression(Expression *e_, std::vector<std::shared_ptr<Expression>> &l_) : e(e_) { l.swap(l_); }

  void repr(ostream &os) const override {
    os << *e << " NOT IN (";
    for (std::size_t i = 0; i < l.size(); ++i) {
      os << l[i] << ((i < (l.size() - 1)) ? ", " : ")");  //-V658
    }
  }

  BoolOrNone eval_bool(const SelectorEnv &env) const override {
    Value ve(e->eval(env));
    if (unknown(ve)) {
      return BN_UNKNOWN;
    }
    BoolOrNone r = BN_TRUE;
    for (const auto &i : l) {
      Value li(i->eval(env));
      if (unknown(li)) {
        r = BN_UNKNOWN;
        continue;
      }
      // Check if types are incompatible. If nothing further in the list
      // matches or is unknown and we had a type incompatibility then
      // result still false.
      if (r != BN_UNKNOWN && !sameType(ve, li) && !(numeric(ve) && numeric(li))) {
        r = BN_FALSE;
        continue;
      }

      if (ve == li) {
        return BN_FALSE;
      }
    }
    return r;
  }
};

// Arithmetic Expression types

class ArithmeticExpression : public Expression {
  ArithmeticOperator *op;
  std::unique_ptr<Expression> e1;
  std::unique_ptr<Expression> e2;

 public:
  ArithmeticExpression(ArithmeticOperator *o, Expression *e, Expression *e_) : op(o), e1(e), e2(e_) {}

  void repr(ostream &os) const override { os << "(" << *e1 << *op << *e2 << ")"; }

  Value eval(const SelectorEnv &env) const override { return op->eval(*e1, *e2, env); }
};

class UnaryArithExpression : public Expression {
  UnaryArithmeticOperator *op;
  std::unique_ptr<Expression> e1;

 public:
  UnaryArithExpression(UnaryArithmeticOperator *o, Expression *e) : op(o), e1(e) {}

  void repr(ostream &os) const override { os << *op << "(" << *e1 << ")"; }

  Value eval(const SelectorEnv &env) const override { return op->eval(*e1, env); }
};

// Expression types...

class Literal : public Expression {
  const Value value;

 public:
  template <typename T>
  explicit Literal(const T &v) : value(v) {}

  void repr(ostream &os) const override { os << value; }

  Value eval(const SelectorEnv &) const override { return value; }
};

class StringLiteral : public Expression {
  const string value;

 public:
  explicit StringLiteral(string v) : value(std::move(v)) {}

  void repr(ostream &os) const override { os << "'" << value << "'"; }

  Value eval(const SelectorEnv &) const override { return Value(value); }
};

class Identifier : public Expression {
  string identifier;

 public:
  explicit Identifier(string i) : identifier(std::move(i)) {}

  void repr(ostream &os) const override { os << "I:" << identifier; }

  Value eval(const SelectorEnv &env) const override { return env.value(identifier); }
};

////////////////////////////////////////////////////

// Some operators...

typedef bool BoolOp(const Value &, const Value &);

BoolOrNone booleval(BoolOp *op, Expression &e1, Expression &e2, const SelectorEnv &env) {
  if (op) {
    const Value v1(e1.eval(env));
    if (!unknown(v1)) {
      const Value v2(e2.eval(env));
      if (!unknown(v2)) {
        return BoolOrNone(op(v1, v2));
      }
    }
  }
  return BN_UNKNOWN;
}

// "="
class Eq : public ComparisonOperator {
  void repr(ostream &os) const override { os << "="; }

  BoolOrNone eval(Expression &e1, Expression &e2, const SelectorEnv &env) const override { return booleval(&operator==, e1, e2, env); }
};

// "<>"
class Neq : public ComparisonOperator {
  void repr(ostream &os) const override { os << "<>"; }

  BoolOrNone eval(Expression &e1, Expression &e2, const SelectorEnv &env) const override { return booleval(&operator!=, e1, e2, env); }
};

// "<"
class Ls : public ComparisonOperator {
  void repr(ostream &os) const override { os << "<"; }

  BoolOrNone eval(Expression &e1, Expression &e2, const SelectorEnv &env) const override { return booleval(&operator<, e1, e2, env); }
};

// ">"
class Gr : public ComparisonOperator {
  void repr(ostream &os) const override { os << ">"; }

  BoolOrNone eval(Expression &e1, Expression &e2, const SelectorEnv &env) const override { return booleval(&operator>, e1, e2, env); }
};

// "<="
class Lseq : public ComparisonOperator {
  void repr(ostream &os) const override { os << "<="; }

  BoolOrNone eval(Expression &e1, Expression &e2, const SelectorEnv &env) const override { return booleval(&operator<=, e1, e2, env); }
};

// ">="
class Greq : public ComparisonOperator {
  void repr(ostream &os) const override { os << ">="; }

  BoolOrNone eval(Expression &e1, Expression &e2, const SelectorEnv &env) const override { return booleval(&operator>=, e1, e2, env); }
};

// "IS NULL"
class IsNull : public UnaryBooleanOperator {
  void repr(ostream &os) const override { os << "IsNull"; }

  BoolOrNone eval(Expression &e, const SelectorEnv &env) const override { return BoolOrNone(unknown(e.eval(env))); }
};

// "IS NOT NULL"
class IsNonNull : public UnaryBooleanOperator {
  void repr(ostream &os) const override { os << "IsNonNull"; }

  BoolOrNone eval(Expression &e, const SelectorEnv &env) const override { return BoolOrNone(!unknown(e.eval(env))); }
};

// "NOT"
class Not : public UnaryBooleanOperator {
  void repr(ostream &os) const override { os << "NOT"; }

  BoolOrNone eval(Expression &e, const SelectorEnv &env) const override {
    BoolOrNone bn = e.eval_bool(env);
    if (bn == BN_UNKNOWN) {
      return bn;
    } else {
      return BoolOrNone(!bn);
    }
  }
};

class Negate : public UnaryArithmeticOperator {
  void repr(ostream &os) const override { os << "-"; }

  Value eval(Expression &e, const SelectorEnv &env) const override { return -e.eval(env); }
};

class Add : public ArithmeticOperator {
  void repr(ostream &os) const override { os << "+"; }

  Value eval(Expression &e1, Expression &e2, const SelectorEnv &env) const override { return e1.eval(env) + e2.eval(env); }
};

class Sub : public ArithmeticOperator {
  void repr(ostream &os) const override { os << "-"; }

  Value eval(Expression &e1, Expression &e2, const SelectorEnv &env) const override { return e1.eval(env) - e2.eval(env); }
};

class Mult : public ArithmeticOperator {
  void repr(ostream &os) const override { os << "*"; }

  Value eval(Expression &e1, Expression &e2, const SelectorEnv &env) const override { return e1.eval(env) * e2.eval(env); }
};

class Div : public ArithmeticOperator {
  void repr(ostream &os) const override { os << "/"; }

  Value eval(Expression &e1, Expression &e2, const SelectorEnv &env) const override { return e1.eval(env) / e2.eval(env); }
};

static Eq eqOp;
static Neq neqOp;
static Ls lsOp;
static Gr grOp;
static Lseq lseqOp;
static Greq greqOp;
static IsNull isNullOp;
static IsNonNull isNonNullOp;
static Not notOp;

static Negate negate;
static Add add;
static Sub sub;
static Mult mult;
static Div div;

////////////////////////////////////////////////////

// Top level parser
class TopBoolExpression : public TopExpression {
  std::unique_ptr<Expression> expression;

  void repr(ostream &os) const override { expression->repr(os); }

  bool eval(const SelectorEnv &env) const override {
    BoolOrNone bn = expression->eval_bool(env);
    return (bn == BN_TRUE);
  }

 public:
  explicit TopBoolExpression(Expression *be) : expression(be) {}
};

void throwParseError(Tokeniser &tokeniser, const string &msg) {
  tokeniser.returnTokens();
  string error("Illegal selector: '");
  error += tokeniser.nextToken().val;
  error += "': ";
  error += msg;
  throw std::range_error(error);
}

class Parse {
  friend TopExpression *TopExpression::parse(const string &);

  string error;

  Expression *selectorExpression(Tokeniser &tokeniser) {
    if (tokeniser.nextToken().type == TokenType::T_EOS) {
      return (new Literal(true));
    }
    tokeniser.returnTokens();
    std::unique_ptr<Expression> e(orExpression(tokeniser));
    return e.release();
  }

  Expression *orExpression(Tokeniser &tokeniser) {
    std::unique_ptr<Expression> e(andExpression(tokeniser));
    if (!e) {
      return nullptr;
    }
    while (tokeniser.nextToken().type == TokenType::T_OR) {
      std::unique_ptr<Expression> e1(std::move(e));
      std::unique_ptr<Expression> e2(andExpression(tokeniser));
      if (!e2) {
        return nullptr;
      }
      e = std::make_unique<OrExpression>(e1.release(), e2.release());
    }
    tokeniser.returnTokens();
    return e.release();
  }

  Expression *andExpression(Tokeniser &tokeniser) {
    std::unique_ptr<Expression> e(comparisonExpression(tokeniser));
    if (!e) {
      return nullptr;
    }
    while (tokeniser.nextToken().type == TokenType::T_AND) {
      std::unique_ptr<Expression> e1(std::move(e));
      std::unique_ptr<Expression> e2(comparisonExpression(tokeniser));
      if (!e2) {
        return nullptr;
      }
      e = std::make_unique<AndExpression>(e1.release(), e2.release());
    }
    tokeniser.returnTokens();
    return e.release();
  }

  BoolExpression *specialComparisons(Tokeniser &tokeniser, std::unique_ptr<Expression> e1, bool negated = false) {
    switch (tokeniser.nextToken().type) {
      case TokenType::T_LIKE: {
        const Token t = tokeniser.nextToken();
        if (t.type != TokenType::T_STRING) {
          error = "expected string after LIKE";
          return nullptr;
        }
        // Check for "ESCAPE"
        std::unique_ptr<BoolExpression> l;
        if (tokeniser.nextToken().type == TokenType::T_ESCAPE) {
          const Token e = tokeniser.nextToken();
          if (e.type != TokenType::T_STRING) {
            error = "expected string after ESCAPE";
            return nullptr;
          }
          if (e.val.size() > 1) {
            throwParseError(tokeniser, "single character string required after ESCAPE");
          }
          if (e.val == "%" || e.val == "_") {
            throwParseError(tokeniser, "'%' and '_' are not allowed as ESCAPE characters");
          }
          l = std::make_unique<LikeExpression>(e1.release(), t.val, e.val);
        } else {
          tokeniser.returnTokens();
          l = std::make_unique<LikeExpression>(e1.release(), t.val);
        }
        return (negated ? (BoolExpression *)new UnaryBooleanExpression(&notOp, l.release()) : l.release());
      }
      case TokenType::T_BETWEEN: {
        std::unique_ptr<Expression> lower(addExpression(tokeniser));
        if (!lower) {
          return nullptr;
        }
        if (tokeniser.nextToken().type != TokenType::T_AND) {
          error = "expected AND after BETWEEN";
          return nullptr;
        }
        std::unique_ptr<Expression> upper(addExpression(tokeniser));
        if (!upper) {
          return nullptr;
        }
        std::unique_ptr<BoolExpression> b;
        b = std::make_unique<BetweenExpression>(e1.release(), lower.release(), upper.release());
        return (negated ? (BoolExpression *)new UnaryBooleanExpression(&notOp, b.release()) : b.release());
      }
      case TokenType::T_IN: {
        if (tokeniser.nextToken().type != TokenType::T_LPAREN) {
          error = "missing '(' after IN";
          return nullptr;
        }
        std::vector<std::shared_ptr<Expression>> list;
        do {
          std::unique_ptr<Expression> e(addExpression(tokeniser));
          if (!e) {
            return nullptr;
          }
          list.emplace_back(e.release());
        } while (tokeniser.nextToken().type == TokenType::T_COMMA);
        tokeniser.returnTokens();
        if (tokeniser.nextToken().type != TokenType::T_RPAREN) {
          error = "missing ',' or ')' after IN";
          return nullptr;
        }
        if (negated) {
          return new NotInExpression(e1.release(), list);
        } else {
          return new InExpression(e1.release(), list);
        }
      }
      default:
        error = "expected LIKE, IN or BETWEEN";
        return nullptr;
    }
  }

  Expression *comparisonExpression(Tokeniser &tokeniser) {
    const Token t = tokeniser.nextToken();
    if (t.type == TokenType::T_NOT) {
      std::unique_ptr<Expression> e(comparisonExpression(tokeniser));
      if (!e) {
        return nullptr;
      }
      return new UnaryBooleanExpression(&notOp, e.release());
    }

    tokeniser.returnTokens();
    std::unique_ptr<Expression> e1(addExpression(tokeniser));
    if (!e1) {
      return nullptr;
    }

    switch (tokeniser.nextToken().type) {
      // Check for "IS NULL" and "IS NOT NULL"
      case TokenType::T_IS:
        // The rest must be T_NULL or T_NOT, T_NULL
        switch (tokeniser.nextToken().type) {
          case TokenType::T_NULL:
            return new UnaryBooleanExpression(&isNullOp, e1.release());
          case TokenType::T_NOT:
            if (tokeniser.nextToken().type == TokenType::T_NULL) {
              return new UnaryBooleanExpression(&isNonNullOp, e1.release());
            }  // -V796
          // Fallthru
          default:
            error = "expected NULL or NOT NULL after IS";
            return nullptr;
        }
      case TokenType::T_NOT: {
        return specialComparisons(tokeniser, std::move(e1), true);
      }
      case TokenType::T_BETWEEN:
      case TokenType::T_LIKE:
      case TokenType::T_IN: {
        tokeniser.returnTokens();
        return specialComparisons(tokeniser, std::move(e1));
      }
      default:
        break;
    }
    tokeniser.returnTokens();

    ComparisonOperator *op;
    switch (tokeniser.nextToken().type) {
      case TokenType::T_EQUAL:
        op = &eqOp;
        break;
      case TokenType::T_NEQ:
        op = &neqOp;
        break;
      case TokenType::T_LESS:
        op = &lsOp;
        break;
      case TokenType::T_GRT:
        op = &grOp;
        break;
      case TokenType::T_LSEQ:
        op = &lseqOp;
        break;
      case TokenType::T_GREQ:
        op = &greqOp;
        break;
      default:
        tokeniser.returnTokens();
        return e1.release();
    }

    std::unique_ptr<Expression> e2(addExpression(tokeniser));
    if (!e2) {
      return nullptr;
    }

    return new ComparisonExpression(op, e1.release(), e2.release());
  }

  Expression *addExpression(Tokeniser &tokeniser) {
    std::unique_ptr<Expression> e(multiplyExpression(tokeniser));
    if (!e) {
      return nullptr;
    }

    Token t = tokeniser.nextToken();
    while (t.type == TokenType::T_PLUS || t.type == TokenType::T_MINUS) {
      ArithmeticOperator *op;
      switch (t.type) {
        case TokenType::T_PLUS:
          op = &add;
          break;
        case TokenType::T_MINUS:
          op = &sub;
          break;
        default:
          error = "internal error processing binary + or -";
          return nullptr;
      }
      std::unique_ptr<Expression> e1(std::move(e));
      std::unique_ptr<Expression> e2(multiplyExpression(tokeniser));
      if (!e2) {
        return nullptr;
      }
      e = std::make_unique<ArithmeticExpression>(op, e1.release(), e2.release());
      t = tokeniser.nextToken();
    }

    tokeniser.returnTokens();
    return e.release();
  }

  Expression *multiplyExpression(Tokeniser &tokeniser) {
    std::unique_ptr<Expression> e(unaryArithExpression(tokeniser));
    if (!e) {
      return nullptr;
    }

    Token t = tokeniser.nextToken();
    while (t.type == TokenType::T_MULT || t.type == TokenType::T_DIV) {
      ArithmeticOperator *op;
      switch (t.type) {
        case TokenType::T_MULT:
          op = &mult;
          break;
        case TokenType::T_DIV:
          op = &div;
          break;
        default:
          error = "internal error processing * or /";
          return nullptr;
      }
      std::unique_ptr<Expression> e1(std::move(e));
      std::unique_ptr<Expression> e2(unaryArithExpression(tokeniser));
      if (!e2) {
        return nullptr;
      }
      e = std::make_unique<ArithmeticExpression>(op, e1.release(), e2.release());
      t = tokeniser.nextToken();
    }

    tokeniser.returnTokens();
    return e.release();
  }

  Expression *unaryArithExpression(Tokeniser &tokeniser) {
    const Token t = tokeniser.nextToken();
    switch (t.type) {
      case TokenType::T_LPAREN: {
        std::unique_ptr<Expression> e(orExpression(tokeniser));
        if (!e) {
          return nullptr;
        }
        if (tokeniser.nextToken().type != TokenType::T_RPAREN) {
          error = "missing ')' after '('";
          return nullptr;
        }
        return e.release();
      }
      case TokenType::T_PLUS:
        break;  // Unary + is no op
      case TokenType::T_MINUS: {
        const Token nt = tokeniser.nextToken();
        // Special case for negative numerics
        if (nt.type == TokenType::T_NUMERIC_EXACT) {
          std::unique_ptr<Expression> e(parseExactNumeric(nt, true));
          return e.release();
        } else {
          tokeniser.returnTokens();
          std::unique_ptr<Expression> e(unaryArithExpression(tokeniser));
          if (!e) {
            return nullptr;
          }
          return new UnaryArithExpression(&negate, e.release());
        }
      }
      default:
        tokeniser.returnTokens();
        break;
    }

    std::unique_ptr<Expression> e(primaryExpression(tokeniser));
    return e.release();
  }

  Expression *parseExactNumeric(const Token &token, bool isNegate) {
    int base = 0;
    string s;
    s.reserve(token.val.size());
    std::remove_copy(token.val.begin(), token.val.end(), std::back_inserter(s), '_');
    if (s[1] == 'b' || s[1] == 'B') {
      base = 2;
      s = s.substr(2);
    } else if (s[1] == 'x' || s[1] == 'X') {
      base = 16;
      s = s.substr(2);
    }
    if (s[0] == '0') {
      base = 8;
    }
    errno = 0;
    uint64_t value = strtoull(s.c_str(), nullptr, base);
    if (!errno && (base || value <= INT64_MAX)) {
      int64_t r = value;
      return new Literal((isNegate ? -r : r));
    }
    if (isNegate && value == INT64_MAX + 1ULL) {
      return new Literal(INT64_MIN);
    }
    error = "integer literal too big";
    return nullptr;
  }

  Expression *parseApproxNumeric(const Token &token) {
    errno = 0;
    string s;
    s.reserve(token.val.size());
    std::remove_copy(token.val.begin(), token.val.end(), std::back_inserter(s), '_');
    double value = std::strtod(s.c_str(), nullptr);
    if (!errno) {
      return new Literal(value);
    }
    error = "floating literal overflow/underflow";
    return nullptr;
  }

  Expression *primaryExpression(Tokeniser &tokeniser

  ) {
    const Token &t = tokeniser.nextToken();
    switch (t.type) {
      case TokenType::T_IDENTIFIER:
        return new Identifier(t.val);
      case TokenType::T_STRING:
        return new StringLiteral(t.val);
      case TokenType::T_FALSE:
        return new Literal(false);
      case TokenType::T_TRUE:
        return new Literal(true);
      case TokenType::T_NUMERIC_EXACT:
        return parseExactNumeric(t, false);
      case TokenType::T_NUMERIC_APPROX:
        return parseApproxNumeric(t);
      default:
        error = "expected literal or identifier";
        return nullptr;
    }
  }
};

TopExpression *TopExpression::parse(const string &exp) {
  string::const_iterator s = exp.begin();
  string::const_iterator e = exp.end();
  Tokeniser tokeniser(s, e);
  Parse parse;
  std::unique_ptr<Expression> b(parse.selectorExpression(tokeniser));
  if (!b) {
    throwParseError(tokeniser, parse.error);
  }
  if (tokeniser.nextToken().type != TokenType::T_EOS) {
    throwParseError(tokeniser, "extra input");
  }
  return new TopBoolExpression(b.release());
}
}  // namespace storage
}  // namespace broker
}  // namespace upmq
