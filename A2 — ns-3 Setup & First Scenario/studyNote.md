# A2 — ns-3 Setup & First Scenario

> [!NOTE]
> **Lubin OUISSE**  
> E11502007

## ns-3 install

### Version

For this install I first intalled the latest version using **homebrew** on my mac. The version was **3.49**. However, since on the [ns-3 website](https://www.nsnam.org/releases/) the latest version was **3.48**, I deleted everything and started again with the package I downloaded from the website. 

> The version I am using now is 3.48 all in one. 

### Build and tests

I then followed the README instructions and built succesfully ns-3. The only problem I had was that I didn't have CMake but this was easily fixed. 

For good measure, I followed the instruction to test the build using `test.py`. As expected, all 1000 tests passed. 

## 3GPP Traffic model

Using Claude, I generated a 3GPP traffic model: 

> **Classic 3GPP/NGMN video streaming model**:
> - A frame every 100 ms, with 8 packets per frame
> - Packet size is truncated Pareto (k = 20 B, α = 1.2, max 250 B).
> - Packet inter-arrival time within a frame is truncated Pareto (k = 2.5 ms, α = 1.2, max 12.5 ms).  

## Running the model

I ran it with ns-3 and it failed at first but after renaming two variables the code produced a ```.csv``` and two ```.pcap``` files.  

Here is what it returned in the terminal after running:
```
Flow 1
  Tx packets : 4720
  Rx packets : 4720
  Throughput : 50.0122 kbps
  Mean delay : 10.1281 ms
```

For the analysis, Claude generated a python file to create the graphs. 

For the python code to run, it needed a few libraries: namely numpy, pandas, and matplotlib. For their install, I used ```pip``` but it complained about an "externally managed environment". 

photo  

I tried to solve the issue using ```pipx``` but this wasn't fixing the issue because it isolates the libraries. 

In the end, I created a virtual environment which solved the issue. 

The python code returned the following: 
```
Packet size (bytes): n=4720, mean=50.068, min=20.000, max=250.000
Inter-arrival time (ms): n=4130, mean=4.754, min=2.501, max=12.497
```

photo matplotlib

## Analysis

