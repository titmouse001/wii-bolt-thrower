#include <stdio.h>
#include <stdlib.h>
#include <string>
//#include "HTTP_util.h"
#include <string.h>		// included for memcpy


#include <unistd.h>		// included for usleep
//The <unistd.h> header defines miscellaneous symbolic constants and types, and declares miscellaneous functions.
// http://pubs.opengroup.org/onlinepubs/000095399/basedefs/unistd.h.html

#include "ogcsys.h"
#include "network.h"



#include "URIManager.h"

URLManager::URLManager()
{
	char IP[16]; // i.e. room for 255.255.255.255 + NULL
	// this calls net_init() in a nice way (must provide first param)
	if ( if_config(IP, NULL, NULL, true) < 0 ) // throw away the IP result - don't need it
		printf("if_config failed");
}

URLManager::~URLManager()
{
	net_deinit();
}


// URLManager section

string URLManager::GetHostNameFromUrl(const string& Url, const string& Match)
{
	//	static const string Match("http://");  // assume incoming string is in lower case
	string SiteName(Url);
	if (Url.find(Match) != string::npos)
	{
		SiteName.replace(0,Match.length(),""); // wipe http:// ... left with something like www.fnordware.com/superpng/...
		size_t pos = SiteName.find("/");
		if (pos != string::npos)
		{
			SiteName = SiteName.substr(0,pos); // now have the 'hostname', for example "www.google.com"  (not "google.com" as that is the 'domain')
			return SiteName;
		}
	}
	return "";  // empty - nothing found
}

// setup the Hypertext Transfer Protocol
string URLManager::CreateHttpRequest(const string& CommandWithSpace, const string& Url) // this function assumes the url has been validated
{
	//Url; http://www.google.com/stuff/test.png
	//path; /stuff/test.png
	//host; www.google.com

	string Host( GetHostNameFromUrl(Url) );
	string Path = Url.substr(string("http://").length() + Host.length(),Url.length() - string("http://").length() );
	string BuildPacket( CommandWithSpace + Path + " HTTP/1.1\r\n" );    //i.e. "GET /stuff/test.png HTTP/1.1\r\n"
	string RefererPath = Url.substr( 0, Url.rfind("/") + 1  );

	BuildPacket +=  "Host: " + Host + "\r\n";  // The domain name of the server (for virtual hosting), mandatory since HTTP/1.1
	BuildPacket +=  "Referer: " + RefererPath + "\r\n";  // (misspelled) This is the address of the previous web page from which a link to the currently requested page was followed.

	//BuildPacket +=  "User-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 5.1; Trident/4.0; .NET4.0C; .NET CLR 1.1.4322; .NET CLR 2.0.50727; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; InfoPath.2)\r\n";
	BuildPacket +=  "User-Agent: WiiBoltThrower/0.6 libogc/1.8.7\r\n"; //User agents SHOULD include this field with requests

	BuildPacket +=  "Connection: close\r\n\r\n";  //HTTP/1.1 applications that do not support persistent connections MUST include the "close" connection option in every message. 
	return BuildPacket;


	//BuildPacket +=  "Accept: image/gif, image/jpeg, image/pjpeg, image/pjpeg, application/x-shockwave-flash, application/xaml+xml, application/x-ms-xbap, application/x-ms-application, application/vnd.ms-excel, application/vnd.ms-powerpoint, application/msword, application/vnd.ms-xpsdocument, */*\r\n";
	//BuildPacket +=  "Accept-Language: en-gb\r\n";
	//BuildPacket +=  "User-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 5.1; Trident/4.0; .NET4.0C; .NET CLR 1.1.4322; .NET CLR 2.0.50727; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; InfoPath.2)\r\n";
	//BuildPacket +=  "Accept-Encoding: gzip, deflate\r\n";


	// or the above could use an older http call with ... CommandWithSpace + URL + " HTTP/1.0\r\n\r\n";
}

void URLManager::SaveURI(string URI, string DestinationPath )
{
	printf("SaveURI");
	MemoryInfo* pMeminfo( NewFromURI( URI ) );
	pMeminfo->SavePath( DestinationPath );
	delete pMeminfo;
}


MemoryInfo* URLManager::NewFromURI(string URI)
{
	MemoryInfo* MemInfo( new MemoryInfo );
	
	string SiteName( GetHostNameFromUrl(URI) );
	if (SiteName.empty())
		return NULL;

	hostent* host( net_gethostbyname( SiteName.c_str() ) );   // todo  switch (h_errno)      HOST_NOT_FOUND , NO_ADDRESS , NO_RECOVERY ,TRY_AGAIN
	if (host==NULL)
		return MemInfo;

	// socket takes, domain family, type & protocol
	int sockfd (net_socket(PF_INET, SOCK_STREAM, IPPROTO_IP));  

	// (getaddrinfo not supported by wii libarary)
	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;  //address family (not really any choice under this Wii lib)
	addr.sin_port = htons(80);
	addr.sin_addr = *((in_addr*)host->h_addr);

	// ------------------------------------------------------------
	// DEBUG section
#if URL_DEBUG
	OutputDebugString("\n");
	OutputDebugString(host->h_name); 
	in_addr** addr_list = (in_addr**)host->h_addr_list;  
	for ( int i = 0; addr_list[i] != NULL; i++ ) 
	{
		OutputDebugString( (string("\n")+ string(inet_ntoa(*addr_list[i])) + string("\n") ).c_str() );
	}
#endif
	// ------------------------------------------------------------

	int connection = net_connect(sockfd, (struct sockaddr *)&addr, sizeof(addr) );
	if (connection == -1)
	{
		net_close(connection);   // NOTE, Use 'close' for unix, net_closesocket for windows
		return NULL;
	}

	string RequestPacket( CreateHttpRequest("GET ", URI) );

	int BytesRemaining( RequestPacket.length() );

#if URL_DEBUG
	std::ostringstream  osstream;
	osstream << BytesRemaining;
	OutputDebugString( ("Sending\n" + RequestPacket + "You sent a RequestPacket using " + osstream.str() + " bytes\n\n").c_str() );
#endif 

	do{
		int BytesSent = net_send(sockfd, RequestPacket.c_str(), BytesRemaining, 0); // My tests under windows found that just one packet is needed - even when I made the packet really big!
		if (BytesSent < 0) // hopefully zero is never a problem
			return NULL;

		BytesRemaining -= BytesSent;
	}while (BytesRemaining > 0);

	if (BytesRemaining!=0)
		return NULL; // NOT GOOD - should never happen

	int  BytesRead(0);
	int TotalReceived(0);
	string WorkingString; 
	// looks like the tiny header comes in on its own - but I'll allow for it to be part of a bigger packet
	int MTU ( 1432 ); // my broadband network has an MTU of 1432 bytes 
	int BufferSize( MTU * 2000 ); // This will grow if not Content-Length is found
	u8* TempStoreForReadData( (u8*)malloc( BufferSize ) ); 
	u8* ptrWorking( TempStoreForReadData );
	int reallocAmount( MTU*2 );

	while( (BytesRead = net_recv(sockfd, (char*)ptrWorking, MTU, 0)) ) // seems to be a max size for this buffer, after a sertain size you just get -1 error
	{
		if (BytesRead<0)
			break;  // write out what we have (adds ".BAD" to filename) - useful for debugging

		if (WorkingString.empty())
		{
			char* Ptr( strstr ((char*)TempStoreForReadData,"\r\n\r\n") ); // can we zero off the last '\n' yet?
			if ( Ptr!=NULL )
			{
				Ptr[3] = 0; // terminate string
				WorkingString = (char*)TempStoreForReadData; // now the header string has a NULL terminator is ok to do this with STD::string
				int Value = GetValueFromHeaderLabel(WorkingString, "Content-Length: ");

				if (Value!=0)
				{
					int size = Value + WorkingString.length() + 1 + MTU;  // ??? for recv to report ZERO it needs eneeded spare buffer at the end??!
					TempStoreForReadData = (u8*)realloc( TempStoreForReadData, size );
					ptrWorking = TempStoreForReadData + TotalReceived;  // recalculate pointer as memory may have been repositioned
					reallocAmount = -1;  // cancel the realloc later on 
#if URL_DEBUG
					char a[128]; sprintf(a,"realloc using Content-Length: value %d, total %d \n", Value , Value + WorkingString.length() + 1) ;
					OutputDebugString( a);
#endif
				}
			}
		}
		ptrWorking += BytesRead;  //5,937,797   // 5,937,412
		TotalReceived += BytesRead; // ptrWorking - TempStoreForReadData;

//#if URL_DEBUG
//		char a[128]; sprintf(a,"%d %p %p\n",(Value+WorkingString.length() + 1)-TotalReceived, ptrWorking , TempStoreForReadData) ;
//		OutputDebugString( a);
//#endif

		if (reallocAmount != -1)  // "Content-Length: " has been found - no need to resize on the run
		{
			if (TotalReceived + MTU > BufferSize)  // will the next read fit?
			{
#if URL_DEBUG
				char a[128]; sprintf(a,"Content-Length: not found yet %d\n",(int)BytesRead) ;
				OutputDebugString( a);
#endif
				BufferSize += reallocAmount ; //1024*4;
				TempStoreForReadData = (u8*)realloc(TempStoreForReadData, BufferSize);  // memory block may be in a new location
				ptrWorking = TempStoreForReadData + TotalReceived;  // recalculate pointer as memory may have been repositioned
			}
		}
	}
	net_close(connection);


	static const int OrigialWasReduceByOneWhenANullWasAdded( 1 );
	int HeaderLength = WorkingString.length() + OrigialWasReduceByOneWhenANullWasAdded;

	int BodyBytes = GetValueFromHeaderLabel(WorkingString, "Content-Length: ");
	if (BodyBytes==0)
		BodyBytes = (TotalReceived - HeaderLength); // in-case we are missing "Content-Length: " from the header section

	u8* BodyData = (u8*) malloc(BodyBytes);
	// if something goes funny (no idea even if it ever can) and say everthing is send OK but some extra data gets plonked at the end
	// This should just ignore rouge data.
	memcpy(BodyData, TempStoreForReadData + HeaderLength, BodyBytes );  // keep a snug fit in memory of the data

	free(TempStoreForReadData);


#if URL_DEBUG
	OutputDebugString("\n-----------------------\n");
	OutputDebugString( WorkingString.c_str()  );
	OutputDebugString("\n-----------------------\n");
	osstream.str("");
	osstream << "total header size: " <<  HeaderLength << " Body size:" << (BodyBytes) << "\n";
	OutputDebugString( osstream.str().c_str()  );
#endif

	MemInfo->SetData( BodyData );
	MemInfo->SetSize( BodyBytes );
	size_t Pos = URI.rfind("/") + 1;
	size_t Length = URI.length() - Pos;
	MemInfo->SetFileNameWithExtension( URI.substr(Pos, Length) );
	if (BytesRead<0)
		MemInfo->SetFileNameWithExtension( URI.substr(Pos, Length) + ".Bad" );

	printf("%s", URI.c_str() );

	return MemInfo;
}

int URLManager::GetValueFromHeaderLabel(string WorkingString, string Label)
{
	string Result = GetStringFromHeaderLabel(WorkingString, Label);
	int Value = atoi(Result.c_str());
	return Value;
}

string URLManager::GetStringFromHeaderLabel(string WorkingString, string Label)
{
	string ContentTypeLabel(Label);
	size_t ContentTypePosition = WorkingString.find(ContentTypeLabel) + ContentTypeLabel.length();
	size_t LFCRPosition = WorkingString.find("\r\n",ContentTypePosition);
	string ContentTypeValue(WorkingString.substr(ContentTypePosition, LFCRPosition - ContentTypePosition));
	return ContentTypeValue;
}



void MemoryInfo::Save(string FullPathFileName)
{
	if ( (m_pData != NULL) && (m_uSize > 0) )
	{
		FILE * pFile( fopen ( FullPathFileName.c_str() , "wb" ) );	
		fwrite (m_pData , 1 , m_uSize , pFile );
		fclose (pFile);
	}
	else
	{
		printf("MemoryInfo failed to save anything");
	}
}

void MemoryInfo::SavePath(string PathName)
{
	Save(PathName + urlDecode( m_FileNameWithExtension ) );
}

string MemoryInfo::urlDecode(string Text) 
{     
	string BuildString;     
	for (string::iterator iter(Text.begin()); iter!=Text.end(); ++iter)
	{         
		char Character( *iter );  
		if (Character == '%')
		{           
			int Temp;     
			sscanf( Text.substr(distance(Text.begin(),iter)+1,2).c_str() , "%x", &Temp);     
			Character = static_cast<char>(Temp); 
			iter += 2;  // fudge, should realy error check this 
		} 
		BuildString += Character;
	}    
	return BuildString; 
} 