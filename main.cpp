#include "mbed.h"
#include "DHT.h"
#include "EthernetInterface.h"
#include "NetworkAPI/buffer.hpp"
#include "NetworkAPI/ip/address.hpp"
#include "NetworkAPI/tcp/socket.hpp"
#include <string>

using namespace std;

static const int PORT        = 80; 
static const char* host      = "192.168.50.4"; //Host IP
static const char* SERVER_IP = "192.168.50.4"; //IP of server board
static const char* CLIENT_IP = "192.168.50.6"; //IP of client board
static const char* MASK      = "255.255.255.0"; //mask
static const char* GATEWAY   = "192.168.50.4"; //gateway

DHT sensor(D2, DHT22);
DHT sensor2(D3, DHT22);
DHT sensor3(D4, DHT11);
DigitalOut Relay(D5);

EthernetInterface eth;
network::tcp::Socket socket;

int main()
{  
    char buffer[1000];
    int error = 0, error2 = 0, error3 = 0;
    eth.init(CLIENT_IP, MASK, GATEWAY);    
    eth.connect();                                                              
    printf("\nCLIENT - Client IP Address is %s\r\n", eth.getIPAddress()); 
    Relay.write(0);
    
    while(1) 
    {
        socket.open(); 
        if (socket.connect(SERVER_IP, PORT) < 0) 
        {
            printf("Failed to connect with %s\n\r", SERVER_IP);
            printf("Resetting Board \n");
            NVIC_SystemReset();
        }
        else
        {
            wait(2.0f);

            error = sensor.readData();
            error2 = sensor2.readData();
            error3 = sensor3.readData();
            
            if ((0 == error) && (0 == error2) && (0 == error3))
            {
                float c   = sensor.ReadTemperature(CELCIUS);
                float h   = sensor.ReadHumidity();
                
                float c2   = sensor2.ReadTemperature(CELCIUS);
                float h2   = sensor2.ReadHumidity();           
                 
                float c3   = sensor3.ReadTemperature(CELCIUS);
                float h3   = sensor3.ReadHumidity();
                
                if ((h >= 55) || (h2 >= 55))
                {
                    Relay.write(1);
                }   
                else
                {
                    Relay.write(0);     
                }
                
                printf("Temperature 1 : %.2f *C\t", c);
                printf("Humidity    1 : %.2f %%\n", h);
                printf("\n");
                printf("Temperature 2 : %.2f *C\t", c2);
                printf("Humidity    2 : %.2f %%\n", h2);       
                printf("\n");     
                printf("Temperature 3 : %.2f *C\t", c3);
                printf("Humidity    3 : %.2f %%\n", h3);
                printf("\n");
                printf("\n");
                
                snprintf(buffer,1000,"/radhumidity2/input.php?temperature=%.2f&humidity=%.2f&temperature2=%.2f&humidity2=%.2f&temperature3=%.2f&humidity3=%.2f",c,h,c2,h2,c3,h3);
                
                std::string request(string("GET ") + buffer + " HTTP/1.1\r\n" +
                             "Host: " + host + "\r\n" +
                             "Connection: close\r\n\r\n");
                
                if (socket.write((void *)request.data(), request.size()) < 0) 
                {
                    printf("Failed to write HTTP request\n\r");
                    printf("Resetting Board \n");
                    NVIC_SystemReset();
                }
                socket.close();
                wait(3);
            }    

            else 
            {
                printf("Error Sensor 1 : %d\n", error);
                printf("Error Sensor 2 : %d\n", error2);
                printf("Error Sensor 3 : %d\n\n", error3);
                printf("Resetting Board \n");
                NVIC_SystemReset();
            }  
        }
    }
}