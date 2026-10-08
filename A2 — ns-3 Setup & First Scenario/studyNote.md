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




Flow 1
  Tx packets : 4720
  Rx packets : 4720
  Throughput : 50.0122 kbps
  Mean delay : 10.1281 ms



Packet size (bytes): n=4720, mean=50.068, min=20.000, max=250.000
Inter-arrival time (ms): n=4130, mean=4.754, min=2.501, max=12.497

