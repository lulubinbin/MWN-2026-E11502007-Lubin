# A2 — ns-3 Setup & First Scenario

> [!NOTE]
> **Lubin OUISSE**  
> E11502007

## ns-3 install

### Version

For this install I first intalled the latest version using **homebrew** on my mac. The version was **3.49**. However, since on the [ns-3 website](https://www.nsnam.org/releases/) the latest version was **3.48**, I deleted everything and started again with the package I downloaded from the website. 

> The version I am using now is **3.48 all in one** running on **MacOS 15.8.1**

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

I ran it with ns-3 and it failed at first but after renaming two variables the code produced a `.csv` and two `.pcap` files.  

Here is what it returned in the terminal after running:
```
Flow 1
  Tx packets : 4720
  Rx packets : 4720
  Throughput : 50.0122 kbps
  Mean delay : 10.1281 ms
```

Here is a screenshot of `a2-video-1-0.pcap` (the receiver): 

<img width="1254" height="978" alt="Capture d’écran 2026-10-12 à 23 08 16" src="https://github.com/user-attachments/assets/96277d16-1236-4164-9014-6049b8dd27f8" />


For the analysis, Claude generated a python file to create the graphs. 

For the python code to run, it needed a few libraries: namely numpy, pandas, and matplotlib. For their install, I used `pip` but it complained about an "externally managed environment". 

<img width="1582" height="979" alt="Capture d’écran 2026-10-08 à 15 34 07" src="https://github.com/user-attachments/assets/f0613cca-9437-4a38-91c4-caa2e1913f46" />

I tried to solve the issue using `pipx` but this wasn't fixing the issue because it isolates the libraries. 

In the end, I created a virtual environment which solved the issue. 

The python code returned the following: 
```
Packet size (bytes): n=4720, mean=50.068, min=20.000, max=250.000
Inter-arrival time (ms): n=4130, mean=4.754, min=2.501, max=12.497
```

<img width="1100" height="800" alt="Figure_1" src="https://github.com/user-attachments/assets/70885cff-a150-4cd2-a697-7bfe3565d990" />


## Analysis

**Run results**: All 4,720 packets sent were received, so there was no loss, as expected on an almost idle 5 Mbps link. The mean delay of 10.13 ms is the 10 ms link propagation delay plus about 0.13 ms transmission time, with no measurable queuing.

**Packet size**: The simulated mean is 50.1 B, with a minimum of 20 B and a maximum of 250 B, which are exactly the truncation bounds. The PDF peaks at the minimum and has a long right tail, which is the heavy-tailed Pareto shape. The simulated CDF overlaps the theoretical curve, so the generator draws from the intended distribution.

**Inter-arrival time**: The simulated mean is 4.75 ms, within the 2.5 to 12.5 ms bounds. Only the gaps inside a frame are included. The number of samples is n = 4,130, which is 590 frames × 7 gaps, because the first packet of each frame has no gap. The PDF and CDF again match the theory.

**Conclusion**: The ns-3 build works, and the generator reproduces the intended traffic model. The means are lower than the untruncated Pareto means because the tail is cut off at the maximum. 
