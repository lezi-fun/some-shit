#include <iostream>
#include <string>
using namespace std;
int main()
{
    int n;
    cin >> n;
    string s;
    cin >> s;
    // 确保n在0-25范围内，避免n过大导致的问题
    n = n % 26;
    for (int i = 0 ; i < s.size(); i++)
    {
        // 只处理小写字母，其他字符原样输出
        if (s[i] >= 'a' && s[i] <= 'z')
        {
            char shifted = (s[i] - 'a' + n) % 26 + 'a';
            cout << shifted;
        }
        else
        {
            cout << s[i];
        }
    }
    return 0;
}
