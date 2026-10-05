import json
import os
import subprocess
import argparse
import csv
import time

def estrai_throughput(filepath):
    min_start_us = float('inf')
    max_stop_us = 0
    tuple_input = 0

    try:
        with open(filepath, 'r') as file:
            for linea in file:
                if linea.startswith("[SRC_METRICS]"):
                    dati = json.loads(linea.split(" ", 1)[1].strip())
                    if dati["start_us"] < min_start_us:
                        min_start_us = dati["start_us"]
                    tuple_input += dati["tuples"]

                elif linea.startswith("[SINK_METRICS]"):
                    dati = json.loads(linea.split(" ", 1)[1].strip())
                    if dati["stop_us"] > max_stop_us:
                        max_stop_us = dati["stop_us"]

    except Exception as e:
        print(f"Errore durante la lettura o il parsing: {e}")
        return None

    if min_start_us == float('inf') or max_stop_us == 0:
        return None

    delta_t_secondi = (max_stop_us - min_start_us) / 1_000_000.0
    if delta_t_secondi > 0:
        return tuple_input / delta_t_secondi
    return 0.0

def esegui_benchmark_eseguibile(exe_path, log_path, num_runs, output_csv):
    risultati_run = []
    
    for i in range(1, num_runs + 1):
        print(f"\n--- Esecuzione {i}/{num_runs} ---")
        
        # Pulizia del log di eventuali run precedenti fallite
        if os.path.exists(log_path):
            os.remove(log_path)
            
        # 1. Avvia il processo e ridireziona stdout e stderr sul file
        log_file = open(log_path, 'w')
        try:
            subprocess.Popen([exe_path], stdout=log_file, stderr=subprocess.STDOUT)
        except Exception as e:
            print(f"Errore nell'avvio dell'eseguibile alla run {i}: {e}")
            log_file.close()
            continue

        # 2. Polling: Attesa attiva che l'eseguibile finisca di scrivere
        timeout_secondi = 600  # Timeout di sicurezza (10 minuti)
        tempo_trascorso = 0
        run_completata = False

        print("In attesa del completamento del processo...", end="", flush=True)
        
        while tempo_trascorso < timeout_secondi:
            # Apriamo il file in lettura senza interrompere la scrittura del processo
            if os.path.exists(log_path):
                try:
                    with open(log_path, 'r') as log_lettura:
                        contenuto = log_lettura.read()
                        if "executed successfully" in contenuto:
                            run_completata = True
                            print(" Fatto!")
                            break
                except IOError:
                    pass
            
            time.sleep(1)
            tempo_trascorso += 1
            if tempo_trascorso % 10 == 0:
                print(".", end="", flush=True)

        # 3. Chiusura del file descriptor per rilasciare il lock del SO
        log_file.close()

        if not run_completata:
            print(f"\nErrore: Timeout o crash. La run {i} non ha stampato 'successfully' entro {timeout_secondi}s.")
            continue
            
        # 4. Parsing delle metriche sul file appena chiuso
        throughput = estrai_throughput(log_path)
        
        if throughput is not None:
            print(f"Throughput registrato: {throughput:,.2f} tuple/s")
            risultati_run.append({"run": i, "throughput_tps": throughput})
        else:
            print(f"Attenzione: metriche vuote o non valide alla run {i}.")
            
        # 5. Backup del log per analisi debug future
        backup_path = f"{log_path}.run_{i}.bak"
        os.rename(log_path, backup_path)

    # 6. Salvataggio su CSV e calcolo medie
    if not risultati_run:
        print("\n[ERRORE] Nessun dato valido raccolto in tutte le run.")
        return

    media_throughput = sum(r["throughput_tps"] for r in risultati_run) / len(risultati_run)
    
    with open(output_csv, 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=["run", "throughput_tps"])
        writer.writeheader()
        for r in risultati_run:
            writer.writerow(r)
        
        # Riga riepilogativa finale
        writer.writerow({"run": "Media", "throughput_tps": media_throughput})
            
    print(f"\n=========================================")
    print(f"[OK] Benchmark concluso. Risultati salvati su: {output_csv}")
    print(f"THROUGHPUT MEDIO: {media_throughput:,.2f} tuple/s")
    print(f"=========================================")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Benchmark Eseguibile C++ WindFlow (Modalità Asincrona con Redirect)")
    parser.add_argument("-e", "--exe", required=True, help="Path all'eseguibile (es. ./windflow_app)")
    parser.add_argument("-l", "--log", required=True, help="Path al file di log dove convogliare l'output")
    parser.add_argument("-r", "--runs", type=int, default=20, help="Numero di esecuzioni (default: 20)")
    parser.add_argument("-o", "--output", type=str, default="throughput_results.csv", help="Nome del file CSV di output")
    
    args = parser.parse_args()
    esegui_benchmark_eseguibile(args.exe, args.log, args.runs, args.output)