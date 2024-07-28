#pragma once
#include <variant>
#include <bits/stdc++.h>
#include "./tokenization.hpp"
#include "./Arena.hpp"

using namespace std;

struct NodeTermInit{
    Token init_lit;
};
struct NodeTermIden{
    Token iden;
};

struct NodeExpr;

struct NodeTermParam{
    NodeExpr* expr;
};
struct NodeTerm{
    variant<NodeTermIden*,NodeTermInit*,NodeTermParam*> var;
};

struct BinExpr;

struct NodeExpr{
    variant<NodeTerm*,BinExpr*> var;
};
struct BinExprAdd{
    NodeExpr* lhs;
    NodeExpr* rhs;
};
struct BinExprMul{
    NodeExpr* lhs;
    NodeExpr* rhs;
};
struct BinExprSub{
    NodeExpr* lhs;
    NodeExpr* rhs;
};
struct BinExprDiv{
    NodeExpr* lhs;
    NodeExpr* rhs;
};
struct BinExpr{
    variant<BinExprAdd*,BinExprMul*,BinExprSub*,BinExprDiv*> var;
};

struct NodeStamtExit{
    NodeExpr* expr;
};
struct NodeStamtLet{
    Token iden;
    NodeExpr* expr;
};
struct NodeStamt;
struct NodeScope{
    vector<NodeStamt*> stamts;
};
struct NodeIfpred;
struct NodeIfpredElif{
    NodeExpr* expr;
    NodeScope* scope;
    optional<NodeIfpred*> pred;
};
struct NodeIfpredElse{
    NodeScope* scope;
};
struct NodeIfpred{
    variant<NodeIfpredElif*,NodeIfpredElse*> var;
};
struct NodeStamtIf{
    NodeExpr* expr;
    NodeScope* scope;
    optional<NodeIfpred*> pred;
};
struct NodeStamtAssign{
    Token iden;
    NodeExpr* expr;
};
struct NodeStamt{
    variant<NodeStamtExit*,NodeStamtLet*,NodeScope*,NodeStamtIf*,NodeStamtAssign*> var;
};
struct NodeProg{
    vector<NodeStamt*> stamts;
};


class Parser{
    private:
        ArenaAllocator m_allocator;
        vector<Token> m_tokens;
        size_t m_index=0;

        inline optional<Token> peek(int offset = 0) const{
            if(m_index + offset >= m_tokens.size())
                return {};
            else
                return {m_tokens[m_index + offset]};
        }

        inline Token consume(){
            return m_tokens[m_index++];
        }

        inline Token try_consume(TokenType type,const string &err_msg){
            if(peek().has_value() && peek().value().type == type){
                return consume();
            }
            else{
                cerr<< err_msg <<endl;
                exit(EXIT_FAILURE);
            }
        }
        inline optional<Token> try_consume(TokenType type){
            if(peek().has_value() && peek().value().type == type){
                return consume();
            }
            else{
                return {};
            }
        }

    public:
        inline explicit Parser(vector<Token> tokens) : m_tokens(move(tokens)),m_allocator(1024 * 1024 * 4){}

        inline optional<NodeTerm*> parse_term(){
            if(auto int_lit = try_consume(TokenType::int_lit)){
                auto node_term = m_allocator.alloc<NodeTermInit>();
                node_term->init_lit = int_lit.value();
                auto term = m_allocator.alloc<NodeTerm>();
                term->var = node_term;
                return term;
            }
            else if(auto iden = try_consume(TokenType::iden)){
                auto node_iden = m_allocator.alloc<NodeTermIden>();
                node_iden->iden = iden.value();
                auto term = m_allocator.alloc<NodeTerm>();
                term->var = node_iden;
                return term;
            }
            else if(auto open_param = try_consume(TokenType::open_param)){
                auto expr = parse_expr();
                if(!expr.has_value()){
                    cerr<<"Expected Expression"<<endl;
                    exit(EXIT_FAILURE);
                }
                try_consume(TokenType::close_param,"Expected ')'");
                auto term_param = m_allocator.alloc<NodeTermParam>();
                term_param->expr=expr.value();
                auto term = m_allocator.alloc<NodeTerm>();
                term->var=term_param;
                return term;
            }
            else{
                return {};
            }
        }

        inline optional<NodeExpr*> parse_expr(int min_prec = 0){
            optional<NodeTerm*> term_lhs = parse_term();
            if (!term_lhs.has_value()) {
                return {};
            }
            auto expr_lhs = m_allocator.alloc<NodeExpr>();
            expr_lhs->var = term_lhs.value();

            while (true) {
                optional<Token> curr_tok = peek();
                optional<int> prec;
                if (curr_tok.has_value()) {
                    prec = bin_prec(curr_tok->type);
                    if (!prec.has_value() || prec < min_prec) {
                        break;
                    }
                }
                else {
                    break;
                }
                Token op = consume();
                int next_min_prec = prec.value() + 1;
                auto expr_rhs = parse_expr(next_min_prec);
                if (!expr_rhs.has_value()) {
                    cerr << "Unable to parse expression" << std::endl;
                    exit(EXIT_FAILURE);
                }
                auto expr = m_allocator.alloc<BinExpr>();
                auto expr_lhs2 = m_allocator.alloc<NodeExpr>();
                if (op.type == TokenType::plus) {
                    auto add = m_allocator.alloc<BinExprAdd>();
                    expr_lhs2->var = expr_lhs->var;
                    add->lhs = expr_lhs2;
                    add->rhs = expr_rhs.value();
                    expr->var = add;
                }
                else if (op.type == TokenType::mul) {
                    auto multi = m_allocator.alloc<BinExprMul>();
                    expr_lhs2->var = expr_lhs->var;
                    multi->lhs = expr_lhs2;
                    multi->rhs = expr_rhs.value();
                    expr->var = multi;
                }
                else if (op.type == TokenType::sub) {
                    auto sub = m_allocator.alloc<BinExprSub>();
                    expr_lhs2->var = expr_lhs->var;
                    sub->lhs = expr_lhs2;
                    sub->rhs = expr_rhs.value();
                    expr->var = sub;
                }
                else if (op.type == TokenType::div) {
                    auto div = m_allocator.alloc<BinExprDiv>();
                    expr_lhs2->var = expr_lhs->var;
                    div->lhs = expr_lhs2;
                    div->rhs = expr_rhs.value();
                    expr->var = div;
                }
                else {
                    assert(false);
                }
                expr_lhs->var = expr;
            }
            return expr_lhs;
        }
        inline optional<NodeScope*> parse_scope(){
            if(!try_consume(TokenType::open_curly).has_value())
                return {};
            auto stamt_scope = m_allocator.alloc<NodeScope>();
            while(auto stamt = parse_stamt()){
                stamt_scope->stamts.push_back(stamt.value());
            }
            try_consume(TokenType::close_curly,"Expected '}'");
            return stamt_scope;
        }

        inline optional<NodeIfpred*> parse_pred(){
            if(try_consume(TokenType::elif)){
                try_consume(TokenType::open_param,"Expected '('");
                auto stamt_elif = m_allocator.alloc<NodeIfpredElif>();
                auto expr = parse_expr();
                if(expr.has_value()){
                    stamt_elif->expr = expr.value();
                }
                else{
                    cerr << "Invalid Expression" <<endl;
                    exit(EXIT_FAILURE);
                }
                try_consume(TokenType::close_param,"Expected ')'");
                auto scope = parse_scope();
                if(scope.has_value()){
                    stamt_elif->scope = scope.value();
                }
                else{
                    cerr << "Invalid Scope" <<endl;
                    exit(EXIT_FAILURE);
                }
                stamt_elif->pred=parse_pred();
                auto pred = m_allocator.alloc<NodeIfpred>();
                pred->var=stamt_elif;
                return pred;
            }
            if(try_consume(TokenType::else_)){
                auto stamt_else = m_allocator.alloc<NodeIfpredElse>();
                auto scope = parse_scope();
                if(scope.has_value()){
                    stamt_else->scope=scope.value();
                }
                else{
                    cerr<<"Invalid Scope"<<endl;
                    exit(EXIT_FAILURE);
                }
                auto pred = m_allocator.alloc<NodeIfpred>();
                pred->var=stamt_else;
                return pred;
            }
            return {};
        }
        inline optional<NodeStamt*> parse_stamt(){
            if(peek().value().type == TokenType::exit && peek(1).has_value()>0 && peek(1).value().type == TokenType::open_param){
                consume();
                consume();
                auto stamt_exit = m_allocator.alloc<NodeStamtExit>();
                auto node_expr = parse_expr();
                if(node_expr.has_value()){
                    stamt_exit->expr = node_expr.value();
                }
                else{
                    cerr<<"Invalid expression"<<endl;
                    exit(EXIT_FAILURE);
                }
                try_consume(TokenType::close_param,"Expected ')'");
                try_consume(TokenType::semi,"Expected ';'");
                
                auto stamt = m_allocator.alloc<NodeStamt>();
                stamt->var = stamt_exit;
                return stamt;
            }
            else if(peek().has_value() && peek().value().type == TokenType::let 
            && peek(1).has_value() && peek(1).value().type == TokenType::iden 
            && peek(2).has_value() && peek(2).value().type == TokenType::eq){
                consume();
                auto var=consume();
                consume();
                auto node = parse_expr();
                auto stamt_let= m_allocator.alloc<NodeStamtLet>();
                if(node.has_value()){
                    stamt_let->iden=var;
                    stamt_let->expr=node.value();
                }
                else{
                    cerr<<"Invalid Expression"<<endl;
                    exit(EXIT_FAILURE);
                }
                try_consume(TokenType::semi,"Expected ';'");
                auto stamt = m_allocator.alloc<NodeStamt>();
                stamt->var=stamt_let;
                return stamt;
            }
            else if(peek().has_value() && peek().value().type == TokenType::open_curly){
                if(auto scope = parse_scope()){
                    auto stamt = m_allocator.alloc<NodeStamt>();
                    stamt->var = scope.value();
                    return stamt;
                }
                else{
                    cerr<<"Invalid Scope"<<endl;
                    exit(EXIT_FAILURE);
                }
            }
            else if(try_consume(TokenType::if_)){
                try_consume(TokenType::open_param,"Expected '('");
                auto stamt_if = m_allocator.alloc<NodeStamtIf>();
                if(auto expr = parse_expr()){
                    stamt_if->expr=expr.value();
                }
                else{
                    cerr<<"Invalid Expression"<<endl;
                    exit(EXIT_FAILURE);
                }
                try_consume(TokenType::close_param,"Expected ')'");
                if(auto scope = parse_scope()){
                    stamt_if->scope = scope.value();
                }
                else{
                    cerr<<"Invalid Scope"<<endl;
                    exit(EXIT_FAILURE);
                }
                stamt_if->pred = parse_pred();
                auto stamt = m_allocator.alloc<NodeStamt>();
                stamt->var=stamt_if;
                return stamt;
            }
            else if(peek().has_value() && peek().value().type == TokenType::iden && peek(1).has_value() && peek(1).value().type == TokenType::eq){
                auto assign = m_allocator.alloc<NodeStamtAssign>();
                assign->iden = consume();
                consume();
                if(auto expr = parse_expr()){
                    assign->expr=expr.value();
                }
                else{
                    cerr<<"Expected an Expression"<<endl;
                    exit(EXIT_FAILURE);
                }
                try_consume(TokenType::semi,"Expected ';'");
                auto stamt = m_allocator.alloc<NodeStamt>();
                stamt->var=assign;
                return stamt;
            }
            else{
                return {};
            }
        }
        inline optional<NodeProg> parse_prog(){
            NodeProg prog;
            while(peek().has_value()){
                if(auto stmt = parse_stamt()){
                    if(stmt.has_value())
                        prog.stamts.push_back(stmt.value());
                }
                else{
                    cerr<<"Invalid Statement"<<endl;
                    exit(EXIT_FAILURE);
                }            
            }
            return prog;
        }
};

