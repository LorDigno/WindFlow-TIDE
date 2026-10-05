import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np

# 1. Definizione dei dati (così lo script è indipendente dal file CSV)
data = [
    {"QUERY": "Q2", "PAR": 2, "Throughput_TIDE": 738507.7719197924, "Throughput_WF": 778771.3076989574, "TIDE_efficiency": "94.8%"},
    {"QUERY": "Q2", "PAR": 5, "Throughput_TIDE": 1807753.533616498, "Throughput_WF": 1916138.089670098, "TIDE_efficiency": "94.3%"},
    {"QUERY": "Q3", "PAR": 2, "Throughput_TIDE": 923357.6547344669, "Throughput_WF": 1003868.7424844274, "TIDE_efficiency": "91.9%"},
    {"QUERY": "Q3", "PAR": 3, "Throughput_TIDE": 1342562.947217228, "Throughput_WF": 1367497.289374568, "TIDE_efficiency": "98.1%"},
    {"QUERY": "Q12", "PAR": 2, "Throughput_TIDE": 750881.0325741309, "Throughput_WF": 781394.3632189556, "TIDE_efficiency": "96%"},
    {"QUERY": "Q12", "PAR": 5, "Throughput_TIDE": 1865051.7513351482, "Throughput_WF": 1894773.3353737385, "TIDE_efficiency": "98.4%"},
    {"QUERY": "Q20", "PAR": 2, "Throughput_TIDE": 500127.3204524965, "Throughput_WF": 532530.50, "TIDE_efficiency": "93.9%"},
    {"QUERY": "Q20", "PAR": 3, "Throughput_TIDE": 1026913.6366931547, "Throughput_WF": 1066697.57, "TIDE_efficiency": "96.2%"}
]

df = pd.DataFrame(data)
queries = df['QUERY'].unique()

# 2. Creazione della griglia 2x2
fig, axes = plt.subplots(2, 2, figsize=(14, 12))
axes = axes.flatten()

width = 0.35

# 3. Iterazione su ogni singola query per popolare i 4 grafici
for i, query in enumerate(queries):
    ax = axes[i]
    # Filtriamo i dati per la query corrente
    subset = df[df['QUERY'] == query].reset_index(drop=True)
    
    x = np.arange(len(subset))
    
    # Generazione delle barre per TIDE e WF
    bars_tide = ax.bar(x - width/2, subset['Throughput_TIDE'], width, label='TIDE', color='#4C72B0', edgecolor='black')
    bars_wf = ax.bar(x + width/2, subset['Throughput_WF'], width, label='WF', color='#DD8452', edgecolor='black')
    
    # Formattazione degli assi
    ax.yaxis.set_major_formatter(ticker.FuncFormatter(lambda val, pos: format(int(val), ',')))
    ax.set_title(f'Performance Comparison: {query}', fontsize=14, pad=15)
    ax.set_xticks(x)
    
    # Label asse X con il parametro di parallelismo
    ax.set_xticklabels(['PAR: ' + str(p) for p in subset['PAR']], fontsize=11)
    ax.set_ylabel('Throughput (Tuple/s)', fontsize=11)
    ax.grid(axis='y', linestyle='--', alpha=0.7)
    
    # Calcolo dinamico dell'altezza massima per far respirare i testi sopra le barre
    max_val = max(subset['Throughput_WF'].max(), subset['Throughput_TIDE'].max())
    ax.set_ylim(0, max_val * 1.3)
    
    # Inserimento dei valori esatti sulle barre TIDE
    for bar in bars_tide:
        yval = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2, yval + (max_val * 0.015), f'{int(yval):,}', ha='center', va='bottom', fontsize=9)
        
    # Inserimento dei valori esatti sulle barre WF
    for bar in bars_wf:
        yval = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2, yval + (max_val * 0.015), f'{int(yval):,}', ha='center', va='bottom', fontsize=9)
        
    # Inserimento dell'efficienza al centro sopra le due barre
    for j, row in subset.iterrows():
        max_h = max(row['Throughput_TIDE'], row['Throughput_WF'])
        ax.text(j, max_h + (max_val * 0.12), f"Eff: {row['TIDE_efficiency']}", ha='center', va='bottom', fontsize=11, fontweight='bold', color='#333333', bbox=dict(facecolor='white', alpha=0.6, edgecolor='none', pad=2))

# 4. Aggiunta della legenda globale in alto al centro
handles, labels = axes[0].get_legend_handles_labels()
fig.legend(handles, labels, title='System Version', loc='upper center', bbox_to_anchor=(0.5, 1.05), ncol=2, fontsize=12, title_fontsize=12)

# 5. Ottimizzazione layout e salvataggio
plt.tight_layout(rect=[0, 0, 1, 1]) # rect serve a non far sovrapporre i grafici con la legenda globale
plt.savefig('tide_vs_wf_subplots.png', dpi=300, bbox_inches='tight')
plt.show()