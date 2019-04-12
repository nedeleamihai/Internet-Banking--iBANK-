#include <iostream>
#include <iomanip>
#include <vector>
#include <stdio.h>
#include <string.h>

using namespace std;

class Client{
	public:
		char nume[12];
		char prenume[12];
		char numar_card[6];
		int pin;
		char parola_secreta[8];
		double sold;
		int blocat;
		int logat;
		
		Client(){
			strcpy(this->nume, "");
			strcpy(this->prenume, "");
			strcpy(this->numar_card, "");
			this->pin = 0;
			strcpy(this->parola_secreta, "");
			this->sold = 0;
			this->blocat = 0;
			this->logat = 0;
		}
		
		Client(char name[], char lastName[], char nrCard[], int newpin, char password[], double newsold){
			strcpy(this->nume, name);
			strcpy(this->prenume, lastName);
			strncpy(this->numar_card, nrCard, 6);			
			this->pin = newpin;
			strcpy(this->parola_secreta, password);
			this->sold = newsold;
			this->blocat = 0;
			this->logat = 0;
		}
};

//Se cauta userul respectiv dupa numarul sau de card
int searchUser(char NumarCard[], vector<Client> v){
	for(int i = 0; i < v.size(); i++){
         	if(strcmp(v[i].numar_card, NumarCard) == 0){
            		return i;
         	} 
     	}
     	
     	return -1;
}

//Verifica daca userul este logat pe alt client
int error2(char NumarCard[], vector<Client> v){
	int nr = searchUser(NumarCard, v);
	
	if(v[nr].logat == 1){
		return 1;
	}
	return 0;
}

//Verifica daca userul a introdus codul PIN corect.
int error3(char NumarCard[], int pin, vector<Client> v){
	int nr = searchUser(NumarCard, v);
	
	if(v[nr].pin != pin){
		return 1;
	}
	return 0;
}

//Verifica daca exista numarul de card al userului care incearca sa se logheze
int error4(char NumarCard[], vector<Client> v){
	int nr = searchUser(NumarCard, v);
	
	if(nr == -1){
		return 1;
	}
	return 0;
}

//Verifica daca cardul userului care incearca sa se logheze este blocat
int error5(char NumarCard[], char block[], vector<Client> &v){
	int nr = searchUser(NumarCard, v);
	
	if(strcmp(block, "blocat") == 0 || v[nr].blocat == 1){
		v[nr].blocat = 1;
		return 1;
	}
	return 0;
}

//Verifica daca cardul userului care incearca sa se logheze nu este blocat
int error6(char NumarCard[], vector<Client> v){
	int nr = searchUser(NumarCard, v);
	
	if(v[nr].blocat == 0){
		return 1;
	}
	return 0;
}

//Verifica daca s-a introdus corect parola secreta
int error7(char NumarCard[], char ParolaSecreta[], vector<Client> &v){
	int nr = searchUser(NumarCard, v);
	
	if(strcmp(v[nr].parola_secreta, ParolaSecreta) != 0){
		return 1;
	}
	v[nr].blocat = 0;
	return 0;
}

//Verifica daca userul respectiva are suficiente fonduri
int error8(int ind, int sold, vector<Client> v){	
	if(v[ind].sold < sold){
		return 1;
	}
	return 0;
}
