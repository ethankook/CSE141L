#include <iostream>
#include <unordered_map>
#include <bitset>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>


using namespace std;


// converts register to binary
string registerToBinary3bits(const string &reg){ 
    if(reg == "R0" || reg == "r0") return "000";
    if(reg == "R1" || reg == "r1") return "001";
    if(reg == "R2" || reg == "r2") return "010";
    if(reg == "R3" || reg == "r3") return "011";
    if(reg == "R4" || reg == "r4") return "100";
    if(reg == "R5" || reg == "r5") return "101";
    if(reg == "R6" || reg == "r6") return "110";
    if(reg == "R7" || reg == "r7") return "111";
    return "";

}

string registerToBinary2bits(const string &reg){ 
    if(reg == "R0" || reg == "r0") return "00";
    if(reg == "R1" || reg == "r1") return "01";
    if(reg == "R2" || reg == "r2") return "10";
    if(reg == "R3" || reg == "r3") return "11";

    return "";

}
// converts immediate to n-bit binary number
string immediateToBinary(const string &immediate, int n){
    string fiveBit = bitset<5>(stoi(immediate)).to_string();
    return fiveBit.substr(5 - n);

}
// encodes most R type instructions
string encodeR(const vector<string> &tokens, const unordered_map<string, string> &opcodes){
    string opcode = opcodes.at(tokens[0]);
    string registerFileAddress1 = registerToBinary2bits(tokens[1]);
    string registerFileAddress2 = registerToBinary3bits(tokens[2]); 
    return "0" + registerFileAddress1 + registerFileAddress2 + opcode;
}

string encodeLdrStr(const vector<string> &tokens, const unordered_map<string, string> &opcodes){
    string opcode = opcodes.at(tokens[0]);
    string registerFileAddress1 = registerToBinary2bits(tokens[1]);
    string registerFileAddress2 = registerToBinary2bits(tokens[2]); 
    return "0" + registerFileAddress2 + registerFileAddress1 + opcode;
}
//encodes B type instructions
string encodeB(const vector<string> &tokens, const unordered_map<string, string> &opcodes) {
    string opcode = opcodes.at(tokens[0]);
    string registerfileAddress = registerToBinary2bits(tokens[1]);
    string lutIndex = immediateToBinary(tokens[2], 5);
    return "1" + registerfileAddress + opcode + lutIndex;
}

//encodes mov instruction
string encodeMov(const vector<string> &tokens, const unordered_map<string, string> &opcodes){
    string opcode = opcodes.at(tokens[0]);
    if (stoi(tokens[1]) < 0 || stoi(tokens[1]) > 31) {
        cerr << "Error: immediate value out of range: " << tokens[1] << endl;
        return "";
    }
    string newValue = immediateToBinary(tokens[1], 4);
    return "0" + newValue + opcode;
}

//encodes Mot/ lso/ Mbs
string encodeMot(const vector<string> &tokens, const unordered_map<string, string> &opcodes){
    string opcode = opcodes.at(tokens[0]);
    string registerFileAddress = registerToBinary3bits(tokens[1]);
    string direction = immediateToBinary(tokens[2], 1);
    return "0" + registerFileAddress + direction + opcode;
}
//encodes cfb instruction
string encodeCfb(const vector<string> &tokens, const unordered_map<string, string> &opcodes){
    string opcode = opcodes.at(tokens[0]);
    string registerFileAddress = registerToBinary3bits(tokens[1]);
    return "0" + registerFileAddress + "0" + opcode;
}

//splits line into tokens separated by whitespace
vector<string> toToken(const string &line){
    stringstream ss(line);
    vector<string> tokens;
    string token;
    while(ss >> token){
        tokens.push_back(token);
    }
    return tokens;
}

string trimWhiteSpace(const string &line){
    int start = line.find_first_not_of(" \t\n\r");
    if(start == string::npos) return "";
    int end = line.find_last_not_of(" \t\n\r");
    return line.substr(start, end - start + 1);
}
void assemble (const string &inputFile, const string &outputFile){

    unordered_map<string, string> opcodes;
    opcodes["and"] = "0000";
    opcodes["or"] = "0001";
    opcodes["lsl"] = "0010";
    opcodes["lsr"] = "0011";
    opcodes["add"] = "0100";
    opcodes["neg"] = "0101";
    opcodes["ldr"] = "0110";
    opcodes["str"] = "0111";
    opcodes["mov"] = "1000";
    opcodes["mot"] = "1001";
    opcodes["lso"] = "1010";
    opcodes["cfb"] = "1011";
    opcodes["cmp"] = "1100";
    opcodes["sub"] = "1101";
    opcodes["mbs"] = "1110";
    opcodes["beq"] = "0"; // branch instr 0
    opcodes["bne"] = "1"; // branch instr 1

    ifstream in(inputFile);
    ofstream out(outputFile);

    if(!in.is_open() || !out.is_open()){
        cerr << "Error: could not open file " << inputFile << endl;
        return;
    }

    string line;
    while(getline(in, line)){
        // remove comments
        int comment = line.find("#");
        if(comment != string::npos) line = line.substr(0, comment);

        line = trimWhiteSpace(line);

        vector<string> tokens = toToken(line);
        string instruction;
        if (tokens.empty() || line.empty()) continue;

        if(opcodes.find(tokens[0]) != opcodes.end()){
            if(tokens[0]=="cfb"){
                if (tokens.size() != 2) {
                    cerr << "Error: invalid number of arguments at line: " << line << endl;
                    continue;
                }
                instruction = encodeCfb(tokens, opcodes);
            } else if(tokens[0]=="mov"){
                if (tokens.size() != 2) {
                    cerr << "Error: invalid number of arguments at line: " << line << endl;
                    continue;
                }
                instruction = encodeMov(tokens, opcodes);
            } else if(tokens[0]=="mot" || tokens[0]=="lso" || tokens[0]=="mbs"){
                if (tokens.size() != 3) {
                    cerr << "Error: invalid number of arguments at line: " << line << endl;
                    continue;
                }
                instruction = encodeMot(tokens, opcodes);
            } else if(tokens[0]=="beq" || tokens[0]=="bne"){
                if (tokens.size() != 3) {
                    cerr << "Error: invalid number of arguments at line: " << line << endl;
                    continue;
                }
                if (stoi(tokens[2]) < 0 || stoi(tokens[2]) > 31) {
                    cerr << "Error: LUT index out of range at line: " << line << endl;
                    continue;
                }
                instruction = encodeB(tokens, opcodes);
            } else if (tokens[0]=="ldr" || tokens[0]=="str") {
                if (tokens.size() != 3) {
                    cerr << "Error: invalid number of arguments at line: " << line << endl;
                    continue;
                }
                instruction = encodeLdrStr(tokens, opcodes);
            } else {
                if (tokens.size() != 3) {
                    cerr << "Error: invalid number of arguments at line: " << line << endl;
                    continue;
                }
                instruction = encodeR(tokens, opcodes);
            }

        }
        else{
            cerr << "Error: invalid instruction " << tokens[0] << " at line: " << line << endl;
            continue;
        }
        out << instruction << endl;
    }

    in.close();
    out.close();
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cout << "Incorrect number of arguments: assembler <input file> <output file>" << endl;
        return 1;
    }

    string inputFile = argv[1];
    string outputFile = argv[2];

    assemble(inputFile, outputFile);

    return 0;
}
