#pragma once
#include "parser.hpp"
#include <bits/stdc++.h>

using namespace std;

class Generator{
    private:
        struct Var{
            string name;
            size_t stack_loc;
        };

        NodeProg m_root;
        stringstream m_out;
        stringstream m_data;
        size_t m_stack_size=0;
        vector<Var> m_vars;
        vector<size_t> m_scopes;
        int m_label_count = 0;
        int m_label_str_count = 0;
        unordered_map<string, string> m_strings;

        void push(string reg){
            m_out<<"    push "<<reg<<"\n";
            m_stack_size++; 
        }
        void pop(string reg){
            m_out<<"    pop "<<reg<<"\n";
            m_stack_size--; 
        }

        void begin_scope(){
            m_scopes.push_back(m_vars.size());
        }

        void close_scope(){
            size_t toremove = m_vars.size() - m_scopes.back();
            m_out<<"    add rsp, " << toremove*8<< "\n";
            m_stack_size-=toremove;
            for(int i=0;i<toremove;i++)
                m_vars.pop_back();
            m_scopes.pop_back();
        }

        string create_label(){
            return "label" + to_string(m_label_count++);
        }

        string get_string_label(const string& str) {
            if (m_strings.find(str) == m_strings.end()) {
                string label = "str" + to_string(m_label_str_count++);
                m_strings[str] = label;
                m_data << "    " << label << " db \"" << str << "\", 0\n";
            }
            return m_strings[str];
        }
    
    public:

        inline Generator(NodeProg root): m_root(move(root)){}

        void gen_term(const NodeTerm *t){
            struct TermVisitor{
                Generator &gen;
                void operator()(const NodeTermInit *term_int) const{
                    gen.m_out<<"    mov rdi," << term_int->init_lit.value.value() <<"\n";
                    gen.push("rdi");
                }
                void operator()(const NodeTermIden *term_iden) const{
                    auto it = find_if(gen.m_vars.begin(),gen.m_vars.end(),[&](const Var &var){
                        return var.name == term_iden->iden.value.value();
                    });
                    if(it == gen.m_vars.end()){
                        cerr<<"Undeclared Identifier:"<<term_iden->iden.value.value()<<endl;
                        exit(EXIT_FAILURE);
                    }
                    stringstream offset;
                    offset<<" QWORD [rsp +"<<(gen.m_stack_size - (*it).stack_loc - 1)*8 <<"]";
                    gen.push(offset.str());
                }
                void operator()(const NodeTermParam *param) const{
                    gen.gen_expr(param->expr);
                }
            };
            TermVisitor visitor({.gen = *this});
            visit(visitor,t->var);

        }

        void binry_expr(const BinExpr *b){
            struct BinExpr{
                Generator &gen;
                void operator()(const BinExprAdd *add) const{
                    gen.gen_expr(add->rhs);
                    gen.gen_expr(add->lhs);
                    gen.pop("rax");
                    gen.pop("rbx");
                    gen.m_out<<"    add rax, rbx\n";
                    gen.push("rax");
                }
                void operator()(const BinExprSub *sub) const{
                    gen.gen_expr(sub->rhs);
                    gen.gen_expr(sub->lhs);
                    gen.pop("rax");
                    gen.pop("rbx");
                    gen.m_out<<"    sub rax, rbx\n";
                    gen.push("rax");
                }
                void operator()(const BinExprMul *multi) const{
                    gen.gen_expr(multi->rhs);
                    gen.gen_expr(multi->lhs);
                    gen.pop("rax");
                    gen.pop("rbx");
                    gen.m_out<<"    mul rbx\n";
                    gen.push("rax");
                }
                void operator()(const BinExprDiv *div) const{
                    gen.gen_expr(div->rhs);
                    gen.gen_expr(div->lhs);
                    gen.pop("rax");
                    gen.pop("rbx");
                    gen.m_out<<"    div rbx\n";
                    gen.push("rax");
                }
                void operator()(const BinExprEq *equal) const{
                    gen.gen_expr(equal->rhs);
                    gen.gen_expr(equal->lhs);
                    gen.pop("rax");
                    gen.pop("rbx");
                    gen.m_out<<"    cmp rax, rbx\n";
                    gen.m_out<<"    sete al\n";
                    gen.m_out<<"    movzx rax, al\n";
                    gen.push("rax");
                }
                void operator()(const BinExprLess *less) const{
                    gen.gen_expr(less->rhs);
                    gen.gen_expr(less->lhs);
                    gen.pop("rax");
                    gen.pop("rbx");
                    gen.m_out<<"    cmp rax, rbx\n";
                    gen.m_out<<"    setl al\n";
                    gen.m_out<<"    movzx rax, al\n";
                    gen.push("rax");
                }
                void operator()(const BinExprLessEq *lessEq) const{
                    gen.gen_expr(lessEq->rhs);
                    gen.gen_expr(lessEq->lhs);
                    gen.pop("rax");
                    gen.pop("rbx");
                    gen.m_out<<"    cmp rax, rbx\n";
                    gen.m_out<<"    setle al\n";
                    gen.m_out<<"    movzx rax, al\n";
                    gen.push("rax");
                }
                void operator()(const BinExprGreat *great) const{
                    gen.gen_expr(great->rhs);
                    gen.gen_expr(great->lhs);
                    gen.pop("rax");
                    gen.pop("rbx");
                    gen.m_out<<"    cmp rax, rbx\n";
                    gen.m_out<<"    setg al\n";
                    gen.m_out<<"    movzx rax, al\n";
                    gen.push("rax");
                }
                void operator()(const BinExprGreatEq *greatEq) const{
                    gen.gen_expr(greatEq->rhs);
                    gen.gen_expr(greatEq->lhs);
                    gen.pop("rax");
                    gen.pop("rbx");
                    gen.m_out<<"    cmp rax, rbx\n";
                    gen.m_out<<"    setge al\n";
                    gen.m_out<<"    movzx rax, al\n";
                    gen.push("rax");
                }
            };
            BinExpr visitor{.gen = *this};
            visit(visitor,b->var);
        };

        void gen_expr(const NodeExpr*e){
            struct ExprVisitor{
                Generator &gen;
                void operator()(const NodeTerm *term) const{
                    gen.gen_term(term);
                }
                void operator()(const BinExpr *bin_expr) const{
                    gen.binry_expr(bin_expr);
                }
            };
            ExprVisitor visitor{.gen = *this};
            visit(visitor,e->var);
        }

        void gen_scope(const NodeScope* stamt_scope){
            begin_scope();
            for(auto stamt:stamt_scope->stamts)
                gen_stamt(stamt);
            close_scope();
        }
        
        void gen_if(const NodeIfpred* if_,string end_label){
            struct NodeIfpredVisitor{
                Generator &gen;
                string end_label;
                void operator()(const NodeIfpredElif *elif) const{
                    gen.gen_expr(elif->expr);
                    gen.pop("rax");
                    string label = gen.create_label();
                    gen.m_out<<"    test rax, rax\n";
                    gen.m_out<<"    jz "<<label<<"\n";
                    gen.gen_scope(elif->scope);
                    gen.m_out<<"    jmp "<<end_label << "\n";
                    gen.m_out<<label <<":\n";
                    if(elif->pred.has_value()){
                        gen.gen_if(elif->pred.value(),end_label);
                    }
                }
                void operator()(const NodeIfpredElse *else_) const{
                    gen.gen_scope(else_->scope);
                }
            };
            NodeIfpredVisitor visitor{.gen = *this,.end_label = end_label};
            visit(visitor,if_->var);
        }

        void gen_stamt(const NodeStamt *s){
            struct StamtVisitor{
                Generator &gen;
                void operator()(const NodeStamtExit *stamt_exit) const{
                    gen.gen_expr(stamt_exit->expr);
                    gen.m_out<<"    mov rax, 60\n";
                    gen.pop("rdi");
                    gen.m_out<<"    syscall\n";
                }
                void operator()(const NodeStamtLet *stamt_let) const{
                    auto it = find_if(gen.m_vars.begin(),gen.m_vars.end(),[&](const Var &var){
                        return var.name == stamt_let->iden.value.value();
                    });
                    if(it != gen.m_vars.end()){
                        cerr<<"Identifier already used:"<<stamt_let->iden.value.value()<<endl;
                        exit(EXIT_FAILURE);
                    }
                    gen.m_vars.push_back({.name = stamt_let->iden.value.value(), .stack_loc=gen.m_stack_size});
                    gen.gen_expr(stamt_let->expr);
                }
                void operator()(const NodeScope *stamt_scope) const{
                    gen.gen_scope(stamt_scope);
                }
                void operator()(const NodeStamtIf *stamt_if) const{
                    gen.gen_expr(stamt_if->expr);
                    gen.pop("rax");
                    string label = gen.create_label();
                    gen.m_out<<"    test rax, rax\n";
                    gen.m_out<<"    jz "<<label<<"\n";
                    gen.gen_scope(stamt_if->scope);
                    if(stamt_if->pred.has_value()){
                        string end_label = gen.create_label();
                        gen.m_out<<"    jmp "<<end_label << "\n";
                        gen.m_out<<label <<":\n";
                        gen.gen_if(stamt_if->pred.value(),end_label);
                        gen.m_out<<end_label <<":\n";
                    }
                    else{
                        gen.m_out<<label <<":\n";
                    }
                }
                void operator()(const NodeStamtAssign *assign) const{
                    auto it = find_if(gen.m_vars.begin() ,gen.m_vars.end(), [&](const Var &var){
                        return var.name == assign->iden.value.value();
                    });
                    if(it == gen.m_vars.end()){
                        cerr<<"Undeclared Identifier: " << assign->iden.value.value();
                        exit(EXIT_FAILURE);
                    }
                    gen.gen_expr(assign->expr);
                    gen.pop("rax");
                    gen.m_out<<"    mov [rsp + " << (gen.m_stack_size - (*it).stack_loc - 1)*8 <<"],rax\n";
                }
                void operator()(const NodeStamtLoop *loop) const{
                    string label1 = gen.create_label();
                    string label2 = gen.create_label();
                    gen.m_out<<label2<<":\n";
                    gen.gen_expr(loop->expr);
                    gen.pop("rax");
                    gen.m_out<<"    test rax, rax\n";
                    gen.m_out<<"    jz "<<label1<<"\n";
                    gen.gen_scope(loop->scope);
                    gen.m_out<<"    jmp "<<label2 << "\n";
                    gen.m_out<<label1 <<":\n";

                }
                void operator()(const NodeStamtPrint *print){
                    string str_label = gen.get_string_label(print->str);
                    gen.m_out << "    mov rax, 1\n";
                    gen.m_out << "    mov rdi, 1\n";
                    gen.m_out << "    mov rsi, " << str_label << "\n";
                    gen.m_out << "    mov rdx, " << (print->str.length()) << "\n";
                    gen.m_out << "    syscall\n";
                    gen.m_out << "    mov rax, 1\n";
                    gen.m_out << "    mov rdi, 1\n";
                    gen.m_out << "    mov rsi, newline\n"; 
                    gen.m_out << "    mov rdx, 1\n"; 
                    gen.m_out << "    syscall\n";
                }
            };
            StamtVisitor visitor{.gen = *this};
            visit(visitor,s->var);
        }
        string generate(){
            for(auto stmnts:m_root.stamts){
                struct StamtVisitor{
                    Generator &gen;
                    void operator()(const NodeStamtExit *stamt_exit) const{
                    }
                    void operator()(const NodeStamtAssign *assign) const{
                    }
                    void operator()(const NodeStamtLoop *loop) const{
                    }
                    void operator()(const NodeStamtIf *if_) const{
                    }
                    void operator()(const NodeStamtLet *let) const{
                    }
                    void operator()(const NodeScope *scope) const{
                    }
                    void operator()(const NodeStamtPrint *print) const{
                        gen.get_string_label(print->str);
                    }

                };
                StamtVisitor visitor{.gen = *this};
                visit(visitor,stmnts->var);
            }
            m_out << "section .data\n";
            m_out << m_data.str();
            m_out << "    newline db 0xA\n";
            m_out << "section .text\n";
            m_out<< "global _start\n_start:\n";
            for(auto stmnts:m_root.stamts){
                gen_stamt(stmnts);
            }
            m_out<<"    mov rax, 60\n";
            m_out<<"    mov rdi, 0\n";
            m_out<<"    syscall";
            return m_out.str();
        }

};