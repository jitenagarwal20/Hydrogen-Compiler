#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <optional>

using namespace std;

enum class TokenType{
    exit,int_lit,semi,close_param,open_param,iden,let,eq,plus,mul,sub,div,open_curly,close_curly,if_,elif,else_,gt,gte,lt,lte,eqe,loop,print,string_
};

struct Token{
    TokenType type;
    optional<string> value;
};

optional<int> bin_prec(TokenType type){
    switch (type)
    {
        case TokenType::eqe:
        case TokenType::gt:
        case TokenType::gte:
        case TokenType::lt:
        case TokenType::lte:
            return 0;
        case TokenType::sub:
        case TokenType::plus:
            return 1;
        case TokenType::div:
        case TokenType::mul:
            return 2;
        default:
            return {};
    }
}

class Tokenizer{
    private:
        const std::string m_src;
        size_t m_index=0;

        inline optional<char> peek(int offset = 0) const{
            if(m_index + offset >= m_src.length())
                return {};
            else
                return {m_src[m_index + offset]};
        }

        inline char consume(){
            return m_src[m_index++];
        }


    public:
        inline Tokenizer(std ::string src) : m_src(move(src)){}



        inline vector<Token> tokenize(){
            vector<Token> tokens;
            string temp="";
            while(peek().has_value()){
                if(isalpha(peek().value())){
                    temp+=consume();
                    while(peek().has_value() && isalnum(peek().value())){
                        temp+=consume();
                    }
                    if(temp == "exit"){
                        tokens.push_back({.type = TokenType::exit,.value = temp});
                        temp="";
                        
                    }
                    else if(temp == "assume"){
                        tokens.push_back({.type = TokenType::let,.value = temp});
                        temp="";
                        
                    }
                    else if(temp == "incase"){
                        tokens.push_back({.type = TokenType::if_,.value = temp});
                        temp="";
                        
                    }
                    else if(temp == "until"){
                        tokens.push_back({.type = TokenType::loop,.value = temp});
                        temp="";
                        
                    }
                    else if(temp == "orelse"){
                        tokens.push_back({.type = TokenType::elif,.value = temp});
                        temp="";
                        
                    }
                    else if(temp == "otherwise"){
                        tokens.push_back({.type = TokenType::else_,.value = temp});
                        temp="";
                        
                    }
                    else if(temp == "say"){
                        tokens.push_back({.type = TokenType::print,.value = temp});
                        temp="";
                    }
                    else{
                        tokens.push_back({.type = TokenType::iden,.value = temp});
                        temp=""; 
                    }
                }
                else if(peek().value()=='"'){
                    consume();
                    while(peek().has_value() && peek().value()!='"'){
                        temp+=consume();
                    }
                    if(!peek().has_value()){
                        cerr<<"Expected \""<<endl;
                        exit(EXIT_FAILURE);
                    }
                    else{
                        consume();
                        tokens.push_back({.type = TokenType::string_ , .value=temp});
                    }
                    temp="";
                }
                else if(isdigit(peek().value())){
                    while(peek().has_value() && isdigit(peek().value())){
                        temp+=consume();
                    }
                    tokens.push_back({.type = TokenType::int_lit , .value = temp});
                    temp="";
                    
                }
                else if(peek().value() == '('){
                    consume();
                    tokens.push_back({.type = TokenType::open_param});
                    
                }
                else if(peek().value() == ')'){
                    consume();
                    tokens.push_back({.type = TokenType::close_param});
                    
                }
                else if(peek().value() == '!'){
                    consume();
                    while(peek().has_value() && peek().value()!='\n')
                        consume();
                }
                else if(peek().value() == '.' && peek(1).has_value() && peek(1).value()=='/'){
                    consume();
                    consume();
                    while(peek().has_value()){
                        if(peek().value()=='/' && peek(1).has_value() && peek(1).value()=='.')
                            break;
                        consume();
                    }
                    if(peek().has_value())
                        consume();
                    if(peek().has_value())
                        consume();
                }
                else if(peek().value() == ';'){
                    consume();
                    tokens.push_back({.type = TokenType::semi});
                    
                }
                else if(peek().value() == '='){
                    consume();
                    if(peek().has_value() && peek().value()=='='){
                        consume();
                        tokens.push_back({.type = TokenType::eqe});
                    }
                    else
                        tokens.push_back({.type = TokenType::eq});
                    
                }
                else if(peek().value() == '+'){
                    consume();
                    tokens.push_back({.type = TokenType::plus});
                    
                }
                else if(peek().value() == '*'){
                    consume();
                    tokens.push_back({.type = TokenType::mul});
                    
                }
                else if(peek().value() == '-'){
                    consume();
                    tokens.push_back({.type = TokenType::sub});
                    
                }
                else if(peek().value() == '/'){
                    consume();
                    tokens.push_back({.type = TokenType::div});
                    
                }
                else if(peek().value() == '{'){
                    consume();
                    tokens.push_back({.type = TokenType::open_curly});
                    
                }
                else if(peek().value() == '}'){
                    consume();
                    tokens.push_back({.type = TokenType::close_curly});
                    
                }
                else if(peek().value() == '<'){
                    consume();
                    if(peek().has_value() && peek().value() == '='){
                        consume();
                        tokens.push_back({.type = TokenType::lte});
                    }
                    else
                        tokens.push_back({.type = TokenType::lt});
                    
                }
                else if(peek().value() == '>'){
                    consume();
                    if(peek().has_value() && peek().value() == '='){
                        consume();
                        tokens.push_back({.type = TokenType::gte});
                    }
                    else
                        tokens.push_back({.type = TokenType::gt});
                    
                }
                else if(isspace(peek().value())){
                    consume();
                    
                }
                else{
                    cout<<m_src[m_index]<<endl;
                    cerr<<"You messed up"<<endl;
                    exit(EXIT_FAILURE);
                }
            }
            m_index=0;
            return tokens;
        }

};