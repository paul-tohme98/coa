#include <cstdlib>  // For exit
#include <iostream> // For std::cerr
#include "irgen.hh"

#include "llvm/Support/raw_ostream.h"

namespace {

// This function can be removed once the lab has been fully implemented.
[[noreturn]] void UNIMPLEMENTED() {
  std::cerr << "Error: unimplemented feature\n";
  exit(1);
}

} // namespace

namespace irgen {

llvm::Value *IRGenerator::visit(const IntegerLiteral &literal) {
  return Builder.getInt32(literal.value);
}

llvm::Value *IRGenerator::visit(const StringLiteral &literal) {
  return Builder.CreateGlobalStringPtr(literal.value.get());
}

llvm::Value *IRGenerator::visit(const Break &b) {
  UNIMPLEMENTED();
}

llvm::Value *IRGenerator::visit(const BinaryOperator &op) {
  // Void values can be compared for equality only. We directly
  // return 1 or 0 depending on the equality/inequality operator.
  if (op.get_left().get_type() == t_void) {
    return Builder.getInt32(op.op == o_eq);
  }

  llvm::Value *l = op.get_left().accept(*this);
  llvm::Value *r = op.get_right().accept(*this);

  if (op.get_left().get_type() == t_string) {
    auto const strcmp = Mod->getOrInsertFunction("__strcmp", Builder.getInt32Ty(),
        Builder.getInt8PtrTy(), Builder.getInt8PtrTy()
#if LLVM_VERSION_MAJOR < 5
        , nullptr  // Older versions of LLVM uses C-vararg with a NULL end sigil
#endif // LLVM_MAJOR_VERSION < 5
        );
    l = Builder.CreateCall(strcmp, {l, r});
    r = Builder.getInt32(0);
  }

  switch(op.op) {
    case o_plus: return Builder.CreateBinOp(llvm::Instruction::Add, l, r);
    case o_minus: return Builder.CreateBinOp(llvm::Instruction::Sub, l, r);
    case o_times: return Builder.CreateBinOp(llvm::Instruction::Mul, l, r);
    case o_divide: return Builder.CreateBinOp(llvm::Instruction::SDiv, l, r);
    default: break;
  }

  // Comparisons return an i1 result which needs to be
  // casted to i32, as Tiger might use that as an integer.
  llvm::Value *cmp;

  switch(op.op) {
    case o_eq: cmp = Builder.CreateICmpEQ(l, r); break;
    case o_neq: cmp = Builder.CreateICmpNE(l, r); break;
    case o_gt: cmp = Builder.CreateICmpSGT(l, r); break;
    case o_lt: cmp = Builder.CreateICmpSLT(l, r); break;
    case o_ge: cmp = Builder.CreateICmpSGE(l, r); break;
    case o_le: cmp = Builder.CreateICmpSLE(l, r); break;
    default: assert(false); __builtin_unreachable();
  }

  return Builder.CreateIntCast(cmp, Builder.getInt32Ty(), false);
}

llvm::Value *IRGenerator::visit(const Sequence &seq) {
  llvm::Value *result = nullptr;
  for (auto expr : seq.get_exprs())
    result = expr->accept(*this);
  // An empty sequence should return () but the result
  // will never be used anyway, so nullptr is fine.
  return result;
}

llvm::Value *IRGenerator::visit(const Let &let) {
  for (auto decl : let.get_decls())
    decl->accept(*this);

  return let.get_sequence().accept(*this);
}

llvm::Value *IRGenerator::visit(const Identifier &id) {
  UNIMPLEMENTED();
  //llvm::Value *decl;
}

llvm::Value *IRGenerator::visit(const IfThenElse &ite) {
  //UNIMPLEMENTED();
  
  // Create an entry block
  llvm::BasicBlock* const entry_block = llvm::BasicBlock::Create(Context, "entry", current_function);
  Builder.SetInsertPoint(entry_block);
  
  // Allocate memory for the result of the if-then-else statement
  llvm::Value *result = alloca_in_entry(llvm_type(ite.get_type()), "if_result");
  
  // Create the if-then-else basic blocks
  llvm::BasicBlock* const then_block = llvm::BasicBlock::Create(Context, "if_then", current_function);
  llvm::BasicBlock* const else_block = llvm::BasicBlock::Create(Context, "if_else", current_function);
  llvm::BasicBlock* const end_block = llvm::BasicBlock::Create(Context, "if_end", current_function);

  // Convert the condition to a boolean value
  llvm::Value* const condition = ite.get_condition().accept(*this);
  //llvm::Value* const condition_bool = Builder.CreateICmpNE(condition, llvm::Constant::getNullValue(condition->getType()));

  // Branch to either the then or else block depending on the condition
  Builder.CreateCondBr(condition, then_block, else_block);

  // Populate the then block
  //if(condition){
    Builder.SetInsertPoint(then_block);
    llvm::Value* const then_result = ite.get_then_part().accept(*this);
    //llvm::Value* const then_result_cast = Builder.CreateBitCast(then_result, result->getType()->getPointerElementType());
    Builder.CreateStore(then_result, result);
    Builder.CreateBr(end_block);
  //}
  //if(!condition){
    // Populate the else block
    Builder.SetInsertPoint(else_block);
    llvm::Value* const else_result = ite.get_else_part().accept(*this);
    //llvm::Value* const else_result_cast = Builder.CreateBitCast(else_result, result->getType()->getPointerElementType());
    Builder.CreateStore(else_result, result);
    Builder.CreateBr(end_block);
  //}

  // Block joining then and else parts
  Builder.SetInsertPoint(end_block);
  llvm::Value* const final_result = Builder.CreateLoad(result);

  // Check for errors
  if (final_result->getType() != llvm_type(ite.get_type())) {
    //throw std::runtime_error("if-then-else statement result has incorrect type");
    return nullptr;
  }

  return final_result;
}

llvm::Value *IRGenerator::visit(const VarDecl &decl) {
  UNIMPLEMENTED();
  /*std::vector<llvm::Type *> var_type;
  auto var_decl = decl.get_expr();
  //if(decl.get_expr().is_initialized()){
  //  return nullptr;
  //}

  llvm::Value *alloc = alloca_in_entry(llvm_type(decl.get_type()), decl.name);

  if(llvm_type(decl.get_type())){
    var_type.push_back(llvm_type(var_decl->get_type()));
    //Builder.CreateStore(val, alloc);
  }
  else{
    var_type.push_back(llvm_type(t_undef));
    //return nullptr;
  }
  if(decl.get_expr().has_value()){
    
    //Builder.CreateStore(val, alloc);
  }
  return nullptr;*/
}

llvm::Value *IRGenerator::visit(const FunDecl &decl) {
  std::vector<llvm::Type *> param_types;

  for (auto param_decl : decl.get_params()) {
    param_types.push_back(llvm_type(param_decl->get_type()));
  }

  llvm::Type *return_type = llvm_type(decl.get_type());

  llvm::FunctionType *ft =
      llvm::FunctionType::get(return_type, param_types, false);

  llvm::Function::Create(ft,
                         decl.is_external ? llvm::Function::ExternalLinkage
                                          : llvm::Function::InternalLinkage,
                         decl.get_external_name().get(), Mod.get());

  if (decl.get_expr())
    pending_func_bodies.push_front(&decl);

  return nullptr;
}

llvm::Value *IRGenerator::visit(const FunCall &call) {
  // Look up the name in the global module table.
  const FunDecl &decl = call.get_decl().get();
  llvm::Function *callee =
      Mod->getFunction(decl.get_external_name().get());

  if (!callee) {
    // This should only happen for primitives whose Decl is out of the AST
    // and has not yet been handled
    assert(!decl.get_expr());
    decl.accept(*this);
    callee = Mod->getFunction(decl.get_external_name().get());
  }

  std::vector<llvm::Value *> args_values;
  for (auto expr : call.get_args()) {
    args_values.push_back(expr->accept(*this));
  }

  if (decl.get_type() == t_void) {
    Builder.CreateCall(callee, args_values);
    return nullptr;
  }
  return Builder.CreateCall(callee, args_values, "call");
}

// While loop
llvm::Value *IRGenerator::visit(const WhileLoop &loop) {
  // UNIMPLEMENTED();
  llvm::BasicBlock *const test_block =
      llvm::BasicBlock::Create(Context, "loop_test", current_function);

  llvm::BasicBlock *const body_block =
      llvm::BasicBlock::Create(Context, "loop_body", current_function);

  llvm::BasicBlock *const end_block =
      llvm::BasicBlock::Create(Context, "loop_end", current_function);

  Builder.CreateBr(test_block);

  Builder.SetInsertPoint(test_block);

  llvm::Value* const condition = loop.get_condition().accept(*this);

  // Branch to either the then or else block depending on the condition
  Builder.CreateCondBr(Builder.CreateIsNotNull(condition), body_block, end_block);

  Builder.SetInsertPoint(body_block);
  loop.get_body().accept(*this);

  Builder.CreateBr(test_block);
  //Builder.SetInsertPoint(test_block);

  Builder.SetInsertPoint(end_block);
  return nullptr;
}

// For loop
llvm::Value *IRGenerator::visit(const ForLoop &loop) {
  llvm::BasicBlock *const test_block =
      llvm::BasicBlock::Create(Context, "loop_test", current_function);

  llvm::BasicBlock *const body_block =
      llvm::BasicBlock::Create(Context, "loop_body", current_function);

  llvm::BasicBlock *const end_block =
      llvm::BasicBlock::Create(Context, "loop_end", current_function);

  llvm::Value *const index = loop.get_variable().accept(*this);
  llvm::Value *const high = loop.get_high().accept(*this);
  Builder.CreateBr(test_block);

  Builder.SetInsertPoint(test_block);
  Builder.CreateCondBr(Builder.CreateICmpSLE(Builder.CreateLoad(index), high),
                       body_block, end_block);

  Builder.SetInsertPoint(body_block);
  loop.get_body().accept(*this);
  Builder.CreateStore(
      Builder.CreateAdd(Builder.CreateLoad(index), Builder.getInt32(1)), index);
  Builder.CreateBr(test_block);

  Builder.SetInsertPoint(end_block);
  return nullptr;
}

llvm::Value *IRGenerator::visit(const Assign &assign) {
  // UNIMPLEMENTED();
  llvm::BasicBlock *const entry_block = llvm::BasicBlock::Create(Context, "entry", current_function);
  Builder.SetInsertPoint(entry_block);
  llvm::Value* const lhs = assign.get_lhs().accept(*this);
  llvm::Value* const rhs = assign.get_rhs().accept(*this);
  //llvm::Value* const lhs = alloca_in_entry(llvm_type(assign.get_type()), "lhs");
  
  return nullptr;
}

} // namespace irgen
