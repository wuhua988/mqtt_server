#include "common/tinyxml2.h"

#include <string>
#include <iostream>

using namespace tinyxml2;

void example1()
{
    XMLDocument doc;
    doc.LoadFile("test.xml");

    XMLElement *mqtt_server = doc.FirstChildElement( "mqtt_server" );
    if (mqtt_server == nullptr)
    {
	std::cout << "Cann't find mqtt_server" << std::endl;
	return;
    }


    std::string  m_str_listen_ip = mqtt_server->FirstChildElement("ip")->GetText();

    std::cout << "ip addr " << m_str_listen_ip << std::endl;
}


int example_3()
{
    static const char* xml =
	"<?xml version=\"1.0\"?>"
	"<!DOCTYPE PLAY SYSTEM \"play.dtd\">"
	"<PLAY>"
	"<TITLE>A Midsummer Night's Dream</TITLE>"
	"</PLAY>";

    XMLDocument doc;
    doc.Parse( xml );

    XMLElement* titleElement = doc.FirstChildElement( "PLAY" )->FirstChildElement( "TITLE" );
    const char* title = titleElement->GetText();
    printf( "Name of play (1): %s\n", title );

    XMLText* textNode = titleElement->FirstChild()->ToText();
    title = textNode->Value();
    printf( "Name of play (2): %s\n", title );

    return doc.ErrorID();
}

#define  ERROR_RETURN(a, b) \
    if ((a))        \
{                   \
    return b;       \
}                   \

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
	fprintf(stderr, "Usage %s xml_file\n", argv[0]);
	return 0;
    }

    XMLDocument doc;
    doc.LoadFile(argv[1]);

    if (doc.ErrorID())
    {
	doc.PrintError();
	fprintf(stderr, "\nErrorid %d, %s\n", doc.ErrorID(),doc.GetErrorStr2());
	return -1;
    }

    XMLElement *mqtt_server = doc.FirstChildElement("mqtt_server");

    ERROR_RETURN(mqtt_server == nullptr, -1);
    ERROR_RETURN(mqtt_server->FirstChildElement("ip") == nullptr, -1);
    ERROR_RETURN(mqtt_server->FirstChildElement("port") == nullptr, -1);


    std::string  m_str_listen_ip = mqtt_server->FirstChildElement("ip")->GetText();
    std::string  str_listen_port = mqtt_server->FirstChildElement("port")->GetText(); 


    std::cout << "ip: " << m_str_listen_ip << "\t port " << str_listen_port << std::endl;

    //example_3();

    return 0;
}
