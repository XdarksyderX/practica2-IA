#ifndef COMPORTAMIENTOAUXILIAR_H
#define COMPORTAMIENTOAUXILIAR_H

#include <chrono>
#include <time.h>
#include <thread>
#include <list>

#include "comportamientos/comportamiento.hpp"
#include "rescatador.hpp"

struct EstadoA {
  int f;
  int c;
  Orientacion brujula;
  bool zapatillas;

  bool operator==(const EstadoA & otro) const {
    return (f == otro.f && c == otro.c && brujula == otro.brujula && zapatillas == otro.zapatillas);
  }
};

struct NodoA {
  EstadoA estado;
  list<Action> acciones;

  bool operator==(const NodoA & otro) const {
    return (estado == otro.estado);
  }
};

class ComportamientoAuxiliar : public Comportamiento {

public:
  ComportamientoAuxiliar(unsigned int size = 0) : Comportamiento(size) {
    // Inicializar Variables de Estado Niveles 0,1,4
    ultimaAccion = IDLE;
    tieneZapatillas = false;
    hayPlan = false;
    giro45Izqda = 0;
    giros = 0;
  }

  ComportamientoAuxiliar(std::vector<std::vector<unsigned char>> mapaR, std::vector<std::vector<unsigned char>> mapaC) : Comportamiento(mapaR,mapaC) {
    hayPlan = false;
    tieneZapatillas = false;
  }

  ComportamientoAuxiliar(const ComportamientoAuxiliar &comport) : Comportamiento(comport) {}

  ~ComportamientoAuxiliar() {}

  Action think(Sensores sensores);

  int interact(Action accion, int valor);

  Action ComportamientoAuxiliarNivel_0(Sensores sensores);
  Action ComportamientoAuxiliarNivel_1(Sensores sensores);
  Action ComportamientoAuxiliarNivel_2(Sensores sensores);
  Action ComportamientoAuxiliarNivel_3(Sensores sensores);
  Action ComportamientoAuxiliarNivel_4(Sensores sensores);
  Action ComportamientoAuxiliarNivel_E(Sensores sensores);

private:
  Action ultimaAccion   = IDLE;   // última acción ejecutada
  int    girosPendientes = 0;     // giros RIGHT restantes para completar un "LEFT"
  int    girosSeguidos   = 0;     // cuenta de TURN_SR consecutivos

  //Lista de acciones a realizar
  list<Action> plan;

  // Indica si hay plan
  bool hayPlan;

   // disponibilidad de zapatillas
   bool tieneZapatillas;

   // almacena la realizacion de giro a la izquierda
   int giro45Izqda;
   int    srLeftCounter   = 0;
   int giros;

   /**
    * metodo para actualizar las variables de estado
    */
   void actualizarEstado(const Sensores & sensores);

   /**
    * metodo de toma de decisiones para el nivel0 (version 0)
    * @param sensores
    */
   Action tomarDecisionNivel_0(const Sensores & sensores);

   /**
  * metodo para determinar que casilla es la mas interesante: la
  * izquierda, central o derecha
  * @param izqda
  * @param frente
  * @param dcha
  * @return
  */
   int indiceCasillaMasInteresante(const Sensores & sensores);

   /**
    * determina si una casilla de las accesibles (indice) es viable
    * por altura: si lo es devuelve su valor y en caso controario
    * devolvera P
    * @param indice
    * @param sensores
    * @return
    */
   char viablePorAltura(int indice, const Sensores & sensores);


   list<Action> avanzaSaltosDeCaballo();
   list<Action> busquedaAnchura(const EstadoA &inicio, const EstadoA &fin, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura);

    EstadoA siguienteCasillaAuxiliar(const EstadoA &st);
    bool casillaAccesibleAuxiliar(const EstadoA &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura);
    bool encontrar(const NodoA &st, const list<NodoA> &lista);
    Orientacion cambiarOrientacion(const Orientacion inicial);
    EstadoA aplicar(Action accion, const EstadoA &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura);
    void visualizarPlan(const EstadoA &st, const list<Action> &plan);
    void anularMatriz(vector<vector<unsigned char>> &matriz);

};

#endif