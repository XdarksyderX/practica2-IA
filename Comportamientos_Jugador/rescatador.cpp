#include "rescatador.hpp"
#include "motorlib/util.h"

Action ComportamientoRescatador::think(Sensores sensores)
{
  Action accion = IDLE;

  switch (sensores.nivel)
  {
  case 0:
    accion = ComportamientoRescatadorNivel_0(sensores);
    break;
  case 1:
    // accion = ComportamientoRescatadorNivel_1 (sensores);
    break;
  case 2:
    // accion = ComportamientoRescatadorNivel_2 (sensores);
    break;
  case 3:
    // accion = ComportamientoRescatadorNivel_3 (sensores);
    break;
  case 4:
    // accion = ComportamientoRescatadorNivel_4 (sensores);
    break;
  }

  return accion;
}

int ComportamientoRescatador::interact(Action accion, int valor)
{
  return 0;
}

Action ComportamientoRescatador::ComportamientoRescatadorNivel_0(Sensores sensores)
{
  // El comportamiento de seguir un camino hasta encontrar un puesto base.
  // actualizar variables de estado
  actualizarEstado(sensores);

  // toma de decisiones
  return tomarDecisionNivel_0(sensores);
}

Action ComportamientoRescatador::ComportamientoRescatadorNivel_1(Sensores sensores)
{
  return IDLE;
}

Action ComportamientoRescatador::ComportamientoRescatadorNivel_2(Sensores sensores)
{
  return IDLE;
}

Action ComportamientoRescatador::ComportamientoRescatadorNivel_3(Sensores sensores)
{
  return IDLE;
}

Action ComportamientoRescatador::ComportamientoRescatadorNivel_4(Sensores sensores)
{
  return IDLE;
}

/******************************* PRIVADO***********************
/**
 * metodo de actualizacion de las variables de estado
 * @param sensores
 */
void ComportamientoRescatador::actualizarEstado(const Sensores &sensores)
{
  if (sensores.superficie[0] == 'D')
  {
    tieneZapatillas = true;
  }

  // se rellenan los mapas resultado y de cotas
  rellenarMapa(sensores, mapaResultado, true);
  rellenarMapa(sensores, mapaCotas, false);
}

/**
 * metodo para toma de decisiones en el nivel 0
 * @param sensores
 * @return
 */
Action ComportamientoRescatador::tomarDecisionNivel_0(const Sensores &sensores)
{
  Action accion = IDLE;

  // si estoy en el destino deseado, parar
  if (sensores.superficie[0] == 'X')
  {
    accion = IDLE;
  }
  else if (giro45Izqda != 0)
  {
    // tengo que completar el giro haciendo giro
    // suave a la derecha
    accion = TURN_SR;
    giro45Izqda--;
  }
  else
  {
    // se determina la casilla mas interesante
    int indice = indiceCasillaMasInteresante(sensores);

    // en funcion del indice, actuar
    switch (indice)
    {
    case 2:
      accion = WALK;
      break;
    case 1:
      accion = TURN_L;
      giro45Izqda = 1;
      break;
    case 3:
      accion = TURN_SR;
      break;
    case 0:
      accion = TURN_L;
      break;
    }
  }

  // se devuelve la accion
  return accion;
}

/**
 * metodo para determinar el indice de la casilla
 * mas interesante
 * @param sensores
 * @return
 */
int ComportamientoRescatador::indiceCasillaMasInteresante(const Sensores &sensores)
{
  // detectar que tengo en las tres casillas mas cercanas
  // version previa: solo mirar que hay en superficie
  // char frente = sensores.superficie[2];
  // char izqda = sensores.superficie[1];
  // char dcha = sensores.superficie[3];
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
  else if (!tieneZapatillas)
  {
    if (frente == 'D')
    {
      indice = 2;
    }
    else if (izqda == 'D')
    {
      indice = 1;
    }
    else if (dcha == 'D')
    {
      indice = 3;
    }
  }

  // si llego aqui con indice == 0,
  // mirar por el camino
  if (indice == 0)
  {
    if (frente == 'C')
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
  }

  // devolver el valor de indice
  return indice;
}

/**
 * determina si la posicion dada por un indice (1, 2, 3)
 * es viable teniendo en cuenta la diferencia de altura
 * @param indice
 * @param sensores
 * @return
 */
char ComportamientoRescatador::viablePorAltura(int indice, const Sensores &sensores)
{
  // buscar el tipo de terreno de interes
  char terrenoIndice = sensores.superficie[indice];

  // determinamos la altura de la posicion del agente
  int alturaActual = sensores.cota[0];

  // determinamos la altura de la posicion dada por indice
  int alturaIndice = sensores.cota[indice];

  // calculamos la diferencia de alturas
  int diferencia = abs(alturaActual - alturaIndice);

  // la posicion es viable: la diferencia es menor de 2 o
  // bien 2 si hay zapatillas. Miramos tambien que tampoco
  // haya otro agente
  if (!(diferencia < 2 or (tieneZapatillas and diferencia < 3) or
        sensores.agentes[indice] != '_'))
  {
    terrenoIndice = 'P';
  }

  // se devuelve terrenoIndice
  return terrenoIndice;
}
