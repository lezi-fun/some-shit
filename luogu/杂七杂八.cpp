#include <iostream>
#include <string>
using namespace std;
int main()
{
    int n;
    cin >> n;
    string s;
    cin >> s;
    n = n % 26;
    for (int i = 0 ; i < s.size(); i++)
    {
        if (s[i] >= 'a' && s[i] <= 'z')
        {
            cout <<(s[i] - 'a' + n) % 26 + 'a';
        }
    }
    return 0;
}
