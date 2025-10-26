#include <iostream>
#include <string>
#include <map>
#include <cctype>
#include <cstdio>

using namespace std;
int n;
string a , b;
map<string , int> q;
int read()
{
	int asd = 0 , qwe = 1; char zxc;
	while(!isdigit(zxc = getchar())) if(zxc = '-') qwe = -1;
	while(isdigit(zxc)) asd = asd * 10 + (zxc - '0') , zxc = getchar();
	return asd * qwe;
}

int main()
{
	n = read();
	for(int i = 1;i <= n;i++)
	{
		cin >> a >> b;
		int x = 0 , y = 0 , flag = 1 , z = b.length();
		for(int j = 1;j <= 5;j++ , x = 0)
		{
			if(!isdigit(b[y])) { flag = 0; break; }
			if(b[y] == '0' && (y + 1 < z && isdigit(b[y + 1]))) { flag = 0; break; }
			for(y;y < z && isdigit(b[y]);y++)
			{
				x = x * 10 + (int)(b[y] - '0');
				if(j <= 4 && x > 255)
                {
                    flag = 0; break;
                }
				if(j == 5 && x > 65535)
                {
                    flag = 0; break;
                }
			}
			if(j < 4 && b[y] != '.') {
                flag = 0; break;
                }
			if(j == 4 && b[y] != ':') 
            {
                flag = 0; break;
            }
			if(j != 5) y++;
		}
		if(flag == 0 || y != z)
		{
			cout << "ERR" << endl;
			continue;
		}
		if(a[0] == 'S')
		{
			if(q[b] == 0) q[b] = i , cout << "OK" << endl;
			else cout << "FAIL" << endl;
		}
		else
		{
			if(q[b] != 0) cout << q[b] << endl;
			else cout << "FAIL" << endl;
		}
	}
	return 0;
}
