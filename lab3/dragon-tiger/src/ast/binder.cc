#include <sstream>

#include "binder.hh"
#include "../utils/errors.hh"
#include "../utils/nolocation.hh"

using utils::error;
using utils::non_fatal_error;

namespace ast {
namespace binder {

/* Returns the current scope */
scope_t &Binder::current_scope() { return scopes.back(); }

/* Pushes a new scope on the stack */
void Binder::push_scope() { scopes.push_back(scope_t()); }

/* Pops the current scope from the stack */
void Binder::pop_scope() { scopes.pop_back(); }

/* Enter a declaration in the current scope. Raises an error if the declared name
 * is already defined */
void Binder::enter(Decl &decl) {
  scope_t &scope = current_scope();
  auto previous = scope.find(decl.name);
  if (previous != scope.end()) {
    non_fatal_error(decl.loc,
                    decl.name.get() + " is already defined in this scope");
    error(previous->second->loc, "previous declaration was here");
  }
  scope[decl.name] = &decl;
}

/* Finds the declaration for a given name. The scope stack is traversed
 * front to back starting from the current scope. The first matching
 * declaration is returned. Raises an error, if no declaration matches. */
Decl &Binder::find(const location loc, const Symbol &name) {
  for (auto scope = scopes.crbegin(); scope != scopes.crend(); scope++) {
    auto decl_entry = scope->find(name);
    if (decl_entry != scope->cend()) {
      return *decl_entry->second;
    }
  }
  error(loc, name.get() + " cannot be found in this scope");
}

Binder::Binder() : scopes() {
  /* Create the top-level scope */
  push_scope();

  /* Populate the top-level scope with all the primitive declarations */
  const Symbol s_int = Symbol("int");
  const Symbol s_string = Symbol("string");
  enter_primitive("print_err", boost::none, {s_string});
  enter_primitive("print", boost::none, {s_string});
  enter_primitive("print_int", boost::none, {s_int});
  enter_primitive("flush", boost::none, {});
  enter_primitive("getchar", s_string, {});
  enter_primitive("ord", s_int, {s_string});
  enter_primitive("chr", s_string, {s_int});
  enter_primitive("size", s_int, {s_string});
  enter_primitive("substring", s_string, {s_string, s_int, s_int});
  enter_primitive("concat", s_string, {s_string, s_string});
  enter_primitive("strcmp", s_int, {s_string, s_string});
  enter_primitive("streq", s_int, {s_string, s_string});
  enter_primitive("not", s_int, {s_int});
  enter_primitive("exit", boost::none, {s_int});
}

/* Declares a new primitive into the current scope*/
void Binder::enter_primitive(
    const std::string &name, const boost::optional<Symbol> &type_name,
    const std::vector<Symbol> &argument_typenames) {
  std::vector<VarDecl *> args;
  int counter = 0;
  for (const Symbol &tn : argument_typenames) {
    std::ostringstream argname;
    argname << "a_" << counter++;
    args.push_back(
        new VarDecl(utils::nl, Symbol(argname.str()), tn, nullptr));
  }

  boost::optional<Symbol> type_name_symbol = boost::none;
  FunDecl *fd = new FunDecl(utils::nl, Symbol(name), type_name, std::move(args), nullptr,
                            true);
  fd->set_external_name(Symbol("__" + name));
  enter(*fd);
}

/* Sets the parent of a function declaration and computes and sets
 * its unique external name */
void Binder::set_parent_and_external_name(FunDecl &decl) {
  auto parent = functions.empty() ? nullptr : functions.back();
  Symbol external_name;
  if (parent) {
    decl.set_parent(parent);
    external_name = parent->get_external_name().get() + '.' + decl.name.get();
  } else
    external_name = decl.name;
  while (external_names.find(external_name) != external_names.end())
    external_name = Symbol(external_name.get() + '_');
  external_names.insert(external_name);
  decl.set_external_name(external_name);
}

/* Binds a whole program. This method wraps the program inside a top-level main
 * function.  Then, it visits the programs with the Binder visitor; binding
 * each identifier to its declaration and computing depths.*/
FunDecl *Binder::analyze_program(Expr &root) {
  std::vector<VarDecl *> main_params;
  Sequence *const main_body = new Sequence(
      utils::nl,
      std::vector<Expr *>({&root, new IntegerLiteral(utils::nl, 0)}));
  FunDecl *const main = new FunDecl(utils::nl, Symbol("main"), Symbol("int"),
                                    main_params, main_body, true);
  main->accept(*this);
  return main;
}

void Binder::visit(IntegerLiteral &literal) {
}

void Binder::visit(StringLiteral &literal) {
}

void Binder::visit(BinaryOperator &op) {		
  op.get_left().accept(*this);
  op.get_right().accept(*this);
}

void Binder::visit(Sequence &seq) {
	size_t i = 0;
	
	std::vector<Expr *> exprs = seq.get_exprs();

	for(i = 0; i < exprs.size(); i++)
		exprs[i]->accept(*this);

}

void Binder::visit(Let &let) {
	push_scope();
	
	std::vector<Decl*> &decls = let.get_decls();
	Sequence &seq = let.get_sequence();
	
	Loop* bcl = bcl_act;
	bcl_act = nullptr;
	
	auto it = decls.begin();
	
	while (it != decls.end())
	{
		
		FunDecl* decl = dynamic_cast<FunDecl*>(*it);
		std::vector<FunDecl*> fun_decls;
		
		if (!decl)
		{
			
			(*it)->accept(*this);
			it++;
			
			continue;
			
		}
		
		while (decl)
		{
			enter(*decl);
		    fun_decls.push_back(decl);
		    it++;
		    
		    if (it == decls.end())
				break;
		
			decl = dynamic_cast<FunDecl*>(*it);
        }
        
        for (size_t i = 0; i < fun_decls.size(); i++)
			fun_decls[i]->accept(*this);
			
	}
			
	bcl_act = bcl;
	seq.accept(*this);
		
	pop_scope();
		
}

void Binder::visit(Identifier &id) {
	/* Initialisation */
	VarDecl* var_decl = dynamic_cast<VarDecl*>(&find(id.loc, id.name));
	
	if(!var_decl)
	{
		utils::error(id.loc, "Error: illegal case....");
	}
	
	else 
	{
		id.set_decl(var_decl);
		id.set_depth(functions.size());
		
		if(var_decl->get_depth() < id.get_depth())
			var_decl->set_escapes();
	}
	
}

void Binder::visit(IfThenElse &ite) {    	
  ite.get_condition().accept(*this); 
  ite.get_then_part().accept(*this);  
  ite.get_else_part().accept(*this);
}

void Binder::visit(VarDecl &decl) {
	
	/* Initialisations */
	optional<Expr &> exprs = decl.get_expr();
	
	decl.set_depth(functions.size());
	
	if(exprs)
		exprs.value().accept(*this);
		
	enter(decl);
	
}

void Binder::visit(FunDecl &decl) {  
	
	/* Initialisations */
	size_t i = 0;
	std::vector<VarDecl*> params = decl.get_params();
	optional<Expr &> exprs = decl.get_expr();
	std::vector<VarDecl*> escp_decls = decl.get_escaping_decls();
	
	set_parent_and_external_name(decl);
    functions.push_back(&decl);
    
    /* ... put your code here ... */
    decl.set_depth(functions.size()-1);

    push_scope(); 
    
    for(i = 0; i < params.size(); i++)
		params[i]->accept(*this);

    if (exprs)
	{
		exprs.value().accept(*this);
	}else
	{
		utils::error(decl.loc, "Error: function isn't defined...");
	}
	
	for (i = 0; i < escp_decls.size(); i++)
		escp_decls[i]->accept(*this);
		
    pop_scope(); 
    
    functions.pop_back();
}

void Binder::visit(FunCall &call) {
  /* Initialisation */
	FunDecl *fun_decl = dynamic_cast<FunDecl*>(&find(call.loc, call.func_name));
	std::vector<Expr *> &expr_args = call.get_args();
	
    if (!fun_decl)
    {
		
		utils::error(call.loc, "Error: Declaration isn't found...");
	
	}else 
	{
		
		call.set_depth(functions.size());
		call.set_decl(fun_decl);

		for (Expr* expr : expr_args)
			expr->accept(*this);
			
	}
}

void Binder::visit(WhileLoop &loop) {
  /* Initialisations */
	Loop* bcl = bcl_act;
	
	loop.get_condition().accept(*this);
	
	bcl_act = &loop;
	
	loop.get_body().accept(*this);
	
	bcl_act = bcl;
}

void Binder::visit(ForLoop &loop) {
}

void Binder::visit(Break &b) {
  if (! bcl_act)
		utils::error(b.loc, "Error: break must be inside the loop...");
		
	b.set_loop(bcl_act);
}

void Binder::visit(Assign &assign) { 	
}

} // namespace binder
} // namespace ast