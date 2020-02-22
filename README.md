# UDPDash

## What is this project?
In its current state, this project uses peripheral device LED lighting to indicate the engine RPM of the user's car in the racing simulator Assetto Corsa. Many racing games have built-in UDP servers to interact with a number of different devices that are currently on the market. These servers constantly push out vehicle data such as speed, RPM, lap times, and more. This program communicates with that server and lights up the keyboard to reflect the information that it receives. 

## Why I Began This Project
As a car fanatic, I have played racing games for a very long time. Also, once I learned that AC has the UDP out function, I knew I wanted to do something with it. I chose C++ to be able to keep up with the network traffic generated by AC (it's a lot) and the structures for the UDP data (seen in ACStructs) were already put out by the developers. Python and Java have always been my go-to languages when working with sockets, so it was nice to change things up a bit.

### Main learning experiences from this project:
1. C++ networking using UDP
2. Working with 3rd party libraries (Corsair SDK) 

### Future
This project is still in its early stages. Some ideas for the future include:
* More device compatibility (Logitech + Razer coming soon)
* More games (Priority - Forza. F1 2019 seems to have a nice UDP out system though. Apparently flight simulators have UDP out functionality?)
* Reading from process memory instead of UDP out
* Data logging thread 
* I would like to be able to take the data and either do some data analysis or implement real-time data visualization

## Acknowledgments
Thanks to those at Kunos Simulazioni for making a fantastic game and for documenting their UDP out functionality. Thanks to Corsair for their easy-to-understand documentation for the CUE SDK.