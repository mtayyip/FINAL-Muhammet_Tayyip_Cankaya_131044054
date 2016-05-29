#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <stdint.h>
#include <inttypes.h> 
#include <pthread.h>
#include <dirent.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>


#define PORT 5260
#define LISTQUEUE 5
#define BUFSIZE 1024
#define MAX_RECV_BUF 1024
#define MAX_SEND_BUF 1024
#define MILISECONDS 1000
pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;


//Global Degiskenler
int soketsPid[LISTQUEUE];
char listServerMessage[BUFSIZE];
char lsClientsMessage[BUFSIZE];
int bagliOlanClientSayisi=0;
char *file_name="abc.gif";
int clients[LISTQUEUE];
int sockfd;




//Yardimci fonksiyonlarim
void *doChild(void *parg);
void *listServer(void *parg);
void *lsClients(void *parg);
void *sendFile(void *parg);
void Ctrlc_Sinyali(int signo); 





int main( int argc, char *argv[]) 
{

	int client_size,new_fd,j=0,parametre,num,yes=1,deger;
	struct sockaddr_in server_addr,client_addr;//server ve client bilgileri icin
	char message[BUFSIZE];   	
	pthread_t thread1;
	time_t basla,bit;
	double t0;

	//Bu kod blogu ders kitabi sayfa 326-Example 8.16 dan alinmistir.
	struct sigaction act;
	act.sa_handler = Ctrlc_Sinyali;
	act.sa_flags = 0;
	if ((sigemptyset(&act.sa_mask) == -1) ||
		(sigaction(SIGINT, &act, NULL) == -1))
			perror("Failed to set SIGINT to handle Ctrl-C");


	parametre=atoi(argv[1]);

	//SOKET OLUSTURMA.sockfd=dosya tanimlayici
	sockfd = socket(AF_INET,SOCK_STREAM,0);   
	if (sockfd < 0) {
		perror("Hata,socket olusturulamadi");
		exit(1);}
   
	
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;




    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)) ==-1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);} 



	//bind() fonksiyonu soketi bir adresle iliskilendirir.
	if (bind(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr))==-1) {
		perror("Hata,bind");
		exit(1);}
   

	printf("0 ms Server basladi.\n");


	//Client'larin kabul edilmesi için server dinlemeye gecmeli.Bunun için listen() fonksiyonu kullanır.   
	if (listen(sockfd,LISTQUEUE) == -1) {
		perror("Hata,listen");
		exit(1);}

	puts("Clients baglantisi icin bekleniliyor...\n");

	time (&basla);


	while (1) 
	{
		
		client_size=sizeof(struct sockaddr_in);
    	
		//Clients'in baglanti istegini accept() fonksiyonu ile yakaliyacagim. 
		new_fd=accept(sockfd,(struct sockaddr*)&client_addr,&client_size);
		
		if (new_fd<0){
			perror("Hata,accept");
			exit(1);}
		printf("Baglanti kabul edildi...\n");


		//Gelen haberlesme soketlerini soketsPid arayime aliyorum.Daha sonra kullanicam.
		soketsPid[j]=new_fd;
		j++;


		num=send(new_fd,"SERVER'A BAGLANDINIZ...",23,0);
		if (num< 0) {
			perror("ERROR writing to socket");
			exit(1);}


		num=recv(new_fd,message,BUFSIZE,0);
		if (num<0){
			perror("ERROR reading from socket");
			exit(1);}         


		time (&bit);
		t0 =MILISECONDS*(difftime(bit, basla));
		printf("%.0f ms Client %d baglandi.Baglanan clients pid degeri==>%s\n\n",t0,bagliOlanClientSayisi+1,message);	
		strcat(lsClientsMessage,message);		
		deger=atoi(message);
		clients[bagliOlanClientSayisi++]=deger;




		if(pthread_create(&thread1,NULL,doChild,(void*)&new_fd)){
			fprintf(stderr,"Cannot create thread!...\n");
			exit(1);}	
	} 

	return 0;
}



/*Ctrl-C sinyalini burada yakalayip socket i kapatiyorum ve clients lari kill ediyorum*/
void Ctrlc_Sinyali(int signo) 
{
	int i;

	for(i=0;i<bagliOlanClientSayisi;i++)
		kill(clients[i],9);

	close(sockfd);
	exit(1);
}



void* doChild(void *parg) 
{
   
	int sock=*(int*)parg,num,error; 
	char buffer[BUFSIZE],notAvailable[BUFSIZE];
	pthread_t thread2,thread3,thread4;
	void* preturn2,*preturn3,*preturn4;

	while(1)
	{
		//The bzero function places nbyte null bytes in the string s. 
		//This function is used to set all the socket structures with null values.
		bzero(buffer,BUFSIZE);


		num=recv(sock,buffer,BUFSIZE,0);
		if (num< 0) {
			perror("ERROR reading from socket");
			exit(1);}
   
		printf("Client'in gonderdigi mesaj:%s\n",buffer);  
      




		//Gelen mesaj listLocal parametresi ise
		if(strcmp(buffer,"listLocal")==0){
			num=send(sock,"",1,0);
			if (num< 0){
				perror("ERROR writing to socket");
				exit(1);}}



		//Gelen mesaj listServer parametresi ise
		else if(strcmp(buffer,"listServer")==0){
			pthread_mutex_lock(&my_mutex);
			if(pthread_create(&thread2,NULL,listServer,(void*)&sock)){
				fprintf(stderr,"Cannot create thread!...\n");
				exit(1);}
			if (error = pthread_join(thread2,&preturn2)){
				fprintf(stderr, "Failed to join thread: %s\n", strerror(error));
				exit(1);}
			pthread_mutex_unlock(&my_mutex); }



		//Gelen mesaj lsClients parametresi ise
		else if(strcmp(buffer,"lsClients")==0){
			pthread_mutex_lock(&my_mutex);
			if(pthread_create(&thread3,NULL,lsClients,(void*)&sock)){
				fprintf(stderr,"Cannot create thread!...\n");
				exit(1);}
			if (error = pthread_join(thread3,&preturn3)){
				fprintf(stderr, "Failed to join thread: %s\n", strerror(error));
				exit(1);}
			pthread_mutex_unlock(&my_mutex); }
      


		//Gelen mesaj sendFile parametresi ise
		else if(strcmp(buffer,"sendFile")==0){
			
			num=send(sock,"",1,0);
			if (num< 0){
				perror("ERROR writing to socket");
				exit(1);} 

			pthread_mutex_lock(&my_mutex);
			if(pthread_create(&thread4,NULL,sendFile,(void*)&sock)){
				fprintf(stderr,"Cannot create thread!...\n");
				exit(1);}
			if (error = pthread_join(thread4,&preturn4)){
				fprintf(stderr, "Failed to join thread: %s\n", strerror(error));
				exit(1);}		
			pthread_mutex_unlock(&my_mutex);}
            
  

		//Gelen mesaj help parametresi ise      
		else if(strcmp(buffer,"help")==0){
			num=send(sock,"",1,0);
			if (num< 0){
				perror("ERROR writing to socket");
				exit(1);}}



		//Gelen mesaj beklenen parametrelerden biri degilse
		else{
			num=send(sock,"",1,0);
			if (num< 0){
				perror("ERROR writing to socket");
				exit(1);}} 
	}	
}





void *listServer (void *parg)
{
   	
	int sock=*(int*)parg,num;
	struct dirent *direntp;
	struct stat status;    
	DIR *directory;
	char mycwd[PATH_MAX];
	strcpy(listServerMessage,"");

	if (getcwd(mycwd, PATH_MAX) == NULL) {
		perror("Failed to get current working directory");
		return 0;}
	/*directory nin NULL olma durumuna bakiyorum*/
	if ((directory = opendir(mycwd))== NULL) {
		fprintf(stderr,"Directory acilamadi veya bulunamadi.\n");
		exit(1);}
	/*Null degilse directory icini okumaya basliyorum*/    
	while ((direntp = readdir(directory)) != NULL) 
	{       
		/*directory olarak . ve .. dosyalari degilse islemleri yapiyorum.*/
		if (strcmp(direntp->d_name, ".")!= 0 && strcmp(direntp->d_name, "..")!= 0) 
 		{
			/*file durumuna bakiyorum.*/
			if (stat(mycwd, &status) == -1){
 				fprintf(stderr,"Failed to get file status.\n");
				break;}  
			if(direntp->d_type==DT_REG){  
				strcat(listServerMessage,direntp->d_name);
				strcat(listServerMessage,"  ");}         
		}   
	}
	/*directory i kapatma islemi.*/
	while ((closedir(directory) == -1) && (errno == EINTR));


	//Client'a klasorde bulunan dosyalari yaziyorum.
	num=send(sock,listServerMessage,strlen(listServerMessage),0);
	if (num< 0){
		perror("ERROR writing to socket");
		exit(1);}

	strcpy(listServerMessage,"");	        
}



void *lsClients(void *parg)
{
	int sock=*(int*)parg,i,j,num;

	//Client'a server'a bagli bulunan clientlari yaziyorum.
	num=send(sock,lsClientsMessage,strlen(lsClientsMessage),0);
	if (num< 0){
		perror("ERROR writing to socket");
		exit(1);}  
	
}




/*Asagida bulunan kodlar belirtilen yerden alinip kendi koduma gore uyarlanmistir.
	https://lms.ksu.edu.sa/bbcswebdav/users/mdahshan/Courses/CEN463/Course-Notes/07-file_transfer_ex.pdf
*/
void* sendFile(void *parg)
{

	int sock=*(int*)parg;


	char send_str [MAX_SEND_BUF]; /* message to be sent to server*/
	int f; /* file handle for receiving file*/
	ssize_t rcvd_bytes, rcvd_file_size;
	char recv_str[MAX_RECV_BUF]; /* buffer to hold received data */
	size_t send_strlen; /* length of transmitted string */
	char msg[BUFSIZE];


	sprintf(send_str, "%s\n", file_name); /* add CR/LF (new line) */
	send_strlen = strlen(send_str); /* length of message to be transmitted */
	if( send(sock, file_name, send_strlen, 0) < 0 ) {
		perror("send error");
		return 0;
		}
	

	recv(sock,msg,BUFSIZE,0);

	if(strcmp(msg,"Filenotfound")==0)
		return 0;

	/* attempt to create file to save received data. 0644 = rw-r--r-- */
	if ( (f = open(file_name, O_WRONLY|O_CREAT, 0644)) < 0 ){
		perror("error creating file");
		return 0;}

	rcvd_file_size = 0; /* size of received file */
	
	/* continue receiving until ? (data or close) */
	while ((rcvd_bytes = recv(sock, recv_str, MAX_RECV_BUF, 0))>0)
	{	
		rcvd_file_size += rcvd_bytes;
		
		if (write(f, recv_str, rcvd_bytes) < 0 ){
			perror("error writing to file");
			return 0;}

		if(rcvd_file_size==atoi(msg))
			return 0;

		strcpy(recv_str,"");
	}	
	close(f);/* close file*/
}