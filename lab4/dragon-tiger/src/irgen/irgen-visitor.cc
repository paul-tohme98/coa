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
// llvm::llvm_set_option("", "-debug")

llvm::Value *IRGenerator::visit(const IntegerLiteral &literal) {
  return Builder.getInt32(literal.value);
}

llvm::Value *IRGenerator::visit(const StringLiteral &literal) {
  return Builder.CreateGlobalStringPtr(literal.value.get());
}

llvm::Value *IRGenerator::visit(const Break &b) {
  //UNIMPLEMENTED();
  //llvm::BasicBlock* break_block = llvm::BasicBlock::Create(Context, "br_block", current_function);
  llvm::BasicBlock *break_block = loop_exit_bbs[b.get_loop().get_ptr()];
  Builder.CreateBr(break_block);
  return nullptr;
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
  //UNIMPLEMENTED();
  // loads value of the identifer allocated mem address
  if(id.get_type() == t_void){
    return nullptr;
  }
  llvm::Value *address = address_of(id);
  // Check if the identifier is stored somewhere in the memory, if yes then load it from memory
  if(address){
    return Builder.CreateLoad(address); ;
  }
  else{
    return nullptr;
  }
}

llvm::Value *IRGenerator::visit(const IfThenElse &ite) {
  //UNIMPLEMENTED();
  llvm::Value *result;
  // Check for errors
  if (!llvm_type(ite.get_type())) {
    return nullptr;
  }
  
  // Create the if-then-else basic blocks
  llvm::BasicBlock* const then_block = llvm::BasicBlock::Create(Context, "if_then", current_function);
  llvm::BasicBlock* const else_block = llvm::BasicBlock::Create(Context, "if_else", current_function);
  llvm::BasicBlock* const end_block = llvm::BasicBlock::Create(Context, "if_end", current_function);

  // If the condition is a void, return null
  if(ite.get_condition().get_type() == t_void){
    return nullptr;
  }
  else{
    // Get the condition
    llvm::Value *const condition = ite.get_condition().accept(*this);
    // If the condition is not null
    if(condition){
      // Branch to either the then or else block depending on the condition
      Builder.CreateCondBr(
        Builder.CreateIsNotNull(condition),
        then_block,
        else_block
      );
      // Allocate memory to store the result of the condition
      result = alloca_in_entry(llvm_type(ite.get_type()), "if_result");

      // Populate the then block
      Builder.SetInsertPoint(then_block);
      // If we're in the then_block, use then_result as result
      llvm::Value* const then_result = ite.get_then_part().accept(*this);
      if(then_result){
        Builder.CreateStore(then_result, result);
        Builder.CreateBr(end_block);    
      }
      // Populate the else block
      Builder.SetInsertPoint(else_block);
      // If we're in the else_block, use else_result as result
      llvm::Value* const else_result = ite.get_else_part().accept(*this);  
      if(else_result){
        Builder.CreateStore(else_result, result);    
        Builder.CreateBr(end_block);
      }   
      // If neither then go to the end_block 
      else{
        Builder.CreateBr(end_block);
      }      
    }
    // If the if condition is null then return null
    else{
      return nullptr;
    }
    // Block joining then and else parts
    Builder.SetInsertPoint(end_block);
    return Builder.CreateLoad(result);
  }
}

llvm::Value *IRGenerator::visit(const VarDecl &decl) {
  //UNIMPLEMENTED();
  // Vector to store variables' types in it
  std::vector<llvm::Type *> var_type;
  // get the decl expression and store it in var_decl
  auto var_decl = decl.get_expr();
  
  // If the expression is null (doesn't exist)
  if(!decl.get_expr()){
    return nullptr;
  }
  // Allocate memory for the variable that we're declaring with the same type as the declaration's type
  llvm::Value *variable = alloca_in_entry(llvm_type(decl.get_type()), decl.name);
  // val will contain the value of the variable declared, if it is declared with a value
  llvm::Value *val;

  // if the declaration is an int or a string (not void)
  if(decl.get_type() != t_void){
    // Add the type to the vector var_type
    var_type.push_back(llvm_type(var_decl->get_type()));
    // Get the value of the declared variable
    val = var_decl.get().accept(*this);
    // If the value exists and is not void
    if((llvm_type(var_decl.value().get_type())) && (var_decl.value().get_type() != t_void)){ // if(val)
      Builder.CreateStore(val, variable);
      allocations[&decl] = variable;      
    }
  }
  // if it's void then push vod to var_type Vector
  else if (decl.get_type() == t_void){
    var_type.push_back(llvm_type(t_void));
  }
  // If not then it's an undefined type, we push t_undef to var_type and return null
  else{
    var_type.push_back(llvm_type(t_undef));
    return nullptr;
  }
  // Return the variable
  return variable;
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
  //UNIMPLEMENTED();
  // Create the basic blocks test, body and end of the loop
  llvm::BasicBlock *const test_block =
      llvm::BasicBlock::Create(Context, "loop_test", current_function);

  llvm::BasicBlock *const body_block =
      llvm::BasicBlock::Create(Context, "loop_body", current_function);

  llvm::BasicBlock *const end_block =
      llvm::BasicBlock::Create(Context, "loop_end", current_function);

  // Create an unconditional branch to test
  Builder.CreateBr(test_block);
  Builder.SetInsertPoint(test_block);
  // Get the loop condition
  llvm::Value *condition = loop.get_condition().accept(*this);
  // If the condition exists
  if(condition){
    // Create a conditional branch (condition true => go to body else go to end)
    Builder.CreateCondBr(Builder.CreateIsNotNull(condition), body_block, end_block);

    Builder.SetInsertPoint(body_block);
    llvm::Value *body_val = loop.get_body().accept(*this);
    // At the end of the body, create an unconditional branch to test
    Builder.CreateBr(test_block);
    Builder.SetInsertPoint(end_block);
    return nullptr;
  }
  else{
    return nullptr;
  }
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
  // Create a basic block that is the entry block
  llvm::BasicBlock *const entry_block = llvm::BasicBlock::Create(Context, "entry", current_function);
  Builder.SetInsertPoint(entry_block);
  // Get the right part of the assignement
  llvm::Value *rhs = assign.get_rhs().accept(*this);
  // If the right part is void then return null
  if(assign.get_rhs().get_type() == t_void){
    return nullptr;
  }
  else{
    // If the address of the assignement is present in memory then get the left part
    if(address_of(assign.get_lhs())){
      llvm::Value *lhs = assign.get_lhs().accept(*this);
      // Store the value (right part) of the assignement in the address of the assignement (address of the left part)
      Builder.CreateStore(rhs, address_of(assign.get_lhs()));
      // Return this address
      return address_of(assign.get_lhs());
    }
    else{
      return nullptr;
    }
  }
}

} // namespace irgen
