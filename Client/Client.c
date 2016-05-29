#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>


#define PORT 5260
#define BUFSIZE 1024
#define MAX_RECV_BUF 1024
#define MAX_SEND_BUF 1024
pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;



//Global Degiskenler
int sockfd;
char listLocalMessage[BUFSIZE];
char helpMessage[BUFSIZE];
char *file_name="abc.gif";



//Yardimci fonksiyonlarim 
void *listLocal (void *parg);
void *help(void *parg);
void *send_file(void *parg);
void Ctrlc_Sinyali(int signo);



int main(int argc, char *argv[]) 
{
	int num,error;
	struct sockaddr_in server_addr;
	struct hostent *serverIPadress;  
	char buffer[BUFSIZE],send_message[BUFSIZE],recv_message[BUFSIZE],mycwd[PATH_MAX],message[BUFSIZE];
	pthread_t thread2,thread3,thread4;
	void* preturn2,*preturn3,*preturn4;   


	if (getcwd(mycwd, PATH_MAX) == NULL) {
		perror("Failed to get current working directory");
		return 0;}

	//Bu kod blogu ders kitabi sayfa 326-Example 8.16 dan alinmistir.
	struct sigaction act;
	act.sa_handler = Ctrlc_Sinyali;
	act.sa_flags = 0;
	if ((sigemptyset(&act.sa_mask) == -1) ||
		(sigaction(SIGINT, &act, NULL) == -1))
			perror("Failed to set SIGINT to handle Ctrl-C");


	serverIPadress=gethostbyname(argv[1]);
   

	//SOKET OLUSTURMA.sockfd=dosya tanimlayici
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
	if (sockfd < 0) {
		perror("Hata,socket olusturulamadi");
		exit(1);}
	   
	if (serverIPadress == NULL) {
		fprintf(stderr,"Hata,baglanilacak server adresi hatasi.\n");
		exit(0);}
   

	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	//baglanilacak serverin ip adresini server_addr icine aliyor.
	server_addr.sin_addr = *((struct in_addr *)serverIPadress->h_addr);  
   	


	//Servera baglanma islemi
	if (connect(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr)) < 0) {
		perror("Hata,connecting.\nBaglanlacak server bulunamadi");
		exit(1);}
   

	num=recv(sockfd,recv_message,23,0);  	
	if(num< 0) {
		perror("ERROR reading from socket");
		exit(1);}	   	
	printf("%s\n",recv_message); 


	snprintf(send_message,BUFSIZE,"%d   ",(int)getpid());
	printf("getpid()==>%s\n\n", send_message);
	num=send(sockfd,send_message,strlen(send_message),0);
	if (num< 0) {
		perror("ERROR writing to socket");
		exit(1);}



	while(1)
	{
    		
		printf("Lutfen mesajinizi giriniz:");
		scanf("%s",buffer);
   


		//Kendi icindeki dosyalari ekrana veriyor.
		if(strcmp(buffer,"listLocal")==0){   			
			num=send(sockfd,buffer,strlen(buffer),0);
			if (num< 0){
				perror("ERROR writing to socket");
				exit(1);}

			pthread_mutex_lock(&my_mutex);				
			if(pthread_create(&thread2,NULL,listLocal,(void*)mycwd)){
				fprintf(stderr,"Cannot create thread!...\n");
				exit(1);}
			if (error = pthread_join(thread2,&preturn2)){
				fprintf(stderr, "Failed to join thread: %s\n", strerror(error));
				exit(1);}		
			pthread_mutex_unlock(&my_mutex); 
			
			strcpy(listLocalMessage,"");}



		/*Server'a mesaj gonderiyorum*/
		else if(strcmp(buffer,"listServer")==0){
			num = send(sockfd,buffer,strlen(buffer),0); 
			if (num < 0) {
				perror("ERROR writing to socket");
				exit(1);}}



		/*Server'a mesaj gonderiyorum*/
		else if(strcmp(buffer,"lsClients")==0){
			num = send(sockfd,buffer,strlen(buffer),0); 
			if (num < 0) {
				perror("ERROR writing to socket");
				exit(1);}}



		/*Server'a mesaj gonderiyorum*/
		else if(strcmp(buffer,"sendFile")==0){
			num = send(sockfd,buffer,strlen(buffer),0); 
			if (num < 0) {
				perror("ERROR writing to socket");
				exit(1);}

			num= recv(sockfd,message,BUFSIZE,0);  
			if (num < 0) {
				perror("ERROR reading from socket");
				exit(1);}
			message[num]='\0';
			strcpy(message,"");

			pthread_mutex_lock(&my_mutex);
			if(pthread_create(&thread4,NULL,send_file,(void*)&sockfd)){
				fprintf(stderr,"Cannot create thread!...\n");
				exit(1);}
			if (error = pthread_join(thread4,&preturn4)){
				fprintf(stderr, "Failed to join thread: %s\n", strerror(error));
				exit(1);}			
			pthread_mutex_unlock(&my_mutex);}



		//komutlarin ne ise yaradigi hakkinda bilgi veriyor.
		else if(strcmp(buffer,"help")==0){   			
			num=send(sockfd,buffer,strlen(buffer),0);
			if (num< 0){
				perror("ERROR writing to socket");
				exit(1);} 
	
			pthread_mutex_lock(&my_mutex);
			if(pthread_create(&thread3,NULL,help,(void*)mycwd)){
				fprintf(stderr,"Cannot create thread!...\n");
				exit(1);}
			if (error = pthread_join(thread3,&preturn3)){
				fprintf(stderr, "Failed to join thread: %s\n", strerror(error));
				exit(1);}			
			pthread_mutex_unlock(&my_mutex); 
			
			strcpy(listLocalMessage,"");}		   		   		



		/*Gelen herhangi bir komutu da Server'a mesaj gonderiyorum*/
		else{
			num = send(sockfd,buffer,strlen(buffer),0); 
			if (num < 0) {
				perror("ERROR writing to socket");
				exit(1);}}   			
   		


		num= read(sockfd,message,BUFSIZE);  
		if (num < 0) {
			perror("ERROR reading from socket");
			exit(1);}
		message[num]='\0';

		printf("%s\n\n",message);
		strcpy(message,"");
	}

	return 0;
}



/*Ctrl-C sinyalini burada yakalayip socket i kapatiyorum*/
void Ctrlc_Sinyali(int signo) 
{
	close(sockfd);
	exit(1);
}



void *listLocal (void *parg)
{
	
	int num;
	struct dirent *direntp;
	struct stat status;    
	DIR *directory;
	char *mycwd=(char*)parg;

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
				strcat(listLocalMessage,direntp->d_name);
				strcat(listLocalMessage,"  ");}         
		}   
	}

	/*directory i kapatma islemi.*/
	while ((closedir(directory) == -1) && (errno == EINTR));  
	printf("%s\n",listLocalMessage);  	 
}



void *help(void *parg)
{
	strcat(helpMessage,"listLocal===>to list the local files in the directory client program started.\n\n");
	strcat(helpMessage,"listServer==>to list the files in the current scope of the server-client\n\n");
	strcat(helpMessage,"lsClients===>lists the clients currently connected to the server with their respective clientids\n\n");
	strcat(helpMessage,"sendFile====> <filename> <clientid> send the file <filename>(if file exists) from ");
	strcat(helpMessage,"local directory to\nthe client with client id clientid. If no client id is given the file is send to the servers local directory."); 
	printf("%s\n",helpMessage);
	strcpy(helpMessage,"");        
}



/*Asagida bulunan kodlar belirtilen yerden alinip kendi koduma gore uyarlanmistir.
	https://lms.ksu.edu.sa/bbcswebdav/users/mdahshan/Courses/CEN463/Course-Notes/07-file_transfer_ex.pdf
*/
void *send_file(void *parg)
{

	int sock=*(int*)parg,count=0;;
	ssize_t read_bytes,sent_bytes;
	char send_buf[MAX_SEND_BUF],message[BUFSIZE]; /* max chunk size for sending file */
	int f,sent_file_size = 0; /* file handle for reading local file*/
	char ch;
	FILE *ftr=fopen(file_name,"r");


	/* attempt to open requested file for reading */
	if( (f = open(file_name, O_RDONLY)) < 0) /* can't open requested file */{
		
		perror(file_name);
		if( (sent_bytes = send(sock,"Filenotfound",12, 0)) < 0 ){
			perror("send error");
			return 0;}}

	else{/* open file successful */	
		while(!feof(ftr)){
			ch=getc(ftr);
			count++;}

		rewind(ftr);
		count=count-1;
		snprintf(message,BUFSIZE,"%d",count);
		send(sock,message,strlen(message),0);
		recv(sock,message,strlen(message),0);

		while( (read_bytes = read(f, send_buf, MAX_RECV_BUF)) > 0 ){
			if( (sent_bytes = send(sock, send_buf, read_bytes, 0))< read_bytes ){
				perror("send error");
				return 0;}
			sent_file_size += sent_bytes;}
		close(f);}
}