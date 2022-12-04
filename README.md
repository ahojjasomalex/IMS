# IMS PROJECT
IMS 2022/23 project developed by Alex Bazo and Marek Danco. Project models 6 months in a pottery workshop.
## Installation
Unpack the archive and run:
```bash
make
```
## Usage
To run predetermined experiments use:
```bash
make run
```
Running own experiments can be done by providing command line arguments
```
-w  <number of workers (max is 10)>
-c  <number of pottery circles used to make clay products (1 worker can use 1 circle at a time)>
-t  <[hours] of workshift time each day (max is 24)>
-l  <[kilograms] of clay that is available for the duration of simulation>
-v  <verbose logging level to stderr (use this with "2> redirect_file")> 
```
