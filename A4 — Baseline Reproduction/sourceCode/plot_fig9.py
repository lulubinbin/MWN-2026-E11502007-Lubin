import csv
import re
import subprocess
from pathlib import Path
import matplotlib.pyplot as plt

# Station counts on the x-axis of Figure 9
STATION_COUNTS = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 16]
ACS = ["AC_VO", "AC_VI", "AC_BE", "AC_BK"]
LABELS = {
    "AC_VO": "AC_VO (voice)",
    "AC_VI": "AC_VI (video)",
    "AC_BE": "AC_BE (best effort)",
    "AC_BK": "AC_BK (background)",
}
MARKERS = {
    "AC_VO": "x",
    "AC_VI": "o",
    "AC_BE": "+",
    "AC_BK": "d",
}

NS3_DIR = Path("/opt/homebrew/Cellar/ns-allinone-3.48/ns-3.48")
LINE_RE = re.compile(r"^\s*(AC_(?:VO|VI|BE|BK))\s+throughput:\s+([0-9]+(?:\.[0-9]+)?)\s+Mb/s\s*$")


def run_simulation(n_stas: int, sim_time: float = 10.0, warmup_time: float = 1.0, run: int = 1) -> dict[str, float]:
    cmd = [
        "./ns3",
        "run",
        "--no-build",
        f"fig9-edca-throughput --nStas={n_stas} --simTime={sim_time} --warmupTime={warmup_time} --run={run}",
    ]
    try:
        proc = subprocess.run(cmd, cwd=NS3_DIR, text=True, capture_output=True, check=True)
    except subprocess.CalledProcessError as exc:
        raise RuntimeError(
            f"ns-3 failed ({exc.returncode}) for nStas={n_stas}\n"
            f"STDOUT:\n{exc.stdout}\nSTDERR:\n{exc.stderr}"
        ) from exc

    results = {m.group(1): float(m.group(2)) for line in proc.stdout.splitlines() if (m := LINE_RE.match(line))}
    if len(results) != len(ACS):
        raise RuntimeError(f"Unexpected output for nStas={n_stas}:\n{proc.stdout}\n{proc.stderr}")
    return results


def main() -> None:
    subprocess.run(["./ns3", "build", "fig9-edca-throughput"], cwd=NS3_DIR, check=True)

    series = {ac: [] for ac in ACS}
    csv_path = Path("fig9_results.csv")
    with csv_path.open("w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["n_stas", "offered_per_ac_mbps", *ACS])

        for n in STATION_COUNTS:
            res = run_simulation(n)
            offered_mbps = n * 0.25  # 250 kb/s per AC per station
            print(f"nStas={n:2d} (offer={offered_mbps:4.2f} Mb/s) -> " + ", ".join(f"{ac}={res[ac]:.3f}" for ac in ACS))
            writer.writerow([n, offered_mbps, *(res[ac] for ac in ACS)])
            for ac in ACS:
                series[ac].append(res[ac])

    print(f"Saved CSV results to {csv_path.resolve()}")

    plt.figure(figsize=(7.5, 5.5))
    offered_line = [n * 0.25 for n in STATION_COUNTS]
    plt.plot(STATION_COUNTS, offered_line, linestyle="--", color="black", linewidth=1.0, label="Total offer per AC")

    for ac in ACS:
        plt.plot(
            STATION_COUNTS,
            series[ac],
            marker=MARKERS[ac],
            linestyle="-",
            linewidth=1.5,
            label=LABELS[ac],
        )

    plt.title("Throughput per AC vs. Number of Stations (802.11a EDCA, 24 Mb/s)")
    plt.xlabel("Number of transmitting stations, each offering 4 x 250 kb/s (1 Mb/s)")
    plt.ylabel("Throughput per AC (Mb/s)")
    plt.xticks(STATION_COUNTS)
    plt.xlim(0.5, 16.5)
    plt.ylim(0, 14)
    plt.grid(True, linestyle=":", alpha=0.6)
    plt.legend(loc="upper left")
    plt.tight_layout()

    out_png = Path("fig9_edca_throughput.png")
    plt.savefig(out_png, dpi=300)
    print(f"Saved plot to {out_png.resolve()}")
    plt.show()


if __name__ == "__main__":
    main()
