#include <iostream>
#include <string>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>
#include "functions.h"
#include "erori.h"

#define MAX_CLIENTS 5
#define BUFLEN 256

using namespace std;

void error(const char* msg){
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]){
	
	int i, j, n, m, N;
	vector<Client> clienti;
	vector<int> bancomate;
	int nrBancomate = 0;
	
	if (argc < 3) {
         	cout<<"Usage : "<<argv[0]<<" <port> <users_data_file>"<<endl;
         	exit(1);
     	}
     
	FILE *f = fopen(argv[2], "r");

	char nume[13];
	char prenume[13];
	char numar_card[7];
	int pin;
	char parola_secreta[9];
	double sold;
	
	fscanf(f, "%d", &N);
	
	for(i = 0; i < N; i++){	 	
	 	fscanf(f, "%s %s %s %d %s %lf", nume, prenume, numar_card, &pin, parola_secreta, &sold);
	 	Client clientaux(nume, prenume, numar_card, pin, parola_secreta, sold);
	 	clienti.push_back(clientaux);	
	}
	
	int sockfdUDP, sockfdTCP, newsockfd, portno, clilen;
        char buffer[BUFLEN];
        struct sockaddr_in serv_addr, cli_addr;
        
        fd_set read_fds; //multimea de citire folosita in select()
        fd_set tmp_fds;	//multime folosita temporar 
        int fdmax; //valoare maxima file descriptor din multimea read_fds
        
        //golim multimea de descriptori de citire (read_fds) si multimea tmp_fds        
     	FD_ZERO(&tmp_fds); 
     	FD_ZERO(&read_fds);
     	
     	portno = atoi(argv[1]);
     	
     	sockfdTCP = socket(AF_INET, SOCK_STREAM, 0);
    	if (sockfdTCP < 0) 
        	error("-10 : Eroare la opening socket");

     	memset((char *) &serv_addr, 0, sizeof(serv_addr));
     	serv_addr.sin_addr.s_addr = INADDR_ANY; // foloseste adresa IP a masinii     	
     	serv_addr.sin_port = htons(portno);
     	serv_addr.sin_family = AF_INET;
              
     	if (bind(sockfdTCP, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
              error("-10 : Eroare la binding");
     
     	listen(sockfdTCP, MAX_CLIENTS);
     
     	//adaugam noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
     	FD_SET(sockfdTCP, &read_fds);
     	FD_SET(0, &read_fds);

     	sockfdUDP = socket(AF_INET, SOCK_DGRAM, 0);
     	if (sockfdUDP < 0) 
        	error("-10 : Eroare la opening socket");
     
     	if (bind(sockfdUDP, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
              error("-10 : Eroare la binding");              
             
     	FD_SET(sockfdUDP, &read_fds);     	
     	fdmax = sockfdUDP;
     	
     	char block[9];
     	int aux;
     	int aux2;
     	 	
     	while (1) {
		tmp_fds = read_fds; 
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) 
			error("-10 : Eroare la select");
	        
		for(i = 0; i <= fdmax; i++) {                      
			if (FD_ISSET(i, &tmp_fds)) {				
				if(i == 0){
                               		//citesc din server comanda quit    
					memset(buffer, 0 , BUFLEN);
					cin>>buffer;
                               		if(strncmp(buffer, "quit", 4) == 0){
                                   		for(j = 0; j < nrBancomate; j++){                                   		
                                        		FD_CLR(j, &read_fds); // scoatem din multimea de citire socketul
                                        		close(j);
                                   		}
                                   		close(sockfdUDP);
                                   		close(sockfdTCP);
                                   		exit(0);
                               		}
                            	}	
				
				//serverul UDP
				if (i == sockfdUDP) {
					memset(buffer, 0, BUFLEN);
					socklen_t clientLenght = sizeof(cli_addr);
					//primesc comanda unlock <numar card> 
					n = recvfrom(sockfdUDP, buffer, BUFLEN, 0, (struct sockaddr *)&cli_addr, &clientLenght);                            
                               		if(n < 0)
                                   		error("-10 : Eroare la apel Receive Unlock");
                                   	
                                   	if(strncmp(buffer, "unlock", 6) == 0){
                                   		memset(numar_card, 0 , sizeof(numar_card));                                                	
     						vector<char*> v;                                          	
                                        	Split(v, buffer);
                                        	strcpy(numar_card, v[1]);
                                        
                                        	memset(buffer, 0, sizeof(buffer));                               	
                                        	if(error4(numar_card, clienti)){
							memcpy(buffer, "-4: Numar card inexistent", BUFLEN);
						}
					
						if(strncmp(buffer, "-", 1) != 0 && error6(numar_card, clienti)){
                                                	memcpy(buffer, "-6: Operatie esuata", BUFLEN);
                                        	}
                                        	
                                        	if(strncmp(buffer, "-", 1) != 0){
                                  			strcpy(buffer, "Trimite parola secreta");
                                  		}
                               		}else{                   			
                                        	memset(numar_card, 0 , sizeof(numar_card));
                                        	memset(parola_secreta, 0 , sizeof(parola_secreta));
                                        	strncpy(numar_card, buffer, 6);
                                        	strcpy(parola_secreta, buffer + 7);
                               			
                                        	memset(buffer, 0, sizeof(buffer));                               	
                                        	if(error7(numar_card, parola_secreta, clienti)){
						   memcpy(buffer, "-7: Deblocare esuata", BUFLEN);
						}
						
						if(strncmp(buffer, "-", 1) != 0){
                                  			strcpy(buffer, "Card deblocat");
                                  		}
                                        }
                                        
                                        //trimit mesaj inapoi la client
                                        m = sendto(sockfdUDP, buffer, strlen(buffer), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
                               		if(m < 0){
			          		error("-10 : Eroare la Send Unlock");
                               		}                 
                               		break;
				}
				
				//serverul TCP
				if (i == sockfdTCP) {
					// a venit ceva pe socketul inactiv (listen) adica, o noua conexiune
					// actiunea serverului(accept())
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfdTCP, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
					
					if (newsockfd == -1) {
						error("-10 : Eroare la apel accept");                                       
					} else {
						//adaug noul socket intors de accept() la multimea descriptorilor de citire						
						nrBancomate++;
                                                bancomate.push_back(newsockfd);
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) { 
							fdmax = newsockfd;
						}
                                                cout<<"Numarul de bancomate : "<<nrBancomate<<endl;
					}
                                        cout<<"Avem o noua conexiune de la "<<inet_ntoa(cli_addr.sin_addr)<<endl;
                                        cout<<"Port "<<ntohs(cli_addr.sin_port)<<endl;
                                        cout<<"Socket_client "<<newsockfd<<endl;
				} else {
					//am primit date pe unul din socketii cu care vorbesc cu clientii
					//actiunea serverului: recv()
					memset(buffer, 0, BUFLEN);
					n = recv(i, buffer, sizeof(buffer), 0);
					if (n <= 0) {
						if (n == 0) {
							//conexiunea s-a inchis
                                                        cout<<"server: socket "<<i<<" a iesit"<<endl;                                                        
							bancomate[i] = -1;
						} else {
							error("-10 : Eroare la apel recv");
						}
						FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul
						close(i); 
					} else {//recv intoarce >0
                                                cout<<"Am primit de la clientul de pe socketul "<<i<<" mesajul: "<<buffer<<endl;
                                                
                                                if(strncmp(buffer,"login",5) == 0){                                                
     							vector<char*> v;
                                                	memset(numar_card, 0 , sizeof(numar_card));
                                                	memset(block, 0,  sizeof(block));                                                	
                                                	Split(v, buffer);
                                                	
                                                	strcpy(numar_card, v[1]);
                                                	pin = atoi(v[2]);
                                                	strcpy(block, v[3]);
                                                	 
                                                	memset(buffer, 0, sizeof(buffer));                               	
                                                	if(error4(numar_card, clienti)){
								memcpy(buffer, "-4: Numar card inexistent", BUFLEN);
							}
                                                	
                                                	if(strncmp(buffer, "-", 1) != 0 && error2(numar_card, clienti)){
                                                		memcpy(buffer, "-2: Sesiune deja deschisa", BUFLEN);
                                                	}
                                                	
                                                	if(strncmp(buffer, "-", 1) != 0 && error5(numar_card, block, clienti)){
                                                		memcpy(buffer, "-5: Card blocat", BUFLEN);
                                                	}
                                                	
                                                	if(strncmp(buffer, "-", 1) != 0 && error3(numar_card, pin, clienti)){
                                                		memcpy(buffer, "-3: Pin gresit", BUFLEN);
                                                	}
                                                	
                                                	if(strncmp(buffer, "-", 1) != 0){                                                		
                                                		aux = searchUser(numar_card, clienti);
                                                		clienti[aux].logat = 1; //clientul s a logat
                                                		strcpy(buffer, "Welcome");
                                                		strcat(buffer, " ");
                                                      		strcat(buffer, clienti[aux].nume);
                                                      		strcat(buffer, " ");
                                                      		strcat(buffer, "");
                                                      		strcat(buffer, clienti[aux].prenume);                                                      		
                                                	}            	
                                                }
                                                
                                                if(strncmp(buffer,"logout",6) == 0){     
                                                	memset(numar_card, 0 , sizeof(numar_card));                                                	
     							vector<char*> v;                                          	
                                                	Split(v, buffer);
                                                	strcpy(numar_card, v[1]);   
                                                	aux = searchUser(numar_card, clienti);
                                                	clienti[aux].logat = 0; //clientul s a delogat
                                                	
                                                	memset(buffer, 0, sizeof(buffer));
                                                	memcpy(buffer, "Clientul a fost deconectat", BUFLEN);
                                                }
                                                
                                                if(strncmp(buffer, "listsold", 8) == 0){
                                                	memset(numar_card, 0 , sizeof(numar_card));                                                	
     							vector<char*> v;                                          	
                                                	Split(v, buffer);
                                                	strcpy(numar_card, v[1]);
                                                	aux = searchUser(numar_card, clienti);
                                                	
                                                	sold = clienti[aux].sold;
                                                	memset(buffer, 0, sizeof(buffer));
                                                	sprintf(buffer, "%0.2f", sold);
                                                }
                                                
                                                if(strncmp(buffer,"transfer",8) == 0){                                                	
     							vector<char*> v;                                          	
                                                	Split(v, buffer);
                                                	
                                                	memset(numar_card, 0 , sizeof(numar_card));
                                                	strcpy(numar_card, v[1]);
                                                	
                                                	aux = searchUser(v[3], clienti); //Clientul care transfera banii
                                                	aux2 = searchUser(v[1], clienti); //Clientul care primeste banii
                                                	sold = atof(v[2]);
                                                	char sm[30];
                                                   	sprintf(sm, "%0.2f", sold);
                                                	
                                                	memset(buffer, 0, sizeof(buffer));                               	
                                                	if(error4(numar_card, clienti)){
								memcpy(buffer, "-4: Numar card inexistent", BUFLEN);
							}
                                                	
                                                	if(strncmp(buffer, "-", 1) != 0 && error8(aux, sold, clienti)){
                                                		memcpy(buffer, "-8: Fonduri insuficiente", BUFLEN);
                                                	}                                                	
                                                	
                                                	if(strncmp(buffer, "-", 1) != 0){
                                                		strcpy(buffer, "Transfer");
                                                		strcat(buffer, " ");
                                                		strcat(buffer, sm);
                                                		strcat(buffer, " ");
                                                		strcat(buffer, "catre");
                                                		strcat(buffer, " ");
                                                        	strcat(buffer, clienti[aux2].nume);
                                                        	strcat(buffer, " ");
                                                        	strcat(buffer, "");
                                                        	strcat(buffer, clienti[aux2].prenume);
                                                        	strcat(buffer, "? [y/n]");
                                                	}
                                                }
                                                
                                                if(strncmp(buffer, "Raspuns:", 8) == 0){
                                                	if(strncmp(buffer + 9, "y", 1) == 0){
    		                                  		clienti[aux2].sold += sold;
                                                        	clienti[aux].sold -= sold;
                                                   	
                                                   		memset(buffer, 0, sizeof(buffer));
                                                   		strcpy(buffer, "Transfer realizat cu succes");
                                                	}else{    
                                                        	memset(buffer, 0, sizeof(buffer));                                                       
                                                   		strcpy(buffer, "-9 : Operatie anulata");
                                                        }
                                                }
                                                
                                                if(strncmp(buffer,"quit",4) == 0){                                                	
                                                   	FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul
                                                   	close(i);                                                   	
                                                   	nrBancomate--;  
                                                	bancomate.erase(remove(bancomate.begin(),bancomate.end(),i), bancomate.end()); 
                                                   	break;
                                                }
                                                
                                                m = send(i, buffer, strlen(buffer), 0);
                                                if(m < 0){
							error("-10 : Eroare la apel Send");
                                                }
                                        }
                                }
			
			}
		}
	}	

	return 0;
}
