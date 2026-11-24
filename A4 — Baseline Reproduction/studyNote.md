# A4 — Baseline Reproduction

> [!NOTE]
> **Lubin OUISSE**
> E11502007

## Final project revision from A3

**Problem Identified (Figure 9):** Under multi-station contention (`N` → 16), the default IEEE 802.11e EDCA parameter set (Table 1) causes severe throughput degradation—particularly for `AC_VO` (`CWmin = 3`, `CWmax = 7`), whose small contention window leads to frequent collisions as the number of contending stations grows.

**Proposed Modification:** Following the authors' recommendation that the Hybrid Coordinator (HC) adapt EDCA parameters to network load, I will modify the EDCA parameter policy (scaling `CWmin` / `CWmax` and/or `AIFSN` with station count `N`) and evaluate it against our A4 baseline in `ns-3.48` on per-AC throughput, aggregate throughput, and packet loss, while verifying on Figure 8 that strict priority differentiation is preserved.

---

> I will reproduce here the results from **Analysis of IEEE 802.11e for QoS Support in Wireless LANs** by *Stefan Mangold*, *Sunghyun Choi*, *Guido R. Hiertz*, *Ole Klein*, and *Bernhard Walke*.

## Figure 9 — EDCA Throughput vs. Number of Stations

### Simulation Setup & Reproducibility Parameters

| Parameter | Setting |
| :--- | :--- |
| **Simulator & Script** | `ns-3.48` (`scratch/fig9-edca-throughput.cc`) |
| **Topology** | 1 AP at `(0, 0, 0)` m, `N` STAs on a 5 m circle (no hidden nodes) |
| **PHY / MAC** | IEEE 802.11a (5 GHz), `ConstantRateWifiManager` (`OfdmRate24Mbps`), RTS/CTS disabled, `TxopLimit = 0 µs` |
| **EDCA (Table 1)** | `AC_VO` (`0xc0`): 2/3/7 · `AC_VI` (`0x80`): 2/7/15 · `AC_BE` (`0x00`): 3/15/1023 · `AC_BK` (`0x20`): 7/15/1023 (`AIFSN`/`CWmin`/`CWmax`) |
| **Traffic Model** | `4N` UDP uplink flows (4 per STA, 1 per AC), 250 kb/s per stream (1 Mb/s per STA), 512-byte payload, `OnOffApplication` (`OnTime ~ Exp(0.05 s)`, `OffTime = 0 s`) |
| **Station Sweep** | `N` ∈ `{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 16}` stations |
| **Duration & RNG** | 10 s sim (1 s warmup, 9 s measured), `RngSeed = 1`, `RngRun = 1`, 1 run per point |

### Results Comparison

| Metric / Access Category | Original Paper | `ns-3.48` Reproduction |
| :--- | :--- | :--- |
| **Peak `AC_VO` (Voice)** | ~4.7 Mb/s (`N = 5`) → 0.4 Mb/s (`N = 16`) | ~2.9 Mb/s (`N = 14`) → 2.8 Mb/s (`N = 16`) |
| **Peak `AC_VI` (Video)** | ~3.9 Mb/s (`N = 4`) → 0.1 Mb/s (`N = 16`) | ~3.3 Mb/s (`N = 14`) → 3.2 Mb/s (`N = 16`) |
| **Peak `AC_BE` (Best Effort)** | ~3.3 Mb/s (`N = 4`) → 0.0 Mb/s (`N = 16`) | ~2.9 Mb/s (`N = 11`) → 0.3 Mb/s (`N = 16`) |
| **Peak `AC_BK` (Background)** | ~2.8 Mb/s (`N = 3`) → 0.0 Mb/s (`N = 16`) | ~2.3 Mb/s (`N = 9`) → 0.0 Mb/s (`N = 16`) |

### Explanation of Differences

While all four ACs match offered load at low contention (`N` ≤ 8) and lower-priority traffic (`AC_BK`, `AC_BE`) drops off as contention increases, overall throughput is lower and saturates later than in Mangold et al. (Figure 9) due to:

1. **Offered Load Scaling:** The script follows the paper's text (`250 kb/s` per stream = `0.25 * N` Mb/s per AC, reaching `4.0 Mb/s` at `N = 16`), whereas the dashed offer line plotted in Figure 9 of the paper rises faster (`~4 Mb/s` at `N = 4` and `~9 Mb/s` at `N = 10`), shifting saturation later.
2. **MAC Version & Framing:** `ns-3.48` implements ratified 802.11 EDCA with full headers and periodic beacons, whereas the paper used pre-standard `802.11e/D5.0` without beacons and a custom 5 GHz error model.
3. **Traffic Arrivals & Contention:** `OnOffApplication` (`OffTime = 0 s`) paces packets rather than pure single-packet exponential inter-arrivals, and at high station counts (`N = 10 ... 16`) `AC_VO`'s small contention window (`3/7`) suffers frequent collisions alongside `AC_VI` (`7/15`).
4. **Sample Size:** I ran 1 seed (`RngRun = 1`) over 9 s per point rather than multi-run LRE stopping (< 5% relative error).

## Figure 8 — EDCA Throughput vs. Offered Load (Baseline)

### Why Figure 8

Figure 8 validates that the `ns-3.48` EDCA configuration correctly enforces priority differentiation (`AC_VO` > `AC_VI` > `AC_BE` > `AC_BK`) when there is no multi-station contention. For the final project where I modify the EDCA parameter set to prevent the collision-induced throughput collapse seen in Figure 9, Figure 8 serves as a sanity check to ensure that tuning `CWmin`, `CWmax`, or `AIFSN` preserves strict QoS prioritization under varying offered loads.

### Simulation Setup & Reproducibility Parameters

| Parameter | Setting |
| :--- | :--- |
| **Simulator & Script** | `ns-3.48` (`scratch/fig8-edca-throughput.cc`) |
| **Topology** | 1 AP at `(0, 0, 0)` m, 3 STAs at 5 m radius (no hidden nodes) |
| **PHY / MAC** | IEEE 802.11a (5 GHz), `ConstantRateWifiManager` (`OfdmRate24Mbps`), RTS/CTS disabled, `TxopLimit = 0 µs` |
| **EDCA (Table 1)** | `AC_VO` (`0xc0`): 2/3/7 · `AC_VI` (`0x80`): 2/7/15 · `AC_BE` (`0x00`): 3/15/1023 · `AC_BK` (`0x20`): 7/15/1023 (`AIFSN`/`CWmin`/`CWmax`) |
| **Traffic Model** | 12 UDP downlink flows (3 per AC), 512-byte payload, `OnOffApplication` (`OnTime ~ Exp(0.05 s)`, `OffTime = 0 s`) |
| **Offered Load Sweep** | `{0, 1.5, 3, 4.5, 6, 7.5, 9, 12, 15, 18, 24, 30}` Mb/s per AC |
| **Duration & RNG** | 10 s sim (1 s warmup, 9 s measured), `RngSeed = 1`, `RngRun = 1`, 1 run per point |

### Results Comparison (Saturation ≥ 15 Mb/s)

| Access Category | Original Paper | `ns-3.48` Reproduction |
| :--- | :--- | :--- |
| **`AC_VO` (Voice)** | ~12.8 Mb/s | ~11.2 Mb/s |
| **`AC_VI` (Video)** | ~1.3 Mb/s | ~2.8 Mb/s |
| **`AC_BE` (Best Effort)** | ~0.0 Mb/s | ~0.1 Mb/s |
| **`AC_BK` (Background)** | ~0.0 Mb/s | ~0.0 Mb/s |
| **Aggregate Total** | ~14.1 Mb/s | ~14.1 Mb/s |

### Explanation of Differences

While total channel capacity (~14.1 Mb/s) and the priority starvation order (`AC_BK` → `AC_BE` → `AC_VI` → `AC_VO`) match, `AC_VO` is ~1.6 Mb/s lower and `AC_VI` is ~1.5 Mb/s higher at saturation due to:

1. **MAC Version & Framing:** `ns-3.48` implements ratified 802.11 EDCA with full QoS/LLC/IP/UDP headers and periodic beacons, whereas the paper used pre-standard `802.11e/D5.0` without beacons.
2. **Traffic Arrivals:** `OnOffApplication` generates active bursts rather than pure single-packet Poisson MSDU arrivals, slightly smoothing the low-load drop-off.
3. **Sample Size:** I ran 1 seed (`RngRun = 1`) over 9 s per point rather than multi-seed LRE stopping (< 5% relative error).
