#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string>
#include "functions.h"

#define BUFLEN 256

using namespace std;

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[]){

	if (argc < 3) {
		cout<<"Usage "<<argv[0]<<" server_address server_port"<<endl;
		exit(0);
	}

	char buffer[BUFLEN];
	int n, m, i, sockfdTCP, sockfdUDP;
	struct sockaddr_in serv_addr;
	struct hostent *server;
		
	sockfdTCP = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfdTCP < 0) 
		error("-10 : Eroare la apel opening socket");

	serv_addr.sin_port = htons(atoi(argv[2]));
	serv_addr.sin_family = AF_INET;
	inet_aton(argv[1], &serv_addr.sin_addr);

	if (connect(sockfdTCP, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
		error("-10 : Eroare la apel connecting");    

	int fdmax;     //valoare maxima file descriptor din multimea read_fds	
	fd_set tmp_fds;    //multime folosita temporar 
	fd_set read_fds;  //multimea de citire folosita in select()

	//golim multimea de descriptori de citire (read_fds) si multimea tmp_fds 
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	FD_SET(0, &read_fds );
	FD_SET(sockfdTCP, &read_fds);

	fdmax = sockfdTCP;	

	sockfdUDP = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfdUDP < 0) 
		error("-10 : Eroare la apel opening socket");

	//Se creeaza numele fisierului client .log
	char fileClient[30];
	int PID = getpid();

	memset(fileClient, 0, sizeof(fileClient));
	strcpy(fileClient, "client-");
	strcat(fileClient, number_to_char(PID));
	strcat(fileClient,".log");

	//Se deschide fisierul .log
	FILE *fp = fopen(fileClient, "w");

	int sesiuneDeschisa = 0;
	int nrIncercari = 1;
	char NumarCard[7] = "000000";	
	char nc[7];
	char pi[5];

	while(1){
		tmp_fds = read_fds; 
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) 
			error("-10 : Eroare la apel select");

		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {				   
				memset(buffer, 0 , BUFLEN);  
				if(i == 0){

					//citesc din client comanda 
					fgets(buffer, BUFLEN-1, stdin);
					fprintf(fp,"%s\n",buffer);
					
					//Afiseaza codul de eroare -1 daca nu e logat niciun user
					if((strncmp(buffer, "logout", 6) == 0 || strncmp(buffer, "listsold", 8) == 0 ||
					    strncmp(buffer, "transfer", 8) == 0) && sesiuneDeschisa == 0){
					    
						fprintf(fp, "IBANK> -1 : Clientul nu este autentificat\n");
						cout<<"IBANK> -1 : Clientul nu este autentificat"<<endl;
						break;
					}
					
					if(strncmp(buffer, "login", 5) == 0){
						if(sesiuneDeschisa == 0){
							memset(nc, 0 , sizeof(nc));
							memset(pi, 0 , sizeof(pi));							
							strcpy(pi, buffer + 13);
							strncpy(nc, buffer + 6, 6);
							
							memset(buffer, 0 , BUFLEN);
							strcpy(buffer, "login ");
							strcat(buffer, nc);
							strcat(buffer, " ");
							strcat(buffer, pi);
							
							if(strncmp(nc, NumarCard, 6) != 0){
								nrIncercari = 1;
								strncpy(NumarCard, nc, 6);
							}
							
							if(nrIncercari == 3){
								strcat(buffer, " blocat");
								nrIncercari = 1;
							}else{
								strcat(buffer, " neblocat");
							}
								
							n = send(sockfdTCP, buffer, strlen(buffer), 0);
							if (n < 0) 
								error("-10 : Eroare la apel Send");
							
							memset(buffer, 0 , BUFLEN);
							m = recv(sockfdTCP, buffer, sizeof(buffer), 0);	
							if (m<0)
								error("-10 : Eroare la apel Receive");
								
							//afiseaza mesajul de la server Welcome sau eroare
                    					fprintf(fp, "IBANK> %s\n", buffer);
                    					cout<<"IBANK> "<<buffer<<endl;
                    					
                    					if(strncmp(buffer, "-3", 2) == 0){
                    						nrIncercari++;
                    						memset(NumarCard, 0, sizeof(NumarCard));
                    						strncpy(NumarCard, nc, 6);
                    					}
                    					
                    					if(strncmp(buffer, "-5", 2) == 0 || strncmp(buffer, "-4", 2) == 0){
                    						memset(NumarCard, 0, sizeof(NumarCard));
                    						strncpy(NumarCard, nc, 6);
                    					}
                    					
                    					if(strncmp(buffer, "Welcome", 7) == 0){
                    						sesiuneDeschisa = 1;
                    						nrIncercari = 1;
                    						memset(NumarCard, 0, sizeof(NumarCard));
                    						strncpy(NumarCard, nc, 6);
                    					}
						}else{                    		
							fprintf(fp, "IBANK>-2 : Sesiune deja deschisa\n");
							cout<<"IBANK>-2 : Sesiune deja deschisa"<<endl;
						}
						break;
					}
					
					if(strncmp(buffer, "logout", 6) == 0){	
						sesiuneDeschisa = 0;
						memset(buffer, 0 , BUFLEN);
						strcpy(buffer, "logout ");
						strcat(buffer, NumarCard);
                    					
						n = send(sockfdTCP, buffer, strlen(buffer), 0);
						if (n < 0) 
							error("-10 : Eroare la apel Send");
							
						memset(buffer, 0 , BUFLEN);
						m = recv(sockfdTCP, buffer, sizeof(buffer), 0);	
						if (m<0)
							error("-10 : Eroare la apel Receive");
								
						//afiseaza mesajul Clientul a fost deconectat                    				
                    				fprintf(fp, "IBANK> %s\n", buffer);
                    				cout<<"IBANK> "<<buffer<<endl;
						break;
					}
					
					if(strncmp(buffer, "listsold", 8) == 0){
						memset(buffer, 0 , BUFLEN);
						strcpy(buffer, "listsold ");
						strcat(buffer, NumarCard);
						
						n = send(sockfdTCP, buffer, strlen(buffer), 0);
						if (n < 0) 
							error("-10 : Eroare la apel Send");
							
						memset(buffer, 0 , BUFLEN);
						m = recv(sockfdTCP, buffer, sizeof(buffer), 0);	
						if (m<0)
							error("-10 : Eroare la apel Receive");
							
						//afiseaza soldul Clientului
                    				fprintf(fp, "IBANK> %s\n", buffer);
                    				cout<<"IBANK> "<<buffer<<endl;
                    				break;
					}
					
					if(strncmp(buffer,"quit",4) == 0){
						memset(buffer, 0 , BUFLEN);
						strcpy(buffer, "quit");
						
						n = send(sockfdTCP, buffer, strlen(buffer), 0);
						if (n < 0) 
							error("-10 : Eroare la apel quit");
						
						close(sockfdTCP);
                        			close(sockfdUDP);  
                        			return 0;
					}
					
					if(strncmp(buffer, "transfer", 8) == 0){
						char nc[7];
						char su[5];
						strncpy(nc, buffer + 9, 6);
						strcpy(su, buffer + 16);
							
						memset(buffer, 0 , BUFLEN);
						strcpy(buffer, "transfer ");
						strcat(buffer, nc);
						strcat(buffer, " ");
						strcat(buffer, su);
						strcat(buffer, " ");
						strcat(buffer, NumarCard);
						
						n = send(sockfdTCP, buffer, strlen(buffer), 0);
						if (n < 0) 
							error("-10 : Eroare la apel Send");
							
						memset(buffer, 0 , BUFLEN);
						m = recv(sockfdTCP, buffer, sizeof(buffer), 0);	
						if (m<0)
							error("-10 : Eroare la apel Receive");
							
						//afiseaza comanda Transfer <suma> sau eroare						
                    				fprintf(fp,"IBANK> %s\n",buffer);
                    				cout<<"IBANK> "<<buffer<<endl;
                    				
                    				if(strncmp(buffer, "-", 1) != 0){
                    					//citesc din client comanda [y/n]  
							memset(buffer, 0 , BUFLEN);
							cin>>buffer;
							
							char rs[3];
							strncpy(rs, buffer, 1);
							memset(buffer, 0 , BUFLEN);
							strcpy(buffer, "Raspuns: ");
							strcat(buffer, rs);
							
							n = send(sockfdTCP, buffer, strlen(buffer), 0);
							if (n < 0) 
								error("-10 : Eroare la apel Send");
							
							memset(buffer, 0 , BUFLEN);
							m = recv(sockfdTCP, buffer, sizeof(buffer), 0);	
							if (m<0)
								error("-10 : Eroare la apel Receive");
								
							//afiseaza Transfer realizat cu succes sau eroare -9
                    					fprintf(fp, "IBANK> %s\n", buffer);
                    					cout<<"IBANK> "<<buffer<<endl;
                    				}                    				
                    				break;
					}
					
					if(strncmp(buffer,"unlock",6) == 0){
                        			memset(buffer, 0, sizeof(buffer));
                        			strcpy(buffer, "unlock");
                        			strcat(buffer, " ");
                        			strcat(buffer, NumarCard);
                        			
                        			//trimit la server udp comanda unlock <numar card>
                        			n = sendto(sockfdUDP, buffer, strlen(buffer), 0, (struct sockaddr *)&serv_addr,sizeof(serv_addr));
                        			if(n < 0)
			    				error("-10 : Eroare la apel Send Unlock");
			    			
			    			socklen_t serverLenght = sizeof(serv_addr);
			    			memset(buffer, 0 , BUFLEN);
			    			
                        			//primesc mesajul <Trimite parola secreta> sau eroare 
                        			m = recvfrom(sockfdUDP, buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, &serverLenght); 
                        			if(m < 0){
                            				error("-10 : Eroare la Receive Unlock");
                        			}
                                                
                        			fprintf(fp,"UNLOCK> %s\n",buffer);
                        			cout<<"UNLOCK> "<<buffer<<endl;
                        			
                        			if(strncmp(buffer,"-",1) != 0){
                        				//citesc din client parola                        				   
							memset(buffer, 0 , BUFLEN);							
							char ps[9];
							
							cin>>buffer;						
							fprintf(fp, "%s\n", buffer);
							
							memset(ps, 0 , sizeof(ps));
							strcpy(ps, buffer);
							
							memset(buffer, 0 , BUFLEN);
							strcpy(buffer, NumarCard);
							strcat(buffer, " ");
							strcat(buffer, ps);
							
							//trimit la server udp comanda <numar card> <secret password>
                        				n = sendto(sockfdUDP, buffer, strlen(buffer), 0, (struct sockaddr *)&serv_addr,sizeof(serv_addr));
                        				if(n < 0)
			    					error("-10 : Eroare la apel Send Unlock");
			    			
			    				memset(buffer, 0 , BUFLEN);
			    			
                        				//primesc mesajul Card deblocat sau eroare 
                        				m = recvfrom(sockfdUDP, buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, &serverLenght); 
                        				if(m < 0){
                            					error("-10 : Eroare la Receive Unlock");
                        				}
                        				
                        				fprintf(fp,"UNLOCK> %s\n",buffer);
                        				cout<<"UNLOCK> "<<buffer<<endl;							
                        			}
                        			break;
                        		}			    			
				}
			}
		}
	}
	
	fclose(fp);
	return 0;
}

