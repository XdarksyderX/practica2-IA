#include "rescatador.hpp"
#include "motorlib/util.h"
#include <climits> // INT_MAX
#include <cmath>

Action ComportamientoRescatador::think(Sensores s)
{
  switch (s.nivel)
  {
  case 0:
    return ComportamientoRescatadorNivel_0(s);
  default:
    return IDLE;
  }
}

int ComportamientoRescatador::interact(Action, int) { return 0; }

/* ----------  NIVEL 0  ---------- */
Action ComportamientoRescatador::ComportamientoRescatadorNivel_0(Sensores s)
{
  actualizarEstado(s);
  return tomarDecisionNivel_0(s);
}

/* =========================================
 *  1.  ACTUALIZAR ESTADO
 * ========================================= */
void ComportamientoRescatador::actualizarEstado(const Sensores &s)
{
  if (s.superficie[0] == 'D')
    tieneZapatillas = true;
  rellenarMapa(s, mapaResultado, true);
  rellenarMapa(s, mapaCotas, false);
}

/* =========================================
 *  2.  DECISIÓN  (solo camina por C o X)
 * ========================================= */
inline bool ComportamientoRescatador::caminoViable(const Sensores &s, int idx) const
{
  if (s.agentes[idx] != '_')
    return false;                        // ocupada
  int dh = abs(s.cota[0] - s.cota[idx]); // desnivel
  if (dh >= 2)
    return false;
  char t = s.superficie[idx];
  return t == 'C' || t == 'X';
}

Action ComportamientoRescatador::tomarDecisionNivel_0(const Sensores& s)
{
    /* 0) si ya estoy en la base */
    if (s.superficie[0]=='X') return IDLE;

    /* 1) completar LEFT (6×TURN_SR) */
    if (leftCounter>0){
        --leftCounter;
        ultimaAccion = TURN_SR;
        return TURN_SR;
    }

    /* 2) buscar X más cercana (exactamente igual que antes) */
    int best=-1, bestDist=INT_MAX;
    for(int j=1;j<16;++j) if (s.superficie[j]=='X'){
        DFC dX=calcularDiferencias(j,s.rumbo);
        for(int i:{1,2,3}) if (caminoViable(s,i)){
            DFC dI=calcularDiferencias(i,s.rumbo);
            int m=abs(dX.df-dI.df)+abs(dX.dc-dI.dc);
            if(m<bestDist){bestDist=m;best=i;}
        }
    }
    if(best!=-1){
        if(best==2)           return WALK;
        if(best==3)           return TURN_SR;
        leftCounter=6;        return TURN_SR;   /* best==1 */
    }

    /* 3) SIN X – puntuamos únicamente C-frente, C-izq, C-dcha */
    int wF = caminoViable(s,2) ? 3 : 0;   // frente = peso 3
    int wL = caminoViable(s,1) ? 2 : 0;   // izquierda = 2
    int wR = caminoViable(s,3) ? 1 : 0;   // derecha   = 1
    int total = wF + wL + wR;

    /* 3a) si hay al menos una casilla de camino: elijo aleatoriamente
           proporcional a los pesos 3/2/1            */
    if (total>0)
    {
        double r = rand01(s) * total;     // 0 … total
        if (r < wF) { ultimaAccion=WALK;           return WALK; }
        r -= wF;
        if (r < wL) { leftCounter=6; ultimaAccion=TURN_SR; return TURN_SR; }
        /* r cayó en el peso de la derecha */
        ultimaAccion=TURN_SR;
        return TURN_SR;
    }

    /* 4) sin caminos visibles → sigo la política de giro derecha */
    ultimaAccion = TURN_SR;
    return TURN_SR;
}