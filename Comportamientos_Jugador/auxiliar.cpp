#include "../Comportamientos_Jugador/auxiliar.hpp"
#include <iostream>
#include "motorlib/util.h"

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
      plan = busquedaAnchura();
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

Action ComportamientoAuxiliar::tomarDecisionNivel_0(const Sensores &sensores)
{
   if (sensores.superficie[0] == 'X')
   {
      return IDLE;
   }

   if (giro45Izqda > 0)
   {
      giro45Izqda--;
      return TURN_L;
   }

   char frente = viablePorAltura(2, sensores);
   char izqda = viablePorAltura(1, sensores);
   char dcha = viablePorAltura(3, sensores);

   if (frente == 'X')
   {
      return WALK;
   }
   else if (izqda == 'X')
   {
      giro45Izqda = 1;
      return TURN_L;
   }
   else if (dcha == 'X')
   {
      return TURN_SR;
   }
   else if (frente == 'C')
   {
      return WALK;
   }
   else if (izqda == 'C')
   {
      giro45Izqda = 1;
      return TURN_L;
   }
   else if (dcha == 'C')
   {
      return TURN_SR;
   }

   return TURN_SR;
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
