/*
 * DataSharing.cpp

 *
 *  Created on: Aug 10, 2013
 *      Author: Alessandro
 */

#include "DataSharing.h"

DataSharing::DataSharing() {
	// TODO Auto-generated constructor stub

	//memset(recvBuff, '0' ,sizeof(recvBuff));






}

DataSharing::~DataSharing() {
	// TODO Auto-generated destructor stub
}


void DataSharing::release(){
close(sockfd);
}

bool DataSharing::connectToSoket(string ipaddress){
	bool returnV;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0){
		      printf("\n Error : Could not create socket \n");
		      returnV=false;
	}else{

	 serv_addr.sin_family = AF_INET;
	 serv_addr.sin_port = htons(8887);
	 serv_addr.sin_addr.s_addr = inet_addr(ipaddress.c_str());

	 if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0){
		returnV=false;
	 }
	 else{
		returnV=true;
	 }
	}

	return(returnV);

}


string DataSharing::convertInt(int number)
{
   stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}


void DataSharing::writeToSoket(int value){

	string valueString;

	valueString=convertInt(value);

	 write(sockfd, valueString.c_str(), strlen(valueString.c_str()));

}
