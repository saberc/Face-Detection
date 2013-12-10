/*
 * DataSharing.h
 *
 *  Created on: Aug 10, 2013
 *      Author: Alessandro
 */

/*
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sstream>

*/


#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <opencv2/opencv.hpp>




using namespace cv;
using namespace std;

#ifndef DATASHARING_H_
#define DATASHARING_H_

class DataSharing {
private:
	int sockfd;
	char dataShare[1024];
	struct sockaddr_in serv_addr;
	string convertInt(int number);

public:
	DataSharing();
	bool connectToSoket(string ipAddress);
	void writeToSoket(int value);
	void release();
	virtual ~DataSharing();
};

#endif /* DATASHARING_H_ */
