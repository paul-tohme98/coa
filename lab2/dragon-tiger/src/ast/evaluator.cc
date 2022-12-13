#include "evaluator.hh"
//class Evaluator;
#include "../utils/errors.hh"

namespace {

char const * const get_type_name(evaluator::Type t) {
  switch (t) {
    case t_int:
      return "int";
    case t_string:
      return "string";
    default:
       utils::error("internal error: attempting to print the type of t_void or t_undef");
  }
}

} // namespace


namespace evaluator {

void Evaluator::visit(const IntegerLiteral &literal) {
  *ostream << literal.value;
}

void Evaluator::visit(const BinaryOperator &binop) {
  *ostream << '(';
  binop.get_left().accept(*this);
  *ostream << operator_name[binop.op];
  binop.get_right().accept(*this);
  *ostream << ')';
}

void Evaluator::visit(const Sequence &seqExpr) {
  *ostream << "(";
  inc();
  const auto exprs = seqExpr.get_exprs();
  for (auto expr = exprs.cbegin(); expr != exprs.cend(); expr++) {
    if (expr != exprs.cbegin())
      *ostream << ';';
    nl();
    (*expr)->accept(*this);
  }
  dnl();
  *ostream << ")";
}

void Evaluator::visit(const IfThenElse &ite) {
  *ostream << "if ";
  inl();
  ite.get_condition().accept(*this);
  dnl();
  *ostream << " then ";
  inl();
  ite.get_then_part().accept(*this);
  dnl();
  *ostream << " else ";
  inl();
  ite.get_else_part().accept(*this);
  dec();
}

} // namespace evaluator
