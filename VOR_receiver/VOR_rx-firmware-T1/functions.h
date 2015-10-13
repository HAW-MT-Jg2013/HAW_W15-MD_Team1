#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// wartet bis IR-Signal detektiert wird, maximal aber maxWaitTimeMS Millisekunden
bool waitForIR(short maxWaitTimeMS);

// wartet bis waitTimeMS Millisekunden nach startTimeMS vergangen sind
void waitMSfrom(long startTimeMS, short waitTimeMS);

// Berechnung der Koordinaten aus den errechneten Winkeln, Rueckgabe per Call-by-Reference
void calculate(unsigned char* xKoord, unsigned char* yKoord, double alpha, double beta, double kappa);

#endif /* FUNCTIONS_H */
