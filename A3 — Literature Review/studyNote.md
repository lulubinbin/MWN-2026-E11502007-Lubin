# A3 — Literature Review

> [!NOTE]
> **Lubin OUISSE**  
> E11502007

## Analysis of IEEE 802.11e for QoS Support in Wireless LANs

> Stefan Mangold, Philips Research  
> Sunghyun Choi, Seoul National University  
> Guido R. Hiertz, Ole Klein, and Bernhard Walke, Aachen University

### Problem
Legacy IEEE 802.11 WLANs offer only best-effort service: the distributed coordination function (DCF) gives all stations identical parameters, so no traffic class can be prioritized, and the point coordination function (PCF) suffers from unpredictable beacon delays and uncontrolled transmission durations of polled stations. As a result, the standard cannot support delay- and jitter-sensitive applications such as voice and video and PCF was never really implemented. 

### Importance
With 802.11 being deployed rapidly in homes, enterprises, and public hotspots, there is growing demand for multimedia applications that require quality of service over wireless networks. The 802.11e extension addresses this by introducing the hybrid coordination function (HCF), with EDCA to provides per-class prioritization through access categories (voice, video, best effort, background) and with polling-based HCCA which enables bounded delays for time-critical traffic via transmission opportunities (TXOPs). Simulations in the paper show that these mechanisms can effectively differentiate traffic and keep delays within limits.

### Challenges
Several open problems remain. As the number of contending stations grows, **collision probabilities** rise sharply — especially in the high-priority voice category, whose small contention windows degrade throughput dramatically unless the coordinator retunes parameters. More critically, in overlapping QBSSs (neighboring networks sharing the same medium), hybrid coordinators **cannot coordinate** with each other, so even the highest-priority polled traffic suffers unpredictable delay and throughput loss. Proposed remedies such as dynamic frequency selection and policy-based coordination were still under discussion in the working group.

### System Model
#### Environment
- An infrastructure QBSS (one AP, which hosts the hybrid coordinator HC, plus associated stations) on an 802.11a PHY at 5 GHz, up to 54 Mb/s.  
- All stations are placed so that nobody is hidden to anybody.  
- A radio error model from the authors' earlier work is used.  
- The paper studies two MAC schemes: EDCA (contention-based, in the CP) and HCCA (HC-controlled polling, in CP and CFP).  

