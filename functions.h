#include <iostream>
#include <iomanip>
#include <vector>
#include<stdio.h>
#include<string.h>

using namespace std;

char* number_to_char (int number){
	    char* Str = new char[20];
	    sprintf(Str, "%d", number);
	    
	    return Str;
}

void Split(vector<char*> &v, char* buffer){
    
    char *token = strtok(buffer, " ");
    
    while (token != NULL) {
        v.push_back(token);
        token = strtok(NULL, " ");
    }
}

