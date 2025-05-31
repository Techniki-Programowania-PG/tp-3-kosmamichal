#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include <matplot/matplot.h>
#include <vector>
#include <complex>
#include <cmath>

namespace py = pybind11;

using namespace matplot;
using namespace std;

const double pi_stale = 3.141592653589793;

struct ParametrySygnalu {
    double czestotliwosc;
    double poczatek;
    double koniec;
    size_t liczbaProb;
    char typ;
    ParametrySygnalu(double f, double p, double k, size_t lp, char t)
        : czestotliwosc(f), poczatek(p), koniec(k), liczbaProb(lp), typ(t) {}
};

static double oblicz_probke(double t, const ParametrySygnalu& param) {
    double faza = 2.0 * pi_stale * param.czestotliwosc * t;
    switch (param.typ) {
        case 's':
            return sin(faza);
        case 'c':
            return cos(faza);
        case 'p': {
            if (sin(faza) >= 0.0)
                return 1.0;
            else
                return -1.0;
        }
        case 'o': {
            double okres = 1.0 / param.czestotliwosc;
            double reszta = fmod(t, okres);
            return 2.0 * (reszta / okres) - 1.0;
        }
        default:
            return 0.0;
    }
}

static vector<double> stworz_wektor_czasu(const ParametrySygnalu& param) {
    vector<double> czasy(param.liczbaProb);
    double dt = (param.koniec - param.poczatek) / static_cast<double>(param.liczbaProb - 1);
    for (size_t i = 0; i < param.liczbaProb; ++i) {
        czasy[i] = param.poczatek + i * dt;
    }
    return czasy;
}

vector<double> generuj_sygnal(double czestotliwosc, double poczatek, double koniec,
                              size_t liczbaProb, char typ) {
    ParametrySygnalu param(czestotliwosc, poczatek, koniec, liczbaProb, typ);
    vector<double> sygnal;
    sygnal.reserve(param.liczbaProb);
    auto czasy = stworz_wektor_czasu(param);
    for (double t : czasy) {
        sygnal.push_back(oblicz_probke(t, param));
    }
    return sygnal;
}

void rysuj_wykres(const vector<double>& czasy, const vector<double>& wartosci, const string& tytul) {
    plot(czasy, wartosci);
    title(tytul);
    xlabel("Czas [s]");
    ylabel("Wartosc");
    show();
}

void rysuj_sygnal(double czestotliwosc, double poczatek, double koniec,
                  size_t liczbaProb, char typ, const string& nazwa) {
    vector<double> czasy(liczbaProb);
    double dt = (koniec - poczatek) / static_cast<double>(liczbaProb - 1);
    for (size_t i = 0; i < liczbaProb; ++i) {
        czasy[i] = poczatek + i * dt;
    }
    auto wartosci = generuj_sygnal(czestotliwosc, poczatek, koniec, liczbaProb, typ);
    rysuj_wykres(czasy, wartosci, nazwa);
}

PYBIND11_MODULE(_core, m) {
    m.def("dodaj", [](int a, int b) {return a + b;});
    m.def("odejmij", [](int a, int b) {return a - b;});
    m.def("generuj_sin", [](double f, double p, double k, size_t n) {
        return generuj_sygnal(f, p, k, n, 's');
    });
    m.def("generuj_cos", [](double f, double p, double k, size_t n) {
        return generuj_sygnal(f, p, k, n, 'c');
    });
    m.def("generuj_prostokat", [](double f, double p, double k, size_t n) {
        return generuj_sygnal(f, p, k, n, 'p');
    });
    m.def("generuj_piloksztaltny", [](double f, double p, double k, size_t n) {
        return generuj_sygnal(f, p, k, n, 'o');
    });
    m.def("rysuj_sin", [](double f, double p, double k, size_t n) {
        rysuj_sygnal(f, p, k, n, 's', "Wykres sinusa");
    });
    m.def("rysuj_cos", [](double f, double p, double k, size_t n) {
        rysuj_sygnal(f, p, k, n, 'c', "Wykres cosinusa");
    });
    m.def("rysuj_prostokat", [](double f, double p, double k, size_t n) {
        rysuj_sygnal(f, p, k, n, 'p', "Wykres prostokatny");
    });
    m.def("rysuj_piloksztaltny", [](double f, double p, double k, size_t n) {
        rysuj_sygnal(f, p, k, n, 'o', "Wykres piloksztaltny");
    });
    m.attr("wersja") = "dev";
}