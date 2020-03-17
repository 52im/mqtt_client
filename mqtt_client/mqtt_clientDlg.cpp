
// mqtt_clientDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "mqtt_client.h"
#include "mqtt_clientDlg.h"
#include "afxdialogex.h"
#include "libemqtt.h"
#include "Dialg_Def.h"
#include <windows.h>
#include <winsock2.h>
#include <graphics.h>

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "iphlpapi.lib")    
#pragma comment(lib, "user32.lib")    
#pragma comment(lib, "ws2_32.lib")
#pragma comment (lib,"ws2_32")

#pragma comment (lib,"ssleay32.lib")
#pragma comment (lib,"libeay32.lib")





//openssl part 
#define OPENSSLSTART  1
typedef struct
{
  SSL     *ssl;
  int     fd;
  X509    x509;
  char *cafile;
  char *capath;
  char *certfile;
  char *keyfile;
  char *ciphers;
  char *psk_hint;
  bool require_certificate;
  SSL_CTX *ssl_ctx;
  char *crlfile;
  bool use_identity_as_username;
  char *tls_version;
}FE_MQTT_SSL;

FE_MQTT_SSL fe_mqtt_ssl;






void mqtt_ssl_init(void)
{
 int res=0;

 //OpenSSL_add_ssl_algorithms(); /*��ʼ��*/  
 SSL_load_error_strings();

#if 1
 
  res=SSL_library_init();
 printf("SSL_library_init ,res :%d \r\n",res);


       //fe_mqtt_ssl.ssl_ctx=SSL_CTX_new(SSLv23_client_method());

	   fe_mqtt_ssl.ssl_ctx = SSL_CTX_new(SSLv23_method());
		//fe_mqtt_ssl.ssl_ctx = SSL_CTX_new(TLSv1_2_server_method());
	//ssl_ctx = SSL_CTX_new(TLSv1_1_server_method());
		//fe_mqtt_ssl.ssl_ctx = SSL_CTX_new(TLSv1_server_method());



//SSL_CTX_set_default_verify_paths();

 
//Create CTX 
 // SSL_CTX_set_verify(SSL_CTX * ctx,int mode,int(* callback)(int,X509_STORE_CTX *));
 
    // res=SSL_CTX_set_default_verify_paths(fe_mqtt_ssl.ssl_ctx);
	//	printf("SSL_CTX_set_default_verify_paths ,res :%d \r\n",res);
		if(res==0)
		   {
	//	printf("SSL_CTX_set_default_verify_paths err:%s\n",ERR_error_string(ERR_get_error(),NULL)); 
		   }

//  SSL_CTX_set_verify(fe_mqtt_ssl.ssl_ctx,SSL_VERIFY_PEER,NULL);
  if(res==0)
	 {
  printf("SSL_CTX_set_verify err:%s\n",ERR_error_string(ERR_get_error(),NULL)); 
	 }

 res=SSL_CTX_load_verify_locations(fe_mqtt_ssl.ssl_ctx,"cer/ca.crt",NULL);
 printf("SSL_CTX_load_verify_locations ,res :%d \r\n",res);
 if(res==0)
 	{
 printf("SSL_CTX_load_verify_locations err:%s\n",ERR_error_string(ERR_get_error(),NULL)); 
 	}



 // ���ر���֤�飬��Կ
 
  res=SSL_CTX_use_certificate_file(fe_mqtt_ssl.ssl_ctx,"cer/client.crt",SSL_FILETYPE_PEM);
 printf("SSL_CTX_use_certificate_file ,res %d \r\n",res);
 if(res==0)
 	{
   printf("SSL_CTX_use_certificate_file err:%s\n",ERR_error_string(ERR_get_error(),NULL)); 
 	}

 // res=SSL_CTX_use_certificate_chain_file(fe_mqtt_ssl.ssl_ctx,"/home/openssl/cer/client.csr");
// printf("SSL_CTX_use_certificate_chain_file ,res :%d \r\n",res);
//
  res=SSL_CTX_use_PrivateKey_file(fe_mqtt_ssl.ssl_ctx,"cer/client.pem",SSL_FILETYPE_PEM);
 printf("SSL_CTX_use_PrivateKey_file ,res :%d \r\n",res);
  if(res==0)
	 {
 printf("SSL_CTX_use_PrivateKey_file err:%s\n",ERR_error_string(ERR_get_error(),NULL)); 
	 }


  res=SSL_CTX_check_private_key(fe_mqtt_ssl.ssl_ctx);
 printf("SSL_CTX_check_private_key ,res %d \r\n",res);
 if(res==0)
 	{
printf("SSL_CTX_check_private_key err:%s\n",ERR_error_string(ERR_get_error(),NULL)); 
 	}


 fe_mqtt_ssl.ssl=SSL_new(fe_mqtt_ssl.ssl_ctx);
 printf("SSL_new ok \r\n");


#endif 
 return ;
}


//-----------------------------------------
#define OPENSSLHERE  1
// mqtt part 
#define VS_MQTT_START  0

// MQTT ���� 

int  FeMqtt_Online=0;// �Ƿ�����mqtt��������
char Femqtt_Client_ID[100];
char Femqtt_Server_Url[100];
char Femqtt_Username[100];
char Femqtt_Password[100];
int  Femqtt_Server_Port=1883;
int  Femqtt_WithSSL_Flag=0;
int  Femqtt_Keep_Alive=1;


int Femqtt_Topic_Recv_Hex=0;
int Femqtt_Topic_Recv_Save=0;

int Femqtt_Topic_Pub_Hex=0;
//int Femqtt_Topic_Pub_Save=0;

mqtt_broker_handle_t broker;
unsigned  short  broker_msg_id;

SOCKET Mqtt_Socket_fd;
sockaddr_in Mqtt_Socket_Addrin;
//sockaddr_in sockAddr1_TCP;



#define RCVBUFSIZE 20480
unsigned char packet_buffer[RCVBUFSIZE];



void Femqtt_Save_Payload_To_File(char *topic,char *buff, int length,int type)
{
	CTime time = CTime::GetCurrentTime(); ///����CTime����
	FILE *file;
	int m_nYear = time.GetYear(); ///��

	int m_nMonth = time.GetMonth(); ///��

	int m_nDay = time.GetDay(); ///��

	int m_nHour = time.GetHour(); ///Сʱ

	int m_nMinute = time.GetMinute(); ///����

	int m_nSecond = time.GetSecond(); ///��


	char filename[1000] = "";
	
	sprintf(filename, "%d_%02d_%02d_log.csv", m_nYear, m_nMonth, m_nDay);
	printf("filename :%s \r\n", filename);
	printf("buff :%s\n", buff);
	file = fopen(filename, "a+");

	if (file == NULL)
	{
		file = fopen(filename, "w+");
		if (file == NULL)
			return;
	}
	
	fprintf(file, "%d_%02d_%02d-%02d:%02d:%02d ,", m_nYear, m_nMonth, m_nDay, m_nHour, m_nMinute, m_nSecond);
	//char buff[4096] = "";
	//sprintf_s(buff,)
	fprintf(file,"%s,",topic);
	if (type == 0)
	{
		 // ascii
		fprintf(file, "(STR)Bytes:%3d B,", length);
	}
	else
	{
		fprintf(file, "(HEX)Bytes:%3d B,", length/2);
	}
	fprintf(file, "%s,\n", buff);
	fclose(file);
}


// 
void wcharTochar(const wchar_t *wchar, char *chr, int length)  
{  
    WideCharToMultiByte( CP_ACP, 0, wchar, -1,  
        chr, length, NULL, NULL );  
} 




void hex_to_ascii(char *asciistr,unsigned char *hexstr,int hex_length)
{
 int i;
 
 for(i=0;i<hex_length;i++)
 	{ 
 	 if(((hexstr[i]>>4)&0x0f)>=0&&((hexstr[i]>>4)&0x0f)<=9)
 	 	{
 	 	 asciistr[2*i+0]=((hexstr[i]>>4)&0x0f)+0x30;		 
 	 	}
	 else if(((hexstr[i]>>4)&0x0f)>=0xA&&((hexstr[i]>>4)&0x0f)<=0x0F)
 	 	{
 	 	 asciistr[2*i+0]='A'+(((hexstr[i]>>4)&0x0f)-0x0A);		 
 	 	}
	 
 	 if(((hexstr[i])&0x0f)>=0&&((hexstr[i])&0x0f)<=9)
 	 	{
 	 	 asciistr[2*i+1]=((hexstr[i])&0x0f)+0x30;		 
 	 	}
	 else if(((hexstr[i])&0x0f)>=0xA&&((hexstr[i])&0x0f)<=0x0F)
 	 	{
 	 	 asciistr[2*i+1]='A'+(((hexstr[i])&0x0f)-0x0A);		 
 	 	}
	 // asciistr[3*i+2]=' ';
 	}

 
}


int check_ascii_is_hex(char a)
{
  if(a>=0x30&&a<=0x39)
  	{
  	 return 1;
  	}
  else if(a>='a'&&a<='f')
  	{
  	 return 1;
  	}
  else if(a>='A'&&a<='F')
  	{
  	 return 1;
  	}
  return 0;// �Ƿ����ַ�
}
unsigned char ascii_to_hex(char a1,char a2)
{

  unsigned char tmp1=0;
  unsigned char tmp2=0;
    if(a1>=0x30&&a1<=0x39)
  	{
  	 tmp1=a1-0x30;
  	}
  else if(a1>='a'&&a1<='f')
  	{
  	tmp1=0x0A+(a1-'a');
  	}
  else if(a1>='A'&&a1<='F')
  	{
  	tmp1=0x0A+(a1-'A');
  	}
  
    if(a2>=0x30&&a2<=0x39)
  	{
  	 tmp2=a2-0x30;
  	}
  else if(a2>='a'&&a2<='f')
  	{
  	tmp2=0x0A+(a2-'a');
  	}
  else if(a2>='A'&&a2<='F')
  	{
  	tmp2=0x0A+(a2-'A');
  	}  
  return tmp2+(tmp1<<4);
  
}

int str_to_hex(char *asciistr,unsigned char *hexstr)
{
  int str_length=0;
  int i;
 
  str_length=strlen(asciistr);
 printf("str_to_hex [%d]:%s \r\n",str_length,asciistr);
  
  if(str_length==0)
  	return 0;
  for(i=0;i<str_length/2;i++)
  	{
     if(check_ascii_is_hex(asciistr[2*i])==1&&
	 	 check_ascii_is_hex(asciistr[2*i+1])==1)
     	{
         hexstr[i]=ascii_to_hex(asciistr[2*i],asciistr[2*i+1]);		 
     	}
     else 
     	{
     	 return -1 ;// �Ƿ��ַ�
     	}
	 
  	}

 return str_length/2;
}
int mqtt_socket_disconnect(mqtt_broker_handle_t* broker,int with_ssl)
{
 return 0;
}

int send_with_ssl(void* socket_info, const void* buf, unsigned int count)
{

	FE_MQTT_SSL this_ssl = *((FE_MQTT_SSL*)socket_info);
	printf("send_with_ssl ,");
	return SSL_write(this_ssl.ssl,buf,count);
}

int send_without_ssl(void* socket_info, const void* buf, unsigned int count)
{

 SOCKET fd =*(SOCKET *)socket_info;
 printf("send_without_ssl ,");
 return send(fd, (char *)buf, count, 0);

 return 0;
}
int read_without_ssl(void* socket_info,  const void* buf, unsigned int count)
{
	SOCKET fd =*(SOCKET *)socket_info;
	printf("read_without_ssl \r\n");
	return recv(fd,(char *)buf,count,NULL);
}

int read_with_ssl(void* socket_info,  void* buf, unsigned int count)
{
	FE_MQTT_SSL this_ssl = *((FE_MQTT_SSL*)socket_info);
	printf("read_with_ssl \r\n");
	return SSL_read(this_ssl.ssl,buf,count);
}





int mqtt_get_host_ip(const char *hostname,char *host_ip)
{ 
    int i;
	struct hostent * site=NULL;

	if (strlen(hostname) == 0)
	{
		printf("����Ϊ�գ��޷�����");
		return -1;
	}
	if (site == NULL)
	{
		printf("mqtt_get_host_ip :%s \r\n", hostname);
	}

    site = gethostbyname(hostname);
    if (site==NULL) return -1;
    
    printf("\r\nIP: ");
    for(i=0;i<site->h_length;i++)
        {
		printf("%d.", (unsigned char)site->h_addr_list[0][i]);
	}
    printf("\r\n");
	sprintf_s(host_ip, 16, "%d.%d.%d.%d", (unsigned char)site->h_addr_list[0][0], (unsigned char)site->h_addr_list[0][1], (unsigned char)site->h_addr_list[0][2], (unsigned char)site->h_addr_list[0][3]);


    return 0;
}

int mqtt_socket_init(mqtt_broker_handle_t* broker, const char* hostname, int port,int with_ssl)
{

 int addr_len;
 int res=0;
 // ����tcp����

 	addr_len = sizeof(struct sockaddr_in);
	 Mqtt_Socket_fd=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);

	//�����ӷ�����
	   Mqtt_Socket_Addrin.sin_family=PF_INET;    
	   Mqtt_Socket_Addrin.sin_addr.S_un.S_addr=inet_addr(hostname);
	   Mqtt_Socket_Addrin.sin_port=htons(port);

	if(connect(Mqtt_Socket_fd,(SOCKADDR *)&Mqtt_Socket_Addrin,sizeof(SOCKADDR))==0)
		{

		printf("���ӳɹ�\r\n");
		#if 0
		int flag = 1;
		int result = setsockopt(Mqtt_Socket_fd,			 /* socket affected */
								IPPROTO_TCP,	 /* set option at TCP level */
								TCP_NODELAY, /* name of option */
								(char *) &flag,  /* the cast is historical
														cruft */
								sizeof(int));	 /* length of option value */  
			int iMode = 1; 
		int iasd=ioctlsocket(Mqtt_Socket_fd,FIONBIO,(unsigned long *) &iMode); //���óɷ�����ģʽ
		#endif
		}
	else
		{
		 printf("����ʧ��");
		 return -1;
		}

	
	  if(with_ssl==1)
		{
		  //ssl ���ӳɹ����� ����Ϊ������
		//  flag = fcntl(mqtt_socket_fd, F_GETFL, 0);  
		//  fcntl(mqtt_socket_fd, F_SETFL, flag &(~O_NONBLOCK)); 
		 mqtt_ssl_init();  
	  // MQTT stuffs
	  res=SSL_set_fd(fe_mqtt_ssl.ssl,(int )Mqtt_Socket_fd);
	  printf("ssl_set_fd : %d \r\n",res);
	  
	  res=SSL_connect(fe_mqtt_ssl.ssl);
	  printf("SSL_connect :%d ,err:%s\r\n",res,ERR_error_string(ERR_get_error(),NULL));
	   if(res==0)
		{
	printf("SSL_CTX_check_private_key err:%s\n",ERR_error_string(ERR_get_error(),NULL)); 
		}
	   }






    broker->socket_fd= (void*)&Mqtt_Socket_fd;
  if(with_ssl==1)
  	{
  	#if 1
	broker->socket_info= (void*)&fe_mqtt_ssl;
	broker->send = send_with_ssl;
	#endif
  	}
  else
  	{
	broker->socket_info= (void*)&Mqtt_Socket_fd;
	broker->send =send_without_ssl;
  	}
  return 0;
}



int mqtt_read_select(void* socket_fd, unsigned int time)
{
	int rVal = 0;
	
    int sockfd=*((int*)socket_fd);
	struct fd_set rset;
	struct timeval timeout;

	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	timeout.tv_sec = time / 1000; //����select������ʱֵ (seconds)
	timeout.tv_usec = 1000 * (time % 1000);//(microseconds)

	//select��������
	rVal = select(sockfd + 1, &rset, NULL, NULL, &timeout);
	rVal = FD_ISSET(sockfd, &rset);
	return rVal;
}
int Femqtt_Read_Packet(int time_out,int with_ssl)
{
	int res;
	int total_bytes = 0, bytes_rcvd, packet_length;	
    res=mqtt_read_select(broker.socket_fd,time_out);
	if(res>0)
	   {
	 printf("Femqtt_Read_Packet,select res :%d  \r\n",res);
	 
	   }
	else
	   {
		return res;
	   }

 memset(packet_buffer, 0, sizeof(packet_buffer));
 while(total_bytes < 2) // Reading fixed header
 {


  if(with_ssl==1)
  	{
  	 bytes_rcvd=read_with_ssl(broker.socket_info,(packet_buffer+total_bytes),RCVBUFSIZE);
     if(bytes_rcvd<=0)
     	{ 
     	 return -1;
     	}
    }
  else
  	{
	  bytes_rcvd=read_without_ssl(broker.socket_info,(packet_buffer+total_bytes),RCVBUFSIZE);
	  if(bytes_rcvd<=0)
		 { 
		  return -1;
		 }
  	}
 
	 
	 total_bytes += bytes_rcvd; // Keep tally of total bytes
 }
 
 packet_length = packet_buffer[1] + 2; // Remaining length + fixed header length
 
 while(total_bytes < packet_length) // Reading the packet
 
  {
 
 if(with_ssl==1)
   {
	//bytes_rcvd=read_with_ssl(broker.socket_info,(packet_buffer+total_bytes),RCVBUFSIZE);
	if(bytes_rcvd<=0)
	   { 
		return -1;
	   }
   }
 else
   {
	 bytes_rcvd=read_without_ssl(broker.socket_info,(packet_buffer+total_bytes),RCVBUFSIZE);
     if(bytes_rcvd<=0)
     	{ 
     	 return -1;
     	}
   }

	 total_bytes += bytes_rcvd; // Keep tally of total bytes
 }
 
 return packet_length;



 return 0;
}

int FEMQTT_MAIN_RUN=0;

int FeMqtt_Keep_Alive_Flag=0;
void *FeMqtt_Keep_Alive_Main(void)
{
 FeMqtt_Keep_Alive_Flag=1;// ��ʾ�Ѿ���������
 int i;
  while(1)
 	{
 	if(FeMqtt_Online)
 		{
 	printf("FeMqtt_Keep_Alive_Main :%d S \r\n",Femqtt_Keep_Alive);
    mqtt_ping(&broker);
 		}
    if(Femqtt_Keep_Alive>0)
     	{
        for(i=0;i<Femqtt_Keep_Alive;i++) 	
    	Sleep(1000);
    	}
	else
		{
		 Sleep(10*1000);// ����10s
		}
  	}
}
void Cmqtt_clientDlg:: FEMQTT_MAIN(LPVOID lpv)
{
  
  int packet_length;
  int msg_id;
  int msg_recv_len;
  unsigned char hex_str[4096] = "";
  Cmqtt_clientDlg * pMain = (Cmqtt_clientDlg *) lpv; //ǿ��ת����ô���������ָ��
  int i=0;
  int res;
  CString tmp;
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
  char host_ip[1000];
 res= mqtt_get_host_ip(Femqtt_Server_Url, host_ip);
 if (res == -1)
 {
	 ::SetWindowText(::GetDlgItem(pMain->m_hWnd, IDC_STATIC_STATU), L"��������ʧ��");
 }
 else if (res == 0)
 {
	 ::SetWindowText(::GetDlgItem(pMain->m_hWnd, IDC_STATIC_STATU), L"���������ɹ�����ʼ��¼");
 }

 
 mqtt_init(&broker, Femqtt_Client_ID);
 mqtt_init_auth(&broker, Femqtt_Username, Femqtt_Password);
 res = mqtt_socket_init(&broker, host_ip, Femqtt_Server_Port, Femqtt_WithSSL_Flag);
 if(res==0)
	{
  printf("mqtt_socket_init ok \r\n");
  //broker.send(broker.socket_info,"hello mqtt",10);
 
	}
	else
	{
		printf("mqtt_socket_init error ,:%d \r\n",res);
		
		FEMQTT_MAIN_RUN = 0;
		::SetWindowText(::GetDlgItem(pMain->m_hWnd, IDC_STATIC_STATU), L"����ʧ��");
		goto MQTT_ERR ;
	}

  
  mqtt_set_alive(&broker,Femqtt_Keep_Alive);
  res=mqtt_connect(&broker);
  printf("mqtt connect res :%d \r\n",res);
 
  packet_length = Femqtt_Read_Packet(1000,Femqtt_WithSSL_Flag);
  if(packet_length<=0)
  	{
  	 printf("��¼ʧ�� \r\n"); 
	 ::SetWindowText(::GetDlgItem(pMain->m_hWnd, IDC_STATIC_STATU), L"��¼ʧ��,����Ӧ");
	 goto MQTT_ERR ;
  	}
   if(MQTTParseMessageType(packet_buffer) != MQTT_MSG_CONNACK)
   {
	   printf("��¼ʧ�� \r\n"); 	   
	   ::SetWindowText(::GetDlgItem(pMain->m_hWnd, IDC_STATIC_STATU), L"��¼ʧ��,��Ϣ����");
	   goto MQTT_ERR ;
   }
   if(packet_buffer[3] != 0x00)
   {
	   printf("��¼ʧ��\r\n"); 	   
	   ::SetWindowText(::GetDlgItem(pMain->m_hWnd, IDC_STATIC_STATU), L"��¼ʧ��,��֤ʧ��");
	   goto MQTT_ERR ;
   }
   printf("��¼�ɹ�\r\n");	   
   ::SetWindowText(::GetDlgItem(pMain->m_hWnd, IDC_STATIC_STATU), L"��¼�ɹ�");   
   ::SetWindowText(::GetDlgItem(pMain->m_hWnd, IDC_BUTTON_MQ_CONNECT), L"�Ͽ�");
   FeMqtt_Online=1;
   
   mqtt_ping(&broker);// ��������߳�֮ǰ���ȷ���������
   
  while (1)
  {


    if(FeMqtt_Online==0)
    	{
    	 printf("�Ͽ����� \r\n");
		 mqtt_disconnect(&broker);
		 
		 ::SetWindowText(::GetDlgItem(pMain->m_hWnd, IDC_BUTTON_MQ_CONNECT), L"����");
		 goto MQTT_ERR ;
    	}

	    res=Femqtt_Read_Packet(1000,Femqtt_WithSSL_Flag);
        if(res==-1)
        	{
			::SetWindowText(::GetDlgItem(pMain->m_hWnd, IDC_STATIC_STATU), L"�쳣�Ͽ�");   
			::SetWindowText(::GetDlgItem(pMain->m_hWnd, IDC_BUTTON_MQ_CONNECT), L"��¼");
			goto MQTT_ERR ;
        	}
		else if(res==0)
			{
			// ��ʱû�����ݣ������ȴ�
			}
		else if(res>0)
			{
			printf("Femqtt_Recv_Main,bytes :%d \r\n",res);
			for(i=0;i<res;i++)
			   {
				if(i%16==0) 
				   printf("\r\n");
				printf("%02x ",packet_buffer[i]);
				   
			   }
			printf("\r\n");
			// ����Э��
			if(MQTTParseMessageType(packet_buffer) == MQTT_MSG_PUBLISH)
			  {
				uint8_t topic[255], msg[RCVBUFSIZE];
				uint16_t len;
				len = mqtt_parse_pub_topic(packet_buffer, topic);
				topic[len] = '\0'; // for printf
				len = mqtt_parse_publish_msg(packet_buffer, msg);
				msg_recv_len=len;
				msg[len] = '\0'; // for printf
				msg_id=mqtt_parse_msg_id(packet_buffer);
			
				printf("%s %s\n", topic, msg);
				printf("recv msg id %d \r\n",msg_id);
				//Cfg_Mqtt_Recv_Msg=msg_id;
				//FeMqtt_Recv_Topic_Cmd_Do(topic,msg);  
                
				TCHAR Paylaod_L[1000]=L"";
				TCHAR topic_L[1000]=L"";
				swprintf(topic_L,L"%S",topic);
				swprintf(Paylaod_L,L"%S",msg);
                char hex_str[4096]="";
				//hex_to_ascii()
				int paylen = msg_recv_len;
				for(int j=0;j<msg_recv_len;j++)
					{
					 printf("%02x ",msg[j]);
					}
				printf("\r\n");
				if(Femqtt_Topic_Recv_Hex==1)
					{
				
				hex_to_ascii((char *)hex_str,msg,msg_recv_len);				
				swprintf(Paylaod_L,L"%S",hex_str);
				paylen = 2 * msg_recv_len;
					}

				::SetWindowText(::GetDlgItem(pMain->m_hWnd, IDC_EDIT_RECV_PAYLOAD), Paylaod_L);   
				::SetWindowText(::GetDlgItem(pMain->m_hWnd, IDC_EDIT_RECV_TOPIC), topic_L);   
				if (Femqtt_Topic_Recv_Save == 1)
				{
					if (Femqtt_Topic_Recv_Hex == 1)
					{
						printf("hexstr :%s \r\n", hex_str);
						Femqtt_Save_Payload_To_File((char *)topic,(char *)hex_str, paylen,1);
					}
					else
					{
						printf("msg_str :%s \r\n", msg);
						Femqtt_Save_Payload_To_File((char *)topic,(char *)msg, paylen,0);
					}
				}
				//CString payLenCS;
				TCHAR msg_Len[1000];
				swprintf(msg_Len, L"%d", paylen);
				::SetWindowText(::GetDlgItem(pMain->m_hWnd, IDC_STATIC_RECV_BYTES), msg_Len);
				mqtt_pubrel(&broker,msg_id);
			  }


			
			}



  }



MQTT_ERR:
 FEMQTT_MAIN_RUN=0;
 return ;
}




#define VS_MQTT_HERE   1
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Cmqtt_clientDlg �Ի���



Cmqtt_clientDlg::Cmqtt_clientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Cmqtt_clientDlg::IDD, pParent)
	, m_edit_url(_T("liangxidong.xyz"))
	, m_edit_user(_T("admin"))
	, m_edit_pass(_T("12345"))
	, m_edit_deviceid(_T("telxd"))
	, m_edit_kalive(60)
	, m_edit_port(1883)
	, m_text_status(_T(""))
	, m_combo_sub_q(1)
	, m_combo_pub_q(1)
	, m_combo_sub_topic(_T(""))
	, m_edit_pub_topic(_T(""))
	, m_edit_pub_payload(_T(""))
	, m_text_ssl(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cmqtt_clientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_Text(pDX, IDC_EDIT_PORT, m_edit_port2);
	DDX_Text(pDX, IDC_EDIT_URL, m_edit_url);
	DDX_Text(pDX, IDC_EDIT_USER, m_edit_user);
	DDX_Text(pDX, IDC_EDIT_PASS, m_edit_pass);
	//  DDX_Text(pDX, IDC_EDIT_KALIVE, m_edit_kalive);
	DDX_Text(pDX, IDC_EDIT_DEVICEID, m_edit_deviceid);
	DDX_Text(pDX, IDC_EDIT_KALIVE, m_edit_kalive);
	DDV_MinMaxUInt(pDX, m_edit_kalive, 0, 65535);
	DDX_Text(pDX, IDC_EDIT_PORT, m_edit_port);
	DDV_MinMaxUInt(pDX, m_edit_port, 0, 65535);
	DDX_Text(pDX, IDC_STATIC_STATU, m_text_status);
	DDX_Control(pDX, IDC_EDIT_PORT, c_edit_port);
	DDX_CBIndex(pDX, IDC_COMBO_SUB_Q, m_combo_sub_q);
	DDX_CBIndex(pDX, IDC_COMBO_PUB_Q, m_combo_pub_q);
	DDX_CBString(pDX, IDC_COMBO_SUB_TOPIC, m_combo_sub_topic);
	DDX_Control(pDX, IDC_COMBO_SUB_TOPIC, c_combo_sub_topic);
	DDX_Control(pDX, IDC_CHECK_RECV_HEX, c_check_recv_hex);
	DDX_Control(pDX, IDC_CHECK_RECV_SAVE, c_check_recv_save);
	DDX_Text(pDX, IDC_EDIT_PUB_TOPIC, m_edit_pub_topic);
	DDX_Text(pDX, IDC_EDIT_PUB_PAYLOAD, m_edit_pub_payload);
	DDX_Control(pDX, IDC_CHECK_PUB_HEX, c_check_pub_hex);
	DDX_Control(pDX, IDC_CHECK_SSL, c_check_ssl);
	DDX_Text(pDX, IDC_STATIC_SSL_TEXT, m_text_ssl);
	DDX_Control(pDX, IDC_BUTTON_AD, c_button_ad);
	DDX_Control(pDX, IDC_CHECK_AD, c_check_ad);
}

BEGIN_MESSAGE_MAP(Cmqtt_clientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_MQ_CONNECT, &Cmqtt_clientDlg::OnBnClickedButtonMqConnect)
	ON_BN_CLICKED(IDC_BUTTON_SUB, &Cmqtt_clientDlg::OnBnClickedButtonSub)
	ON_BN_CLICKED(IDC_BUTTON_UNSUB, &Cmqtt_clientDlg::OnBnClickedButtonUnsub)
	ON_BN_CLICKED(IDC_CHECK_PUB_HEX, &Cmqtt_clientDlg::OnBnClickedCheckPubHex)
	ON_BN_CLICKED(IDC_CHECK_RECV_HEX, &Cmqtt_clientDlg::OnBnClickedCheckRecvHex)
	ON_BN_CLICKED(IDC_CHECK_PUB_DEF, &Cmqtt_clientDlg::OnBnClickedCheckPubDef)
	ON_BN_CLICKED(IDC_CHECK_RECV_SAVE, &Cmqtt_clientDlg::OnBnClickedCheckRecvSave)
	ON_BN_CLICKED(IDC_BUTTON_PUB, &Cmqtt_clientDlg::OnBnClickedButtonPub)
	ON_BN_CLICKED(IDC_BUTTON_DEF, &Cmqtt_clientDlg::OnBnClickedButtonDef)
	ON_BN_CLICKED(IDC_CHECK_SSL, &Cmqtt_clientDlg::OnBnClickedCheckSsl)
	ON_BN_CLICKED(IDC_CHECK_AD, &Cmqtt_clientDlg::OnBnClickedCheckAd)
	ON_BN_CLICKED(IDC_BUTTON_AD, &Cmqtt_clientDlg::OnBnClickedButtonAd)
	ON_BN_CLICKED(IDC_BUTTON_TELL, &Cmqtt_clientDlg::OnBnClickedButtonTell)
//	ON_WM_CTLCOLOR()
//ON_WM_NCPAINT()
END_MESSAGE_MAP()


// Cmqtt_clientDlg ��Ϣ�������

BOOL Cmqtt_clientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
    srand(time(0));
	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	//ShowWindow(SW_MAXIMIZE);

   UpdateData(true);
    m_edit_deviceid.Format(L"%08X",rand());
	 UpdateData(false);
	// TODO:  �ڴ���Ӷ���ĳ�ʼ������


	

#ifdef _DEBUG
#define new DEBUG_NEW

	AllocConsole();                     // �򿪿���̨��Դ,add by lxd 
	freopen( "CONOUT$", "w+t", stdout );// ����д
	freopen( "CONIN$", "r+t", stdin );  // �����
#endif
//	setlocale(LC_ALL, "chs");//����ڼ���ȥ������һЩ���� ��ʾ����
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void Cmqtt_clientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void Cmqtt_clientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{

	
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR Cmqtt_clientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void Cmqtt_clientDlg::OnBnClickedButtonMqConnect()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	wcharTochar(m_edit_url, Femqtt_Server_Url, sizeof(Femqtt_Server_Url));
	wcharTochar(m_edit_user, Femqtt_Username, sizeof(Femqtt_Username));
	wcharTochar(m_edit_pass, Femqtt_Password, sizeof(Femqtt_Password));
	wcharTochar(m_edit_deviceid, Femqtt_Client_ID, sizeof(Femqtt_Client_ID));
	Femqtt_Server_Port=m_edit_port;
	Femqtt_Keep_Alive=m_edit_kalive;
	printf("usr  :%s \r\n", Femqtt_Server_Url);
	printf("port :%d \r\n", Femqtt_Server_Port);
	printf("user :%s \r\n", Femqtt_Username);
	printf("pass :%s \r\n", Femqtt_Password);
	printf("deid :%s \r\n", Femqtt_Client_ID);
	printf("live :%d \r\n", Femqtt_Keep_Alive);
	m_text_status.Format(L"��ӭ");
	c_combo_sub_topic.ResetContent();// ÿ�ζ�Ҫ���

	if(FEMQTT_MAIN_RUN==0)
	 {
	 FEMQTT_MAIN_RUN=1;
	 CWinThread* cWth1 = AfxBeginThread((AFX_THREADPROC)FEMQTT_MAIN, (LPVOID)this);  //�����߳�
     if(FeMqtt_Keep_Alive_Flag==0)
     	{
	 CWinThread* cWth2 = AfxBeginThread((AFX_THREADPROC)FeMqtt_Keep_Alive_Main, (LPVOID)this);  //�����߳�
     	}
	}

    if(FeMqtt_Online==1)
    	{
    	 FeMqtt_Online=0;
    	}


	
	UpdateData(false);

}


void Cmqtt_clientDlg::OnBnClickedButtonSub()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	int q;
	char topic_str[1000];
	UpdateData(true);
	wcharTochar(m_combo_sub_topic, topic_str, sizeof(topic_str));
	q = m_combo_sub_q;
	printf("topic :%s \r\n", topic_str);
	printf("q     :%d \r\n", q);
	if (m_combo_sub_topic.IsEmpty())
	{
		MessageBox(L"���ⲻ��Ϊ��");
		return;
	}

	// ����
	printf("box :%d \r\n", c_combo_sub_topic.GetCount());
	
	//c_combo_sub_topic.InitStorage(100, 100 * 1000);
	//c_combo_sub_topic.ResetContent();
	c_combo_sub_topic.AddString(m_combo_sub_topic);
	//c_combo_sub_topic.InsertString(0, L"12356");
	//c_combo_sub_topic.InsertString(1, L"32356");
	//c_combo_sub_topic.InsertString(2, L"22356");
	//((CComboBox*)GetDlgItem(IDC_COMBO_SUB_TOPIC))->AddString(L"123565");

	mqtt_subscribe(&broker, topic_str, &broker_msg_id, q);
	UpdateData(false);



}


void Cmqtt_clientDlg::OnBnClickedButtonUnsub()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	int q;
	char topic_str[1000];
	int index;
	UpdateData(true);
	wcharTochar(m_combo_sub_topic, topic_str, sizeof(topic_str));
	q = m_combo_sub_q;

	if (m_combo_sub_topic.IsEmpty())
	{
		MessageBox(L"���ⲻ��Ϊ��");
		return;
	}
	

	printf("topic :%s \r\n", topic_str);
	printf("q     :%d \r\n", q);

	//c_combo_sub_topic.ResetContent();

	index = c_combo_sub_topic.GetCurSel();
	printf("box index :%d \r\n", index);

	if (index == -1)
	{
		// �����ַ���
		int nDex = 0;
		while ((nDex = c_combo_sub_topic.FindStringExact(nDex, m_combo_sub_topic))
			!= CB_ERR)
		{
			printf("�ҵ�ƥ����\r\n");
			c_combo_sub_topic.DeleteString(nDex);
		}

	}

	if (index>=0)
	c_combo_sub_topic.DeleteString(index);
//	c_combo_sub_topic.ResetContent();

	// ȡ������
	mqtt_unsubscribe(&broker, topic_str, &broker_msg_id);

	UpdateData(false);
}


void Cmqtt_clientDlg::OnBnClickedCheckPubHex()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	 
	if (c_check_pub_hex.GetCheck())
	{
		printf("��������HEX\r\n");
		Femqtt_Topic_Pub_Hex=1;
		printf("hex :%d \r\n",Femqtt_Topic_Pub_Hex);
	}
	else
	{
		printf("����������HEX\r\n");
		Femqtt_Topic_Pub_Hex=0;
		printf("hex :%d \r\n",Femqtt_Topic_Pub_Hex);
	}
	
	printf("hex :%d \r\n",Femqtt_Topic_Pub_Hex);
	UpdateData(false);
}


void Cmqtt_clientDlg::OnBnClickedCheckRecvHex()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	if (c_check_recv_hex.GetCheck())
	{
		printf("����HEX\r\n");
		Femqtt_Topic_Recv_Hex=1;
	}
	else
	{
		printf("������HEX\r\n");
		Femqtt_Topic_Recv_Hex=0;
	}
	UpdateData(false);

}


void Cmqtt_clientDlg::OnBnClickedCheckPubDef()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}


void Cmqtt_clientDlg::OnBnClickedCheckRecvSave()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	
	if (c_check_recv_save.GetCheck())
	{
		printf("����\r\n");
		Femqtt_Topic_Recv_Save=1;
		
	}
	else
	{
		printf("������\r\n");		
		Femqtt_Topic_Recv_Save=0;
	}
	UpdateData(false);

}


void Cmqtt_clientDlg::OnBnClickedButtonPub()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	char topic[1000];
	
	int i;
	int flag;
	int q;
	char payload[1000];
	UpdateData(true);
    unsigned char hex_str[1000];
	int hex_length;
 wcharTochar(m_edit_pub_topic, topic, sizeof(topic));
 wcharTochar(m_edit_pub_payload, payload, sizeof(payload));
 printf("hex :%d \r\n",Femqtt_Topic_Pub_Hex);
 printf("topic :%s \r\n",topic);
 q = m_combo_pub_q;
 if(Femqtt_Topic_Pub_Hex==0)
 	{
	 printf("payload :%s \r\n",payload);
     memcpy(hex_str,payload,sizeof(payload));
	 hex_length=strlen((char *)hex_str);
    }
 else 
 	{
    hex_length=str_to_hex(payload,hex_str);
	if(hex_length==-1)
		{
		printf("���ڷǷ�����\r\n");

		}
 	
 	}
 if(hex_length>0)
 	{
	 for(i=0;i<hex_length;i++)
	 	{
	 	printf("%02x",hex_str[i]);
	 	}
	 printf("\r\n");
    if(Femqtt_Topic_Pub_Hex==0)
    	{
	flag=mqtt_publish_with_qos(&broker, topic, (char *)hex_str, 0, q, &broker_msg_id);
    printf("mqtt_publish_with_qos :%d \r\n",flag);
	}
    else 
    	{ 
		flag=mqtt_publish_with_qos_hex(&broker, topic, (char *)hex_str, 0, q, &broker_msg_id,hex_length);
		printf("mqtt_publish_with_qos_hex :%d \r\n",flag);
    	}
 	}
 UpdateData(false);

}


void Cmqtt_clientDlg::OnBnClickedButtonDef()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CDialg_Def Def;
	Def.DoModal();
	//Def.do

}


void Cmqtt_clientDlg::OnBnClickedCheckSsl()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	if (c_check_ssl.GetCheck())
	{
		Femqtt_WithSSL_Flag = 1;
		printf("����SSL %d \r\n", Femqtt_WithSSL_Flag);

		m_text_ssl.Format(L"�밴����Ŀ¼�������ļ���\r\ncer/ca.crt \r\ncer/client.crt\r\ncer/client.pem");



	}
	else
	{
		Femqtt_WithSSL_Flag = 0;
		printf("������SSL %d \r\n", Femqtt_WithSSL_Flag);
		m_text_ssl.Format(L"");
	}
	UpdateData(false);

}


void Cmqtt_clientDlg::OnBnClickedCheckAd()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	if (c_check_ad.GetCheck())
	{
		c_button_ad.ShowWindow(true);
	}
	else
	{
		c_button_ad.ShowWindow(false);
	}
	UpdateData(false);
}


TCHAR ad_str[100][1000] = { 
L"��һ��Ƕ��ʽ����ʦ����ʵ��̫����",
L"��һ�䴿����Ϊ�˴�����",
L"����ܻ����������д��ʫ",
L"������µ�ÿһ����\r\n�󺣶������ʰ��\r\n���һƬ��ɫ��˼��",
L"����������ˣ��ͻع�����ˮ��",
L"������Ҳ�п���дС˵",
L"���п��ܻῴ���ظ��ģ���Ϊ�����������",
L"MFCд�Ľ���ȷʵ���ÿ�����Ҫ���²��ˣ��������������UI��",
L"�㻹���̲�ס���˹��",
L"�����ϲ�������������Դ򵽵绰��ǰ̨��л���ߣ�ǰ̨�Ǹ���Ư����������Ŷ",
L"��������˽ⴥ������������32���HMIŶ��������ʾ",
L"���������PLC,������������Ʒ���պã�����������PLC",
L"�п��ܻ���ʾ�հף���Ϊ�ܶ�����û��д�����ǿյ�",
L" �հ� ",
L" �Ҿ�����ƽ�����ܻ��ܲ���������������ʾ",
L"�ţ���ʵ�����Ǹ�90��С˧��",
L"ʮ��֮ǰ�и��˺���˵���������30֮����δȢ�����ͼ޸��ҡ��������룬��������������ʮ��ǰ�ǵ���Ц����ʮ����컯Ū��"};


int click_time = 0;
void Cmqtt_clientDlg::OnBnClickedButtonAd()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������

	UpdateData(true);
	
	if (click_time == 0)
	{
	
		MessageBox(L"���ʱ��,ÿ��һ�ζ����о�ϲŶ");
	}
	else
	{
		MessageBox(ad_str[rand() % 26]);
	}
	click_time++;
	UpdateData(false);

}


void Cmqtt_clientDlg::OnBnClickedButtonTell()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	MessageBox(L"-----------����------------\r\n0 Ĭ�ϵĵ��Է�����������ʱ�ر�\r\n1 ���������ʹ�ã���ɫ�ⰲװ��\r\n2 MQTTЭ��汾Ϊ3��Э�鲿�ֲο���EMQ��client���ֵĴ���\r\n3 MQTT���Ժ�����ѹ����ѹ���Խ�������ʹ�ã��������������ж�֤��\r\n4 ʹ�ô˹�����������Ӱ�����ʧ�������߲������������쳣����������ݲ��������������ͻ�����쳣��\r\n5 ����������bug���ɸ�֪���߻����ʹ�ã�ллʹ�á�\r\n");
}


//HBRUSH Cmqtt_clientDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
//{
//	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
//
//	// TODO:  �ڴ˸��� DC ���κ�����
//	//if (nCtlColor == CTLCOLOR_EDIT)
//
//
//	
//	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
//	return hbr;
//}


//void Cmqtt_clientDlg::OnNcPaint()
//{
//	// TODO:  �ڴ˴������Ϣ����������
//	// TODO: Add your message handler code here
//	//GetStockObject(BLACK_BRUSH);
//	// Do not call CDialog::OnNcPaint() for painting messages
//	// ��Ϊ��ͼ��Ϣ����
//	CBitmap bmp;
//	bmp.LoadBitmap(IDR_MAINFRAME);
//	CWindowDC dc(this);
//	CDC memDC;
//	memDC.CreateCompatibleDC(&dc);
//	CRect rect;
//	GetWindowRect(&rect);
//	memDC.SelectObject(&bmp);
//	dc.StretchBlt(0, 0, rect.Width(), 25, &memDC, 0, 0, 10, 25, SRCCOPY);
//	CDialogEx::OnNcPaint();
//}
