#include <bits/stdc++.h>
#include "./Arena.hpp"
#include "./tokenization.hpp"
#include "./parser.hpp"
#include "./generator.hpp"


using namespace std;


int main(int args,char* argv[]){
    if(args != 2){
        cerr<<"Incorrect usage. Correct usage is..."<<endl;
        cerr<<"./build/hydro <test.hy>"<<endl;
        return EXIT_FAILURE;
    }
    string contents;
    {
        fstream input(argv[1], ios::in);
        if (!input.is_open()){
            cerr << "Error: Could not open file " << argv[1] << endl;
            return EXIT_FAILURE;
        }
        stringstream values;
        values << input.rdbuf();
        contents = values.str();
    }
    Tokenizer tokenizer = Tokenizer(move(contents));
    vector<Token> result = tokenizer.tokenize();

    Parser parser = Parser(move(result));
    optional<NodeProg> tree = parser.parse_prog();

    if(!tree.has_value()){
        cout<<"No exit statement found"<<endl;
        return EXIT_FAILURE;
    }
    Generator generator = Generator(tree.value());
    {
        fstream file("out.asm",ios::out);
        file<<generator.generate();
    }

    system("nasm -felf64 out.asm -o out.o");
    system("ld -o out out.o");

    return EXIT_SUCCESS;
}
