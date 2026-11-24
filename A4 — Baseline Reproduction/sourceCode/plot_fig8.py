import re
import subprocess
from pathlib import Path
import matplotlib.pyplot as plt

# X-axis ticks visible in Figure 8 (offered traffic per AC in Mb/s)
OFFERED_RATES = [0, 1.5, 3, 4.5, 6, 7.5, 9, 12, 15, 18, 24, 30]
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


def run_simulation(offered_mbps: float, sim_time: float = 10.0, warmup_time: float = 1.0) -> dict[str, float]:
    cmd = [
        "./ns3",
        "run",
        "--no-build",
        f"fig8-edca-throughput --offeredPerAcMbps={offered_mbps} --simTime={sim_time} --warmupTime={warmup_time}",
    ]
    try:
        proc = subprocess.run(cmd, cwd=NS3_DIR, text=True, capture_output=True, check=True)
    except subprocess.CalledProcessError as exc:
        raise RuntimeError(
            f"ns-3 failed ({exc.returncode}) for offeredPerAcMbps={offered_mbps}\n"
            f"STDOUT:\n{exc.stdout}\nSTDERR:\n{exc.stderr}"
        ) from exc
    results = {m.group(1): float(m.group(2)) for line in proc.stdout.splitlines() if (m := LINE_RE.match(line))}
    if len(results) != len(ACS):
        raise RuntimeError(f"Unexpected output for {offered_mbps} Mb/s:\n{proc.stdout}\n{proc.stderr}")
    return results


def main() -> None:
    # Build once first so --no-build runs fast inside the sweep loop
    subprocess.run(["./ns3", "build", "fig8-edca-throughput"], cwd=NS3_DIR, check=True)

    series = {ac: [] for ac in ACS}
    for rate in OFFERED_RATES:
        res = run_simulation(rate)
        print(f"Offered={rate:4.1f} Mb/s -> " + ", ".join(f"{ac}={res[ac]:.3f}" for ac in ACS))
        for ac in ACS:
            series[ac].append(res[ac])

    plt.figure(figsize=(7.5, 5.5))
    plt.plot(OFFERED_RATES, OFFERED_RATES, linestyle="--", color="black", linewidth=1.0, label="Total offer per AC")

    for ac in ACS:
        plt.plot(
            OFFERED_RATES,
            series[ac],
            marker=MARKERS[ac],
            linestyle="-",
            linewidth=1.5,
            label=LABELS[ac],
        )

    plt.title("Throughput per AC (802.11a EDCA, 1 AP -> 3 STAs, 24 Mb/s)")
    plt.xlabel("Offered traffic per AC (3 streams each) (Mb/s)")
    plt.ylabel("Throughput per AC (3 streams each) (Mb/s)")
    plt.xlim(0, 30)
    plt.ylim(0, 14)
    plt.grid(True, linestyle=":", alpha=0.6)
    plt.legend(loc="upper left")
    plt.tight_layout()

    out_png = Path("fig8_edca_throughput.png")
    plt.savefig(out_png, dpi=300)
    print(f"Saved plot to {out_png.resolve()}")
    plt.show()


if __name__ == "__main__":
    main()