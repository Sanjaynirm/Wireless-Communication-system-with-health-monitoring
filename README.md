# Wireless-Communication-system-with-health-monitoring
A wireless communication device with PIC18F45K22 and ESP-01 with MPU 6050 for step tracking and MAX30102 for heart rate sensing
We have tried 2 ways to make the communication between 2 devices work
Both the devices are a set of PIC18F45K22, MPU-6050, MAX-30102, ESP-01, Keypad(4X4), LCD, Breadboard, Wires, PICKIT3(for programming)
1st way is by using the AT command approach which comes built in on ESP-01 controllers - PIC18F45K22 is programmed to send AT commands to connect to server and send data with HTTP get and post protocols. Both the ESP-01 are connected to one Wi-Fi and According to the device count(in this 3) the master ESP will have the ip address of the other 2 devices to share data. According to the user who wants to send the message ip will be selected and the data will be sent using HTTP protocols.
2nd way is by pre programming the esp to get the message or data from PIC's USART which will be whom to sent and what to sent, interpreting it and send it to esp's and the recieving side will then take the data sent to pic and pic will then display it on the keypad.
Both approaches are shared on this repo
Thanks :)
