#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<map>
#include<stack>
using namespace std;

vector<pair<string,string>> token; // toekname과 value가 들어있음.
vector<string> tokens_for_syntax;  //syntax analyzer전용. 대부분 tokenname이 들어있지만 KEYWORD와 ARITHMETIC_OP만 토큰네임이 아닌 상세한 value가 들어있음. CFG의 구조 때문에.
vector<int> token_line;
bool error=false;
string error_string;
string syntax_error_string;
int nowline=0;

class lexical_analyser
{
    private:
        int line;//현재 input파일의 몇번째 줄을 읽고 있는지
        int pointer=0;//현재 읽은 줄의 어떤 글자를 point하고 있는지(인덱스)
    public:
        lexical_analyser(string input,int lineNum)
        {
            line=lineNum;
        }
        void add_token_table(string input); //lexeme을 나누고 symbol table에 추가까지 해주는 함수. 잘못된 부분을 찾으면 아예 그 부분은 symboltable에 포함하지않고 에러메시지를 출력해줌
};
bool is_digit(char a)
{
    if(a=='0'||a=='1'||a=='2'||a=='3'||a=='4'||a=='5'||a=='6'||a=='7'||a=='8'||a=='9')  return true;
    else return false;
}
bool is_non_0_digit(char a)
{
    if(a=='1'||a=='2'||a=='3'||a=='4'||a=='5'||a=='6'||a=='7'||a=='8'||a=='9')  return true;
    else return false;
}
bool is_letter(char a)
{
    if(a=='a'||a=='b'||a=='c'||a=='d'||a=='e'||a=='f'||a=='g'||a=='h'||a=='i'||a=='j'||a=='k'||a=='l'||a=='m'||a=='n'||a=='o'||a=='p'||a=='q'||a=='r'||a=='s'||a=='t'||a=='u'||a=='v'||a=='w'||a=='x'||a=='y'||a=='z'||a=='A'||a=='B'||a=='C'||a=='D'||a=='E'||a=='F'||a=='G'||a=='H'||a=='I'||a=='J'||a=='K'||a=='L'||a=='M'||a=='N'||a=='O'||a=='P'||a=='Q'||a=='R'||a=='S'||a=='T'||a=='U'||a=='V'||a=='W'||a=='X'||a=='Y'||a=='Z')
        return true;
    else return false;
}
void write_token_table(string input,int start_index,int end_index,string token_name) //add_token_table에서 구한 lexeme과 token이름을 매칭시켜서 symboltable(토큰테이블)에 추가해줌
{
    string temp=input.substr(start_index,end_index-start_index+1);
    pair<string,string> p;
    p.first=token_name; p.second=temp;
    token.push_back(p);
}
void branch_to_ID(string input,int* pointer, char c,int start_index,int end_index) // 특정문자(여기선 인자인 c)를 제외한 LETTER,DIGIT이 나오면 ID state로 넘어가는 처리. 그 후 ID를 끝까지 구분해서 table에 넣어줌.
{
    string token_name="ID";
    if((*pointer < input.length()) && input[*pointer]!=c && (is_letter(input[*pointer]) || is_digit(input[*pointer])) ) //특정문자가 아닌 경우 뒤에 잇따라 오는 ID형식을 포함하여 최종 ID상태로 넘어감
    {
        token_name="ID";
        *pointer=*pointer+1;
        while((*pointer < input.length()) && (is_letter(input[*pointer]) || is_digit(input[*pointer])))
        {
            token_name = "ID";
            *pointer=(*pointer)+1; 
        }
        end_index = (*pointer) - 1;
        write_token_table(input, start_index, end_index, token_name);
        token_line.push_back(nowline);
    }
    else //특정문자까지만 ID 임. 그 후는 ID형식에 맞지않거나 끝까지 읽은 경우
    {
        end_index=*pointer-1;
        write_token_table(input, start_index, end_index, token_name);
        token_line.push_back(nowline);
    }
}

void lexical_analyser::add_token_table(string input) // 만든 DFA를 그대로 구현함
{
    while(pointer<input.length())
    {
        string token_name="null";
        int start_index = pointer;
        int end_index = pointer;
        if (input[pointer] == '=') // =기호와 ==기호 구분
        {
            token_name = "ASSIGNMENT_OP";
            pointer++;
            if (pointer < input.length() && input[pointer] == '=')
            {
                token_name = "COMPARISON_OP";
                pointer++;
            }
            end_index=pointer-1;
            write_token_table(input,start_index,end_index,token_name);
            token_line.push_back(line);
        }

        else if(input[pointer]=='<' || input[pointer]=='>') // <,>,<=,>=기호 구분
        {
            token_name="COMPARISON_OP";
            pointer++;
            if(pointer < input.length() && input[pointer] == '=')
            {
                token_name="COMPARISON_OP";
                pointer++;
            }
            end_index=pointer-1;
            write_token_table(input,start_index,end_index,token_name);
            token_line.push_back(line);
        }

        else if(input[pointer]=='!') // !=구분
        {
            token_name="error"; 
            pointer++;
            if(pointer < input.length() && input[pointer] == '=')
            {
                token_name="COMPARISON_OP";
                pointer++;
                end_index = pointer - 1;
                write_token_table(input, start_index, end_index, token_name);
                token_line.push_back(line);
            }
            else // 이어서 =이 오지않으면 에러처리. table에 !는 추가해주지 않음. 오류메시지만 출력
            {
                cout<<"ERROR:"<<line<<"번째줄 "<<start_index+1<<"번 인덱스에 =이 와서 !=을 완성해야하지만 오지 않았습니다.\n";
                error=true;
                error_string+="ERROR:"+to_string(line)+"번째줄 "+to_string(start_index+1)+"번 인덱스에 =이 와서 !=을 완성해야하지만 오지 않았습니다.\n";
            }
        }
        
        else if(input[pointer]==';') // ;구분
        {
            token_name="SEMI";
            pointer++;
            end_index = pointer - 1;
            write_token_table(input, start_index, end_index, token_name);
            token_line.push_back(line);
        }

        else if(input[pointer]=='{') // {구분
        {
            token_name="L_BRACKET";
            pointer++;
            end_index = pointer - 1;
            write_token_table(input, start_index, end_index, token_name);
            token_line.push_back(line);
        }

        else if(input[pointer]=='}') // }구분
        {
            token_name="R_BRACKET";
            pointer++;
            end_index = pointer - 1;
            write_token_table(input, start_index, end_index, token_name);
            token_line.push_back(line);
        }

        else if(input[pointer]=='(') // (구분
        {
            token_name="L_PAREN";
            pointer++;
            end_index = pointer - 1;
            write_token_table(input, start_index, end_index, token_name);
            token_line.push_back(line);
        }

        else if(input[pointer]==')') // )구분
        {
            token_name="R_PAREN";
            pointer++;
            end_index = pointer - 1;
            write_token_table(input, start_index, end_index, token_name);
            token_line.push_back(line);
        }

        else if(input[pointer]==',') // ,구분
        {
            token_name="SEPARATING";
            pointer++;
            end_index = pointer - 1;
            write_token_table(input, start_index, end_index, token_name);
            token_line.push_back(line);
        }

        else if(input[pointer]=='\t'||input[pointer]=='\n'||input[pointer]==' '||input[pointer]=='\r') //화이트스페이스임. 토큰테이블에 추가는 해주지않음.
        {
            token_name="WHITESPACE";
            pointer++;
            end_index = pointer - 1;
        }
        else if(input[pointer]=='-' && (token.back().first=="ARITHMETIC_OP"||token.back().first=="ASSIGNMENT_OP"||token.back().first=="COMPARISON_OP")) // -의 경우 바로앞전 토큰이 op(arithmetic이든 assignment든 comparison이든)이었을 경우에만 음수의-이고 그 외에는 op처리. 이부분은 음수의 -인지를 판별하는 부분
        {
            token_name="error";
            pointer++;
            if(pointer < input.length() && is_non_0_digit(input[pointer]))
            {
                token_name="SIGNED_INTEGER";
                pointer++;
                while(pointer<input.length() && is_digit(input[pointer]))
                {
                    token_name = "SIGNED_INTEGER";
                    pointer++;
                }
                end_index = pointer - 1;
                write_token_table(input, start_index, end_index, token_name);
                token_line.push_back(line);
            }
            else // 음수이므로 -뒤에 숫자가 와야하는데 안왔으므로 에러처리
            {
                cout<<"ERROR:"<<line<<"번째줄 "<<start_index+1<<"번 인덱스에 0이 아닌 숫자가 와서 음수를 구성해야하지만 오지 않았습니다.\n";
                error=true;
                error_string+="ERROR:"+to_string(line)+"번째줄 "+to_string(start_index+1)+"번 인덱스에 0이 아닌 숫자가 와서 음수를 구성해야하지만 오지 않았습니다.\n";
            }
        }
        else if(input[pointer]=='+'||input[pointer]=='-'||input[pointer]=='*'||input[pointer]=='/') // +,-,*,/ 구분
        {
            token_name="ARITHMETIC_OP";
            pointer++;
            end_index = pointer - 1;
            write_token_table(input, start_index, end_index, token_name);
            token_line.push_back(line);
        }
        else if(input[pointer]=='0') // 단순 숫자 0 구분
        {
            token_name="SIGNED_INTEGER";
            pointer++;
            end_index = pointer - 1;
            write_token_table(input, start_index, end_index, token_name);
            token_line.push_back(line);
        }
        else if(is_non_0_digit(input[pointer])) //양의 정수인지 확인
        {
            token_name="SIGNED_INTEGER";
            pointer++;
            while ((pointer < input.length()) && is_digit(input[pointer]))
            {
                token_name = "SIGNED_INTEGER";
                pointer++;
            }
            end_index = pointer - 1;
            write_token_table(input, start_index, end_index, token_name);
            token_line.push_back(line);
        }
        else if(input[pointer]=='"') // "로 시작하고 "로 끝나는 String인지 구분. 저장할때 value는 "는 제외하고 저장함
        {
            token_name="error";
            pointer++;
            while((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer]) || input[pointer]==' '))
            {
                token_name="error";
                pointer++;
            }
            if((pointer < input.length()) && input[pointer]=='"')
            {
                token_name="STRING";
                pointer++;
                end_index=pointer-2;
                if(start_index==end_index) //빈 string일 경우 빈string 넣어줌.
                {
                    string temp = "";
                    pair<string, string> p;
                    p.first = token_name;
                    p.second = temp;
                    token.push_back(p);
                    token_line.push_back(line);
                }
                else
                {
                    start_index+=1;
                    write_token_table(input, start_index, end_index, token_name);
                    token_line.push_back(line);
                }
            }
            else
            {
                cout<<"ERROR:"<<line<<"번째줄에서 \"기호가 나타나고 닫는 \"기호가 나오지 않았거나, letter,digit,빈칸 이외에 다른 문자가 등장하였습니다.\n";
                error=true;
                error_string+="ERROR:"+to_string(line)+"번째줄에서 \"기호가 나타나고 닫는 \"기호가 나오지 않았거나, letter,digit,빈칸 이외에 다른 문자가 등장하였습니다.\n";
            }
        }
        else if(input[pointer]=='i') // int, if 또는 ID 를 구분
        {
            token_name="ID";
            pointer++;
            if(input[pointer]=='f')
            {
                token_name="KEYWORD";
                pointer++;
                while ((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer]))) //이 while문 한번이라도 도는 순간 if(KEYWORD)가 아닌 ID가 됨
                {
                    token_name = "ID";
                    pointer++;
                }
                end_index = pointer - 1;
                write_token_table(input, start_index, end_index, token_name);
                token_line.push_back(line);
            }
            else if(input[pointer]=='n')
            {
                token_name="ID";
                pointer++;
                if((pointer < input.length()) &&input[pointer]=='t')
                {
                    token_name="VTYPE";
                    pointer++;
                    while ((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer]))) //이 while문 한번이라도 도는 순간 int(VTYPE)가 아닌 ID가 됨
                    {
                        token_name = "ID";
                        pointer++;
                    }
                    end_index = pointer - 1;
                    write_token_table(input, start_index, end_index, token_name);
                    token_line.push_back(line);
                }
                else if(input[pointer]!='t' && (is_letter(input[pointer]) || is_digit(input[pointer])))
                {
                    token_name="ID";
                    pointer++;
                    while ((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer])))
                    {
                        token_name = "ID";
                        pointer++;
                    }
                    end_index = pointer - 1;
                    write_token_table(input, start_index, end_index, token_name);
                    token_line.push_back(line);
                }
                else
                {
                    token_name="ID";
                    end_index = pointer - 1;
                    write_token_table(input, start_index, end_index, token_name);
                    token_line.push_back(line);
                }
            }
            else if((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer])))
            {
                token_name="ID";
                pointer++;
                while ((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer]))) 
                {
                    token_name = "ID";
                    pointer++;
                }
                end_index = pointer - 1;
                write_token_table(input, start_index, end_index, token_name);
                token_line.push_back(line);
            }
            else
            {
                token_name="ID";
                end_index = pointer - 1;
                write_token_table(input, start_index, end_index, token_name);
                token_line.push_back(line);
            }
        }
        else if(input[pointer]=='I') // INT, IF 또는 ID 를 구분
        {
            token_name="ID";
            pointer++;
            if(input[pointer]=='F')
            {
                token_name="KEYWORD";
                pointer++;
                while ((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer]))) //이 while문 한번이라도 도는 순간 if(KEYWORD)가 아닌 ID가 됨
                {
                    token_name = "ID";
                    pointer++;
                }
                end_index = pointer - 1;
                write_token_table(input, start_index, end_index, token_name);
                token_line.push_back(line);
            }
            else if(input[pointer]=='N')
            {
                token_name="ID";
                pointer++;
                if((pointer < input.length()) &&input[pointer]=='T')
                {
                    token_name="VTYPE";
                    pointer++;
                    while ((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer]))) //이 while문 한번이라도 도는 순간 int(VTYPE)가 아닌 ID가 됨
                    {
                        token_name = "ID";
                        pointer++;
                    }
                    end_index = pointer - 1;
                    write_token_table(input, start_index, end_index, token_name);
                    token_line.push_back(line);
                }
                else if(input[pointer]!='T' && (is_letter(input[pointer]) || is_digit(input[pointer])))
                {
                    token_name="ID";
                    pointer++;
                    while ((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer])))
                    {
                        token_name = "ID";
                        pointer++;
                    }
                    end_index = pointer - 1;
                    write_token_table(input, start_index, end_index, token_name);
                    token_line.push_back(line);
                }
                else
                {
                    token_name="ID";
                    end_index = pointer - 1;
                    write_token_table(input, start_index, end_index, token_name);
                    token_line.push_back(line);
                }
            }
            else if((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer])))
            {
                token_name="ID";
                pointer++;
                while ((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer]))) 
                {
                    token_name = "ID";
                    pointer++;
                }
                end_index = pointer - 1;
                write_token_table(input, start_index, end_index, token_name);
                token_line.push_back(line);
            }
            else
            {
                token_name="ID";
                end_index = pointer - 1;
                write_token_table(input, start_index, end_index, token_name);
                token_line.push_back(line);
            }
        }
        else if(input[pointer]=='c') //char(VTYPE),ID구분
        {
            token_name="ID";
            pointer++;
            if((pointer < input.length()) && input[pointer]=='h')
            {
                token_name="ID";
                pointer++;
                if((pointer < input.length()) && input[pointer]=='a')
                {
                    token_name="ID";
                    pointer++;
                    if((pointer < input.length()) && input[pointer]=='r')
                    {
                        token_name="VTYPE";
                        pointer++;
                        while ((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer])))
                        {
                            token_name = "ID";
                            pointer++;
                        }
                        end_index = pointer - 1;
                        write_token_table(input, start_index, end_index, token_name);
                        token_line.push_back(line);
                    }
                    else branch_to_ID(input,&pointer,'r',start_index,end_index);
                }
                else branch_to_ID(input,&pointer,'a',start_index,end_index);
            }
            else branch_to_ID(input,&pointer,'h',start_index,end_index);
        }
        else if(input[pointer]=='C') //CHAR(VTYPE),ID구분
        {
            token_name="ID";
            pointer++;
            if((pointer < input.length()) && input[pointer]=='H')
            {
                token_name="ID";
                pointer++;
                if((pointer < input.length()) && input[pointer]=='A')
                {
                    token_name="ID";
                    pointer++;
                    if((pointer < input.length()) && input[pointer]=='R')
                    {
                        token_name="VTYPE";
                        pointer++;
                        while ((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer])))
                        {
                            token_name = "ID";
                            pointer++;
                        }
                        end_index = pointer - 1;
                        write_token_table(input, start_index, end_index, token_name);
                        token_line.push_back(line);
                    }
                    else branch_to_ID(input,&pointer,'R',start_index,end_index);
                }
                else branch_to_ID(input,&pointer,'A',start_index,end_index);
            }
            else branch_to_ID(input,&pointer,'H',start_index,end_index);
        }
        else if(input[pointer]=='e') //else(KEYWORD),ID구분
        {
            token_name="ID";
            pointer++;
            if((pointer < input.length()) && input[pointer]=='l')
            {
                token_name="ID";
                pointer++;
                if((pointer < input.length()) && input[pointer]=='s')
                {
                    token_name="ID";
                    pointer++;
                    if((pointer < input.length()) && input[pointer]=='e')
                    {
                        token_name="KEYWORD";
                        pointer++;
                        while ((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer])))
                        {
                            token_name = "ID";
                            pointer++;
                        }
                        end_index = pointer - 1;
                        write_token_table(input, start_index, end_index, token_name);
                        token_line.push_back(line);
                    }
                    else branch_to_ID(input,&pointer,'e',start_index,end_index);
                }
                else branch_to_ID(input,&pointer,'s',start_index,end_index);
            }
            else branch_to_ID(input,&pointer,'l',start_index,end_index);
        }
        else if(input[pointer]=='e') //ELSE(KEYWORD),ID구분
        {
            token_name="ID";
            pointer++;
            if((pointer < input.length()) && input[pointer]=='L')
            {
                token_name="ID";
                pointer++;
                if((pointer < input.length()) && input[pointer]=='S')
                {
                    token_name="ID";
                    pointer++;
                    if((pointer < input.length()) && input[pointer]=='E')
                    {
                        token_name="KEYWORD";
                        pointer++;
                        while ((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer])))
                        {
                            token_name = "ID";
                            pointer++;
                        }
                        end_index = pointer - 1;
                        write_token_table(input, start_index, end_index, token_name);
                        token_line.push_back(line);
                    }
                    else branch_to_ID(input,&pointer,'E',start_index,end_index);
                }
                else branch_to_ID(input,&pointer,'S',start_index,end_index);
            }
            else branch_to_ID(input,&pointer,'L',start_index,end_index);
        }
        else if(input[pointer]=='w') //while(KEYWORD),ID구분
        {
            token_name="ID";
            pointer++;
            if((pointer < input.length()) && input[pointer]=='h')
            {
                token_name="ID";
                pointer++;
                if((pointer < input.length()) && input[pointer]=='i')
                {
                    token_name="ID";
                    pointer++;
                    if((pointer < input.length()) && input[pointer]=='l')
                    {
                        token_name = "ID";
                        pointer++;
                        if ((pointer < input.length()) && input[pointer] == 'e')
                        {
                            token_name = "KEYWORD";
                            pointer++;
                            while ((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer])))
                            {
                                token_name = "ID";
                                pointer++;
                            }
                            end_index = pointer - 1;
                            write_token_table(input, start_index, end_index, token_name);
                            token_line.push_back(line);
                        }
                        else branch_to_ID(input,&pointer,'e',start_index,end_index);
                    }
                    else branch_to_ID(input,&pointer,'l',start_index,end_index);
                }
                else branch_to_ID(input,&pointer,'i',start_index,end_index);
            }
            else branch_to_ID(input,&pointer,'h',start_index,end_index);
        }
        else if(input[pointer]=='W') //WHILE(KEYWORD),ID구분
        {
            token_name="ID";
            pointer++;
            if((pointer < input.length()) && input[pointer]=='H')
            {
                token_name="ID";
                pointer++;
                if((pointer < input.length()) && input[pointer]=='I')
                {
                    token_name="ID";
                    pointer++;
                    if((pointer < input.length()) && input[pointer]=='L')
                    {
                        token_name = "ID";
                        pointer++;
                        if ((pointer < input.length()) && input[pointer] == 'E')
                        {
                            token_name = "KEYWORD";
                            pointer++;
                            while ((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer])))
                            {
                                token_name = "ID";
                                pointer++;
                            }
                            end_index = pointer - 1;
                            write_token_table(input, start_index, end_index, token_name);
                            token_line.push_back(line);
                        }
                        else branch_to_ID(input,&pointer,'E',start_index,end_index);
                    }
                    else branch_to_ID(input,&pointer,'L',start_index,end_index);
                }
                else branch_to_ID(input,&pointer,'I',start_index,end_index);
            }
            else branch_to_ID(input,&pointer,'H',start_index,end_index);
        }
        else if(input[pointer]=='r') //return(KEYWORD),ID구분
        {
            token_name="ID";
            pointer++;
            if((pointer < input.length()) && input[pointer]=='e')
            {
                token_name="ID";
                pointer++;
                if((pointer < input.length()) && input[pointer]=='t')
                {
                    token_name="ID";
                    pointer++;
                    if((pointer < input.length()) && input[pointer]=='u')
                    {
                        token_name = "ID";
                        pointer++;
                        if ((pointer < input.length()) && input[pointer] == 'r')
                        {
                            token_name="ID";
                            pointer++;
                            if((pointer < input.length()) && input[pointer] == 'n')
                            {
                                token_name = "KEYWORD";
                                pointer++;
                                while ((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer])))
                                {
                                    token_name = "ID";
                                    pointer++;
                                }
                                end_index = pointer - 1;
                                write_token_table(input, start_index, end_index, token_name);
                                token_line.push_back(line);
                            }
                            else branch_to_ID(input,&pointer,'n',start_index,end_index);
                        }
                        else branch_to_ID(input,&pointer,'r',start_index,end_index);
                    }
                    else branch_to_ID(input,&pointer,'u',start_index,end_index);
                }
                else branch_to_ID(input,&pointer,'t',start_index,end_index);
            }
            else branch_to_ID(input,&pointer,'e',start_index,end_index);
        }
        else if(input[pointer]=='R') //RETURN(KEYWORD),ID구분
        {
            token_name="ID";
            pointer++;
            if((pointer < input.length()) && input[pointer]=='E')
            {
                token_name="ID";
                pointer++;
                if((pointer < input.length()) && input[pointer]=='T')
                {
                    token_name="ID";
                    pointer++;
                    if((pointer < input.length()) && input[pointer]=='U')
                    {
                        token_name = "ID";
                        pointer++;
                        if ((pointer < input.length()) && input[pointer] == 'R')
                        {
                            token_name="ID";
                            pointer++;
                            if((pointer < input.length()) && input[pointer] == 'N')
                            {
                                token_name = "KEYWORD";
                                pointer++;
                                while ((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer])))
                                {
                                    token_name = "ID";
                                    pointer++;
                                }
                                end_index = pointer - 1;
                                write_token_table(input, start_index, end_index, token_name);
                                token_line.push_back(line);
                            }
                            else branch_to_ID(input,&pointer,'N',start_index,end_index);
                        }
                        else branch_to_ID(input,&pointer,'R',start_index,end_index);
                    }
                    else branch_to_ID(input,&pointer,'U',start_index,end_index);
                }
                else branch_to_ID(input,&pointer,'T',start_index,end_index);
            }
            else branch_to_ID(input,&pointer,'E',start_index,end_index);
        }
        else if((is_letter(input[pointer]) || is_digit(input[pointer]))) //ID인 경우
        {
            token_name = "ID";
            pointer++;
            while ((pointer < input.length()) && (is_letter(input[pointer]) || is_digit(input[pointer])))
            {
                token_name = "ID";
                pointer++;
            }
            end_index = pointer - 1;
            write_token_table(input, start_index, end_index, token_name);
            token_line.push_back(line);
        }
        else //토큰이 절대로 될 수 없는 charcter를 읽었을때
        {
            cout<<"ERROR:"<<line<<"번째줄 "<<pointer<<"번 인덱스에 해당 컴파일러가 인식할 수 없는 charcter가 존재합니다.\n";
            pointer++;
            error=true;
            error_string+="ERROR:"+to_string(line)+"번째줄 "+to_string(pointer)+"번 인덱스에 해당 컴파일러가 인식할 수 없는 charcter가 존재합니다.\n";
        }
    }
   
}


class syntax_analyzer
{
    private:
        vector<map<string, string>> action_table; // action테이블 저장. ex)표에서 row(state)=1 이고 column(터미널)="ID"인 값은 action_table[1]["ID"]로 접근함.
        vector<map<string, string>> goto_table;   // goto테이블 저장. ex)표에서 row(state)=1 이고 column(논터미널)="STMT"인 값은 action_table[1]["STMT"]로 접근함.
        vector<string> terminal;                  // action표에서 사용되는 터미널들을 저장함.
        vector<string> nonterminal;               // goto표에서 사용되는 논터미널들을 저장함.
        stack<int> syntax_stack;                  //SLR파싱 과정에서 사용할 스택
        string next_input_symbol;                 //현재 next_input_symbol이 무엇인지 저장
        pair<int,string> reduce_arr[27]; //reduce를 실행할때 스택에서 몇개나 pop해줘야하는지와 A->a에서 A문자열을 저장해줌. ex)R5이면 reduce_arr[5].first의 값만큼 pop을해줌.
    public:
        void set_terminal(string line);              // action테이블의 terminal의 값을 설정해줌.
        void set_action_table(string line, int row); // action테이블에 파일로부터 읽어온 값을 채워 넣는 메소드

        void set_nonterminal(string line);         // goto테이블의 nonterminal의 값을 설정해줌.
        void set_goto_table(string line, int row); // goto테이블에 파일로부터 읽어온 값을 채워 넣는 메소드
        void set_reduce_arr();//reduce_arr 페어에 값을 넣어줌. 이는 SLR파서 테이블을만드는 사이트에서 사용한 정보를 넣어주는 과정임.
        void check_syntax();
};

void syntax_analyzer::set_terminal(string line)
{
    int index=0;
    int start_index=0;
    int count=0;
    while(index<line.length())
    {
        if(line[index]=='\t' || line[index]=='\n' || line[index]==' ' || line[index]=='\r') //화이트스페이스 무시
        {
            index++;
        }
        else
        {
            start_index=index;
            while(index<line.length() && line[index]!='\t' && line[index]!='\n' && line[index]!=' ' && line[index]!='\r')
            {
                index++;
            }
            if(index==line.length()-1)
            {
                index++;
            }
            count=index-start_index;
            terminal.push_back(line.substr(start_index,count));
        }
    }
}
void syntax_analyzer::set_action_table(string line,int row)
{
    int index = 0;
    int start_index = 0;
    int count = 0;
    int terminal_count=0;
    map<string,string> ma;
    action_table.push_back(ma);
    while (index < line.length())
    {
        if (line[index] == '\t' || line[index] == '\n' || line[index] == ' ' || line[index] == '\r') //화이트스페이스 무시
        {
            index++;
        }
        else
        {
            start_index = index;
            while (index < line.length() && line[index] != '\t' && line[index] != '\n' && line[index] != ' ' && line[index] != '\r')
            {
                index++;
            }
            if (index == line.length() - 1)
            {
                index++;
            }
            count = index - start_index;
            action_table[row][terminal[terminal_count]]=line.substr(start_index, count);
            terminal_count++;
        }
    }
}

void syntax_analyzer::set_nonterminal(string line)
{
    int index=0;
    int start_index=0;
    int count=0;
    while(index<line.length())
    {
        if(line[index]=='\t' || line[index]=='\n' || line[index]==' ' || line[index]=='\r') //화이트스페이스 무시
        {
            index++;
        }
        else
        {
            start_index=index;
            while(index<line.length() && line[index]!='\t' && line[index]!='\n' && line[index]!=' ' && line[index]!='\r')
            {
                index++;
            }
            if(index==line.length()-1)
            {
                index++;
            }
            count=index-start_index;
            nonterminal.push_back(line.substr(start_index,count));
        }
    }
}

void syntax_analyzer::set_goto_table(string line,int row)
{
    int index = 0;
    int start_index = 0;
    int count = 0;
    int nonterminal_count=0;
    map<string,string> ma;
    goto_table.push_back(ma);
    while (index < line.length())
    {
        if (line[index] == '\t' || line[index] == '\n' || line[index] == ' ' || line[index] == '\r') //화이트스페이스 무시
        {
            index++;
        }
        else
        {
            start_index = index;
            while (index < line.length() && line[index] != '\t' && line[index] != '\n' && line[index] != ' ' && line[index] != '\r')
            {
                index++;
            }
            count = index - start_index;
            goto_table[row][nonterminal[nonterminal_count]]=line.substr(start_index, count);
            nonterminal_count++;
        }
    }
}
void syntax_analyzer::set_reduce_arr()
{
    int intarr[27]={1,2,2,0,3,9,3,0,4,0,2,0,1,4,11,7,1,1,3,1,3,1,3,1,1,3,3};
    string stringarr[27]={"S'","CODE","CODE","CODE","VDECL","FDECL","ARG","ARG","MOREARGS","MOREARGS","BLOCK","BLOCK","STMT","STMT","STMT","STMT","RHS","RHS","EXPR","EXPR","TERM","TERM","FACTOR","FACTOR","FACTOR","COND","RETURN"};
    for(int i=0;i<27;i++)
    {
        reduce_arr[i].first=intarr[i];
        reduce_arr[i].second=stringarr[i];
    }
}

int get_number(string input) //action table에서 S나 R뒤에 있는 숫자를 추출해줌.
{
    string temp;
    for(int i=1;i<input.size();i++)
    {
        temp+=input[i];
    }
    return stoi(temp);
}
void syntax_analyzer::check_syntax() //CFG에 따른 syntax 적합성 검사
{
    int index=0;
    syntax_stack.push(0);//스택에 start state 넣어줌
    next_input_symbol=tokens_for_syntax[index]; //next input symbol 초기설정. shift할때마다 바뀔예정
    set_reduce_arr();
    while(1)
    {
        string value=action_table[syntax_stack.top()][next_input_symbol];
        if(value[0]=='s') //shift관련 처리. index수정 및 next_input_symbol 수정 필요함.
        {
            syntax_stack.push(get_number(value));
            index++;
            next_input_symbol=tokens_for_syntax[index];
        }
        else if(value[0]=='r')//reduce관련 처리. goto관련 처리도 여기서 함.
        {
            for(int i=0;i<reduce_arr[get_number(value)].first;i++)
            {
                syntax_stack.pop();
            }
            syntax_stack.push(stoi(goto_table[syntax_stack.top()][reduce_arr[get_number(value)].second]));
        }
        else if(value[0]=='a') //acc관련 처리. 마지막인지?
        {
            cout<<"\nSyntax OK. Accept";
            syntax_error_string+="Syntax OK. Accept";
            break;
        }
        else // 오류처리. 표에서 #일때. (즉 빈칸)
        {
            cout<<"\nsynatx에 어긋납니다.\n";
            cout<<token_line[index]<<"번째 줄, ";
            cout<<"'"<<token[index].second<<"'가 올 수 없는 자리입니다\n";
            syntax_error_string+="\nsynatx에 어긋납니다.\n"+to_string(token_line[index])+"번째 줄, '"+token[index].second+"'가 올 수 없는 자리입니다\n";
            for(int i=0; i<terminal.size(); i++)
            {
                if(action_table[syntax_stack.top()][terminal[i]]!="#")
                {
                    string temp_value=action_table[syntax_stack.top()][terminal[i]];
                    stack<int> temp_stack(syntax_stack); // 다음에 올수있는 토큰이 과연 유효할지를 검사하기 위한 스택.
                    if(temp_value[0]=='r')
                    {
                        for (int j = 0; j < reduce_arr[get_number(temp_value)].first; j++)
                        {
                           temp_stack.pop();
                        }
                        temp_stack.push(stoi(goto_table[temp_stack.top()][reduce_arr[get_number(temp_value)].second]));
                        if(action_table[temp_stack.top()][terminal[i]]!="#")
                        {
                            cout<<terminal[i]<<" ";
                            syntax_error_string+=terminal[i]+" ";
                        }
                    }
                    else    {cout<<terminal[i]<<" ";  syntax_error_string+=terminal[i]+" ";}
                }
            }
            cout<<"<== 이 중에서 하나가 등장해야 문법상 맞을 가능성이 있습니다.";
            cout<<"\nsyntax analyzer를 종료합니다.";
            syntax_error_string+="<== 이 중에서 하나가 등장해야 문법상 맞을 가능성이 있습니다.";
            syntax_error_string+="\nsyntax analyzer를 종료합니다.";
            break;
        }
    }
}

//syntax analyzer에서 사용하는 CFG에 맞춰서 symbol table의 내용을 적절히 처리해서 tokens_for_syntax전역변수에 넣어줌. 
//이 vector타입 변수 tokens_for_syntax는 syntax_analyzer에서 input으로 사용됨
void set_tokens_for_syntax()
{
    for(int i=0; i<token.size(); i++)
    {
        if (token[i].first == "KEYWORD")
        {
            if(token[i].second =="if" || token[i].second =="IF")
                tokens_for_syntax.push_back("if");
            else if(token[i].second =="else" || token[i].second =="ELSE")
                tokens_for_syntax.push_back("else");
            else if(token[i].second =="while" || token[i].second =="WHILE")
                tokens_for_syntax.push_back("while");
            else if(token[i].second =="return" || token[i].second =="RETURN")
                tokens_for_syntax.push_back("return");
        }
        else if(token[i].first == "ARITHMETIC_OP")
        {
            if(token[i].second=="+" || token[i].second=="-")
                tokens_for_syntax.push_back("addsub");
            else if(token[i].second=="*" || token[i].second=="/")
                tokens_for_syntax.push_back("multdiv");
        }
        else
            tokens_for_syntax.push_back(token[i].first);
    }
    tokens_for_syntax.push_back("$");//마지막에 $기호 넣어줘야함.
}

int main(int argc,char* argv[])
{
    string input_txt;
    input_txt=argv[1];
    
	ifstream in(input_txt);
	if (in.is_open())
	{
		std::cout << "유효한 파일 이름입니다. Parser를 실행합니다.\n\n " << endl;
	}
	else
	{
		std::cout << "파일을 찾을 수 없습니다!" << std::endl;
		return 0;
	}

    
    int line=1;
	while (!in.eof()) //한줄씩 lexical analyser 수행
	{
        string input;
		getline(in, input);
        nowline=line;
        lexical_analyser lexical_analyser(input,line);
        lexical_analyser.add_token_table(input);
        line++;
	}
    string temp=input_txt;
    temp+="(lexical).out";
    ofstream lexical_output(temp);
    if (lexical_output.fail())
	{
		std::cerr << "Error!" << std::endl;
		return -1;
	}
    if(error==false) //에러가 없을 경우에 lexical analyser한 결과 output파일 생성
    {

        for (int i = 0; i < token.size(); i++)
        {
            cout << "<" << token[i].first << "," << token[i].second << ">\n";
            lexical_output << "<" << token[i].first << "," << token[i].second << ">\n";
        }
    }
    else //에러가 있을경우 error report를 output파일로 생성
    {
        lexical_output<<error_string;
    }

    if(error==false) //lexical analyser가 정상적으로 실행되었으면 Syntax analyzer 실행
    {
        cout<<"Lexical OK.\n";
        syntax_analyzer syntax_analyzer;
        /*****************************************************/
        //action테이블 설정
        in.close();
        in.clear();//action테이블 값이 있는 파일을 읽어오기 위하여 close,clear한뒤 ifstream을(여기선 in) 재사용함.
        in.open("action.txt");
        string temp;
        getline(in,temp);
        syntax_analyzer.set_terminal(temp);
        int row=0;
        while (!in.eof()) //한줄씩 action 테이블에 넣음
        {
            string input;
            getline(in,input);
            syntax_analyzer.set_action_table(input,row);
            row++;
        }
        /*****************************************************/

        /*****************************************************/
        in.close();
        in.clear();//goto테이블 값이 있는 파일을 읽어오기 위하여 close,clear한뒤 ifstream을(여기선 in) 재사용함.
        in.open("goto.txt");
        string temp2;
        getline(in,temp2);
        syntax_analyzer.set_nonterminal(temp2);
        int row2=0;
        while (!in.eof()) //한줄씩 action 테이블에 넣음
        {
            string input;
            getline(in,input);
            syntax_analyzer.set_goto_table(input,row2);
            row2++;
        }
        /*****************************************************/
        set_tokens_for_syntax();
        syntax_analyzer.check_syntax();  
        string input2=input_txt;
        input2+="(syntax).out";
        ofstream syntax_output(input2);
        syntax_output<<syntax_error_string;
    }
}