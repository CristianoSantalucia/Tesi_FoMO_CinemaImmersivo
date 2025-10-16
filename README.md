Tesi — Prototipo sperimentale di un'interfaccia per fornire feedback termico come strategia per ridurre la FOMO nel cinema immersivo - Cristiano Santalucia

-------------------------------------------------------------------------------------------------

Questa repository raccoglie i materiali di documentazione di una tesi sperimentale del Politecnico di Torino sul tema della Fear of Missing Out (FoMO) nei video immersivi a 360°.

Il lavoro indaga come segnalazioni direzionali termiche, integrate in un pipeline Arduino ↔ Unity VR, possano aiutare lo spettatore a non perdere eventi narrativi fuori dal campo visivo, preservando immersione e comprensione della storia. 

I file qui presenti servono a contestualizzare il progetto, descriverne obiettivi, premesse di design e metodologia di analisi (metriche e questionari), oltre a documentare test e risultati sintetici.


------------------------------ Documenti inclusi -----------------------------------------------


Abstract della tesi: inquadramento del problema FoMO nel 360° VR, obiettivi sperimentali, ipotesi di lavoro, approccio metodologico e principali risultati/implicazioni. Fornisce una panoramica rapida per chi si avvicina al tema senza conoscenze pregresse.

Sketch Arduino usato nel progetto: gestione di 4 pad termici con controllo PWM, lettura sensori DS18B20 (OneWire/DallasTemperature), calibrazione per-pad con offset (EEPROM), limiti termici min/max (sicurezza), comandi seriali per target e batch, ALL_OFF ed E-Stop con telemetria. È la fonte per comprendere la logica embedded e i segnali scambiati con la parte VR.

Foglio di lavoro per tracciamenti e punteggi sessione/condizione: riepiloghi per video, marcatori temporali, indicatori FoMO/attenzione, campi per note sperimentali e calcoli aggregati. Utile per leggere la struttura dei dati e i confronti tra condizioni.

Raccolta dei questionari e dei relativi calcoli di scoring utilizzati nello studio (es. presenza/immersione, usabilità, comfort indossabile, FoMO). Include inversioni di item, normalizzazioni e output riassuntivi per sezione e globale.

Rilievi e tabelle sul comportamento termico dei pad: tempi di salita/raffreddamento, stabilità attorno ai target, confronto tra pad, note di calibrazione e verifica rispetto ai limiti di sicurezza.

Raccolta di prove e pilot precedenti: configurazioni adottate, parametri, osservazioni qualitative dei partecipanti e sintesi delle modifiche successive. Utile per comprendere le scelte progettuali e l’evoluzione del sistema.
Nota per il lettore

------------------------------ Note -----------------------------------------------------------

I file forniscono contesto, metriche e risultati a supporto del lavoro di tesi; non includono istruzioni operative né dipendenze per la messa in funzione.

I nomi dei fogli/aree nei file Excel possono includere tabelle, intervalli denominati e grafici usati per i calcoli; sono parte integrante della documentazione.
 
