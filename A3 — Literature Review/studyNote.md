# A3 — Literature Review

> [!NOTE]
> **Lubin OUISSE**  
> E11502007

# Analysis of IEEE 802.11e for QoS Support in Wireless LANs

> Stefan Mangold, Philips Research  
> Sunghyun Choi, Seoul National University  
> Guido R. Hiertz, Ole Klein, and Bernhard Walke, Aachen University

## Problem
Legacy IEEE 802.11 WLANs offer only best-effort service: the distributed coordination function (DCF) gives all stations identical parameters, so no traffic class can be prioritized, and the point coordination function (PCF) suffers from unpredictable beacon delays and uncontrolled transmission durations of polled stations. As a result, the standard cannot support delay- and jitter-sensitive applications such as voice and video and PCF was never really implemented. 

## Importance
With 802.11 being deployed rapidly in homes, enterprises, and public hotspots, there is growing demand for multimedia applications that require quality of service over wireless networks. The 802.11e extension addresses this by introducing the hybrid coordination function (HCF), with EDCA to provides per-class prioritization through access categories (voice, video, best effort, background) and with polling-based HCCA which enables bounded delays for time-critical traffic via transmission opportunities (TXOPs). Simulations in the paper show that these mechanisms can effectively differentiate traffic and keep delays within limits.

## Challenges
Several open problems remain. As the number of contending stations grows, **collision probabilities** rise sharply — especially in the high-priority voice category, whose small contention windows degrade throughput dramatically unless the coordinator retunes parameters. More critically, in overlapping QBSSs (neighboring networks sharing the same medium), hybrid coordinators **cannot coordinate** with each other, so even the highest-priority polled traffic suffers unpredictable delay and throughput loss. Proposed remedies such as dynamic frequency selection and policy-based coordination were still under discussion in the working group.

## System Model and Environment
- An infrastructure QBSS (one AP, which hosts the hybrid coordinator HC, plus associated stations) on an 802.11a PHY at 5 GHz, up to 54 Mb/s.  
- All stations are placed so that nobody is hidden to anybody.  
- A radio error model from the authors' earlier work is used.  
- The paper studies two MAC schemes: EDCA (contention-based, in the CP) and HCCA (HC-controlled polling, in CP and CFP).

## Experiments

### Experiment 1 : EDCA throughput vs. offered load (Fig. 8)
#### Inputs

| Item | Setting |
| --- | --- |
| Topology | 1 AP + 3 stations, AP sends 12 downstream streams (one per AC to each station) |
| Traffic | 512-byte MSDUs, negative-exponential inter-arrivals, 250 kb/s per stream |
| Offered load | Swept per AC from 0 to 30 Mb/s |
| EDCA parameters | Table 1 defaults (draft 802.11e/D5.0): AC_VO 2/3/7, AC_VI 2/7/15, AC_BE 3/15/1023, AC_BK 7/15/1023 (AIFSN/CWmin/CWmax) |
| PHY/MAC rates | All frames (data, control) at 24 Mb/s |
| Other MAC settings | One data frame per EDCA-TXOP, no RTS/CTS, no fragmentation, no beacons |

#### Outputs

- Throughput per AC (Mb/s) as a function of offered traffic per AC.

#### Results

- At low load, every AC carries its offered traffic.
- At saturation, strict priority ordering emerges: AC_VO ≈ 12 Mb/s, AC_VI ≈ 6 Mb/s, AC_BE ≈ 4 Mb/s, AC_BK ≈ 1 Mb/s.
- Higher-priority ACs (smaller AIFSN, CWmin, CWmax) starve the lower ones; the total channel capacity is shared according to the EDCA parameter ratios.

---

### Experiment 2 : EDCA throughput vs. number of stations (Fig. 9)

#### Inputs

| Item | Setting |
| --- | --- |
| Topology | N contending stations (N = 1…16) around the AP, every station transmits |
| Traffic | Same model: 512-byte MSDUs, exponential inter-arrivals. 4 streams per station (one per AC) at 250 kb/s each = 1 Mb/s per station |
| EDCA parameters | Same Table 1 defaults as Experiment 1 |
| PHY/MAC rates | All frames at 24 Mb/s |
| Other MAC settings | One data frame per EDCA-TXOP, no RTS/CTS, no fragmentation; no beacons |

#### Outputs

- Throughput per AC (Mb/s) as a function of the number of stations.

#### Results

- Per-AC throughput peaks around N = 4: AC_VO ≈ 4, AC_VI ≈ 3, AC_BE ≈ 2, AC_BK ≈ 1 Mb/s.
- After that, throughput collapses toward zero at N = 16, because collision probability grows with the number of contending backoff entities.
- The collapse is worst for AC_VO: its tiny CWmin = 3 / CWmax = 7 gives little protection against collisions at high contention.
- Authors' interpretation: with default parameters, EDCA does not scale to hotspots; the HC should dynamically re-set the EDCA parameters, and real voice load per station would be far lower than the 250 kb/s tested here.

---

### Experiment 3 : Delay guarantees with HCCA polling (Fig. 10)

#### Inputs

| Item | Setting |
| --- | --- |
| Topology | One isolated QBSS; plus a second variant with two overlapping QBSSs interfering with each other |
| Station traffic | AC_H: 80-byte MSDUs at constant periods (5 ms = 128 kb/s, voice-like); AC_M and AC_L: 200-byte MSDUs, exponential inter-arrivals, 160 kb/s each |
| HC traffic | Isochronous 128 kb/s downstream (80-byte MSDUs) sent by the AP at HCCA priority: transmitted after a PIFS idle period, no backoff |
| EDCA parameters | Different set for this study: AC_H 2/7/7, AC_M 4/10/31, AC_L 7/15/255 (AIFSN/CWmin/CWmax) |
| PHY/MAC rates | Data at 24 Mb/s, control frames (ACKs) at 6 Mb/s |
| Delay bound | HC aims to keep HCCA MSDU delivery delay below 300 µs via TXOPlimit |

#### Outputs

- MSDU delivery delay as complementary CDFs, P{delay > x}, for the HCCA stream and for AC_M (EDCA), in the isolated and the overlapping-QBSS cases.

#### Results

- Isolated QBSS: EDCA (AC_M) delays grow unpredictably as offered traffic increases, HCCA delays stay bounded below the target (≈ 300 µs), and remain bounded even under increased load. Only the HCCA stream guarantees a maximum delay.
- Overlapping QBSSs: even the highest-priority HCCA stream exceeds its delay bound, because the two HCs share the medium without coordination. Worst case: if both HCs poll at the same time, all poll frames collide and HCCA throughput drops to zero.
- Conclusion: HCCA provides time-bounded delivery only when all QBSSs in radio range follow its coordination; the overlapping-QBSS problem remained open (candidate solutions: dynamic frequency selection, coordination policies).

