# esercizio-C-2020-05-21-forker

un processo apre un file in scrittura (se esiste già sovrascrive i contenuti del file),
poi lancia M threads.

"fase 1" vuol dire: dormire per un intervallo random di tempo compreso tra 0 e 3 secondi, poi scrivere nel file il messaggio: 
"fase 1, thread id=<tid>, sleep period=<n> secondi"

"fase 2" vuol dire: scrivere nel file il messaggio "fase 2, thread id=<tid>, dopo la barriera" poi dormire per 10 millisecondi, scrivere nel file il messggio "thread id=<tid> bye!".
 

per ogni thread: effettuare "fase 1", poi aspettare che tutti i thread abbiano completato la fase 1 (barriera); poi effettuare "fase 2" e terminare il thread.

