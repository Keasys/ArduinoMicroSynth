ArduinoMicroSynth
Intendo realizzare un synth analogico pilotato via MIDI attraverso un Arduino Due.
Il sintetizzatore è costituito da due organi fondamentali:

  -un oscillatore
  costruito con un integratore e un comparatore i quali forniscono rispettivamente onda quadra e triangolare.Inoltre è presente un   
  input, ricevente una tensione in ingresso, la quale sceglierà la frequenza dell'oscillatore.
  
  -un amplificatore
  costruito con un operazionale che regola l'ampiezza dell'onda generata dall'oscillatore.
  l'amplificatore riceve anch'esso una tensione d'ingresso per regolare il volume in uscita
  
Il software si occupa di gestire 3 funzionalità principali:
  -gestione dei messaggi MIDI in ingresso
  -erogazione della tensione per l'oscillatore ,attraverso la DAC intergrata di Arduino due, secondo il messaggio MIDI ricevuto
  -erogazione della tensione per l'amplificatore, attraverso un output digitale PWM, filtrato opportunatamente, per ottenere una               
  tensione continua




  
