#include "../Comportamientos_Jugador/auxiliar.hpp"
#include <iostream>
#include "motorlib/util.h"
#include <limits.h>

// ************************** MÉTODOS PÚBLICOS **************************

list<Action> ComportamientoAuxiliar::avanzaSaltosDeCaballo()
{
   list<Action> plan;
   plan.push_back(WALK);
   plan.push_back(WALK);
   plan.push_back(TURN_SR);
   plan.push_back(TURN_SR);
   plan.push_back(WALK);
   return plan;
}

Action ComportamientoAuxiliar::think(Sensores sensores)
{
   Action accion = IDLE;

   switch (sensores.nivel)
   {
   case 0:
      accion = ComportamientoAuxiliarNivel_0(sensores);
      break;
   case 1:
      // accion = ComportamientoAuxiliarNivel_1 (sensores);
      break;
   case 2:
      // accion = ComportamientoAuxiliarNivel_2 (sensores);
      break;
   case 3:
      accion = ComportamientoAuxiliarNivel_E(sensores);
      break;
   case 4:
      // accion = ComportamientoAuxiliarNivel_4 (sensores);
      break;
   }

   return accion;
}

int ComportamientoAuxiliar::interact(Action accion, int valor)
{
   return 0;
}

list<Action> ComportamientoAuxiliar::busquedaAnchura(const EstadoA &inicio, const EstadoA &fin, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura)
{
   NodoA nodoActual;
   list<NodoA> frontera;
   list<NodoA> explorados;

   nodoActual.estado = inicio;
   frontera.push_back(nodoActual);
   bool solucion = nodoActual.estado.f == fin.f and nodoActual.estado.c == fin.c;
   while (!frontera.empty() and !solucion)
   {
      frontera.pop_front();
      explorados.push_back(nodoActual);

      if (terreno[nodoActual.estado.f][nodoActual.estado.c] == 'D')
      {
         nodoActual.estado.zapatillas = true;
      }
      NodoA hijoWalk = nodoActual;
      hijoWalk.estado = aplicar(WALK, nodoActual.estado, terreno, altura);
      hijoWalk.acciones.push_back(WALK);
      solucion = hijoWalk.estado.f == fin.f and hijoWalk.estado.c == fin.c;
      if (solucion)
      {
         nodoActual = hijoWalk;
      }

      if (!encontrar(hijoWalk, frontera) and !encontrar(hijoWalk, explorados))
      {
         frontera.push_back(hijoWalk);
      }

      if (!solucion)
      {
         NodoA hijoTurnSR = nodoActual;
         hijoTurnSR.estado = aplicar(TURN_SR, nodoActual.estado, terreno, altura);
         hijoTurnSR.acciones.push_back(TURN_SR);
         if (!encontrar(hijoTurnSR, frontera) and !encontrar(hijoTurnSR, explorados))
         {
            frontera.push_back(hijoWalk);
         }
      }
      if (!solucion and !frontera.empty())
      {
         nodoActual = frontera.front();
         solucion = nodoActual.estado.f == fin.f and nodoActual.estado.c == fin.c;
      }
   }

   if (solucion)
   {
      return nodoActual.acciones;
   }
   return list<Action>({});
}

// ************************** MÉTODOS PRIVADOS **************************

Action ComportamientoAuxiliar::ComportamientoAuxiliarNivel_0(Sensores sensores)
{
   actualizarEstado(sensores);
   return tomarDecisionNivel_0(sensores);
}

Action ComportamientoAuxiliar::ComportamientoAuxiliarNivel_1(Sensores sensores)
{
   return IDLE;
}

Action ComportamientoAuxiliar::ComportamientoAuxiliarNivel_2(Sensores sensores)
{
   return IDLE;
}

Action ComportamientoAuxiliar::ComportamientoAuxiliarNivel_3(Sensores sensores)
{
   return IDLE;
}

Action ComportamientoAuxiliar::ComportamientoAuxiliarNivel_4(Sensores sensores)
{
   return IDLE;
}

Action ComportamientoAuxiliar::ComportamientoAuxiliarNivel_E(Sensores sensores)
{
   Action action = IDLE;

   if (!hayPlan)
   {
      //plan = busquedaAnchura();
      hayPlan = true;
   }
   else if (hayPlan && !plan.empty())
   {
      action = plan.front();
      plan.pop_front();
   }
   else
   {
      hayPlan = false;
   }
   return action;
}

void ComportamientoAuxiliar::actualizarEstado(const Sensores &sensores)
{
   if (sensores.superficie[0] == 'D')
   {
      tieneZapatillas = true;
   }

   rellenarMapa(sensores, mapaResultado, true);
   rellenarMapa(sensores, mapaCotas, false);
}

Action ComportamientoAuxiliar::tomarDecisionNivel_0(const Sensores &s)
{
    // 0) Si ya estoy en base, paro
    if (s.superficie[0] == 'X') {
        ultimaAccion = IDLE;
        return IDLE;
    }

    // 1) Ceder paso si veo al Rescatador (r) en 1-2-3
    if (s.agentes[1]=='r' || s.agentes[2]=='r' || s.agentes[3]=='r') {
        ultimaAccion = IDLE;
        return IDLE;
    }

    // 2) Recopilo todas las casillas "viables" en mi cono (1…15)
    struct Cand { int idx, prio; int df, dc; };
    std::vector<Cand> cand;
    for (int j = 1; j < 16; ++j) {
        // a) terrenos relevantes y viabilidad
        char t = s.superficie[j];
        int dh = abs(s.cota[0] - s.cota[j]);
        bool alturaOK = dh < 2;               // Auxiliar nunca zapatillas
        bool libre    = s.agentes[j] == '_';  // evita choque
        if (!alturaOK || !libre) continue;

        // b) prioridad: X(4) > C(3) > S(2) > ?(1)
        int p = (t=='X'?4:(t=='C'?3:(t=='S'?2:(t=='?'?1:0))));
        if (p == 0) continue;

        // c) posición relativa
        DFC d = calcularDiferencias(j, s.rumbo);
        cand.push_back({j, p, d.df, d.dc});
    }

    // 3) Si hay candidatos, elijo el "mejor"
    if (!cand.empty()) {
        // a) máximo priority; b) mínimo manhattan; c) mínimo angular (|j-2|)
        int best = 0;
        for (int k = 1; k < (int)cand.size(); ++k) {
            auto &A = cand[k], &B = cand[best];
            if (A.prio > B.prio) best = k;
            else if (A.prio == B.prio) {
                int mA = abs(A.df) + abs(A.dc);
                int mB = abs(B.df) + abs(B.dc);
                if (mA < mB) best = k;
                else if (mA == mB) {
                    if (abs(A.idx - 2) < abs(B.idx - 2)) best = k;
                }
            }
        }

        int objetivo = cand[best].idx;
        // 4) Si está justo enfrente, avanzo
        if (objetivo == 2) {
            ultimaAccion = WALK;
            return WALK;
        }
        // 5) Si está a la izquierda o derecha, giro 45° a la derecha
        ultimaAccion = TURN_SR;
        return TURN_SR;
    }

    // 6) Si no hay ningún candidato viable, me paro
    ultimaAccion = IDLE;
    return IDLE;
}

int ComportamientoAuxiliar::indiceCasillaMasInteresante(const Sensores &sensores)
{
   char frente = viablePorAltura(2, sensores);
   char izqda = viablePorAltura(1, sensores);
   char dcha = viablePorAltura(3, sensores);

   int indice = 0;
   if (frente == 'X')
   {
      indice = 2;
   }
   else if (izqda == 'X')
   {
      indice = 1;
   }
   else if (dcha == 'X')
   {
      indice = 3;
   }
   else if (frente == 'C')
   {
      indice = 2;
   }
   else if (izqda == 'C')
   {
      indice = 1;
   }
   else if (dcha == 'C')
   {
      indice = 3;
   }
   else
   {
      indice = 0;
   }

   return indice;
}

char ComportamientoAuxiliar::viablePorAltura(int indice, const Sensores &sensores)
{
   char terrenoIndice = sensores.superficie[indice];
   int alturaActual = sensores.cota[0];
   int alturaIndice = sensores.cota[indice];
   int dif = abs(alturaIndice - alturaActual);

   if (!(dif < 2))
   {
      terrenoIndice = 'P';
   }

   return terrenoIndice;
}

bool ComportamientoAuxiliar::casillaAccesibleAuxiliar(const EstadoA &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura)
{
   EstadoA siguiente = siguienteCasillaAuxiliar(st);
   bool check1 = terreno[siguiente.f][siguiente.c] != 'P' and terreno[siguiente.f][siguiente.c] != 'M';
   bool check2 = terreno[siguiente.f][siguiente.c] != 'B' or (terreno[siguiente.f][siguiente.c] == 'B' and st.zapatillas);
   bool check3 = abs(altura[siguiente.f][siguiente.c] - altura[st.f][st.c]) <= 1;
   return check1 and check2 and check3;
}

EstadoA ComportamientoAuxiliar::siguienteCasillaAuxiliar(const EstadoA &st)
{
   EstadoA siguiente = st;
   switch (st.brujula)
   {
   case norte:
      siguiente.f = st.f - 1;
      break;
   case noreste:
      siguiente.f = st.f - 1;
      siguiente.c = st.c + 1;
      break;
   case este:
      siguiente.c = st.c + 1;
      break;
   case sureste:
      siguiente.f = st.f + 1;
      siguiente.c = st.c + 1;
      break;
   case sur:
      siguiente.f = st.f + 1;
      break;
   case suroeste:
      siguiente.f = st.f + 1;
      siguiente.c = st.c - 1;
      break;
   case oeste:
      siguiente.c = st.c - 1;
      break;
   case noroeste:
      siguiente.f = st.f - 1;
      siguiente.c = st.c - 1;
      break;
   }

   return siguiente;
}

EstadoA ComportamientoAuxiliar::aplicar(Action accion, const EstadoA &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura)
{
   EstadoA siguiente = st;
   cout << "estado actual: " << "fila: " << st.f << " col: " << st.c << " orientacion: " << st.brujula << endl;
   switch (accion)
   {
   case WALK:
      if (casillaAccesibleAuxiliar(st, terreno, altura))
      {
         siguiente = siguienteCasillaAuxiliar(st);
      }
      break;
   case TURN_SR:
      siguiente.brujula = cambiarOrientacion(siguiente.brujula);
      break;
   }
   cout << "estado final: " << "fila: " << siguiente.f << " col: " << siguiente.c << " orientacion: " << siguiente.brujula << endl;

   return siguiente;
}

Orientacion ComportamientoAuxiliar::cambiarOrientacion(const Orientacion inicial)
{
   Orientacion final = norte;
   switch (inicial)
   {
   case norte:
      final = noreste;
      break;
   case noreste:
      final = este;
      break;
   case este:
      final = sureste;
      break;
   case sureste:
      final = sur;
      break;
   case sur:
      final = suroeste;
      break;
   case suroeste:
      final = oeste;
      break;
   case oeste:
      final = noroeste;
      break;
   case noroeste:
      final = norte;
      break;
   }

   return final;
}

bool ComportamientoAuxiliar::encontrar(const NodoA &st, const list<NodoA> &lista)
{
   auto it = lista.begin();
   while (it != lista.end() and !((*it) == st))
   {
      it++;
   }

   return (it != lista.end());
}

void ComportamientoAuxiliar::visualizarPlan(const EstadoA &st, const list<Action> &plan)
{
   anularMatriz(mapaConPlan);
   EstadoA siguiente = st;

   auto it = plan.begin();
   while (it != plan.end())
   {
      switch (*it)
      {
      case WALK:
         siguiente = siguienteCasillaAuxiliar(siguiente);
         mapaConPlan[siguiente.f][siguiente.c] = 2;
         break;
      case TURN_SR:
         siguiente.brujula = cambiarOrientacion(siguiente.brujula);
         break;
      }

      it++;
   }
}

void ComportamientoAuxiliar::anularMatriz(vector<vector<unsigned char>> &matriz)
{
   for (int i = 0; i < matriz.size(); i++)
   {
      for (int j = 0; j < matriz.size(); j++)
      {
         matriz[i][j] = 0;
      }
   }
}
