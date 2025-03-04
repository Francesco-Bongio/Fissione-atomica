# ⚙️ Sistemi Operativi - Simulazione di Processi Concorrenti

## 📌 Descrizione

Questo progetto implementa una simulazione basata su **processi concorrenti** in **C**. Il sistema è composto da diversi processi che interagiscono tra loro utilizzando **memoria condivisa**, **semafori** e **segnali UNIX**.

### 🔍 Funzionamento Generale

Il programma simula un sistema in cui gli **atomi** si dividono e generano energia, mentre un **inibitore** controlla la produzione energetica per evitare esplosioni. I processi comunicano tramite **segnali (SIGUSR1, SIGUSR2, SIGTERM)** e condividono dati attraverso la **memoria condivisa**.

---

## 🚀 Tecnologie Utilizzate

- **Linguaggio C** (per la programmazione di basso livello)
- **Processi UNIX** (`fork`, `exec`, `wait`)
- **Memoria Condivisa** (`shmget`, `shmat`)
- **Semafori** (`semop`)
- **Segnali UNIX** (`SIGUSR1`, `SIGUSR2`, `SIGTERM`, `SIGALRM`)
- **Makefile** (per la compilazione automatizzata)

---

## 📂 Struttura del Progetto

```
sistemi-operativi/
│── src/
│   ├── master.c         # Processo principale che gestisce la simulazione
│   ├── inibitore.c      # Processo che assorbe energia e previene esplosioni
│   ├── attivatore.c     # Processo che attiva la divisione degli atomi
│   ├── alimentazione.c  # (Da verificare, potrebbe gestire il mantenimento energetico)
│   ├── atomo.c          # Processo che rappresenta un atomo, che può dividersi o esplodere
│   ├── common.h         # Strutture dati e costanti condivise
│   ├── alimentazione.h  # Header file per il modulo alimentazione
│   ├── attivatore.h     # Header file per il modulo attivatore
│── settings.txt         # File di configurazione con i parametri della simulazione
│── Makefile             # File per la compilazione del progetto
│── README.md            # Documentazione del progetto
```

---

## 🔧 Installazione e Utilizzo

### 1️⃣ Compilazione del progetto

Assicurati di avere un compilatore **GCC** installato, quindi esegui:

```sh
make
```

Questo genererà gli eseguibili necessari.

### 2️⃣ Avvio della simulazione

```sh
./master
```

Il processo `master` avvierà gli altri processi (`atomo`, `attivatore`, `inibitore`, ecc.) e gestirà la simulazione.

### 3️⃣ Terminazione della simulazione

Per terminare il sistema in modo controllato:

```sh
killall master inibitore attivatore atomo
```

---

## 🛠️ Flusso di Funzionamento

1. **Avvio del processo **``
   - Legge le impostazioni dal file `settings.txt`
   - Inizializza la **memoria condivisa** e i **semafori**
   - Crea i processi **atomo**, **inibitore** e **attivatore**
2. **Esecuzione dei processi**
   - Gli **atomi** si dividono e generano energia.
   - L’**attivatore** invia segnali per stimolare la divisione.
   - L’**inibitore** assorbe energia in base alla soglia configurata.
3. **Gestione della comunicazione**
   - I processi usano **segnali UNIX** per coordinarsi.
   - Dati e variabili condivise sono accessibili tramite **memoria condivisa**.
4. **Chiusura controllata**
   - Il master riceve il segnale di terminazione (`SIGTERM`).
   - Tutti i processi vengono terminati in modo sicuro.

---

## ✅ Funzionalità

✔️ Simulazione di un sistema con processi concorrenti\
✔️ Gestione avanzata tramite memoria condivisa\
✔️ Sincronizzazione con semafori UNIX\
✔️ Comunicazione tra processi con segnali\
✔️ Configurazione tramite file esterno\
✔️ Compilazione automatizzata con Makefile

---

## 🔍 Dettagli Tecnici

- **Processi e Forking**: Il `master` crea più processi usando `fork()`.
- **Memoria Condivisa**: I processi leggono e scrivono dati condivisi.
- **Segnali UNIX**: Usati per coordinare le azioni dei processi.
- **Sincronizzazione con Semafori**: Per prevenire race condition.

---

## 📜 Licenza

Questo progetto è distribuito sotto la licenza **MIT**.

---

📩 **Autore:** Bongiovanni Francesco\
🌍 **GitHub:** (https://github.com/Francesco-Bongio)

