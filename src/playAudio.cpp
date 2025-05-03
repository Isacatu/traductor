#include <iostream>
#include <fstream>
#include <cstdlib>  // Para system()

using namespace std;

const string audio = "sounds/";  // Ruta a la carpeta de audio

// Función para verificar si el archivo existe
bool existeArchivo(const string &ruta) {
    ifstream archivo(ruta);
    return archivo.good();  // Retorna true si el archivo se puede abrir
}

// Función para reproducir el audio
void reproducirAudio(const string &nombreArchivo) {
    string rutaCompleta = audio + nombreArchivo;

    if (!existeArchivo(rutaCompleta)) {
        cout << "Error: No se encontro el archivo de audio " << nombreArchivo << ".\n";
        return;
    }
    string comando = "start " + rutaCompleta;

    system(comando.c_str());
}

int main() {
    int opcion;
    int opc1;

    do {
        cout << "\nOpciones:\n";
        cout << "1. ES a EN\n";
        cout << "2. ES a IT\n";
        cout << "3. ES a FR\n";
        cout << "4. ES a DE\n";
        cout << "5. Salir\n";
        cout << "Seleccione una opcion: ";
        cin >> opcion;

        string nombreAudio;  // Almacena el nombre del archivo de audio

        switch (opcion) {
            case 1:
                cout << "\nSeleccione la palabra a traducir:\n";
                cout << "1. Hola\n";
                cout << "2. Adios\n";
                cout << "3. Gracias\n";
                cout << "Seleccione una opcion: ";
                cin >> opc1;

                switch (opc1) {
                    case 1: cout << "Hello\n"; nombreAudio = "1i.mp3"; break;
                    case 2: cout << "Goodbye\n"; nombreAudio = "2i.mp3"; break;
                    case 3: cout << "Thanks\n"; nombreAudio = "3i.mp3"; break;
                    default: cout << "Opcion invalida.\n";
                }
                break;

            case 2:
                cout << "\nSeleccione la palabra a traducir:\n";
                cout << "1. Hola\n";
                cout << "2. Adios\n";
                cout << "3. Gracias\n";
                cout << "Seleccione una opcion: ";
                cin >> opc1;

                switch (opc1) {
                    case 1: cout << "Ciao\n"; nombreAudio = "1it.mp3"; break;
                    case 2: cout << "Ciao\n"; nombreAudio = "2it.mp3"; break;
                    case 3: cout << "Grazie\n"; nombreAudio = "3it.mp3"; break;
                    default: cout << "Opcion invalida.\n";
                }
                break;

            case 3:
                cout << "\nSeleccione la palabra a traducir:\n";
                cout << "1. Hola\n";
                cout << "2. Adios\n";
                cout << "3. Gracias\n";
                cout << "Seleccione una opcion: ";
                cin >> opc1;

                switch (opc1) {
                    case 1: cout << "Bonjour\n"; nombreAudio = "1fr.mp3"; break;
                    case 2: cout << "Au revoir\n"; nombreAudio = "2fr.mp3"; break;
                    case 3: cout << "Merci\n"; nombreAudio = "3fr.mp3"; break;
                    default: cout << "Opcion invalida.\n";
                }
                break;

            case 4:
                cout << "\nSeleccione la palabra a traducir:\n";
                cout << "1. Hola\n";
                cout << "2. Adiós\n";
                cout << "3. Gracias\n";
                cout << "Seleccione una opcion: ";
                cin >> opc1;

                switch (opc1) {
                    case 1: cout << "Hallo\n"; nombreAudio = "1a.mp3"; break;
                    case 2: cout << "Tschuss\n"; nombreAudio = "2a.mp3"; break;
                    case 3: cout << "Danke\n"; nombreAudio = "3a.mp3"; break;
                    default: cout << "Opción invalida.\n";
                }
                break;

            case 5:
                cout << "Saliendo del programa...\n";
                break;

            default:
                cout << "Opcion no válida. Intente nuevamente.\n";
        }

        if (!nombreAudio.empty()) {
            reproducirAudio(nombreAudio);
        }

    } while (opcion != 5);

    return 0;
}
