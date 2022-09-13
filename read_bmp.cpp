/*
读入bmp文件: 按字节读取
读取的顺序就是存储的顺序, 高位对应前面, 低位对应后面
*/

#include <iostream>
#include <vector>
#include <string>

using namespace std;

int main()
{
    vector<string> msg {"Hello", "C++", "World", "from", "VS Code", "and the C++ extension!"};
    
    for (const string& word : msg)
    {
        cout << word << " ";
    }
    cout << endl;
}