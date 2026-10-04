import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np

# 1. Lettura dal dataset unito (2 split)
df = pd.read_csv('./TIDE-data/q9-runs.csv')

# Formattazione per la legenda dell'asse X includendo entrambi gli split (es. 16, 8)
df['Config'] = 'PAR: ' + df['PAR'].astype(str) + '\nSPLIT: ' + df['SPLIT1'].astype(str) + ', ' + df['SPLIT2'].astype(str)

# 2. Creazione della figura con 2 grafici affiancati
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(18, 7))

# --- PLOT A: Throughput ---
bars_tp = ax1.bar(df['Config'], df['Throughput_tps'], color='#4C72B0', edgecolor='black')
ax1.yaxis.set_major_formatter(ticker.FuncFormatter(lambda x, p: format(int(x), ',')))
ax1.set_xlabel('Configuration (parallelism and split-sizes bid-auction)', fontsize=12)
ax1.set_ylabel('Throughput (Tuple/s)', fontsize=12)
ax1.set_title('A) Throughput', fontsize=14, pad=15)

for bar in bars_tp:
    yval = bar.get_height()
    ax1.text(bar.get_x() + bar.get_width()/2, yval + 20000, f'{int(yval):,}', ha='center', va='bottom', fontsize=10)

ax1.grid(axis='y', linestyle='--', alpha=0.7)
ax1.set_ylim(0, max(df['Throughput_tps']) * 1.15) 

# --- PLOT B: Data Skew (Grouped) ---
threads = ['T1', 'T2', 'T3', 'T4', 'T5']
colors = ['#4C72B0', '#DD8452', '#55A868', '#C44E52', '#8172B3']
x = np.arange(len(df['Config']))
width = 0.15
offsets = [-2 * width, -width, 0, width, 2 * width]

for i, (t, color) in enumerate(zip(threads, colors)):
    bars_sk = ax2.bar(x + offsets[i], df[t], width=width, label=f'Thread {i+1}', color=color, edgecolor='black', alpha=0.85)
    
    for bar in bars_sk:
        h = bar.get_height()
        if h > 0:
            ax2.text(bar.get_x() + bar.get_width() / 2, h + 1.5, f"{int(h)}%", ha='center', va='bottom', fontsize=9, fontweight='bold', color='#333333')

ax2.set_xlabel('Configuration (parallelism and split-sizes bid-auction)', fontsize=12)
ax2.set_ylabel('Tuple at Sink %', fontsize=12)
ax2.set_title('B) Data Skew by Threads', fontsize=14, pad=15)
ax2.set_xticks(x)
ax2.set_xticklabels(df['Config'])
ax2.set_ylim(0, 100)
ax2.grid(axis='y', linestyle='--', alpha=0.5)
ax2.legend(title='Replicas', bbox_to_anchor=(1.05, 1), loc='upper left')

# 3. Ottimizzazione layout e salvataggio
plt.tight_layout()
plt.savefig('q9-graph.png', dpi=300)
plt.show()