import json
import os
import subprocess
import argparse
import csv
from collections import defaultdict

def calcola_throughput_windflow(filepath):
    min_start_us = float('inf')
    max_stop_us = 0
    tuple_input = 0
    tuple_output = 0
    
    src_durations_us = []
    sink_durations_us = []
    sink_tuples_per_replica = {}

    try:
        with open(filepath, 'r') as file:
            for linea in file:
                if linea.startswith("[SRC_METRICS]"):
                    dati = json.loads(linea.split(" ", 1)[1])
                    if dati["start_us"] < min_start_us:
                        min_start_us = dati["start_us"]
                    
                    tuple_input += dati["tuples"]
                    src_durations_us.append(dati["stop_us"] - dati["start_us"])

                elif linea.startswith("[SINK_METRICS]"):
                    dati = json.loads(linea.split(" ", 1)[1])
                    if dati["stop_us"] > max_stop_us:
                        max_stop_us = dati["stop_us"]
                    
                    tuple_output += dati["tuples"]
                    sink_durations_us.append(dati["stop_us"] - dati["start_us"])
                    
                    replica_id = dati["replica"]
                    sink_tuples_per_replica[replica_id] = dati["tuples"]

    except Exception as e:
        print(f"Errore durante la lettura o il parsing: {e}")
        return {}

    if min_start_us == float('inf') or max_stop_us == 0:
        return {}

    delta_t_secondi = (max_stop_us - min_start_us) / 1_000_000.0
    throughput_input = tuple_input / delta_t_secondi if delta_t_secondi > 0 else 0.0
    
    if src_durations_us:
        min_src = min(src_durations_us) / 1_000_000.0
        max_src = max(src_durations_us) / 1_000_000.0
        avg_src = (sum(src_durations_us) / len(src_durations_us)) / 1_000_000.0
    else:
        min_src = max_src = avg_src = 0.0

    if sink_durations_us:
        min_sink = min(sink_durations_us) / 1_000_000.0
        max_sink = max(sink_durations_us) / 1_000_000.0
        avg_sink = (sum(sink_durations_us) / len(sink_durations_us)) / 1_000_000.0
    else:
        min_sink = max_sink = avg_sink = 0.0

    risultati = {
        "delta_t_s": delta_t_secondi,
        "tuple_input": tuple_input,
        "tuple_output": tuple_output,
        "throughput_input_tps": throughput_input,
        "src_duration_min_s": min_src,
        "src_duration_avg_s": avg_src,
        "src_duration_max_s": max_src,
        "sink_duration_min_s": min_sink,
        "sink_duration_avg_s": avg_sink,
        "sink_duration_max_s": max_sink
    }
    
    for rep_id, tups in sink_tuples_per_replica.items():
        risultati[f"sink_tuples_replica_{rep_id}"] = tups
        if tuple_output > 0:
            risultati[f"sink_percentage_replica_{rep_id}"] = (tups / tuple_output) * 100
        else:
            risultati[f"sink_percentage_replica_{rep_id}"] = 0.0

    return risultati

def esegui_benchmark_automatico(script_path, log_path, num_runs, output_csv):
    metriche_accumulate = defaultdict(list)
    tutte_le_run = []
    
    for i in range(1, num_runs + 1):
        print(f"--- Esecuzione {i}/{num_runs} ---")
        
        try:
            subprocess.run(["python3", script_path], check=True)
        except subprocess.CalledProcessError as e:
            print(f"Errore: lo script ha fallito alla run {i} ({e}).")
            continue
            
        if not os.path.exists(log_path):
            print(f"Errore: log {log_path} non trovato.")
            continue
            
        metriche_run = calcola_throughput_windflow(log_path)
        
        if metriche_run:
            metriche_run["run"] = i
            tutte_le_run.append(metriche_run)
            for chiave, valore in metriche_run.items():
                if chiave != "run":
                    metriche_accumulate[chiave].append(valore)
        else:
            print(f"Attenzione: metriche vuote alla run {i}.")
            
        backup_path = f"{log_path}.run_{i}.bak"
        os.rename(log_path, backup_path)

    # Calcolo delle medie
    metriche_medie = {"run": "Media"}
    for chiave, lista_valori in metriche_accumulate.items():
        if lista_valori:
            metriche_medie[chiave] = str(sum(lista_valori) / len(lista_valori))

    # Scrittura su CSV
    if tutte_le_run:
        chiavi_csv = set()
        for run_data in tutte_le_run:
            chiavi_csv.update(run_data.keys())
        
        # Ordina le colonne: metti "run" al primo posto
        header = ["run"] + sorted([k for k in chiavi_csv if k != "run"])
        
        with open(output_csv, 'w', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=header)
            writer.writeheader()
            for run_data in tutte_le_run:
                writer.writerow(run_data)
            writer.writerow(metriche_medie)
            
        print(f"\n[OK] Risultati scritti con successo su {output_csv}")

    # Stampa a schermo della media formattata
    metriche_medie_senza_run = {k: v for k, v in metriche_medie.items() if k != "run"}
    print(f"\n--- RISULTATI MEDI ({len(tutte_le_run)} run) ---")
    print(json.dumps(metriche_medie_senza_run, indent=4))

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Automazione Benchmark WindFlow")
    parser.add_argument("-s", "--script", required=True, help="Path allo script da eseguire")
    parser.add_argument("-l", "--log", required=True, help="Path al file di log generato")
    parser.add_argument("-r", "--runs", type=int, default=20, help="Numero di esecuzioni (default: 20)")
    parser.add_argument("-o", "--output", type=str, default="risultati.csv", help="Nome del file CSV di output")
    
    args = parser.parse_args()
    
    esegui_benchmark_automatico(args.script, args.log, args.runs, args.output)