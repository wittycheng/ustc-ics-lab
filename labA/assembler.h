/*
 * @Author       : Chivier Humber
 * @Date         : 2021-08-30 14:36:39
 * @LastEditors  : liuly
 * @LastEditTime : 2022-11-15 21:12:51
 * @Description  : header file for small assembler
 */

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

const int kLC3LineLength = 16;

extern bool gIsErrorLogMode;
extern bool gIsHexMode;

const std::vector<std::string> kLC3Pseudos({
    ".ORIG",
    ".END",
    ".STRINGZ",
    ".FILL",
    ".BLKW",
});

const std::vector<std::string> kLC3Commands({
    "ADD",    // 00: "0001" + reg(line[1]) + reg(line[2]) + op(line[3])
    "AND",    // 01: "0101" + reg(line[1]) + reg(line[2]) + op(line[3])
    "BR",     // 02: "0000000" + pcoffset(line[1],9)
    "BRN",    // 03: "0000100" + pcoffset(line[1],9)
    "BRZ",    // 04: "0000010" + pcoffset(line[1],9)
    "BRP",    // 05: "0000001" + pcoffset(line[1],9)
    "BRNZ",   // 06: "0000110" + pcoffset(line[1],9)
    "BRNP",   // 07: "0000101" + pcoffset(line[1],9)
    "BRZP",   // 08: "0000011" + pcoffset(line[1],9)
    "BRNZP",  // 09: "0000111" + pcoffset(line[1],9)
    "JMP",    // 10: "1100000" + reg(line[1]) + "000000"
    "JSR",    // 11: "01001" + pcoffset(line[1],11)
    "JSRR",   // 12: "0100000"+reg(line[1])+"000000"
    "LD",     // 13: "0010" + reg(line[1]) + pcoffset(line[2],9)
    "LDI",    // 14: "1010" + reg(line[1]) + pcoffset(line[2],9)
    "LDR",    // 15: "0110" + reg(line[1]) + reg(line[2]) + offset(line[3])
    "LEA",    // 16: "1110" + reg(line[1]) + pcoffset(line[2],9)
    "NOT",    // 17: "1001" + reg(line[1]) + reg(line[2]) + "111111"
    "RET",    // 18: "1100000111000000"
    "RTI",    // 19: "1000000000000000"
    "ST",     // 20: "0011" + reg(line[1]) + pcoffset(line[2],9)
    "STI",    // 21: "1011" + reg(line[1]) + pcoffset(line[2],9)
    "STR",    // 22: "0111" + reg(line[1]) + reg(line[2]) + offset(line[3])
    "TRAP"    // 23: "11110000" + h2b(line[1],8)
});

const std::vector<std::string> kLC3TrapRoutine({
    "GETC",   // x20
    "OUT",    // x21
    "PUTS",   // x22
    "IN",     // x23
    "PUTSP",  // x24
    "HALT"    // x25
});

const std::vector<std::string> kLC3TrapMachineCode({
    "1111000000100000",
    "1111000000100001",
    "1111000000100010",
    "1111000000100011",
    "1111000000100100",
    "1111000000100101"
});

enum CommandType { OPERATION, PSEUDO };

static inline void SetErrorLogMode(bool error) {
    gIsErrorLogMode = error;
}

static inline void SetHexMode(bool hex) {
    gIsHexMode = hex;
}

// A warpper class for std::unorderd_map in order to map label to its address
class LabelMapType {
private:
    std::unordered_map<std::string, unsigned> labels_;

public:
    void AddLabel(const std::string &str, unsigned address);
    unsigned GetAddress(const std::string &str) const;
};

static inline int IsLC3Pseudo(const std::string &str) {
    int index = 0;
    for (const auto &command : kLC3Pseudos) {
        if (str == command) {
            return index;
        }
        ++index;
    }
    return -1;
}

static inline int IsLC3Command(const std::string &str) {
    int index = 0;
    for (const auto &command : kLC3Commands) {
        if (str == command) {
            return index;
        }
        ++index;
    }
    return -1;
}

static inline int IsLC3TrapRoutine(const std::string &str) {
    int index = 0;
    for (const auto &trap : kLC3TrapRoutine) {
        if (str == trap) {
            return index;
        }
        ++index;
    }
    return -1;
}

static inline int CharToDec(const char &ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    return -1;
}

static inline char DecToChar(const int &num) {
    if (num <= 9) {
        return num + '0';
    }
    return num - 10 + 'A';
}

// trim string from both left & right
static inline std::string &Trim(std::string &s) {
    // TO BE DONE
    //*check
    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

// Format one line from asm file, do the following:
// 1. remove comments
// 2. convert the line into uppercase
// 3. replace all commas with whitespace (for splitting)
// 4. replace all "\t\n\r\f\v" with whitespace
// 5. remove the leading and trailing whitespace chars
// Note: please implement function Trim first
//check
static std::string FormatLine(const std::string &line) {
    // TO BE DONE
    std::string s=line;
    s=Trim(s);
    if( !s.empty() )
    {
        int n=s.find(";");
        //std::cout<<'\n'<<n<<'\n';
        if (n!=-1)
            s.erase(n,std::string::npos);
        s.erase(0,s.find_first_not_of(" "));
        s.erase(s.find_last_not_of(" ") + 1);
        int pos=0;
        while((pos=s.find("\n",pos))!=std::string::npos)
        {
            s.replace(pos,1," ");
            pos+=1;
        }
        pos=0;
        while((pos=s.find(",",pos))!=std::string::npos)
        {
            s.replace(pos,1," ");
            pos+=1;
        }
        pos=0;
        while((pos=s.find("\t",pos))!=std::string::npos)
        {
            s.replace(pos,1," ");
            pos+=1;
        }
        pos=0;
        while((pos=s.find("\r",pos))!=std::string::npos)
        {
            s.replace(pos,1," ");
            pos+=1;
        }
        pos=0;
        while((pos=s.find("\f",pos))!=std::string::npos)
        {
            s.replace(pos,1," ");
            pos+=1;
        }
        pos=0;
        while((pos=s.find("\v",pos))!=std::string::npos)
        {
            s.replace(pos,1," ");
            pos+=1;
        }
        s.erase(0,s.find_first_not_of(" "));
        s.erase(s.find_last_not_of(" ") + 1);
        transform(s.begin(),s.end(),s.begin(),::toupper);
    }
    //std::cout<<s<<std::endl;
    return s;

}
//
static int RecognizeNumberValue(const std::string &str) {
    // Convert string `str` into a number and return it
    // TO BE DONE
    if (str[0] == '#')
        return std::stoi(str.substr(1));
    else if (str[0] == 'X' || str[0] == 'x')
        return std::stoi(str.substr(1), 0, 16);
    else if(str[0]>='0'&&str[0]<='9')
        return std::stoi(str);
    else return std::numeric_limits<int>::max();
}

static std::string NumberToAssemble(const int &number) {
    // Convert `number` into a 16 bit binary string
    // TO BE DONE
    std::string a="";
    for (int i=0;i<16;i++){
        a.append(1,(char)((number>>(15-i))&1)+48);
    }
    return a;
}

static std::string NumberToAssemble(const std::string &number) {
    // Convert `number` into a 16 bit binary string
    // You might use `RecognizeNumberValue` in this function
    // TO BE DONE
    int t = RecognizeNumberValue(number);
    return NumberToAssemble(t);
}

static std::string ConvertBin2Hex(const std::string &b) {
    // Convert the binary string `bin` into a hex string
    // TO BE DONE
    std::string bin=b;
    bin=Trim(bin);
    int l=bin.size();
    int temp=0;
    int k=0;
    std::string res="";
    std::string a;
    for(int i=0;i<l;i++){
        k=i%4;
        temp+=((int)(bin[l-1-i]-48))<<(k);
        if (i%4==3||i==l-1){
            std::cout<<temp<<'\n';
            switch (temp)
            {
            case 0:
                 a="0";
                res.insert(0,a);

                break;
            case 1:
                 a="1";
                res.insert(0,a);
                break;
            case 2:
                 a="2";
                res.insert(0,a);
                break;
            case 3:
                 a="3";
                res.insert(0,a);
                break;    
            case 4:
                 a="4";
                res.insert(0,a);
                break;  
            case 5:
                 a="5";
                res.insert(0,a);
                break;
            case 6:
                 a="6";
                res.insert(0,a);
                break;       
            case 7:
                 a="7";
                res.insert(0,a);
                break;           
            case 8:
                 a="8";
                res.insert(0,a);
                break;
            case 9:
                 a="9";
                res.insert(0,a);
                break;
            case 10:
                 a="A";
                res.insert(0,a);
                break;           
            case 11:
                 a="B";
                res.insert(0,a);
                break;     
            case 12:
                 a="C";
                res.insert(0,a);
                break; 
            case 13:
                 a="D";
                res.insert(0,a);
                break; 
            case 14:
                 a="E";
                res.insert(0,a);
                break; 
            case 15:
                 a="F";
                res.insert(0,a);
                break; 
            default:
                break;
            }
            temp=0;
        }
    }
    return res;
}

class assembler {
    using Commands = std::vector<std::tuple<unsigned, std::string, CommandType>>;

private:
    LabelMapType label_map;
    Commands commands;

    static std::string TranslatePseudo(std::stringstream &command_stream);
    std::string TranslateCommand(std::stringstream &command_stream,
                                 unsigned int current_address);
    std::string TranslateOprand(unsigned int current_address, std::string str,
                                int opcode_length = 3);
    std::string LineLabelSplit(const std::string &line, int current_address);
    int firstPass(std::string &input_filename);
    int secondPass(std::string &output_filename);

public:
    int assemble(std::string &input_filename, std::string &output_filename);
};
