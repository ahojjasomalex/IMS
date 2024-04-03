# Modelling and Simulation scholl project
2022/23 project developed by Alex Bazo and Marek Danco. Project models 6 months in a pottery workshop.

## Prerequisites
For running this project, it is required to have [SIMLIB](https://www.fit.vutbr.cz/~peringer/SIMLIB/) installed at `/usr/local/include/simlib`.

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
Running own experiments can be done by running the program with `./build/pottery` providing command line arguments
```
    -w, --workers            <number of workers (max is 10)>
    -c, --circles            <number of pottery circles used to make clay products>
    -t, --workshift          <[hours] of workshift time each day (max is 24)>
    -l, --clay               <[kilograms] of clay that is available for the duration of simulation>
    -v, --verbose            <verbose logging level to stderr (use this with 2> redirect_file)>
    -x, --furnace-capacity   <number of products that can be baked at the time>
    -z, --workshop-capacity  <number products that can be in workshop at the time>
```
