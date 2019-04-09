#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;


bool cmp_strings(string str1, string str2) {
	if (str1.size() != str2.size())
		return str1.size() < str2.size();
	return str1 < str2;
}

bool cmp_number_strings(string str1, string str2) {
	if (str1[0] == str2[0] && str1[0] == '-')
		return !cmp_strings(str1, str2);
	if (str1[0] == '-' || str2[0] == '-')
		return str1 < str2;
	return cmp_strings(str1, str2);
}


void read_numbers_from_file(ifstream &file, vector<string> &numbers) {
	string line;
	while(getline(file, line))
		for (int i = 0; i < line.size(); i++)
			if (isdigit(line[i])) {
				if (line[i] == '0') {
					numbers.push_back("0");
					continue;
				}
				string num;
				if (i > 0 && line[i - 1] == '-')
					num.push_back('-');
				while (i < line.size() && isdigit(line[i]))
					num.push_back(line[i++]);
				numbers.push_back(num);
			}
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		cout << "Invalid arguments" << endl;
		return 0;
	}
	
	vector<string> numbers;
	ifstream infile;
	for (int i = 1; i < argc - 1; i++)
	{
		infile.open(argv[i]);
		if(!infile.is_open()) {
			cout << "Error open" << endl;
			return 0;
	   	}
		read_numbers_from_file(infile, numbers);
		infile.close();
	}
	sort(numbers.begin(), numbers.end(), cmp_number_strings);
	
	ofstream out(argv[argc - 1]);
	if (!out.is_open()) {
		cout << "Cannot open file to write" << endl;
		return 0;
	}
	for (int i = 0; i < numbers.size(); i++) {
		out << numbers[i] << endl;
	}
	out.close();
}
