# A4 — Baseline Reproduction

> [!NOTE]
> **Lubin OUISSE**  
> E11502007

## Final project revision from A3

---
> I will reproduce here the results from **Analysis of IEEE 802.11e for QoS Support in Wireless LANs** by *Stefan Mangold*, *Sunghyun Choi*, *Guido R. Hiertz*, *Ole Klein*, and *Bernhard Walke*.

## Figure 9 - EDCA throughput vs. number of stations - [EXPERIMENT 2]
### A3 Reminder
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

## Figure 8 - EDCA throughput vs. offered load - [EXPERIMENT 1]
### A3 Reminder
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

### Why Figure 8
