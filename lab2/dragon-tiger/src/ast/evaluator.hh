#ifndef EVALUATOR_HH
#define EVALUATOR_HH

#include <ostream>

#include "nodes.hh"
//class ConstASTIntVisitor;

namespace evaluator {

class Evaluator : public ConstASTIntVisitor {
  std::ostream *ostream;
  bool verbose;
  unsigned indent_level = 0;
  void inc() { indent_level++; }
  void inl() {
    inc();
    nl();
  };
  void dec() { indent_level--; }
  void dnl() {
    dec();
    nl();
  };

public:
  Evaluator(std::ostream *_ostream, bool _verbose)
      : ostream(_ostream), verbose(_verbose) {}
  void nl() {
    *ostream << std::endl;
    for (unsigned i = 0; i < indent_level; i++)
      *ostream << "  ";
  };
  virtual void visit(const IntegerLiteral &);
  virtual void visit(const BinaryOperator &);
  virtual void visit(const Sequence &);
  virtual void visit(const IfThenElse &);
};

} // namespace evaluator

#endif // _EVALUATOR_HH
