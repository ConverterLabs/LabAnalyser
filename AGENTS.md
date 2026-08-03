# Qt Build Guide for LabAnalyser Projects

This guide describes the Windows toolchain used for LabAnalyser and its Qt
plugin DLLs. It is intended to be copied into other projects on this machine.

## Toolchain

Use the MSYS2 MINGW64 Qt toolchain. Do not mix it with a separate Qt or MinGW
installation.

| Component | Expected path |
| --- | --- |
| Qt/qmake | `C:\msys64\mingw64\bin\qmake6.exe` |
| Make | `C:\msys64\mingw64\bin\mingw32-make.exe` |
| Compiler | `C:\msys64\mingw64\bin\g++.exe` |
| Qt version | Qt 6.9.2 |
| MSYS2 MINGW64 root | `C:\msys64\mingw64` |

Before invoking qmake, put the MINGW64 binaries first in `PATH`:

```powershell
$env:PATH = "C:\msys64\mingw64\bin;C:\msys64\usr\bin;$env:PATH"
$env:MSYSTEM = 'MINGW64'
$env:CHERE_INVOKING = '1'
```

This is important for `moc`, `uic`, and `rcc`: mixing DLLs from another Qt
installation can make those tools fail at startup.

## Build LabAnalyser

The repository provides `build-msys2.ps1`. Run it from the repository root:

```powershell
.\build-msys2.ps1 -Configuration release
.\build-msys2.ps1 -Configuration debug
.\build-msys2.ps1 -Configuration release -Clean -Deploy
```

The script creates an out-of-source build directory at
`build\msys2-mingw64-<configuration>`. It validates the external dependencies.
With `-Deploy`, it additionally creates a clean standalone directory at
`dist\LabAnalyser-<configuration>`. That directory contains the executable,
Qt runtime, compiler runtime, and the required HDF5, FFTW, matio, and native
transitive dependency DLLs. Generated object files and qmake build files stay
in the separate `build` directory and are not copied into the standalone
directory.

For a clean release package, use:

```powershell
.\build-msys2.ps1 -Configuration release -Clean -Deploy
```

The resulting standalone application is in
`dist\LabAnalyser-release`. The deployment directory is removed and recreated
on every `-Deploy` run.

LabAnalyser additionally requires:

- the MSYS2 MINGW64 matio package (`matio.h` and `libmatio.dll.a`)
- MSYS2 MINGW64 import libraries `libhdf5.dll.a` and `libfftw3.dll.a`
- the HighFive headers below `C:\msys64\mingw64\include\highfive`

For a manual release build, use the following from the repository root:

```powershell
$env:PATH = "C:\msys64\mingw64\bin;C:\msys64\usr\bin;$env:PATH"
$env:MSYSTEM = 'MINGW64'

New-Item -ItemType Directory -Force build\msys2-mingw64-release | Out-Null
Push-Location build\msys2-mingw64-release
& C:\msys64\mingw64\bin\qmake6.exe ..\..\LabAnalyser.pro -spec win32-g++ `
    'CONFIG+=release' 'CONFIG-=debug' `
    'QMAKE_LIBDIR+=C:/msys64/mingw64/lib'
& C:\msys64\mingw64\bin\mingw32-make.exe -j$env:NUMBER_OF_PROCESSORS
Pop-Location
```

Re-run qmake after changing a `.pro` file, Qt modules, include paths, library
paths, or build configuration. Use a clean build after changing the compiler,
Qt version, or target architecture.

## Build a LabAnalyser Plugin DLL

Build plugins with the same architecture, MSYS2 MinGW compiler, Qt major/minor
version, and build configuration as the LabAnalyser executable that loads them.
Use `release` plugins with a release LabAnalyser build.

A minimal qmake plugin project looks like this:

```qmake
QT += core gui widgets
CONFIG += plugin c++17
TEMPLATE = lib
TARGET = MyLabAnalyserPlugin

LABANALYSER_ROOT = C:/Projekte/LabAnalyser
INCLUDEPATH += $$LABANALYSER_ROOT

SOURCES += MyLabAnalyserPlugin.cpp
HEADERS += MyLabAnalyserPlugin.h \
           $$LABANALYSER_ROOT/plugins/platforminterface.h \
           $$LABANALYSER_ROOT/plugins/InterfaceDataType.h
```

The plugin class must implement `Platform_Fabric`, export the existing Qt
interface IID, and use Qt plugin metadata:

```cpp
#include <QObject>
#include "plugins/platforminterface.h"

class MyLabAnalyserPlugin : public QObject, public Platform_Fabric
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EchoInterface_iid)
    Q_INTERFACES(Platform_Fabric)

public:
    Platform_Interface *GetInterface(QObject *messenger) override;
};
```

Keep `plugins/platforminterface.h` and `plugins/InterfaceDataType.h` compatible
with the LabAnalyser version that loads the DLL. Do not change the interface IID
or replace these types with local variants. Prefer the public plugin interface
over direct dependencies on LabAnalyser implementation classes.

Build a plugin from a separate build directory:

```powershell
$env:PATH = "C:\msys64\mingw64\bin;C:\msys64\usr\bin;$env:PATH"
$env:MSYSTEM = 'MINGW64'

New-Item -ItemType Directory -Force build\msys2-mingw64-release | Out-Null
Push-Location build\msys2-mingw64-release
& C:\msys64\mingw64\bin\qmake6.exe ..\..\MyLabAnalyserPlugin.pro -spec win32-g++ `
    'CONFIG+=release' 'CONFIG-=debug'
& C:\msys64\mingw64\bin\mingw32-make.exe -j$env:NUMBER_OF_PROCESSORS
Pop-Location
```

Set `DESTDIR` in the plugin `.pro` file, or copy the resulting DLL to the exact
path referenced by the `DevicePlugin` attribute in the LabAnalyser experiment
XML. `QPluginLoader` loads that file path directly.

```qmake
DESTDIR = C:/path/referenced/by/the/experiment
```

For a standalone plugin test environment, place the required Qt and third-party
DLLs next to the host executable or make them available through `PATH`. Use
`C:\msys64\mingw64\bin\windeployqt6.exe --release --compiler-runtime <host.exe>`
to deploy the Qt runtime for the host application.

## Troubleshooting

- `qmake`, `moc`, `uic`, or `rcc` fails to start: put
  `C:\msys64\mingw64\bin` first in `PATH`; remove conflicting Qt directories
  from the active shell and start a new PowerShell session.
- Headers or libraries are not found: pass paths through `INCLUDEPATH +=` and
  `LIBS += -L... -l...` in the `.pro` file, or use `QMAKE_LIBDIR+=...` on the
  qmake command line.
- The DLL is not loaded: verify the `DevicePlugin` path, the matching interface
  IID, the target architecture, and every dependent DLL using `PATH`.
- A source change is ignored: run qmake again, then rebuild. Delete only the
  project-local build directory for a clean rebuild.

## Repository Hygiene

Keep generated files out of version control. The repository `.gitignore` covers
qmake, CMake, compiler, linker, IDE, test, and build-directory artefacts.
Commit `.pro`, `.cpp`, `.h`, `.ui`, `.qrc`, resources, and source tests; do not
commit generated `Makefile`, `moc_*`, `ui_*`, `qrc_*`, object files, DLLs, or
executables.

# AGENTS.md

## Zweck

Diese Datei definiert verbindliche Regeln für alle Entwickler und Coding-Agents, die an diesem C++-Projekt arbeiten.

Das Ziel ist Code, der:

* korrekt,
* verständlich,
* wartbar,
* testbar,
* sicher,
* performant,
* konsistent

ist.

Neue Änderungen müssen sich in die bestehende Architektur einfügen. Konsistenz mit gutem bestehendem Code hat Vorrang vor persönlichen Präferenzen.

---

# 1. Allgemeine Grundsätze

## 1.1 Programmiersprache

* Verwende mindestens **C++20**.
* Nutze moderne C++-Sprachmittel, wenn sie Lesbarkeit und Sicherheit verbessern.
* Vermeide unnötige Abhängigkeiten von compiler- oder plattformspezifischem Verhalten.
* Verwende keine veralteten C-Idiome, wenn eine sichere C++-Alternative existiert.

Bevorzugt:

```cpp
std::string
std::vector
std::array
std::span
std::optional
std::variant
std::unique_ptr
std::shared_ptr
std::filesystem
std::chrono
```

Zu vermeiden:

```cpp
char*
malloc
calloc
realloc
free
#define für Konstanten
manuelle Speicherverwaltung
```

## 1.2 Prioritäten

Bei Designentscheidungen gilt folgende Reihenfolge:

1. Korrektheit
2. Sicherheit
3. Verständlichkeit
4. Wartbarkeit
5. Testbarkeit
6. Performance
7. Kürze

Kurzer Code ist nicht automatisch guter Code.

## 1.3 Umfang von Änderungen

* Ändere nur Code, der für die Aufgabe relevant ist.
* Vermeide große Refactorings innerhalb fachlich kleiner Änderungen.
* Entferne keinen bestehenden Code ohne nachvollziehbaren Grund.
* Ändere öffentliche Schnittstellen nur, wenn dies ausdrücklich erforderlich ist.
* Bewahre Abwärtskompatibilität, sofern keine andere Entscheidung dokumentiert wurde.
* Ergänze bei Verhaltensänderungen passende Tests.

---

# 2. Dateinamen und Verzeichnisstruktur

## 2.1 Dateinamen

Alle C++-Dateien verwenden `snake_case`.

Erlaubt:

```text
http_client.cpp
http_client.hpp
user_repository.cpp
user_repository.hpp
application_config.hpp
```

Nicht erlaubt:

```text
HttpClient.cpp
httpClient.cpp
HTTPClient.cpp
http-client.cpp
```

## 2.2 Dateiendungen

Verwende:

```text
.hpp    Header-Dateien
.cpp    Implementierungsdateien
.inl    optionale Template-Implementierungen
```

Verwende innerhalb eines Projekts keine Mischung aus `.h`, `.hh`, `.hpp`, `.cc`, `.cxx` und `.cpp`.

## 2.3 Testdateien

Testdateien enden mit:

```text
_test.cpp
```

Beispiele:

```text
http_client_test.cpp
user_repository_test.cpp
```

## 2.4 Verzeichnisnamen

Verzeichnisse verwenden `snake_case`.

Beispiel:

```text
src/
include/
tests/
cmake/
tools/
third_party/
```

Fachliche Module erhalten eigene Verzeichnisse:

```text
src/network/
src/storage/
src/authentication/
src/configuration/
```

## 2.5 Zuordnung von Klassen zu Dateien

* Eine zentrale Klasse erhält normalerweise ein eigenes Header-/Source-Paar.
* Der Dateiname entspricht dem Klassennamen in `snake_case`.
* Kleine Hilfstypen dürfen gemeinsam mit dem zugehörigen Haupttyp definiert werden.
* Sammeldateien wie `utils.cpp`, `helpers.cpp` oder `common.cpp` sind zu vermeiden.

Beispiel:

```text
Klasse: HttpClient
Dateien:
http_client.hpp
http_client.cpp
```

---

# 3. Formatierung

## 3.1 Automatische Formatierung

* Der gesamte C++-Code muss mit `clang-format` formatiert werden.
* Die projektweite `.clang-format` ist verbindlich.
* Formatierung darf nicht manuell gegen die Konfiguration erzwungen werden.
* Reine Formatierungsänderungen sollen nicht mit funktionalen Änderungen vermischt werden.

## 3.2 Einrückung

* Verwende vier Leerzeichen.
* Verwende keine Tabs.
* Eine Zeile sollte höchstens 100 bis 120 Zeichen enthalten.
* Lange Ausdrücke müssen sinnvoll umgebrochen werden.

## 3.3 Klammern

Öffnende geschweifte Klammern stehen in derselben Zeile.

```cpp
if (is_ready) {
    start();
}
```

Dies gilt auch für Funktionen, Schleifen, Klassen und Namespaces.

## 3.4 Kontrollstrukturen

Geschweifte Klammern sind immer erforderlich.

Richtig:

```cpp
if (is_valid) {
    process();
}
```

Falsch:

```cpp
if (is_valid)
    process();
```

## 3.5 Leerzeichen

Richtig:

```cpp
const int result = left + right;

if (result > 0) {
    process(result);
}
```

Falsch:

```cpp
const int result=left+right;

if(result>0){
    process(result);
}
```

---

# 4. Benennungsregeln

## 4.1 Grundregel

Namen müssen die fachliche Bedeutung ausdrücken.

Namen sollen beantworten:

* Was enthält dieser Wert?
* Welche Einheit besitzt er?
* Welche Verantwortung hat diese Funktion?
* Welche Bedingung beschreibt dieser boolesche Wert?
* Wem gehört das Objekt?

Vermeide unklare Abkürzungen und generische Namen.

Zu vermeiden:

```cpp
int x;
int tmp;
int val;
int data;
int obj;
int res;
int ret;
```

Besser:

```cpp
int retry_count;
std::string customer_name;
RequestResult request_result;
std::chrono::milliseconds timeout;
```

## 4.2 Variablen

Lokale Variablen verwenden `snake_case`.

```cpp
const std::size_t message_count = messages.size();
const auto connection_timeout = std::chrono::seconds{5};
```

## 4.3 Membervariablen

Private und geschützte Membervariablen verwenden `snake_case_` mit nachgestelltem Unterstrich.

```cpp
class HttpClient {
private:
    std::string base_url_;
    std::chrono::milliseconds timeout_;
    bool connected_;
};
```

Keine Präfixe wie:

```cpp
m_value
mValue
_value
```

## 4.4 Globale Variablen

Globale veränderbare Variablen sind grundsätzlich verboten.

Falls ein globaler Zustand unvermeidbar ist:

* muss er begründet werden,
* muss sein Zugriff gekapselt sein,
* muss Thread-Sicherheit berücksichtigt werden,
* muss seine Lebensdauer eindeutig sein.

## 4.5 Konstanten

Compile-Time-Konstanten verwenden `kPascalCase`.

```cpp
constexpr std::size_t kMaximumRetryCount = 3;
constexpr auto kDefaultTimeout = std::chrono::seconds{5};
```

Keine Präprozessor-Konstanten:

```cpp
#define MAX_RETRY_COUNT 3
```

Stattdessen:

```cpp
constexpr std::size_t kMaximumRetryCount = 3;
```

## 4.6 Funktionen

Funktionen und Methoden verwenden `snake_case`.

```cpp
void connect_to_server();
bool is_connection_available() const;
std::string load_configuration();
```

Funktionsnamen beginnen normalerweise mit einem Verb.

Beispiele:

```cpp
load_user()
save_document()
calculate_checksum()
validate_request()
create_connection()
```

## 4.7 Boolesche Werte

Boolesche Variablen und Funktionen müssen wie Fragen lesbar sein.

Bevorzugte Präfixe:

```text
is_
has_
can_
should_
was_
needs_
supports_
```

Beispiele:

```cpp
bool is_valid;
bool has_permission;
bool should_retry;
bool can_connect;
```

Nicht verwenden:

```cpp
bool valid;
bool permission;
bool retry;
bool connection;
```

## 4.8 Klassen und Strukturen

Klassen, Strukturen, Enums, Aliase und Konzepte verwenden `PascalCase`.

```cpp
class HttpClient;
struct UserRecord;
enum class ConnectionState;
using UserId = std::uint64_t;
template<typename T>
concept Serializable;
```

## 4.9 Namespaces

Namespaces verwenden `snake_case`.

```cpp
namespace peltecs::network {
}
```

## 4.10 Enum-Werte

Enum-Werte verwenden `PascalCase`.

```cpp
enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Failed
};
```

Verwende immer `enum class` anstelle von unscoped `enum`.

## 4.11 Template-Parameter

Template-Typen verwenden aussagekräftiges `PascalCase`.

```cpp
template<typename ValueType>
class Cache;
```

Einzelbuchstaben sind nur bei allgemein bekannten mathematischen oder sehr kleinen generischen Kontexten zulässig.

```cpp
template<typename T>
```

## 4.12 Abkürzungen

Abkürzungen werden wie normale Wörter behandelt.

Richtig:

```cpp
HttpClient
JsonParser
XmlDocument
UserId
```

Nicht:

```cpp
HTTPClient
JSONParser
XMLDocument
UserID
```

Ausgenommen sind etablierte externe API-Namen, die exakt übernommen werden müssen.

## 4.13 Einheiten in Namen

Bei numerischen Werten muss die Einheit erkennbar sein, sofern der Typ sie nicht bereits ausdrückt.

Besser:

```cpp
std::chrono::milliseconds timeout;
std::uint64_t file_size_bytes;
double temperature_celsius;
```

Schlechter:

```cpp
int timeout;
int size;
double temperature;
```

---

# 5. Variablenregeln

## 5.1 Initialisierung

Jede Variable muss bei ihrer Deklaration initialisiert werden.

Richtig:

```cpp
int retry_count = 0;
std::string response_body;
ConnectionState state = ConnectionState::Disconnected;
```

Falsch:

```cpp
int retry_count;
```

## 5.2 Kleinster Gültigkeitsbereich

Variablen werden so spät wie möglich und im kleinsten sinnvollen Scope deklariert.

Richtig:

```cpp
if (request.is_valid()) {
    const auto response = client.send(request);
    handle_response(response);
}
```

Nicht unnötig früh:

```cpp
Response response;

if (request.is_valid()) {
    response = client.send(request);
    handle_response(response);
}
```

## 5.3 `const`

Verwende `const`, sobald ein Wert nach der Initialisierung nicht mehr geändert wird.

```cpp
const auto user = repository.load_user(user_id);
const std::size_t item_count = items.size();
```

Memberfunktionen, die den Objektzustand nicht verändern, müssen `const` sein.

```cpp
bool is_connected() const;
```

## 5.4 `auto`

`auto` ist erlaubt, wenn der Typ:

* offensichtlich ist,
* sehr lang ist,
* durch Iteratoren oder Templates entsteht,
* nicht für das Verständnis entscheidend ist.

Gut:

```cpp
const auto timeout = std::chrono::seconds{5};
const auto user = repository.load_user(user_id);
for (const auto& item : items) {
}
```

Expliziter Typ bevorzugt:

```cpp
std::uint32_t retry_count = 0;
ConnectionState state = ConnectionState::Disconnected;
```

Nicht verwenden, wenn dadurch wichtige Typinformationen verborgen werden:

```cpp
auto value = calculate_value();
```

## 5.5 Ganzzahltypen

* Verwende feste Breiten wie `std::uint32_t`, wenn die Breite fachlich oder binär relevant ist.
* Verwende `std::size_t` für Größen und Indizes, die aus Standardcontainern stammen.
* Verwende keine vorzeichenlosen Typen nur zur Darstellung „nicht negativer“ fachlicher Werte.
* Vermeide implizite Konvertierungen zwischen signed und unsigned.

## 5.6 Fließkommazahlen

Fließkommazahlen dürfen nicht direkt mit `==` verglichen werden, außer exakte Repräsentation ist garantiert.

Zu vermeiden:

```cpp
if (result == 0.1) {
}
```

Bevorzugt:

```cpp
if (std::abs(result - expected) < epsilon) {
}
```

## 5.7 Magic Numbers

Nicht offensichtliche Zahlenwerte müssen benannt werden.

Falsch:

```cpp
if (retry_count > 3) {
}
```

Richtig:

```cpp
constexpr std::size_t kMaximumRetryCount = 3;

if (retry_count > kMaximumRetryCount) {
}
```

Offensichtliche Werte wie `0`, `1` oder `2` dürfen verwendet werden, wenn ihre Bedeutung eindeutig ist.

---

# 6. Funktionen

## 6.1 Eine Verantwortung

Eine Funktion soll genau eine klar erkennbare Aufgabe erfüllen.

Wenn der Funktionsname „und“ enthalten müsste, besitzt die Funktion wahrscheinlich mehrere Verantwortlichkeiten.

Schlecht:

```cpp
void load_validate_and_save_configuration();
```

Besser:

```cpp
Configuration load_configuration();
ValidationResult validate_configuration(const Configuration& configuration);
void save_configuration(const Configuration& configuration);
```

## 6.2 Funktionslänge

* Funktionen sollen normalerweise weniger als 40 Zeilen umfassen.
* Lange Funktionen müssen in logisch benannte Teilfunktionen zerlegt werden.
* Eine Funktion darf länger sein, wenn eine Aufteilung die Verständlichkeit verschlechtern würde.

## 6.3 Parameteranzahl

* Bevorzuge maximal vier Parameter.
* Bei vielen zusammengehörigen Parametern ist ein eigener Typ zu erstellen.
* Boolesche Steuerparameter sind zu vermeiden.

Schlecht:

```cpp
send_message(message, true, false, true);
```

Besser:

```cpp
SendOptions options{
    .encrypt = true,
    .compress = false,
    .request_confirmation = true
};

send_message(message, options);
```

## 6.4 Parameterübergabe

Verwende:

* kleine triviale Typen per Wert,
* große unveränderliche Typen per `const&`,
* veränderbare Eingabeparameter per `&`,
* optionale Besitzübertragung per `std::unique_ptr`,
* nicht besitzende Sequenzen per `std::span`,
* Textansichten per `std::string_view`.

Beispiele:

```cpp
void set_retry_count(std::uint32_t retry_count);

void process_request(const Request& request);

void update_user(User& user);

void consume_task(std::unique_ptr<Task> task);

void process_values(std::span<const int> values);

bool starts_with(std::string_view text, std::string_view prefix);
```

## 6.5 Rückgabewerte

* Bevorzuge Rückgabewerte gegenüber Output-Parametern.
* Verwende `std::optional<T>`, wenn ein Wert gültig fehlen kann.
* Verwende einen Ergebnistyp, wenn sowohl Wert als auch Fehler relevant sind.
* Gib keine Referenz oder Zeiger auf lokale Variablen zurück.

Bevorzugt:

```cpp
std::optional<User> find_user(UserId user_id);
```

Nicht:

```cpp
bool find_user(UserId user_id, User& output_user);
```

## 6.6 `[[nodiscard]]`

Funktionen, deren Rückgabewert nicht ignoriert werden darf, erhalten `[[nodiscard]]`.

```cpp
[[nodiscard]] bool save_configuration();
[[nodiscard]] ValidationResult validate_request(const Request& request);
```

## 6.7 `noexcept`

Verwende `noexcept`, wenn eine Funktion garantiert keine Exception auslöst.

Insbesondere prüfen bei:

* Destruktoren,
* Move-Konstruktoren,
* Move-Zuweisungsoperatoren,
* einfachen Zugriffsfunktionen,
* Swap-Funktionen.

```cpp
HttpClient(HttpClient&& other) noexcept;
HttpClient& operator=(HttpClient&& other) noexcept;
```

---

# 7. Klassen und Objektdesign

## 7.1 Kapselung

* Membervariablen sind grundsätzlich `private`.
* Öffentliche Datenmember sind nur für einfache passive Datenstrukturen zulässig.
* Klassen müssen ihre Invarianten selbst schützen.
* Ein Objekt darf nach erfolgreicher Konstruktion nicht in einem ungültigen Zustand sein.

## 7.2 `class` und `struct`

Verwende `struct`, wenn:

* der Typ primär Daten enthält,
* seine Member öffentlich sein dürfen,
* keine komplexen Invarianten bestehen.

Verwende `class`, wenn:

* Daten gekapselt werden,
* Invarianten geschützt werden,
* Verhalten im Vordergrund steht.

## 7.3 Konstruktoren

* Konstruktoren mit einem Parameter müssen `explicit` sein.
* Konstruktoren dürfen keine unnötig komplexe Logik enthalten.
* Fehlerhafte Konstruktion muss verhindert oder eindeutig signalisiert werden.

```cpp
class UserId {
public:
    explicit UserId(std::uint64_t value);
};
```

## 7.4 Rule of Zero

Bevorzuge die Rule of Zero.

Klassen sollen Ressourcen über Standardtypen verwalten:

```cpp
std::string
std::vector
std::unique_ptr
std::shared_ptr
```

Eigene Destruktoren, Copy- oder Move-Operationen sind nur zu implementieren, wenn dies tatsächlich erforderlich ist.

## 7.5 Vererbung

* Bevorzuge Komposition gegenüber Vererbung.
* Verwende Vererbung nur für echte Ist-ein-Beziehungen.
* Basisklassen mit virtuellen Methoden benötigen einen virtuellen Destruktor.
* Überschriebene Methoden müssen `override` verwenden.
* Verwende nicht gleichzeitig `virtual` und `override`.

```cpp
class Handler {
public:
    virtual ~Handler() = default;
    virtual void handle(const Request& request) = 0;
};

class LoggingHandler final : public Handler {
public:
    void handle(const Request& request) override;
};
```

## 7.6 `final`

Verwende `final`, wenn eine Klasse oder Methode nicht weiter überschrieben werden soll.

## 7.7 Getter und Setter

Erstelle Getter und Setter nicht automatisch.

Ein Objekt soll fachliche Operationen anbieten.

Schlecht:

```cpp
account.set_balance(account.get_balance() - amount);
```

Besser:

```cpp
account.withdraw(amount);
```

---

# 8. Speicherverwaltung und Ownership

## 8.1 Keine manuelle Speicherverwaltung

Direkte Verwendung von `new` und `delete` ist grundsätzlich verboten.

Falsch:

```cpp
Widget* widget = new Widget();
delete widget;
```

Richtig:

```cpp
auto widget = std::make_unique<Widget>();
```

## 8.2 Smart Pointer

Verwende:

* `std::unique_ptr` für eindeutigen Besitz,
* `std::shared_ptr` nur für tatsächlich geteilten Besitz,
* `std::weak_ptr` zum Aufbrechen zyklischer Abhängigkeiten.

`std::shared_ptr` darf nicht als Standardlösung verwendet werden.

## 8.3 Rohe Zeiger

Rohe Zeiger dürfen nicht besitzend sein.

Ein roher Zeiger bedeutet:

* optionale nicht besitzende Referenz,
* externe Lebensdauer,
* möglicherweise `nullptr`.

Wenn `nullptr` nicht zulässig ist, verwende eine Referenz.

```cpp
void process(User& user);
void process_optional(User* user);
```

## 8.4 Lebensdauer

* Speichere keine Referenzen oder `std::string_view`, deren Quelle früher zerstört werden kann.
* Dokumentiere nicht offensichtliche Lebensdaueranforderungen.
* Gib keine Views auf temporäre Objekte zurück.

---

# 9. Fehlerbehandlung

## 9.1 Fehler dürfen nicht ignoriert werden

Jeder Fehler muss:

* behandelt,
* weitergegeben,
* protokolliert,
* oder bewusst dokumentiert verworfen

werden.

Leere Catch-Blöcke sind verboten.

Falsch:

```cpp
try {
    save();
} catch (...) {
}
```

## 9.2 Exceptions

Exceptions dürfen für unerwartete Fehler verwendet werden, nicht für normale Kontrollflüsse.

Geeignet:

* Ressourcen konnten nicht initialisiert werden,
* eine erforderliche Invariante wurde verletzt,
* eine Operation kann nicht sinnvoll fortgesetzt werden.

Nicht geeignet:

* „Element nicht gefunden“ als normaler Fall,
* Validierungsfehler bei Benutzereingaben,
* erwartbare optionale Ergebnisse.

## 9.3 Exception-Typen

* Wirf konkrete Exception-Typen.
* Fange Exceptions bevorzugt per `const&`.
* Fange nicht pauschal `...`, außer an Prozess- oder Thread-Grenzen.
* Verwende beim Weiterwerfen `throw;`.

```cpp
catch (const std::runtime_error& error) {
    log_error(error.what());
    throw;
}
```

## 9.4 Fehlermeldungen

Fehlermeldungen müssen enthalten:

* was fehlgeschlagen ist,
* welches Objekt betroffen ist,
* relevante Identifikatoren,
* keine vertraulichen Daten.

Schlecht:

```text
Operation failed
```

Besser:

```text
Failed to load configuration file '/etc/example/config.json'
```

## 9.5 Assertions

`assert` ist nur für interne Programmierfehler und Invarianten zulässig.

`assert` darf nicht zur Behandlung externer Eingaben oder Laufzeitfehler verwendet werden.

---

# 10. Kontrollfluss

## 10.1 Early Return

Bevorzuge frühe Rückgaben, um Verschachtelung zu reduzieren.

Schlecht:

```cpp
if (request.is_valid()) {
    if (user.has_permission()) {
        process(request);
    }
}
```

Besser:

```cpp
if (!request.is_valid()) {
    return;
}

if (!user.has_permission()) {
    return;
}

process(request);
```

## 10.2 Verschachtelung

* Vermeide mehr als drei Verschachtelungsebenen.
* Teile komplexe Logik in Funktionen auf.
* Nutze Guard Clauses.

## 10.3 Schleifen

* Bevorzuge Range-based-for-Schleifen.
* Verwende Standardalgorithmen, wenn sie die Absicht klarer ausdrücken.
* Vermeide komplizierte Schleifenbedingungen.

Bevorzugt:

```cpp
for (const auto& user : users) {
    notify(user);
}
```

Oder:

```cpp
const auto matching_user = std::ranges::find(users, user_id, &User::id);
```

## 10.4 `switch`

* Verwende `enum class`.
* Behandle alle Enum-Werte explizit.
* Vermeide `default`, wenn der Compiler bei neuen Enum-Werten warnen soll.
* Markiere beabsichtigtes Durchfallen mit `[[fallthrough]]`.

---

# 11. Header-Dateien

## 11.1 Include Guards

Verwende `#pragma once`.

```cpp
#pragma once
```

## 11.2 Self-contained Header

Jeder Header muss eigenständig kompilierbar sein.

Ein Header muss alle Typen inkludieren oder deklarieren, die er benötigt.

## 11.3 Include-Reihenfolge

Reihenfolge in `.cpp`-Dateien:

1. eigener Header,
2. projektinterne Header,
3. externe Bibliotheken,
4. Standardbibliothek.

Beispiel:

```cpp
#include "network/http_client.hpp"

#include "configuration/application_config.hpp"
#include "logging/logger.hpp"

#include <fmt/format.h>

#include <chrono>
#include <string>
#include <utility>
```

Zwischen den Gruppen steht jeweils eine Leerzeile.

## 11.4 Keine indirekten Includes

Verlasse dich nicht darauf, dass ein benötigter Header indirekt durch einen anderen Header eingebunden wird.

## 11.5 Forward Declarations

Forward Declarations sind erlaubt, wenn sie:

* Compile-Zeiten verbessern,
* keine fragile Kopplung verursachen,
* für den verwendeten Typ ausreichen.

Bei Standardbibliothekstypen sind keine eigenen Forward Declarations erlaubt.

## 11.6 Header-Inhalt

Header sollen nur enthalten, was für die öffentliche Schnittstelle erforderlich ist.

Implementierungsdetails gehören in `.cpp`-Dateien.

## 11.7 `using namespace`

`using namespace` ist in Header-Dateien verboten.

Auch in `.cpp`-Dateien soll es vermieden werden.

Falsch:

```cpp
using namespace std;
```

---

# 12. Präprozessor

## 12.1 Makros

Makros sind zu vermeiden.

Erlaubte Anwendungsfälle:

* Plattformabstraktion,
* Compilerattribute,
* Include Guards, falls `#pragma once` nicht verwendet wird,
* externe Framework-Anforderungen.

Bevorzugte Alternativen:

```cpp
constexpr
inline
template
enum class
using
```

## 12.2 Bedingte Kompilierung

Plattformspezifischer Code muss gekapselt werden.

Schlecht:

```cpp
#ifdef _WIN32
// Plattformlogik mitten in der Fachlogik
#endif
```

Besser:

```cpp
std::unique_ptr<FileSystemBackend> create_platform_file_system();
```

---

# 13. Nebenläufigkeit

## 13.1 Thread-Sicherheit

Thread-Sicherheit muss explizit entschieden und dokumentiert werden.

Eine Klasse ist entweder:

* thread-safe,
* bedingt thread-safe,
* oder nicht thread-safe.

## 13.2 Synchronisation

* Schütze gemeinsam veränderbaren Zustand.
* Halte Lock-Bereiche möglichst klein.
* Führe keine langsamen Operationen unter einem Lock aus.
* Rufe nach Möglichkeit keine fremden Callbacks unter einem Lock auf.
* Bevorzuge RAII-Locks.

```cpp
std::scoped_lock lock{mutex_};
```

## 13.3 Atomics

Atomics dürfen nur verwendet werden, wenn ihre Speicherordnung verstanden wird.

Bei Unsicherheit:

```cpp
std::memory_order_seq_cst
```

oder eine mutexbasierte Lösung verwenden.

## 13.4 Threads

* Verwende bevorzugt `std::jthread`.
* Jeder gestartete Thread benötigt eine eindeutige Lebensdauer.
* Stop- und Shutdown-Verhalten müssen definiert sein.
* Exceptions dürfen Thread-Grenzen nicht unkontrolliert überschreiten.

---

# 14. Logging

## 14.1 Log-Level

Verwende Log-Level konsistent:

```text
trace    sehr detaillierte Diagnose
debug    technische Entwicklungsinformationen
info     normaler Betriebsablauf
warning  unerwarteter, aber behandelbarer Zustand
error    fehlgeschlagene Operation
critical Prozess oder Hauptfunktion nicht fortsetzbar
```

## 14.2 Log-Inhalt

Logs sollen enthalten:

* Operation,
* betroffene ID,
* relevanten Zustand,
* Fehlerursache.

Logs dürfen nicht enthalten:

* Passwörter,
* Tokens,
* API-Keys,
* private Schlüssel,
* sensible personenbezogene Daten,
* vollständige vertrauliche Payloads.

## 14.3 Keine Doppelprotokollierung

Ein Fehler soll nicht auf jeder Ebene erneut geloggt werden.

Grundregel:

* Entweder behandeln und loggen,
* oder weitergeben.

---

# 15. Kommentare und Dokumentation

## 15.1 Kommentare erklären das Warum

Kommentare sollen erklären:

* warum eine ungewöhnliche Lösung notwendig ist,
* welche Invariante gilt,
* welcher Workaround verwendet wird,
* welche externe Einschränkung besteht.

Kommentare sollen nicht wiederholen, was der Code bereits sagt.

Schlecht:

```cpp
// Erhöhe den Zähler
retry_count++;
```

Besser:

```cpp
// Der erste Verbindungsversuch zählt bereits als Versuch.
++retry_count;
```

## 15.2 Veraltete Kommentare

Bei Codeänderungen müssen zugehörige Kommentare aktualisiert oder entfernt werden.

Falsche Kommentare sind schlimmer als fehlende Kommentare.

## 15.3 TODO-Kommentare

TODOs müssen einen konkreten Kontext besitzen.

```cpp
// TODO(PROJ-184): Remove compatibility path after migration to API v3.
```

Nicht:

```cpp
// TODO: fix later
```

## 15.4 Öffentliche APIs

Öffentliche Klassen und Funktionen müssen dokumentiert werden, wenn:

* ihr Verhalten nicht offensichtlich ist,
* besondere Vorbedingungen gelten,
* Fehlerfälle relevant sind,
* Ownership oder Lebensdauer wichtig ist,
* Nebenwirkungen auftreten.

---

# 16. Tests

## 16.1 Testpflicht

Neue oder geänderte Logik benötigt Tests.

Mindestens zu prüfen:

* Normalfall,
* Grenzfälle,
* Fehlerfälle,
* ungültige Eingaben,
* relevante Zustandsübergänge.

## 16.2 Testbenennung

Testnamen müssen das erwartete Verhalten beschreiben.

Beispiel:

```cpp
TEST(HttpClientTest, ReturnsErrorWhenConnectionTimesOut)
```

Oder bei `snake_case`-basierten Testframeworks:

```cpp
http_client_returns_error_when_connection_times_out
```

## 16.3 Teststruktur

Tests verwenden Arrange, Act, Assert.

```cpp
TEST(UserRepositoryTest, ReturnsUserWhenIdExists) {
    // Arrange
    UserRepository repository;
    repository.add(User{UserId{42}, "Ada"});

    // Act
    const auto result = repository.find(UserId{42});

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name(), "Ada");
}
```

Die Kommentare dürfen entfallen, wenn die Struktur eindeutig ist.

## 16.4 Testunabhängigkeit

Tests müssen:

* unabhängig voneinander laufen,
* wiederholbar sein,
* keine Ausführungsreihenfolge voraussetzen,
* keine externen Systeme ohne explizite Integrationstestumgebung benötigen.

## 16.5 Keine produktive Logik nur für Tests

Produktivcode darf nicht mit Test-Sonderfällen verschmutzt werden.

Stattdessen sind Abhängigkeiten injizierbar zu gestalten.

---

# 17. Sicherheit

## 17.1 Eingabevalidierung

Alle externen Eingaben müssen validiert werden.

Dazu gehören:

* Netzwerkdaten,
* Dateien,
* Kommandozeilenparameter,
* Umgebungsvariablen,
* Datenbankwerte,
* Konfigurationsdateien,
* Benutzereingaben.

## 17.2 Grenzen und Größen

Vor Speicherallokationen und Schleifen müssen externe Größen geprüft werden.

```cpp
if (payload_size > kMaximumPayloadSize) {
    return PayloadError::TooLarge;
}
```

## 17.3 String- und Buffer-Sicherheit

* Verwende keine unsicheren C-Funktionen.
* Vermeide `strcpy`, `strcat`, `sprintf` und ähnliche Funktionen.
* Verwende `std::string`, `std::span`, `std::array` und sichere Formatierungsbibliotheken.
* Prüfe Indizes vor Zugriffen, wenn sie aus externen Quellen stammen.

## 17.4 Geheimnisse

Geheimnisse dürfen nicht:

* im Quellcode,
* in Logs,
* in Tests,
* in Fehlermeldungen,
* in Repository-Dateien

gespeichert werden.

---

# 18. Performance

## 18.1 Keine unbegründete Optimierung

Optimiere erst, wenn:

* ein messbares Problem besteht,
* ein Profiling-Ergebnis vorliegt,
* die Änderung nachvollziehbar dokumentiert werden kann.

## 18.2 Kopien vermeiden

Vermeide unnötige Kopien großer Objekte.

Nutze:

```cpp
const&
std::move
std::string_view
std::span
emplace_back
reserve
```

Nur wenn Ownership und Lebensdauer eindeutig sind.

## 18.3 `std::move`

Verwende `std::move` nur, wenn das Quellobjekt anschließend nicht mehr benötigt wird.

Verwende kein `std::move` beim Rückgeben lokaler Variablen, wenn dadurch Return Value Optimization verhindert oder erschwert werden kann.

Bevorzugt:

```cpp
return result;
```

Nicht unnötig:

```cpp
return std::move(result);
```

## 18.4 Containerwahl

Wähle Container nach Zugriffsmuster:

* `std::vector` als Standardcontainer,
* `std::array` für feste Größe,
* `std::unordered_map` für Schlüsselzugriff ohne Reihenfolge,
* `std::map` nur bei benötigter Sortierung,
* `std::deque` bei stabilen Einfügungen an beiden Enden.

Verwende keine verkettete Liste ohne konkrete Begründung.

---

# 19. Build-System und Compiler

## 19.1 CMake

* Verwende target-basiertes modernes CMake.
* Nutze keine globalen Include-Pfade oder Compilerflags.
* Abhängigkeiten werden pro Target definiert.

Bevorzugt:

```cmake
target_include_directories(
    project_core
    PUBLIC
        include
)

target_link_libraries(
    project_core
    PRIVATE
        fmt::fmt
)
```

## 19.2 Compiler-Warnungen

Warnungen müssen aktiviert und ernst genommen werden.

Empfohlene GCC-/Clang-Optionen:

```text
-Wall
-Wextra
-Wpedantic
-Wconversion
-Wsign-conversion
-Wshadow
-Wnon-virtual-dtor
-Wold-style-cast
-Woverloaded-virtual
-Wnull-dereference
-Wdouble-promotion
-Wformat=2
```

Für MSVC:

```text
/W4
/permissive-
```

Projektcode sollte in CI mit „Warnungen als Fehler“ gebaut werden.

## 19.3 Sanitizer

In Entwicklungs- oder CI-Builds sollen verwendet werden:

```text
AddressSanitizer
UndefinedBehaviorSanitizer
ThreadSanitizer, sofern relevant
```

---

# 20. Statische Analyse

Der Code soll regelmäßig mit folgenden Werkzeugen geprüft werden:

```text
clang-tidy
clang-format
include-what-you-use
cppcheck
```

Mindestens `clang-format` und `clang-tidy` sollen automatisiert in CI ausgeführt werden.

Warnungen dürfen nicht ohne Begründung deaktiviert werden.

Eine Unterdrückung muss lokal und dokumentiert sein.

---

# 21. Abhängigkeiten

## 21.1 Neue Bibliotheken

Eine neue Abhängigkeit darf nur hinzugefügt werden, wenn:

* Standardbibliothek oder bestehende Abhängigkeiten nicht ausreichen,
* Wartungszustand und Lizenz geprüft wurden,
* der Nutzen den zusätzlichen Aufwand rechtfertigt,
* Sicherheits- und Update-Risiken berücksichtigt wurden.

## 21.2 Abhängigkeitsgrenzen

Externe Bibliothekstypen sollen nicht unnötig Teil öffentlicher Projekt-APIs werden.

Kapsle externe Bibliotheken hinter eigenen Schnittstellen, wenn ein späterer Austausch realistisch ist.

---

# 22. API-Design

## 22.1 Eindeutige Verträge

Jede öffentliche Funktion muss einen klaren Vertrag besitzen:

* gültige Eingaben,
* Rückgabewert,
* Fehlerverhalten,
* Nebenwirkungen,
* Ownership,
* Thread-Sicherheit.

## 22.2 Starke Typen

Verwende unterschiedliche Typen für fachlich unterschiedliche Werte.

Schlecht:

```cpp
void transfer(std::uint64_t from, std::uint64_t to, double amount);
```

Besser:

```cpp
void transfer(AccountId from, AccountId to, Money amount);
```

## 22.3 Ungültige Zustände vermeiden

Modelliere Daten so, dass ungültige Zustände möglichst nicht darstellbar sind.

Verwende:

```cpp
enum class
std::optional
std::variant
starke Typen
validierende Konstruktoren
Factory-Funktionen
```

---

# 23. Verbotene oder unerwünschte Praktiken

Folgende Praktiken sind ohne zwingende und dokumentierte Begründung verboten:

```text
using namespace std;
globale veränderbare Variablen
direktes new und delete
malloc und free
C-Style-Casts
reinterpret_cast ohne technische Begründung
#define für Konstanten
leere catch-Blöcke
ignorierte Fehlercodes
uninitialisierte Variablen
Magic Numbers
unklare Einbuchstabenvariablen
boolesche Parameter ohne benannten Optionstyp
gemeinsamer Besitz ohne Notwendigkeit
unnötige Singleton-Architekturen
God Classes
Funktionen mit mehreren Verantwortlichkeiten
öffentliche Membervariablen in zustandsbehafteten Klassen
plattformabhängige Logik in Fachcode
Passwörter, Tokens oder Schlüssel im Repository
```

C-Style-Casts sind nicht erlaubt:

```cpp
int value = (int)number;
```

Verwende den passenden C++-Cast:

```cpp
int value = static_cast<int>(number);
```

---

# 24. Regeln für Coding-Agents

Ein Coding-Agent muss vor jeder Änderung:

1. die betroffenen Dateien lesen,
2. die bestehende Architektur verstehen,
3. bestehende Namens- und Formatierungsmuster prüfen,
4. vorhandene Tests identifizieren,
5. den kleinstmöglichen sinnvollen Änderungsumfang wählen.

Ein Coding-Agent darf nicht:

* APIs erfinden, ohne ihre Nutzung zu prüfen,
* nicht existierende Bibliotheksfunktionen voraussetzen,
* Tests entfernen, damit der Build erfolgreich ist,
* Warnungen pauschal deaktivieren,
* Fehler durch leere Catch-Blöcke unterdrücken,
* unsichere Platzhalter als fertige Implementierung ausgeben,
* öffentliche Schnittstellen ohne Notwendigkeit ändern,
* neue Abhängigkeiten ohne Begründung hinzufügen.

Nach einer Änderung muss der Agent:

1. den Code formatieren,
2. das betroffene Target kompilieren,
3. relevante Tests ausführen,
4. Compiler-Warnungen prüfen,
5. mögliche Randfälle prüfen,
6. die Änderung auf unnötige Komplexität untersuchen.

## 24.1 Keine Annahmen über unbekannte APIs

Wenn Verhalten oder Signatur einer API unklar sind:

* bestehende Deklarationen lesen,
* Verwendungen im Projekt suchen,
* Dokumentation prüfen,
* keine Signaturen oder Rückgabewerte erfinden.

## 24.2 Bestehende Muster respektieren

Bei konsistentem bestehendem Code ist dessen Stil zu übernehmen, sofern er nicht gegen Sicherheits- oder Korrektheitsregeln verstößt.

## 24.3 Keine unvollständigen Lösungen

Nicht als fertige Lösung zulässig:

```cpp
// TODO: implement
return {};
```

Mocks, Stubs und Platzhalter sind nur zulässig, wenn sie ausdrücklich Teil der Aufgabe sind.

## 24.4 Fehler sichtbar machen

Wenn eine Änderung nicht vollständig validiert werden konnte, muss dies klar genannt werden.

Beispiele:

```text
Build konnte nicht ausgeführt werden, weil Abhängigkeit X fehlt.
Unit-Tests wurden ausgeführt; Integrationstests benötigen einen Datenbankdienst.
```

---

# 25. Definition of Done

Eine Änderung gilt nur als abgeschlossen, wenn:

* der Code kompiliert,
* keine neuen Compiler-Warnungen entstehen,
* der Code formatiert ist,
* statische Analyse keine relevanten neuen Fehler meldet,
* vorhandene Tests erfolgreich sind,
* neue Logik durch Tests abgedeckt ist,
* Fehlerfälle berücksichtigt sind,
* keine Geheimnisse oder vertraulichen Daten enthalten sind,
* öffentliche Schnittstellen dokumentiert sind,
* keine unnötige Komplexität eingeführt wurde,
* Dateinamen und Bezeichner den Regeln entsprechen.

---

# 26. Beispiel für regelkonformen Code

```cpp
#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

namespace project::network {

enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Failed
};

struct ConnectionOptions {
    std::chrono::milliseconds timeout{5000};
    std::uint32_t maximum_retry_count{3};
};

class HttpClient final {
public:
    explicit HttpClient(std::string base_url, ConnectionOptions options = {});

    [[nodiscard]] bool connect();
    [[nodiscard]] bool is_connected() const noexcept;
    [[nodiscard]] std::optional<std::string> get(std::string_view path) const;

private:
    [[nodiscard]] bool should_retry(std::uint32_t retry_count) const noexcept;

    std::string base_url_;
    ConnectionOptions options_;
    ConnectionState state_{ConnectionState::Disconnected};
};

}  // namespace project::network
```

---

# 27. Entscheidungsregel bei Unsicherheit

Wenn eine Situation nicht ausdrücklich geregelt ist, gilt:

1. Sicherheit vor Bequemlichkeit.
2. Klarheit vor Kürze.
3. Explizite Typen vor verstecktem Verhalten.
4. Komposition vor Vererbung.
5. RAII vor manueller Ressourcenverwaltung.
6. Standardbibliothek vor eigener Hilfsimplementierung.
7. Kleine, testbare Einheiten vor großen Funktionen.
8. Bestehende Projektkonvention vor persönlicher Präferenz.
