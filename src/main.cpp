#include <iostream>
#include <pqxx/pqxx>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <sstream>
#include <algorithm>

using namespace std;

// Directorio de sonidos
const string SOUND_DIR = "./src/sounds/";
// Archivo CSV de traducciones
const string CSV_FILE  = "translations.csv";

// Reglas de encriptado (Umg)
string encriptarUmg(const string& palabra) {
    const string vowelsLower = "aeiou";
    const string vowelsUpper = "AEIOU";
    const string consLower   = "bcdfghjklmnpqrstvwxyz";
    const string consUpper   = "BCDFGHJKLMNPQRSTUVWXYZ";

    string resultado;
    for (char c : palabra) {
        // Vocales minúsculas
        auto p1 = vowelsLower.find(c);
        if (p1 != string::npos) {
            resultado += 'U';
            resultado += char('1' + p1);
            continue;
        }

        // Vocales mayúsculas
        auto p2 = vowelsUpper.find(c);
        if (p2 != string::npos) {
            resultado += 'U';
            resultado += char('1' + p2);
            continue;
        }

        // Consonantes minúsculas
        auto p3 = consLower.find(c);
        if (p3 != string::npos) {
            resultado += 'm';
            resultado += to_string(p3+1);
            continue;
        }

        // Consonantes mayúsculas
        auto p4 = consUpper.find(c);
        if (p4 != string::npos) {
            resultado += 'g';
            resultado += to_string(p4+1);
            continue;
        }

        // Carácter no alfabético
        resultado += c;
    }
    return resultado;
}

// Estructura para el árbol AVL (historial de búsqueda)
struct NodoAVL {
    string palabra;
    NodoAVL* iz;
    NodoAVL* de;
    int altura;
    NodoAVL(const string& p) : palabra(p), iz(nullptr), de(nullptr), altura(1) {}
};

// Funciones auxiliares para AVL
int altura(NodoAVL* n) { return n ? n->altura : 0; }
int balance(NodoAVL* n) { return n ? altura(n->iz) - altura(n->de) : 0; }

// Rotación a la derecha
NodoAVL* rotR(NodoAVL* y) {
    NodoAVL* x = y->iz;
    NodoAVL* T2 = x->de;
    x->de = y;
    y->iz = T2;
    y->altura = 1 + max(altura(y->iz), altura(y->de));
    x->altura = 1 + max(altura(x->iz), altura(x->de));
    return x;
}

// Rotación a la izquierda
NodoAVL* rotL(NodoAVL* x) {
    NodoAVL* y = x->de;
    NodoAVL* T2 = y->iz;
    y->iz = x;
    x->de = T2;
    x->altura = 1 + max(altura(x->iz), altura(x->de));
    y->altura = 1 + max(altura(y->iz), altura(y->de));
    return y;
}

// Inserción en el árbol AVL
NodoAVL* insertarAVL(NodoAVL* nodo, const string& w) {
    if (!nodo) return new NodoAVL(w);
    if (w < nodo->palabra) nodo->iz = insertarAVL(nodo->iz, w);
    else if (w > nodo->palabra) nodo->de = insertarAVL(nodo->de, w);
    else return nodo; // No insertar duplicados

    nodo->altura = 1 + max(altura(nodo->iz), altura(nodo->de));

    int b = balance(nodo);
    if (b > 1 && w < nodo->iz->palabra) return rotR(nodo);
    if (b < -1 && w > nodo->de->palabra) return rotL(nodo);
    if (b > 1 && w > nodo->iz->palabra) { nodo->iz = rotL(nodo->iz); return rotR(nodo); }
    if (b < -1 && w < nodo->de->palabra) { nodo->de = rotR(nodo->de); return rotL(nodo); }

    return nodo;
}

// Recorrido en orden del árbol AVL para generar archivos de historial
void inOrden(NodoAVL* nodo, ofstream& orig, ofstream& enc) {
    if (!nodo) return;
    inOrden(nodo->iz, orig, enc);
    orig << nodo->palabra << "\n";
    enc << encriptarUmg(nodo->palabra) << "\n";
    inOrden(nodo->de, orig, enc);
}

// Carga las traducciones desde el archivo CSV
unordered_map<string, string> translations;
void loadTranslations() {
    ifstream f(CSV_FILE);
    if (!f.is_open()) {
        cerr << "No pude abrir " << CSV_FILE << "\n";
        return;
    }
    string line;
    getline(f, line);            
    while (getline(f, line)) {
        stringstream ss(line);
        string user;             
        string srcLang, srcWord;
        string tgtLang, tgtWord;

        // 1) leer y descartar "User"
        getline(ss, user, ',');
        // 2) leer las columnas que sí usamos
        getline(ss, srcLang, ',');
        getline(ss, srcWord, ',');
        getline(ss, tgtLang, ',');
        getline(ss, tgtWord, ',');


        // clave: "ES|EN|Hola"
        string key = srcLang + "|" + tgtLang + "|" + srcWord;
        translations[key] = tgtWord;
    }
    cout << "Cargadas " << translations.size() << " traducciones.\n";
}

// Exporta las traducciones de la base de datos a un archivo CSV
void export_to_csv(pqxx::connection& C) {
    const string fn = CSV_FILE;
    bool exists = filesystem::exists(fn);
    ofstream o(fn, ios::app);
    if (!o) { cerr << "No puedo abrir el archivo " << fn << "\n"; return; }
    if (!exists) o << "User,Source Language,Source Word,Target Language,Target Word\n";

    pqxx::work W(C);
    auto R = W.exec(
        "SELECT u.username, l1.code, w1.text, l2.code, w2.text "
        "FROM user_words uw "
        "JOIN users u ON uw.user_id = u.id "
        "JOIN words w1 ON uw.word_id = w1.id "
        "JOIN languages l1 ON w1.language_id = l1.id "
        "JOIN translations t ON t.source_word_id = w1.id "
        "JOIN words w2 ON t.target_word_id = w2.id "
        "JOIN languages l2 ON w2.language_id = l2.id "
        "ORDER BY u.username, l1.code, w1.text;"
    );

    for (const auto& r : R) {
        o << r[0].c_str() << "," << r[1].c_str() << "," << r[2].c_str() << ","
          << r[3].c_str() << "," << r[4].c_str() << "\n";
    }

    W.commit();
    cout << "Exportado " << fn << "\n";
}

// Reproduce el audio correspondiente al archivo de sonido
void reproducirAudio(const string& archivo) {
    string filePath = SOUND_DIR + archivo;
    if (!filesystem::exists(filePath)) {
        cout << "Archivo de audio no encontrado: " << filePath << "\n";
        return;
    }
    string cmd = "powershell.exe -Command Start-Process '" + filePath + "'";
    system(cmd.c_str());
}

int main() {
    // Configuración de conexión a la base de datos
    const char* dbName = getenv("DB_NAME");
    const char* dbUser = getenv("DB_USER");
    const char* dbPassword = getenv("DB_PASSWORD");
    const char* dbHost = getenv("DB_HOST");
    const char* dbPort = getenv("DB_PORT");

    if (!dbName || !dbUser || !dbPassword || !dbHost || !dbPort) {
        cerr << "Faltan variables de entorno para la base de datos.\n";
        return 1;
    }

    string connStr = "dbname=" + string(dbName) + " user=" + dbUser + " password=" + dbPassword
                     + " host=" + dbHost + " port=" + dbPort;
    pqxx::connection C(connStr);
    if (!C.is_open()) {
        cerr << "No se pudo conectar a la base de datos.\n";
        return 1;
    }

    cout << "Conectado a la base de datos: " << C.dbname() << "\n";

    // Exporta las traducciones
    export_to_csv(C);

    // Carga las traducciones al mapa
    loadTranslations();

    NodoAVL* historial = nullptr;

    vector<pair<string, string>> idiomas = {{"1", "EN"}, {"2", "IT"}, {"3", "FR"}, {"4", "DE"}};

    while (true) {
        cout << "\n1)EN 2)IT 3)FR 4)DE 5)Salir\n> ";
        string opcion;
        cin >> opcion;
        if (opcion == "5") break;
    
        auto itLang = find_if(
          idiomas.begin(), idiomas.end(),
          [&](const auto& p){ return p.first == opcion; }
        );
        if (itLang == idiomas.end()) {
            cout << "Opción inválida.\n";
            continue;
        }
    
        string idiomaDestino = itLang->second;
        cout << "Palabra ES→" << idiomaDestino << ":\n> ";
        string palabra;
        cin >> palabra;
    
        // 1) construye la misma clave que usaste en loadDict:
        string key = string("ES|") + idiomaDestino + "|" + palabra;
    
        auto fnd = translations.find(key);
        if (fnd == translations.end()) {
            cout << "Traducción no disponible.\n";
        } else {
            // 2) muestra y reproduce la palabra *traducida*
            cout << "Traducción: " << fnd->second << "\n";
            reproducirAudio(fnd->second + "_" + idiomaDestino + ".mp3");
        }
    
        // 3) inserta en el AVL tu palabra original (o la traducida, según prefieras)
        historial = insertarAVL(historial, palabra);
    }
    
    // Volcar historial a los archivos de texto (original y encriptado)
    ofstream originalFile("original.txt", ios::app);
    ofstream encriptadoFile("encriptado.txt", ios::app);
    inOrden(historial, originalFile, encriptadoFile);

    cout << "Proceso finalizado. ¡Hasta luego!\n";
    return 0;
}
