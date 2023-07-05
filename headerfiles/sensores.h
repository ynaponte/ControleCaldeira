#ifndef SENSORES_H
#define SENSORES_H

void sensor_put(double temp, double nivel, double perturb, double ti);
double sensor_get(char s[5]);
void sensor_alarme(double limite, double nivel_max, double nivel_min);

#endif



