/* Scrivere un programma che acquisisce una seq di num reali corrisp a diversi valori di 
 * pressione in una caldaia. Questi val sono misurati a intervalli regolari in sequenza.
 * La seq è terminata dall'inserimento di un valore negativo. Il programma calcola la
 * media dei diversi valori di pressione e la media dei picchi di pressione. Un valore p1
 * è un picco (max locale) se è seguito da un valore p2 che è minore di p1 e se è precedu-
 * to da un valore p0 minore di p1. Primo e ultimo val della seq non sono considerati pic-
 * chi. Se non vengono inseriti valori il prog stampa trattino sia per la press media che
 * per la media dei picchi. Se no picchi stampa trattino come media picchi. */

#include <stdio.h>

int main(int argc, char *argv[]) {
  float p1 __attribute((annotate("no_float")));
  float p2 __attribute((annotate("no_float")));
  float p3;  /* FIXME: don't convert this because it's a param to the scanf */
  float sPeak __attribute((annotate("no_float")));
  float sAll __attribute((annotate("no_float")));
  int cPeak, cAll;
  
  p3 = p2 = p1 = -1;
  sPeak = sAll = 0;
  cPeak = cAll = 0;
  
  scanf("%f",&p3);
  while (p3 > 0) {
    if (p3 > 0 && p2 > 0 && p1 > 0)
      if (p3 < p2 && p1 < p2) {
        sPeak += p2;
        cPeak++;
      }
    sAll += p3;
    cAll++;
    p1 = p2;
    p2 = p3;
    scanf("%f",&p3);
  }
  
  printf("Media picchi: ");
  if (cPeak > 0)
    printf("%f\n", sPeak / cPeak);
  else
    printf("-\n");
  printf("Media totale: ");
  if (cAll > 0)
    printf("%f\n", sAll / cAll);
  else
    printf("-\n");
    
  return 0;
}
