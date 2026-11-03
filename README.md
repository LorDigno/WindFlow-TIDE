# WindFlow TIDE (Table Interface for Declarative Execution)

**TIDE** è un framework dichiarativo in Python che fornisce un'astrazione relazionale (SQL-like) sopra **WindFlow**, libreria nativa C++17 ad alte prestazioni per il Data Stream Processing su architetture multi-core (*scale-up*).

Il sistema permette di scrivere pipeline di streaming complesse in poche righe di Python: TIDE valida staticamente gli schemi, serializza il piano logico in un AST JSON, genera codice C++17 nativo ottimizzato e ne orchestra la compilazione e l'esecuzione in modo trasparente.

## Struttura del Repository

Il progetto è articolato in tre livelli principali all'interno del package Python (`api`, `codegen`, `runtime`), affiancati dagli header C++ dei builder custom e dalla suite di validazione NEXMark:

```
WindFlow-Table-API/
├── src/
│   └── windflow_tide/  #omettendo gli init
│       | #i seguenti file vengono usati sia da api che da codegen, andrebbero riorganizzati in un modulo a parte
│       ├── expr_ops.py        # Classi per gli operatori su espressioni e typechecking (ExprOp)
│       ├── object_names.py    # Enum varie per garantire la coerenza di nomi fra api e codegen (OpType, ExprType, etc...)
│       ├── times.py           # TimeUnits e TimePolicy
│       ├── types.py           # Gerarchia di DataTypes e TimeFormats sotto TypeDescriptor
|       |
│       ├── api/        # 1. Frontend Dichiarativo (Python DSL)
│       │   ├── draft.py          # Accumulatore temporaneo delle trasformazioni
│       │   ├── durations.py      # Costrutti temporali
│       │   ├── expressions.py    # AST simbolico (col, lit, operatori, aggregazioni) e Type Checker
│       │   ├── file_config.py    # Configurazioni dei file di input
│       │   ├── job_handle.py     # Handle dell'esecuzione del processo cpp
│       │   ├── operators.py      # Operatiri python logici (Unary, Binary, FromOp, TableRefOp)
│       │   ├── schema.py         # Definizione di Field, Schema immutabili e SchemaBuilder
│       │   ├── table.py          # Classi Table e Query
│       │   ├── table_env.py      # TableEnvironment: catalogo, validazione e serializzazione JSON
│       │   └── windows.py        # Costrutti di finestra e intervalli 
│       │
│       ├── codegen/    # 2. Motore di Traduzione (JSON AST -> C++17)
│       │   ├── builder_generator.py     # Offre i metodi che chiamano i templates dei nodi
│       │   ├── code_generator.py        # Entry point del modulo
│       │   ├── explorer.py              # Visita l'AST parsato e istanzia i vari generatori
│       │   ├── expr_translator.py       # Traduzione ricorsiva di espressioni simboliche in codice C++
│       │   ├── lambda_gen.py            # Costruzione delle lambda per funtori e predicati
│       │   ├── operation_nodes.py       # Rappresentazione intermedia degli operatori logici in AST tipizzato
│       │   ├── optimizer.py             # Modulo estendibile con ottimizzazioni sull'AST (per ora identity erasure)
│       │   ├── parser.py                # Deserializzazione JSON e ordinamento topologico del grafo
│       │   ├── schema_gen.py            # Sintesi e deduplicazione struct C++
│       │   ├── utility.py               # Alcune funzioni chiamate da altri file
│       │   └── templates/               # Template Jinja2 (main.cpp.jinja2, structs.hpp.jinja2, ecc.)
│       │
│       └── runtime/    # 3. Gestore di Compilazione ed Esecuzione
│           ├── cmake_manager.py      # Generazione dinamica di CMakeLists.txt
│           ├── compiler.py           # Invocazione sincrona bloccante di CMake e g++/Make
│           └── runner.py             # Esecuzione del binario nativo e gestione del JobHandle
│
| #in include sono presenti anche Windflow e FastFlow 
├── include/Table_Windflow        # Header C++17 che implementano le TableOprations
│   ├── distinct_builder.hpp              # Deduplicazione via unordered_set e una Filter
│   ├── global_group_builder.hpp          # Aggregazioni continue in-stream (wf::Reduce_Builder)
│   ├── helper_functions.hpp              # Funzioni di parsing e materializzazione per Source e Sink
│   ├── intersect_all_builder.hpp         # Intersect All via FlatMap (potrebbe essere una Filter in realtà)
│   ├── intersect_builder.hpp             # Intersect via FlatMap (potrebbe essere una Filter in realtà)
│   ├── select_builder.hpp                # Wrapper su wf::Map_Builder per proiezioni relazionali
│   ├── table_interval_join_builder.hpp   # Wrapper per wf::Interval_Join_Builder
│   ├── table_sink_builder.hpp            # Scrittura CSV formattata e gestione End-Of-Stream
│   ├── table_source_builder.hpp          # Lettura CSV parallela a split, parsing e avanzamento Watermark
│   ├── table_window_join_builder.hpp     # Wrapper per wf::Window_Join_Builder
│   ├── tagged_tuple.hpp                  # Wrapper per il routing su operazioni insiemistiche (per ora solo intersect/all)
│   ├── where_builder.hpp                 # Wrapper su wf::Filter_Builder per predicati
│   ├── windflow_table_api.hpp            # Header ombrello per i sorgenti generati
│   └── windowed_group_builder.hpp        # Aggregazioni a finestra su wf::Keyed_Windows_Builder
│
├── graphs/                       # Dati e script per la generazione dei grafici
├── nexmark_queries/              # Suite di benchmark NEXMark (script TIDE)
├── native_benchmarks/            # Suite di benchmark NEXMark (applicazioni WF native)
├── pyproject.toml                # Configurazione del package Python e dipendenze
└── README.md
```

## Prerequisiti di Sistema

Per compilare ed eseguire le pipeline generate da TIDE, l'ambiente di sviluppo deve soddisfare i seguenti requisiti software e di libreria:
Backend C++ e Compilazione

    Compilatore C++17: g++ (≥9.0, testato e validato con GCC 13.3) o clang++ equivalente.   
    Tesi_Lorenzoni_2026.pdf

    Build System: CMake (≥3.15) e Make (oppure Ninja).

    Librerie di Supporto (Header-Only):

        FastFlow: runtime a memoria condivisa basato su code non bloccanti.

        WindFlow: motore C++17 nativo per streaming multicore ad alte prestazioni.

    Librerie di Sistema: pthread (necessaria per il threading POSIX nativo di FastFlow e WindFlow).

Frontend Python e Generazione Codice

    Python: versione ≥3.10 (ambiente di riferimento testato su Python 3.12.3).   

    Dipendenze Python: Jinja2 (utilizzata dal motore di templating per sintetizzare i sorgenti C++). 
    (dovrebbe installarsi alla clonazione del repo)

Installazione e Configurazione
I seguenti passaggi vanno fatti una volta clonato il repo. Nel caso non si volesse clonare tutto bastano i contenuti di include/ e di windflow_tide/ con ovviamentre le varie dipendenze specificate nel toml. 

1. Ambiente Virtuale Python
Si consiglia di isolare le dipendenze in un virtual environment dedicato:
```
python3 -m venv .venv
source .venv/bin/activate
```
2. Installazione del Package in Modalità Sviluppatore
Installa il pacchetto windflow_tide in modalità editable (-e), in modo che qualsiasi modifica all'API o ai template Jinja2 si rifletta immediatamente a runtime senza dover reinstallare il modulo:
```
pip install -e .
```

## Guida all'Uso: Composizione ed Esecuzione di una Query

La struttura di uno script TIDE, la semantica e sintassi dei suoi costrutti sono ben delimitati e scpiegati nella tesi.



Per scrivere delle nuove query si consigliano i seguenti import:
```
from windflow_tide import *
from pathlib import Path
```
Al momento della scrittura devo ancora sistemare per bene tutti gli init che sono un po' disordinati, in ogni caso l'utente non dovrebbe mai utilizzare classi al di fuori del sottomodulo api/.

Scritto il file python della query e inizializzato l'ambiente virtuale basta eseguire con:
```
python3 example_query.py
```


