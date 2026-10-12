import numpy as np, pandas as pd, matplotlib.pyplot as plt

df = pd.read_csv("a2_tx_log.csv")
size = df["size"].values
iat = df.loc[df["gap"] > 0, "gap"].values * 1000  # ms

def tp_cdf(x, k, a, m):
    x = np.clip(x, k, m)
    return (1 - (k / x) ** a) / (1 - (k / m) ** a)

fig, ax = plt.subplots(2, 2, figsize=(11, 8))
for row, (data, name, k, a, m) in enumerate([
        (size, "Packet size (bytes)", 20, 1.2, 250),
        (iat, "Inter-arrival time (ms)", 2.5, 1.2, 12.5)]):
    ax[row, 0].hist(data, bins=50, density=True, alpha=.7)
    ax[row, 0].set(title=f"PDF – {name}", xlabel=name, ylabel="density")
    xs = np.sort(data)
    ax[row, 1].plot(xs, np.arange(1, len(xs) + 1) / len(xs), label="simulated")
    g = np.linspace(k, m, 300)
    ax[row, 1].plot(g, tp_cdf(g, k, a, m), "--", label="theoretical")
    ax[row, 1].set(title=f"CDF – {name}", xlabel=name, ylabel="P(X ≤ x)")
    ax[row, 1].legend()
    print(f"{name}: n={len(data)}, mean={data.mean():.3f}, min={data.min():.3f}, max={data.max():.3f}")
plt.tight_layout()
plt.savefig("a2_distributions.png", dpi=150)
plt.show()