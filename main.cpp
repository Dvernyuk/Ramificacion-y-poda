// Dmytro Vernyuk
// RyP | Practica 5: ejercicio 8 de la hoja de problemas

#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <chrono>
#include <queue>
using namespace std;

const int MAX_CUADROS = 100; // Cuadros por fichero input
int N_COTA; // Indica con que cota optimista trabajamos

// vector sol -> get<0> es True si hemos elegido colgar el cuadro y False en el caso contrario
//               get<1> es True si lo colgamos en horizontal y False en vertical
typedef struct nodo {
    vector<tuple<bool,bool>> sol; // Vector solucion (Ver arriba para ver el significado de la tupla)
    int k = 0; // Nivel del arbol
    int l_rest = 0; // L restante de la pared
    int prestigio = 0; // Prestigio acumulado
    int prestOpt = 0; // Prestigio optimista

    bool operator<(const nodo &o) const {return prestOpt < o.prestOpt;} // Necesario para la cola con prioridad
}t_nodo;

// vector cuadros -> get<0> == prestigio del cuadro, get<1> == L horizontal y get<2> == L vertical
int cotaOpt(vector<tuple<int, int, int>> cuadros, t_nodo nodo) {
    int ret = 0, ladoMenor;
    //int ladoMenorMax = -1, kMenorMax = -1; // Para la cota optimista 3 descartada: descrita en la memoria
    int l_rest = nodo.l_rest, k = nodo.k + 1;

    ret += nodo.prestigio;

    while (l_rest > 0 && k < MAX_CUADROS) {
        if (get<1>(cuadros[k]) < get<2>(cuadros[k]))
            ladoMenor = get<1>(cuadros[k]);
        else
            ladoMenor = get<2>(cuadros[k]);

        if (ladoMenor <= l_rest) {
            ret += get<0>(cuadros[k]);
            l_rest -= ladoMenor;
        }
        else {
            // Cota optimista 1 (menos costosa)
            if (N_COTA == 1) {
                // Si el ladoMenor del cuadro k supera a l_rest -> partimos el cuadro y el prestigio
                ret += get<0>(cuadros[k]) * l_rest / ladoMenor;
                l_rest = 0;
            }


            // Cota optimista 2 (más costosa): No hace falta codigo para ella


            // Cota optimista 3 descartada: descrita en la memoria

            /*else if (N_COTA == 2) {
                if (ladoMenorMax == -1 || ladoMenor < ladoMenorMax) {
                    kMenorMax = k;
                    ladoMenorMax = ladoMenor;
                }

                if (k + 1 == MAX_CUADROS && kMenorMax != -1) {
                    ret += get<0>(cuadros[kMenorMax]) * l_rest / ladoMenorMax;
                    l_rest = 0;
                }
            }*/
        }
        k++;
    }

    return ret;
}

// vector cuadros -> get<0> == prestigio del cuadro, get<1> == L horizontal y get<2> == L vertical
bool factible(vector<tuple<int, int, int>> cuadros, t_nodo nodo) {
    bool ret = false;
    int l_rest = nodo.l_rest, k = nodo.k + 1;

    while (l_rest > 0 && k < MAX_CUADROS) {
        if (get<1>(cuadros[k]) > get<2>(cuadros[k]))
            l_rest -= get<1>(cuadros[k]);
        else
            l_rest -= get<2>(cuadros[k]);

        k++;
    }

    if (l_rest <= 0) ret = true;

    return ret;
}

// vector cuadros -> get<0> == prestigio del cuadro, get<1> == L horizontal y get<2> == L vertical
// vector sol_mejor -> get<0> es True si hemos elegido colgar el cuadro y False en el caso contrario
//                     get<1> es True si lo colgamos en horizontal y False en vertical
bool solve(int longitud, vector<tuple<int, int, int>> cuadros, vector<tuple<bool, bool>>& sol_mejor, int& prest_mejor) {
    int contador = 0; // Contador de nodos explorados
    t_nodo x, y;
    std::priority_queue<t_nodo> cp_nodos; // Cola de prioridad

    y.k = -1; y.prestigio = 0; y.l_rest = longitud;
    y.sol.resize(MAX_CUADROS);
    
    y.prestOpt = cotaOpt(cuadros, y); // Cota optimista
    prest_mejor = 0; // Cota pesimista

    cp_nodos.push(y);

    // Inicializacion para el calculo del tiempo
    using clock = std::chrono::system_clock;
    using sec = std::chrono::duration<double, std::milli>;
    const auto t_inicio = clock::now();

    // Antes de nada comprobamos si cabe la posiblidad de rellenar L (la pared) aunque sea pasandose
    if(factible(cuadros, x))
        while (!cp_nodos.empty() && (y = cp_nodos.top()).prestOpt > prest_mejor) {
            contador++;

            cp_nodos.pop();
            x.k = y.k + 1; x.sol = y.sol;

            // Colgar el cuadro en horizontal
            if (y.l_rest - get<1>(cuadros[x.k]) >= 0) {
                get<0>(x.sol[x.k]) = true; // Hemos elegido el cuadro
                get<1>(x.sol[x.k]) = true; // Esta en horizontal
                x.prestigio = y.prestigio + get<0>(cuadros[x.k]);
                x.l_rest = y.l_rest - get<1>(cuadros[x.k]);

                // Si escogemos el lado mayor o igual al otro no hace falta calcular la factibilidad
                if (get<1>(cuadros[x.k]) >= get<2>(cuadros[x.k]) || factible(cuadros, x)) {

                    // Si escogemos el lado menor o igual al otro no hace falta calcular otra vez la cotaOpt
                    if (get<1>(cuadros[x.k]) <= get<2>(cuadros[x.k]))
                        x.prestOpt = y.prestOpt;
                    else
                        x.prestOpt = cotaOpt(cuadros, x);

                    // Si estamos en la iteracion final
                    if (x.k + 1 == MAX_CUADROS) {
                        if (x.l_rest == 0 && x.prestigio > prest_mejor) {
                            prest_mejor = x.prestigio;
                            sol_mejor = x.sol;
                        }
                    }
                    else {
                        cp_nodos.push(x);
                    }
                }
            }

            // Colgar el cuadro en vertical
            if (y.l_rest - get<2>(cuadros[x.k]) >= 0) {
                get<0>(x.sol[x.k]) = true; // Hemos elegido el cuadro
                get<1>(x.sol[x.k]) = false; // Esta en vertical
                x.prestigio = y.prestigio + get<0>(cuadros[x.k]);
                x.l_rest = y.l_rest - get<2>(cuadros[x.k]);

                // Si escogemos el lado mayor o igual al otro no hace falta calcular la factibilidad
                if (get<2>(cuadros[x.k]) >= get<1>(cuadros[x.k]) || factible(cuadros, x)) {

                    // Si escogemos el lado menor o igual al otro no hace falta calcular otra vez la cotaOpt
                    if (get<2>(cuadros[x.k]) <= get<1>(cuadros[x.k]))
                        x.prestOpt = y.prestOpt;
                    else
                        x.prestOpt = cotaOpt(cuadros, x);

                    // Si estamos en la iteracion final
                    if (x.k + 1 == MAX_CUADROS) {
                        if (x.l_rest == 0 && x.prestigio > prest_mejor) {
                            prest_mejor = x.prestigio;
                            sol_mejor = x.sol;
                        }
                    }
                    else {
                        cp_nodos.push(x);
                    }
                }
            }

            // No colgar el cuadro
            get<0>(x.sol[x.k]) = false; // No hemos elegido el cuadro
            x.prestigio = y.prestigio;
            x.l_rest = y.l_rest;

            if (factible(cuadros, x)) {

                x.prestOpt = cotaOpt(cuadros, x);

                // Si estamos en la iteracion final
                if (x.k + 1 == MAX_CUADROS) {
                    if (x.l_rest == 0 && x.prestigio > prest_mejor) {
                        prest_mejor = x.prestigio;
                        sol_mejor = x.sol;
                    }
                }
                else {
                    cp_nodos.push(x);
                }
            }

        }
        // fin while; bucle principal

    const sec t_final = clock::now() - t_inicio; // Calculo del tiempo transcurrido

    cout << "  Prestigio total " << prest_mejor;
    cout << ", nodos explorados " << contador;
    cout << ", tiempo total " << t_final.count() << " ms, ";
    cout << "tiempo medio " << t_final.count() / contador << " ms" << endl << endl;

    // Comprueba si hemos obtenido una solucion
    return prest_mejor > 0;
}

// vector cuadros -> get<0> == prestigio del cuadro, get<1> == L horizontal y get<2> == L vertical
// Funcion auxiliar para la funcion ordenar()
float valor(tuple<int, int, int> cuadro) {
    float ret = 0;

    if (get<1>(cuadro) < get<2>(cuadro)) {
        ret = float(get<0>(cuadro)) / float(get<1>(cuadro));
    }
    else {
        ret = float(get<0>(cuadro)) / float(get<2>(cuadro));
    }

    return ret;
}

// Ordenamos el vector de cuadros segun el valor de proporcion de prestigio/ladoMenor de la funcion valor() (de mayor a menor)
void ordenar(vector<tuple<int, int, int>>& cuadros) {
    tuple<int, int, int> aux;
    
    for (int i = 0; i < MAX_CUADROS - 1; i++)
        for (int j = i + 1; j < MAX_CUADROS; j++) {
            if (valor(cuadros[i]) < valor(cuadros[j])) {
                aux = cuadros[i];
                cuadros[i] = cuadros[j];
                cuadros[j] = aux;
            }
        }
}

// Lee el input de los ficheros
bool leerInput(string nom, int& longitud, vector<tuple<int, int, int>>& cuadros) {
    bool ret = false;

    ifstream in(nom);
    if (in.is_open()) {
        int num0, num1, num2;

        in >> num0;
        longitud = num0;

        for (int i = 0; i < MAX_CUADROS; i++) {
            in >> num0 >> num1 >> num2;
            get<0>(cuadros[i]) = num0;
            get<1>(cuadros[i]) = num1;
            get<2>(cuadros[i]) = num2;
        }

        in.close();
        ret = true;
    }

    return ret;
}

// vector sol_mejor -> get<0> es True si hemos elegido colgar el cuadro y False en el caso contrario
//                     get<1> es True si lo colgamos en horizontal y False en vertical
// Genera el output en los ficheros
void generarOutput(int numArchivo, int longitud, vector<tuple<int, int, int>> cuadros,
    vector<tuple<bool, bool>> sol_mejor, int prest_mejor) {
    
    ofstream out("RyP_out_" + to_string(numArchivo + 1) + "_cota_" + to_string(N_COTA) + ".txt");

    out << "Mejor prestigio: " << prest_mejor << endl;
    out << "Pared, de longitud "<< longitud <<", compuesta por los siguientes cuadros:" << endl << endl;
    for (int i = 0; i < MAX_CUADROS; i++) {

        if (get<0>(sol_mejor[i])) {
            out << "Cuadro de prestigio " << get<0>(cuadros[i]) << "; longitud ";
            if (get<1>(sol_mejor[i])) {
                out << get<1>(cuadros[i]) << "; proporción " << get<0>(cuadros[i]) / get<1>(cuadros[i]) << "; colgado en horizontal" << endl;
            }
            else {
                out << get<2>(cuadros[i]) << "; proporción " << get<0>(cuadros[i]) / get<2>(cuadros[i]) << "; colgado en vertical" << endl;
            }
        }
    }
    out.close();
}

// Funcion que se encarga de la logistica de la aplicacion
bool resuelveCaso() {
    vector<tuple<int, int, int>> cuadros(MAX_CUADROS);
    int longitud;
    vector<tuple<bool, bool>> sol_mejor(MAX_CUADROS);
    int prest_mejor;

    for (int i = 0; i < 3; i++) {
        string str = "RyP_in_" + to_string(i + 1) + ".txt";

        if (!leerInput(str, longitud, cuadros)) {
            cout << "- No se ha podido encontrar el archivo " << str << " para generar el output" << endl << endl;
        }
        else {
            cout << "- Resultado de " << str << ": " << endl;
            ordenar(cuadros);

            if (!solve(longitud, cuadros, sol_mejor, prest_mejor)) {
                cout << "- No se ha podido encontrar una solucion para el archivo " << str  << endl << endl;
            }
            else {
                generarOutput(i, longitud, cuadros, sol_mejor, prest_mejor);
            }
        }
    }

    return true;
}

// Generamos input aleatorio para leerlo posteriormente
void generarInput() {
    int num;

    srand(time(NULL));
    for (int i = 0; i < 3; i++) {
        ofstream out("RyP_in_" + to_string(i + 1) + ".txt");
        
        // Longitud L aleatoria
        num = 100 + rand() % (1001 - 100);
        out << num << endl;

        for (int j = 0; j < MAX_CUADROS; j++) {
            for (int k = 0; k < 3; k++) {
                // Valor de los cuadros como sus longitudes
                // (se generan con la misma probabilidad)
                num = 5 + rand() % (100 - 5);
                if(k == 2)
                    out << num << endl;
                else
                    out << num << " ";
            }
        }
        out.close();
    }
}

// El menu de la aplicacion
int main() {

    while (true) {
        int opt;
        
        cout << "1 ~> Generar output con cota optimista 1 (menos costosa)" << endl;
        cout << "2 ~> Generar output con cota optimista 2 (mas costosa)" << endl;
        cout << "3 ~> Generar input" << endl;
        cout << "0 ~> Salir" << endl;
        cin >> opt;
        
        if (opt == 1 || opt == 2) {
            cout << endl;
            N_COTA = opt;
            resuelveCaso();
        }
        else if (opt == 3) {
            cout << endl;
            generarInput();
        }
        else if (opt == 0) {
            break;
        }
    }
    
    return 0;
}